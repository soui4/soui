/*
SOUI窗口布局接口
*/
#pragma once

#include <interface/sobject-i.h>

SNSBEGIN

typedef struct IWindow IWindow;

typedef enum ORIENTATION
{
    Horz,
    Vert,
    Any,
    Both,
}ORIENTATION;

enum
{
    SIZE_UNDEF = -3,
    SIZE_WRAP_CONTENT = -1,
    SIZE_MATCH_PARENT = -2,
    SIZE_SPEC = 0,
};

#undef INTERFACE
#define INTERFACE ILayoutParam
DECLARE_INTERFACE_(ILayoutParam, IObject)
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

    STDMETHOD_(int, GetID)(THIS) SCONST PURE;
    STDMETHOD_(void, SetID)(THIS_ int nID) PURE;

    STDMETHOD_(LPCWSTR, GetName)(THIS) SCONST PURE;
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
     * @param    LPCWSTR pszAttr --  属性名
     * @param    LPCWSTR pszValue --  属性值
     * @param    BOOL bLoading --  对象创建时由系统调用标志
     * @return   HRESULT -- 处理处理结果
     * Describe
     */
    STDMETHOD_(HRESULT, SetAttribute)(THIS_ LPCSTR pszAttr, LPCSTR pszValue, BOOL bLoading) PURE;

	 /**
     * SetAttributeW
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

    //----------------------------------------------------------
    
    /**
     * @brief 清空数据
     * @return 
    */
    STDMETHOD_(void, Clear)(THIS) PURE;

    /**
     * @brief 布局充满父窗口标志
     * @param orientation ORIENTATION--布局方向
     * @return TRUE--布局充满父窗口
    */
    STDMETHOD_(BOOL, IsMatchParent)(THIS_ ORIENTATION orientation) SCONST PURE;

    /**
     * @brief 布局适应窗口内容标志
     * @param orientation ORIENTATION--布局方向
     * @return TRUE--适应窗口内容标志
     */
    STDMETHOD_(BOOL, IsWrapContent)(THIS_ ORIENTATION orientation) SCONST PURE;

    /**
     * @brief 布局指定大小标志
     * @param orientation ORIENTATION--布局方向
     * @return TRUE--指定大小
    */
    STDMETHOD_(BOOL, IsSpecifiedSize)(THIS_ ORIENTATION orientation) SCONST PURE;

    /**
     * @brief 获取指定的布局大小
     * @param orientation ORIENTATION--布局方向
     * @return SLayoutSize--布局大小
    */
    STDMETHOD_(SLayoutSize, GetSpecifiedSize)(THIS_ ORIENTATION orientation) SCONST PURE;

    /**
     * @brief 设定布局适应父窗口大小
     * @param orientation ORIENTATION--布局方向
     * @return 
    */
    STDMETHOD_(void, SetMatchParent)(THIS_ ORIENTATION orientation) PURE;

    /**
     * @brief 设定布局适应内容
     * @param orientation ORIENTATION--布局方向
     * @return 
    */
    STDMETHOD_(void, SetWrapContent)(THIS_ ORIENTATION orientation) PURE;

    /**
     * @brief 设定布局大小
     * @param orientation ORIENTATION--布局方向
     * @param layoutSize SLayoutSize--布局大小
     * @return 
    */
    STDMETHOD_(void, SetSpecifiedSize)
    (THIS_ ORIENTATION orientation, const SLayoutSize &layoutSize) PURE;

    /**
     * @brief 获取布局结构体数据
     * @return void*布局结构体
    */
    STDMETHOD_(void *, GetRawData)(THIS) PURE;

    /**
     * @brief Clone布局参数
     * @return ILayoutParam *--布局参数对象
    */
    STDMETHOD_(ILayoutParam *, Clone)(THIS) SCONST PURE;
};

#undef INTERFACE
#define INTERFACE ILayout
DECLARE_INTERFACE_(ILayout, IObject)
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
    //------------------------------------------------------------------------

    /**
     * @brief 判断当前布局类型和布局参数是否匹配
     * @param pLayoutParam const ILayoutParam *--布局参数
     * @return TRUE--匹配
    */
    STDMETHOD_(BOOL, IsParamAcceptable)(THIS_ const ILayoutParam *pLayoutParam) SCONST PURE;

    /**
     * @brief 布局指定控件的子窗口
     * @param pParent IWindow *--布局控件
     * @return 
    */
    STDMETHOD_(void, LayoutChildren)(THIS_ IWindow * pParent) PURE;

    /**
     * @brief 创建和当前布局匹配的布局参数对象
     * @return ILayoutParam *--布局参数对象
    */
    STDMETHOD_(ILayoutParam *, CreateLayoutParam)(THIS) SCONST PURE;

    /**
     * @brief 计算窗口大小
     * @param pParent const IWindow*--目标窗口
     * @param nWidth int--父窗口宽度
     * @param nHeight int--父窗口高度
     * @return SIZE--目标窗口大小
    */
    STDMETHOD_(SIZE, MeasureChildren)
    (THIS_ const IWindow *pParent, int nWidth, int nHeight) SCONST PURE;
};

SNSEND