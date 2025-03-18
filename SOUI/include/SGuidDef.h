#include <soui_exp.h>

#include <guiddef.h>

#ifdef DEFINE_SGUID
#undef DEFINE_SGUID
#endif

#ifdef DEFINE_SGUID2
#undef DEFINE_SGUID2
#endif

#ifdef INITGUID
#define DEFINE_SGUID(name, l, w1, w2, b1, b2, b3, b4, b5, b6, b7, b8) \
        EXTERN_C const GUID SOUI_EXP name \
                = { l, w1, w2, { b1, b2,  b3,  b4,  b5,  b6,  b7,  b8 } }
#define DEFINE_SGUID2(name, l, w1, w2, b1, b2, b3, b4, b5, b6, b7, b8) \
        EXTERN_C const GUID  name \
                = { l, w1, w2, { b1, b2,  b3,  b4,  b5,  b6,  b7,  b8 } }

#else
#define DEFINE_SGUID(name, l, w1, w2, b1, b2, b3, b4, b5, b6, b7, b8) \
    EXTERN_C const GUID SOUI_EXP name
#define DEFINE_SGUID2(name, l, w1, w2, b1, b2, b3, b4, b5, b6, b7, b8) \
        EXTERN_C const GUID  name 
#endif // INITGUID
