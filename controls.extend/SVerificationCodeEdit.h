#ifndef _SVERIFICATIONCODEEDIT_H_
#define _SVERIFICATIONCODEEDIT_H_

#include <SEdit2.h>

SNSBEGIN

/**
 * @class SVerificationCodeEdit
 * @brief Verification Code Edit Control
 * @details A control that allows users to input verification codes with visual character boxes.
 */
class SVerificationCodeEdit : public SEdit2 {
    DEF_SOBJECT(SEdit2, L"verificationCodeEdit")

public:
    /**
     * @brief Constructor
     */
    SVerificationCodeEdit();

    /**
     * @brief Destructor
     */
    ~SVerificationCodeEdit();

protected:
    SIZE MeasureContent(int nParentWid, int nParentHei) override;
    /**
     * @brief Paint the control
     * @param pRT Rendering target handle
     */
    void OnPaint(IRenderTarget *pRT);

    /**
     * @brief Handle keyboard input
     * @param nChar Key code
     * @param nRepCnt Repeat count
     * @param nFlags Flags
     */
    void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);

    /**
     * @brief Handle mouse left button down event
     * @param flags Flags
     * @param pt Mouse coordinates
     */
    void OnLButtonDown(UINT flags, CPoint pt);

    /**
     * @brief Handle mouse left button up event
     * @param flags Flags
     * @param pt Mouse coordinates
     */
    void OnLButtonUp(UINT flags, CPoint pt);

    /**
     * @brief Handle mouse move event
     * @param flags Flags
     * @param pt Mouse coordinates
     */
    void OnMouseMove(UINT flags, CPoint pt);

    /**
     * @brief Handle set focus event
     * @param wndOld Previous focus window handle
     */
    void OnSetFocus(SWND wndOld);

    /**
     * @brief Handle kill focus event
     * @param wndFocus New focus window handle
     */
    void OnKillFocus(SWND wndFocus);

    /**
     * @brief Handle text change event
     */
    void OnTextChanged();

    SOUI_MSG_MAP_BEGIN()
        MSG_WM_PAINT_EX(OnPaint)
        MSG_WM_KEYDOWN(OnKeyDown)
        MSG_WM_LBUTTONDOWN(OnLButtonDown)
        MSG_WM_LBUTTONUP(OnLButtonUp)
        MSG_WM_MOUSEMOVE(OnMouseMove)
        MSG_WM_SETFOCUS_EX(OnSetFocus)
        MSG_WM_KILLFOCUS_EX(OnKillFocus)
    SOUI_MSG_MAP_END()

    SOUI_ATTRS_BEGIN()
        ATTR_COLOR(L"boxColor", m_crBox, TRUE)
        ATTR_COLOR(L"activeBoxColor", m_crActiveBox, TRUE)
        ATTR_COLOR(L"textColor", m_crText, TRUE)
        ATTR_INT(L"boxSpacing", m_nBoxSpacing, TRUE)
        ATTR_INT(L"boxWidth", m_nBoxWidth, TRUE)
        ATTR_INT(L"boxHeight", m_nBoxHeight, TRUE)
    SOUI_ATTRS_END()

protected:
    /**
     * @brief Draw character boxes
     * @param pRT Rendering target handle
     */
    void DrawCharBoxes(IRenderTarget *pRT);

    /**
     * @brief Update cursor position to the end
     */
    void UpdateCursorPosition();

protected:
    COLORREF m_crBox;          /**< Normal box color */
    COLORREF m_crActiveBox;     /**< Active box color */
    COLORREF m_crText;          /**< Text color */
    int m_nBoxSpacing;          /**< Spacing between boxes */
    int m_nBoxWidth;            /**< Width of each box */
    int m_nBoxHeight;           /**< Height of each box */
};

SNSEND

#endif // _SVERIFICATIONCODEEDIT_H_