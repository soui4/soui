/**
 * @file SCaption.h
 * @brief Caption control
 * @version v1.0
 * @author soui
 * @date 2014-05-28
 *
 * @details This class implements the caption control.
 */

#ifndef __SCAPTION__H__
#define __SCAPTION__H__

#include "core/SWnd.h"

SNSBEGIN

/**
 * @class SCaption
 * @brief Caption control class
 * @details This class implements a caption control, supporting mouse event handling.
 */
class SOUI_EXP SCaption : public SWindow {
    DEF_SOBJECT(SWindow, L"caption")

  public:
    /**
     * @brief Constructor
     * @details Initialize the caption control.
     */
    SCaption(void);

    /**
     * @brief Destructor
     * @details Release the caption control resources.
     */
    virtual ~SCaption(void);

  protected:
    /**
     * @brief Handle left button down event
     * @param nFlags flags
     * @param point mouse position
     * @details This is a message handler that handles the left button down event.
     */
    void OnLButtonDown(UINT nFlags, CPoint point);

    /**
     * @brief Handle left button double-click event
     * @param nFlags flags
     * @param point mouse position
     * @details This is a message handler that handles the left button double-click event.
     */
    void OnLButtonDblClk(UINT nFlags, CPoint point);

    /**
     * @brief Handle mouse move event
     * @param nFlags flags
     * @param point mouse position
     * @details This is a message handler that handles the mouse move event.
     */
    void OnMouseMove(UINT nFlags, CPoint point);

    /**
     * @brief Handle left button up event
     * @param nFlags flags
     * @param point mouse position
     * @details This is a message handler that handles the left button up event.
     */
    void OnLButtonUp(UINT nFlags, CPoint point);

    SOUI_MSG_MAP_BEGIN()
        MSG_WM_LBUTTONDOWN(OnLButtonDown)
        MSG_WM_LBUTTONDBLCLK(OnLButtonDblClk)
        MSG_WM_MOUSEMOVE(OnMouseMove)
        MSG_WM_LBUTTONUP(OnLButtonUp)
    SOUI_MSG_MAP_END()

  private:
    /**
     * @brief Flag indicating whether the left mouse button is pressed when maximized
     */
    BOOL m_bIsMaxDown;
};

SNSEND

#endif /**< __SCAPTION__H__ */