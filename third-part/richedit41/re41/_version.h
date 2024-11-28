#define RICHEDIT_VER "5.41.21.2500"
#define RICHEDIT_VERMAJ 41
#define RICHEDIT_VERMIN 21
#define RICHEDIT_VERBUILD 2500
#ifdef DEBUG
#define RICHEDIT_BUILD RICHEDIT_VER ## " (Debug)"
#else
#define RICHEDIT_BUILD RICHEDIT_VER
#endif

#ifdef _WIN32
#define RICHEDIT_HEADER "Msftedit " ## RICHEDIT_VER
#define RTF_GENINFO "{\\*\\generator " ## RICHEDIT_VER ## ";}"
#else
#define RICHEDIT_HEADER "Msftedit 5.41.21.2500"
#define RTF_GENINFO "{\\*\\generator 5.41.21.2500;}"
#endif//_WIN32