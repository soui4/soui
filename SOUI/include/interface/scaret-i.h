#pragma once
#include <interface/sobject-i.h>

SNSBEGIN

#undef INTERFACE
#define INTERFACE ICaret
DECLARE_INTERFACE_(ICaret, IObject)
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
    STDMETHOD_(BOOL, IsVisible)(THIS) SCONST PURE;

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
    STDMETHOD_(RECT, GetRect)(THIS) SCONST PURE;
};

SNSEND