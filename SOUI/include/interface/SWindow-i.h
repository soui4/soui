#pragma once

#include <interface/sobject-i.h>
#include <interface/sstring-i.h>
#include <interface/SAnimation-i.h>
#include <interface/SRender-i.h>
#include <interface/SMatrix-i.h>
#include <interface/SWndContainer-i.h>
SNSBEGIN

typedef struct ILayout ILayout;
typedef struct ILayoutParam ILayoutParam;

#undef INTERFACE
#define INTERFACE IWindow
DECLARE_INTERFACE_(IWindow, IObject)
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

    //////////////////////////////////////////////////////////////////////////

    /**
     * @brief 查询控件接口
     * @param id REFGUID--控件接口ID
     * @param [out] ppRet IObjRef * *--控件接口
     * @return S_OK--成功
     */
    STDMETHOD_(HRESULT, QueryInterface)(THIS_ REFGUID id, IObjRef * *ppRet) PURE;

    /**
     * @brief 获取控件容器接口
     * @return ISwndContainer *--控件容器接口
     */
    STDMETHOD_(ISwndContainer *, GetContainer)(THIS) PURE;

    /**
     * @brief 设置控件容器接口
     * @param pContainer ISwndContainer *--控件容器接口
     * @return
     * @remark 内部调用
     */
    STDMETHOD_(void, SetContainer)(THIS_ ISwndContainer * pContainer) PURE;

    /**
     * @brief 获取SOUI容器句柄
     * @return SWND--SOUI容器句柄
     */
    STDMETHOD_(SWND, GetSwnd)(THIS) SCONST PURE;

    /**
     * @brief 获取布局对象
     * @return ILayout *--布局对象
     */
    STDMETHOD_(ILayout *, GetLayout)(THIS) PURE;

    /**
     * @brief 获取布局参数对象
     * @return ILayoutParam *--布局参数对象
     */
    STDMETHOD_(ILayoutParam *, GetLayoutParam)(THIS) SCONST PURE;

    /**
     * @brief 设置布局参数对象
     * @param pLayoutParam ILayoutParam *--布局参数对象
     * @return TRUE--成功，FALSE--失败
     * @remark 只有布局参数对象和窗口的布局类型匹配才会设置成功
     */
    STDMETHOD_(BOOL, SetLayoutParam)(THIS_ ILayoutParam * pLayoutParam) PURE;

    /**
     * @brief 获取当前窗口参与自动布局标志
     * @return TRUE--参与布局,FALSE--不参与布局，需要用户在代码中手动布局
     */
    STDMETHOD_(BOOL, IsFloat)(THIS) SCONST PURE;

    /**
     * @brief 获取隐藏占位标志
     * @return TRUE--窗口隐藏的时候占有该布局位置，FALSE--隐藏时不占位
     */
    STDMETHOD_(BOOL, IsDisplay)(THIS) SCONST PURE;

    /**
     * @brief 获取窗口是否处理鼠标消息
     * @return TRUE--鼠标消息透传给父窗口
     */
    STDMETHOD_(BOOL, IsMsgTransparent)(THIS) SCONST PURE;

    /**
     * @brief 获取窗口是否自动剪裁客户区
     * @return TRUE--自动剪裁客户区
     * @remark 剪裁客户区可以限制窗口及它的子窗口绘制不超过当前窗口客户区，但是要增加一次剪裁操作
     */
    STDMETHOD_(BOOL, IsClipClient)(THIS) SCONST PURE;

    /**
     * @brief 设置tooltip
     * @param pszText LPCTSTR--tooltip字符串
     * @return
     * @remark 支持多语言
     */
    STDMETHOD_(void, SetToolTipText)(THIS_ LPCTSTR pszText) PURE;

    /**
     * @brief 获取窗口的check状态标志
     * @return TRUE--checked
     */
    STDMETHOD_(BOOL, IsChecked)(THIS) SCONST PURE;

    /**
     * @brief 设置窗口check状态
     * @param bCheck BOOL--check状态
     * @return
     */
    STDMETHOD_(void, SetCheck)(THIS_ BOOL bCheck) PURE;

    /**
     * @brief 获取窗口的禁用状态
     * @return TRUE--窗口禁用
     */
    STDMETHOD_(BOOL, IsDisabled)(THIS_ BOOL bCheckParent) SCONST PURE;

    /**
     * @brief 设置窗口启用状态
     * @param bEnable BOOL--启用窗口
     * @param bUpdate BOOL--同时刷新窗口标志
     * @return
     */
    STDMETHOD_(void, EnableWindow)(THIS_ BOOL bEnable, BOOL bUpdate) PURE;

    /**
     * @brief 获取窗口的可见标志
     * @return TRUE--可见
     */
    STDMETHOD_(BOOL, IsVisible)(THIS_ BOOL bCheckParent) SCONST PURE;

    /**
     * @brief 设置窗口可见标志
     * @param bVisible BOOL--可见性
     * @param bUpdate  BOOL--同时刷新窗口标志
     * @return
     */
    STDMETHOD_(void, SetVisible)(THIS_ BOOL bVisible, BOOL bUpdate) PURE;

    /**
     * @brief 获取窗口的用户数据
     * @return ULONG_PTR--用户数据
     */
    STDMETHOD_(ULONG_PTR, GetUserData)(THIS) SCONST PURE;

    /**
     * @brief 设置窗口用户数据
     * @param uData ULONG_PTR--新用户数据
     * @return ULONG_PTR--原用户数据
     */
    STDMETHOD_(ULONG_PTR, SetUserData)(THIS_ ULONG_PTR uData) PURE;

    /**
     * @brief 执行调色
     * @param cr COLORREF--目标色调
     * @return
     */
    STDMETHOD_(void, DoColorize)(THIS_ COLORREF cr) PURE;

    /**
     * @brief 获取当前调色的色调
     * @return COLORREF--色调
     */
    STDMETHOD_(COLORREF, GetColorizeColor)(THIS) SCONST PURE;

    /**
     * @brief 发送一个消息
     * @param uMsg UINT-消息类型
     * @param wParam WPARAM--参数1
     * @param lParam LPARAM--参数2
     * @param [out] pbMsgHandled BOOL*--消息被处理标志
     * @return
     */
    STDMETHOD_(LRESULT, SSendMessage)
    (THIS_ UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL * pbMsgHandled) PURE;

    /**
     * @brief 分发一个消息到当前窗口及它所有子窗口
     * @param uMsg UINT-消息类型
     * @param wParam WPARAM--参数1
     * @param lParam LPARAM--参数2
     * @return
     */
    STDMETHOD_(void, SDispatchMessage)(THIS_ UINT uMsg, WPARAM wParam, LPARAM lParam) PURE;

    /**
     * @brief 获取窗口是否允许设置为焦点
     * @return TRUE--允许
     */
    STDMETHOD_(BOOL, IsFocusable)(THIS) SCONST PURE;

    /**
     * @brief 判断是否为焦点窗口
     * @return TRUE--焦点窗口
     */
    STDMETHOD_(BOOL, IsFocused)(THIS) SCONST PURE;

    /**
     * @brief 将当前窗口设置为焦点窗口
     * @return
     */
    STDMETHOD_(void, SetFocus)(THIS) PURE;

    /**
     * @brief 取消焦点
     * @return
     */
    STDMETHOD_(void, KillFocus)(THIS) PURE;

    /**
     * @brief 刷新当前窗口客户区
     * @return
     */
    STDMETHOD_(void, Invalidate)(THIS) PURE;

    /**
     * @brief 刷新当前窗口指定位置
     * @param lprect LPCRECT--窗口位置
     * @return
     * @remark 可以包含非客户区，但是不刷新窗口外位置
     */
    STDMETHOD_(void, InvalidateRect)(THIS_ LPCRECT lprect) PURE;

    /**
     * @brief 锁住刷新，中断刷新调用
     * @return
     */
    STDMETHOD_(void, LockUpdate)(THIS) PURE;

    /**
     * @brief 解锁刷新，恢复刷新调用
     * @return
     */
    STDMETHOD_(void, UnlockUpdate)(THIS) PURE;

    /**
     * @brief 获取刷新加锁状态
     * @return TRUE--当前刷新为加锁状态
     */
    STDMETHOD_(BOOL, IsUpdateLocked)(THIS) SCONST PURE;

    /**
     * @brief 请求立即更新窗口
     * @return
     * @remark 通常要刷新一个窗口，应该使用Invalidate/InvalidateRect，这样系统中所有请求都会在
     下一次刷新的时候被重新绘制。但是这可能会出现延时。为了能够立即重新绘制，可以调用Update
    */
    STDMETHOD_(void, Update)(THIS) PURE;

    /**
     * @brief 设置窗口显示区域
     * @param pRgn IRegionS*--显示区域
     * @param bRedraw BOOL--重新绘制
     * @return
     * @remark 区域的左上角坐标为(0,0),在显示过程中会根据窗口真实坐标调整位置
     */
    STDMETHOD_(void, SetWindowRgn)(THIS_ IRegionS * pRgn, BOOL bRedraw) PURE;

    /**
     * @brief 获取窗口显示区域
     * @return IRegionS*--显示区域
     */
    STDMETHOD_(IRegionS *, GetWindowRgn)(THIS) SCONST PURE;

    /**
     * @brief 设置窗口显示区域
     * @param pRgn IPathS*--显示区域
     * @param bRedraw BOOL--重新绘制
     * @return
     * @remark 区域的左上角坐标为(0,0),在显示过程中会根据窗口真实坐标调整位置
     */
    STDMETHOD_(void, SetWindowPath)(THIS_ IPathS * pPath, BOOL bRedraw) PURE;

    /**
     * @brief 获取窗口显示区域
     * @return IPathS*--显示区域
     */
    STDMETHOD_(IPathS *, GetWindowPath)(THIS) SCONST PURE;

    /**
     * Move2
     * @brief    将窗口移动到指定位置
     * @param    int x --  left
     * @param    int y --  top
     * @param    int cx --  width
     * @param    int cy --  height
     * @return   void
     *
     * Describe
     * @see     Move(LPRECT prect)
     */
    STDMETHOD_(void, Move2)(THIS_ int x, int y, int cx, int cy) PURE;

    /**
     * @brief 将窗口移动到指定位置
     * @param prect LPCRECT--窗口坐标
     * @return
     * remark 容器在容器中的坐标
     */
    STDMETHOD_(void, Move)(THIS_ LPCRECT prect) PURE;

    /**
     * @brief 获取窗口的显示位置，包含窗口的非客户区
     * @param [out] LPRECT--窗口显示位置
     * @remark SOUI窗口位置左上角不是(0,0),而是宿主窗口中的坐标
     */
    STDMETHOD_(void, GetWindowRect)(THIS_ LPRECT prect) SCONST PURE;

    /**
     * @brief 获取窗口的显示位置，不包含窗口的非客户区
     * @param [out] LPRECT--窗口客户区显示位置
     * @remark SOUI窗口位置左上角不是(0,0),而是宿主窗口中的坐标
     */
    STDMETHOD_(void, GetClientRect)(THIS_ LPRECT prect) SCONST PURE;

    /**
     * @brief 检测一个点是不是在窗口范围内
     * @param pt POINT--被检查坐标
     * @param bClientOnly BOOL--测试客户区标志
     * @return TRUE--在窗口范围内
     */
    STDMETHOD_(BOOL, IsContainPoint)(THIS_ POINT pt, BOOL bClientOnly) SCONST PURE;

    /**
     * @brief 检查pt指向的窗口
     * @param [in,out] pt POINT *--测试坐标
     * @param bIncludeMsgTransparent BOOL--测试消息透传窗口标志
     * @return SWND--包含指定坐标的最顶层窗口
     */
    STDMETHOD_(SWND, SwndFromPoint)(THIS_ POINT * pt, BOOL bIncludeMsgTransparent) SCONST PURE;

    /**
     * @brief 设置容器定时器
     * @param id char--定时器ID，范围从0-127
     * @param uElapse UINT--定时器延时
     * @return TRUE--成功
     */
    STDMETHOD_(BOOL, SetTimer)(THIS_ char id, UINT uElapse) PURE;

    /**
     * @brief 删除SetTimer创建的定时器
     * @param id char--SetTimer创建的定时器ID
     * @return
     */
    STDMETHOD_(void, KillTimer)(THIS_ char id) PURE;

    /**
     * SetTimer2
     * @brief    利用函数定时器来模拟一个兼容窗口定时器
     * @param    UINT_PTR id --  定时器ID
     * @param    UINT uElapse --  延时(MS)
     * @return   BOOL
     *
     * Describe  由于SetTimer只支持0-127的定时器ID，SetTimer2提供设置其它timerid
     *           能够使用SetTimer时尽量不用SetTimer2，在Kill时效率会比较低
     */
    STDMETHOD_(BOOL, SetTimer2)(THIS_ UINT_PTR id, UINT uElapse) PURE;

    /**
     * KillTimer2
     * @brief    删除一个SetTimer2设置的定时器
     * @param    UINT_PTR id --  SetTimer2设置的定时器ID
     * @return   void
     *
     * Describe  需要枚举定时器列表
     */
    STDMETHOD_(void, KillTimer2)(THIS_ UINT_PTR id) PURE;

    /**
     * @brief 获取鼠标捕获
     * @return SWND--鼠标捕获窗口句柄
     */
    STDMETHOD_(SWND, GetCapture)(THIS) SCONST PURE;

    /**
     * @brief 设置容器为鼠标捕获
     * @return SWND--前鼠标捕获窗口句柄
     */
    STDMETHOD_(SWND, SetCapture)(THIS) PURE;

    /**
     * @brief 释放鼠标捕获
     * @return TRUE--释放成功
     */
    STDMETHOD_(BOOL, ReleaseCapture)(THIS) PURE;

    /**
     * @brief 设置一个窗口动画
     * @param animation IAnimation *--动画对象
     * @return
     */
    STDMETHOD_(void, SetAnimation)(THIS_ IAnimation * animation) PURE;

    /**
     * @brief 立即启动一个动画对象
     * @param animation IAnimation *--动画对象
     * @return
     * @remark 和setAnimation不同在于立即启动，而不是等待动画对象中的延时启动时间
     */
    STDMETHOD_(void, StartAnimation)(THIS_ IAnimation * animation) PURE;

    /**
     * @brief 获取当前正在运行的动画对象
     * @return IAnimation *--动画对象
     */
    STDMETHOD_(IAnimation *, GetAnimation)(THIS) SCONST PURE;

    /**
     * @brief 清除动画
     * @return
     */
    STDMETHOD_(void, ClearAnimation)(THIS) PURE;

    /**
     * @brief 设置窗口透明度
     * @param byAlpha BYTE--透明度,[0,255]
     * @return
     */
    STDMETHOD_(void, SetAlpha)(THIS_ BYTE byAlpha) PURE;

    /**
     * @brief 获取窗口透明度
     * @return BYTE--窗口透明度
     */
    STDMETHOD_(BYTE, GetAlpha)(THIS) SCONST PURE;

    /**
     * @brief 设置窗口变换矩阵
     * @param mtx const IMatrix *--变换矩阵
     * @return
     */
    STDMETHOD_(void, SetMatrix)(THIS_ const IMatrix *mtx) PURE;

    /**
     * @brief 获取窗口变换矩阵
     */
    STDMETHOD_(void, GetMatrix)(THIS_ IMatrix * mtx) SCONST PURE;

    /**
     * @brief 获取dpi放大倍数
     * @return int--dpi放大倍数,以100为基数
     */
    STDMETHOD_(int, GetScale)(THIS) SCONST PURE;

    /**
     * @brief 请求重新布局
     * @return
     */
    STDMETHOD_(void, RequestRelayout)(THIS) PURE;

    /**
     * @brief 重新布局当前窗口的子窗口，只在窗口布局脏了才生效
     * @return
     */
    STDMETHOD_(void, UpdateLayout)(THIS) PURE;

    /**
     * @brief 强制重新布局子窗口
     * @return
     */
    STDMETHOD_(void, UpdateChildrenPosition)(THIS) PURE;

    /**
     * @brief 获取布局脏标志
     * @return TRUE--布局脏
     */
    STDMETHOD_(BOOL, IsLayoutDirty)(THIS) SCONST PURE;

    /**
     * @brief 获取窗口响应键盘的标志位
     * @return UINT--响应键盘的标志位
     */
    STDMETHOD_(UINT, OnGetDlgCode)(THIS) SCONST PURE;

    /**
     * @brief 获取根窗口
     * @return IWindow *--根窗口对象
     */
    STDMETHOD_(IWindow *, GetIRoot)(THIS) SCONST PURE;

    /**
     * @brief 获取父窗口
     * @return IWindow *--父窗口对象
     */
    STDMETHOD_(IWindow *, GetIParent)(THIS) SCONST PURE;

    /**
     * @brief 根据uCode获取与当前窗口在dom树上关联的窗口
     * @param uCode UINT--关联类型
     * @return IWindow *--当前窗口在dom树上关联的窗口
     */
    STDMETHOD_(IWindow *, GetIWindow)(THIS_ int uCode) SCONST PURE;

    /**
     * @brief 获取子窗口
     * @param iChild int--子窗口序号
     * @return
     */
    STDMETHOD_(IWindow *, GetIChild)(THIS_ int iChild) SCONST PURE;

    /**
     * @brief 获取子窗口数量
     * @return UINT--子窗口数量
     */
    STDMETHOD_(UINT, GetChildrenCount)(THIS) SCONST PURE;

    /**
     * @brief 判断一个窗口是不是当前窗口的子孙窗口
     * @param pTest const IWindow *--测试窗口
     * @return TRUE--是子孙窗口
     */
    STDMETHOD_(BOOL, IsIDescendant)(THIS_ const IWindow *pTest) SCONST PURE;

    /**
     * @brief 设置窗口的Owner
     * @param pOwner IWindow *--Owner窗口
     * @return
     */
    STDMETHOD_(void, SetIOwner)(THIS_ IWindow * pOwner) PURE;

    /**
     * @brief 获取Owner窗口
     * @return IWindow *--Owner窗口
     */
    STDMETHOD_(IWindow *, GetIOwner)(THIS) SCONST PURE;

    /**
     * @brief 将当前窗口移动到同级兄弟窗口的zorder最顶层
     * @param
     * @return
     */
    STDMETHOD_(void, BringWindowToTop)(THIS) PURE;

    /**
     * AdjustZOrder
     * @brief    调整窗口Z序
     * @param    SWindow *pInsertAfter --  插入在这个窗口之后
     * @return   bool,  pInsertAfter与this非同级窗口返回失败
     *
     * Describe  pInsertAfter可以为NULL，或是与this同一级的兄弟窗口
     */
    STDMETHOD_(BOOL, AdjustIZOrder)(THIS_ IWindow * pInsertAfter) PURE;

    /**
     * InsertChild
     * @brief    在窗口树中插入一个子窗口
     * @param    SWindow * pNewChild --  子窗口对象
     * @param    SWindow * pInsertAfter --  插入位置
     * @return   void
     *
     * Describe  一般用于UI初始化的时候创建，插入的窗口不会自动进入布局流程
     */
    STDMETHOD_(void, InsertIChild)(THIS_ IWindow * pNewChild, IWindow * pInsertAfter) PURE;

    /**
     * RemoveChild
     * @brief    从窗口树中移除一个子窗口对象
     * @param    SWindow * pChild --  子窗口对象
     * @return   BOOL
     *
     * Describe  子窗口不会自动释放
     */
    STDMETHOD_(BOOL, RemoveIChild)(THIS_ IWindow * pChild) PURE;

    /**
     * CreateChildren
     * @brief    从XML创建子窗口
     * @param    LPCWSTR pszXml --  合法的utf16编码XML字符串
     * @return   BOOL 是否创建成功
     *
     * Describe
     */
    STDMETHOD_(BOOL, CreateChildrenFromXml)(THIS_ LPCWSTR pszXml) PURE;

    /**
     * @brief 根据ID查找子窗口
     * @param nId int--子窗口ID
     * @param nDeep int--查找深度,-1代码无限
     * @return IWindow*--匹配窗口
     * @remark 采用广度优先算法搜索匹配子窗口
     */
    STDMETHOD_(IWindow *, FindIChildByID)(THIS_ int nId, int nDeep) PURE;

    /**
     * @brief 根据Name查找子窗口
     * @param pszName LPCWSTR--子窗口Name
     * @param nDeep int--查找深度,-1代码无限
     * @return IWindow*--匹配窗口
     * @remark 采用广度优先算法搜索匹配子窗口
     */
    STDMETHOD_(IWindow *, FindIChildByName)(THIS_ LPCWSTR pszName, int nDeep) PURE;

    /**
     * DestroyChild
     * @brief    销毁一个子窗口
     * @param    SWindow * pChild --  子窗口对象
     * @return   BOOL
     *
     * Describe  先调用RemoveChild，再调用pChild->Release来释放子窗口对象
     */
    STDMETHOD_(BOOL, DestroyIChild)(THIS_ IWindow * pChild) PURE;

    /**
     * @brief 销毁所有子窗口
     * @return
     */
    STDMETHOD_(void, DestroyAllChildren)(THIS) PURE;

    /**
     * @brief 销毁窗口
     * @return TRUE--成功
     */
    STDMETHOD_(BOOL, Destroy)(THIS) PURE;

    /**
     * @brief 获取指定窗口的下一个参与布局的窗口对象
     * @param pCurChild const IWindow *--当前窗口
     * @return IWindow *--下一个布局窗口
     */
    STDMETHOD_(IWindow *, GetNextLayoutIChild)(THIS_ const IWindow *pCurChild) SCONST PURE;

    /**
     * GetChildrenLayoutRect
     * @brief    获得子窗口的布局空间
     * @return   CRect
     *
     * Describe  通常是客户区，但是tab,group这样的控件不一样
     */
    STDMETHOD_(RECT, GetChildrenLayoutRect)(THIS) SCONST PURE;

    /**
     * GetDesiredSize
     * @brief    当没有指定窗口大小时，通过如皮肤计算窗口的期望大小
     * @param    int nParentWid -- 容器宽度，<0代表容器宽度依赖当前窗口宽度
     * @param    int nParentHei -- 容器高度，<0代表容器高度依赖当前窗口高度
     * @return   CSize
     *
     * Describe
     */
    STDMETHOD_(SIZE, GetDesiredSize)(THIS_ int nParentWid, int nParentHei) PURE;

    /**
     * @brief 获取窗口背景色
     * @return COLORREF--窗口背景色
     */
    STDMETHOD_(COLORREF, GetBkgndColor)(THIS) SCONST PURE;

    /**
     * @brief 设计窗口显示文本
     * @param lpszText LPCTSTR--窗口显示文本
     * @return
     * @remark 支持自动翻译
     */
    STDMETHOD_(void, SetWindowText)(THIS_ LPCTSTR lpszText) PURE;

    /**
     * @brief 获取窗口显示文本
     * @param pBuf TCHAR*--缓冲区
     * @param nBufLen int--缓冲区长度
     * @param bRawText BOOL--获取翻译前文本标志
     * @return int--复制的文本长度，pBuf为NULL时返回总缓冲区长度
     */
    STDMETHOD_(int, GetWindowText)(THIS_ TCHAR * pBuf, int nBufLen, BOOL bRawText) PURE;

    /**
     * @brief 获取窗口状态
     * @return DWORD -- 窗口状态
     */
    STDMETHOD_(DWORD, GetState)(THIS) SCONST PURE;

    /**
     * @brief 修改窗口状态
     * @param dwStateAdd DWORD--增加状态位
     * @param dwStateRemove DWORD--删除状态位
     * @param bUpdate BOOL--刷新窗口
     * @return DWORD--新状态
     */
    STDMETHOD_(DWORD, ModifyState)(THIS_ DWORD dwStateAdd, DWORD dwStateRemove, BOOL bUpdate) PURE;

    /**
     * GetISelectedSiblingInGroup
     * @brief    获得在一个group中选中状态的窗口
     * @return   SWindow *
     *
     * Describe  不是group中的窗口时返回NULL
     */
    STDMETHOD_(IWindow *, GetISelectedSiblingInGroup)(THIS) PURE;

    /**
     * GetSelectedChildInGroup
     * @brief    获取有选择状态的子窗口
     * @return   IWindow * -- 选中状态窗口
     * Describe
     */
    STDMETHOD_(IWindow *, GetISelectedChildInGroup)(THIS) PURE;

    /**
     * @brief 同类型兄弟窗口自动成组标志
     * @return TRUE--自动成组
     * @remark 自动成组的窗口在用户按钮方向键时，组内窗口循环顺序获得焦点
     */
    STDMETHOD_(BOOL, IsSiblingsAutoGroupped)(THIS) SCONST PURE;

    // caret相关方法

    /**
     * @brief 创建光标
     * @param pBmp 创建光标的位图,可为null
     * @param nWid int--光标宽度
     * @param nHeight int--光标高度
     * @return TRUE--成功
     */
    STDMETHOD_(BOOL, CreateCaret)(THIS_ HBITMAP pBmp, int nWid, int nHeight) PURE;

    /**
     * @brief 控制光标显示/隐藏
     * @param bShow BOOL--显示标志
     * @return
     */
    STDMETHOD_(void, ShowCaret)(THIS_ BOOL bShow) PURE;

    /**
     * @brief 设置光标显示位置
     * @param x int-X
     * @param y int-Y
     * @return
     */
    STDMETHOD_(void, SetCaretPos)(THIS_ int x, int y) PURE;

    /**
     * @brief 设置当前控件是否允许发出事件
     * @param bMute BOOL-允许发出事件标志
     * @return
     */
    STDMETHOD_(void, SetEventMute)(THIS_ BOOL bMute) PURE;

    /**
     * @brief 订阅窗口事件
     * @param evtId DWORD--事件ID
     * @param pSlot const IEvtSlot *--事件处理对象
     * @return TRUE--成功
     */
    STDMETHOD_(BOOL, SubscribeEvent)(THIS_ DWORD evtId, const IEvtSlot *pSlot) PURE;

    /**
     * @brief 取消窗口事件订阅
     * @param evtId DWORD--事件ID
     * @param pSlot const IEvtSlot *--事件处理对象
     * @return TRUE--成功
     */
    STDMETHOD_(BOOL, UnsubscribeEvent)(THIS_ DWORD evtId, const IEvtSlot *pSlot) PURE;

    /**
     * @brief  发射一个事件到应用层
     * @param evt IEvtArgs *--事件对象
     * @return TRUE--成功
     */
    STDMETHOD_(BOOL, FireEvent)(THIS_ IEvtArgs * evt) PURE;

    /**
     * FireCommand
     * @brief    激活窗口的EVT_CMD事件
     * @return   BOOL-- true:EVT_CMD事件被处理
     *
     * Describe
     */
    STDMETHOD_(BOOL, FireCommand)(THIS) PURE;

    /**
     * FireCtxMenu
     * @brief    激活快捷菜单事件
     * @param    CPoint pt --  鼠标点击位置
     * @return   BOOL -- true:外部处理了快捷菜单事件
     *
     * Describe
     */
    STDMETHOD_(BOOL, FireCtxMenu)(THIS_ POINT pt) PURE;
};

SNSEND