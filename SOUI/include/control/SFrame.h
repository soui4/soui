#ifndef __SFRAME__H__
#define __SFRAME__H__

#include <core/SWnd.h>

SNSBEGIN

/**
 * @class SFrame
 * @brief Frame Control
 * @details A frame control that can contain other child windows.
 */
class SOUI_EXP SFrame : public SWindow {
    DEF_SOBJECT(SWindow, L"frame")

  public:
    /**
     * @brief Constructor
     */
    SFrame(void);

    /**
     * @brief Destructor
     */
    ~SFrame(void);

  public:
    /**
     * @brief Check if the control is focusable
     * @return TRUE if focusable, FALSE otherwise
     */
    STDMETHOD_(BOOL, IsFocusable)(THIS) SCONST OVERRIDE;

  protected:
    /**
     * @brief Find the window at a given point
     * @param pt Point to check
     * @param bIncludeMsgTransparent Whether to include message-transparent windows
     * @return Handle to the window at the point
     */
    SWND SwndFromPoint(CPoint &pt, BOOL bIncludeMsgTransparent) const override;
};

SNSEND

#endif // __SFRAME__H__