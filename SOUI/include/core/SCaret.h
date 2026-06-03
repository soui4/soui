#ifndef __SCARET__H__
#define __SCARET__H__

#include <interface/scaret-i.h>
#include <interface/sinterpolator-i.h>
#include <interface/STimelineHandler-i.h>
#include <sobject/Sobject.hpp>

SNSBEGIN

/**
 * @class      SCaret
 * @brief      Caret management class
 *
 * @details    This class manages the caret, including its initialization, positioning, visibility,
 *             drawing, and animation.
 */
class SOUI_EXP SCaret
    : public TObjRefImpl<SObjectImpl<ICaret> >
    , public ITimelineHandler {
    DEF_SOBJECT(SObjectImpl<ICaret>, L"caret")

  public:
    /**
     * @brief    Constructor
     * @param    pContainer  Pointer to the container window
     *
     * @details  Initializes the caret with a container window.
     */
    SCaret(ISwndContainer *pContainer);

    /**
     * @brief    Destructor
     *
     * @details  Cleans up the caret object.
     */
    ~SCaret();

  public:
    // Methods inherited from ICaret

    /**
     * @brief    Initializes the caret
     * @param    hBmp    Handle to the bitmap for the caret
     * @param    nWid    Width of the caret
     * @param    nHei    Height of the caret
     * @return   TRUE if successful, otherwise FALSE
     *
     * @details  Initializes the caret with a bitmap and specified dimensions.
     */
    STDMETHOD_(BOOL, Init)(THIS_ HBITMAP hBmp, int nWid, int nHei) OVERRIDE;

    /**
     * @brief    Sets the position of the caret
     * @param    x       X-coordinate of the caret
     * @param    y       Y-coordinate of the caret
     *
     * @details  Sets the position of the caret to the specified coordinates.
     */
    STDMETHOD_(void, SetPosition)(THIS_ int x, int y) OVERRIDE;

    /**
     * @brief    Sets the visibility of the caret
     * @param    bVisible  TRUE to make the caret visible, FALSE to hide it
     * @param    owner     Handle to the owner window
     * @return   TRUE if successful, otherwise FALSE
     *
     * @details  Sets the visibility of the caret and associates it with the owner window.
     */
    STDMETHOD_(BOOL, SetVisible)(THIS_ BOOL bVisible, SWND owner) OVERRIDE;

    /**
     * @brief    Checks if the caret is visible
     * @return   TRUE if the caret is visible, otherwise FALSE
     *
     * @details  Returns the visibility status of the caret.
     */
    STDMETHOD_(BOOL, IsVisible)(THIS) SCONST OVERRIDE;

    /**
     * @brief    Draws the caret
     * @param    pRT     Pointer to the rendering target
     *
     * @details  Draws the caret on the specified rendering target.
     */
    STDMETHOD_(void, Draw)(THIS_ IRenderTarget *pRT) OVERRIDE;

    /**
     * @brief    Gets the rectangle of the caret
     * @return   Rectangle of the caret
     *
     * @details  Returns the bounding rectangle of the caret.
     */
    STDMETHOD_(RECT, GetRect)(THIS) SCONST OVERRIDE;

  public:
    // Method inherited from ITimelineHandler

    /**
     * @brief    Handles the next frame in the timeline
     *
     * @details  Updates the caret's state for the next frame, handling animation if enabled.
     */
    STDMETHOD_(void, OnNextFrame)(THIS_) OVERRIDE;

  public:
    SOUI_ATTRS_BEGIN()
        ATTR_BOOL(L"animate", m_bAniCaret, FALSE)
        ATTR_COLOR(L"color", m_crCaret, FALSE)
        ATTR_INT(L"fadeTime", m_nAniFrames, FALSE)
        ATTR_INT(L"showTime", m_nShowFrames, FALSE)
        ATTR_INTERPOLATOR(L"interpolator", m_AniInterpolator, FALSE)
        ATTR_CHAIN_PTR(m_AniInterpolator, 0)
    SOUI_ATTRS_END()

  protected:
    /**
     * @brief    Invalidates the caret area
     *
     * @details  Invalidates the area where the caret is drawn to trigger a redraw.
     */
    void Invalidate();

  protected:
    BOOL m_bVisible;                  /**< TRUE if the caret is currently visible. */
    CPoint m_ptCaret;                 /**< Position of the caret. */
    SAutoRefPtr<IBitmapS> m_bmpCaret; /**< Bitmap representing the caret. */
    bool m_bDrawCaret;                /**< TRUE if the caret is currently drawing. */
    int m_iFrame;                     /**< Current frame index for animation. */
    BYTE m_byAlpha;                   /**< Alpha value for caret transparency. */

    BOOL m_bAniCaret;                             /**< TRUE if the caret has animation enabled. */
    COLORREF m_crCaret;                           /**< Color of the caret. */
    int m_nAniFrames;                             /**< Number of frames for fade animation. */
    int m_nShowFrames;                            /**< Number of frames for show/hide animation. */
    SAutoRefPtr<IInterpolator> m_AniInterpolator; /**< Interpolator for caret animation. */
    ISwndContainer *m_pContainer;                 /**< Pointer to the container window. */
    SWND m_hOwner;                                /**< Handle to the owner window. */
};

SNSEND

#endif // __SCARET__H__