#ifndef __STRANSFORM_I__H__
#define __STRANSFORM_I__H__

#include <interface/SMatrix-i.h>
SNSBEGIN

enum
{
    /** Indicates a transformation that has no effect (alpha = 1 and identity matrix.) */
    TYPE_IDENTITY = 0x0,
    /** Indicates a transformation that applies an alpha only (uses an identity matrix.) */
    TYPE_ALPHA = 0x1,
    /** Indicates a transformation that applies a matrix only (alpha = 1.) */
    TYPE_MATRIX = 0x2,
    /** Indicates a transformation that applies an alpha and a matrix. */
    TYPE_BOTH = TYPE_ALPHA | TYPE_MATRIX,
};

#undef INTERFACE
#define INTERFACE ITransformation

DECLARE_INTERFACE(ITransformation)
{
    /**
     * @brief Get the transformation matrix
     * @return IMatrix *--Transformation matrix
     */
    STDMETHOD_(IMatrix *, GetMatrix)(THIS) PURE;

    /**
     * @brief Get the opacity
     * @return BYTE--Opacity
     */
    STDMETHOD_(BYTE, GetAlpha)(CTHIS) SCONST PURE;

    /**
     * @brief Set the opacity
     * @param alpha BYTE--Opacity
     * @return
     */
    STDMETHOD_(void, SetAlpha)(THIS_ BYTE alpha) PURE;

    /**
     * @brief Clear the transformation matrix and opacity
     * @return
     */
    STDMETHOD_(void, Clear)(THIS) PURE;

    /**
     * @brief Set the change type
     * @param type int--Type
     * @return
     */
    STDMETHOD_(void, SetTransformationType)(THIS_ int type) PURE;

    /**
     * @brief Combine the transformation matrix and opacity
     * @param t ITransformation *--Transformation object
     * @return
     */
    STDMETHOD_(void, Compose)(const ITransformation *t) PURE;
};

SNSEND
#endif /**< __STRANSFORM_I__H__ */