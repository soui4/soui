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

int WINAPI _tWinMain(HINSTANCE hInstance, HINSTANCE /*hPrevInstance*/, LPTSTR lpstrCmdLine, int /*nCmdShow*/)
{
    int nRet = 0;
    SApplication app(hInstance,_T("soui4host"),SObjectEmptyRegister());
    SAppCfg cfg;
    cfg.SetRender(Render_Gdi).SetImgDecoder(ImgDecoder_Stb);

    if (!cfg.DoConfig(&app))
    {
        return -1;
    }
    SStringT cmdLine(lpstrCmdLine);
    SStringTList strArgs;
    int argc = SplitString(cmdLine, _T(' '), strArgs);
    if (strArgs[0].Find(_T("svg2png.exe")) == -1)
    {
        strArgs.InsertAt(0, _T("svg2png.exe"));
        argc++;
    }
    TCHAR **argv = new TCHAR *[argc];
    for(int i=0;i<argc;i++){
        strArgs[i].Trim('\"');
        argv[i]=(TCHAR*)strArgs[i].c_str();
    }
    SStringT strInput, strOutput;
    int nSize = 32;
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
    delete []argv;
    if(strInput.IsEmpty() || !strInput.EndsWith(_T(".svg"),true))
    {
        _ftprintf(stderr,_T("Usage: svg2png -i input.xml -o output.png -s size\n"));
        return -1;
    }
    ISvgObj * pSvg = CreateSvgFromFile(strInput);
    if(!pSvg){
        _ftprintf(stderr,_T("load svg failed! input=%s\n"),strInput.c_str());
        return -2;
    }
    if(strOutput.IsEmpty()){
        strOutput = strInput.Left(strInput.GetLength()-4)+SStringT().Format(_T("_%d.png"),nSize);
    }
    IRenderTarget *pRT=NULL;
    GETRENDERFACTORY->CreateRenderTarget(&pRT,nSize,nSize);
    pRT->BeginDraw();
    CRect rcDst(0,0,nSize,nSize);
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
    HINSTANCE hInst = GetModuleHandle(NULL);
    return _tWinMain(hInst, 0, GetCommandLine(), SW_SHOWNORMAL);
}
#endif //_WIN32