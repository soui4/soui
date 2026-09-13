#ifndef __SMATRIX_I__H__
#define __SMATRIX_I__H__
#include <interface/SRender-i.h>

SNSBEGIN

#undef INTERFACE
#define INTERFACE IMatrix
DECLARE_INTERFACE(IMatrix)
{
    /**
     * @brief Get matrix data
     * @return IxForm* -- matrix data
     */
    STDMETHOD_(IxForm *, Data)(CTHIS) SCONST PURE;

    /**
     * @brief Clear matrix
     * @return void
     */
    STDMETHOD_(void, reset)(THIS) PURE;

    /**
     * @brief Set matrix to identity matrix
     * @return void
     */
    STDMETHOD_(void, setIdentity)(THIS) PURE;

    /**
     * @brief Check whether matrix is identity matrix
     * @return BOOL -- TRUE: matrix is identity matrix
     */
    STDMETHOD_(BOOL, isIdentity)(CTHIS) SCONST PURE;

    /**
     * @brief Set matrix translation
     * @param dx float -- translation on x axis
     * @param dy float -- translation on y axis
     * @return void
     */
    STDMETHOD_(void, setTranslate)(THIS_ float dx, float dy) PURE;

    /**
     * @brief Set matrix scale
     * @param sx float -- scale ratio on x axis
     * @param sy float -- scale ratio on y axis
     * @return void
     */
    STDMETHOD_(void, setScale)(THIS_ float sx, float sy) PURE;

    /**
     * @brief Set matrix scale with specified scale center
     * @param sx float -- scale ratio on x axis
     * @param sy float -- scale ratio on y axis
     * @param px float -- x coordinate of scale center
     * @param py float -- y coordinate of scale center
     * @return void
     */
    STDMETHOD_(void, setScale2)(THIS_ float sx, float sy, float px, float py) PURE;

    /**
     * @brief Set matrix rotation
     * @param degrees float -- rotation angle
     * @return void
     */
    STDMETHOD_(void, setRotate)(THIS_ float degrees) PURE;

    /**
     * @brief Set matrix rotation with specified rotation center
     * @param degrees float -- rotation angle
     * @param px float -- x coordinate of rotation center
     * @param py float -- y coordinate of rotation center
     * @return void
     */
    STDMETHOD_(void, setRotate2)(THIS_ float degrees, float px, float py) PURE;

    /**
     * @brief Set matrix skew
     * @param kx float -- skew amount on x axis
     * @param ky float -- skew amount on y axis
     * @return void
     */
    STDMETHOD_(void, setSkew)(THIS_ float kx, float ky) PURE;

    /**
     * @brief Set matrix skew with specified skew center
     * @param kx float -- skew amount on x axis
     * @param ky float -- skew amount on y axis
     * @param px float -- x coordinate of skew center
     * @param py float -- y coordinate of skew center
     * @return void
     */
    STDMETHOD_(void, setSkew2)(THIS_ float kx, float ky, float px, float py) PURE;
};

SNSEND
#endif /**< __SMATRIX_I__H__ */
