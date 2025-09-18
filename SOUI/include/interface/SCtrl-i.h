/************************************************************************/
/*   这里定义控件操作接口,所有接口都从IObjRef继承,并且指定自己的接口ID  */
/************************************************************************/
#ifndef __SCTRL_I__H__
#define __SCTRL_I__H__
#include <interface/SWindow-i.h>
#include <SGuidDef.h>
SNSBEGIN

// {BCB520F9-3C81-4AD7-AD E2-FE72BD761032}
DEFINE_SGUID(IID_ICtrl, 0xBCB520F9, 0x3C81, 0x4AD7, 0xAD, 0xe2, 0xFE, 0x72, 0xBD, 0x76, 0x10, 0x32);
#undef INTERFACE
#define INTERFACE ICtrl
DECLARE_INTERFACE_(ICtrl, IObjRef)
{
    DECLARE_CLASS_SIID(IID_ICtrl)

    /**
     * @brief 增加引用计数
     * @return 新引用计数
     */
    STDMETHOD_(long, AddRef)(THIS) PURE;

    /**
     * @brief 减少引用计数
     * @return 新引用计数
     */
    STDMETHOD_(long, Release)(THIS) PURE;

    /**
     * @brief 释放对象
     * @return void
     */
    STDMETHOD_(void, OnFinalRelease)(THIS) PURE;

    //////////////////////////////////////////////////////////////////////////

    /**
     * @brief 转换为IWindow*接口
     * @return IWindow*接口
     */
    STDMETHOD_(IWindow *, ToIWindow)(THIS) PURE;
};


SNSEND
#endif // __SCTRL_I__H__