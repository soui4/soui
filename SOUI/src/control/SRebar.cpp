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
    , m_bVertical(false)
{
    GetStyle().m_crBg = RGBA(245, 245, 245, 255);
    GetStyle().m_crBorder = RGBA(192, 192, 192, 255);
}

SRebar::~SRebar()
{
    m_bands.RemoveAll();
}

void SRebar::SetVertical(bool bVertical)
{
    if (m_bVertical == bVertical)
        return;

    m_bVertical = bVertical;
    // A drag in progress was computed against the old geometry.
    if (m_bDragging)
    {
        m_bDragging = false;
        m_bResizing = false;
        ReleaseCapture();
    }
    UpdateBandLayout();
    Invalidate();
    if (GetParent())
        GetParent()->RequestRelayout();
}

void SRebar::GetBandOrder(SArray<SStringT> *titles) const
{
    if (!titles)
        return;
    titles->RemoveAll();
    for (size_t i = 0; i < m_bands.GetCount(); i++)
        titles->Add(m_bands[i].title);
}

bool SRebar::SetBandOrder(const SArray<SStringT> &titles)
{
    if (m_bands.IsEmpty() || titles.IsEmpty())
        return false;

    // Rebuild in the requested order. Titles that are absent from the current
    // set are ignored, so an order saved by an older build (or one that mentions
    // a band this build does not have) cannot lose bands. A title may only claim
    // one band: bands are not guaranteed to have unique titles, and a repeated
    // title must not duplicate one band while dropping another.
    SArray<BandInfo> pool;
    pool.Copy(m_bands);

    SArray<BandInfo> ordered;
    for (size_t i = 0; i < titles.GetCount(); i++)
    {
        for (size_t j = 0; j < pool.GetCount(); j++)
        {
            if (pool[j].title == titles[i])
            {
                ordered.Add(pool[j]);
                pool.RemoveAt(j);
                break;
            }
        }
    }
    // Bands the list does not mention keep their relative order at the end.
    for (size_t j = 0; j < pool.GetCount(); j++)
        ordered.Add(pool[j]);

    if (ordered.GetCount() != m_bands.GetCount())
        return false;

    bool changed = false;
    for (size_t i = 0; i < ordered.GetCount(); i++)
    {
        if (ordered[i].pChild != m_bands[i].pChild)
        {
            changed = true;
            break;
        }
    }
    if (!changed)
        return false;

    m_bands = ordered;
    m_nDragBand = -1;
    UpdateBandLayout();
    Invalidate();
    return true;
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
    if (!m_bSingleRow && !m_bVertical)
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

CRect SRebar::GetGripperRect(const CRect &rcBand) const
{
    // The handle runs along the band's leading edge, so it is a vertical strip
    // in a horizontal rebar and a horizontal strip in a vertical one.
    if (m_bVertical)
        return CRect(rcBand.left, rcBand.top, rcBand.right, rcBand.top + kRebarGripperWidth);
    return CRect(rcBand.left, rcBand.top, rcBand.left + kRebarGripperWidth, rcBand.bottom);
}

CRect SRebar::GetSplitterRect(const CRect &rcBand) const
{
    if (m_bVertical)
        return CRect(rcBand.left, rcBand.bottom - kRebarSplitterWidth, rcBand.right, rcBand.bottom);
    return CRect(rcBand.right - kRebarSplitterWidth, rcBand.top, rcBand.right, rcBand.bottom);
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

    // Vertical: a single column, so the height is simply the sum of the bands.
    if (m_bVertical)
    {
        int total = 0;
        for (size_t i = 0; i < m_bands.GetCount(); i++)
        {
            if (!IsBandShown((int)i))
                continue;
            total += m_bands[i].height;
        }
        return total > 0 ? total : 24;
    }

    if (m_bSingleRow)
    {
        // Single row: all bands on one row, height is the max height of a single row
        int rowHeight = 0;
        for (size_t i = 0; i < m_bands.GetCount(); i++)
        {
            if (!IsBandShown((int)i))
                continue;
            rowHeight = smax(rowHeight, m_bands[i].height);
        }
        return rowHeight > 0 ? rowHeight : 24;
    }

    // Multi-row: line breaks determined by bNewRow, independent of width (VS logic).
    // A hidden band contributes no space, so skip it.
    int y = 0;
    int rowHeight = 0;

    for (size_t i = 0; i < m_bands.GetCount(); i++)
    {
        if (!IsBandShown((int)i))
            continue;
        const BandInfo &band = m_bands[i];

        if (i > 0 && band.bNewRow)
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
    // A child hidden by Expand/Collapse is ours to show again; a child hidden by the
    // host (e.g. a toolbar toggled from a "View" menu) must stay hidden.
    if (band.bChildHiddenByRebar)
        return true;
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
            // Vertical mode insets the child by the gripper strip above and the splitter
            // below, so the band has to reserve those pixels on top of the height the
            // child asked for -- otherwise the last button of every band is clipped.
            if (m_bVertical)
                desiredHei += kRebarGripperWidth + kRebarSplitterWidth;
            // Vertical mode: the band height plays the role the band width plays in
            // horizontal mode, so once the user dragged the splitter we stop deriving
            // it from the child -- otherwise the next layout pass undoes the drag.
            if (!(m_bVertical && band.bHeightUserSet))
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

        // Vertical mode deliberately leaves band.width alone: the column width is the
        // rebar's client width and UpdateBandLayout uses that directly. Writing it here
        // would destroy the horizontal width and the bands would come back one button
        // wide when the user switches back to horizontal.
    }
}

void SRebar::UpdateBandLayout()
{
    m_rcClient = GetClientRect();

    // Collapse hides the band child, expanding shows it again. A child the host hid
    // itself (a toolbar toggled from a "View" menu) is left alone, otherwise every
    // layout pass would resurrect it.
    for (size_t i = 0; i < m_bands.GetCount(); i++)
    {
        BandInfo &band = m_bands[i];
        if (!band.pChild)
            continue;
        if (band.bCollapsed)
        {
            if (band.pChild->IsVisible(FALSE))
            {
                band.pChild->SetVisible(FALSE);
                band.bChildHiddenByRebar = true;
            }
        }
        else if (band.bChildHiddenByRebar)
        {
            band.pChild->SetVisible(TRUE);
            band.bChildHiddenByRebar = false;
        }
    }

    CalculateBandLayout();

    m_bandRects.SetCount(m_bands.GetCount());

    // Vertical: one column, bands stacked top-to-bottom and each one as wide as
    // the rebar. The gripper strip of a band is above its content and the
    // splitter below it, mirroring the left/right pair of the horizontal mode.
    if (m_bVertical)
    {
        int y = m_rcClient.top;
        for (size_t i = 0; i < m_bands.GetCount(); i++)
        {
            BandInfo &band = m_bands[i];
            if (!IsBandShown((int)i))
            {
                m_bandRects[i].SetRectEmpty();
                continue;
            }

            int bandHeight = band.height;

            if (y + bandHeight > m_rcClient.bottom)
            {
                // Out of room: drop the remaining bands instead of emitting rects whose
                // top has run past their clamped bottom.
                CRect rcBand(m_rcClient.left, y, m_rcClient.right, m_rcClient.bottom);
                if (rcBand.Height() > kRebarGripperWidth + kRebarSplitterWidth)
                {
                    m_bandRects[i] = rcBand;
                    int yChildTop = rcBand.top + kRebarGripperWidth;
                    int yChildBottom = rcBand.bottom - kRebarSplitterWidth;
                    CRect rcChild(rcBand.left, yChildTop, rcBand.right, yChildBottom);
                    band.pChild->Move(rcChild);
                }
                for (size_t j = i + 1; j < m_bands.GetCount(); j++)
                    m_bandRects[j].SetRectEmpty();
                break;
            }

            CRect rcBand(m_rcClient.left, y, m_rcClient.right, y + bandHeight);
            m_bandRects[i] = rcBand;

            if (band.pChild)
            {
                int yChildTop = rcBand.top + kRebarGripperWidth;
                int yChildBottom = rcBand.bottom - kRebarSplitterWidth;
                if (yChildBottom <= yChildTop)
                    yChildBottom = yChildTop + 1;
                CRect rcChild(rcBand.left, yChildTop, rcBand.right, yChildBottom);
                band.pChild->Move(rcChild);
            }

            y += bandHeight;
        }

        if (GetParent())
            GetParent()->RequestRelayout();
        return;
    }

    int x = m_rcClient.left + kRebarGripperWidth;
    int y = m_rcClient.top;
    int rowHeight = 0;

    for (size_t i = 0; i < m_bands.GetCount(); i++)
    {
        BandInfo &band = m_bands[i];
        if (!IsBandShown((int)i))
        {
            // Hidden band (collapsed / no child / child hidden by the host): occupy no
            // space, leave an empty rect so HitTest skips it.
            m_bandRects[i].SetRectEmpty();
            continue;
        }

        int bandHeight = band.height;
        int bandWidth = band.width > 0 ? band.width : 80;

        // Single row: the bar is a horizontal strip, so let every band take the full
        // height of that strip and centre its content there (the child's ygravity does
        // the centring). band.height then only matters as the row height in multi-row mode.
        if (m_bSingleRow && bandHeight < m_rcClient.Height())
            bandHeight = m_rcClient.Height();

        // Multi-row: line breaks only by bNewRow (VS logic, independent of width)
        if (!m_bSingleRow && i > 0 && band.bNewRow)
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
        }

        rowHeight = smax(rowHeight, bandHeight);
        x += bandWidth;
    }

    if (GetParent())
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
        // VS style: the gripper runs along the leading edge, drag to reorder
        CRect rcGripper = GetGripperRect(rcBand);
        DrawGripper(pRT, rcGripper);
        // The splitter sits on the trailing edge; drag to resize
        bool bRoomForSplitter = m_bVertical ? (rcBand.Height() > kRebarGripperWidth + kRebarSplitterWidth) : (rcBand.Width() > kRebarGripperWidth + kRebarSplitterWidth);
        if (bRoomForSplitter)
        {
            CRect rcSplitter = GetSplitterRect(rcBand);
            pRT->FillSolidRect(rcSplitter, RGBA(230, 230, 230, 255));
            IPenS *pPen = NULL;
            if (SUCCEEDED(pRT->CreatePen(PS_SOLID, GetStyle().m_crBorder, 1, &pPen)))
            {
                IRenderObj *pOldPen = NULL;
                pRT->SelectObject(pPen, &pOldPen);
                POINT ptS, ptE;
                if (m_bVertical)
                {
                    ptS.x = rcSplitter.left;
                    ptS.y = rcSplitter.top;
                    ptE.x = rcSplitter.right;
                    ptE.y = rcSplitter.top;
                }
                else
                {
                    ptS.x = rcSplitter.left;
                    ptS.y = rcSplitter.top;
                    ptE.x = rcSplitter.left;
                    ptE.y = rcSplitter.bottom;
                }
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

        POINT ptStart1, ptEnd1;
        if (m_bVertical)
        {
            // Horizontal strip: the separator runs along its bottom edge.
            ptStart1.x = rcGripper.left;
            ptStart1.y = rcGripper.bottom;
            ptEnd1.x = rcGripper.right;
            ptEnd1.y = rcGripper.bottom;
        }
        else
        {
            ptStart1.x = rcGripper.left;
            ptStart1.y = rcGripper.top;
            ptEnd1.x = rcGripper.left;
            ptEnd1.y = rcGripper.bottom;
        }
        pRT->DrawLine(ptStart1, ptEnd1);

        pRT->SelectObject(pOldPen);
        pPen->Release();
    }

    // Dots run along the strip's long axis.
    if (m_bVertical)
    {
        int cy = rcGripper.top + rcGripper.Height() / 2;
        for (int x = rcGripper.left + 4; x < rcGripper.right - 4; x += 4)
        {
            CRect rcDot(x, cy - 1, x + 2, cy + 2);
            pRT->FillSolidRect(rcDot, m_crGripperDot);
        }
    }
    else
    {
        int cx = rcGripper.left + rcGripper.Width() / 2;
        for (int y = rcGripper.top + 4; y < rcGripper.bottom - 4; y += 4)
        {
            CRect rcDot(cx - 1, y, cx + 2, y + 2);
            pRT->FillSolidRect(rcDot, m_crGripperDot);
        }
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
                CRect rcGripper = GetGripperRect(rcBand);
                if (rcGripper.PtInRect(point))
                {
                    nHitTest = 2; // gripper: drag to reorder
                    return i;
                }
                CRect rcSplitter = GetSplitterRect(rcBand);
                if (rcSplitter.PtInRect(point))
                {
                    nHitTest = 3; // splitter: drag to resize
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
            // splitter: only for adjusting this band's size (VS logic)
            m_nDragBand = nBand;
            m_nDragPos = m_bVertical ? point.y : point.x;
            m_nDragStartWidth = 0;
            if (nBand < (int)m_bands.GetCount())
            {
                BandInfo &band = m_bands[nBand];
                if (m_bVertical)
                    m_nDragStartWidth = band.height;
                else
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
        if (GetParent())
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

    int bandCount = (int)m_bands.GetCount();
    if (m_nDragBand >= bandCount)
        return TRUE;

    // Resize logic (drag the splitter along the rebar's main axis)
    if (m_bResizing)
    {
        BandInfo &band = m_bands[m_nDragBand];
        int delta = (m_bVertical ? point.y : point.x) - m_nDragPos;

        if (m_bVertical)
        {
            int newHeight = smax(band.minHeight, smin(band.maxHeight, m_nDragStartWidth + delta));
            if (newHeight != band.height)
            {
                band.height = newHeight;
                band.bHeightUserSet = true;
                UpdateBandLayout();
                Invalidate();
            }
            return TRUE;
        }

        int newWidth = smax(band.minWidth, smin(band.maxWidth, m_nDragStartWidth + delta));
        if (newWidth != band.width)
        {
            band.width = newWidth;
            UpdateBandLayout();
            Invalidate();
        }
        return TRUE;
    }

    // Vertical reorder: one column, so the only decision is "which band am I
    // above", taken from the vertical midpoint of each band.
    if (m_bVertical)
    {
        if (bandCount < 2)
            return TRUE;
        if ((int)m_bandRects.GetCount() != bandCount)
            UpdateBandLayout();

        int insertIndex = bandCount;
        for (int i = 0; i < bandCount; ++i)
        {
            if (i == m_nDragBand)
                continue;
            int midY = (m_bandRects[i].top + m_bandRects[i].bottom) / 2;
            if (point.y < midY)
            {
                insertIndex = i;
                break;
            }
            insertIndex = i + 1;
        }

        // insertIndex == m_nDragBand + 1 means "put back where it was"
        if (insertIndex == m_nDragBand || insertIndex == m_nDragBand + 1)
            return TRUE;

        // Copy, do not alias: RemoveAt() below shifts the array under a reference.
        BandInfo curBand = m_bands[m_nDragBand];
        m_bands.RemoveAt(m_nDragBand);
        if (insertIndex > m_nDragBand)
            insertIndex--;
        if (insertIndex < 0)
            insertIndex = 0;
        if (insertIndex > (int)m_bands.GetCount())
            insertIndex = (int)m_bands.GetCount();

        m_bands.InsertAt(insertIndex, curBand);
        m_nDragBand = insertIndex;
        UpdateBandLayout();
        Invalidate();
        return TRUE;
    }

    // Single row: the bands cannot wrap, so the insert index is simply "which band
    // am I past" -- the same rule the vertical branch uses on the y axis. The
    // bNewRow bookkeeping below only means something when bands can wrap, and
    // running it on a single row is what used to leave bands overlapping.
    if (m_bSingleRow)
    {
        if (bandCount < 2)
            return TRUE;
        if ((int)m_bandRects.GetCount() != bandCount)
            UpdateBandLayout();

        int insertIndex = bandCount;
        for (int i = 0; i < bandCount; ++i)
        {
            if (i == m_nDragBand)
                continue;
            int midX = (m_bandRects[i].left + m_bandRects[i].right) / 2;
            if (point.x < midX)
            {
                insertIndex = i;
                break;
            }
            insertIndex = i + 1;
        }

        // insertIndex == m_nDragBand + 1 means "put back where it was"
        if (insertIndex == m_nDragBand || insertIndex == m_nDragBand + 1)
            return TRUE;

        // Copy, do not alias: RemoveAt() below shifts the array under a reference.
        BandInfo curBand = m_bands[m_nDragBand];
        m_bands.RemoveAt(m_nDragBand);
        if (insertIndex > m_nDragBand)
            insertIndex--;
        if (insertIndex < 0)
            insertIndex = 0;
        if (insertIndex > (int)m_bands.GetCount())
            insertIndex = (int)m_bands.GetCount();

        m_bands.InsertAt(insertIndex, curBand);
        m_nDragBand = insertIndex;
        UpdateBandLayout();
        Invalidate();
        return TRUE;
    }

    // Reordering logic
    if (bandCount < 2)
        return TRUE;

    if ((int)m_bandRects.GetCount() != bandCount)
        UpdateBandLayout();

    // Build row-based ranges, find the row under the mouse (or above the first row / below the last row)
    int rowIndex = -9999; // sentinel: not found
    int start = 0;
    int curRow = 0;
    while (start < bandCount)
    {
        int rowFirst = start;
        int rowLast = start;
        int rowTop = m_bandRects[rowFirst].top;
        int rowBottom = m_bandRects[rowFirst].bottom;
        // Expand to all bands in this row (until the next bNewRow or the end)
        while (rowLast + 1 < bandCount && !m_bands[rowLast + 1].bNewRow)
        {
            ++rowLast;
            rowBottom = smax(rowBottom, m_bandRects[rowLast].bottom);
        }

        if (point.y < rowTop)
        {
            rowIndex = -1; // Above all bands (above the first row)
            break;
        }
        if (point.y <= rowBottom)
        {
            rowIndex = curRow; // In this row
            break;
        }
        // Otherwise continue to the next row
        start = rowLast + 1;
        ++curRow;
    }
    if (start >= bandCount && rowIndex == -9999)
    {
        // If no row matched and the mouse Y is below the last row
        rowIndex = INT_MAX; // Marked as below all bands
    }

    // Handle being above the first row (place at the start of the first row)
    if (rowIndex == -1)
    {
        if (m_nDragBand != 0)
        {
            // Preserve original line-break semantics: if the dragged item was bNewRow, pass the flag to its successor (to avoid loss)
            bool wasNewRow = m_bands[m_nDragBand].bNewRow;
            if (wasNewRow && m_nDragBand + 1 < (int)m_bands.GetCount())
            {
                m_bands[m_nDragBand + 1].bNewRow = true;
            }
            // Copy, do not alias: RemoveAt() below shifts the array under a reference.
            BandInfo curBand = m_bands[m_nDragBand];
            m_bands.RemoveAt(m_nDragBand);
            m_bands.InsertAt(0, curBand);
            m_nDragBand = 0;
        }
        // If a second band exists, ensure it starts a new row (isolate the item dragged to the first position)
        if ((int)m_bands.GetCount() > 1)
            m_bands[1].bNewRow = true;
        m_bands[0].bNewRow = true;
        UpdateBandLayout();
        return TRUE;
    }

    // Handle being below all bands (place at the end)
    if (rowIndex == INT_MAX)
    {
        // If the dragged item was originally at the start of a row, pass the row-start semantics to its successor
        bool wasNewRow = m_bands[m_nDragBand].bNewRow;
        if (wasNewRow && m_nDragBand + 1 < (int)m_bands.GetCount())
        {
            m_bands[m_nDragBand + 1].bNewRow = true;
        }
        if (m_nDragBand != (int)m_bands.GetCount() - 1)
        {
            // Copy, do not alias: RemoveAt() below shifts the array under a reference.
            BandInfo curBand = m_bands[m_nDragBand];
            m_bands.RemoveAt(m_nDragBand);
            m_bands.Add(curBand); // append
            m_nDragBand = (int)m_bands.GetCount() - 1;
        }
        m_bands[m_nDragBand].bNewRow = true;
        UpdateBandLayout();
        return TRUE;
    }

    // Inside a row: find row start/end indices
    int targetRow = rowIndex;
    int rowStart = 0;
    int rowCounter = 0;
    for (int i = 0; i < bandCount; ++i)
    {
        if (rowCounter == targetRow)
        {
            rowStart = i;
            break;
        }
        if (i + 1 < bandCount && m_bands[i + 1].bNewRow)
            ++rowCounter;
    }
    // Compute rowEnd
    int rowEnd = rowStart;
    while (rowEnd + 1 < bandCount && !m_bands[rowEnd + 1].bNewRow)
        ++rowEnd;

    // Within a row, decide insert before/after based on each band's midpoint. Skip self (m_nDragBand).
    int insertIndex = rowEnd + 1; // Default insert at row end (rowEnd+1)
    for (int i = rowStart; i <= rowEnd; ++i)
    {
        if (i == m_nDragBand)
            continue;
        int midX = (m_bandRects[i].left + m_bandRects[i].right) / 2;
        if (point.x < midX)
        {
            insertIndex = i;
            break;
        }
        // Otherwise insert after i (continue checking the next one)
        insertIndex = i + 1;
    }

    // If the insertion position equals the current drag position (unchanged), no reordering needed
    if (insertIndex == m_nDragBand || insertIndex == m_nDragBand + 1)
    {
        // insertIndex == m_nDragBand + 1 means put back to original position (because index shifts left after remove)
        return TRUE;
    }

    // First preserve the dragged item's bNewRow semantics and pass to its successor (to avoid loss)
    bool dragWasNewRow = m_bands[m_nDragBand].bNewRow;
    if (dragWasNewRow && m_nDragBand + 1 < (int)m_bands.GetCount())
    {
        m_bands[m_nDragBand + 1].bNewRow = true;
    }

    // Perform the actual remove and insert (watch index changes)
    // Copy, do not alias: RemoveAt() below shifts the array under a reference.
    BandInfo curBand = m_bands[m_nDragBand];
    m_bands.RemoveAt(m_nDragBand);
    // Adjust insertIndex: if insertIndex is after the removed item, the index shifts left by 1 after removal
    if (insertIndex > m_nDragBand)
        insertIndex--;
    if (insertIndex < 0)
        insertIndex = 0;
    if (insertIndex > (int)m_bands.GetCount())
        insertIndex = (int)m_bands.GetCount();

    m_bands.InsertAt(insertIndex, curBand);
    m_nDragBand = insertIndex;

    // If inserted at row start, ensure it is the row start; otherwise clear its row-start flag
    if (insertIndex == rowStart)
    {
        m_bands[insertIndex].bNewRow = true;
        if (insertIndex + 1 < (int)m_bands.GetCount())
            m_bands[insertIndex + 1].bNewRow = false;
    }
    else
    {
        m_bands[insertIndex].bNewRow = false;
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
            SetCursor(SApplication::getSingleton().LoadCursor(m_bVertical ? IDC_SIZENS : IDC_SIZEWE)); // splitter: resize
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
