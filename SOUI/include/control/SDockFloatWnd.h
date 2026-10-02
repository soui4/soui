#ifndef __SDOCKFLOATWND__H__
#define __SDOCKFLOATWND__H__

#include <core/SHostWnd.h>

SNSBEGIN

class SDockBar;

/**
 * @class SDockFloatWnd
 * @brief Floating container host window for a dock bar
 * @details When a dock bar is floated, it is reparented into this host window.
 *          The float window owns itself: once Create() is called, the object
 *          manages its own lifetime and is deleted when the native window is
 *          destroyed (OnFinalMessage).
 */
class SOUI_EXP SDockFloatWnd : public SHostWnd {
    typedef SHostWnd __baseCls;

  public:
    SDockFloatWnd();
    virtual ~SDockFloatWnd();

    /**
     * @brief Create the float window and reparent the dock bar into it
     * @param pDockBar dock bar to host
     * @param hOwner owner native window (the float window is destroyed together with its owner)
     * @param ptScreen desired screen top-left of the float window
     * @param szFloat desired size of the float window
     * @return TRUE on success
     * @note On failure the object is released by this method; the caller must
     *       not touch the pointer after it returns FALSE.
     */
    BOOL Create(SDockBar *pDockBar, HWND hOwner, const CPoint &ptScreen, const CSize &szFloat);

    /**
     * @brief Get the dock bar hosted in this window
     */
    SDockBar *GetDockBar() const
    {
        return m_pDockBar;
    }

    /**
     * @brief Move the float window to a screen position
     */
    void MoveTo(const CPoint &ptTopLeft);

    /**
     * @brief Close the float window asynchronously
     * @details The window is closed through a posted WM_CLOSE so that the
     *          current message handler can return before the window is destroyed.
     */
    void RequestClose();

  protected:
    LRESULT OnClose(UINT uMsg, WPARAM wParam, LPARAM lParam);
    void OnFinalMessage(HWND hwnd) override;

  protected:
    SDockBar *m_pDockBar;

    BEGIN_MSG_MAP_EX(SDockFloatWnd)
        MESSAGE_HANDLER_EX(WM_CLOSE, OnClose)
        CHAIN_MSG_MAP(SHostWnd)
    END_MSG_MAP()
};

SNSEND

#endif /**< __SDOCKFLOATWND__H__ */
