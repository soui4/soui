#ifndef __ROOTROOTWINDOW__H__
#define __ROOTROOTWINDOW__H__
#include <core/SWnd.h>
#include <valueAnimator/SValueAnimator.h>
#include <souicoll.h>

SNSBEGIN

enum
{
    ROOT_ID = -100, /**< The ID of the root window */
};

enum
{
    kAni4Destroy = 1,
    kAni4Hide = 2,
};

enum AniState
{
    Ani_none = 0,
    Ani_win_enter = 1,
    Ani_win_exit = 2,
    Ani_win = (Ani_win_enter | Ani_win_exit),
    Ani_host = 4,
    Ani_All = (Ani_win | Ani_host),
};

class SHostWnd;
class SOUI_EXP SRootWindow
    : public SWindow
    , public IAnimatorUpdateListener
    , public SAnimatorListener {
    /** DEF_SOBJECT macro defines the type name of the class */
    DEF_SOBJECT(SWindow, L"root")
    /** Declare SHostWnd as a friend class to allow access to private members */
    friend class SHostWnd;

  public:
    /**
     * @brief Constructor for SRootWindow.
     *
     * Initializes the SRootWindow object with a pointer to the host window.
     *
     * @param pHostWnd Pointer to the host window.
     */
    SRootWindow();

    /**
     * @brief Destructor for SRootWindow.
     */
    ~SRootWindow();

  public:
    /**
     * @brief Sets the pointer to the host window.
     *
     * @param pHostWnd Pointer to the host window.
     */
    void SetHostWnd(SHostWnd *pHostWnd);

    /**
     * @brief Gets the pointer to the host window.
     *
     * @return Pointer to the host window.
     */
    SHostWnd *GetHostWnd() const;

    /**
     * @brief Fires a menu command event.
     *
     * @param menuID ID of the menu command to trigger.
     */
    void FireMenuCmd(int menuID);

    void DrawFocusAni(IRenderTarget *pRT);

  public:
    /**
     * @brief Updates the layout of the window.
     *
     * Overrides the base class method to update the layout.
     */
    STDMETHOD_(void, UpdateLayout)(THIS) OVERRIDE;

    /**
     * @brief Gets the ID of the window.
     *
     * @return ID of the window.
     */
    STDMETHOD_(int, GetID)(CTHIS) SCONST OVERRIDE
    {
        return ROOT_ID;
    }

  protected:
    /**
     * @brief Called when an animation stops.
     *
     * Overrides the base class method to handle animation stop events.
     *
     * @param pAni Pointer to the animation object.
     */
    STDMETHOD_(void, OnAnimationStop)(THIS_ IAnimation *pAni) OVERRIDE;

    /**
     * @brief Called when an animation is invalidated.
     *
     * @param bErase Flag indicating whether to erase the animation.
     */
    virtual void OnAnimationInvalidate(bool bErase) OVERRIDE;

    STDMETHOD_(void, onAnimationUpdate)(THIS_ IValueAnimator *pAnimator) OVERRIDE;
    STDMETHOD_(void, onAnimationEnd)(THIS_ IValueAnimator *pAnimator) OVERRIDE;

  protected: /**< SWindow virtual methods */
    /**
     * @brief Called before painting the window.
     *
     * @param pRT Pointer to the render target.
     * @param painter Painter object.
     */
    void BeforePaint(IRenderTarget *pRT, SPainter &painter) const override;

    /**
     * @brief Called after painting the window.
     *
     * @param pRT Pointer to the render target.
     * @param painter Painter object.
     */
    void AfterPaint(IRenderTarget *pRT, SPainter &painter) const override;

    /**
     * @brief Builds the painter object.
     */
    void BuildPainter(SPainter &painter) const override;

    /**
     * @brief Checks if the window is a layered window.
     *
     * @return TRUE if the window is a layered window, FALSE otherwise.
     */
    virtual BOOL IsLayeredWindow() const OVERRIDE
    {
        return FALSE;
    }

    /**
     * @brief Called when the language changes.
     *
     * @return HRESULT indicating the success or failure of the operation.
     */
    virtual HRESULT OnLanguageChanged() OVERRIDE;

    /**
     * @brief Called when the scale changes.
     *
     * @param scale New scale factor.
     */
    virtual void OnScaleChanged(int scale) OVERRIDE;

    /**
     * @brief Requests a relayout of the window.
     *
     * @param hSource Source window handle.
     * @param bSourceResizable Flag indicating if the source window is resizable.
     */
    virtual void RequestRelayout(SWND hSource, BOOL bSourceResizable) OVERRIDE;

    void OnDecendantFocusChanged(SWND swnd, BOOL bSet) OVERRIDE;

  public:
    /** Define attributes for enter and exit animations */
    SOUI_ATTRS_BEGIN()
        ATTR_ANIMATION(L"enterAnimation", m_aniEnter, FALSE)
        ATTR_ANIMATION(L"exitAnimation", m_aniExit, FALSE)
        ATTR_SKIN(L"focusAniSkin", m_pSkinFocusAni, FALSE)                     /**< Skin of the focus moving frame; without it the feature is off */
        ATTR_CHAIN_PTR_PREFIX(m_focusAnimator, 0, SAttrChainPrefix::FOCUSANI)  /**< animator:duration / animator:interpolator of the move */
        ATTR_CHAIN_PTR_PREFIX(m_focusAlphaAnimator, 0, SAttrChainPrefix::FADE) /**< fade:duration / fade:interpolator of the fade out */
    SOUI_ATTRS_END()

  protected:
    /**
     * @brief Skin painting the focus moving frame.
     *
     * It is the switch of the feature as well: an empty skin means no frame is painted and no
     * animation is started, so no extra boolean is needed.
     */
    SAutoRefPtr<ISkinObj> m_pSkinFocusAni;

    /**
     * @brief RECT animator driving the focus moving frame.
     *
     * Its parameters come from the animator: namespace of the SOUI node, e.g.
     * animator:duration="200" animator:interpolator="Decelerate".
     */
    SAutoRefPtr<SRectAnimator> m_focusAnimator;

    /**
     * @brief Alpha animator that ends the life of the focus frame.
     *
     * Once the frame has arrived it stays opaque on the target for a while, then fades out; when this
     * animator ends the frame is forgotten and repainted away. It is driven by the fade: namespace of
     * the SOUI node, e.g. fade:duration="200" fade:interpolator="Decelerate".
     */
    SAutoRefPtr<SByteAnimator> m_focusAlphaAnimator;

    CRect m_rcFocusAni; /**< Frame rect painted last, kept to repaint (erase) it when the frame moves. */
    SWND m_hFocus;      /**< Window that held the focus when the frame was last pointed at a target. */
    /** Auto-managed pointers for enter and exit animations */
    SAutoRefPtr<IAnimation> m_aniEnter, m_aniExit;
    /** Pointer to the host window */
    SHostWnd *m_pHostWnd;
};
SNSEND
#endif /**< __ROOTROOTWINDOW__H__ */