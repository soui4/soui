#include <souistd.h>
#include <control/SRebar.h>

SNSBEGIN

static const int kRebarGripperWidth = 8;
static const int kRebarSplitterWidth = 4; /**< Right-side separator bar of the band; drag to adjust width (VS style) */

SRebar::SRebar()
    : m_nDragBand(-1)
    , m_nHoverBand(-1)
    , m_nHoverPart(-1)
    , m_bDragging(false)
    , m_bResizing(false)
    , m_crGripper(RGBA(192, 192, 192, 255))
    , m_crGripperDot(RGBA(192, 192, 192, 255))
    , m_nDragPos(0)
    , m_nDragStartWidth(0)
    , m_bSingleRow(true)
{
    GetStyle().m_crBg = RGBA(245, 245, 245, 255);
    GetStyle().m_crBorder = RGBA(192, 192, 192, 255);
}

SRebar::~SRebar()
{
    m_bands.RemoveAll();
}

int SRebar::AddBand(SWindow *pChild, const SStringT &title, int height, bool bCollapsible)
{
    BandInfo band;
    band.title = title;
    band.height = height;
    band.bCollapsible = bCollapsible;
    band.bCollapsed = false;
    band.pChild = pChild;

    m_bands.Add(band);
    UpdateBandLayout();
    return m_bands.GetCount() - 1;
}

bool SRebar::RemoveBand(int nIndex)
{
    if (nIndex < 0 || nIndex >= (int)m_bands.GetCount())
        return false;

    if (m_bands[nIndex].pChild)
    {
        DestroyChild(m_bands[nIndex].pChild);
    }

    m_bands.RemoveAt(nIndex);
    UpdateBandLayout();
    return true;
}

bool SRebar::SetBandHeight(int nIndex, int height)
{
    if (nIndex < 0 || nIndex >= (int)m_bands.GetCount())
        return false;

    m_bands[nIndex].height = smax(m_bands[nIndex].minHeight, smin(m_bands[nIndex].maxHeight, height));
    UpdateBandLayout();
    return true;
}

bool SRebar::SetBandTitle(int nIndex, const SStringT &title)
{
    if (nIndex < 0 || nIndex >= (int)m_bands.GetCount())
        return false;

    m_bands[nIndex].title = title;
    Invalidate();
    return true;
}

bool SRebar::SetBandCollapsible(int nIndex, bool bCollapsible)
{
    if (nIndex < 0 || nIndex >= (int)m_bands.GetCount())
        return false;

    m_bands[nIndex].bCollapsible = bCollapsible;
    Invalidate();
    return true;
}

bool SRebar::CollapseBand(int nIndex, bool bCollapse)
{
    if (nIndex < 0 || nIndex >= (int)m_bands.GetCount())
        return false;

    BandInfo &band = m_bands[nIndex];
    if (!band.bCollapsible)
        return false;

    band.bCollapsed = bCollapse;
    UpdateBandLayout();
    return true;
}

bool SRebar::IsBandCollapsed(int nIndex) const
{
    if (nIndex < 0 || nIndex >= (int)m_bands.GetCount())
        return false;
    return m_bands[nIndex].bCollapsed;
}

int SRebar::GetBandCount() const
{
    return m_bands.GetCount();
}

SRebar::BandInfo *SRebar::GetBandInfo(int nIndex)
{
    if (nIndex < 0 || nIndex >= (int)m_bands.GetCount())
        return NULL;
    return &m_bands[nIndex];
}

BOOL SRebar::CreateChildren(SXmlNode xmlNode)
{
    bool bRet = TRUE;
    // Records the requested initial row for each band (parsed from the per-band
    // `row` attribute). -1 means "not specified" and defaults to row 0.
    SArray<int> rowOf;

    for (SXmlNode xmlChild = xmlNode.first_child(); xmlChild; xmlChild = xmlChild.next_sibling())
    {
        if (xmlChild.type() != node_element)
            continue;

        SStringW strName = xmlChild.name();
        if (strName == L"band")
        {
            SStringT title = S_CW2T(xmlChild.attribute(L"title").value());
            int height = xmlChild.attribute(L"height").as_int(24);
            bool bCollapsible = xmlChild.attribute(L"collapsible").as_bool(false);
            // Optional explicit band width. When omitted (0) the width is derived
            // from the child window's desired size (see CalculateBandLayout).
            int bandWidth = xmlChild.attribute(L"width").as_int(0);
            // Optional initial row (0-based). Only effective when singleRow is false.
            int bandRow = xmlChild.attribute(L"row").as_int(-1);

            SWindow *pChild = NULL;
            for (SXmlNode xmlBandChild = xmlChild.first_child(); xmlBandChild; xmlBandChild = xmlBandChild.next_sibling())
            {
                if (xmlBandChild.type() != node_element)
                    continue;
                pChild = CreateChildByName(xmlBandChild.name());
                if (pChild)
                {
                    InsertChild(pChild);
                    pChild->InitFromXml(&xmlBandChild);
                    int nIdx = AddBand(pChild, title, height, bCollapsible);
                    if (nIdx >= 0)
                    {
                        if (bandWidth > 0)
                            m_bands[nIdx].width = bandWidth;
                        rowOf.Add(bandRow);
                    }
                    break;
                }
            }
        }
        else
        {
            if (!CreateChild(xmlChild))
                bRet = FALSE;
        }
    }

    // Derive the bNewRow flags from the requested initial rows (only in multi-row
    // mode). A band starts a new row when its row index differs from the previous
    // band's; without a `row` attribute a band defaults to row 0 (single row).
    if (!m_bSingleRow)
    {
        int prevRow = -999;
        for (int i = 0; i < (int)m_bands.GetCount(); i++)
        {
            int r = (i < (int)rowOf.GetCount()) ? rowOf[i] : -1;
            if (r < 0)
                r = 0;
            if (i == 0)
                m_bands[i].bNewRow = true;
            else
                m_bands[i].bNewRow = (r != prevRow);
            prevRow = r;
        }
    }

    return bRet;
}

void SRebar::GetDesiredSize(SIZE *psz, int nParentWid, int nParentHei)
{
    ILayoutParam *pLayoutParam = GetLayoutParam();

    if (pLayoutParam->IsSpecifiedSize(Horz))
    {
        SLayoutSize layoutSize;
        pLayoutParam->GetSpecifiedSize(Horz, &layoutSize);
        psz->cx = layoutSize.toPixelSize(GetScale());
    }
    else if (pLayoutParam->IsMatchParent(Horz) && nParentWid >= 0)
    {
        psz->cx = nParentWid;
    }
    else
    {
        psz->cx = nParentWid > 0 ? nParentWid : 200;
    }

    if (pLayoutParam->IsSpecifiedSize(Vert))
    {
        SLayoutSize layoutSize;
        pLayoutParam->GetSpecifiedSize(Vert, &layoutSize);
        psz->cy = layoutSize.toPixelSize(GetScale());
    }
    else if (pLayoutParam->IsMatchParent(Vert) && nParentHei >= 0)
    {
        psz->cy = nParentHei;
    }
    else
    {
        // Height is computed from the “可用宽度” (available width), so the parent window only needs RequestRelayout +
        // Calling GetDesiredSize again yields the appropriate height of SRebar at the current width,
        // Without directly moving SRebar itself.
        int cxAvail = psz->cx > 0 ? psz->cx : (nParentWid > 0 ? nParentWid : GetClientRect().Width());
        psz->cy = GetTotalHeightForWidth(cxAvail);
    }
}

int SRebar::GetTotalHeightForWidth(int cx) const
{
    if (m_bands.IsEmpty())
        return 24;

    // Rows are separated by bands whose bNewRow flag is set. A single-row rebar is
    // simply one with no bNewRow set. A hidden band contributes no space, so we just
    // skip it (its bNewRow would have applied to the next visible band anyway).
    // The height is independent of the available width (VS Rebar logic).
    int y = 0;
    int rowHeight = 0;

    for (size_t i = 0; i < m_bands.GetCount(); i++)
    {
        if (!IsBandShown((int)i))
            continue;
        const BandInfo &band = m_bands[i];

        if (i > 0 && band.bNewRow && !m_bSingleRow)
        {
            y += rowHeight;
            rowHeight = 0;
        }
        rowHeight = smax(rowHeight, band.height);
    }
    return y + (rowHeight == 0 ? 24 : rowHeight);
}

int SRebar::GetTotalHeight() const
{
    if (m_bands.IsEmpty())
        return 24;
    // Height is determined by singleRow + bNewRow, independent of width
    return GetTotalHeightForWidth(0);
}

bool SRebar::IsBandShown(int i) const
{
    if (i < 0 || i >= (int)m_bands.GetCount())
        return false;
    const BandInfo &band = m_bands[i];
    if (band.bCollapsed || !band.pChild)
        return false;
    // Respect the child's own visibility so external Show/Hide (e.g. a toolbar
    // toggled by the "View" menu) hides the whole band.
    return band.pChild->IsVisible(FALSE);
}

void SRebar::CalculateBandLayout()
{
    for (size_t i = 0; i < m_bands.GetCount(); i++)
    {
        BandInfo &band = m_bands[i];
        if (band.pChild && !band.bCollapsed)
        {
            // First determine the band height from the child window's desired height, then clip according to the band's constraints,
            // So that “band size change” and “child control size” are linked.
            SIZE szChild = { 0, 0 };
            int nAvailWid = m_rcClient.Width() > 0 ? m_rcClient.Width() : -1;
            band.pChild->GetDesiredSize(&szChild, nAvailWid, -1);

            int desiredHei = szChild.cy > 0 ? szChild.cy : band.height;
            band.height = smax(band.minHeight, smin(band.maxHeight, desiredHei));

            // Only initialize band width from the child window's desired width when the user has not yet adjusted the width (width==0)
            if (band.width == 0)
            {
                int desiredWid = szChild.cx > 0 ? (szChild.cx + kRebarGripperWidth + kRebarSplitterWidth) : 80;
                band.width = smax(band.minWidth, smin(band.maxWidth, desiredWid));
            }
        }
        else
        {
            // When there is no child window, only constrain the existing height by smin/smax to avoid invalid heights
            band.height = smax(band.minHeight, smin(band.maxHeight, band.height));

            if (band.width == 0)
            {
                band.width = smax(band.minWidth, smin(band.maxWidth, 80));
            }
        }
    }
}

void SRebar::UpdateBandLayout()
{
    m_rcClient = GetClientRect();

    CalculateBandLayout();

    m_bandRects.SetCount(m_bands.GetCount());

    int x = m_rcClient.left + kRebarGripperWidth;
    int y = m_rcClient.top;
    int rowHeight = 0;

    for (size_t i = 0; i < m_bands.GetCount(); i++)
    {
        BandInfo &band = m_bands[i];
        bool bShow = IsBandShown((int)i);
        if (!bShow)
        {
            // Hidden band (collapsed / no child / child hidden): occupy no space,
            // leave an empty rect so HitTest skips it.
            m_bandRects[i].SetRectEmpty();
            if (band.pChild)
                band.pChild->SetVisible(FALSE);
            continue;
        }

        int bandHeight = band.height;
        int bandWidth = band.width > 0 ? band.width : 80;

        // Rows are broken by the bNewRow flag. When singleRow is locked, bNewRow is
        // ignored so the rebar stays on one row regardless. Hidden bands were skipped
        // above so they consume no space.
        if (i > 0 && band.bNewRow && !m_bSingleRow)
        {
            x = m_rcClient.left + kRebarGripperWidth;
            y += rowHeight;
            rowHeight = 0;
        }

        CRect rcBand(x - kRebarGripperWidth, y, x - kRebarGripperWidth + bandWidth, y + bandHeight);
        if (rcBand.right > m_rcClient.right)
            rcBand.right = m_rcClient.right;
        if (rcBand.left < m_rcClient.left)
            rcBand.left = m_rcClient.left;
        m_bandRects[i] = rcBand;

        if (band.pChild)
        {
            int xChildLeft = rcBand.left + kRebarGripperWidth;
            int xChildRight = rcBand.right - kRebarSplitterWidth;
            // The child window must not exceed the Rebar client area; reduce its width if it does
            int rightLimit = m_rcClient.right - kRebarSplitterWidth;
            if (xChildRight > rightLimit)
                xChildRight = rightLimit;
            if (xChildLeft < m_rcClient.left)
                xChildLeft = m_rcClient.left;
            if (xChildRight <= xChildLeft)
                xChildRight = xChildLeft + 1;
            CRect rcChild(xChildLeft, y, xChildRight, y + bandHeight);
            band.pChild->Move(rcChild);
            band.pChild->SetVisible(TRUE);
        }

        rowHeight = smax(rowHeight, bandHeight);
        x += bandWidth;
    }

    GetParent()->RequestRelayout();
}

void SRebar::UpdateChildrenPosition()
{
    // We fully own the positioning of band children (see UpdateBandLayout), so
    // the default SouiLayout must not reposition them by their own XML layout
    // params - that would undo the manual Move() above.
    UpdateBandLayout();
}

void SRebar::OnPaint(IRenderTarget *pRT)
{
    m_rcClient = GetClientRect();

    if (m_bands.IsEmpty())
    {
        pRT->FillSolidRect(m_rcClient, GetStyle().m_crBg);

        IPenS *pPen = NULL;
        if (SUCCEEDED(pRT->CreatePen(PS_SOLID, GetStyle().m_crBorder, 1, &pPen)))
        {
            IRenderObj *pOldPen = NULL;
            pRT->SelectObject(pPen, &pOldPen);
            pRT->DrawRectangle(m_rcClient);
            pRT->SelectObject(pOldPen);
            pPen->Release();
        }
        return;
    }

    for (size_t i = 0; i < m_bands.GetCount(); i++)
    {
        CRect rcBand = (i < m_bandRects.GetCount()) ? m_bandRects[i] : CRect(m_rcClient.left, m_rcClient.top, m_rcClient.left, m_rcClient.top);
        DrawBand(pRT, i, rcBand);
    }
}

void SRebar::DrawBand(IRenderTarget *pRT, int nIndex, const CRect &rcBand)
{
    BandInfo &band = m_bands[nIndex];

    // Normal background
    pRT->FillSolidRect(rcBand, GetStyle().m_crBg);

    // Highlight the band being dragged for reordering (not drawn while resizing width)
    if (m_bDragging && !m_bResizing && nIndex == m_nDragBand)
    {
        IPenS *pPen = NULL;
        COLORREF crHighlight = RGBA(0, 120, 215, 255); // Close to VS selection blue
        if (SUCCEEDED(pRT->CreatePen(PS_SOLID, crHighlight, 1, &pPen)))
        {
            IRenderObj *pOldPen = NULL;
            pRT->SelectObject(pPen, &pOldPen);
            pRT->DrawRectangle(rcBand);
            pRT->SelectObject(pOldPen);
            pPen->Release();
        }
    }

    if (!band.bCollapsed)
    {
        // VS style: left gripper (drag to reorder)
        CRect rcGripper(rcBand.left, rcBand.top, rcBand.left + kRebarGripperWidth, rcBand.bottom);
        DrawGripper(pRT, rcGripper);
        // Right splitter (drag to resize width)
        if (rcBand.Width() > kRebarGripperWidth + kRebarSplitterWidth)
        {
            CRect rcSplitter(rcBand.right - kRebarSplitterWidth, rcBand.top, rcBand.right, rcBand.bottom);
            pRT->FillSolidRect(rcSplitter, RGBA(230, 230, 230, 255));
            IPenS *pPen = NULL;
            if (SUCCEEDED(pRT->CreatePen(PS_SOLID, GetStyle().m_crBorder, 1, &pPen)))
            {
                IRenderObj *pOldPen = NULL;
                pRT->SelectObject(pPen, &pOldPen);
                POINT ptS = { rcSplitter.left, rcSplitter.top };
                POINT ptE = { rcSplitter.left, rcSplitter.bottom };
                pRT->DrawLine(ptS, ptE);
                pRT->SelectObject(pOldPen);
                pPen->Release();
            }
        }
    }
}

void SRebar::DrawGripper(IRenderTarget *pRT, const CRect &rcGripper)
{
    // The gripper strip background is driven by the `colorGripper` attribute
    // (m_crGripper), so it can be made to match the rebar's own background.
    pRT->FillSolidRect(rcGripper, m_crGripper);

    IPenS *pPen = NULL;
    if (SUCCEEDED(pRT->CreatePen(PS_SOLID, GetStyle().m_crBorder, 1, &pPen)))
    {
        IRenderObj *pOldPen = NULL;
        pRT->SelectObject(pPen, &pOldPen);

        POINT ptStart1 = { rcGripper.left, rcGripper.top };
        POINT ptEnd1 = { rcGripper.left, rcGripper.bottom };
        pRT->DrawLine(ptStart1, ptEnd1);

        pRT->SelectObject(pOldPen);
        pPen->Release();
    }

    int cx = rcGripper.left + rcGripper.Width() / 2;
    for (int y = rcGripper.top + 4; y < rcGripper.bottom - 4; y += 4)
    {
        CRect rcDot(cx - 1, y, cx + 2, y + 2);
        pRT->FillSolidRect(rcDot, m_crGripperDot);
    }
}

int SRebar::HitTest(CPoint point, int &nHitTest) const
{
    nHitTest = -1;

    for (size_t i = 0; i < m_bands.GetCount(); i++)
    {
        const BandInfo &band = m_bands[i];
        if (i >= m_bandRects.GetCount())
            continue;

        CRect rcBand = m_bandRects[i];

        if (rcBand.PtInRect(point))
        {
            if (!band.bCollapsed)
            {
                CRect rcGripper(rcBand.left, rcBand.top, rcBand.left + kRebarGripperWidth, rcBand.bottom);
                if (rcGripper.PtInRect(point))
                {
                    nHitTest = 2; // gripper: drag to reorder
                    return i;
                }
                CRect rcSplitter(rcBand.right - kRebarSplitterWidth, rcBand.top, rcBand.right, rcBand.bottom);
                if (rcSplitter.PtInRect(point))
                {
                    nHitTest = 3; // splitter: drag to resize width
                    return i;
                }
            }

            nHitTest = 0;
            return i;
        }
    }

    return -1;
}

void SRebar::OnSize(UINT nType, CSize size)
{
    UpdateBandLayout();
}

BOOL SRebar::OnMouseHover(UINT nFlags, CPoint point)
{
    int nHitTest = -1;
    int nBand = HitTest(point, nHitTest);

    if (nBand != m_nHoverBand || nHitTest != m_nHoverPart)
    {
        m_nHoverBand = nBand;
        m_nHoverPart = nHitTest;
        Invalidate();
    }
    return TRUE;
}

BOOL SRebar::OnMouseLeave()
{
    if (m_nHoverBand != -1 || m_nHoverPart != -1)
    {
        m_nHoverBand = -1;
        m_nHoverPart = -1;
        Invalidate();
    }

    return TRUE;
}

BOOL SRebar::OnLButtonDown(UINT nFlags, CPoint point)
{
    int nHitTest = -1;
    int nBand = HitTest(point, nHitTest);

    if (nBand != -1)
    {
        if (nHitTest == 2)
        {
            // gripper: only for drag reordering (VS logic)
            m_nDragBand = nBand;
            m_bResizing = false;
            m_bDragging = true;
            SetCapture();
        }
        else if (nHitTest == 3)
        {
            // splitter: only for adjusting this band's width (VS logic)
            m_nDragBand = nBand;
            m_nDragPos = point.x;
            m_nDragStartWidth = 0;
            if (nBand < (int)m_bands.GetCount())
            {
                BandInfo &band = m_bands[nBand];
                m_nDragStartWidth = band.width > 0 ? band.width : (nBand < (int)m_bandRects.GetCount() ? m_bandRects[nBand].Width() : 80);
            }
            m_bResizing = true;
            m_bDragging = true;
            SetCapture();
        }
    }

    return TRUE;
}

BOOL SRebar::OnLButtonUp(UINT nFlags, CPoint point)
{
    if (m_bDragging)
    {
        m_bDragging = false;
        m_bResizing = false;
        ReleaseCapture();
        // After single/multi-row or row-count change, notify the parent to relayout (height may have changed)
        GetParent()->RequestRelayout();
    }

    return TRUE;
}

BOOL SRebar::CancelCaptureMode(int reason)
{
    if (reason == CANCEL_REASON_SCROLL && m_bDragging)
        return FALSE;
    return __baseCls::CancelCaptureMode(reason);
}

BOOL SRebar::OnMouseMove(UINT nFlags, CPoint point)
{
    if (!m_bDragging || m_nDragBand < 0)
        return TRUE;

    // Width adjustment logic (unchanged)
    if (m_bResizing)
    {
        if (m_nDragBand >= (int)m_bands.GetCount())
            return TRUE;
        BandInfo &band = m_bands[m_nDragBand];
        int deltaX = point.x - m_nDragPos;
        int newWidth = smax(band.minWidth, smin(band.maxWidth, m_nDragStartWidth + deltaX));
        if (newWidth != band.width)
        {
            band.width = newWidth;
            UpdateBandLayout();
            Invalidate();
        }
        return TRUE;
    }

    // Reordering logic.
    // Dragging a band's gripper can move it to a different slot within a row (by X),
    // or to a different row. Dropping it above the first row or below the last row
    // (or into the vertical gap between rows) creates a NEW row - i.e. the rebar
    // becomes multi-row on demand, just like MFC's CRebar.
    int bandCount = (int)m_bands.GetCount();
    if (bandCount < 2)
        return TRUE;

    if ((int)m_bandRects.GetCount() != bandCount)
        UpdateBandLayout();

    int dragBand = m_nDragBand;
    if (dragBand < 0 || dragBand >= bandCount)
        return TRUE;

    // Locked single-row mode: only reorder within the one row; never create a new
    // row. The single-row invariant is preserved by keeping bNewRow true only on
    // the first band.
    if (m_bSingleRow)
    {
        SWindow *pDragChild = m_bands[dragBand].pChild;
        SArray<int> order;
        for (int i = 0; i < bandCount; ++i)
            if (i != dragBand)
                order.Add(i);

        int pos = (int)order.GetCount();
        for (int k = 0; k < (int)order.GetCount(); ++k)
        {
            int b = order[k];
            if (b >= (int)m_bandRects.GetCount())
                continue;
            int midX = (m_bandRects[b].left + m_bandRects[b].right) / 2;
            if (point.x < midX)
            {
                pos = k;
                break;
            }
            pos = k + 1;
        }
        order.InsertAt(pos, dragBand);

        SArray<BandInfo> ordered;
        for (int k = 0; k < (int)order.GetCount(); ++k)
            ordered.Add(m_bands[order[k]]);
        m_bands.RemoveAll();
        for (int k = 0; k < (int)ordered.GetCount(); ++k)
            m_bands.Add(ordered[k]);
        for (int k = 0; k < (int)m_bands.GetCount(); ++k)
            m_bands[k].bNewRow = false;
        if (m_bands.GetCount() > 0)
            m_bands[0].bNewRow = true;

        for (int k = 0; k < (int)m_bands.GetCount(); ++k)
        {
            if (m_bands[k].pChild == pDragChild)
            {
                m_nDragBand = k;
                break;
            }
        }
        UpdateBandLayout();
        return TRUE;
    }

    // 1) Compute current rows (row starters) from the bNewRow flags.
    SArray<int> rowStarts;
    rowStarts.Add(0);
    for (int i = 1; i < bandCount; ++i)
    {
        if (m_bands[i].bNewRow)
            rowStarts.Add(i);
    }
    int numRows = (int)rowStarts.GetCount();

    // 2) Row rectangles for vertical (Y) hit testing.
    SArray<CRect> rowRects;
    for (int r = 0; r < numRows; ++r)
    {
        int s = rowStarts[r];
        int e = (r + 1 < numRows) ? rowStarts[r + 1] - 1 : bandCount - 1;
        CRect rcRow;
        rcRow.SetRectEmpty();
        for (int i = s; i <= e; ++i)
        {
            if (i < (int)m_bandRects.GetCount() && !m_bandRects[i].IsRectEmpty())
            {
                if (rcRow.IsRectEmpty())
                    rcRow = m_bandRects[i];
                else
                    rcRow.UnionRect(rcRow, m_bandRects[i]);
            }
        }
        rowRects.Add(rcRow);
    }
    if (numRows <= 0)
        return TRUE;

    // 3) Determine the target row from the mouse Y.
    bool bNewRowAbove = false;
    bool bNewRowBelow = false;
    int targetRow = -2; // -2 unset, -1 new row above, >=numRows new row below
    if (point.y < rowRects[0].top)
    {
        targetRow = -1;
        bNewRowAbove = true;
    }
    else if (point.y > rowRects[numRows - 1].bottom)
    {
        targetRow = numRows;
        bNewRowBelow = true;
    }
    else
    {
        for (int r = 0; r < numRows; ++r)
        {
            if (point.y <= rowRects[r].bottom)
            {
                targetRow = r;
                break;
            }
        }
        if (targetRow == -2)
        {
            targetRow = numRows;
            bNewRowBelow = true;
        }
    }

    // 4) For an existing target row, compute the X insertion position.
    SArray<int> rowBands;
    if (!bNewRowAbove && !bNewRowBelow && targetRow >= 0 && targetRow < numRows)
    {
        int s = rowStarts[targetRow];
        int e = (targetRow + 1 < numRows) ? rowStarts[targetRow + 1] - 1 : bandCount - 1;
        for (int i = s; i <= e; ++i)
        {
            if (i == dragBand)
                continue;
            rowBands.Add(i);
        }
        int pos = (int)rowBands.GetCount();
        for (int k = 0; k < (int)rowBands.GetCount(); ++k)
        {
            int b = rowBands[k];
            if (b >= (int)m_bandRects.GetCount())
                continue;
            int midX = (m_bandRects[b].left + m_bandRects[b].right) / 2;
            if (point.x < midX)
            {
                pos = k;
                break;
            }
            pos = k + 1;
        }
        rowBands.InsertAt(pos, dragBand);
    }

    // 5) Build the new band order and the list of row starters.
    SArray<int> newOrder;
    SArray<int> newStarts;
    SWindow *pDragChild = m_bands[dragBand].pChild;

    if (bNewRowAbove)
    {
        newOrder.Add(dragBand);
        newStarts.Add(0);
    }

    for (int r = 0; r < numRows; ++r)
    {
        int s = rowStarts[r];
        int e = (r + 1 < numRows) ? rowStarts[r + 1] - 1 : bandCount - 1;

        if (!bNewRowAbove && !bNewRowBelow && r == targetRow)
        {
            // Target row: use the reordered rowBands (dragBand already inserted).
            if ((int)rowBands.GetCount() > 0)
            {
                int groupStart = (int)newOrder.GetCount();
                for (int k = 0; k < (int)rowBands.GetCount(); ++k)
                    newOrder.Add(rowBands[k]);
                newStarts.Add(groupStart);
            }
        }
        else
        {
            // Keep the original row, but drop the dragged band (it moved out).
            int groupStart = (int)newOrder.GetCount();
            bool bHas = false;
            for (int i = s; i <= e; ++i)
            {
                if (i == dragBand)
                    continue;
                newOrder.Add(i);
                bHas = true;
            }
            if (bHas)
                newStarts.Add(groupStart);
        }
    }

    if (bNewRowBelow)
    {
        int groupStart = (int)newOrder.GetCount();
        newOrder.Add(dragBand);
        newStarts.Add(groupStart);
    }

    if ((int)newOrder.GetCount() != bandCount)
    {
        // A row became empty or the dragged band was lost - bail out safely.
        return TRUE;
    }

    // 6) Apply: reorder m_bands and refresh bNewRow flags from the new starters.
    SArray<BandInfo> ordered;
    for (int k = 0; k < (int)newOrder.GetCount(); ++k)
        ordered.Add(m_bands[newOrder[k]]);

    m_bands.RemoveAll();
    for (int k = 0; k < (int)ordered.GetCount(); ++k)
        m_bands.Add(ordered[k]);

    for (int k = 0; k < (int)m_bands.GetCount(); ++k)
        m_bands[k].bNewRow = false;
    for (int k = 0; k < (int)newStarts.GetCount(); ++k)
        m_bands[newStarts[k]].bNewRow = true;

    // Locate the dragged band again after the reorder.
    for (int k = 0; k < (int)m_bands.GetCount(); ++k)
    {
        if (m_bands[k].pChild == pDragChild)
        {
            m_nDragBand = k;
            break;
        }
    }

    UpdateBandLayout();
    return TRUE;
}
BOOL SRebar::OnSetCursor(const CPoint &pt)
{
    int nHitTest = -1;
    int nBand = HitTest(pt, nHitTest);

    if (nBand != -1)
    {
        if (nHitTest == 2)
        {
            SetCursor(SApplication::getSingleton().LoadCursor(IDC_SIZEALL)); // gripper: reorder
            return TRUE;
        }
        if (nHitTest == 3)
        {
            SetCursor(SApplication::getSingleton().LoadCursor(IDC_SIZEWE)); // splitter: adjust width
            return TRUE;
        }
        if (nHitTest == 0)
        {
            SetCursor(SApplication::getSingleton().LoadCursor(IDC_ARROW));
            return TRUE;
        }
    }

    return __baseCls::OnSetCursor(pt);
}

SNSEND
