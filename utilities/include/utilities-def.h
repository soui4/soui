#include <config.h>

#include <sdef.h>
#include <stddef.h>
#include <objbase.h>

#if defined(DLL_CORE) && defined(_WIN32)
#ifdef UTILITIES_EXPORTS
#define UTILITIES_API __declspec(dllexport)
#else
#define UTILITIES_API __declspec(dllimport)
#endif
#else
#define UTILITIES_API
#endif