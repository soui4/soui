// svgcvt.cpp : 将 SVG 渲染为位图并导出 png / ico / icns
//
// 支持的输出格式：
//   png  - 单尺寸位图（内存编码，直出文件）
//   ico  - 多尺寸图标容器，内部直接内嵌各尺寸的 PNG（Windows 原生支持 PNG 图标）
//   icns - macOS 图标容器，内部内嵌标准尺寸（32/64/128/256/512/1024）的 PNG
//


#include "stdafx.h"
#ifndef SOUI_ENABLE_COM_LIB
#define STB_IMAGE_WRITE_IMPLEMENTATION
#endif//SOUI_ENABLE_COM_LIB
#include <stb_image_write.h>
#include <commask.h>
#include <vector>
#include <map>
#include <cstdio>
#define SCOM_MASK (scom_mask_render_gdi|scom_mask_imgdecoder_stb)

#include <SAppCfg.h>
#include <helper/SplitString.h>
#include <core/Svg.h>
#include <tchar.h>
#include "XGetopt.h"
using namespace SNS;

#ifdef _UNICODE
#define _ttoi _wtoi
#define CreateSvgFromFile CreateSvgFromFileW
#else
#define _ttoi atoi
#define CreateSvgFromFile CreateSvgFromFileA
#endif

// ---------- 小工具 ----------

// 以二进制方式打开文件（兼容 _UNICODE / 多字节）
static FILE* OpenFile(const SStringT& path, bool bWrite)
{
#ifdef _UNICODE
    return _wfopen(path.c_str(), bWrite ? L"wb" : L"rb");
#else
    return fopen(path.c_str(), bWrite ? "wb" : "rb");
#endif
}

// 把字节缓冲写入文件
static bool WriteBytesToFile(const SStringT& path, const std::vector<BYTE>& buf)
{
    FILE* f = OpenFile(path, true);
    if (!f) return false;
    size_t w = fwrite(&buf[0], 1, buf.size(), f);
    fclose(f);
    return w == buf.size();
}

// 小端写入
static void PutLE(std::vector<BYTE>& b, WORD v)
{
    b.push_back((BYTE)(v & 0xFF));
    b.push_back((BYTE)((v >> 8) & 0xFF));
}
static void PutLE(std::vector<BYTE>& b, DWORD v)
{
    b.push_back((BYTE)(v & 0xFF));
    b.push_back((BYTE)((v >> 8) & 0xFF));
    b.push_back((BYTE)((v >> 16) & 0xFF));
    b.push_back((BYTE)((v >> 24) & 0xFF));
}
// 大端写入
static void PutBE(std::vector<BYTE>& b, DWORD v)
{
    b.push_back((BYTE)((v >> 24) & 0xFF));
    b.push_back((BYTE)((v >> 16) & 0xFF));
    b.push_back((BYTE)((v >> 8) & 0xFF));
    b.push_back((BYTE)(v & 0xFF));
}

// 把 SVG 渲染为指定尺寸位图，锁定像素，转成 RGBA 直显（解预乘）PNG 字节（内存，无中间文件）
static bool RenderSvgToPngBytes(ISvgObj* pSvg, int cx, int cy, std::vector<BYTE>& out)
{
    IRenderTarget* pRT = NULL;
    if (!GETRENDERFACTORY->CreateRenderTarget(&pRT, cx, cy))
        return false;
    pRT->BeginDraw();
    CRect rc(0, 0, cx, cy);
    pRT->DrawSVG(pSvg, &rc);
    pRT->EndDraw();

    IBitmapS* pBmp = (IBitmapS*)pRT->GetCurrentObject(OT_BITMAP);
    if (!pBmp)
    {
        pRT->Release();
        return false;
    }

    LPVOID pLock = pBmp->LockPixelBits();
    if (!pLock)
    {
        pRT->Release();
        return false;
    }

    UINT w = pBmp->Width();
    UINT h = pBmp->Height();
    // 每像素 4 字节，w*4 永远 4 字节对齐，无行 padding；pitch = w*4。
    const BYTE* src = (const BYTE*)pLock;

    // 复制像素并立即解锁（BGRA 预乘 -> RGBA 直显）
    std::vector<BYTE> rgba((size_t)w * h * 4);
    for (UINT y = 0; y < h; ++y)
    {
        const BYTE* srow = src + (size_t)y * w * 4;
        BYTE* drow = &rgba[0] + (size_t)y * w * 4;
        for (UINT x = 0; x < w; ++x)
        {
            BYTE b = srow[x * 4 + 0];
            BYTE g = srow[x * 4 + 1];
            BYTE r = srow[x * 4 + 2];
            BYTE a = srow[x * 4 + 3];
            // 解预乘（与 SOUI SDIBHelper::DisabledStyleMode 同公式）
            if (a != 0 && a != 255)
            {
                r = (BYTE)((r * 255) / a);
                g = (BYTE)((g * 255) / a);
                b = (BYTE)((b * 255) / a);
            }
            drow[x * 4 + 0] = r;
            drow[x * 4 + 1] = g;
            drow[x * 4 + 2] = b;
            drow[x * 4 + 3] = a;
        }
    }
    pBmp->UnlockPixelBits(pLock);
    pRT->Release(); // pBmp 由 RT 托管，随 RT 释放

    // 内存中直接编码 PNG（stb 编码路径与 SOUI Save2 一致，但此处为直显 alpha）
    int len = 0;
    unsigned char* png = stbi_write_png_to_mem(&rgba[0], (int)(w * 4), (int)w, (int)h, 4, &len);
    if (!png || len <= 0)
        return false;
    out.assign(png, png + len);
    free(png); // stb 用 malloc 分配，用 free 释放
    return true;
}

typedef std::vector<std::pair<int, std::vector<BYTE> > > PNGLIST;
// 写出 ICO：ICONDIR + N*ICONDIRENTRY + 各 PNG 数据（全部小端）
static bool WriteIco(const SStringT& path,
                     const PNGLIST& pngs)
{
    std::vector<BYTE> buf;
    PutLE(buf, (WORD)0);          // idReserved
    PutLE(buf, (WORD)1);          // idType = icon
    PutLE(buf, (WORD)pngs.size()); // idCount

    DWORD dataOffset = 6 + 16 * (DWORD)pngs.size();
	for (PNGLIST::const_iterator it = pngs.begin();it!= pngs.end();it++)
    {
        int sz = it->first;         // 正方形图标：宽 == 高
        buf.push_back((BYTE)(sz >= 256 ? 0 : sz)); // bWidth（256 记为 0）
        buf.push_back((BYTE)(sz >= 256 ? 0 : sz)); // bHeight
        buf.push_back(0);  // bColorCount
        buf.push_back(0);  // bReserved
        PutLE(buf, (WORD)1);    // wPlanes
        PutLE(buf, (WORD)32);   // wBitCount
        PutLE(buf, (DWORD)it->second.size()); // dwBytesInRes
        PutLE(buf, dataOffset);               // dwImageOffset
        dataOffset += (DWORD)it->second.size();
    }
    for (PNGLIST::const_iterator it = pngs.begin();it!= pngs.end();it++)
        buf.insert(buf.end(), it->second.begin(), it->second.end());

    return WriteBytesToFile(path, buf);
}

// 写出 ICNS：'icns' + 总长 + 各（OSType + 长度 + PNG 数据），长度均大端
// 标准 PNG 类型码：ic11=32 ic12=64 ic07=128 ic13=256 ic09=512 ic10=1024
static bool WriteIcns(const SStringT& path,
                      const PNGLIST& pngs)
{
	std::map<int, SStringA> kType;
	struct {
		int id;
		LPCSTR val;
	} types[]={
        {32, "ic11"}, {64, "ic12"}, {128, "ic07"},
        {256, "ic13"}, {512, "ic09"}, {1024, "ic10"}
    };
	for(int i=0;i<ARRAYSIZE(types);i++){
		kType.insert(std::make_pair(types[i].id,types[i].val));
	}
    std::vector<BYTE> body;
	for (PNGLIST::const_iterator it2=pngs.begin();it2!=pngs.end();it2++)
    {
		std::map<int, SStringA>::iterator it = kType.find(it2->first);
        if (it == kType.end())
        {
            _tprintf(_T("warning: skip unsupported icns size %d\n"), it2->first);
            continue;
        }
        const char* t = it->second.c_str();
        body.push_back((BYTE)t[0]); body.push_back((BYTE)t[1]);
        body.push_back((BYTE)t[2]); body.push_back((BYTE)t[3]);
        PutBE(body, (DWORD)(it2->second.size() + 8)); // 长度含本 OSType+长度 字段
        body.insert(body.end(), it2->second.begin(), it2->second.end());
    }
    if (body.empty())
        return false;

    std::vector<BYTE> buf;
    buf.push_back('i'); buf.push_back('c'); buf.push_back('n'); buf.push_back('s');
    PutBE(buf, (DWORD)(8 + body.size()));
    buf.insert(buf.end(), body.begin(), body.end());
    return WriteBytesToFile(path, buf);
}

int _tmain(int argc, TCHAR* argv[])
{
    SStringT strInput, strOutput,strFormat=_T("png");
    int nSize = -1;
    int c;
	while ((c = xgetopt(argc, argv, _T(":i:s:o:f:"))) != EOF || optarg != NULL)
    {
        switch (c)
        {
        case 'i':strInput = optarg; break;
        case 'o':strOutput = optarg; break;
        case 's':nSize = _ttoi(optarg); break;
		case 'f':strFormat=optarg;break;
        case EOF:
            optind++;
            break;
        }
    }
    if (strInput.IsEmpty() || !strInput.EndsWith(_T(".svg"), true))
    {
        _tprintf(_T("Usage:svgcvt -i <input.svg> [-o <output>] [-s <size>] [-f png|ico|icns]\n"));
        return -1;
    }

    HINSTANCE hInst = GetModuleHandle(NULL);
    SApplication app(hInst, _T("soui4host"), SObjectEmptyRegister());
    SAppCfg cfg;
    cfg.SetRender(Render_Gdi).SetImgDecoder(ImgDecoder_Stb);

    if (!cfg.DoConfig(&app))
    {
        return -1;
    }
    ISvgObj* pSvg = CreateSvgFromFile(strInput);
    if (!pSvg) {
        _tprintf(_T("load svg failed! input=%s\n"), strInput.c_str());
        return -2;
    }
    int nWid = pSvg->GetWidth();
    int nHei = pSvg->GetHeight();
    if (nWid <= 0 || nHei <= 0) {
        _tprintf(_T("invalid image size, width=%d,height=%d! input=%s\n"), nWid, nHei, strInput.c_str());
        pSvg->Release();
        return -3;
    }
    float fRatio = (float)nHei / nWid;

	strFormat.MakeLower();
    // 解析输出格式（取扩展名）
    if (strOutput.IsEmpty())
    {
        int cxDef = nSize == -1 ? nWid : nSize;
		if(strFormat==_T("png"))
			strOutput = strInput.Left(strInput.GetLength() - 4) + SStringT().Format(_T("_%d.%s"), cxDef,strFormat.c_str());
		else
			strOutput = strInput.Left(strInput.GetLength() - 4) + _T(".") + strFormat;
    }

    bool bOk = false;

    if (strFormat == _T("png"))
    {
        int cx = nSize == -1 ? nWid : nSize;
        int cy = (int)(cx * fRatio);
        std::vector<BYTE> bytes;
        bOk = RenderSvgToPngBytes(pSvg, cx, cy, bytes) && WriteBytesToFile(strOutput, bytes);
        if (!bOk)
            _tprintf(_T("save png failed! output=%s\n"), strOutput.c_str());
    }
    else if (strFormat == _T("ico") || strFormat == _T("icns"))
    {
        if (nSize != -1)
        {
            _tprintf(_T("note: -s is ignored for ico/icns (multi-size by design)\n"));
        }
		BOOL bIco = strFormat == _T("ico");
		// ico / icns 天生是多分辨率，使用标准尺寸集（保持 SVG 宽高比，宽==高用于 ICO/ICNS 头）
		const int kSizeIco[] = {16, 24, 32, 48, 64, 128, 256};
		const int kSizeIcns[] = {32, 64, 128, 256, 512, 1024};
		const int *SizeIco = bIco?kSizeIco:kSizeIcns;
		int   nSizeIco = bIco?ARRAYSIZE(kSizeIco):ARRAYSIZE(kSizeIcns);
		
        std::vector<std::pair<int, std::vector<BYTE>>> pngs;
        for (int i=0;i<nSizeIco;i++)
        {
			int s = SizeIco[i];
            int cx = s;
            int cy = (int)(s * fRatio);
            if (cy <= 0) cy = 1;
            std::vector<BYTE> bytes;
            if (!RenderSvgToPngBytes(pSvg, cx, cy, bytes))
            {
                _tprintf(_T("warning: render size %d failed, skipped\n"), s);
                continue;
            }
			pngs.push_back(std::make_pair(s, bytes));
        }

        if (pngs.empty())
        {
            _tprintf(_T("no icon images generated! output=%s\n"), strOutput.c_str());
            pSvg->Release();
            return -5;
        }
        bOk =bIco ? WriteIco(strOutput, pngs) : WriteIcns(strOutput, pngs);
        if (!bOk)
            _tprintf(_T("save %s failed! output=%s\n"), strFormat.c_str(), strOutput.c_str());
    }
    else
    {
        _tprintf(_T("unsupported output format: %s (use png/ico/icns)\n"), strFormat.c_str());
        pSvg->Release();
        return -4;
    }

    pSvg->Release();
    if (!bOk)
        return -6;

    _tprintf(_T("ok: %s\n"), strOutput.c_str());
    return 0;
}

#if !defined(_WIN32) || defined(__MINGW32__)
int main(int argc, char** argv)
{
    return _tmain(argc, argv);
}
#endif //_WIN32
