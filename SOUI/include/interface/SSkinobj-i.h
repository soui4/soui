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
#pragma once

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
#include <interface/SobjectApi.h>
    //------------------------------------------------------------------------

    /**
     * DrawByState
     * @brief    将this绘制到RenderTarget上去
     * @param    IRenderTarget * pRT --  绘制用的RenderTarget
     * @param    LPCRECT rcDraw --  绘制位置
     * @param    DWORD dwState --  绘制状态
     * @param    BYTE byAlpha --  绘制透明度
     * @return   void
     * Describe
     */
    STDMETHOD_(void, DrawByState2)
    (CTHIS_ IRenderTarget * pRT, LPCRECT rcDraw, DWORD dwState, BYTE byAlpha) SCONST PURE;

    /**
     * DrawByState
     * @brief    将this绘制到RenderTarget上去
     * @param    IRenderTarget * pRT --  绘制用的RenderTarget
     * @param    LPCRECT rcDraw --  绘制位置
     * @param    DWORD dwState --  绘制状态
     * @return   void
     * Describe  使用SkinObj的GetAlpha属性绘制
     */
    STDMETHOD_(void, DrawByState)
    (CTHIS_ IRenderTarget * pRT, LPCRECT rcDraw, DWORD dwState) SCONST PURE;

    /**
     * DrawByIndex
     * @brief    将this绘制到RenderTarget上去
     * @param    IRenderTarget * pRT --  绘制用的RenderTarget
     * @param    LPCRECT rcDraw --  绘制位置
     * @param    int iState --  绘制Index
     * @param    BYTE byAlpha --  绘制透明度
     * @return   void
     * Describe
     */
    STDMETHOD_(void, DrawByIndex2)
    (CTHIS_ IRenderTarget * pRT, LPCRECT rcDraw, int iState, BYTE byAlpha) SCONST PURE;

    /**
     * DrawByIndex
     * @brief    将this绘制到RenderTarget上去
     * @param    IRenderTarget * pRT --  绘制用的RenderTarget
     * @param    LPCRECT rcDraw --  绘制位置
     * @param    int iState --  绘制Index
     * @return   void
     * Describe  使用SkinObj的GetAlpha属性绘制
     */
    STDMETHOD_(void, DrawByIndex)
    (CTHIS_ IRenderTarget * pRT, LPCRECT rcDraw, int iState) SCONST PURE;

    /**
     * GetSkinSize
     * @brief    获得Skin的默认大小
     * @return   SIZE -- Skin的默认大小
     * Describe  派生类应该根据skin的特点实现该接口
     */
    STDMETHOD_(SIZE, GetSkinSize)(CTHIS) SCONST PURE;

    /**
     * GetStates
     * @brief    获得skin对象包含的状态数量
     * @return   int -- 状态数量
     * Describe  默认为1
     */
    STDMETHOD_(int, GetStates)(CTHIS) SCONST PURE;

    /**
     * GetAlpha
     * @brief    获得skin对象包含透明度
     * @return   BYTE -- 透明度
     * Describe  [0-255]
     */
    STDMETHOD_(BYTE, GetAlpha)(CTHIS) SCONST PURE;

    /**
     * SetAlpha
     * @brief    设定skin对象包含透明度
     * @param    BYTE byAlpha-- 透明度
     * @return   void
     * Describe
     */
    STDMETHOD_(void, SetAlpha)(THIS_ BYTE byAlpha) PURE;

    /**
     * OnColorize
     * @brief    调整皮肤对象色调
     * @param    COLORREF cr -- 参考颜色
     * @return   void
     * Describe  默认不处理
     */
    STDMETHOD_(void, OnColorize)(THIS_ COLORREF cr) PURE;

    /**
     * GetScale
     * @brief    获取放大比例
     * @return   int 放大比例
     * Describe
     */
    STDMETHOD_(int, GetScale)(CTHIS) SCONST PURE;

    /**
     * Scale
     * @brief    将当前Skin按照指定比例缩放
     * @param    int nScale -- 放大比例
     * @return   ISkinObj * -- 缩放后的SkinObj
     * Describe
     */
    STDMETHOD_(ISkinObj *, Scale)(THIS_ int nScale) PURE;
};

SNSEND