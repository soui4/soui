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
    /**
     * @brief Add reference
     * @return long -- reference count
     */
    STDMETHOD_(long, AddRef)(THIS) PURE;

    /**
     * @brief Release reference
     * @return long -- reference count
     */
    STDMETHOD_(long, Release)(THIS) PURE;

    /**
     * @brief Release object
     * @return void
     */
    STDMETHOD_(void, OnFinalRelease)(THIS) PURE;

    /**
     * @brief Get gradient data
     * @return const GradientItem* -- gradient data
     */
    STDMETHOD_(const GradientItem *, GetGradientData)(CTHIS) SCONST PURE;

    /**
     * @brief Get gradient length
     * @return int -- gradient length
     */
    STDMETHOD_(int, GetGradientLength)(CTHIS) SCONST PURE;
};

SNSEND
#endif /**< _GRADIENT_I_H_ */
