#ifndef __PLATFORM_EXP_H_
#define __PLATFORM_EXP_H_

#if defined(DLL_CORE) && defined(_WIN32)
#ifdef PLATFORM_EXPORTS
#define PLATFORM_API __declspec(dllexport)
#else
#define PLATFORM_API __declspec(dllimport)
#endif
#else
#define PLATFORM_API
#endif


#endif//__PLATFORM_EXP_H_