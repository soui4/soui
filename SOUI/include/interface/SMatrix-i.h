#pragma once
#include <interface/SRender-i.h>

SNSBEGIN

#undef INTERFACE
#define INTERFACE IMatrix
DECLARE_INTERFACE(IMatrix)
{
    /**
     * @brief 获取矩阵数据
     * @return IxForm *--矩阵数据
     */
    STDMETHOD_(IxForm *, Data)(CTHIS) SCONST PURE;

    /**
     * @brief 清空矩阵
     * @return
     */
    STDMETHOD_(void, reset)(THIS) PURE;

    /**
     * @brief 将矩阵设定为单位矩阵
     * @return
     */
    STDMETHOD_(void, setIdentity)(THIS) PURE;

    /**
     * @brief 判断矩阵是否为单位矩阵
     * @return TRUE--矩阵是单位矩阵
     */
    STDMETHOD_(BOOL, isIdentity)(CTHIS) SCONST PURE;

    /**
     * @brief Set the matrix to translate by (dx, dy).
     * @param dx int
     * @param dy int
     * @return
     */
    STDMETHOD_(void, setTranslate)(THIS_ float dx, float dy) PURE;

    /**
     * @brief Set the matrix to scale by sx and sy.
     * @param sx int
     * @param sy int
     * @return
     */
    STDMETHOD_(void, setScale)(THIS_ float sx, float sy) PURE;

    /**
     * @brief Set the matrix to scale by sx and sy, with a pivot point at (0, 0).
        The pivot point is the coordinate that should remain unchanged by the
        specified transformation.
     * @param sx int
     * @param sy int
     * @param px int
     * @param py int
     * @return
    */
    STDMETHOD_(void, setScale2)(THIS_ float sx, float sy, float px, float py) PURE;

    /**
     * @brief Set the matrix to rotate by the specified number of degrees
     * @param degrees float--rotate degree
     * @return
     */
    STDMETHOD_(void, setRotate)(THIS_ float degrees) PURE;

    /**
     * @brief Set the matrix to rotate by the specified number of degrees, with a
    pivot point at (px, py). The pivot point is the coordinate that should
    remain unchanged by the specified transformation.
     * @param degrees
     * @param px
     * @param py
     * @return
    */
    STDMETHOD_(void, setRotate2)(THIS_ float degrees, float px, float py) PURE;

    /**
     * @brief Set the matrix to skew by kx and ky.
     * @param kx int
     * @param ky int
     * @return
     */
    STDMETHOD_(void, setSkew)(THIS_ float kx, float ky) PURE;

    /**
     * @brief Set the matrix to skew by kx and ky, with a pivot point at (px, py).
    The pivot point is the coordinate that should remain unchanged by the
    specified transformation.
     * @param kx
     * @param ky
     * @param px
     * @param py
    */
    STDMETHOD_(void, setSkew2)(THIS_ float kx, float ky, float px, float py) PURE;
};

SNSEND