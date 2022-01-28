#pragma once

#include <interface/SMatrix-i.h>
SNSBEGIN

    enum
    {
        /**
         * Indicates a transformation that has no effect (alpha = 1 and identity matrix.)
         */
        TYPE_IDENTITY = 0x0,
        /**
         * Indicates a transformation that applies an alpha only (uses an identity matrix.)
         */
        TYPE_ALPHA = 0x1,
        /**
         * Indicates a transformation that applies a matrix only (alpha = 1.)
         */
        TYPE_MATRIX = 0x2,
        /**
         * Indicates a transformation that applies an alpha and a matrix.
         */
        TYPE_BOTH = TYPE_ALPHA | TYPE_MATRIX,
    };


#undef INTERFACE
#define INTERFACE ITransformation
DECLARE_INTERFACE(ITransformation)
{
	STDMETHOD_(IMatrix *,GetMatrix)(THIS) PURE;

	STDMETHOD_(BYTE,getAlpha) (THIS) SCONST PURE;

	STDMETHOD_(void,setAlpha)(THIS_ BYTE alpha) PURE;

	STDMETHOD_(void,clear)(THIS) PURE;

	STDMETHOD_(void,setTransformationType)(THIS_ int type) PURE;

	STDMETHOD_(void,compose)(const ITransformation *t) PURE;
};

SNSEND