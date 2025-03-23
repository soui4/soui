/**
 * Copyright (C) 2014-2050
 * All rights reserved.
 *
 * @file       SSkinobj-i.h
 * @brief
 * @version    v1.0
 * @author     SOUI group
 * @date       2014/08/01
 *
 * Describe    interface of SSkinObj
 */
#ifndef __SSKINOBJ_I__H__
#define __SSKINOBJ_I__H__

#include <interface/obj-ref-i.h>
#include <interface/SRender-i.h>

SNSBEGIN

/**
 * @struct     ISkinObj
 * @brief      Skin 对象
 *
 * Describe
 */
#undef INTERFACE
#define INTERFACE ISkinObj
DECLARE_INTERFACE_(ISkinObj, IObject)
{
    DEF_OBJ_BASE(ISkinObj, Skin)
#include <interface/SobjectApi.h>
    //------------------------------------------------------------------------

    /**
     * @brief 将this绘制到RenderTarget上去
     * @param pRT IRenderTarget* -- 绘制用的RenderTarget
     * @param rcDraw LPCRECT -- 绘制位置
     * @param dwState DWORD -- 绘制状态
     * @param byAlpha BYTE -- 绘制透明度
     * @return void
     */
    STDMETHOD_(void, DrawByState2)
    (CTHIS_ IRenderTarget * pRT, LPCRECT rcDraw, DWORD dwState, BYTE byAlpha) SCONST PURE;

    /**
     * @brief 将this绘制到RenderTarget上去
     * @param pRT IRenderTarget* -- 绘制用的RenderTarget
     * @param rcDraw LPCRECT -- 绘制位置
     * @param dwState DWORD -- 绘制状态
     * @return void
     * @remark 使用SkinObj的GetAlpha属性绘制
     */
    STDMETHOD_(void, DrawByState)
    (CTHIS_ IRenderTarget * pRT, LPCRECT rcDraw, DWORD dwState) SCONST PURE;

    /**
     * @brief 将this绘制到RenderTarget上去
     * @param pRT IRenderTarget* -- 绘制用的RenderTarget
     * @param rcDraw LPCRECT -- 绘制位置
     * @param iState int -- 绘制Index
     * @param byAlpha BYTE -- 绘制透明度
     * @return void
     */
    STDMETHOD_(void, DrawByIndex2)
    (CTHIS_ IRenderTarget * pRT, LPCRECT rcDraw, int iState, BYTE byAlpha) SCONST PURE;

    /**
     * @brief 将this绘制到RenderTarget上去
     * @param pRT IRenderTarget* -- 绘制用的RenderTarget
     * @param rcDraw LPCRECT -- 绘制位置
     * @param iState int -- 绘制Index
     * @return void
     * @remark 使用SkinObj的GetAlpha属性绘制
     */
    STDMETHOD_(void, DrawByIndex)
    (CTHIS_ IRenderTarget * pRT, LPCRECT rcDraw, int iState) SCONST PURE;

    /**
     * @brief 获得Skin的默认大小
     * @return SIZE -- Skin的默认大小
     * @remark 派生类应该根据skin的特点实现该接口
     */
    STDMETHOD_(SIZE, GetSkinSize)(CTHIS) SCONST PURE;

    /**
     * @brief 获得skin对象包含的状态数量
     * @return int -- 状态数量
     * @remark 默认为1
     */
    STDMETHOD_(int, GetStates)(CTHIS) SCONST PURE;

    /**
     * @brief 获得skin对象包含透明度
     * @return BYTE -- 透明度
     * @remark [0-255]
     */
    STDMETHOD_(BYTE, GetAlpha)(CTHIS) SCONST PURE;

    /**
     * @brief 设定skin对象包含透明度
     * @param byAlpha BYTE -- 透明度
     * @return void
     */
    STDMETHOD_(void, SetAlpha)(THIS_ BYTE byAlpha) PURE;

    /**
     * @brief 调整皮肤对象色调
     * @param cr COLORREF -- 参考颜色
     * @return void
     * @remark 默认不处理
     */
    STDMETHOD_(void, OnColorize)(THIS_ COLORREF cr) PURE;

    /**
     * @brief 获取放大比例
     * @return int -- 放大比例
     */
    STDMETHOD_(int, GetScale)(CTHIS) SCONST PURE;

    /**
     * @brief 设置放大比例
     * @param scale int -- 放大比例
     * @remark 内部接口，不要在业务层调用
     */
    STDMETHOD_(void, SetScale)(THIS_ int scale) PURE;

    /**
     * @brief 将当前Skin按照指定比例缩放
     * @param nScale int -- 放大比例
     * @return ISkinObj* -- 缩放后的SkinObj
     */
    STDMETHOD_(ISkinObj *, Scale)(THIS_ int nScale) PURE;
};

SNSEND
#endif // __SSKINOBJ_I__H__

