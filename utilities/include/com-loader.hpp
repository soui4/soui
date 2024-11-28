#ifndef _COM_LOADER_H_
#define _COM_LOADER_H_

#include <windows.h>
#include <interface/obj-ref-i.h>
#include <string/tstring.h>

namespace SOUI {
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
        SStringT strPath(pszDllPath);
        if(!m_funCreateInst)
        {
            #ifdef _WIN32
            strPath+=_T(".dll");
            #else
            strPath+=_T(".so");
            #endif
            m_hMod=LoadLibrary(strPath);
            if (!m_hMod) {
#ifndef _WIN32
                const char * err = dlerror();
                printf("load so failed, err=%s\n", err);
#endif
                return FALSE;
            }
            m_funCreateInst=(funSCreateInstance)GetProcAddress(m_hMod,pszFnName);
            if(!m_funCreateInst)
            {
                FreeLibrary(m_hMod);
                return FALSE;
            }
            _tcscpy(m_szDllPath,strPath);
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

}//end of soui

#endif//_COM_LOADER_H_
