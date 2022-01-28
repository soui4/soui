#pragma once
#include <interface/SRender-i.h>

SNSBEGIN

#undef INTERFACE
#define INTERFACE IMatrix
DECLARE_INTERFACE(IMatrix)
{
	STDMETHOD_(IxForm *,Data)(THIS) PURE;

	STDMETHOD_(void,reset)(THIS) PURE;
    // alias for reset()
    STDMETHOD_(void,setIdentity)(THIS) PURE;

	STDMETHOD_(BOOL,isIdentity)(THIS) SCONST PURE;

    /** Set the matrix to translate by (dx, dy).
     */
    STDMETHOD_(void,setTranslate)(THIS_ float dx, float dy) PURE;

    /** Set the matrix to scale by sx and sy, with a pivot point at (px, py).
        The pivot point is the coordinate that should remain unchanged by the
        specified transformation.
    */
    STDMETHOD_(void,setScale)(THIS_ float sx, float sy) PURE;

	    /** Set the matrix to scale by sx and sy, with a pivot point at (px, py).
        The pivot point is the coordinate that should remain unchanged by the
        specified transformation.
    */
    STDMETHOD_(void,setScale2)(THIS_ float sx, float sy, float px, float py) PURE;

    /** Set the matrix to scale by sx and sy.
     */
    STDMETHOD_(void,setRotate)(THIS_ float degrees) PURE;

	    /** Set the matrix to rotate by the specified number of degrees, with a
        pivot point at (px, py). The pivot point is the coordinate that should
        remain unchanged by the specified transformation.
    */
    STDMETHOD_(void,setRotate2)(THIS_ float degrees, float px, float py) PURE;

	 /** Set the matrix to skew by sx and sy.
     */
    STDMETHOD_(void,setSkew)(THIS_ float kx, float ky) PURE;

};

SNSEND