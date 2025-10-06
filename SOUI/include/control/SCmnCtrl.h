/**
 * @file SCmnCtrl.h
 * @brief 通用控件
 * @version v1.0
 * @author soui
 * @date 2014-05-28
 *
 * @details 此文件中定义了很多通用控件：静态文本、超链接、按钮、单选按钮等。
 */

#ifndef __SCMNCTRL__H__
#define __SCMNCTRL__H__
#include <core/SWnd.h>
#include <core/SAccelerator.h>
#include <core/SFocusManager.h>
#include <interface/SCtrls-i.h>
#include <proxy/SWindowProxy.h>

SNSBEGIN

/**
 * @class SStatic
 * @brief 静态文本控件类
 * @details 静态文本控件可支持多行，有多行属性时，`\n`可以强制换行。
 * @usage `<text>inner text example</text>`
 */
class SOUI_EXP SStatic : public SWindow {
    DEF_SOBJECT(SWindow, L"text")

  public:
    /**
     * @brief 构造函数
     */
    SStatic();

    /**
     * @brief 绘制文本
     * @param pRT 绘制设备句柄
     * @param pszBuf 文本内容字符串
     * @param cchText 字符串长度
     * @param pRect 指向矩形结构RECT的指针
     * @param uFormat 正文的绘制选项
     * @details 对DrawText封装
     */
    virtual void DrawText(IRenderTarget *pRT, LPCTSTR pszBuf, int cchText, LPRECT pRect, UINT uFormat);

  protected:
    /**
     * @brief 绘制单行文本
     * @param pRT 绘制设备句柄
     * @param pszBuf 文本内容字符串
     * @param iBegin 文本起始位置
     * @param cchText 字符串长度
     * @param pRect 指向矩形结构RECT的指针
     * @param uFormat 正文的绘制选项
     */
    virtual void OnDrawLine(IRenderTarget *pRT, LPCTSTR pszBuf, int iBegin, int cchText, LPRECT pRect, UINT uFormat);

    /**
     * @brief 测量文本大小
     * @param pRT 绘制设备句柄
     * @param pszBuf 文本内容字符串
     * @param cchText 字符串长度
     * @return 文本大小
     */
    virtual SIZE OnMeasureText(IRenderTarget *pRT, LPCTSTR pszBuf, int cchText);

    /**
     * @brief 绘制多行文本
     * @param pRT 绘制设备句柄
     * @param pszBuf 文本内容字符串
     * @param cchText 字符串长度
     * @param pRect 指向矩形结构RECT的指针
     * @param uFormat 正文的绘制选项
     */
    void DrawMultiLine(IRenderTarget *pRT, LPCTSTR pszBuf, int cchText, LPRECT pRect, UINT uFormat);

  protected:
    /**
     * @brief 行间距
     */
    int m_nLineInter;

    /**
     * @brief 是否自动换行
     */
    bool m_bWordbreak;

    SOUI_ATTRS_BEGIN()
        ATTR_INT(L"interHeight", m_nLineInter, TRUE)
        ATTR_BOOL(L"wordBreak", m_bWordbreak, TRUE)
    SOUI_ATTRS_END()
};
/**
 * @class SLink
 * @brief 超链接控件类
 * @details 用于显示超链接文本。
 * @usage `<link>inner text example</link>`
 */
class SOUI_EXP SLink : public SWindow {
    DEF_SOBJECT(SWindow, L"link")

  public:
    /**
     * @brief 构造函数
     */
    SLink();

  protected:
    /**
     * @brief 解析XML设置属性
     * @param pNode XML节点
     * @details 根据XML文件设置相关属性
     */
    virtual void WINAPI OnInitFinished(IXmlNode *pNode);

    /**
     * @brief 绘制文本
     * @param pRT 绘制设备句柄
     * @param pszBuf 文本内容字符串
     * @param cchText 字符串长度
     * @param pRect 指向矩形结构RECT的指针
     * @param uFormat 正文的绘制选项
     * @details 对DrawText封装
     */
    virtual void DrawText(IRenderTarget *pRT, LPCTSTR pszBuf, int cchText, LPRECT pRect, UINT uFormat);

    /**
     * @brief 设置光标样式和位置
     * @param pt 设置光标位置
     * @return 成功--TRUE 失败--FALSE
     * @details 函数内部会加载光标样式
     */
    virtual BOOL OnSetCursor(const CPoint &pt);

    /**
     * @brief 处理左键按下事件
     * @param nFlags 标志
     * @param pt 鼠标坐标
     */
    void OnLButtonDown(UINT nFlags, CPoint pt);

    /**
     * @brief 处理左键释放事件
     * @param nFlags 标志
     * @param pt 鼠标坐标
     */
    void OnLButtonUp(UINT nFlags, CPoint pt);

    /**
     * @brief 处理鼠标移动事件
     * @param nFlags 标志
     * @param pt 鼠标坐标
     */
    void OnMouseMove(UINT nFlags, CPoint pt);

    /**
     * @brief 处理鼠标悬停事件
     * @param wParam 额外参数
     * @param ptPos 鼠标位置
     */
    void OnMouseHover(WPARAM wParam, CPoint ptPos);

  protected:
    /**
     * @brief 文本显示所在位置
     */
    CRect m_rcText;

    /**
     * @brief 窗口URL
     */
    SStringT m_strLinkUrl;

    SOUI_ATTRS_BEGIN()
        ATTR_STRINGT(L"href", m_strLinkUrl, FALSE)
    SOUI_ATTRS_END()

    SOUI_MSG_MAP_BEGIN()
        MSG_WM_LBUTTONDOWN(OnLButtonDown)
        MSG_WM_LBUTTONUP(OnLButtonUp)
        MSG_WM_MOUSEMOVE(OnMouseMove)
        MSG_WM_MOUSEHOVER(OnMouseHover)
    SOUI_MSG_MAP_END()
};

/**
 * @class SButton
 * @brief 按钮控件类
 * @details 通过属性ID绑定click事件。使用id属性来处理点击事件。
 * @usage `<button id=xx>inner text example</button>`
 */
class SOUI_EXP SButton
    : public SWindow
    , public IAcceleratorTarget
    , public ITimelineHandler {
    DEF_SOBJECT(SWindow, L"button")

  public:
    /**
     * @brief 构造函数
     */
    SButton();

  protected:
    /**
     * @brief 状态变化需要重画
     * @return 成功--TRUE 失败--FALSE
     * @details 当按钮状态发生变化时需要重新绘制，默认返回TRUE
     */
    virtual BOOL NeedRedrawWhenStateChange()
    {
        return TRUE;
    }

    /**
     * @brief 获取编码
     * @return 返回宏定义SC_WANTCHARS代表需要WM_CHAR消息
     */
    virtual UINT WINAPI OnGetDlgCode() const
    {
        return SC_WANTCHARS;
    }

    /**
     * @brief 处理加速键按下事件
     * @param accelerator 加速键相关结构体
     * @return 成功--TRUE 失败--FALSE
     * @details 处理加速键响应消息
     */
    virtual BOOL WINAPI OnAcceleratorPressed(const IAccelerator *accelerator) OVERRIDE;

    /**
     * @brief 从XML初始化控件
     * @param pNode XML节点
     * @return 成功--TRUE 失败--FALSE
     */
    virtual BOOL WINAPI InitFromXml(IXmlNode *pNode) OVERRIDE;

  protected:
    /**
     * @brief 状态改变处理函数
     * @param dwOldState 旧状态
     * @param dwNewState 新状态
     */
    virtual void OnStateChanged(DWORD dwOldState, DWORD dwNewState);

    /**
     * @brief 容器改变处理函数
     * @param pOldContainer 旧容器
     * @param pNewContainer 新容器
     */
    virtual void OnContainerChanged(ISwndContainer *pOldContainer, ISwndContainer *pNewContainer);

    /**
     * @brief 绘制控件
     * @param pRT 绘制设备句柄
     */
    void OnPaint(IRenderTarget *pRT);

    /**
     * @brief 处理按键按下事件
     * @param nChar 字符
     * @param nRepCnt 重复次数
     * @param nFlags 标志
     */
    void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);

    /**
     * @brief 处理按键释放事件
     * @param nChar 字符
     * @param nRepCnt 重复次数
     * @param nFlags 标志
     */
    void OnKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags);

    /**
     * @brief 处理销毁事件
     */
    void OnDestroy();

    /**
     * @brief 处理大小改变事件
     * @param nType 类型
     * @param size 新大小
     */
    void OnSize(UINT nType, CSize size);

    /**
     * @brief 处理擦除背景事件
     * @param pRT 绘制设备句柄
     * @return 成功--TRUE 失败--FALSE
     */
    BOOL OnEraseBkgnd(IRenderTarget *pRT)
    {
        return TRUE;
    }

    /**
     * @brief 处理自定义属性accel
     * @param strAccel 加速键字符串
     * @param bLoading 是否加载中
     * @return 成功--TRUE 失败--FALSE
     */
    HRESULT OnAttrAccel(SStringW strAccel, BOOL bLoading);

  protected:
    /**
     * @brief 处理下一帧事件
     */
    STDMETHOD_(void, OnNextFrame)(THIS_) OVERRIDE;

    /**
     * @brief 停止动画
     */
    void StopCurAnimate();

    /**
     * @brief 加速键
     */
    DWORD m_accel;

    /**
     * @brief 动画标志
     */
    BOOL m_bAnimate;

    /**
     * @brief 动画状态
     */
    WORD m_byAlphaAni;

    /**
     * @brief alpha for animate step
     */
    BYTE m_nAniStep;

    /**
     * @brief 禁用不可见时的加速键
     */
    BOOL m_bDisableAccelIfInvisible;

    SOUI_ATTRS_BEGIN()
        ATTR_CUSTOM(L"accel", OnAttrAccel)
        ATTR_BOOL(L"animate", m_bAnimate, FALSE)
        ATTR_INT(L"animateStep", m_nAniStep, FALSE)
        ATTR_BOOL(L"disableAccelIfInvisible", m_bDisableAccelIfInvisible, FALSE)
    SOUI_ATTRS_END()

    SOUI_MSG_MAP_BEGIN()
        MSG_WM_PAINT_EX(OnPaint)
        MSG_WM_ERASEBKGND_EX(OnEraseBkgnd)
        MSG_WM_LBUTTONDBLCLK(OnLButtonDown) // 将双击消息处理为单击
        MSG_WM_KEYDOWN(OnKeyDown)
        MSG_WM_KEYUP(OnKeyUp)
        MSG_WM_DESTROY(OnDestroy)
        MSG_WM_SIZE(OnSize)
    SOUI_MSG_MAP_END()
};

/**
 * @class SImageButton
 * @brief 图片按钮类
 * @details 继承自SButton，用于显示图片按钮。
 */
class SOUI_EXP SImageButton : public SButton {
    DEF_SOBJECT(SButton, L"imgbtn")

  public:
    /**
     * @brief 构造函数
     */
    SImageButton();

  protected:
    /**
     * @brief 测量内容大小
     * @param wid 父容器宽度
     * @param hei 父容器高度
     * @return 内容大小
     */
    SIZE MeasureContent(int wid, int hei) override;
};

/**
 * @class SImageWnd
 * @brief 图片控件类
 * @details 用于显示图片。
 * @usage `<img skin="skin" sub="0"/>`
 */
class SOUI_EXP SImageWnd : public TWindowProxy<IImageWnd> {
    DEF_SOBJECT(SWindow, L"img")

  public:
    /**
     * @brief 构造函数
     */
    SImageWnd();

    /**
     * @brief 析构函数
     */
    virtual ~SImageWnd();

  public:
    /**
     * @brief 设置skin
     * @param pSkin skin对象
     * @param iFrame skin的子图索引
     * @param bAutoFree 控件管理pSkin标志，为TRUE时调用方可以释放该Skin
     * @return 成功--TRUE 失败--FALSE
     */
    STDMETHOD_(BOOL, SetSkin)(THIS_ ISkinObj *pSkin, int iFrame DEF_VAL(0), BOOL bAutoFree DEF_VAL(TRUE)) OVERRIDE;

    /**
     * @brief 获取资源
     * @return ISkinObj指针
     */
    STDMETHOD_(ISkinObj *, GetSkin)(THIS) OVERRIDE;

    /**
     * @brief 设置绘制图片
     * @param pBitmap 图片对象
     * @param fl FilterLevel
     */
    STDMETHOD_(void, SetImage)(THIS_ IBitmapS *pBitmap, FilterLevel fl DEF_VAL(kNone_FilterLevel)) OVERRIDE;

    /**
     * @brief 获取当前设置的IBitmapS对象
     * @return 当前设置的IBitmapS对象
     */
    STDMETHOD_(IBitmapS *, GetImage)(THIS) OVERRIDE;

    /**
     * @brief 设置图标
     * @param nSubID 子图在Skin中的索引号
     * @return 成功--TRUE 失败--FALSE
     */
    STDMETHOD_(BOOL, SetIcon)(THIS_ int nSubID) OVERRIDE;

  protected:
    /**
     * @brief 处理颜色化事件
     * @param cr 颜色
     */
    virtual void OnColorize(COLORREF cr);

    /**
     * @brief 处理缩放变化事件
     * @param scale 缩放比例
     */
    virtual void OnScaleChanged(int scale);

    /**
     * @brief 测量内容大小
     * @param nParentWid 父容器宽度
     * @param nParentHei 父容器高度
     * @return 内容大小
     */
    virtual SIZE MeasureContent(int nParentWid, int nParentHei);

  protected:
    /**
     * @brief 绘制是否平铺, 0--位伸（默认），1--不变常规绘制, 2--平铺
     */
    int m_iTile;

    /**
     * @brief 是否要自动释放当前的m_pSkin对象
     */
    BOOL m_bManaged;

    /**
     * @brief 绘制状态索引
     */
    int m_iIcon;

    /**
     * @brief ISkinObj对象
     */
    SAutoRefPtr<ISkinObj> m_pSkin;

    /**
     * @brief 使用代码设定的图片
     */
    SAutoRefPtr<IBitmapS> m_pImg;

    /**
     * @brief 绘制图片的放大精度
     */
    FilterLevel m_fl;

    /**
     * @brief 保持纵横比
     */
    bool m_bKeepAspect;

    SOUI_ATTRS_BEGIN()
        ATTR_SKIN(L"skin", m_pSkin, TRUE)
        ATTR_INT(L"iconIndex", m_iIcon, FALSE)
        ATTR_INT(L"tile", m_iTile, TRUE)
        ATTR_BOOL(L"keepAspect", m_bKeepAspect, TRUE)
    SOUI_ATTRS_END()

  protected:
    /**
     * @brief 绘制控件
     * @param pRT 绘制设备句柄
     */
    void OnPaint(IRenderTarget *pRT);

    SOUI_MSG_MAP_BEGIN()
        MSG_WM_PAINT_EX(OnPaint)
    SOUI_MSG_MAP_END()
};

/**
 * @class SAnimateImgWnd
 * @brief 动画图片窗口
 * @details 此窗口支持动画效果。
 */
class SOUI_EXP SAnimateImgWnd
    : public TWindowProxy<IAnimateImgWnd>
    , public ITimelineHandler {
    DEF_SOBJECT(SWindow, L"animateimg")

  public:
    /**
     * @brief 构造函数
     */
    SAnimateImgWnd();

    /**
     * @brief 析构函数
     */
    virtual ~SAnimateImgWnd()
    {
    }

    /**
     * @brief 启动动画
     */
    void WINAPI Start();

    /**
     * @brief 停止动画
     */
    void WINAPI Stop();

    /**
     * @brief 判断动画运行状态
     * @return 运行中--TRUE
     */
    BOOL WINAPI IsPlaying() SCONST
    {
        return m_bPlaying;
    }

  protected:
    /**
     * @brief 测量内容大小
     * @param wid 父容器宽度
     * @param hei 父容器高度
     * @return 内容大小
     */
    virtual SIZE MeasureContent(int wid, int hei);

    /**
     * @brief 处理下一帧事件
     */
    STDMETHOD_(void, OnNextFrame)(THIS_) OVERRIDE;

    /**
     * @brief 处理颜色化事件
     * @param cr 颜色
     */
    virtual void OnColorize(COLORREF cr);

    /**
     * @brief 容器改变处理函数
     * @param pOldContainer 旧容器
     * @param pNewContainer 新容器
     */
    virtual void OnContainerChanged(ISwndContainer *pOldContainer, ISwndContainer *pNewContainer);

    /**
     * @brief 处理缩放变化事件
     * @param scale 缩放比例
     */
    void OnScaleChanged(int scale) override;

    /**
     * @brief 绘制控件
     * @param pRT 绘制设备句柄
     */
    void OnPaint(IRenderTarget *pRT);

    /**
     * @brief 处理显示窗口事件
     * @param bShow 显示状态
     * @param nStatus 状态
     */
    void OnShowWindow(BOOL bShow, UINT nStatus);

    /**
     * @brief 处理销毁事件
     */
    void OnDestroy();

    SOUI_MSG_MAP_BEGIN()
        MSG_WM_PAINT_EX(OnPaint)
        MSG_WM_DESTROY(OnDestroy)
        MSG_WM_SHOWWINDOW(OnShowWindow)
    SOUI_MSG_MAP_END()

    SOUI_ATTRS_BEGIN()
        ATTR_SKIN(L"skin", m_pSkin, TRUE)
        ATTR_UINT(L"speed", m_nSpeed, FALSE)
        ATTR_BOOL(L"autoStart", m_bAutoStart, FALSE)
        ATTR_INT(L"repeat", m_nRepeat, FALSE)
    SOUI_ATTRS_END()

  protected:
    /**
     * @brief 动画图片
     */
    SAutoRefPtr<ISkinObj> m_pSkin;

    /**
     * @brief 速度
     */
    int m_nSpeed;

    /**
     * @brief 当前帧
     */
    int m_iCurFrame;

    /**
     * @brief 是否自动启动
     */
    BOOL m_bAutoStart;

    /**
     * @brief 是否运行中
     */
    BOOL m_bPlaying;

    /**
     * @brief OnNextFrame的执行次数
     */
    int m_iTimeFrame;

    /**
     * @brief 播放循环次数,-1代表无限循环
     */
    int m_nRepeat;

    /**
     * @brief 当前播放循环轮次
     */
    int m_iRepeat;
};

/**
 * @class SProgress
 * @brief 进度条类
 * @details 进度条控件。
 * @usage `<progress bgskin=xx posskin=xx min=0 max=100 value=10,showpercent=0/>`
 */
class SOUI_EXP SProgress : public TWindowProxy<IProgress> {
    DEF_SOBJECT(SWindow, L"progress")

  public:
    /**
     * @brief 构造函数
     */
    SProgress();

    /**
     * @brief 设置进度条进度值
     * @param nValue 进度值
     * @return 设置成功--TRUE
     */
    STDMETHOD_(BOOL, SetValue)(THIS_ int nValue) OVERRIDE;

    /**
     * @brief 获取进度值
     * @return 进度值
     */
    STDMETHOD_(int, GetValue)(CTHIS) SCONST OVERRIDE
    {
        return m_nValue;
    }

    /**
     * @brief 设置进度值最小大值
     * @param nMin 进度最小值
     * @param nMax 进度最大值
     */
    STDMETHOD_(void, SetRange)(THIS_ int nMin, int nMax) OVERRIDE;

    /**
     * @brief 获取进度值最小大值
     * @param pMin 进度最小值
     * @param pMax 进度最大值
     */
    STDMETHOD_(void, GetRange)(CTHIS_ int *pMin, int *pMax) SCONST OVERRIDE;

    /**
     * @brief 判断进度条是否为竖直状态
     * @return 竖直状态--TRUE
     */
    STDMETHOD_(BOOL, IsVertical)(CTHIS) SCONST OVERRIDE
    {
        return m_bVertical;
    }

  protected:
    /**
     * @brief 获取预期大小
     * @param psz 输出大小
     * @param wid 父容器宽度
     * @param hei 父容器高度
     */
    STDMETHOD_(void, GetDesiredSize)(THIS_ SIZE *psz, int wid, int hei) OVERRIDE;

    /**
     * @brief 处理颜色化事件
     * @param cr 颜色
     */
    virtual void OnColorize(COLORREF cr);

    /**
     * @brief 处理缩放变化事件
     * @param scale 缩放比例
     */
    virtual void OnScaleChanged(int scale);

    /**
     * @brief 绘制控件
     * @param pRT 绘制设备句柄
     */
    void OnPaint(IRenderTarget *pRT);

    /**
     * @brief 处理创建事件
     * @param lp 创建参数
     * @return 成功--0
     */
    int OnCreate(void *);

    /**
     * @brief 进度最小值
     */
    int m_nMinValue;

    /**
     * @brief 进度最大值
     */
    int m_nMaxValue;

    /**
     * @brief 进度值
     */
    int m_nValue;

    /**
     * @brief 是否显示百分比
     */
    BOOL m_bShowPercent;

    /**
     * @brief 是否竖直状态
     */
    BOOL m_bVertical;

    /**
     * @brief 背景资源
     */
    SAutoRefPtr<ISkinObj> m_pSkinBg;

    /**
     * @brief 前景资源
     */
    SAutoRefPtr<ISkinObj> m_pSkinPos;

    SOUI_MSG_MAP_BEGIN()
        MSG_WM_PAINT_EX(OnPaint)
        MSG_WM_CREATE(OnCreate)
    SOUI_MSG_MAP_END()

    SOUI_ATTRS_BEGIN()
        ATTR_SKIN(L"bkgndSkin", m_pSkinBg, TRUE)
        ATTR_SKIN(L"posSkin", m_pSkinPos, TRUE)
        ATTR_INT(L"min", m_nMinValue, FALSE)
        ATTR_INT(L"max", m_nMaxValue, FALSE)
        ATTR_INT(L"value", m_nValue, FALSE)
        ATTR_BOOL(L"vertical", m_bVertical, FALSE)
        ATTR_BOOL(L"showPercent", m_bShowPercent, FALSE)
    SOUI_ATTRS_END()
};

/**
 * @class SHrLine
 * @brief 线条控件
 * @details 线条控件。
 * @usage `<hr size=1 mode="vert" lineStyle="dash"/>`
 */
class SOUI_EXP SHrLine : public SWindow {
    DEF_SOBJECT(SWindow, L"hr")

  public:
    /**
     * @brief 构造函数
     */
    SHrLine();

    /**
     * @brief 绘制控件
     * @param pRT 绘制设备句柄
     */
    void OnPaint(IRenderTarget *pRT);

  protected:
    /**
     * @brief 线条样式
     */
    int m_nLineStyle;

    /**
     * @brief 线条大小
     */
    int m_nLineSize;

    /**
     * @brief 线条颜色
     */
    COLORREF m_crLine;

    /**
     * @brief 线条模式枚举
     */
    enum HRMODE
    {
        HR_HORZ = 0,
        HR_VERT,
        HR_TILT,
    } m_mode;

    SOUI_ATTRS_BEGIN()
        ATTR_COLOR(L"colorLine", m_crLine, FALSE)
        ATTR_COLOR(L"lineColor", m_crLine, FALSE)
        ATTR_INT(L"lineSize", m_nLineSize, FALSE)
        ATTR_ENUM_BEGIN(L"mode", HRMODE, FALSE)
            ATTR_ENUM_VALUE(L"vertical", HR_VERT)
            ATTR_ENUM_VALUE(L"horizontal", HR_HORZ)
            ATTR_ENUM_VALUE(L"tilt", HR_TILT)
        ATTR_ENUM_END(m_mode)
        ATTR_ENUM_BEGIN(L"lineStyle", int, FALSE)
            ATTR_ENUM_VALUE(L"solid", PS_SOLID)           // default
            ATTR_ENUM_VALUE(L"dash", PS_DASH)             /* -------  */
            ATTR_ENUM_VALUE(L"dot", PS_DOT)               /* .......  */
            ATTR_ENUM_VALUE(L"dashdot", PS_DASHDOT)       /* _._._._  */
            ATTR_ENUM_VALUE(L"dashdotdot", PS_DASHDOTDOT) /* _.._.._  */
        ATTR_ENUM_END(m_nLineStyle)
    SOUI_ATTRS_END()

    SOUI_MSG_MAP_BEGIN()
        MSG_WM_PAINT_EX(OnPaint)
    SOUI_MSG_MAP_END()
};

/**
 * @class SCheckBox
 * @brief 复选框控件类
 * @details 用于实现复选框功能的控件。
 * @usage `<check checked="1">This is a check-box</check>`
 */
class SOUI_EXP SCheckBox : public SWindow {
    DEF_SOBJECT(SWindow, L"check") // 定义对象类型为 "check"

    enum
    {
        CheckBoxSpacing = 4, // 复选框与文本之间的默认间距
    };

  public:
    /**
     * @brief 构造函数
     * @details 初始化复选框控件。
     */
    SCheckBox();

  public:
    /**
     * @brief Retrieves the layout rectangle of the children.
     * @param prc Pointer to the rectangle to receive the layout area.
     */
    STDMETHOD_(void, GetChildrenLayoutRect)(THIS_ RECT *prc) SCONST OVERRIDE;
    /**
     * @brief Retrieves the desired size of the window.
     * @param psz Pointer to the size structure to receive the desired size.
     * @param nParentWid Width of the parent window.
     * @param nParentHei Height of the parent window.
     */

  protected:
    /**
     * @brief 状态图片资源
     * @details 用于绘制复选框的不同状态（如选中、未选中等）。
     */
    SAutoRefPtr<ISkinObj> m_pSkin;

    /**
     * @brief 焦点状态资源
     * @details 当复选框获得焦点时使用的皮肤资源。
     */
    SAutoRefPtr<ISkinObj> m_pFocusSkin;

    int m_nCheckBoxSpacing; /** < 复选框与文本之间的间距 */

    /**
     * @brief 获取复选框矩形区域
     * @return 返回复选框的显示位置矩形 (CRect)。
     */
    CRect GetCheckRect();

    /**
     * @brief 测量内容所需的大小
     * @param wid 父容器宽度
     * @param hei 父容器高度
     * @return 返回控件所需的内容大小 (SIZE)。
     */
    SIZE MeasureContent(int wid, int hei) override;
    SIZE MeasureChildren(int wid, int hei) override;

    /**
     * @brief 获取文本显示区域的大小
     * @param pRect 输出参数，表示文本显示区域的矩形。
     */
    virtual void GetTextRect(LPRECT pRect);

    /**
     * @brief 判断状态改变时是否需要重绘控件
     * @return TRUE 表示需要重绘，FALSE 表示不需要重绘。
     */
    virtual BOOL NeedRedrawWhenStateChange()
    {
        return TRUE; // 默认情况下，状态改变时需要重绘
    }

    /**
     * @brief 获取对话框代码
     * @return 返回值为 UINT 类型，表示对话框代码。
     * @details 返回 SC_WANTCHARS 表示控件需要处理字符消息。
     */
    virtual UINT WINAPI OnGetDlgCode() const
    {
        return SC_WANTCHARS;
    }

    /**
     * @brief 绘制焦点样式
     * @param pRT 绘制设备句柄 (IRenderTarget*)。
     * @details 当控件获得焦点时调用此方法绘制焦点样式。
     */
    virtual void DrawFocus(IRenderTarget *pRT);

    /**
     * @brief 处理颜色化事件
     * @param cr 颜色值 (COLORREF)。
     * @details 当控件的颜色发生变化时调用此方法。
     */
    virtual void OnColorize(COLORREF cr);

    /**
     * @brief 处理缩放变化事件
     * @param scale 缩放比例 (int)。
     * @details 当控件的缩放比例发生变化时调用此方法。
     */
    virtual void OnScaleChanged(int scale);

    /**
     * @brief 处理鼠标左键释放事件
     * @param nFlags 标志位 (UINT)。
     * @param pt 鼠标位置 (CPoint)。
     * @details 当用户释放鼠标左键时调用此方法。
     */
    void OnLButtonUp(UINT nFlags, CPoint pt);

    /**
     * @brief 处理按键按下事件
     * @param nChar 字符 (UINT)。
     * @param nRepCnt 重复次数 (UINT)。
     * @param nFlags 标志位 (UINT)。
     * @details 当用户按下键盘按键时调用此方法。
     */
    void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);

    /**
     * @brief 处理绘制事件
     * @param pRT 绘制设备句柄 (IRenderTarget*)。
     * @details 负责绘制控件的外观。
     */
    void OnPaint(IRenderTarget *pRT);

    /**
     * @brief 处理自定义属性 "checked"
     * @param strValue 属性值 (SStringW)。
     * @param bLoading 是否正在加载 (BOOL)。
     * @return 成功返回 S_OK，失败返回 HRESULT 错误码。
     * @details 用于解析和设置 "checked" 属性。
     */
    HRESULT OnAttrCheck(const SStringW &strValue, BOOL bLoading);

    /**
     * @brief 定义控件的属性集合
     */
    SOUI_ATTRS_BEGIN()
        ATTR_SKIN(L"skin", m_pSkin, FALSE)           // 定义皮肤资源属性
        ATTR_SKIN(L"focusSkin", m_pFocusSkin, FALSE) // 定义焦点皮肤资源属性
        ATTR_CUSTOM(L"checked", OnAttrCheck)         // 定义自定义属性 "checked"
        ATTR_INT(L"sep", m_nCheckBoxSpacing, TRUE)   // 定义间距属性
    SOUI_ATTRS_END()

    /**
     * @brief 定义控件的消息映射
     */
    SOUI_MSG_MAP_BEGIN()
        MSG_WM_PAINT_EX(OnPaint)            // 绑定绘制消息
        MSG_WM_LBUTTONDBLCLK(OnLButtonDown) // 绑定鼠标左键双击消息
        MSG_WM_LBUTTONUP(OnLButtonUp)       // 绑定鼠标左键释放消息
        MSG_WM_KEYDOWN(OnKeyDown)           // 绑定按键按下消息
    SOUI_MSG_MAP_END()
};

/**
 * @class SIconWnd
 * @brief 图标控件
 * @details 图标控件。
 * @usage `<icon src="icon:16" />`
 */
class SOUI_EXP SIconWnd : public TWindowProxy<IIconWnd> {
    DEF_SOBJECT(SWindow, L"icon")

  public:
    /**
     * @brief 构造函数
     */
    SIconWnd();

    /**
     * @brief 析构函数
     */
    virtual ~SIconWnd();

    /**
     * @brief 设置图标
     * @param hIcon 图标句柄
     */
    STDMETHOD_(void, SetIcon)(THIS_ HICON hIcon) OVERRIDE;

  protected:
    /**
     * @brief 测量内容大小
     * @param wid 父容器宽度
     * @param hei 父容器高度
     * @return 内容大小
     */
    SIZE MeasureContent(int wid, int hei) override;

    /**
     * @brief 绘制控件
     * @param pRT 绘制设备句柄
     */
    void OnPaint(IRenderTarget *pRT);

    SOUI_MSG_MAP_BEGIN()
        MSG_WM_PAINT_EX(OnPaint)
    SOUI_MSG_MAP_END()

  public:
    SOUI_ATTRS_BEGIN()
        ATTR_CUSTOM(L"src", OnAttrIcon)
    SOUI_ATTRS_END()
  protected:
    HRESULT OnAttrIcon(const SStringW &value, BOOL bLoading);

    void OnScaleChanged(int scale) override;

  protected:
    SStringW m_strIconSrc;
    HICON m_theIcon; /**< 图标资源句柄 */
};

/**
 * @class SRadioBox
 * @brief 单选框控件类
 * @details 用于实现单选框功能的控件。
 * @usage `<radio checked="1">This is a check-box</radio>`
 */
class SOUI_EXP SRadioBox : public SWindow {
    DEF_SOBJECT(SWindow, L"radio") // 定义对象类型为 "radio"

    enum
    {
        RadioBoxSpacing = 4,
    };

  public:
    /**
     * @brief 构造函数
     * @details 初始化单选框控件。
     */
    SRadioBox();

  public:
    /**
     * @brief Retrieves the layout rectangle of the children.
     * @param prc Pointer to the rectangle to receive the layout area.
     */
    STDMETHOD_(void, GetChildrenLayoutRect)(THIS_ RECT *prc) SCONST OVERRIDE;
    /**
     * @brief Retrieves the desired size of the window.
     * @param psz Pointer to the size structure to receive the desired size.
     * @param nParentWid Width of the parent window.
     * @param nParentHei Height of the parent window.
     */

  protected:
    /**
     * @brief 获取单选框显示位置的矩形区域
     * @return 返回单选框的显示位置矩形 (CRect)。
     */
    CRect GetRadioRect();

    /**
     * @brief 获取文本显示区域的大小
     * @param pRect 输出参数，表示文本显示区域的矩形。
     */
    virtual void GetTextRect(LPRECT pRect);

    /**
     * @brief 测量内容所需的大小
     * @param wid 父容器宽度
     * @param hei 父容器高度
     * @return 返回控件所需的内容大小 (SIZE)。
     */
    SIZE MeasureContent(int wid, int hei) OVERRIDE;
    SIZE MeasureChildren(int wid, int hei) OVERRIDE;

    /**
     * @brief 获取对话框代码
     * @return 返回值为 UINT 类型，表示对话框代码。
     * @details 返回 0 表示不处理特定消息。
     */
    STDMETHOD_(UINT, OnGetDlgCode)(THIS) SCONST OVERRIDE
    {
        return 0;
    }

    /**
     * @brief 判断是否自动将相同名称的单选按钮添加到同一组中
     * @return TRUE 表示自动分组，FALSE 表示不自动分组。
     */
    STDMETHOD_(BOOL, IsSiblingsAutoGroupped)(THIS) SCONST OVERRIDE
    {
        return TRUE; // 默认情况下，相同名称的单选按钮会自动分组
    }

    /**
     * @brief 判断状态改变时是否需要重绘控件
     * @return TRUE 表示需要重绘，FALSE 表示不需要重绘。
     */
    virtual BOOL NeedRedrawWhenStateChange();

    /**
     * @brief 绘制焦点样式
     * @param pRT 绘制设备句柄 (IRenderTarget*)。
     * @details 当控件获得焦点时调用此方法绘制焦点样式。
     */
    virtual void DrawFocus(IRenderTarget *pRT);

    /**
     * @brief 获取当前组中被选中的兄弟控件
     * @return 返回指向被选中兄弟控件的指针 (SWindow*)，如果没有则返回 NULL。
     */
    virtual SWindow *GetSelectedSiblingInGroup();

    /**
     * @brief 处理状态变化事件
     * @param dwOldState 原状态 (DWORD)。
     * @param dwNewState 新状态 (DWORD)。
     */
    virtual void OnStateChanging(DWORD dwOldState, DWORD dwNewState);

    /**
     * @brief 处理颜色化事件
     * @param cr 颜色值 (COLORREF)。
     */
    virtual void OnColorize(COLORREF cr);

    /**
     * @brief 处理缩放变化事件
     * @param nScale 缩放比例 (int)。
     */
    virtual void OnScaleChanged(int nScale);

  protected:
    /**
     * @brief 处理焦点改变事件
     * @param wndOld 原窗口句柄 (SWND)。
     * @param reason 焦点改变原因 (SFocusManager::FocusChangeReason)。
     */
    void OnSetFocus(SWND wndOld, SFocusManager::FocusChangeReason reason);

    /**
     * @brief 处理鼠标左键释放事件
     * @param nFlags 标志位 (UINT)。
     * @param pt 鼠标位置 (CPoint)。
     */
    void OnLButtonUp(UINT nFlags, CPoint pt);

    /**
     * @brief 绘制控件
     * @param pRT 绘制设备句柄 (IRenderTarget*)。
     */
    void OnPaint(IRenderTarget *pRT);

    /**
     * @brief 处理自定义属性 "checked"
     * @param strValue 属性值 (SStringW)。
     * @param bLoading 是否正在加载 (BOOL)。
     * @return 成功返回 S_OK，失败返回 HRESULT 错误码。
     */
    HRESULT OnAttrCheck(const SStringW &strValue, BOOL bLoading);

    /**
     * @brief 定义控件的属性集合
     */
    SOUI_ATTRS_BEGIN()
        ATTR_SKIN(L"skin", m_pSkin, FALSE)                      // 定义皮肤资源属性
        ATTR_SKIN(L"focusSkin", m_pFocusSkin, FALSE)            // 定义焦点皮肤资源属性
        ATTR_CUSTOM(L"checked", OnAttrCheck)                    // 定义自定义属性 "checked"
        ATTR_ENUM_BEGIN(L"iconAlign", UINT, TRUE)               // 定义图标对齐方式属性
            ATTR_ENUM_VALUE(L"left", SwndStyle::Align_Left)     // 左对齐
            ATTR_ENUM_VALUE(L"center", SwndStyle::Align_Center) // 居中对齐
            ATTR_ENUM_VALUE(L"right", SwndStyle::Align_Right)   // 右对齐
        ATTR_ENUM_END(m_uIconAlign)
        ATTR_ENUM_BEGIN(L"iconValign", UINT, TRUE)               // 定义图标垂直对齐方式属性
            ATTR_ENUM_VALUE(L"top", SwndStyle::VAlign_Top)       // 顶部对齐
            ATTR_ENUM_VALUE(L"middle", SwndStyle::VAlign_Middle) // 中间对齐
            ATTR_ENUM_VALUE(L"bottom", SwndStyle::VAlign_Bottom) // 底部对齐
        ATTR_ENUM_END(m_uIconVAlign)
        ATTR_INT(L"sep", m_nRadioBoxSpacing, TRUE) // 定义间距属性
    SOUI_ATTRS_END()

    /**
     * @brief 定义控件的消息映射
     */
    SOUI_MSG_MAP_BEGIN()
        MSG_WM_PAINT_EX(OnPaint)        // 绑定绘制消息
        MSG_WM_LBUTTONUP(OnLButtonUp)   // 绑定鼠标左键释放消息
        MSG_WM_SETFOCUS_EX2(OnSetFocus) // 绑定焦点改变消息
    SOUI_MSG_MAP_END()

  protected:
    /**
     * @brief 皮肤资源
     */
    SAutoRefPtr<ISkinObj> m_pSkin;

    /**
     * @brief 焦点皮肤资源
     */
    SAutoRefPtr<ISkinObj> m_pFocusSkin;

    /**
     * @brief 图标水平对齐方式
     */
    UINT m_uIconAlign;

    /**
     * @brief 图标垂直对齐方式
     */
    UINT m_uIconVAlign;

    /**
     * @brief 单选框与文本之间的间距
     */
    int m_nRadioBoxSpacing;
};

/**
 * @class SRadioGroup
 * @brief 单选按钮组控件
 * @details 用于管理一组单选按钮，确保同一组中的单选按钮互斥。
 */
class SOUI_EXP SRadioGroup : public SWindow {
    DEF_SOBJECT(SWindow, L"radioGroup")

  public:
    /**
     * @brief 构造函数
     */
    SRadioGroup();

    /**
     * @brief 检查指定ID的单选按钮
     * @param nID 单选按钮ID
     * @return 成功--TRUE 失败--FALSE
     */
    BOOL Check(int nID);

    /**
     * @brief 检查指定名称的单选按钮
     * @param pszName 单选按钮名称
     * @return 成功--TRUE 失败--FALSE
     */
    BOOL Check(LPCTSTR pszName);

    /**
     * @brief 清除所有选中的单选按钮
     * @return 成功--TRUE 失败--FALSE
     */
    BOOL ClearCheck();

  protected:
    /**
     * @brief 触发事件
     * @param evt 事件参数
     * @return 成功--TRUE 失败--FALSE
     */
    STDMETHOD_(BOOL, FireEvent)(THIS_ IEvtArgs *evt) OVERRIDE;

    /**
     * @brief 在插入子控件后处理
     * @param pChild 子控件指针
     */
    virtual void OnAfterInsertChild(SWindow *pChild);

    /**
     * @brief 在移除子控件前处理
     * @param pChild 子控件指针
     */
    virtual void OnBeforeRemoveChild(SWindow *pChild);
};

/**
 * @class SToggle
 * @brief Toggle控件
 * @details Toggle控件，继承自SCheckBox，用于实现开关效果。
 */
class SOUI_EXP SToggle : public SCheckBox {
    DEF_SOBJECT(SCheckBox, L"toggle")

  public:
    /**
     * @brief 构造函数
     */
    SToggle();

    /**
     * @brief 设置Toggle属性
     * @param bToggle 是否启用Toggle特效
     * @param bUpdate 是否更新，默认值TRUE
     */
    void SetToggle(BOOL bToggle, BOOL bUpdate = TRUE);

    /**
     * @brief 获取Toggle属性
     * @return 返回值BOOL
     * @details 获取Toggle属性，主要用于获取是否启用Toggle特效
     */
    BOOL GetToggle();

  protected:
    /**
     * @brief 测量内容大小
     * @param nParentWid 父容器宽度
     * @param nParentHei 父容器高度
     * @return 内容大小
     */
    SIZE MeasureContent(int nParentWid, int nParentHei) override;

    /**
     * @brief 状态改变是否需要重画
     * @return 需要重画--TRUE
     */
    virtual BOOL NeedRedrawWhenStateChange()
    {
        return TRUE;
    }

  protected:
    /**
     * @brief 绘制控件
     * @param pRT 绘制设备句柄
     */
    void OnPaint(IRenderTarget *pRT);

    SOUI_MSG_MAP_BEGIN()
        MSG_WM_PAINT_EX(OnPaint)
    SOUI_MSG_MAP_END()
};

/**
 * @class SGroup
 * @brief 组控件
 * @details 组控件，用于将多个控件分组显示。
 * @usage `<group colorLine1="#b8d5e2" colorLine2="#999999">group text</group>`
 */
class SOUI_EXP SGroup : public SWindow {
    DEF_SOBJECT(SWindow, L"group")

  public:
    /**
     * @brief 构造函数
     */
    SGroup();

  protected:
    /**
     * @brief 获取子控件布局矩形
     * @param prc 输出矩形
     */
    STDMETHOD_(void, GetChildrenLayoutRect)(THIS_ RECT *prc) SCONST OVERRIDE;

    /**
     * @brief 获取预期大小
     * @param psz 输出大小
     * @param wid 父容器宽度
     * @param hei 父容器高度
     */
    STDMETHOD_(void, GetDesiredSize)(THIS_ SIZE *psz, int wid, int hei) OVERRIDE;

    /**
     * @brief 绘制控件
     * @param pRT 绘制设备句柄
     */
    void OnPaint(IRenderTarget *pRT);

  protected:
    /**
     * @brief group 3D显示使用的两种颜色
     */
    COLORREF m_crLine1, m_crLine2;

    /**
     * @brief 圆角半径
     */
    SLayoutSize m_nRound;

    /**
     * @brief 头部高度
     */
    SLayoutSize m_nHeaderHeight;

    SOUI_MSG_MAP_BEGIN()
        MSG_WM_PAINT_EX(OnPaint)
    SOUI_MSG_MAP_END()

    SOUI_ATTRS_BEGIN()
        ATTR_COLOR(L"colorLine1", m_crLine1, FALSE)
        ATTR_COLOR(L"colorLine2", m_crLine2, FALSE)
        ATTR_LAYOUTSIZE(L"round", m_nRound, FALSE)
        ATTR_LAYOUTSIZE(L"headerHeight", m_nHeaderHeight, TRUE)
    SOUI_ATTRS_END()
};

SNSEND

#endif // __SCMNCTRL__H__