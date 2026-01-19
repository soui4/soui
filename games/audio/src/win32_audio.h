#ifndef WIN32_AUDIO_H
#define WIN32_AUDIO_H


#include <Windows.h>

BOOL Win32PlaySound(LPCTSTR pszSound, HMODULE hmod, DWORD fdwSound);

#endif//WIN32_AUDIO_H
