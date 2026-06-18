#include <souistd.h>
#include <control/SRebar.h>

SNSBEGIN

static const int kRebarGripperWidth = 8;
static const int kRebarSplitterWidth = 4; // band 右侧分隔条，拖拽调节宽度（VS 风格）

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
                    AddBand(pChild, title, height, bCollapsible);
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
        // 高度根据“可用宽度”来计算，这样父窗口只需要 RequestRelayout +
        // 再次调用 GetDesiredSize，就能得到 SRebar 在当前宽度下的合适高度，
        // 而不需要直接 Move SRebar 本身。
        int cxAvail = psz->cx > 0 ? psz->cx : (nParentWid > 0 ? nParentWid : GetClientRect().Width());
        psz->cy = GetTotalHeightForWidth(cxAvail);
    }
}

int SRebar::GetTotalHeightForWidth(int cx) const
{
    if (m_bands.IsEmpty())
        return 24;

    if (m_bSingleRow)
    {
        // 单行：所有 band 一行，高度为单行最大高度
        int rowHeight = 0;
        for (size_t i = 0; i < m_bands.GetCount(); i++)
        {
            int h = m_bands[i].bCollapsed ? 24 : m_bands[i].height;
            rowHeight = smax(rowHeight, h);
        }
        return rowHeight > 0 ? rowHeight : 24;
    }

    // 多行：由 bNewRow 决定换行，与宽度无关（VS 逻辑）
    int y = 0;
    int rowHeight = 0;
    int left = kRebarGripperWidth;

    for (size_t i = 0; i < m_bands.GetCount(); i++)
    {
        const BandInfo &band = m_bands[i];
        int bandHeight = band.bCollapsed ? 24 : band.height;
        int bandWidth = band.width > 0 ? band.width : 80;

        if (i > 0 && band.bNewRow)
        {
            y += rowHeight;
            rowHeight = 0;
        }
        rowHeight = smax(rowHeight, bandHeight);
    }
    return y + (rowHeight == 0 ? 24 : rowHeight);
}

int SRebar::GetTotalHeight() const
{
    if (m_bands.GetCount())
        return 24;
    // 高度由 singleRow + bNewRow 决定，与宽度无关
    return GetTotalHeightForWidth(0);
}

void SRebar::CalculateBandLayout()
{
    for (size_t i = 0; i < m_bands.GetCount(); i++)
    {
        BandInfo &band = m_bands[i];
        if (band.pChild && !band.bCollapsed)
        {
            // 先根据子窗口的期望高度来决定 band 的高度，再按 band 的约束进行裁剪，
            // 这样“band 大小变化”与“子控件大小”是联动的。
            SIZE szChild = { 0, 0 };
            int nAvailWid = m_rcClient.Width() > 0 ? m_rcClient.Width() : -1;
            band.pChild->GetDesiredSize(&szChild, nAvailWid, -1);

            int desiredHei = szChild.cy > 0 ? szChild.cy : band.height;
            band.height = smax(band.minHeight, smin(band.maxHeight, desiredHei));

            // 仅在尚未被用户调整宽度（width==0）时，用子窗口期望宽度初始化 band 宽度
            if (band.width == 0)
            {
                int desiredWid = szChild.cx > 0 ? (szChild.cx + kRebarGripperWidth + kRebarSplitterWidth) : 80;
                band.width = smax(band.minWidth, smin(band.maxWidth, desiredWid));
            }
        }
        else
        {
            // 没有子窗口时，仅按 smin/smax 约束现有高度，避免出现非法高度
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
    int rightEdge = m_rcClient.right;

    for (size_t i = 0; i < m_bands.GetCount(); i++)
    {
        BandInfo &band = m_bands[i];
        int bandHeight = band.bCollapsed ? 24 : band.height;
        int bandWidth = band.width > 0 ? band.width : 80;

        if (m_bSingleRow)
        {
            // 单行：不换行，一行排满
        }
        else
        {
            // 多行：仅由 bNewRow 换行（VS 逻辑，与宽度无关）
            if (i > 0 && band.bNewRow)
            {
                x = m_rcClient.left + kRebarGripperWidth;
                y += rowHeight;
                rowHeight = 0;
            }
        }

        CRect rcBand(x - kRebarGripperWidth, y, x - kRebarGripperWidth + bandWidth, y + bandHeight);
        if (rcBand.right > m_rcClient.right)
            rcBand.right = m_rcClient.right;
        if (rcBand.left < m_rcClient.left)
            rcBand.left = m_rcClient.left;
        m_bandRects[i] = rcBand;

        if (band.pChild)
        {
            if (!band.bCollapsed)
            {
                int xChildLeft = rcBand.left + kRebarGripperWidth;
                int xChildRight = rcBand.right - kRebarSplitterWidth;
                // 子窗口不得超出 Rebar 客户区，超出时减小子窗口宽度
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
            else
            {
                band.pChild->SetVisible(FALSE);
            }
        }

        rowHeight = smax(rowHeight, bandHeight);
        x += bandWidth;
    }

    GetParent()->RequestRelayout();
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

    // 普通背景
    pRT->FillSolidRect(rcBand, GetStyle().m_crBg);

    // 正在被拖动重排的 band 高亮（调宽时不画）
    if (m_bDragging && !m_bResizing && nIndex == m_nDragBand)
    {
        IPenS *pPen = NULL;
        COLORREF crHighlight = RGBA(0, 120, 215, 255); // 接近 VS 选中蓝色
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
        // VS 风格：左侧 gripper（拖动重排）
        CRect rcGripper(rcBand.left, rcBand.top, rcBand.left + kRebarGripperWidth, rcBand.bottom);
        DrawGripper(pRT, rcGripper);
        // 右侧 splitter（拖动调宽）
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
    COLORREF crGripperBg = RGBA(230, 230, 230, 255);
    pRT->FillSolidRect(rcGripper, crGripperBg);

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
                    nHitTest = 2; // gripper：拖动重排
                    return i;
                }
                CRect rcSplitter(rcBand.right - kRebarSplitterWidth, rcBand.top, rcBand.right, rcBand.bottom);
                if (rcSplitter.PtInRect(point))
                {
                    nHitTest = 3; // splitter：拖动调宽
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
            // gripper：仅用于拖动重排（VS 逻辑）
            m_nDragBand = nBand;
            m_bResizing = false;
            m_bDragging = true;
            SetCapture();
        }
        else if (nHitTest == 3)
        {
            // splitter：仅用于调节该 band 宽度（VS 逻辑）
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
        // 单行/多行或行数变化后，通知父窗口重新布局（高度可能已变）
        GetParent()->RequestRelayout();
    }

    return TRUE;
}

BOOL SRebar::OnMouseMove(UINT nFlags, CPoint point)
{
    if (!m_bDragging || m_nDragBand < 0)
        return TRUE;

    // 调整宽度逻辑（不变）
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

    // 重新排序逻辑
    int bandCount = (int)m_bands.GetCount();
    if (bandCount < 2)
        return TRUE;

    if ((int)m_bandRects.GetCount() != bandCount)
        UpdateBandLayout();

    // 构建按行的区间，找出鼠标所在的行（或在首行之上/末行之下）
    int rowIndex = -9999; // sentinel: 未找到
    int firstRowTop = (bandCount > 0) ? m_bandRects[0].top : m_rcClient.top;
    int start = 0;
    int curRow = 0;
    while (start < bandCount)
    {
        int rowFirst = start;
        int rowLast = start;
        int rowTop = m_bandRects[rowFirst].top;
        int rowBottom = m_bandRects[rowFirst].bottom;
        // 扩展到本行所有 band（直到下一个 bNewRow 或末尾）
        while (rowLast + 1 < bandCount && !m_bands[rowLast + 1].bNewRow)
        {
            ++rowLast;
            rowBottom = smax(rowBottom, m_bandRects[rowLast].bottom);
        }

        if (point.y < rowTop)
        {
            rowIndex = -1; // 在所有 band 之上（第一行之上）
            break;
        }
        if (point.y <= rowBottom)
        {
            rowIndex = curRow; // 在该行
            break;
        }
        // 否则继续下一行
        start = rowLast + 1;
        ++curRow;
    }
    if (start >= bandCount && rowIndex == -9999)
    {
        // 如果未匹配任何行并且鼠标 Y 在最后行下面
        rowIndex = INT_MAX; // 标记为在所有 band 之下
    }

    // 处理在第一行之上（放到第一行开头）
    if (rowIndex == -1)
    {
        if (m_nDragBand != 0)
        {
            // 保留原来换行语义：若拖动项曾是 bNewRow，则把该标记传递给其后继（避免丢失）
            bool wasNewRow = m_bands[m_nDragBand].bNewRow;
            if (wasNewRow && m_nDragBand + 1 < (int)m_bands.GetCount())
            {
                m_bands[m_nDragBand + 1].bNewRow = true;
            }
            const BandInfo &curBand = m_bands[m_nDragBand];
            m_bands.RemoveAt(m_nDragBand);
            m_bands.InsertAt(0, curBand);
            m_nDragBand = 0;
        }
        // 如果存在第二个 band，确保它开始新行（把拖到第一的位置独立出来）
        if ((int)m_bands.GetCount() > 1)
            m_bands[1].bNewRow = true;
        m_bands[0].bNewRow = true;
        UpdateBandLayout();
        return TRUE;
    }

    // 处理在所有 band 之下（放到末尾）
    if (rowIndex == INT_MAX)
    {
        // 若拖动项原来是行首，则把行首语义传递给其后继
        bool wasNewRow = m_bands[m_nDragBand].bNewRow;
        if (wasNewRow && m_nDragBand + 1 < (int)m_bands.GetCount())
        {
            m_bands[m_nDragBand + 1].bNewRow = true;
        }
        if (m_nDragBand != (int)m_bands.GetCount() - 1)
        {
            const BandInfo & curBand = m_bands[m_nDragBand];
            m_bands.RemoveAt(m_nDragBand);
            m_bands.Add(curBand); // append
            m_nDragBand = (int)m_bands.GetCount() - 1;
        }
        m_bands[m_nDragBand].bNewRow = true;
        UpdateBandLayout();
        return TRUE;
    }

    // 在某一行内部：找行起始/结束索引
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
    // 计算 rowEnd
    int rowEnd = rowStart;
    while (rowEnd + 1 < bandCount && !m_bands[rowEnd + 1].bNewRow)
        ++rowEnd;

    // 在行内根据每个 band 的中点决定插入前/后。跳过自己（m_nDragBand）。
    int insertIndex = rowEnd + 1; // 默认插在行末（rowEnd+1）
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
        // 否则插入到 i 的后面（继续判断下一个）
        insertIndex = i + 1;
    }

    // 若插入位置等于当前拖动位置（不变），则不需要重排
    if (insertIndex == m_nDragBand || insertIndex == m_nDragBand + 1)
    {
        // insertIndex == m_nDragBand + 1 表示放回原位（因为 remove 后索引会左移）
        return TRUE;
    }

    // 先把拖动项的 bNewRow 语义保留并传递给后继（避免丢失）
    bool dragWasNewRow = m_bands[m_nDragBand].bNewRow;
    if (dragWasNewRow && m_nDragBand + 1 < (int)m_bands.GetCount())
    {
        m_bands[m_nDragBand + 1].bNewRow = true;
    }

    // 做真正的移除与插入（注意索引变化）
    const BandInfo & curBand = m_bands[m_nDragBand];
    m_bands.RemoveAt(m_nDragBand);
    // 调整 insertIndex：如果 insertIndex 在被移除项之后，移除后索引会左移 1
    if (insertIndex > m_nDragBand)
        insertIndex--;
    if (insertIndex < 0)
        insertIndex = 0;
    if (insertIndex > (int)m_bands.GetCount())
        insertIndex = (int)m_bands.GetCount();

    m_bands.InsertAt(insertIndex, curBand);
    m_nDragBand = insertIndex;

    // 如果插入到行首，确保其为行起点；否则清除其行起点标志
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
            SetCursor(SApplication::getSingleton().LoadCursor(IDC_SIZEALL)); // gripper：重排
            return TRUE;
        }
        if (nHitTest == 3)
        {
            SetCursor(SApplication::getSingleton().LoadCursor(IDC_SIZEWE)); // splitter：调宽
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
