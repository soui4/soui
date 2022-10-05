#ifdef DLL_CORE
#ifdef SOUI_EXPORTS
#define SOUI_EXP __declspec(dllexport)
#else
#define SOUI_EXP __declspec(dllimport)
#endif // SOUI_EXPORTS
#else
#define SOUI_EXP
#endif
