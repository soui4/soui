#pragma once

#include <interface/sobject-i.h>

SNSBEGIN

#undef INTERFACE
#define INTERFACE IEvtArgs
DECLARE_INTERFACE_(IEvtArgs, IObject)
{
#include <interface/SobjectApi.h>

    //-------------------------------------------------------------------------------------------

    /**
     * @brief 获取事件发送对象
     * @return IObject *--事件发送对象
     */
    STDMETHOD_(IObject *, Sender)(THIS) PURE;

    /**
     * @brief 获取事件发送对象ID
     * @return int--事件发送对象ID
     */
    STDMETHOD_(int, IdFrom)(CTHIS) SCONST PURE;

    /**
     * @brief 设置事件的发送对象ID
     * @param id int--发送对象ID
     * @return
     */
    STDMETHOD_(void, SetIdFrom)(THIS_ int id) PURE;

    /**
     * @brief 获取事件发送对象Name
     * @return LPCWSTR--事件发送对象Name
     */
    STDMETHOD_(LPCWSTR, NameFrom)(CTHIS) SCONST PURE;

    /**
     * @brief 设置事件发送对象Name
     * @param name LPCWSTR--事件发送对象Name
     * @return
     */
    STDMETHOD_(void, SetNameFrom)(THIS_ LPCWSTR name) PURE;

    /**
     * @brief 获取事件是否需要冒泡传递
     * @return BOOL TRUE--冒泡传递
     */
    STDMETHOD_(BOOL, IsBubbleUp)(CTHIS) SCONST PURE;

    /**
     * @brief 设置事件的冒泡传递
     * @param bBubbleUp BOOL, FALSE--中止传递
     * @return
     */
    STDMETHOD_(void, SetBubbleUp)(THIS_ BOOL bBubbleUp) PURE;

    /**
     * @brief 获取事件处理次数
     * @return UINT 事件处理次数
     */
    STDMETHOD_(UINT, HandleCount)(CTHIS) SCONST PURE;

    /**
     * @brief 增加事件处理计数
     * @return
     */
    STDMETHOD_(void, IncreaseHandleCount)(THIS) PURE;

    /**
     * @brief 获取事件对象关联的数据结构体指针
     * @return 事件对象关联的数据结构体指针
     * @remark 不同事件对象包含不同的数据，获取指针再转换为结构体指针。主要给C接口使用。
     */
    STDMETHOD_(LPVOID, Data)(THIS) PURE;
};

/*!
\brief
    Defines abstract interface which will be used when constructing various
    functor objects that bind slots to signals (or in CEGUI terms, handlers to
    events).
*/
#undef INTERFACE
#define INTERFACE IEvtSlot
DECLARE_INTERFACE_(IEvtSlot, IObjRef)
{
    //!添加引用
    /*!
     */
    STDMETHOD_(long, AddRef)(THIS) PURE;

    //!释放引用
    /*!
     */
    STDMETHOD_(long, Release)(THIS) PURE;

    //!释放对象
    /*!
     */
    STDMETHOD_(void, OnFinalRelease)(THIS) PURE;

    /**
     * @brief 执行事件
     * @param pArg IEvtArgs* 事件对象
     * @return BOOL TRUE--响应了事件，事件对象增加使用计数, FALSE--事件没有响应
     */
    STDMETHOD_(BOOL, Run)(THIS_ IEvtArgs * pArg) PURE;

    /**
     * @brief clone一个事件槽
     * @return IEvtSlot * clone出来的对象
     */
    STDMETHOD_(IEvtSlot *, Clone)(CTHIS) SCONST PURE;

    /**
     * @brief 判断两个IEvtSlot是否相等
     * @param sour IEvtSlot * 比较的IEvtSlot
     * @return
     */
    STDMETHOD_(BOOL, Equal)(CTHIS_ const IEvtSlot *sour) SCONST PURE;

    /**
     * @brief 获取IEvtSlot类型
     * @return UINT IEvtSlot类型
     */
    STDMETHOD_(UINT, GetSlotType)(CTHIS) SCONST PURE;
};

/**
 * @brief 事件处理回调
 * @param pEvt IEvtArgs * 事件对象
 * @param Ctx void * 事件处理上下文
 * @return BOOL TRUE--事件被处理，增加处理计数
 */
typedef BOOL (*FunCallback)(IEvtArgs *pEvt, void *Ctx);

#define EventArgs IEvtArgs
SNSEND