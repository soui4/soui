// dui-demo.cpp : main source file
//

#include "stdafx.h"
#include <commask.h>
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

int _tmain(int argc, TCHAR* argv[]) 
{
    int nRet = 0;
    HINSTANCE hInst = GetModuleHandle(NULL);
    SApplication app(hInst,_T("soui4host"),SObjectEmptyRegister());
    SAppCfg cfg;
    cfg.SetRender(Render_Gdi).SetImgDecoder(ImgDecoder_Stb);

    if (!cfg.DoConfig(&app))
    {
        return -1;
    }
    SStringT strInput, strOutput;
    int nSize = -1;
    int c;
	while ((c = xgetopt(argc, argv, _T(":i:s:o:"))) != EOF || optarg!=NULL)
	{
		switch (c)
		{
		case 'i':strInput=optarg;break;
		case 'o':strOutput=optarg;break;
		case 's':nSize=_ttoi(optarg);break;
        case EOF:
            optind++;
            break;
        }
	}
    if(strInput.IsEmpty() || !strInput.EndsWith(_T(".svg"),true))
    {
        _ftprintf(stderr,_T("Usage: svg2png -i input.svg -o output.png -s size\n"));
        return -1;
    }
    ISvgObj * pSvg = CreateSvgFromFile(strInput);
    if(!pSvg){
        _ftprintf(stderr,_T("load svg failed! input=%s\n"),strInput.c_str());
        return -2;
    }
    int nWid = pSvg->GetWidth();
    int nHei = pSvg->GetHeight();
	if(nWid<=0 || nHei<=0){
		_ftprintf(stderr,_T("invalid image size, width=%d,height=%d! input=%s\n"),nWid, nHei,strInput.c_str());
		pSvg->Release();
		return -3;
	}
    float fRatio = (float)nHei/nWid;
    int cx = nSize==-1?nWid:nSize;
    int cy = cx * fRatio;

    if(strOutput.IsEmpty()){
        strOutput = strInput.Left(strInput.GetLength()-4)+SStringT().Format(_T("_%d.png"),cx);
    }
    IRenderTarget *pRT=NULL;
    GETRENDERFACTORY->CreateRenderTarget(&pRT,cx,cy);
    pRT->BeginDraw();
    CRect rcDst(0,0,cx,cy);
    pRT->DrawSVG(pSvg,&rcDst);
    pRT->EndDraw();
    IBitmapS *pBmp=(IBitmapS*)pRT->GetCurrentObject(OT_BITMAP);
    pBmp->Save2(S_CT2W(strOutput), Img_PNG);
    pSvg->Release();
    pRT->Release();
    return 0;
}

#if !defined(_WIN32) || defined(__MINGW32__)
int main(int argc, char **argv)
{
    return _tmain(argc,argv);
}
#endif //_WIN32