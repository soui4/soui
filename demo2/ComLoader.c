#include <Windows.h>
#include <config.h>
#include "ComLoader.h"
#include <tchar.h>
#include <commask.h>

#ifndef SCOM_MASK
#define SCOM_MASK scom_mask_scom_all
#endif 


typedef BOOL (*FunCreateInstance)(IObjRef **ppObj);

#ifdef DLL_SOUI_COM

struct SComInfo
{
	SComID id;
	LPCTSTR pszDll;
	HMODULE hMod;
};

#if defined(_DEBUG) && !defined(NO_DEBUG_SUFFIX)
struct SComInfo s_comInfo[]={
	{Decoder_Png,_T("imgdecoder-pngd.dll"),NULL},
	{Decoder_Gdip,_T("imgdecoder-gdipd.dll"),NULL},
	{Decoder_Wic,_T("imgdecoder-wicd.dll"),NULL},
	{Decoder_Stb,_T("imgdecoder-stbd.dll"),NULL},
	{Render_Gdi,_T("render-gdid.dll"),NULL},
	{Render_Skia,_T("render-skiad.dll"),NULL},
	{Render_D2D,_T("render-d2dd.dll"),NULL},
	{Log4Z,_T("log4zd.dll"),NULL},
	{Resprovider_7Zip,_T("resprovider-7zipd.dll"),NULL},
	{Resprovider_Zip,_T("resprovider-zipd.dll"),NULL},
	{Script_Lua,_T("scriptmodule-luad.dll"),NULL},
	{TaskLoop,_T("TaskLoopd.dll"),NULL},
	{Translator,_T("translatord.dll"),NULL},
};
#else
struct SComInfo s_comInfo[]={
	{Decoder_Png,_T("imgdecoder-png.dll"),NULL},
	{Decoder_Gdip,_T("imgdecoder-gdip.dll"),NULL},
	{Decoder_Wic,_T("imgdecoder-wic.dll"),NULL},
	{Decoder_Stb,_T("imgdecoder-stb.dll"),NULL},
	{Render_Gdi,_T("render-gdi.dll"),NULL},
	{Render_Skia,_T("render-skia.dll"),NULL},
	{Render_D2D,_T("render-d2d.dll"),NULL},
	{Log4Z,_T("log4z.dll"),NULL},
	{Resprovider_7Zip,_T("resprovider-7zip.dll"),NULL},
	{Resprovider_Zip,_T("resprovider-zip.dll"),NULL},
	{Script_Lua,_T("scriptmodule-lua.dll"),NULL},
	{TaskLoop,_T("TaskLoop.dll"),NULL},
	{Translator,_T("translator.dll"),NULL},
};
#endif
	BOOL LoadComObj(SComID id,IObjRef ** ppObj)
	{
		FunCreateInstance fun=NULL;
		HMODULE hDll=s_comInfo[id].hMod;
		LPCTSTR pszDll = s_comInfo[id].pszDll;
		if(!hDll)
		{
			hDll = LoadLibrary(pszDll);
			if(!hDll)
				return FALSE;
		}
		fun = (FunCreateInstance)GetProcAddress(hDll,"SCreateInstance");
		if(!fun)
		{
			FreeLibrary(hDll);
			return FALSE;
		}
		s_comInfo[id].hMod = hDll;
		return fun(ppObj);
	}
#else

#ifdef _MSC_VER
#pragma message("LIB_SOUI_COM")
#endif

#pragma comment(lib,"Usp10")
#pragma comment(lib,"opengl32")

#if defined(_DEBUG) && !defined(NO_DEBUG_SUFFIX)
#if(SCOM_MASK&scom_mask_render_skia)
#pragma comment(lib,"skiad")
#pragma comment(lib,"render-skiad")
#endif
#if(SCOM_MASK&scom_mask_render_gdi)
#pragma comment(lib,"render-gdid")
#endif
#if(SCOM_MASK&scom_mask_render_d2d)
#pragma comment(lib,"render-d2dd")
#endif
#if(SCOM_MASK&scom_mask_imgdecoder_wic)
#pragma comment(lib,"imgdecoder-wicd")
#endif
#if(SCOM_MASK&scom_mask_imgdecoder_png)
#pragma comment(lib,"pngd")
#pragma comment(lib,"zlibd")
#pragma comment(lib,"imgdecoder-pngd")
#endif
#if(SCOM_MASK&scom_mask_imgdecoder_stb)
#pragma comment(lib,"imgdecoder-stbd")
#endif
#if(SCOM_MASK&scom_mask_imgdecoder_gdip)
#pragma comment(lib,"imgdecoder-gdipd")
#endif
#if(SCOM_MASK&scom_mask_resprovider_zip)
#pragma comment(lib,"zlibd")
#pragma comment(lib,"resprovider-zipd")
#endif
#if(SCOM_MASK&scom_mask_resprovider_7z)
#pragma comment(lib,"7zd")
#pragma comment(lib,"resprovider-7zipd")
#endif
#if(SCOM_MASK&scom_mask_translator)
#pragma comment(lib,"translatord")
#endif
#if(SCOM_MASK&scom_mask_log4z)
#pragma comment(lib,"log4zd")
#endif
#if(SCOM_MASK&scom_mask_taskloop)
#pragma comment(lib,"taskloopd")
#endif
#if(SCOM_MASK&scom_mask_ipcobject)
#pragma comment(lib,"sipcobjectd")
#endif
#if(SCOM_MASK&scom_mask_script_lua)
#pragma comment(lib,"lua-54d")
#pragma comment(lib,"scriptmodule-luad")
#endif
#else//_DEBUG
#if(SCOM_MASK&scom_mask_render_skia)
#pragma comment(lib,"skia")
#pragma comment(lib,"render-skia")
#endif
#if(SCOM_MASK&scom_mask_render_gdi)
#pragma comment(lib,"render-gdi")
#endif
#if(SCOM_MASK&scom_mask_render_d2d)
#pragma comment(lib,"render-d2d")
#endif
#if(SCOM_MASK&scom_mask_imgdecoder_wic)
#pragma comment(lib,"imgdecoder-wic")
#endif
#if(SCOM_MASK&scom_mask_imgdecoder_png)
#pragma comment(lib,"png")
#pragma comment(lib,"zlib")
#pragma comment(lib,"imgdecoder-png")
#endif
#if(SCOM_MASK&scom_mask_imgdecoder_stb)
#pragma comment(lib,"imgdecoder-stb")
#endif
#if(SCOM_MASK&scom_mask_imgdecoder_gdip)
#pragma comment(lib,"imgdecoder-gdip")
#endif
#if(SCOM_MASK&scom_mask_resprovider_zip)
#pragma comment(lib,"zlib")
#pragma comment(lib,"resprovider-zip")
#endif
#if(SCOM_MASK&scom_mask_resprovider_7z)
#pragma comment(lib,"7z")
#pragma comment(lib,"resprovider-7zip")
#endif
#if(SCOM_MASK&scom_mask_translator)
#pragma comment(lib,"translator")
#endif
#if(SCOM_MASK&scom_mask_log4z)
#pragma comment(lib,"log4z")
#endif
#if(SCOM_MASK&scom_mask_taskloop)
#pragma comment(lib,"taskloop")
#endif
#if(SCOM_MASK&scom_mask_ipcobject)
#pragma comment(lib,"sipcobject")
#endif
#if(SCOM_MASK&scom_mask_script_lua)
#pragma comment(lib,"lua-54")
#pragma comment(lib,"scriptmodule-lua")
#endif
#endif//_DEBUG

struct SComInfo
{
	SComID id;
	FunCreateInstance fun;
};

SOUI_COM_C BOOL Decoder_Png_SCreateInstance(IObjRef **ppObj);
SOUI_COM_C BOOL Decoder_Gdip_SCreateInstance(IObjRef **ppObj);
SOUI_COM_C BOOL Decoder_Wic_SCreateInstance(IObjRef **ppObj);
SOUI_COM_C BOOL Decoder_Stb_SCreateInstance(IObjRef **ppObj);
SOUI_COM_C BOOL Render_Gdi_SCreateInstance(IObjRef **ppObj);
SOUI_COM_C BOOL Render_Skia_SCreateInstance(IObjRef **ppObj);
SOUI_COM_C BOOL Render_D2D_SCreateInstance(IObjRef **ppObj);
SOUI_COM_C BOOL Log4z_SCreateInstance(IObjRef **ppObj);
SOUI_COM_C BOOL Resp_7zip_SCreateInstance(IObjRef ** ppObj);
SOUI_COM_C BOOL Resp_Zip_SCreateInstance(IObjRef ** ppObj);
SOUI_COM_C BOOL Script_Lua_SCreateInstance(IObjRef ** ppObj);
SOUI_COM_C BOOL TaskLoop_SCreateInstance(IObjRef **ppObj);
SOUI_COM_C BOOL Translator_SCreateInstance(IObjRef **ppObj);

struct SComInfo s_comInfo[]={
	{Decoder_Png,Translator_SCreateInstance},
	{Decoder_Gdip,Decoder_Gdip_SCreateInstance},
	{Decoder_Wic,Decoder_Wic_SCreateInstance},
	{Decoder_Stb,Decoder_Stb_SCreateInstance},
	{Render_Gdi,Render_Gdi_SCreateInstance},
	{Render_Skia,Render_Skia_SCreateInstance},
	{Render_D2D,Render_D2D_SCreateInstance},
	{Log4Z,Log4z_SCreateInstance},
	{Resprovider_7Zip,Resp_7zip_SCreateInstance},
	{Resprovider_Zip,Resp_Zip_SCreateInstance},
	{Script_Lua,Script_Lua_SCreateInstance},
	{TaskLoop,TaskLoop_SCreateInstance},
	{Translator,Translator_SCreateInstance},
};

BOOL LoadComObj(SComID id,IObjRef ** ppObj)
{
	return s_comInfo[id].fun(ppObj);
}

#endif