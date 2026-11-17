// SPlaySound 的非 Windows 实现:转发到 swinx mmsystem 适配层的 PlaySound
// (macOS=AudioServices, Linux=alsa/aplay, Android=PlatformAPI 桥接)。
// swinx 的 PlaySound 仅支持文件路径(SND_FILENAME),其余 flags 按各平台语义处理。
#include <windows.h>
#include <mmsystem.h>
#include <SPlaySound.h>

#include <string>

// SPlaySound 对外是 LPCTSTR(UNICODE 下 = const wchar_t*)，而 swinx 的
// PlaySound 只收 LPCSTR(UTF-8/ANSI)。这里把宽字符路径按 UTF-8 转成窄字符再转发，
// 与 Windows 分支 wav 文件路径的行为保持一致。
BOOL WINAPI SPlaySound(LPCTSTR pszSound, HMODULE hmod, DWORD fdwSound)
{
    if (!pszSound)
        return PlaySound((LPCSTR)NULL, hmod, fdwSound);

#ifdef _UNICODE
    // 手写 UTF-32(wchar_t, WCHAR_SIZE=4) -> UTF-8，不依赖进程 locale。
    std::string narrow;
    for (const wchar_t *p = pszSound; *p; ++p)
    {
        unsigned int cp = (unsigned int)*p;
        if (cp < 0x80)
        {
            narrow.push_back((char)cp);
        }
        else if (cp < 0x800)
        {
            narrow.push_back((char)(0xC0 | (cp >> 6)));
            narrow.push_back((char)(0x80 | (cp & 0x3F)));
        }
        else if (cp < 0x10000)
        {
            narrow.push_back((char)(0xE0 | (cp >> 12)));
            narrow.push_back((char)(0x80 | ((cp >> 6) & 0x3F)));
            narrow.push_back((char)(0x80 | (cp & 0x3F)));
        }
        else
        {
            narrow.push_back((char)(0xF0 | (cp >> 18)));
            narrow.push_back((char)(0x80 | ((cp >> 12) & 0x3F)));
            narrow.push_back((char)(0x80 | ((cp >> 6) & 0x3F)));
            narrow.push_back((char)(0x80 | (cp & 0x3F)));
        }
    }
    return PlaySound(narrow.c_str(), hmod, fdwSound);
#else
    return PlaySound(pszSound, hmod, fdwSound);
#endif
}
