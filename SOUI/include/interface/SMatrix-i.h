#ifndef __SMATRIX_I__H__
#define __SMATRIX_I__H__
#include <interface/SRender-i.h>

SNSBEGIN

#undef INTERFACE
#define INTERFACE IMatrix
DECLARE_INTERFACE(IMatrix)
{
    /**
     * @brief 获取矩阵数据
     * @return IxForm* -- 矩阵数据
     */
    STDMETHOD_(IxForm *, Data)(CTHIS) SCONST PURE;

    /**
     * @brief 清空矩阵
     * @return void
     */
    STDMETHOD_(void, reset)(THIS) PURE;

    /**
     * @brief 将矩阵设定为单位矩阵
     * @return void
     */
    STDMETHOD_(void, setIdentity)(THIS) PURE;

    /**
     * @brief 判断矩阵是否为单位矩阵
     * @return BOOL -- TRUE: 矩阵是单位矩阵
     */
    STDMETHOD_(BOOL, isIdentity)(CTHIS) SCONST PURE;

    /**
     * @brief 设置矩阵平移
     * @param dx float -- x方向平移量
     * @param dy float -- y方向平移量
     * @return void
     */
    STDMETHOD_(void, setTranslate)(THIS_ float dx, float dy) PURE;

    /**
     * @brief 设置矩阵缩放
     * @param sx float -- x方向缩放比例
     * @param sy float -- y方向缩放比例
     * @return void
     */
    STDMETHOD_(void, setScale)(THIS_ float sx, float sy) PURE;

    /**
     * @brief 设置矩阵缩放，并指定缩放中心点
     * @param sx float -- x方向缩放比例
     * @param sy float -- y方向缩放比例
     * @param px float -- 缩放中心点x坐标
     * @param py float -- 缩放中心点y坐标
     * @return void
     */
    STDMETHOD_(void, setScale2)(THIS_ float sx, float sy, float px, float py) PURE;

    /**
     * @brief 设置矩阵旋转
     * @param degrees float -- 旋转角度
     * @return void
     */
    STDMETHOD_(void, setRotate)(THIS_ float degrees) PURE;

    /**
     * @brief 设置矩阵旋转，并指定旋转中心点
     * @param degrees float -- 旋转角度
     * @param px float -- 旋转中心点x坐标
     * @param py float -- 旋转中心点y坐标
     * @return void
     */
    STDMETHOD_(void, setRotate2)(THIS_ float degrees, float px, float py) PURE;

    /**
     * @brief 设置矩阵倾斜
     * @param kx float -- x方向倾斜量
     * @param ky float -- y方向倾斜量
     * @return void
     */
    STDMETHOD_(void, setSkew)(THIS_ float kx, float ky) PURE;

    /**
     * @brief 设置矩阵倾斜，并指定倾斜中心点
     * @param kx float -- x方向倾斜量
     * @param ky float -- y方向倾斜量
     * @param px float -- 倾斜中心点x坐标
     * @param py float -- 倾斜中心点y坐标
     * @return void
     */
    STDMETHOD_(void, setSkew2)(THIS_ float kx, float ky, float px, float py) PURE;
};

SNSEND
#endif // __SMATRIX_I__H__
