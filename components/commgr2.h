//SOUI组件配置
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
#define COM_NETWORK _T("network")
#define COM_WS _T("ws")
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
#define COM_NETWORK _T("libnetwork")
#define COM_WS _T("libws")
#endif//_WIN32

#ifdef LIB_SOUI_COM

SNSBEGIN
    namespace IMGDECODOR_WIC
    {
        BOOL SCreateInstance(IObjRef **);
    }
    namespace IMGDECODOR_STB
    {
        BOOL SCreateInstance(IObjRef **);
    }
#if defined(_WIN32)
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

    namespace NETWORK {
		BOOL SCreateInstance(IObjRef **);
	}
    namespace WS {
		BOOL SCreateInstance(IObjRef **);
	}
    
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
#if(SCOM_MASK&scom_mask_imgdecoder_wic) && defined(_WIN32)
        if(strImgDecoder == _T("imgdecoder-wic") || strImgDecoder == _T("libimgdecoder-wic"))
            return IMGDECODOR_WIC::SCreateInstance(ppObj);
#endif
#if(SCOM_MASK&scom_mask_imgdecoder_stb)
        if(strImgDecoder == _T("imgdecoder-stb") || strImgDecoder == _T("libimgdecoder-stb"))
            return IMGDECODOR_STB::SCreateInstance(ppObj);
#endif
#if(SCOM_MASK&scom_mask_imgdecoder_gdip) && defined(_WIN32)
        if(strImgDecoder == _T("imgdecoder-gdip") || strImgDecoder == _T("libimgdecoder-gdip"))
            return IMGDECODOR_GDIP::SCreateInstance(ppObj);
#endif
         SASSERT(0);
         return FALSE;
    }

    BOOL CreateRender_GDI(IObjRef **ppObj)
    {
        #if(SCOM_MASK&scom_mask_render_gdi)
        return RENDER_GDI::SCreateInstance(ppObj);
        #else
        return FALSE;
        #endif
    }

    BOOL CreateRender_Skia(IObjRef **ppObj)
    {
        #if(SCOM_MASK&scom_mask_render_skia)
        return RENDER_SKIA::SCreateInstance(ppObj);
        #else
        return FALSE;
        #endif
    }

	BOOL CreateRender_D2D(IObjRef **ppObj)
	{
        #if(SCOM_MASK&scom_mask_render_d2d) && defined(_WIN32)  && !defined(__MINGW32__)
		return RENDER_D2D::SCreateInstance(ppObj);
        #else
        return FALSE;
        #endif
	}

    BOOL CreateScrpit_Lua(IObjRef **ppObj)
    {
        #if(SCOM_MASK&scom_mask_script_lua) && defined(DLL_SOUI_COM)
        return SCRIPT_LUA::SCreateInstance(ppObj);
        #else
        return FALSE;
        #endif
    }

    BOOL CreateTranslator(IObjRef **ppObj)
    {
        #if(SCOM_MASK&scom_mask_translator)
        return TRANSLATOR::SCreateInstance(ppObj);
        #else
        return FALSE;
        #endif
    }

    BOOL CreateResProvider_ZIP(IObjRef **ppObj)
    {
        #if(SCOM_MASK&scom_mask_resprovider_zip)
        return RESPROVIDER_ZIP::SCreateInstance(ppObj);
        #else
        return FALSE;
        #endif
    }


	BOOL CreateResProvider_7ZIP(IObjRef **ppObj)
	{
        #if(SCOM_MASK&scom_mask_resprovider_7z)
		return RESPROVIDER_7ZIP::SCreateInstance(ppObj);
        #else
        return FALSE;
        #endif
	}
    
    BOOL CreateLog4z(IObjRef **ppObj)
    {
        #if(SCOM_MASK&scom_mask_log4z)
        return LOG4Z::SCreateInstance(ppObj);
        #else
        return FALSE;
        #endif
    }

	BOOL CreateTaskLoop(IObjRef **ppObj)
	{
        #if(SCOM_MASK&scom_mask_taskloop)
		return TASKLOOP::SCreateInstance(ppObj);
        #else
        return FALSE;
        #endif
	}

	BOOL CreateIpcObject(IObjRef **ppObj)
	{
        #if(SCOM_MASK&scom_mask_ipcobject)
		return IPC::SCreateInstance(ppObj);
        #else
        return FALSE;
        #endif
	}

	BOOL CreateHttpClient(IObjRef **ppObj)
	{
        #if(SCOM_MASK&scom_mask_httpclient)
		return HttpClient::SCreateInstance(ppObj);
        #else
        return FALSE;
        #endif
	}

    BOOL CreateNetwork(IObjRef **ppObj)
	{
        #if(SCOM_MASK&scom_mask_network)
		return NETWORK::SCreateInstance(ppObj);
        #else
        return FALSE;
        #endif
	}

    BOOL CreateWS(IObjRef **ppObj)
	{
        #if(SCOM_MASK&scom_mask_ws)
		return WS::SCreateInstance(ppObj);
        #else
        return FALSE;
        #endif
	}

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

    BOOL CreateNetwork(IObjRef **ppObj)
	{
		return networkLoader.CreateInstance(m_strDllPath + COM_NETWORK, ppObj);
	}

    BOOL CreateWS(IObjRef **ppObj)
	{
		return wsLoader.CreateInstance(m_strDllPath + COM_WS, ppObj);
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
    SComLoader networkLoader;
    SComLoader wsLoader;

    SStringT m_strImgDecoder;
	SStringT m_strDllPath;
};

SNSEND
#endif//LIB_SOUI_COM

#endif//_SOUI_COMMGR2_H_