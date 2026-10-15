#ifndef __SPLAYSOUND_H__
#define __SPLAYSOUND_H__

// SPlaySound:跨平台播放 wav 文件音效(utilities4 导出)。
// 参数:pszSound=wav 文件绝对路径;hmod=保留,传 NULL;fdwSound=SND_* 标志组合
//      (SND_MEMORY/RESOURCE/ALIAS 等仅部分平台支持,文件路径播放全平台可用)。
// 返回:TRUE=已提交播放(异步,不代表声音已播完);FALSE=失败。
// 实现:Windows=audio/SPlaySound.cpp(MCI 线程化,链 winmm);其余平台=
//      audio/SPlaySound_unix.cpp(转发 swinx mmsystem 适配层 PlaySound:
//      macOS=AudioServices, Linux=alsa/aplay, Android=PlatformAPI)。
#include <windows.h>
#include <mmsystem.h>
#include <utilities-def.h>

UTILITIES_API BOOL SPlaySound(LPCTSTR pszSound, HMODULE hmod, DWORD fdwSound);

#endif//__SPLAYSOUND_H__
