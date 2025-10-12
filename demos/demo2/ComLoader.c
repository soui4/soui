#include <windows.h>
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

#ifdef _WIN32
#if defined(__MINGW32__) || defined(__MINGW64__)
struct SComInfo s_comInfo[]={
	{Decoder_Gdip,_T("libimgdecoder-gdip.dll"),NULL},
	{Decoder_Wic,_T("libimgdecoder-wic.dll"),NULL},
	{Decoder_Stb,_T("libimgdecoder-stb.dll"),NULL},
	{Render_GDI,_T("librender-gdi.dll"),NULL},
	{Render_SKIA,_T("librender-skia.dll"),NULL},
	{Render_D2D,_T("librender-d2d.dll"),NULL},
	{Log4Z,_T("liblog4z.dll"),NULL},
	{Resprovider_7Zip,_T("libresprovider-7zip.dll"),NULL},
	{Resprovider_Zip,_T("libresprovider-zip.dll"),NULL},
	{Script_Lua,_T("libscriptmodule-lua.dll"),NULL},
	{TaskLoop,_T("libtaskloop.dll"),NULL},
	{Translator,_T("libtranslator.dll"),NULL},
};
#else
struct SComInfo s_comInfo[]={
	{Decoder_Gdip,_T("imgdecoder-gdip.dll"),NULL},
	{Decoder_Wic,_T("imgdecoder-wic.dll"),NULL},
	{Decoder_Stb,_T("imgdecoder-stb.dll"),NULL},
	{Render_GDI,_T("render-gdi.dll"),NULL},
	{Render_SKIA,_T("render-skia.dll"),NULL},
	{Render_D2D,_T("render-d2d.dll"),NULL},
	{Log4Z,_T("log4z.dll"),NULL},
	{Resprovider_7Zip,_T("resprovider-7zip.dll"),NULL},
	{Resprovider_Zip,_T("resprovider-zip.dll"),NULL},
	{Script_Lua,_T("scriptmodule-lua.dll"),NULL},
	{TaskLoop,_T("taskloop.dll"),NULL},
	{Translator,_T("translator.dll"),NULL},
};
#endif
#elif defined(__APPLE__)
struct SComInfo s_comInfo[] = {
	{Decoder_Gdip,_T("libimgdecoder-gdip.dylib"),NULL},
	{Decoder_Wic,_T("libimgdecoder-wic.dylib"),NULL},
	{Decoder_Stb,_T("libimgdecoder-stb.dylib"),NULL},
	{Render_GDI,_T("librender-gdi.dylib"),NULL},
	{Render_SKIA,_T("librender-skia.dylib"),NULL},
	{Render_D2D,_T("librender-d2d.dylib"),NULL},
	{Log4Z,_T("liblog4z.dylib"),NULL},
	{Resprovider_7Zip,_T("libresprovider-7zip.dylib"),NULL},
	{Resprovider_Zip,_T("libresprovider-zip.dylib"),NULL},
	{Script_Lua,_T("libscriptmodule-lua.dylib"),NULL},
	{TaskLoop,_T("libtaskloop.dylib"),NULL},
	{Translator,_T("libtranslator.dylib"),NULL},
};
#else
struct SComInfo s_comInfo[] = {
	{Decoder_Gdip,_T("libimgdecoder-gdip.so"),NULL},
	{Decoder_Wic,_T("libimgdecoder-wic.so"),NULL},
	{Decoder_Stb,_T("libimgdecoder-stb.so"),NULL},
	{Render_GDI,_T("librender-gdi.so"),NULL},
	{Render_SKIA,_T("librender-skia.so"),NULL},
	{Render_D2D,_T("librender-d2d.so"),NULL},
	{Log4Z,_T("liblog4z.so"),NULL},
	{Resprovider_7Zip,_T("libresprovider-7zip.so"),NULL},
	{Resprovider_Zip,_T("libresprovider-zip.so"),NULL},
	{Script_Lua,_T("libscriptmodule-lua.so"),NULL},
	{TaskLoop,_T("libtaskloop.so"),NULL},
	{Translator,_T("libtranslator.so"),NULL},
};
#endif//_WIN32
	BOOL LoadComObj(SComID id,IObjRef ** ppObj)
	{
		FunCreateInstance fun=NULL;
		HMODULE hDll=s_comInfo[id].hMod;
		LPCTSTR pszDll = s_comInfo[id].pszDll;
		if(!hDll)
		{
			hDll = LoadLibrary(pszDll);
			if (!hDll) {
#ifndef _WIN32
				const char* err = dlerror();
				printf("load so failed, err=%s\n", err);
#endif
				return FALSE;
			}
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
#if(SCOM_MASK&scom_mask_imgdecoder_stb)
#pragma comment(lib, "aupng")
#pragma comment(lib, "imgdecoder-stb")
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


struct SComInfo
{
	SComID id;
	FunCreateInstance fun;
};

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
	{Decoder_Gdip,Decoder_Gdip_SCreateInstance},
	{Decoder_Wic,Decoder_Wic_SCreateInstance},
	{Decoder_Stb,Decoder_Stb_SCreateInstance},
	{Render_GDI,Render_Gdi_SCreateInstance},
	{Render_SKIA,Render_Skia_SCreateInstance},
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