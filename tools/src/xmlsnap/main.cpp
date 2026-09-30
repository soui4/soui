/**
 * xmlsnap - SOUI 控件离屏截图工具
 *
 * 用途：为 soui-docs 文档批量生成控件示例 PNG 截图。
 *
 * 原理：
 *   1. SAppCfg(Render_Gdi + ImgDecoder_Stb + SysRes + AppRes) 初始化 SApplication，
 *      内核控件默认注册，SCtrlsRegister 注册扩展控件（listctrl/propgrid/treectrl 等）。
 *   2. 解析 <resdir>/uires.idx，收集全部 layout XML。
 *   3. 每个布局：创建宿主窗口（移到屏幕外 -30000 并 SW_SHOWNOACTIVATE 显示，
 *      泵消息让 WM_SIZE 下发 -> 各控件缓存 RT 创建），再 PrintWindow 触发
 *      SHostWnd::OnPrint —— SOUI 官方离屏渲染管线（UpdateLayout -> _RedrawRegion
 *      -> UpdatePresenter(dc,...)），输出到 DIB 内存位图。
 *   4. DIB 的 BGRA 像素转 RGBA（alpha 强制 255，GDI 不写 alpha 字节），
 *      stb 在内存编码 PNG，直接写文件（无中间临时文件）。
 *
 * 用法：
 *   xmlsnap -r <resdir> -o <outdir|file.png> [-i name[,name2...]] [-w width] [-h height] [-y sysres-path]
 *
 * 退出码：0 成功；-1 用法错误；-2 初始化失败；-3 渲染失败
 */

#include "stdafx.h"
#include <SAppCfg.h>
#include <shellapi.h>
#include <vector>
#include <string>

#ifndef LIB_SOUI_COM
#define STB_IMAGE_WRITE_IMPLEMENTATION
#endif//LIB_SOUI_COM
#include <stb_image_write.h>

#include "SCtrlsRegister.h"
#include <helper/SAdapterBase.h>
#include <control/SListView.h>
#include <control/SMCListView.h>
#include <control/STileView.h>
#include <control/STreeView.h>
#include <control/SComboView.h>

// ---------- 通用辅助 ----------

static bool WriteBytesToFile(const SStringT &path, const std::vector<BYTE> &buf)
{
    FILE *f = _tfopen(path.c_str(), _T("wb"));
    if (!f)
        return false;
    size_t w = buf.empty() ? 0 : fwrite(&buf[0], 1, buf.size(), f);
    fclose(f);
    return w == buf.size();
}

// 把 BGRA 像素缓冲（GDI DIB）转成 RGBA 直显，并用 stb 在内存中编码 PNG（无中间文件）。
// GDI 绘制不写 alpha 字节（恒为 0），文档截图要求不透明，强制 255。
static bool BgraBitsToPngBytes(const BYTE *src, int w, int h, std::vector<BYTE> &out)
{
    if (!src || w <= 0 || h <= 0)
        return false;
    std::vector<BYTE> rgba((size_t)w * h * 4);
    for (int y = 0; y < h; ++y)
    {
        const BYTE *srow = src + (size_t)y * w * 4;
        BYTE *drow = &rgba[0] + (size_t)y * w * 4;
        for (int x = 0; x < w; ++x)
        {
            drow[x * 4 + 0] = srow[x * 4 + 2];
            drow[x * 4 + 1] = srow[x * 4 + 1];
            drow[x * 4 + 2] = srow[x * 4 + 0];
            drow[x * 4 + 3] = 255;
        }
    }
    int len = 0;
    unsigned char *png = stbi_write_png_to_mem(&rgba[0], (int)(w * 4), (int)w, (int)h, 4, &len);
    if (!png || len <= 0)
        return false;
    out.assign(png, png + len);
    free(png);
    return true;
}

// ---------- 资源目录解析 ----------

struct LayoutEntry
{
    SStringT strName; // layout 资源名
    SStringT strPath; // 相对 resdir 的文件路径
};

// 解析 <resdir>/uires.idx，收集 <layout> 段声明的全部 <file name path/>
static bool LoadLayoutIndex(const SStringT &strResDir, std::vector<LayoutEntry> &lst)
{
    SStringT strIdx = strResDir + _T("/uires.idx");
    SXmlDoc xmlDoc;
    if (!xmlDoc.load_file(strIdx.c_str(), xml_parse_default, enc_auto))
        return false;
    SXmlNode xmlResource = xmlDoc.root().child(L"resource");
    if (!xmlResource)
        return false;
    SXmlNode xmlLayout = xmlResource.child(L"layout");
    if (!xmlLayout)
        return false;
    SXmlNode xmlFile = xmlLayout.child(L"file");
    while (xmlFile)
    {
        LayoutEntry entry;
        entry.strName = S_CW2T(xmlFile.attribute(L"name").as_string());
        entry.strPath = S_CW2T(xmlFile.attribute(L"path").as_string());
        entry.strPath.ReplaceChar(_T('\\'), _T('/'));
        if (!entry.strName.IsEmpty() && !entry.strPath.IsEmpty())
            lst.push_back(entry);
        xmlFile = xmlFile.next_sibling(L"file");
    }
    return !lst.empty();
}

static int PrintUsage()
{
    _tprintf(_T("Usage: xmlsnap -r <resdir> -o <outdir|file.png> [-i <name[,name2...]>] [-w <width>] [-h <height>] [-y <sysres-path>]\n"));
    _tprintf(_T("  -r  resource dir containing uires.idx + uidef + layout xml\n"));
    _tprintf(_T("  -o  output dir (batch mode) or single .png file (with -i one name)\n"));
    _tprintf(_T("  -i  comma separated layout names; omit to render all layouts\n"));
    _tprintf(_T("  -y  explicit path of soui-sys-resource library (default: beside exe)\n"));
    return -1;
}

// ---------- 演示数据适配器：为 MVC 控件（listview/mclistview/tileview/treeview/comboview）截图提供数据 ----------

class CSnapLvAdapter : public SAdapterBase
{
  public:
    explicit CSnapLvAdapter(const SStringT &strPrefix)
        : m_strPrefix(strPrefix)
    {
    }

    virtual int WINAPI getCount()
    {
        return 8;
    }

    virtual void WINAPI getView(int position, SItemPanel *pItem, SXmlNode xmlTemplate)
    {
        if (pItem->GetChildrenCount() == 0)
            pItem->InitFromXml(&xmlTemplate);
        SWindow *pTxt = pItem->FindChildByName(L"txt_label");
        if (pTxt)
            pTxt->SetWindowText(SStringT().Format(_T("%s %d"), m_strPrefix.c_str(), position + 1));
    }

  private:
    SStringT m_strPrefix;
};

class CSnapMcAdapter : public SMcAdapterBase
{
  public:
    virtual int WINAPI getCount()
    {
        return 6;
    }

    STDMETHOD_(SStringW, GetColumnName)(int iCol) SCONST
    {
        return SStringW().Format(L"col%d", iCol+1);
    }

    virtual void WINAPI getView(int position, SItemPanel *pItem, SXmlNode xmlTemplate)
    {
        if (pItem->GetChildrenCount() == 0)
            pItem->InitFromXml(&xmlTemplate);
        static const struct data
        {
            LPCTSTR name, age, city;
        } rows[] = {
            { _T("张三"), _T("28"), _T("北京") },
            { _T("李四"), _T("25"), _T("上海") },
            { _T("王五"), _T("31"), _T("广州") },
            { _T("赵六"), _T("27"), _T("深圳") },
            { _T("钱七"), _T("35"), _T("杭州") },
        };
        const struct data&r = rows[position % (int)(sizeof(rows) / sizeof(rows[0]))];
        SWindow *pName = pItem->FindChildByName(L"txt_name");
        if (pName)
            pName->SetWindowText(SStringT().Format(_T("%s（第%d行）"), r.name, position + 1));
        SWindow *pAge = pItem->FindChildByName(L"txt_age");
        if (pAge)
            pAge->SetWindowText(r.age);
        SWindow *pCity = pItem->FindChildByName(L"txt_city");
        if (pCity)
            pCity->SetWindowText(r.city);
    }
};

class CSnapTvAdapter : public STreeAdapterBase<SStringT>
{
  public:
    CSnapTvAdapter()
    {
        HSTREEITEM h1 = InsertItem(SStringT(_T("华北地区")));
        SetItemExpanded(h1, TRUE);
        InsertItem(SStringT(_T("北京")), h1);
        InsertItem(SStringT(_T("天津")), h1);
        HSTREEITEM h2 = InsertItem(SStringT(_T("华东地区")));
        SetItemExpanded(h2, TRUE);
        InsertItem(SStringT(_T("上海")), h2);
        InsertItem(SStringT(_T("杭州")), h2);
        InsertItem(SStringT(_T("南京")), h2);
    }

    virtual void WINAPI getView(HSTREEITEM loc, SItemPanel *pItem, SXmlNode xmlTemplate)
    {
        if (pItem->GetChildrenCount() == 0)
            pItem->InitFromXml(&xmlTemplate);
        SWindow *pTxt = pItem->FindChildByName(L"txt_label");
        if (pTxt)
            pTxt->SetWindowText(GetItemData(loc));
    }
};

// 给布局中预留的命名控件填充演示数据（如 listctrl 的行数据）
static void PopulateDemoData(SHostWnd &host)
{
    SListCtrl *pLC = host.FindChildByName2<SListCtrl>(L"lc_demo");
    if (pLC && pLC->GetColumnCount() > 0)
    {
        struct RowData
        {
            LPCTSTR name, gender, age, city;
        };
        static const RowData rows[] = {
            { _T("张三"), _T("男"), _T("28"), _T("北京") },
            { _T("李四"), _T("女"), _T("25"), _T("上海") },
            { _T("王五"), _T("男"), _T("31"), _T("广州") },
            { _T("赵六"), _T("女"), _T("27"), _T("深圳") },
            { _T("钱七"), _T("男"), _T("35"), _T("杭州") },
        };
        for (int i = 0; i < (int)(sizeof(rows) / sizeof(rows[0])); ++i)
        {
            int nItem = pLC->InsertItem(i, rows[i].name);
            if (nItem < 0)
                break;
            pLC->SetSubItemText(nItem, 1, rows[i].gender);
            pLC->SetSubItemText(nItem, 2, rows[i].age);
            pLC->SetSubItemText(nItem, 3, rows[i].city);
        }
    }

    // listview（虚拟化单列列表）
    SListView *pLv = host.FindChildByName2<SListView>(L"lv_demo");
    if (pLv)
    {
        ILvAdapter *pAdapter = new CSnapLvAdapter(_T("列表项目"));
        pLv->SetAdapter(pAdapter);
        pAdapter->Release();
    }

    // mclistview（虚拟化多列列表）
    SMCListView *pMcLv = host.FindChildByName2<SMCListView>(L"mclv_demo");
    if (pMcLv)
    {
        IMcAdapter *pAdapter = new CSnapMcAdapter;
        pMcLv->SetAdapter(pAdapter);
        pAdapter->Release();
    }

    // tileview（宫格列表）
    STileView *pTile = host.FindChildByName2<STileView>(L"tile_demo");
    if (pTile)
    {
        ILvAdapter *pAdapter = new CSnapLvAdapter(_T("宫格"));
        pTile->SetAdapter(pAdapter);
        pAdapter->Release();
    }

    // treeview（树形列表）
    STreeView *pTv = host.FindChildByName2<STreeView>(L"tv_demo");
    if (pTv)
    {
        ITvAdapter *pAdapter = new CSnapTvAdapter;
        pTv->SetAdapter(pAdapter);
        pAdapter->Release();
    }

    // comboview（下拉内嵌虚拟化列表）：选中第 1 项让控件显示文本
    SComboView *pCombo = host.FindChildByName2<SComboView>(L"combo_demo");
    if (pCombo)
    {
        SListView *pLvDrop = pCombo->GetListView();
        if (pLvDrop)
        {
            ILvAdapter *pAdapter = new CSnapLvAdapter(_T("城市选项"));
            pLvDrop->SetAdapter(pAdapter);
            pAdapter->Release();
            pCombo->SetCurSel(1);
        }
    }
}

// 渲染单个布局：加载 XML -> 创建屏幕外宿主窗口 -> PrintWindow 离屏绘制 -> PNG
static bool RenderLayout(const SStringT &strResDir,
                         const LayoutEntry &entry,
                         const SStringT &strOutFile,
                         int nForceW, int nForceH)
{
    SStringT strXmlFile = strResDir + _T("/") + entry.strPath;
    SXmlDoc xmlDoc;
    if (!xmlDoc.load_file(strXmlFile.c_str(), xml_parse_default, enc_auto))
    {
        _tprintf(_T("error: load layout xml failed: %s\n"), strXmlFile.c_str());
        return false;
    }
    SXmlNode xmlRoot = xmlDoc.root().first_child();
    if (!xmlRoot)
    {
        _tprintf(_T("error: empty layout xml: %s\n"), strXmlFile.c_str());
        return false;
    }

    // 尺寸：优先命令行 -w/-h，其次布局根节点 width/height 属性，最后默认值
    int nW = nForceW;
    int nH = nForceH;
    if (nW <= 0)
        nW = (int)xmlRoot.attribute(L"width").as_int(0);
    if (nH <= 0)
        nH = (int)xmlRoot.attribute(L"height").as_int(0);
    if (nW <= 0)
        nW = 420;
    if (nH <= 0)
        nH = 260;

    // 创建宿主窗口：放在屏幕外隐藏区域，但必须真正 ShowWindow，
    // 否则 WM_SIZE 不会下发，各控件的缓存 RT（m_cachedRT）不会创建，
    // 且 SHostWnd::OnPrint 开头的 IsWindowVisible 检查也不会通过
    SHostWnd host;
    HWND hWnd = host.CreateEx(NULL, WS_POPUP, WS_EX_TOOLWINDOW | WS_EX_TOPMOST, 0, 0, nW, nH, &xmlRoot);
    if (!hWnd)
    {
        _tprintf(_T("error: create host window failed: %s\n"), entry.strName.c_str());
        return false;
    }

    // 填充演示数据（listctrl 行、MVC 控件适配器等）
    PopulateDemoData(host);

    bool bOk = false;
    do
    {
        SWindow *pRoot = host.GetRoot();
        if (!pRoot)
            break;
        CRect rcWnd;
        pRoot->GetWindowRect(&rcWnd);
        if (rcWnd.Width() <= 0 || rcWnd.Height() <= 0)
            break;
        SAutoRefPtr<IRenderTarget> memRT;
        GETRENDERFACTORY->CreateRenderTarget(&memRT, nW, nH);
        if (!memRT)
            break;
        HDC hdc = memRT->GetDC(0);
        host.SendMessage(WM_PRINT, (WPARAM)hdc, PRF_CLIENT);
        memRT->ReleaseDC(hdc);
        IBitmapS* pBmp = (IBitmapS*)memRT->GetCurrentObject(OT_BITMAP);
        void* pBits = pBmp->LockPixelBits();
        std::vector<BYTE> png;
        bOk = BgraBitsToPngBytes((const BYTE*)pBits, rcWnd.Width(), rcWnd.Height(), png);
        pBmp->UnlockPixelBits(pBits);
        
        if (!bOk)
        {
            _tprintf(_T("error: render to png failed: %s\n"),
                     entry.strName.c_str());
            break;
        }
        if (!WriteBytesToFile(strOutFile, png))
        {
            _tprintf(_T("error: write file failed: %s\n"), strOutFile.c_str());
            bOk = false;
            break;
        }
        _tprintf(_T("ok: %s (%dx%d)\n"), strOutFile.c_str(), rcWnd.Width(), rcWnd.Height());
    } while (false);

    host.DestroyWindow();
    return bOk;
}

int _tmain(int argc, TCHAR *argv[])
{
    SStringT strResDir, strOutput, strInclude, strSysRes;
    int nForceW = -1, nForceH = -1;

    // 手动解析命令行（保持与 svgcvt 一致的 -x 选项风格）
    for (int i = 1; i < argc; ++i)
    {
        SStringT arg = argv[i];
        if (arg.GetLength() != 2 || arg[0] != _T('-'))
        {
            _tprintf(_T("unknown argument: %s\n"), arg.c_str());
            return PrintUsage();
        }
        switch (arg[1])
        {
        case 'r':
            if (++i >= argc)
                return PrintUsage();
            strResDir = argv[i];
            break;
        case 'o':
            if (++i >= argc)
                return PrintUsage();
            strOutput = argv[i];
            break;
        case 'i':
            if (++i >= argc)
                return PrintUsage();
            strInclude = argv[i];
            break;
        case 'w':
            if (++i >= argc)
                return PrintUsage();
            nForceW = _ttoi(argv[i]);
            break;
        case 'h':
            if (++i >= argc)
                return PrintUsage();
            nForceH = _ttoi(argv[i]);
            break;
        case 'y':
            if (++i >= argc)
                return PrintUsage();
            strSysRes = argv[i];
            break;
        default:
            _tprintf(_T("unknown option: %s\n"), arg.c_str());
            return PrintUsage();
        }
    }

    if (strResDir.IsEmpty() || strOutput.IsEmpty())
    {
        return PrintUsage();
    }

    // 解析 resdir 为绝对路径（资源加载依赖完整路径）
    TCHAR szFull[MAX_PATH * 2];
    if (GetFullPathName(strResDir.c_str(), MAX_PATH * 2, szFull, NULL))
        strResDir = szFull;

    // 解析待渲染的 layout 列表
    std::vector<LayoutEntry> lstAll;
    if (!LoadLayoutIndex(strResDir, lstAll))
    {
        _tprintf(_T("error: no layout entries found in %s/uires.idx\n"), strResDir.c_str());
        return -1;
    }

    std::vector<LayoutEntry> lstTodo;
    if (strInclude.IsEmpty())
    {
        lstTodo = lstAll;
    }
    else
    {
        // 按逗号拆分 -i 列表
        SStringT strList = strInclude;
        int nPos = 0;
        while (nPos <= strList.GetLength())
        {
            int nComma = strList.Find(_T(","), nPos);
            SStringT one = (nComma < 0) ? strList.Mid(nPos) : strList.Mid(nPos, nComma - nPos);
            one.TrimBlank();
            if (!one.IsEmpty())
            {
                bool bFound = false;
                for (size_t k = 0; k < lstAll.size(); ++k)
                {
                    if (lstAll[k].strName.CompareNoCase(one) == 0)
                    {
                        lstTodo.push_back(lstAll[k]);
                        bFound = true;
                        break;
                    }
                }
                if (!bFound)
                    _tprintf(_T("warning: layout not found in uires.idx: %s\n"), one.c_str());
            }
            if (nComma < 0)
                break;
            nPos = nComma + 1;
        }
    }
    if (lstTodo.empty())
    {
        _tprintf(_T("error: nothing to render\n"));
        return -1;
    }

    // 单个输出且是 .png 结尾 => 单文件模式
    bool bSingleFile = (lstTodo.size() == 1) && strOutput.EndsWith(_T(".png"), true);

    // ---------- 初始化 SOUI ----------
    HINSTANCE hInst = GetModuleHandle(NULL);
    SApplication app(hInst, _T("soui4host"));

    // 注册 controls.extend 扩展控件（listctrl/propgrid/treectrl 等）
    SCtrlsRegister::RegisterCtrls(&app);

    SAppCfg cfg;
    cfg.SetRender(Render_Gdi).SetImgDecoder(ImgDecoder_Stb);
    cfg.SetSysResFile(strSysRes);
    // 应用资源目录（uidef 皮肤/字体 + layout XML）
    cfg.SetAppResFile(strResDir);

    if (!cfg.DoConfig(&app))
    {
        _tprintf(_T("error: init soui failed (sysres=%s resdir=%s)\n"), strSysRes.c_str(), strResDir.c_str());
        return -2;
    }

    // ---------- 逐个渲染 ----------
    SStringT strOutDir = strOutput;
    if (!bSingleFile)
    {
        // 目录模式：确保输出目录存在
        CreateDirectory(strOutDir.c_str(), NULL);
    }
    else
    {
        int nCut = strOutDir.ReverseFind(_T('/'));
        int nCut2 = strOutDir.ReverseFind(_T('\\'));
        if (nCut < nCut2)
            nCut = nCut2;
        if (nCut > 0)
            CreateDirectory(strOutDir.Left(nCut).c_str(), NULL);
    }

    int nFail = 0;
    for (size_t k = 0; k < lstTodo.size(); ++k)
    {
        SStringT strOutFile;
        if (bSingleFile)
            strOutFile = strOutput;
        else
            strOutFile.Format(_T("%s/%s.png"), strOutDir.c_str(), lstTodo[k].strName.c_str());
        if (!RenderLayout(strResDir, lstTodo[k], strOutFile, nForceW, nForceH))
            nFail++;
    }

    if (nFail > 0)
    {
        _tprintf(_T("done with %d failure(s)\n"), nFail);
        return -3;
    }
    _tprintf(_T("all done: %d image(s)\n"), (int)lstTodo.size());
    return 0;
}

#if !defined(_WIN32) || defined(__MINGW32__)
int main(int argc, TCHAR** argv)
{
	return _tmain(argc, argv);
}
#endif //_WIN32
