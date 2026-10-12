/**
 * @file       SInplaceEdit.cpp
 * @brief      Implementation of the shared in-place label editor.
 * @version    1.0
 *
 * @copyright  Copyright (C) 2012-2050 SOUI Team
 *             All rights reserved.
 */

#include "souistd.h"
#include "control/SInplaceEdit.h"

SNSBEGIN

SInplaceEdit::SInplaceEdit(IInplaceEditHost *pHost)
    : m_pHost(pHost)
    , m_bUpdateData(TRUE)
{
}

void SInplaceEdit::ApplyDefaultStyle()
{
    // Match the SGridCtrl default inplace editor look. Applied before
    // InitFromXml(NULL) so a custom style XML can override every attribute.
    SetAttribute(L"colorBkgnd", L"#FFFFFF", TRUE);
    SetAttribute(L"colorText", L"#000000", TRUE);
    SetAttribute(L"autoWordSel", L"1", TRUE);
    SetAttribute(L"margin", L"1", TRUE);
}

BOOL SInplaceEdit::ProcessSwndMessage(UINT uMsg, WPARAM wParam, LPARAM lParam, LRESULT &lResult)
{
    if (uMsg == WM_KEYDOWN)
    {
        UINT nChar = (UINT)wParam;
        if (nChar == VK_ESCAPE)
            m_bUpdateData = FALSE;
        if (nChar == VK_ESCAPE || nChar == VK_RETURN || nChar == VK_TAB)
        {
            // Hand focus back to the host control: the WM_KILLFOCUS that
            // follows ends the session, committing (RETURN / TAB) or
            // discarding (ESC) the text.
            SWindow *pParent = GetParent();
            if (pParent)
                pParent->SetFocus();
            lResult = 0;
            return TRUE;
        }
    }
    else if (uMsg == WM_KILLFOCUS)
    {
        // Keep the base cue-text handling, then end the session.
        SEdit::OnKillFocus((SWND)wParam);
        if (m_pHost)
            m_pHost->OnInplaceEditEnd(m_bUpdateData);
        lResult = 0;
        return TRUE;
    }
    return SEdit::ProcessSwndMessage(uMsg, wParam, lParam, lResult);
}

SNSEND
