#include "souistd.h"
#include "helper/SAppDir.h"

SNSBEGIN
SAppDir::SAppDir(HINSTANCE hInst)
{
    TCHAR apppath[MAX_PATH] = { 0 };
    if (GetModuleFileName(hInst, apppath, MAX_PATH))
    {
        TCHAR appfullpath[MAX_PATH];
        GetFullPathName(apppath, MAX_PATH, appfullpath, NULL);
        LPTSTR lpInsertPos = _tcsrchr(appfullpath, PATH_SLASH);
        if (lpInsertPos)
        {
            *lpInsertPos = 0;
        }
        m_strAppPath = appfullpath;
    }
}

const SStringT &SAppDir::AppDir()
{
    return m_strAppPath;
}
SNSEND