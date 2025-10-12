﻿//SOUI组件配置
#ifndef _SOUI_COMMGR2_H_
#define _SOUI_COMMGR2_H_

#include <config.h>
#include <string/tstring.h>
#include <commask.h>

#ifndef SCOM_MASK
#define SCOM_MASK scom_mask_scom_all
#endif 

#if  defined(_WIN32) && !defined(__MINGW32__) && !defined(__MINGW64__) 
#define COM_IMGDECODER  _T("imgdecoder-stb")
#define COM_RENDER_GDI  _T("render-gdi")
#define COM_RENDER_SKIA _T("render-skia")
#define COM_RENDER_D2D _T("render-d2d")
#define COM_SCRIPT_LUA _T("scriptmodule-lua")
#define COM_TRANSLATOR _T("translator")
#define COM_ZIPRESPROVIDER _T("resprovider-zip")
#define COM_LOG4Z   _T("log4z")
#define COM_7ZIPRESPROVIDER _T("resprovider-7zip")
#define COM_TASKLOOP _T("taskloop")
#define COM_IPCOBJ _T("sipcobject")
#define COM_HTTPCLIENT _T("httpclient")
#else
#define COM_IMGDECODER  _T("libimgdecoder-stb")
#define COM_RENDER_GDI  _T("librender-gdi")
#define COM_RENDER_SKIA _T("librender-skia")
#define COM_SCRIPT_LUA _T("libscriptmodule-lua")
#define COM_TRANSLATOR _T("libtranslator")
#define COM_ZIPRESPROVIDER _T("libresprovider-zip")
#define COM_LOG4Z   _T("liblog4z")
#define COM_7ZIPRESPROVIDER _T("libresprovider-7zip")
#define COM_TASKLOOP _T("libtaskloop")
#define COM_RENDER_D2D _T("librender-d2d")
#define COM_IPCOBJ _T("libsipcobject")
#define COM_HTTPCLIENT _T("libhttpclient")
#endif//_WIN32

#ifdef LIB_SOUI_COM

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
#if defined(_WIN32)  && !defined(__MINGW32__)
#pragma comment(lib,"render-d2d")
#pragma comment(lib,"d2d1")
#pragma comment(lib,"dwrite")
#endif
#endif
#if(SCOM_MASK&scom_mask_imgdecoder_wic)
    #pragma comment(lib,"imgdecoder-wic")
#endif
#if(SCOM_MASK&scom_mask_imgdecoder_stb)
    #pragma comment(lib, "aupng")
    #pragma comment(lib,"imgdecoder-stb")
#endif
#if(SCOM_MASK&scom_mask_imgdecoder_gdip)
#if defined(_WIN32)  && !defined(__MINGW32__)
    #pragma comment(lib,"imgdecoder-gdip")
#endif
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
#if(SCOM_MASK&scom_mask_ipcobject)
	#pragma comment(lib,"httpclient")
#endif
#if(SCOM_MASK&scom_mask_script_lua)
	#pragma comment(lib,"lua-54")
	#pragma comment(lib,"ScriptModule-LUA")
#endif


SNSBEGIN
    namespace IMGDECODOR_WIC
    {
        BOOL SCreateInstance(IObjRef **);
    }
    namespace IMGDECODOR_STB
    {
        BOOL SCreateInstance(IObjRef **);
    }
#if defined(_WIN32)  && !defined(__MINGW32__)
    namespace IMGDECODOR_GDIP
    {
        BOOL SCreateInstance(IObjRef **);
    }
#endif//_WIN32
    namespace RENDER_GDI
    {
        BOOL SCreateInstance(IObjRef **);
    }
    namespace RENDER_SKIA
    {
        BOOL SCreateInstance(IObjRef **);
    }
#if defined(_WIN32)  && !defined(__MINGW32__)
	namespace RENDER_D2D
	{
		BOOL SCreateInstance(IObjRef **);
	}
#endif//_WIN32
    namespace SCRIPT_LUA
    {
        BOOL SCreateInstance(IObjRef **);
    }
    namespace TRANSLATOR
    {
        BOOL SCreateInstance(IObjRef **);
    }
    namespace RESPROVIDER_ZIP
    {
        BOOL SCreateInstance(IObjRef **);
    }
	namespace RESPROVIDER_7ZIP
	{
		BOOL SCreateInstance(IObjRef **);
	} 
    namespace LOG4Z
    {
        BOOL SCreateInstance(IObjRef **);
    }
	namespace TASKLOOP {
		BOOL SCreateInstance(IObjRef **);
	}
	namespace IPC {
		BOOL SCreateInstance(IObjRef **);
	}
	namespace HttpClient {
		BOOL SCreateInstance(IObjRef **);
	}
SNSEND

SNSBEGIN
class SComMgr2
{
public:
    SComMgr2(LPCTSTR pszImgDecoder = NULL)
    {
        if(pszImgDecoder) m_strImgDecoder = pszImgDecoder;
        else m_strImgDecoder = COM_IMGDECODER;
    }

	void SetDllPath(const SStringT & strDllPath){}

    BOOL CreateImgDecoder(IObjRef ** ppObj,LPCTSTR pszImgDecoder=NULL)
    {
        SStringT strImgDecoder = pszImgDecoder?SStringT(pszImgDecoder):m_strImgDecoder;
        strImgDecoder.MakeLower();
#if(SCOM_MASK&scom_mask_imgdecoder_wic)
        if(strImgDecoder == _T("imgdecoder-wic"))
            return IMGDECODOR_WIC::SCreateInstance(ppObj);
#endif
#if(SCOM_MASK&scom_mask_imgdecoder_stb)
        if(strImgDecoder == _T("imgdecoder-stb"))
            return IMGDECODOR_STB::SCreateInstance(ppObj);
#endif
#if(SCOM_MASK&scom_mask_imgdecoder_gdip)
        if(strImgDecoder == _T("imgdecoder-gdip"))
            return IMGDECODOR_GDIP::SCreateInstance(ppObj);
#endif
         SASSERT(0);
         return FALSE;
    }

#if(SCOM_MASK&scom_mask_render_gdi)
    BOOL CreateRender_GDI(IObjRef **ppObj)
    {
        return RENDER_GDI::SCreateInstance(ppObj);
    }
#endif

#if(SCOM_MASK&scom_mask_render_skia)
    BOOL CreateRender_Skia(IObjRef **ppObj)
    {
        return RENDER_SKIA::SCreateInstance(ppObj);
    }
#endif

#if(SCOM_MASK&scom_mask_render_d2d)
	BOOL CreateRender_D2D(IObjRef **ppObj)
	{
		return RENDER_D2D::SCreateInstance(ppObj);
	}
#endif

#if(SCOM_MASK&scom_mask_script_lua)
    BOOL CreateScrpit_Lua(IObjRef **ppObj)
    {
        return SCRIPT_LUA::SCreateInstance(ppObj);
    }
#endif

#if(SCOM_MASK&scom_mask_translator)
    BOOL CreateTranslator(IObjRef **ppObj)
    {
        return TRANSLATOR::SCreateInstance(ppObj);
    }
#endif

#if(SCOM_MASK&scom_mask_resprovider_zip)
    BOOL CreateResProvider_ZIP(IObjRef **ppObj)
    {
        return RESPROVIDER_ZIP::SCreateInstance(ppObj);
    }
#endif


#if(SCOM_MASK&scom_mask_resprovider_7z)
	BOOL CreateResProvider_7ZIP(IObjRef **ppObj)
	{
		return RESPROVIDER_7ZIP::SCreateInstance(ppObj);
	}
#endif
    
#if(SCOM_MASK&scom_mask_log4z)
    BOOL CreateLog4z(IObjRef **ppObj)
    {
        return LOG4Z::SCreateInstance(ppObj);
    }
#endif

#if(SCOM_MASK&scom_mask_taskloop)
	BOOL CreateTaskLoop(IObjRef **ppObj)
	{
		return TASKLOOP::SCreateInstance(ppObj);
	}
#endif
#if(SCOM_MASK&scom_mask_ipcobject)
	BOOL CreateIpcObject(IObjRef **ppObj)
	{
		return IPC::SCreateInstance(ppObj);
	}
#endif
#if(SCOM_MASK&scom_mask_httpclient)
	BOOL CreateHttpClient(IObjRef **ppObj)
	{
		return HttpClient::SCreateInstance(ppObj);
	}
#endif

	HMODULE GetRenderModule(){
		return NULL;
	}

	SStringT m_strImgDecoder;
};

SNSEND
#else
	
#include <com-loader.hpp>
#ifdef _WIN32
#define SPATH_SLASH _T("\\")
#else
#define SPATH_SLASH _T("/")
#endif//SPATH_SLASH

SNSBEGIN

class SComMgr2
{
public:
    SComMgr2(LPCTSTR pszImgDecoder = NULL)
    {
        if(pszImgDecoder) m_strImgDecoder = pszImgDecoder;
        else m_strImgDecoder = COM_IMGDECODER;
    }

	void SetDllPath(const SStringT & strDllPath)
	{
		m_strDllPath = strDllPath;
		if(!m_strDllPath.IsEmpty())
		{
			if(m_strDllPath.Right(1)!=SPATH_SLASH)
			{
				m_strDllPath+=SPATH_SLASH;
			}
		}
	}

    BOOL CreateImgDecoder(IObjRef ** ppObj,LPCTSTR pszImgDecoder=NULL)
    {
        SStringT strImgDecoder = pszImgDecoder?SStringT(pszImgDecoder):m_strImgDecoder;
        strImgDecoder.MakeLower();
        return imgDecLoader.CreateInstance(m_strDllPath + strImgDecoder, ppObj);
    }

    BOOL CreateRender_GDI(IObjRef **ppObj)
    {
        return renderLoader.CreateInstance(m_strDllPath+COM_RENDER_GDI,ppObj);
    }

    BOOL CreateRender_Skia(IObjRef **ppObj)
    {
        return renderLoader.CreateInstance(m_strDllPath+COM_RENDER_SKIA,ppObj);
    }

	BOOL CreateRender_D2D(IObjRef **ppObj)
	{
		return renderLoader.CreateInstance(m_strDllPath+COM_RENDER_D2D,ppObj);
	}
	
	BOOL CreateScrpit_Lua(IObjRef **ppObj)
    {
        return scriptLoader.CreateInstance(m_strDllPath+COM_SCRIPT_LUA,ppObj);
    }

    BOOL CreateTranslator(IObjRef **ppObj)
    {
        return transLoader.CreateInstance(m_strDllPath+COM_TRANSLATOR,ppObj);
    }
    BOOL CreateResProvider_ZIP(IObjRef **ppObj)
    {
        return zipResLoader.CreateInstance(m_strDllPath+COM_ZIPRESPROVIDER,ppObj);
    }

	BOOL CreateResProvider_7ZIP(IObjRef **ppObj)
	{
		return zip7ResLoader.CreateInstance(m_strDllPath+COM_7ZIPRESPROVIDER, ppObj);
	}
	
    BOOL CreateLog4z(IObjRef **ppObj)
    {
        return log4zLoader.CreateInstance(m_strDllPath+COM_LOG4Z,ppObj);
    }

	BOOL CreateTaskLoop(IObjRef **ppObj)
	{
		return taskLoopLoader.CreateInstance(m_strDllPath + COM_TASKLOOP, ppObj);
	}

	BOOL CreateIpcObject(IObjRef **ppObj)
	{
		return ipcLoader.CreateInstance(m_strDllPath + COM_IPCOBJ, ppObj);
	}

	BOOL CreateHttpClient(IObjRef **ppObj)
	{
		return httpClientLoader.CreateInstance(m_strDllPath + COM_HTTPCLIENT, ppObj);
	}

	HMODULE GetRenderModule(){
		return renderLoader.GetModule();
	}
protected:
    //SComLoader实现从DLL的指定函数创建符号SOUI要求的类COM组件。
    SComLoader imgDecLoader;
    SComLoader renderLoader;
    SComLoader transLoader;
    SComLoader scriptLoader;
    SComLoader zipResLoader;
    SComLoader log4zLoader;
    SComLoader zip7ResLoader;
	SComLoader taskLoopLoader;
	SComLoader ipcLoader;
    SComLoader httpClientLoader;

    SStringT m_strImgDecoder;
	SStringT m_strDllPath;
};

SNSEND
#endif//LIB_SOUI_COM

#endif//_SOUI_COMMGR2_H_