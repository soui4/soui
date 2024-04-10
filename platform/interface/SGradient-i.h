#ifndef _GRADIENT_I_H_
#define _GRADIENT_I_H_

#include <interface/sobject-i.h>

SNSBEGIN

typedef struct _GradientItem
{
    COLORREF cr;
    float pos;
} GradientItem;

#undef INTERFACE
#define INTERFACE IGradient
DECLARE_INTERFACE_(IGradient, IObject)
{
    //!添加引用
    /*!
     */
    STDMETHOD_(long, AddRef)(THIS) PURE;

    //!释放引用
    /*!
     */
    STDMETHOD_(long, Release)(THIS) PURE;

    //!释放对象
    /*!
     */
    STDMETHOD_(void, OnFinalRelease)(THIS) PURE;

    STDMETHOD_(const GradientItem *, GetGradientData)(CTHIS) SCONST PURE;

    STDMETHOD_(int, GetGradientLength)(CTHIS) SCONST PURE;
};

SNSEND
#endif //_GRADIENT_I_H_
