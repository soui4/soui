#pragma once

#include <control/SCmnCtrl.h>
#include <control/STabCtrl.h>
#include <control/SRebar.h>
#include <res.mgr/SUiDef.h>

// Keep native SButton keyboard/command semantics; replace only its legacy
// dotted focus rectangle with a theme-aware Fluent ring in this executable.
class GalleryButton : public SNS::SButton {
    DEF_SOBJECT(SNS::SButton, L"gallerybutton")

  protected:
    void DrawFocus(SNS::IRenderTarget *target)
    {
        if (!IsFocusable() || !m_bDrawFocusRect)
            return;
        if (SNS::ISkinObj *focus = GETSKIN(L"gallery_focus", GetScale()))
        {
            CRect bounds;
            GetClientRect(&bounds);
            bounds.DeflateRect(2, 2);
            focus->DrawByIndex(target, bounds, 0);
        }
    }
};

// STabCtrl paints its focus inside OnPaint instead of calling DrawFocus.
// Keep its header/input implementation and decorate only the selected item.
class GalleryTabCtrl : public SNS::STabCtrl {
    DEF_SOBJECT(SNS::STabCtrl, L"gallerytabs")

  public:
    GalleryTabCtrl()
    {
        m_bDrawFocusRect = FALSE;
    }

  protected:
    void DrawItem(SNS::IRenderTarget *target, const CRect &item, int index, DWORD state)
    {
        SNS::STabCtrl::DrawItem(target, item, index, state);
        if (index == GetCurSel() && IsFocused() && IsFocusable())
        {
            if (SNS::ISkinObj *focus = GETSKIN(L"gallery_focus", GetScale()))
            {
                CRect bounds(item);
                bounds.DeflateRect(3, 3);
                focus->DrawByIndex(target, bounds, 0);
            }
        }
    }
};

// Core SRebar hard-codes light band grips/splitters. Retain its complete
// layout, child lifetime and drag implementation; override painting only.
class GalleryRebar : public SNS::SRebar {
    DEF_SOBJECT(SNS::SRebar, L"galleryrebar")

  protected:
    void OnPaint(SNS::IRenderTarget *target)
    {
        m_rcClient = GetClientRect();
        target->FillSolidRect(m_rcClient, GetStyle().m_crBg);
        for (size_t i = 0; i < m_bands.GetCount() && i < m_bandRects.GetCount(); ++i)
        {
            const CRect bounds = m_bandRects[i];
            if (!m_bands[i].bCollapsed)
            {
                // Match SRebar::HitTest's existing 8px/4px grip geometry.
                const CRect grip(bounds.left, bounds.top, bounds.left + 8, bounds.bottom);
                target->FillSolidRect(grip, m_crGripper);
                for (int y = grip.top + 4; y < grip.bottom - 4; y += 4)
                    target->FillSolidRect(CRect(grip.CenterPoint().x - 1, y, grip.CenterPoint().x + 1, y + 2), m_crGripperDot);
                if (bounds.Width() > 12)
                    target->FillSolidRect(CRect(bounds.right - 4, bounds.top, bounds.right, bounds.bottom), GetStyle().m_crBorder);
            }
            if (m_bDragging && !m_bResizing && static_cast<int>(i) == m_nDragBand)
            {
                const COLORREF accent = GETCOLOR(L"@color/fluent_accent");
                target->FillSolidRect(CRect(bounds.left, bounds.top, bounds.right, bounds.top + 1), accent);
                target->FillSolidRect(CRect(bounds.left, bounds.bottom - 1, bounds.right, bounds.bottom), accent);
                target->FillSolidRect(CRect(bounds.left, bounds.top, bounds.left + 1, bounds.bottom), accent);
                target->FillSolidRect(CRect(bounds.right - 1, bounds.top, bounds.right, bounds.bottom), accent);
            }
        }
    }
    SOUI_MSG_MAP_BEGIN()
        MSG_WM_PAINT_EX(OnPaint)
    SOUI_MSG_MAP_END()
};
