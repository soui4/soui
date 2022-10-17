#pragma once
#include <interface/sobject-i.h>

SNSBEGIN

#undef INTERFACE
#define INTERFACE ICaret
DECLARE_INTERFACE_(ICaret, IObject)
{
#include <interface/SobjectApi.h>
    //-----------------------------------------------------------------------------------

    /**
     * @brief 初始化光标
     * @param hBmp --光标的位图，可以为空
     * @param nWid --光标宽度
     * @param nHei --光标高度
     * @return BOOL TRUE-成功
     */
    STDMETHOD_(BOOL, Init)(THIS_ HBITMAP hBmp, int nWid, int nHei) PURE;

    /**
     * @brief 设置光标位置
     * @param x -- X
     * @param y -- Y
     * @return
     */
    STDMETHOD_(void, SetPosition)(THIS_ int x, int y) PURE;

    /**
     * @brief 设置光标显示状态
     * @param bVisible --显示状态
     * @param owner --光标的宿主
     * @return BOOL TRUE-成功
     */
    STDMETHOD_(BOOL, SetVisible)(THIS_ BOOL bVisible, SWND owner) PURE;

    /**
     * @brief 获取光标显示状态
     * @return BOOL TRUE-显示
     */
    STDMETHOD_(BOOL, IsVisible)(CTHIS) SCONST PURE;

    /**
     * @brief 绘制光标
     * @param pRT --绘制光标的RenderTarget
     * @return
     */
    STDMETHOD_(void, Draw)(THIS_ IRenderTarget * pRT) PURE;

    /**
     * @brief 获取光标显示矩形
     * @return RECT
     */
    STDMETHOD_(RECT, GetRect)(CTHIS) SCONST PURE;
};

SNSEND