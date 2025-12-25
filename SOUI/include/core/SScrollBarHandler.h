#ifndef __SSCROLLBARHANDLER__H__
#define __SSCROLLBARHANDLER__H__

SNSBEGIN

/**
 * @struct IScrollBarHost
 * @brief Interface for scrollbar host.
 *
 * Description: Defines the interface for the scrollbar host, which provides necessary methods for scrollbar operations.
 */
struct IScrollBarHost
{
    enum kSbConst
    {
        Timer_Wait = 100,
        Timer_Go = 101,
        kTime_Wait = 200,
        kTime_Go = 100,
    };

    /**
     * @brief Gets the rectangle of the scrollbar.
     * @param bVert TRUE for vertical scrollbar, FALSE for horizontal scrollbar.
     * @return Rectangle of the scrollbar.
     */
    STDMETHOD_(CRect, GetScrollBarRect)(CTHIS_ BOOL bVert) SCONST PURE;

    /**
     * @brief Gets the skin for the scrollbar.
     * @param bVert TRUE for vertical scrollbar, FALSE for horizontal scrollbar.
     * @return Pointer to the scrollbar skin.
     */
    STDMETHOD_(ISkinObj *, GetScrollBarSkin)(CTHIS_ BOOL bVert) SCONST PURE;

    /**
     * @brief Gets the scroll information for the scrollbar.
     * @param bVert TRUE for vertical scrollbar, FALSE for horizontal scrollbar.
     * @return Pointer to the scroll information structure.
     */
    STDMETHOD_(const SCROLLINFO *, GetScrollBarInfo)(CTHIS_ BOOL bVert) SCONST PURE;

    /**
     * @brief Gets the arrow size for the scrollbar.
     * @param bVert TRUE for vertical scrollbar, FALSE for horizontal scrollbar.
     * @return Arrow size.
     */
    STDMETHOD_(int, GetScrollBarArrowSize)(CTHIS_ BOOL bVert) SCONST PURE;

    /**
     * @brief Updates a part of the scrollbar.
     * @param bVert TRUE for vertical scrollbar, FALSE for horizontal scrollbar.
     * @param iPart Part to update.
     */
    STDMETHOD_(void, OnScrollUpdatePart)(THIS_ BOOL bVert, int iPart) PURE;

    /**
     * @brief Updates the thumb track of the scrollbar.
     * @param bVert TRUE for vertical scrollbar, FALSE for horizontal scrollbar.
     * @param nPos New position of the thumb.
     */
    STDMETHOD_(void, OnScrollUpdateThumbTrack)(THIS_ BOOL bVert, int nPos) PURE;

    /**
     * @brief Gets the container for the scrollbar.
     * @return Pointer to the scrollbar container.
     */
    STDMETHOD_(ISwndContainer *, GetScrollBarContainer)(THIS) PURE;

    /**
     * @brief Checks if the scrollbar is enabled.
     * @param bVertical TRUE for vertical scrollbar, FALSE for horizontal scrollbar.
     * @return TRUE if the scrollbar is enabled, FALSE otherwise.
     */
    STDMETHOD_(BOOL, IsScrollBarEnable)(THIS_ BOOL bVertical) SCONST PURE;

    /**
     * @brief Handles scroll commands.
     * @param bVert TRUE for vertical scrollbar, FALSE for horizontal scrollbar.
     * @param iCmd Command identifier.
     * @param nPos Position for the command.
     */
    STDMETHOD_(void, OnScrollCommand)(THIS_ BOOL bVert, int iCmd, int nPos) PURE;

    /**
     * @brief Sets a timer for the scrollbar.
     * @param bVert TRUE for vertical scrollbar, FALSE for horizontal scrollbar.
     * @param id Timer identifier.
     * @param uElapse Elapse time for the timer.
     */
    STDMETHOD_(void, OnScrollSetTimer)(THIS_ BOOL bVert, char id, UINT uElapse) PURE;

    /**
     * @brief Kills a timer for the scrollbar.
     * @param bVert TRUE for vertical scrollbar, FALSE for horizontal scrollbar.
     * @param id Timer identifier.
     */
    STDMETHOD_(void, OnScrollKillTimer)(THIS_ BOOL bVert, char id) PURE;

    /**
     * @brief Gets the interpolator for the scrollbar.
     * @return Pointer to the interpolator.
     */
    STDMETHOD_(const IInterpolator *, GetScrollInterpolator)(CTHIS) SCONST PURE;

    /**
     * @brief Gets the number of fade frames for the scrollbar.
     * @return Number of fade frames.
     */
    STDMETHOD_(int, GetScrollFadeFrames)(CTHIS) SCONST PURE;

    /**
     * @brief Gets the minimum alpha value for the scrollbar thumb track.
     * @return Minimum alpha value.
     */
    STDMETHOD_(BYTE, GetScrollThumbTrackMinAlpha)(CTHIS) SCONST PURE;
};

/**
 * @class     SScrollBarHandler
 * @brief     Scrollbar Handler
 *
 * Description: Manages the behavior and appearance of scrollbars.
 */
class SOUI_EXP SScrollBarHandler : public ITimelineHandler {
  private:
    SScrollBarHandler(const SScrollBarHandler &);                  // Private copy constructor
    const SScrollBarHandler &operator=(const SScrollBarHandler &); // Private assignment operator

  public:
    enum
    {
        kSbRail = 100,
    };

    /**
     * @brief Constructor for SScrollBarHandler.
     * @param pCB Pointer to the scrollbar host.
     * @param bVert TRUE for vertical scrollbar, FALSE for horizontal scrollbar.
     */
    SScrollBarHandler(IScrollBarHost *pCB, bool bVert = false);

  public:
    /**
     * @brief Gets the rectangle of a scrollbar part.
     * @param iPart Part identifier.
     * @return Rectangle of the part.
     */
    CRect GetPartRect(int iPart) const;

    /**
     * @brief Gets the hit part of the scrollbar.
     * @return Hit part identifier.
     */
    int GetHitPart() const;

    /**
     * @brief Gets the clicked part of the scrollbar.
     * @return Clicked part identifier.
     */
    int GetClickPart() const;

    /**
     * @brief Checks if the scrollbar is vertical.
     * @return TRUE if vertical, FALSE if horizontal.
     */
    bool IsVertical() const;

    /**
     * @brief Sets the orientation of the scrollbar.
     * @param bVert TRUE for vertical, FALSE for horizontal.
     */
    void SetVertical(bool bVert);

    /**
     * @brief Handles mouse hover events.
     * @param pt Point where the mouse is hovering.
     */
    void OnMouseHover(CPoint pt);

    /**
     * @brief Handles mouse down events.
     * @param pt Point where the mouse button was pressed.
     * @return TRUE if handled, FALSE otherwise.
     */
    bool OnMouseDown(CPoint pt);

    /**
     * @brief Handles mouse up events.
     * @param pt Point where the mouse button was released.
     */
    void OnMouseUp(CPoint pt);

    /**
     * @brief Handles mouse move events.
     * @param pt Point where the mouse moved.
     */
    void OnMouseMove(CPoint pt);

    /**
     * @brief Handles mouse leave events.
     */
    void OnMouseLeave();

    /**
     * @brief Draws a part of the scrollbar.
     * @param pRT Pointer to the render target.
     * @param iPart Part identifier to draw.
     */
    void OnDraw(IRenderTarget *pRT, int iPart) const;

    /**
     * @brief Handles timer events.
     * @param id Timer identifier.
     */
    void OnTimer(char id);

    /**
     * @brief Handles destruction events.
     */
    void OnDestroy();

    /**
     * @brief Performs hit testing on the scrollbar.
     * @param pt Point to test.
     * @return Hit part identifier.
     */
    int HitTest(CPoint pt) const;

    /**
     * @brief Handles container change events.
     * @param pOldContainer Pointer to the old container.
     * @param pNewContainer Pointer to the new container.
     */
    void OnContainerChanged(ISwndContainer *pOldContainer, ISwndContainer *pNewContainer);

    /**
     * @brief Checks if the scrollbar is in thumb tracking mode.
     * @return TRUE if in thumb tracking mode, FALSE otherwise.
     */
    BOOL IsThumbTracking() const;

  protected:
    /**
     * @brief Gets the container for the scrollbar.
     * @return Pointer to the scrollbar container.
     */
    ISwndContainer *GetContainer();

    /**
     * @brief Gets the interpolator for the scrollbar.
     * @return Pointer to the interpolator.
     */
    const IInterpolator *GetInterpolator() const;

    /**
     * @brief Gets the alpha value for a part of the scrollbar.
     * @param iPart Part identifier.
     * @return Alpha value.
     */
    BYTE GetAlpha(int iPart) const;

    /**
     * @brief Gets the fade step for the scrollbar.
     * @return Fade step.
     */
    int GetFadeStep() const;

    /**
     * @brief Gets the state of a part of the scrollbar.
     * @param iPart Part identifier.
     * @return State of the part.
     */
    DWORD GetPartState(int iPart) const;

  protected:
    /**
     * @brief Handles the next frame in the timeline.
     */
    STDMETHOD_(void, OnNextFrame)(THIS_) OVERRIDE;

  private:
    enum FADEMODE
    {
        FADEOUT = -1,
        FADE_STOP = 0,
        FADEIN = 1,
    };

    IScrollBarHost *m_pSbHost; // Pointer to the scrollbar host
    bool m_bVert;              // TRUE if vertical, FALSE if horizontal
    int m_iFrame;              // Current frame index
    FADEMODE m_fadeMode;       // Fade mode
    int m_iHitPart;            // Hit part identifier
    int m_iClickPart;          // Clicked part identifier
    CPoint m_ptClick;          // Click point
    CPoint m_ptCursor;         // Cursor point
    int m_nClickPos;           // Click position
};

SNSEND

#endif // __SSCROLLBARHANDLER__H__