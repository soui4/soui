/**
 * @file SCmnCtrl.h
 * @brief Common controls
 * @version v1.0
 * @author soui
 * @date 2014-05-28
 *
 * @details This file defines many common controls: static text, hyperlink, button, radio button, etc.
 */

#ifndef __SCMNCTRL__H__
#define __SCMNCTRL__H__
#include <core/SWnd.h>
#include <core/SAccelerator.h>
#include <core/SFocusManager.h>
#include <interface/SCtrls-i.h>
#include <interface/sinterpolator-i.h>
#include <proxy/SWindowProxy.h>
#include <valueAnimator/SValueAnimator.h>

SNSBEGIN

/**
 * @class SStatic
 * @brief Static text control class
 * @details The static text control supports multiple lines; when the multiline attribute is set, `@n` forces a line break.
 * @usage `<text>inner text example</text>`
 */
class SOUI_EXP SStatic : public SWindow {
    DEF_SOBJECT(SWindow, L"text")

  public:
    /**
     * @brief Constructor
     */
    SStatic();

    /**
     * @brief Draw text
     * @param pRT render device handle
     * @param pszBuf text content string
     * @param cchText string length
     * @param pRect pointer to RECT rectangle structure
     * @param uFormat text drawing options
     * @details Wrapper around DrawText
     */
    virtual void DrawText(IRenderTarget *pRT, LPCTSTR pszBuf, int cchText, LPRECT pRect, UINT uFormat);

  protected:
    /**
     * @brief Draw single-line text
     * @param pRT render device handle
     * @param pszBuf text content string
     * @param iBegin text start position
     * @param cchText string length
     * @param pRect pointer to RECT rectangle structure
     * @param uFormat text drawing options
     */
    virtual void OnDrawLine(IRenderTarget *pRT, LPCTSTR pszBuf, int iBegin, int cchText, LPRECT pRect, UINT uFormat);

    /**
     * @brief Measure text size
     * @param pRT render device handle
     * @param pszBuf text content string
     * @param cchText string length
     * @return text size
     */
    virtual SIZE OnMeasureText(IRenderTarget *pRT, LPCTSTR pszBuf, int cchText);

    /**
     * @brief Draw multi-line text
     * @param pRT render device handle
     * @param pszBuf text content string
     * @param cchText string length
     * @param pRect pointer to RECT rectangle structure
     * @param uFormat text drawing options
     */
    void DrawMultiLine(IRenderTarget *pRT, LPCTSTR pszBuf, int cchText, LPRECT pRect, UINT uFormat);

  protected:
    /**
     * @brief Line spacing
     */
    int m_nLineInter;

    /**
     * @brief Whether to wrap automatically
     */
    bool m_bWordbreak;

    /**
     * @brief Whether to hide prefix
     */
    bool m_bNoPrefix;

    SOUI_ATTRS_BEGIN()
        ATTR_INT(L"interHeight", m_nLineInter, TRUE)
        ATTR_BOOL(L"wordBreak", m_bWordbreak, TRUE)
        ATTR_BOOL(L"noPrefix", m_bNoPrefix, TRUE)
    SOUI_ATTRS_END()
};
/**
 * @class SLink
 * @brief Hyperlink control class
 * @details Used to display hyperlink text.
 * @usage `<link>inner text example</link>`
 */
class SOUI_EXP SLink : public SWindow {
    DEF_SOBJECT(SWindow, L"link")

  public:
    /**
     * @brief Constructor
     */
    SLink();

  protected:
    /**
     * @brief Parse XML to set attributes
     * @param pNode XML node
     * @details Set related attributes according to the XML file
     */
    virtual void WINAPI OnInitFinished(IXmlNode *pNode);

    /**
     * @brief Draw text
     * @param pRT render device handle
     * @param pszBuf text content string
     * @param cchText string length
     * @param pRect pointer to RECT rectangle structure
     * @param uFormat text drawing options
     * @details Wrapper around DrawText
     */
    virtual void DrawText(IRenderTarget *pRT, LPCTSTR pszBuf, int cchText, LPRECT pRect, UINT uFormat);

    /**
     * @brief Set cursor style and position
     * @param pt cursor position to set
     * @return success--TRUE, failure--FALSE
     * @details The function loads the cursor style internally
     */
    virtual BOOL OnSetCursor(const CPoint &pt);

    /**
     * @brief Handle left button down event
     * @param nFlags flags
     * @param pt mouse position
     */
    void OnLButtonDown(UINT nFlags, CPoint pt);

    /**
     * @brief Handle left button up event
     * @param nFlags flags
     * @param pt mouse position
     */
    void OnLButtonUp(UINT nFlags, CPoint pt);

    /**
     * @brief Handle mouse move event
     * @param nFlags flags
     * @param pt mouse position
     */
    void OnMouseMove(UINT nFlags, CPoint pt);

    /**
     * @brief Handle mouse hover event
     * @param wParam extra parameter
     * @param ptPos mouse position
     */
    void OnMouseHover(WPARAM wParam, CPoint ptPos);

  protected:
    /**
     * @brief Text display location
     */
    CRect m_rcText;

    /**
     * @brief Window URL
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
 * @brief Button control class
 * @details The click event is bound via the attribute ID. The id attribute is used to handle click events.
 * @usage `<button id=xx>inner text example</button>`
 */
class SOUI_EXP SButton
    : public SWindow
    , public IAcceleratorTarget
    , public IAnimatorUpdateListener {
    DEF_SOBJECT(SWindow, L"button")

  public:
    /**
     * @brief Constructor
     */
    SButton();

    /**
     * @brief Get accelerator
     * @return accelerator
     */
    DWORD GetAccel() const
    {
        return m_accel;
    }

  protected:
    /**
     * @brief State change requires redraw
     * @return success--TRUE, failure--FALSE
     * @details When the button state changes, it needs to be redrawn; returns TRUE by default
     */
    virtual BOOL NeedRedrawWhenStateChange() OVERRIDE
    {
        return TRUE;
    }

    /**
     * @brief Get encoding
     * @return Returns the macro SC_WANTCHARS indicating WM_CHAR message is needed
     */
    virtual UINT WINAPI OnGetDlgCode() const OVERRIDE
    {
        return SC_WANTCHARS;
    }

    /**
     * @brief Handle accelerator key down event
     * @param accelerator accelerator-related structure
     * @return success--TRUE, failure--FALSE
     * @details Handle accelerator response message
     */
    virtual BOOL WINAPI OnAcceleratorPressed(const IAccelerator *accelerator) OVERRIDE;

    /**
     * @brief Initialize control from XML
     * @param pNode XML node
     * @return success--TRUE, failure--FALSE
     */
    virtual BOOL WINAPI InitFromXml(IXmlNode *pNode) OVERRIDE;

  protected:
    /**
     * @brief State change handler
     * @param dwOldState old state
     * @param dwNewState new state
     */
    virtual void OnStateChanged(DWORD dwOldState, DWORD dwNewState) OVERRIDE;

    /**
     * @brief Container change handler
     * @param pOldContainer old container
     * @param pNewContainer new container
     */
    virtual void OnContainerChanged(ISwndContainer *pOldContainer, ISwndContainer *pNewContainer) OVERRIDE;

    /**
     * @brief Draw control
     * @param pRT render device handle
     */
    void OnPaint(IRenderTarget *pRT);

    /**
     * @brief Handle key down event
     * @param nChar character
     * @param nRepCnt repeat count
     * @param nFlags flags
     */
    void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);

    /**
     * @brief Handle key up event
     * @param nChar character
     * @param nRepCnt repeat count
     * @param nFlags flags
     */
    void OnKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags);

    /**
     * @brief Handle destroy event
     */
    void OnDestroy();

    /**
     * @brief Handle size change event
     * @param nType type
     * @param size new size
     */
    void OnSize(UINT nType, CSize size);

    /**
     * @brief Handle erase background event
     * @param pRT render device handle
     * @return success--TRUE, failure--FALSE
     */
    BOOL OnEraseBkgnd(IRenderTarget *pRT)
    {
        return TRUE;
    }

    /**
     * @brief Handle custom attribute accel
     * @param strAccel accelerator string
     * @param bLoading whether loading
     * @return success--TRUE, failure--FALSE
     */
    HRESULT OnAttrAccel(SStringW strAccel, BOOL bLoading);

  protected:
    /**
     * @brief Stop animation
     */
    void StopCurAnimate();

    /**
     * @brief Accelerator
     */
    DWORD m_accel;

    /**
     * @brief Animation flag
     */
    BOOL m_bAnimate;

    /**
     * @brief Current alpha value of hover animation
     */
    BYTE m_byAlphaAni;

    /**
     * @brief Disable accelerator when invisible
     */
    BOOL m_bDisableAccelIfInvisible;

    /**
     * @brief Animation duration (ms)
     */
    int m_nAnimDuration;

    /**
     * @brief Hover state animator (manages alpha from 50-255)
     */
    SAutoRefPtr<SByteAnimator> m_pHoverAni;

  protected:
    /**
     * @brief Handle animation update callback
     */
    STDMETHOD_(void, onAnimationUpdate)(THIS_ IValueAnimator *p) OVERRIDE;

    SOUI_ATTRS_BEGIN()
        ATTR_CUSTOM(L"accel", OnAttrAccel)
        ATTR_BOOL(L"animate", m_bAnimate, FALSE)
        ATTR_CHAIN_PTR(m_pHoverAni, 0)
        ATTR_BOOL(L"disableAccelIfInvisible", m_bDisableAccelIfInvisible, FALSE)
    SOUI_ATTRS_END()

    SOUI_MSG_MAP_BEGIN()
        MSG_WM_PAINT_EX(OnPaint)
        MSG_WM_ERASEBKGND_EX(OnEraseBkgnd)
        MSG_WM_LBUTTONDBLCLK(OnLButtonDown) /**< Treat double-click message as single click */
        MSG_WM_KEYDOWN(OnKeyDown)
        MSG_WM_KEYUP(OnKeyUp)
        MSG_WM_DESTROY(OnDestroy)
        MSG_WM_SIZE(OnSize)
    SOUI_MSG_MAP_END()
};

/**
 * @class SImageButton
 * @brief Image button class
 * @details Inherits from SButton, used to display image buttons.
 */
class SOUI_EXP SImageButton : public SButton {
    DEF_SOBJECT(SButton, L"imgbtn")

  public:
    /**
     * @brief Constructor
     */
    SImageButton();

  protected:
    /**
     * @brief Measure content size
     * @param wid parent container width
     * @param hei parent container height
     * @return content size
     */
    SIZE MeasureContent(int wid, int hei) override;
};

/**
 * @class SImageWnd
 * @brief Image control class
 * @details Used to display images.
 * @usage `<img skin="skin" sub="0"/>`
 */
class SOUI_EXP SImageWnd : public TWindowProxy<IImageWnd> {
    DEF_SOBJECT(SWindow, L"img")

  public:
    /**
     * @brief Constructor
     */
    SImageWnd();

    /**
     * @brief Destructor
     */
    virtual ~SImageWnd();

  public:
    /**
     * @brief Set skin
     * @param pSkin skin object
     * @param iFrame skin sub-image index
     * @param bAutoFree flag indicating whether the control manages pSkin; when TRUE the caller may release the skin
     * @return success--TRUE, failure--FALSE
     */
    STDMETHOD_(BOOL, SetSkin)(THIS_ ISkinObj *pSkin, int iFrame DEF_VAL(0), BOOL bAutoFree DEF_VAL(TRUE)) OVERRIDE;

    /**
     * @brief Get resource
     * @return ISkinObj pointer
     */
    STDMETHOD_(ISkinObj *, GetSkin)(CTHIS) SCONST OVERRIDE;

    /**
     * @brief Set drawing image
     * @param pBitmap image object
     * @param fl FilterLevel
     */
    STDMETHOD_(void, SetImage)(THIS_ IBitmapS *pBitmap, FilterLevel fl DEF_VAL(kNone_FilterLevel)) OVERRIDE;

    /**
     * @brief Get the currently set IBitmapS object
     * @return the currently set IBitmapS object
     */
    STDMETHOD_(IBitmapS *, GetImage)(THIS) OVERRIDE;

    /**
     * @brief Set icon
     * @param nSubID index of sub-image within the skin
     * @return success--TRUE, failure--FALSE
     */
    STDMETHOD_(BOOL, SetIcon)(THIS_ int nSubID) OVERRIDE;

    /**
     * @brief Get icon
     * @return icon index
     */
    STDMETHOD_(int, GetIcon)(CTHIS) SCONST OVERRIDE;

    /**
     * @brief Get desired size
     * @param pSize desired size
     * @param nParentWid parent container width
     * @param nParentHei parent container height
     */
    STDMETHOD_(void, GetDesiredSize)(THIS_ SIZE *pSize, int nParentWid, int nParentHei) OVERRIDE;

    STDMETHOD_(void, SetSvg)(THIS_ ISvgObj *pSvg) OVERRIDE;

    STDMETHOD_(ISvgObj *, GetSvg)(CTHIS) SCONST OVERRIDE;

  protected:
    /**
     * @brief Handle colorize event
     * @param cr color
     */
    virtual void OnColorize(COLORREF cr) OVERRIDE;

    /**
     * @brief Handle scale change event
     * @param scale scale factor
     */
    virtual void OnScaleChanged(int scale) OVERRIDE;

    /**
     * @brief Measure content size
     * @param nParentWid parent container width
     * @param nParentHei parent container height
     * @return content size
     */
    virtual SIZE MeasureContent(int nParentWid, int nParentHei) OVERRIDE;

  protected:
    /**
     * @brief Whether to automatically release the current m_pSkin object
     */
    BOOL m_bManaged;

    /**
     * @brief Draw state index
     */
    int m_iIcon;

    /**
     * @brief ISkinObj object
     */
    SAutoRefPtr<ISkinObj> m_pSkin;

    /**
     * @brief Image set via code
     */
    SAutoRefPtr<IBitmapS> m_pImg;

    /**
     * @brief SVG object set via code
     */
    SAutoRefPtr<ISvgObj> m_pSvg;

    /**
     * @brief Scaling quality when drawing the image
     */
    FilterLevel m_fl;

    /**
     * @brief Keep aspect ratio
     */
    BOOL m_bKeepAspect;

    /**
     * @brief Fit to image size
     */
    BOOL m_bFitImage;

    SOUI_ATTRS_BEGIN()
        ATTR_SKIN(L"skin", m_pSkin, TRUE)
        ATTR_INT(L"iconIndex", m_iIcon, FALSE)
        ATTR_BOOL(L"keepAspect", m_bKeepAspect, TRUE)
        ATTR_BOOL(L"fitImage", m_bFitImage, TRUE)
    SOUI_ATTRS_END()

  protected:
    /**
     * @brief Draw control
     * @param pRT render device handle
     */
    void OnPaint(IRenderTarget *pRT);

    SOUI_MSG_MAP_BEGIN()
        MSG_WM_PAINT_EX(OnPaint)
    SOUI_MSG_MAP_END()
};

/**
 * @class SAnimateImgWnd
 * @brief Animated image window
 * @details This window supports animation effects.
 */
class SOUI_EXP SAnimateImgWnd
    : public TWindowProxy<IAnimateImgWnd>
    , public ITimelineHandler {
    DEF_SOBJECT(SWindow, L"animateimg")

  public:
    /**
     * @brief Constructor
     */
    SAnimateImgWnd();

    /**
     * @brief Destructor
     */
    virtual ~SAnimateImgWnd()
    {
    }

    /**
     * @brief Start animation
     */
    void WINAPI Start() OVERRIDE;

    /**
     * @brief Stop animation
     */
    void WINAPI Stop() OVERRIDE;

    /**
     * @brief Check animation running state
     * @return running--TRUE
     */
    BOOL WINAPI IsPlaying() SCONST OVERRIDE
    {
        return m_bPlaying;
    }

  protected:
    /**
     * @brief Measure content size
     * @param wid parent container width
     * @param hei parent container height
     * @return content size
     */
    virtual SIZE MeasureContent(int wid, int hei) OVERRIDE;

    /**
     * @brief Handle next frame event
     */
    STDMETHOD_(void, OnNextFrame)(THIS_) OVERRIDE;

    /**
     * @brief Handle colorize event
     * @param cr color
     */
    virtual void OnColorize(COLORREF cr) OVERRIDE;

    /**
     * @brief Container change handler
     * @param pOldContainer old container
     * @param pNewContainer new container
     */
    virtual void OnContainerChanged(ISwndContainer *pOldContainer, ISwndContainer *pNewContainer) OVERRIDE;

    /**
     * @brief Handle scale change event
     * @param scale scale factor
     */
    void OnScaleChanged(int scale) override;

    /**
     * @brief Draw control
     * @param pRT render device handle
     */
    void OnPaint(IRenderTarget *pRT);

    /**
     * @brief Handle show window event
     * @param bShow show state
     * @param nStatus state
     */
    void OnShowWindow(BOOL bShow, UINT nStatus);

    /**
     * @brief Handle destroy event
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
     * @brief Animation image
     */
    SAutoRefPtr<ISkinObj> m_pSkin;

    /**
     * @brief Speed
     */
    int m_nSpeed;

    /**
     * @brief Current frame
     */
    int m_iCurFrame;

    /**
     * @brief Whether to auto start
     */
    BOOL m_bAutoStart;

    /**
     * @brief Whether running
     */
    BOOL m_bPlaying;

    /**
     * @brief OnNextFrame execution count
     */
    int m_iTimeFrame;

    /**
     * @brief Play loop count, -1 means infinite loop
     */
    int m_nRepeat;

    /**
     * @brief Current play loop round
     */
    int m_iRepeat;
};

/**
 * @class SProgress
 * @brief Progress bar class
 * @details Progress bar control.
 * @usage `<progress bgskin=xx posskin=xx min=0 max=100 value=10,showpercent=0/>`
 */
class SOUI_EXP SProgress
    : public TWindowProxy<IProgress>
    , public ITimelineHandler {
    DEF_SOBJECT(SWindow, L"progress")

  public:
    enum
    {
        PC_RAIL = 0,
        PC_SELECT,
    };

    /**
     * @brief Constructor
     */
    SProgress();

    /**
     * @brief Set progress bar value
     * @param nValue progress value
     * @return set successfully--TRUE
     * @note For SetValue with animation effect, please use SSliderBar
     */
    STDMETHOD_(BOOL, SetValue)(THIS_ int nValue) OVERRIDE;

    /**
     * @brief Get progress value
     * @return progress value
     */
    STDMETHOD_(int, GetValue)(CTHIS) SCONST OVERRIDE
    {
        return m_nValue;
    }

    /**
     * @brief Set progress min/max values
     * @param nMin progress minimum
     * @param nMax progress maximum
     */
    STDMETHOD_(void, SetRange)(THIS_ int nMin, int nMax) OVERRIDE;

    /**
     * @brief Get progress min/max values
     * @param pMin progress minimum
     * @param pMax progress maximum
     */
    STDMETHOD_(void, GetRange)(CTHIS_ int *pMin, int *pMax) SCONST OVERRIDE;

    /**
     * @brief Check whether the progress bar is vertical
     * @return vertical state--TRUE
     */
    STDMETHOD_(BOOL, IsVertical)(CTHIS) SCONST OVERRIDE
    {
        return m_bVertical;
    }

  protected:
    /**
     * @brief Handle next frame event (ITimelineHandler interface implementation)
     * @note Only used for the wave effect
     */
    STDMETHOD_(void, OnNextFrame)() OVERRIDE;

  protected:
    /**
     * @brief Handle colorize event
     * @param cr color
     */
    virtual void OnColorize(COLORREF cr) override;

    /**
     * @brief Handle scale change event
     * @param scale scale factor
     */
    virtual void OnScaleChanged(int scale) override;
    SIZE MeasureContent(int nParentWid, int nParentHei) override;
    virtual void OnContainerChanged(ISwndContainer *pOldContainer, ISwndContainer *pNewContainer) override;

    virtual void DrawRail(IRenderTarget *pRT, const CRect &rcClient);
    virtual void DrawPos(IRenderTarget *pRT, const CRect &rcClient);
    virtual void DrawOthers(IRenderTarget *pRT, const CRect &rcClient);
    CRect GetPartRect2(UINT uSBCode) const;
    virtual CRect GetPartRect(const CRect &rcClient, UINT uSBCode) const;
    virtual void OnValueChanged(int nValue, int reason);

  protected:
    /**
     * @brief Progress minimum
     */
    int m_nMinValue;

    /**
     * @brief Progress maximum
     */
    int m_nMaxValue;

    /**
     * @brief Progress value
     */
    int m_nValue;

    /**
     * @brief Whether to show progress value
     */
    BOOL m_bShowText;

    /**
     * @brief Whether vertical
     */
    BOOL m_bVertical;

    /**
     * @brief Background resource
     */
    SAutoRefPtr<ISkinObj> m_pSkinBg;

    /**
     * @brief Foreground resource
     */
    SAutoRefPtr<ISkinObj> m_pSkinPos;

    /**
     * @brief Wave effect skin
     */
    SAutoRefPtr<ISkinObj> m_pSkinWaveEffect;

    /**
     * @brief Wave effect current position (between 0-1)
     */
    float m_fWaveEffectPos;

    /**
     * @brief Wave effect movement direction (1=increase, -1=decrease)
     */
    int m_nWaveEffectDir;

  protected:
    /**
     * @brief Draw control
     * @param pRT render device handle
     */
    void OnPaint(IRenderTarget *pRT);

    /**
     * @brief Handle create event
     * @param lp creation parameters
     * @return success--0
     */
    int OnCreate(void *);

    /**
     * @brief Handle destroy event
     */
    void OnDestroy();

    SOUI_MSG_MAP_BEGIN()
        MSG_WM_PAINT_EX(OnPaint)
        MSG_WM_CREATE(OnCreate)
        MSG_WM_DESTROY(OnDestroy)
    SOUI_MSG_MAP_END()

    HRESULT OnAttrRange(const SStringW &strValue, BOOL bLoading);
    SOUI_ATTRS_BEGIN()
        ATTR_SKIN(L"bkgndSkin", m_pSkinBg, TRUE)
        ATTR_SKIN(L"trackSkin", m_pSkinBg, TRUE)
        ATTR_SKIN(L"posSkin", m_pSkinPos, TRUE)
        ATTR_SKIN(L"filledSkin", m_pSkinPos, TRUE)
        ATTR_SKIN(L"waveSkin", m_pSkinWaveEffect, TRUE)
        ATTR_INT(L"min", m_nMinValue, FALSE)
        ATTR_INT(L"max", m_nMaxValue, FALSE)
        ATTR_CUSTOM(L"range", OnAttrRange)
        ATTR_INT(L"value", m_nValue, FALSE)
        ATTR_BOOL(L"vertical", m_bVertical, FALSE)
        ATTR_BOOL(L"showText", m_bShowText, FALSE)
    SOUI_ATTRS_END()
};

/**
 * @class SHrLine
 * @brief Line control
 * @details Line control.
 * @usage `<hr size=1 mode="vert" lineStyle="dash"/>`
 */
class SOUI_EXP SHrLine : public SWindow {
    DEF_SOBJECT(SWindow, L"hr")

  public:
    /**
     * @brief Constructor
     */
    SHrLine();

    /**
     * @brief Draw control
     * @param pRT render device handle
     */
    void OnPaint(IRenderTarget *pRT);

  protected:
    /**
     * @brief Line style
     */
    int m_nLineStyle;

    /**
     * @brief Line size
     */
    int m_nLineSize;

    /**
     * @brief Line color
     */
    COLORREF m_crLine;

    /**
     * @brief Line mode enum
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
            ATTR_ENUM_VALUE(L"solid", PS_SOLID)           /**< default */
            ATTR_ENUM_VALUE(L"dash", PS_DASH)             /**< ------- */
            ATTR_ENUM_VALUE(L"dot", PS_DOT)               /**< ....... */
            ATTR_ENUM_VALUE(L"dashdot", PS_DASHDOT)       /**< _._._._ */
            ATTR_ENUM_VALUE(L"dashdotdot", PS_DASHDOTDOT) /**< _.._.._ */
        ATTR_ENUM_END(m_nLineStyle)
    SOUI_ATTRS_END()

    SOUI_MSG_MAP_BEGIN()
        MSG_WM_PAINT_EX(OnPaint)
    SOUI_MSG_MAP_END()
};

/**
 * @class SCheckBox
 * @brief Check box control class
 * @details Control used to implement check box functionality.
 * @usage `<check checked="1">This is a check-box</check>`
 */
class SOUI_EXP SCheckBox : public SWindow {
    DEF_SOBJECT(SWindow, L"check") /**< Define object type as "check" */

    enum
    {
        CheckBoxSpacing = 4, /**< Default spacing between check box and text */
    };

  public:
    /**
     * @brief Constructor
     * @details Initialize the check box control.
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
     * @brief State image resource
     * @details Used to draw different check box states (such as checked, unchecked, etc.).
     */
    SAutoRefPtr<ISkinObj> m_pSkin;

    /**
     * @brief Focus state resource
     * @details Skin resource used when the check box gains focus.
     */
    SAutoRefPtr<ISkinObj> m_pFocusSkin;

    int m_nCheckBoxSpacing; /** < Spacing between check box and text */

    /**
     * @brief Get check box rectangle region
     * @return Returns the display position rectangle of the check box (CRect).
     */
    CRect GetCheckRect();

    /**
     * @brief Measure the size required for content
     * @param wid parent container width
     * @param hei parent container height
     * @return Returns the content size required by the control (SIZE).
     */
    SIZE MeasureContent(int wid, int hei) override;
    SIZE MeasureChildren(int wid, int hei) override;

    /**
     * @brief Get the size of the text display area
     * @param pRect Output parameter representing the rectangle of the text display area.
     */
    virtual void GetTextRect(LPRECT pRect) OVERRIDE;

    /**
     * @brief Determine whether the control needs redrawing when the state changes
     * @return TRUE means redraw is needed, FALSE means no redraw needed.
     */
    virtual BOOL NeedRedrawWhenStateChange() OVERRIDE
    {
        return TRUE; // By default, redraw is needed when the state changes
    }

    /**
     * @brief Get dialog code
     * @return Return value is of type UINT, representing the dialog code.
     * @details Returning SC_WANTCHARS indicates the control needs to process character messages.
     */
    virtual UINT WINAPI OnGetDlgCode() const OVERRIDE
    {
        return SC_WANTCHARS;
    }

    /**
     * @brief Draw focus style
     * @param pRT render device handle (IRenderTarget*).
     * @details This method is called to draw the focus style when the control gains focus.
     */
    virtual void DrawFocus(IRenderTarget *pRT) OVERRIDE;

    /**
     * @brief Handle colorize event
     * @param cr color value (COLORREF).
     * @details This method is called when the control's color changes.
     */
    virtual void OnColorize(COLORREF cr) OVERRIDE;

    /**
     * @brief Handle scale change event
     * @param scale scale factor (int).
     * @details This method is called when the control's scale factor changes.
     */
    virtual void OnScaleChanged(int scale) OVERRIDE;

    /**
     * @brief Handle left mouse button up event
     * @param nFlags flags (UINT).
     * @param pt mouse position (CPoint).
     * @details This method is called when the user releases the left mouse button.
     */
    void OnLButtonUp(UINT nFlags, CPoint pt);

    /**
     * @brief Handle key down event
     * @param nChar character (UINT).
     * @param nRepCnt repeat count (UINT).
     * @param nFlags flags (UINT).
     * @details This method is called when the user presses a keyboard key.
     */
    void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);

    /**
     * @brief Handle draw event
     * @param pRT render device handle (IRenderTarget*).
     * @details Responsible for drawing the control's appearance.
     */
    void OnPaint(IRenderTarget *pRT);

    /**
     * @brief Handle custom attribute "checked"
     * @param strValue attribute value (SStringW).
     * @param bLoading whether loading (BOOL).
     * @return Returns S_OK on success, or an HRESULT error code on failure.
     * @details Used to parse and set the "checked" attribute.
     */
    HRESULT OnAttrCheck(const SStringW &strValue, BOOL bLoading);

    /**
     * @brief Define the control's attribute collection
     */
    SOUI_ATTRS_BEGIN()
        ATTR_SKIN(L"skin", m_pSkin, FALSE)           /**< Define skin resource attribute */
        ATTR_SKIN(L"focusSkin", m_pFocusSkin, FALSE) /**< Define focus skin resource attribute */
        ATTR_CUSTOM(L"checked", OnAttrCheck)         /**< Define custom attribute "checked" */
        ATTR_INT(L"sep", m_nCheckBoxSpacing, TRUE)   /**< Define spacing attribute */
    SOUI_ATTRS_END()

    /**
     * @brief Define the control's message map
     */
    SOUI_MSG_MAP_BEGIN()
        MSG_WM_PAINT_EX(OnPaint)            /**< Bind draw message */
        MSG_WM_LBUTTONDBLCLK(OnLButtonDown) /**< Bind left mouse button double-click message */
        MSG_WM_LBUTTONUP(OnLButtonUp)       /**< Bind left mouse button up message */
        MSG_WM_KEYDOWN(OnKeyDown)           /**< Bind key down message */
    SOUI_MSG_MAP_END()
};

/**
 * @class SIconWnd
 * @brief Icon control
 * @details Icon control.
 * @usage `<icon src="icon:16" />`
 */
class SOUI_EXP SIconWnd : public TWindowProxy<IIconWnd> {
    DEF_SOBJECT(SWindow, L"icon")

  public:
    /**
     * @brief Constructor
     */
    SIconWnd();

    /**
     * @brief Destructor
     */
    virtual ~SIconWnd();

    /**
     * @brief Set icon
     * @param hIcon icon handle
     */
    STDMETHOD_(void, SetIcon)(THIS_ HICON hIcon) OVERRIDE;

  protected:
    /**
     * @brief Measure content size
     * @param wid parent container width
     * @param hei parent container height
     * @return content size
     */
    SIZE MeasureContent(int wid, int hei) override;

    /**
     * @brief Draw control
     * @param pRT render device handle
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
    HICON m_theIcon; /**< Icon resource handle */
};

/**
 * @class SRadioBox
 * @brief Radio box control class
 * @details Control used to implement radio box functionality.
 * @usage `<radio checked="1">This is a check-box</radio>`
 */
class SOUI_EXP SRadioBox : public SWindow {
    DEF_SOBJECT(SWindow, L"radio") /**< Define object type as "radio" */

    enum
    {
        RadioBoxSpacing = 4,
    };

  public:
    /**
     * @brief Constructor
     * @details Initialize the radio box control.
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
     * @brief Get the radio box display position rectangle region
     * @return Returns the radio box display position rectangle (CRect).
     */
    CRect GetRadioRect();

    /**
     * @brief Get the size of the text display area
     * @param pRect Output parameter representing the rectangle of the text display area.
     */
    virtual void GetTextRect(LPRECT pRect) OVERRIDE;

    /**
     * @brief Measure the size required for content
     * @param wid parent container width
     * @param hei parent container height
     * @return Returns the content size required by the control (SIZE).
     */
    SIZE MeasureContent(int wid, int hei) OVERRIDE;
    SIZE MeasureChildren(int wid, int hei) OVERRIDE;

    /**
     * @brief Get dialog code
     * @return Return value is of type UINT, representing the dialog code.
     * @details Returning 0 means no specific message is processed.
     */
    STDMETHOD_(UINT, OnGetDlgCode)(THIS) SCONST OVERRIDE
    {
        return 0;
    }

    /**
     * @brief Determine whether radio buttons with the same name are automatically added to the same group
     * @return TRUE means automatic grouping, FALSE means no automatic grouping.
     */
    STDMETHOD_(BOOL, IsSiblingsAutoGroupped)(THIS) SCONST OVERRIDE
    {
        return TRUE; /**< By default, radio buttons with the same name are grouped automatically */
    }

    /**
     * @brief Determine whether the control needs redrawing when the state changes
     * @return TRUE means redraw is needed, FALSE means no redraw needed.
     */
    virtual BOOL NeedRedrawWhenStateChange() OVERRIDE;

    /**
     * @brief Draw focus style
     * @param pRT render device handle (IRenderTarget*).
     * @details This method is called to draw the focus style when the control gains focus.
     */
    virtual void DrawFocus(IRenderTarget *pRT) OVERRIDE;

    /**
     * @brief Get the selected sibling control in the current group
     * @return Returns a pointer to the selected sibling control (SWindow*), or NULL if none.
     */
    virtual SWindow *GetSelectedSiblingInGroup() OVERRIDE;

    /**
     * @brief Handle state change event
     * @param dwOldState old state (DWORD).
     * @param dwNewState new state (DWORD).
     */
    virtual void OnStateChanging(DWORD dwOldState, DWORD dwNewState) OVERRIDE;

    /**
     * @brief Handle colorize event
     * @param cr color value (COLORREF).
     */
    virtual void OnColorize(COLORREF cr) OVERRIDE;

    /**
     * @brief Handle scale change event
     * @param nScale scale factor (int).
     */
    virtual void OnScaleChanged(int nScale) OVERRIDE;

  protected:
    /**
     * @brief Handle focus change event
     * @param wndOld old window handle (SWND).
     * @param reason focus change reason (SFocusManager::FocusChangeReason).
     */
    void OnSetFocus(SWND wndOld, SFocusManager::FocusChangeReason reason);

    /**
     * @brief Handle left mouse button up event
     * @param nFlags flags (UINT).
     * @param pt mouse position (CPoint).
     */
    void OnLButtonUp(UINT nFlags, CPoint pt);

    /**
     * @brief Draw control
     * @param pRT render device handle (IRenderTarget*).
     */
    void OnPaint(IRenderTarget *pRT);

    /**
     * @brief Handle custom attribute "checked"
     * @param strValue attribute value (SStringW).
     * @param bLoading whether loading (BOOL).
     * @return Returns S_OK on success, or an HRESULT error code on failure.
     */
    HRESULT OnAttrCheck(const SStringW &strValue, BOOL bLoading);

    /**
     * @brief Define the control's attribute collection
     */
    SOUI_ATTRS_BEGIN()
        ATTR_SKIN(L"skin", m_pSkin, FALSE)                      /**< Define skin resource attribute */
        ATTR_SKIN(L"focusSkin", m_pFocusSkin, FALSE)            /**< Define focus skin resource attribute */
        ATTR_CUSTOM(L"checked", OnAttrCheck)                    /**< Define custom attribute "checked" */
        ATTR_ENUM_BEGIN(L"iconAlign", UINT, TRUE)               /**< Define icon alignment attribute */
            ATTR_ENUM_VALUE(L"left", SwndStyle::Align_Left)     /**< Left align */
            ATTR_ENUM_VALUE(L"center", SwndStyle::Align_Center) /**< Center align */
            ATTR_ENUM_VALUE(L"right", SwndStyle::Align_Right)   /**< Right align */
        ATTR_ENUM_END(m_uIconAlign)
        ATTR_ENUM_BEGIN(L"iconValign", UINT, TRUE)               /**< Define icon vertical alignment attribute */
            ATTR_ENUM_VALUE(L"top", SwndStyle::VAlign_Top)       /**< Top align */
            ATTR_ENUM_VALUE(L"middle", SwndStyle::VAlign_Middle) /**< Middle align */
            ATTR_ENUM_VALUE(L"bottom", SwndStyle::VAlign_Bottom) /**< Bottom align */
        ATTR_ENUM_END(m_uIconVAlign)
        ATTR_INT(L"sep", m_nRadioBoxSpacing, TRUE) /**< Define spacing attribute */
    SOUI_ATTRS_END()

    /**
     * @brief Define the control's message map
     */
    SOUI_MSG_MAP_BEGIN()
        MSG_WM_PAINT_EX(OnPaint)        /**< Bind draw message */
        MSG_WM_LBUTTONUP(OnLButtonUp)   /**< Bind left mouse button up message */
        MSG_WM_SETFOCUS_EX2(OnSetFocus) /**< Bind focus change message */
    SOUI_MSG_MAP_END()

  protected:
    /**
     * @brief Skin resource
     */
    SAutoRefPtr<ISkinObj> m_pSkin;

    /**
     * @brief Focus skin resource
     */
    SAutoRefPtr<ISkinObj> m_pFocusSkin;

    /**
     * @brief Icon horizontal alignment
     */
    UINT m_uIconAlign;

    /**
     * @brief Vertical alignment of the icon
     */
    UINT m_uIconVAlign;

    /**
     * @brief Spacing between the radio button and the text
     */
    int m_nRadioBoxSpacing;
};

/**
 * @class SRadioGroup
 * @brief Radio button group control
 * @details Manages a group of radio buttons, ensuring mutual exclusion within the same group.
 */
class SOUI_EXP SRadioGroup : public SWindow {
    DEF_SOBJECT(SWindow, L"radioGroup")

  public:
    /**
     * @brief Constructor
     */
    SRadioGroup();

    /**
     * @brief Check the radio button with the specified ID
     * @param nID Radio button ID
     * @return Success--TRUE Failure--FALSE
     */
    BOOL Check(int nID);

    /**
     * @brief Check the radio button with the specified name
     * @param pszName Radio button name
     * @return Success--TRUE Failure--FALSE
     */
    BOOL Check(LPCTSTR pszName);

    /**
     * @brief Clear all selected radio buttons
     * @return Success--TRUE Failure--FALSE
     */
    BOOL ClearCheck();

  protected:
    /**
     * @brief Fire event
     * @param evt Event parameter
     * @return Success--TRUE Failure--FALSE
     */
    STDMETHOD_(BOOL, FireEvent)(THIS_ IEvtArgs *evt) OVERRIDE;

    /**
     * @brief Handle after inserting a child control
     * @param pChild Child control pointer
     */
    virtual void OnAfterInsertChild(SWindow *pChild) OVERRIDE;

    /**
     * @brief Handle before removing a child control
     * @param pChild Child control pointer
     */
    virtual void OnBeforeRemoveChild(SWindow *pChild) OVERRIDE;
};

/**
 * @class SToggle
 * @brief Toggle control
 * @details Toggle control, inherited from SCheckBox, used to implement on/off (toggle) effect.
 */
class SOUI_EXP SToggle : public SCheckBox {
    DEF_SOBJECT(SCheckBox, L"toggle")

  public:
    /**
     * @brief Constructor
     */
    SToggle();

    /**
     * @brief Set Toggle attribute
     * @param bToggle Whether to enable the Toggle effect
     * @param bUpdate Whether to update, default value TRUE
     */
    void SetToggle(BOOL bToggle, BOOL bUpdate = TRUE);

    /**
     * @brief Get Toggle attribute
     * @return Returns BOOL
     * @details Gets the Toggle attribute, mainly used to retrieve whether the Toggle effect is enabled
     */
    BOOL GetToggle();

  protected:
    /**
     * @brief Measure content size
     * @param nParentWid parent container width
     * @param nParentHei parent container height
     * @return content size
     */
    SIZE MeasureContent(int nParentWid, int nParentHei) override;

    /**
     * @brief Whether a state change requires repainting
     * @return Needs repaint--TRUE
     */
    virtual BOOL NeedRedrawWhenStateChange() OVERRIDE
    {
        return TRUE;
    }

  protected:
    /**
     * @brief Draw control
     * @param pRT render device handle
     */
    void OnPaint(IRenderTarget *pRT);

    SOUI_MSG_MAP_BEGIN()
        MSG_WM_PAINT_EX(OnPaint)
    SOUI_MSG_MAP_END()
};

/**
 * @class SGroup
 * @brief Group control
 * @details Group control, used to group and display multiple controls.
 * @usage `<group colorLine1="#b8d5e2" colorLine2="#999999">group text</group>`
 */
class SOUI_EXP SGroup : public SWindow {
    DEF_SOBJECT(SWindow, L"group")

  public:
    /**
     * @brief Constructor
     */
    SGroup();

  protected:
    /**
     * @brief Get the layout rectangle of the child control
     * @param prc Output rectangle
     */
    STDMETHOD_(void, GetChildrenLayoutRect)(THIS_ RECT *prc) SCONST OVERRIDE;

    /**
     * @brief Get the desired size
     * @param psz Output size
     * @param wid Parent container width
     * @param hei Parent container height
     */
    STDMETHOD_(void, GetDesiredSize)(THIS_ SIZE *psz, int wid, int hei) OVERRIDE;

    /**
     * @brief Draw control
     * @param pRT render device handle
     */
    void OnPaint(IRenderTarget *pRT);

  protected:
    /**
     * @brief Two colors used for group 3D display
     */
    COLORREF m_crLine1, m_crLine2;

    /**
     * @brief Corner radius
     */
    SLayoutSize m_nRound;

    /**
     * @brief Header height
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

class SOUI_EXP SKeyboardSpacer : public SWindow {
    DEF_SOBJECT(SWindow, L"keyboardSpacer")
  public:
    SKeyboardSpacer();

  protected:
    void OnKeyboardHeight(int height);
    SOUI_MSG_MAP_BEGIN()
        MSG_KEYBOARD_HEIGHT(OnKeyboardHeight)
    SOUI_MSG_MAP_END()
};

SNSEND

#endif /**< __SCMNCTRL__H__ */