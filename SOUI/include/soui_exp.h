#if defined(DLL_CORE) && defined(_WIN32)
#ifdef SOUI_EXPORTS
#define SOUI_EXP __declspec(dllexport)
#else
#define SOUI_EXP __declspec(dllimport)
#endif // SOUI_EXPORTS
#else
#define SOUI_EXP
#endif
