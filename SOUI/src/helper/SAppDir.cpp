#include "souistd.h"
#include "helper/SAppDir.h"

SNSBEGIN
SAppDir::SAppDir(HINSTANCE hInst)
{
    TCHAR szCurrentDir[MAX_PATH] = { 0 };
    if (GetModuleFileName(hInst, szCurrentDir, MAX_PATH))
    {
#ifdef _WIN32
        LPTSTR lpInsertPos = _tcsrchr(szCurrentDir, _T('\\'));
#else
        LPTSTR lpInsertPos = _tcsrchr(szCurrentDir, _T('/'));
#endif //
        if (lpInsertPos)
        {
            *lpInsertPos = 0;
        }
        m_strAppPath = szCurrentDir;
    }
}

const SStringT &SAppDir::AppDir()
{
    return m_strAppPath;
}
SNSEND