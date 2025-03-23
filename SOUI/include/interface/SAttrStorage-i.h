#ifndef __SATTRSTORAGE_I__H__
#define __SATTRSTORAGE_I__H__
#include <interface/obj-ref-i.h>
#include <interface/sstring-i.h>

SNSBEGIN

typedef struct IWindow IWindow;

#undef INTERFACE
#define INTERFACE IAttrStorage
DECLARE_INTERFACE_(IAttrStorage, IObjRef)
{
    /**
     * @brief 添加引用
     * @return long -- 引用计数
     */
    STDMETHOD_(long, AddRef)(THIS) PURE;

    /**
     * @brief 释放引用
     * @return long -- 引用计数
     */
    STDMETHOD_(long, Release)(THIS) PURE;

    /**
     * @brief 释放对象
     * @return void
     */
    STDMETHOD_(void, OnFinalRelease)(THIS) PURE;

    //////////////////////////////////////////////////////////////////////////

    /**
     * @brief 响应IObject::SetAttribute的时保存attribute值的方法
     * @param strName const IStringW* -- 属性名
     * @param strValue const IStringW* -- 属性值
     * @param bHandled BOOL -- 该属性是否已经被处理
     * @return void
     */
    STDMETHOD_(void, OnSetAttribute)
    (THIS_ const IStringW *strName, const IStringW *strValue, BOOL bHandled) PURE;

    /**
     * @brief 响应IObject::GetAttribute调用
     * @param strName const IStringW* -- 属性名
     * @param[out] strValue IStringW* -- 返回的属性值
     * @return BOOL -- TRUE: 成功
     */
    STDMETHOD_(BOOL, OnGetAttribute)(CTHIS_ const IStringW *strName, IStringW *strValue) SCONST PURE;
};

#undef INTERFACE
#define INTERFACE IAttrStorageFactory
DECLARE_INTERFACE_(IAttrStorageFactory, IObjRef)
{
    /**
     * @brief 添加引用
     * @return long -- 引用计数
     */
    STDMETHOD_(long, AddRef)(THIS) PURE;

    /**
     * @brief 释放引用
     * @return long -- 引用计数
     */
    STDMETHOD_(long, Release)(THIS) PURE;

    /**
     * @brief 释放对象
     * @return void
     */
    STDMETHOD_(void, OnFinalRelease)(THIS) PURE;

    //////////////////////////////////////////////////////////////////////////

    /**
     * @brief 创建IAttrStorage对象
     * @param owner IWindow* -- IAttrStorage的宿主
     * @param[out] ppAttrStorage IAttrStorage** -- 返回值
     * @return HRESULT -- S_OK: 成功
     */
    STDMETHOD_(HRESULT, CreateAttrStorage)
    (CTHIS_ IWindow * owner, IAttrStorage * *ppAttrStorage) SCONST PURE;
};

SNSEND
#endif // __SATTRSTORAGE_I__H__



