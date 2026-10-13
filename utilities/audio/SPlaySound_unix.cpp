// SPlaySound 的非 Windows 实现:转发到 swinx mmsystem 适配层的 PlaySound
// (macOS=AudioServices, Linux=alsa/aplay, Android=PlatformAPI 桥接)。
// swinx 的 PlaySound 仅支持文件路径(SND_FILENAME),其余 flags 按各平台语义处理。
#include <windows.h>
#include <mmsystem.h>
#include <SPlaySound.h>

BOOL WINAPI SPlaySound(LPCTSTR pszSound, HMODULE hmod, DWORD fdwSound)
{
    return PlaySound(pszSound, hmod, fdwSound);
}
