#include <soui_exp.h>

#include <guiddef.h>

#ifdef DEFINE_SGUID
#undef DEFINE_SGUID
#endif

#ifdef INITGUID
#define DEFINE_SGUID(name, l, w1, w2, b1, b2, b3, b4, b5, b6, b7, b8) \
        EXTERN_C const GUID SOUI_EXP name \
                = { l, w1, w2, { b1, b2,  b3,  b4,  b5,  b6,  b7,  b8 } }
#else
#define DEFINE_SGUID(name, l, w1, w2, b1, b2, b3, b4, b5, b6, b7, b8) \
    EXTERN_C const GUID SOUI_EXP name
#endif // INITGUID
