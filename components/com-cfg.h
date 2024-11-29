//SOUI组件配置
#ifndef _SOUI_COM_CFG_H_
#define _SOUI_COM_CFG_H_

#include <config.h>
#include <string/tstring.h>

#define COM_IMGDECODER  _T("imgdecoder-gdip")

#define COM_RENDER_GDI  _T("render-gdi")
#define COM_RENDER_SKIA _T("render-skia")
#define COM_RENDER_D2D  _T("render-d2d")
#define COM_SCRIPT_LUA _T("scriptmodule-lua")
#define COM_TRANSLATOR _T("translator")
#define COM_ZIPRESPROVIDER _T("resprovider-zip")
#define COM_LOG4Z   _T("log4z")
#define COM_7ZIPRESPROVIDER _T("resprovider-7zip")
#define COM_TASKLOOP _T("taskloop")


#ifdef LIB_SOUI_COM
#pragma message("LIB_SOUI_COM")

#pragma comment(lib,"Usp10")
#pragma comment(lib,"opengl32")

#pragma comment(lib,"skia")
#pragma comment(lib,"zlib")
#pragma comment(lib,"png")
#pragma comment(lib,"imgdecoder-wic")
#pragma comment(lib,"imgdecoder-stb")
#pragma comment(lib,"imgdecoder-png")
#pragma comment(lib,"imgdecoder-gdip")
#pragma comment(lib,"render-gdi")
#pragma comment(lib,"render-skia")
#pragma comment(lib,"render-d2d")
#pragma comment(lib,"translator")
#pragma comment(lib,"resprovider-zip")
#pragma comment(lib,"7z")
#pragma comment(lib,"resprovider-7zip")
#pragma comment(lib,"log4z")
#pragma comment(lib,"taskloop")

namespace SOUI
{
	namespace IMGDECODOR_WIC
	{
		BOOL SCreateInstance(IObjRef **);
	}
	namespace IMGDECODOR_STB
	{
		BOOL SCreateInstance(IObjRef **);
	}
	namespace IMGDECODOR_PNG
	{
		BOOL SCreateInstance(IObjRef **);
	}
	namespace IMGDECODOR_GDIP
	{
		BOOL SCreateInstance(IObjRef **);
	}

	namespace RENDER_GDI
	{
		BOOL SCreateInstance(IObjRef **);
	}
	namespace RENDER_SKIA
	{
		BOOL SCreateInstance(IObjRef **);
	}
	namespace RENDER_D2D
	{
		BOOL SCreateInstance(IObjRef **);
	}
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

	class SComMgr
	{
	public:
		SComMgr(LPCTSTR pszImgDecoder = NULL)
		{
			if (pszImgDecoder) m_strImgDecoder = pszImgDecoder;
			else m_strImgDecoder = COM_IMGDECODER;
		}

		void SetDllPath(const SStringT & strDllPath) {}

		BOOL CreateImgDecoder(IObjRef ** ppObj)
		{
			if (m_strImgDecoder == _T("imgdecoder-wic"))
				return IMGDECODOR_WIC::SCreateInstance(ppObj);
			else if (m_strImgDecoder == _T("imgdecoder-stb"))
				return IMGDECODOR_STB::SCreateInstance(ppObj);
			else if (m_strImgDecoder == _T("imgdecoder-png"))
				return IMGDECODOR_PNG::SCreateInstance(ppObj);
			else if (m_strImgDecoder == _T("imgdecoder-gdip"))
				return IMGDECODOR_GDIP::SCreateInstance(ppObj);
			else
			{
				SASSERT(0);
				return FALSE;
			}
		}

		BOOL CreateRender_GDI(IObjRef **ppObj)
		{
			return RENDER_GDI::SCreateInstance(ppObj);
		}

		BOOL CreateRender_Skia(IObjRef **ppObj)
		{
			return RENDER_SKIA::SCreateInstance(ppObj);
		}
		BOOL CreateRender_D2D(IObjRef **ppObj)
		{
			return RENDER_D2D::SCreateInstance(ppObj);
		}

		BOOL CreateScrpit_Lua(IObjRef **ppObj)
		{
			return SCRIPT_LUA::SCreateInstance(ppObj);
		}

		BOOL CreateTranslator(IObjRef **ppObj)
		{
			return TRANSLATOR::SCreateInstance(ppObj);
		}
		BOOL CreateResProvider_ZIP(IObjRef **ppObj)
		{
			return RESPROVIDER_ZIP::SCreateInstance(ppObj);
		}


		BOOL CreateResProvider_7ZIP(IObjRef **ppObj)
		{
			return RESPROVIDER_7ZIP::SCreateInstance(ppObj);
		}

		BOOL CreateLog4z(IObjRef **ppObj)
		{
			return LOG4Z::SCreateInstance(ppObj);
		}

		BOOL CreateTaskLoop(IObjRef **ppObj)
		{
			return TASKLOOP::SCreateInstance(ppObj);
		}

		HMODULE GetRenderModule() {
			return NULL;
		}

		SStringT    m_strImgDecoder;
	};

}//end of soui

#else

#include <com-loader.hpp>

namespace SOUI {
	class SComMgr
	{
	public:
		SComMgr(LPCTSTR pszImgDecoder = NULL)
		{
			if (pszImgDecoder) m_strImgDecoder = pszImgDecoder;
			else m_strImgDecoder = COM_IMGDECODER;
		}

		void SetDllPath(const SStringT & strDllPath)
		{
			m_strDllPath = strDllPath;
			if (!m_strDllPath.IsEmpty())
			{
				if (m_strDllPath.Right(1) != _T("\\"))
				{
					m_strDllPath += _T("\\");
				}
			}
		}

		BOOL CreateImgDecoder(IObjRef ** ppObj)
		{
			return imgDecLoader.CreateInstance(m_strDllPath + m_strImgDecoder, ppObj);
		}

		BOOL CreateRender_GDI(IObjRef **ppObj)
		{
			return renderLoader.CreateInstance(m_strDllPath + COM_RENDER_GDI, ppObj);
		}

		BOOL CreateRender_Skia(IObjRef **ppObj)
		{
			return renderLoader.CreateInstance(m_strDllPath + COM_RENDER_SKIA, ppObj);
		}
		BOOL CreateRender_D2D(IObjRef **ppObj)
		{
			return renderLoader.CreateInstance(m_strDllPath + COM_RENDER_D2D, ppObj);
		}

		BOOL CreateScrpit_Lua(IObjRef **ppObj)
		{
			return scriptLoader.CreateInstance(m_strDllPath + COM_SCRIPT_LUA, ppObj);
		}

		BOOL CreateTranslator(IObjRef **ppObj)
		{
			return transLoader.CreateInstance(m_strDllPath + COM_TRANSLATOR, ppObj);
		}
		BOOL CreateResProvider_ZIP(IObjRef **ppObj)
		{
			return zipResLoader.CreateInstance(m_strDllPath + COM_ZIPRESPROVIDER, ppObj);
		}

		BOOL CreateResProvider_7ZIP(IObjRef **ppObj)
		{
			return zip7ResLoader.CreateInstance(m_strDllPath + COM_7ZIPRESPROVIDER, ppObj);
		}

		BOOL CreateLog4z(IObjRef **ppObj)
		{
			return log4zLoader.CreateInstance(m_strDllPath + COM_LOG4Z, ppObj);
		}

		BOOL CreateTaskLoop(IObjRef **ppObj)
		{
			return taskLoopLoader.CreateInstance(m_strDllPath + COM_TASKLOOP, ppObj);
		}

		HMODULE GetRenderModule() {
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

		SStringT m_strImgDecoder;
		SStringT m_strDllPath;
	};

}//end of soui
#endif

#endif//_SOUI_COM_CFG_H_