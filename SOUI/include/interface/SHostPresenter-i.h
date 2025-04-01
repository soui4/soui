#ifndef __SHOSTPRESENTER_I__H__
#define __SHOSTPRESENTER_I__H__
#include <interface/obj-ref-i.h>
#include <interface/SRender-i.h>
SNSBEGIN

#undef INTERFACE
#define INTERFACE IHostPresenter
DECLARE_INTERFACE_(IHostPresenter, IObjRef)
{
    //! 添加引用
    /*!
     * @return long - 当前引用计数
     */
    STDMETHOD_(long, AddRef)(THIS) PURE;

    //! 释放引用
    /*!
     * @return long - 当前引用计数
     */
    STDMETHOD_(long, Release)(THIS) PURE;

    //! 释放对象
    /*!
     * @return void
     */
    STDMETHOD_(void, OnFinalRelease)(THIS) PURE;

    //////////////////////////////////////////////////////////////////////////

    /**
     * @brief 宿主创建时调用
     * @return void
     */
    STDMETHOD_(void, OnHostCreate)(THIS) PURE;

    /**
     * @brief 宿主销毁时调用
     * @return void
     */
    STDMETHOD_(void, OnHostDestroy)(THIS) PURE;

    /**
     * @brief 宿主大小调整时调用
     * @param szHost - 宿主的新大小
     * @return void
     */
    STDMETHOD_(void, OnHostResize)(THIS_ SIZE szHost) PURE;

    /**
     * @brief 宿主呈现时调用
     * @param hdc - 设备上下文句柄
     * @param pMemRT - 内存渲染目标
     * @param rcDirty - 脏矩形区域
     * @param byAlpha - 透明度
     * @return void
     */
    STDMETHOD_(void, OnHostPresent)(THIS_ HDC hdc, IRenderTarget * pMemRT, LPCRECT rcDirty, BYTE byAlpha) PURE;
};

SNSEND
#endif // __SHOSTPRESENTER_I__H__
