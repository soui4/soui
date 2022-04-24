#pragma once

#include <interface/sobject-i.h>

SNSBEGIN

#undef INTERFACE
#define INTERFACE IEvtArgs
DECLARE_INTERFACE_(IEvtArgs, IObject)
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
     * IsClass
     * @brief    判断this是不是属于指定的类型
     * @param    LPCWSTR lpszName --  测试类型名
     * @return   BOOL -- true是测试类型
     * Describe
     */
    STDMETHOD_(BOOL, IsClass)(THIS_ LPCWSTR lpszName) SCONST PURE;

    /**
     * GetObjectClass
     * @brief    获得类型名
     * @return   LPCWSTR -- 类型名
     * Describe  这是一个虚函数，注意与GetClassName的区别。
     */
    STDMETHOD_(LPCWSTR, GetObjectClass)(THIS) SCONST PURE;

    /**
     * GetObjectType
     * @brief    获得对象类型
     * @return   int -- 对象类型
     * Describe  这是一个虚函数，注意与GetClassType的区别。
     */
    STDMETHOD_(int, GetObjectType)(THIS) SCONST PURE;

    /**
     * GetID
     * @brief    获取对象ID
     * @return   int -- 对象ID
     * Describe
     */
    STDMETHOD_(int, GetID)(THIS) SCONST PURE;

    /**
     * @brief 设置对象ID
     * @param nID int--对象ID
     * @return
     */
    STDMETHOD_(void, SetID)(THIS_ int nID) PURE;

    /**
     * GetName
     * @brief    获取对象Name
     * @return   LPCWSTR -- 对象Name
     * Describe
     */
    STDMETHOD_(LPCWSTR, GetName)(THIS) SCONST PURE;

    /**
     * @brief 设置对象Name
     * @param pszName LPCWSTR -- 对象Name
     * @return
     */
    STDMETHOD_(void, SetName)(THIS_ LPCWSTR pszName) PURE;

    /**
     * InitFromXml
     * @brief    从XML结节初始化SObject对象
     * @param    SXmlNode --  XML结节
     * @return   BOOL -- 成功返回TRUE
     * Describe
     */
    STDMETHOD_(BOOL, InitFromXml)(THIS_ IXmlNode * xmlNode) PURE;

    /**
     * OnInitFinished
     * @brief    属性初始化完成处理接口
     * @param    SXmlNode xmlNode --  属性节点
     * @return   void
     * Describe
     */
    STDMETHOD_(void, OnInitFinished)(THIS_ IXmlNode * xmlNode) PURE;

    /**
     * SetAttributeA
     * @brief    设置一个对象属性
     * @param    const IStringA * strAttribName --  属性名
     * @param    const IStringA * strValue --  属性值
     * @param    BOOL bLoading --  对象创建时由系统调用标志
     * @return   HRESULT -- 处理处理结果
     * Describe
     */
    STDMETHOD_(HRESULT, ISetAttribute)
    (THIS_ const IStringA *strAttribName, const IStringA *strValue, BOOL bLoading) PURE;

    /**
     * SetAttributeW
     * @brief    设置一个对象属性
     * @param    const IStringA *strAttribName --  属性名
     * @param    const IStringA *strValue --  属性值
     * @param    BOOL bLoading --  对象创建时由系统调用标志
     * @return   HRESULT -- 处理处理结果
     * Describe
     */
    STDMETHOD_(HRESULT, ISetAttributeW)
    (THIS_ const IStringW *strAttribName, const IStringW *strValue, BOOL bLoading) PURE;

    /**
     * SetAttribute
     * @brief    设置一个对象属性
     * @param    LPCSTR pszAttr --  属性名
     * @param    LPCSTR pszValue --  属性值
     * @param    BOOL bLoading --  对象创建时由系统调用标志
     * @return   HRESULT -- 处理处理结果
     * Describe
     */
    STDMETHOD_(HRESULT, SetAttribute)(THIS_ LPCSTR pszAttr, LPCSTR pszValue, BOOL bLoading) PURE;

    /**
     * SetAttribute
     * @brief    设置一个对象属性
     * @param    LPCWSTR pszAttr --  属性名
     * @param    LPCWSTR pszValue --  属性值
     * @param    BOOL bLoading --  对象创建时由系统调用标志
     * @return   HRESULT -- 处理处理结果
     * Describe
     */
    STDMETHOD_(HRESULT, SetAttributeW)(THIS_ LPCWSTR pszAttr, LPCWSTR pszValue, BOOL bLoading) PURE;

    /**
     * GetAttribute
     * @brief    通过属性名查询属性值
     * @param    const SStringW & strAttr --  属性名
     * @param    IStringW * pValue -- 属性值
     * @return   BOOL, TRUE:获取成功，FALSE:获取失败，属性不存在
     * Describe  默认返回空
     */
    STDMETHOD_(BOOL, GetAttribute)(THIS_ const IStringW *strAttr, IStringW *pValue) SCONST PURE;

    /**
     * OnAttribute
     * @brief    属性处理后调用的方法
     * @param    const SStringW & strAttribName --  属性名
     * @param    const SStringW & strValue --  属性名
     * @param    BOOL bLoading --  对象创建时由系统调用标志
     * @param    HRESULT hr --  属性处理结果
     * Describe  不做处理，直接返回
     */
    STDMETHOD_(HRESULT, AfterAttribute)
    (THIS_ const IStringW *strAttribName, const IStringW *strValue, BOOL bLoading, HRESULT hr) PURE;

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
    STDMETHOD_(int, IdFrom)(THIS) SCONST PURE;

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
    STDMETHOD_(LPCWSTR, NameFrom)(THIS) SCONST PURE;

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
    STDMETHOD_(BOOL, IsBubbleUp)(THIS) SCONST PURE;

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
    STDMETHOD_(UINT, HandleCount)(THIS) SCONST PURE;

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
    STDMETHOD_(IEvtSlot *, Clone)(THIS) SCONST PURE;

    /**
     * @brief 判断两个IEvtSlot是否相等
     * @param sour IEvtSlot * 比较的IEvtSlot
     * @return
     */
    STDMETHOD_(BOOL, Equal)(THIS_ const IEvtSlot *sour) SCONST PURE;

    /**
     * @brief 获取IEvtSlot类型
     * @return UINT IEvtSlot类型
     */
    STDMETHOD_(UINT, GetSlotType)(THIS) SCONST PURE;
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