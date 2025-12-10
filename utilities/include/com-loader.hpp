#ifndef _COM_LOADER_H_
#define _COM_LOADER_H_

#include <windows.h>
#include <tchar.h>
#include <interface/obj-ref-i.h>
#include <stdio.h>

SNSBEGIN
//加载组件辅助类
//组件需要提供SCreateInstance接口。接口定义必须是funSCreateInstance
class SComLoader
{
    typedef BOOL (*funSCreateInstance)(IObjRef **);
public:
    SComLoader()
    :m_hMod(0)
    ,m_funCreateInst(NULL)
    {
        m_szDllPath[0]=0;
    }
    ~SComLoader()
    {
        if(m_hMod) FreeLibrary(m_hMod);
    }

    BOOL CreateInstance(LPCTSTR pszDllPath,IObjRef **ppObj,LPCSTR pszFnName = "SCreateInstance")
    {
        TCHAR szDllPath[MAX_PATH];
        _tcscpy(szDllPath,pszDllPath);
        if(!m_funCreateInst)
        {
            #ifdef _WIN32
            _tcscat(szDllPath,_T(".dll"));
            #elif defined(__APPLE__)
            _tcscat(szDllPath,_T(".dylib"));
            #else
            _tcscat(szDllPath,_T(".so"));
            #endif
            m_hMod=LoadLibrary(szDllPath);
            if (!m_hMod) {
#ifndef _WIN32
                const char * err = dlerror();
                printf("load so failed, err=%s\n", err);
#else
                int err = GetLastError();
                printf("load so failed, err=%d\n",err);
#endif
#if defined(_WIN32) && (defined(__MINGW32__) || defined(__MINGW64__))
                if(_tcsnicmp(pszDllPath,_T("lib"),3) != 0)
                {//try to load libxxx.dll
                    _stprintf(szDllPath,_T("lib%s.dll"),pszDllPath);
                    m_hMod=LoadLibrary(szDllPath);
                }
#endif
                if(!m_hMod)
                    return FALSE;
            }
            m_funCreateInst=(funSCreateInstance)GetProcAddress(m_hMod,pszFnName);
            if(!m_funCreateInst)
            {
                FreeLibrary(m_hMod);
                return FALSE;
            }
            _tcscpy(m_szDllPath,szDllPath);
        }
        return m_funCreateInst(ppObj);
    }

	HMODULE GetModule(){
		return m_hMod;
	}
protected:
    HMODULE m_hMod;
    funSCreateInstance m_funCreateInst;
    TCHAR   m_szDllPath[MAX_PATH];
};

SNSEND

#endif//_COM_LOADER_H_
