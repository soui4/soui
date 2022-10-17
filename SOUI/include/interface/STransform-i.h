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
    /**
     * @brief 获取变换矩阵
     * @return IMatrix *--变换矩阵
     */
    STDMETHOD_(IMatrix *, GetMatrix)(THIS) PURE;

    /**
     * @brief 获取透明度
     * @return BYTE--透明度
     */
    STDMETHOD_(BYTE, GetAlpha)(CTHIS) SCONST PURE;

    /**
     * @brief 设置透明度
     * @param alpha BYTE--透明度
     * @return
     */
    STDMETHOD_(void, SetAlpha)(THIS_ BYTE alpha) PURE;

    /**
     * @brief 清空变换矩阵及透明度
     * @return
     */
    STDMETHOD_(void, Clear)(THIS) PURE;

    /**
     * @brief 设置变化类型
     * @param type int--类型
     * @return
     */
    STDMETHOD_(void, SetTransformationType)(THIS_ int type) PURE;

    /**
     * @brief 组合变换矩阵及透明度
     * @param t ITransformation *--变换对象
     * @return
     */
    STDMETHOD_(void, Compose)(const ITransformation *t) PURE;
};

SNSEND