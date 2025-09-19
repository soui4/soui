#include "stdafx.h"
#include "SGridCtrl.h"
#include <control/SEdit.h>
#include <control/SComboBox.h>
#include <control/SCmnCtrl.h>
#include "SSelRangeMgr.h"

SNSBEGIN

//////////////////////////////////////////////////////////////////////////
// SGridCell implementation
SGridCell::SGridCell()
    : m_nImage(-1)
    , m_lParam(0)
    , m_nState(0)
    , m_nFormat(DT_LEFT | DT_VCENTER | DT_SINGLELINE)
    , m_crFgClr(CLR_DEFAULT)
    , m_crBkClr(CLR_DEFAULT)
    , m_nMargin(0)
    , m_bDefaultFont(TRUE)
    , m_nRow(-1)
    , m_nCol(-1)
    , m_pGrid(NULL)
{
    memset(&m_lfFont, 0, sizeof(LOGFONT));
}

SGridCell::~SGridCell()
{
}

#define IMPLMETHOD(type, name) type name

IMPLMETHOD(void, SGridCell::Init)(THIS_ IGridCtrl* pGrid, int nRow, int nCol){
    m_pGrid = pGrid;
    m_nRow = nRow;
    m_nCol = nCol;
}
// IGridCell interface implementation
IMPLMETHOD(void, SGridCell::SetText)(LPCTSTR szText)
{
    m_strText = szText ? szText : _T("");
}

IMPLMETHOD(void, SGridCell::SetImage)(int nImage)
{
    m_nImage = nImage;
}

IMPLMETHOD(void, SGridCell::SetData)(LPARAM lParam)
{
    m_lParam = lParam;
}

IMPLMETHOD(void, SGridCell::SetState)(DWORD nState)
{
    m_nState = nState;
}

IMPLMETHOD(void, SGridCell::SetFormat)(DWORD nFormat)
{
    m_nFormat = nFormat;
}

IMPLMETHOD(void, SGridCell::SetTextClr)(COLORREF clr)
{
    m_crFgClr = clr;
}

IMPLMETHOD(void, SGridCell::SetBackClr)(COLORREF clr)
{
    m_crBkClr = clr;
}

IMPLMETHOD(void, SGridCell::SetFont)(const LOGFONT* plf)
{
    if (plf)
    {
        memcpy(&m_lfFont, plf, sizeof(LOGFONT));
        m_bDefaultFont = FALSE;
    }
    else
    {
        memset(&m_lfFont, 0, sizeof(LOGFONT));
        m_bDefaultFont = TRUE;
    }
}

IMPLMETHOD(void, SGridCell::SetReadOnly)(BOOL bReadOnly)
{
    m_nState &= ~SGVIS_READONLY;
}

IMPLMETHOD(void, SGridCell::SetMargin)(UINT nMargin)
{
    m_nMargin = nMargin;
}

IMPLMETHOD(LPCTSTR, SGridCell::GetText)() SCONST
{
    return m_strText.c_str();
}

IMPLMETHOD(LPCTSTR, SGridCell::GetTipText)() SCONST
{
    return GetText(); // Default implementation returns the same as GetText
}

IMPLMETHOD(int, SGridCell::GetImage)() SCONST
{
    return m_nImage;
}

IMPLMETHOD(LPARAM, SGridCell::GetData)() SCONST
{
    return m_lParam;
}

IMPLMETHOD(DWORD, SGridCell::GetState)() SCONST
{
    return m_nState;
}

IMPLMETHOD(DWORD, SGridCell::GetFormat)() SCONST
{
    return m_nFormat;
}

IMPLMETHOD(COLORREF, SGridCell::GetTextClr)() SCONST
{
    return m_crFgClr;
}

IMPLMETHOD(COLORREF, SGridCell::GetBackClr)() SCONST
{
    return m_crBkClr;
}

IMPLMETHOD(LOGFONT*, SGridCell::GetFont)() SCONST
{
    return m_bDefaultFont ? NULL : const_cast<LOGFONT*>(&m_lfFont);
}

IMPLMETHOD(UINT, SGridCell::GetMargin)() SCONST
{
    return m_nMargin;
}

IMPLMETHOD(BOOL, SGridCell::IsEditing)() SCONST
{
    return FALSE; // TODO: Implement editing state check
}

IMPLMETHOD(BOOL, SGridCell::IsFocused)() SCONST
{
    return (m_nState & SGVIS_FOCUSED) != 0;
}

IMPLMETHOD(BOOL, SGridCell::IsFixed)() SCONST
{
    return (m_nState & SGVIS_FIXED) != 0;
}

IMPLMETHOD(BOOL, SGridCell::IsFixedCol)() SCONST
{
    return (m_nState & SGVIS_FIXEDCOL) != 0;
}

IMPLMETHOD(BOOL, SGridCell::IsFixedRow)() SCONST
{
    return (m_nState & SGVIS_FIXEDROW) != 0;
}

IMPLMETHOD(BOOL, SGridCell::IsSelected)() SCONST
{
    return (m_nState & SGVIS_SELECTED) != 0;
}

IMPLMETHOD(BOOL, SGridCell::IsReadOnly)() SCONST
{
    return (m_nState & SGVIS_READONLY) != 0;
}

IMPLMETHOD(BOOL, SGridCell::IsModified)() SCONST
{
    return (m_nState & SGVIS_MODIFIED) != 0;
}

IMPLMETHOD(BOOL, SGridCell::IsDropHighlighted)() SCONST
{
    return (m_nState & SGVIS_DROPHILITED) != 0;
}

IMPLMETHOD(void, SGridCell::Reset)()
{
    m_strText.Empty();
    m_nImage = -1;
    m_lParam = 0;
    m_nState = 0;
    m_nFormat = DT_LEFT | DT_VCENTER | DT_SINGLELINE;
    m_crFgClr = CLR_DEFAULT;
    m_crBkClr = CLR_DEFAULT;
    m_nMargin = 0;
    m_bDefaultFont = TRUE;
    memset(&m_lfFont, 0, sizeof(LOGFONT));
}

IMPLMETHOD(BOOL, SGridCell::Draw)(IRenderTarget* pRT, int nRow, int nCol, RECT rect, BOOL bEraseBkgnd)
{
    if (!pRT)
        return FALSE;

    // Draw background
    if (bEraseBkgnd)
    {
        COLORREF crBk = GetBackClr();
        if (crBk == CLR_DEFAULT)
        {
            // Use default background color based on cell state
            if (IsSelected())
                crBk = RGBA(51, 153, 255,255); // Brighter selection color
            else if (IsFixed())
                crBk = RGBA(240, 240, 240,255); // Default fixed cell color
            else
                crBk = RGBA(255, 255, 255,255); // Default cell color
        }
        pRT->FillSolidRect(&rect, crBk);
    }

    // Draw focus/selection border
    if (IsFocused() || IsSelected())
    {
        COLORREF crBorder = IsFocused() ? RGBA(0, 120, 215,255) : RGBA(30, 120, 200,255);
        int nBorderWidth = IsFocused() ? 2 : 1;

        SAutoRefPtr<IPenS> pPen;
        pRT->CreatePen(PS_SOLID, crBorder, nBorderWidth, &pPen);
        if (pPen)
        {
            SAutoRefPtr<IPenS> pOldPen;
            pRT->SelectObject(pPen, (IRenderObj**)&pOldPen);

            CRect rcBorder = rect;
            if (nBorderWidth > 1)
            {
                rcBorder.DeflateRect(nBorderWidth/2, nBorderWidth/2);
            }
            pRT->DrawRectangle(&rcBorder);
            pRT->SelectObject(pOldPen, NULL);
        }
    }

    // Draw image if present
    CRect rcContent = rect;
    rcContent.DeflateRect(GetMargin(), GetMargin());

    if (GetImage() >= 0 && m_pGrid && m_pGrid->GetImageList())
    {
        ISkinObj* pImageList = m_pGrid->GetImageList();
        if (pImageList)
        {
            CSize imageSize = pImageList->GetSkinSize();
            CRect rcImage = rcContent;
            rcImage.right = rcImage.left + imageSize.cx;
            rcImage.bottom = rcImage.top + imageSize.cy;

            // Center image vertically if cell is taller than image
            if (rcContent.Height() > imageSize.cy)
            {
                int nOffset = (rcContent.Height() - imageSize.cy) / 2;
                rcImage.OffsetRect(0, nOffset);
            }

            // Draw the image
            pImageList->DrawByIndex(pRT, &rcImage, GetImage());

            // Adjust content rect for text
            rcContent.left += imageSize.cx + GetMargin();
        }
    }

    // Draw text
    SStringT strText = GetText();
    if (!strText.IsEmpty())
    {
        COLORREF crText = GetTextClr();
        if (crText == CLR_DEFAULT)
        {
            crText = IsSelected() ? RGBA(255, 255, 255,255) : RGBA(0, 0, 0,255);
        }

        CRect rcText = rcContent;
        rcText.DeflateRect(2, 1); // Extra padding for border

        COLORREF crOldText = pRT->SetTextColor(crText);
        pRT->DrawText(strText, strText.GetLength(), &rcText, GetFormat());
        pRT->SetTextColor(crOldText);
    }

    return TRUE;
}

IMPLMETHOD(BOOL, SGridCell::GetTextRect)(LPRECT pRect)
{
    if (!pRect)
        return FALSE;

    // Adjust for image if present
    if (GetImage() >= 0 && m_pGrid && m_pGrid->GetImageList())
    {
        // Get actual image size from the image list
        ISkinObj* pImageList = m_pGrid->GetImageList();
        CSize imageSize = pImageList->GetSkinSize();
        pRect->left += imageSize.cx + GetMargin();
    }

    // Adjust for margins
    pRect->left += GetMargin();
    pRect->top += GetMargin();
    pRect->right -= GetMargin();
    pRect->bottom -= GetMargin();

    // Ensure we don't have negative dimensions
    if (pRect->right <= pRect->left)
        pRect->right = pRect->left + 1;
    if (pRect->bottom <= pRect->top)
        pRect->bottom = pRect->top + 1;

    return TRUE;
}

IMPLMETHOD(void, SGridCell::GetTextExtent)(LPCTSTR str, SIZE* pSize)
{
    if (!pSize)
        return;

    if (!str || !*str)
    {
        *pSize = CSize(0, 0);
        return;
    }

    // Get the grid to access render target
    if (!m_pGrid)
    {
        // Default size if no grid available
        *pSize = CSize(100, 20);
        return;
    }

    // Create a temporary render target for measurement
    CAutoRefPtr<IRenderTarget> pRT;
    GETRENDERFACTORY->CreateRenderTarget(&pRT, 0, 0);
    if (!pRT)
    {
        // Default size if no render target
        *pSize = CSize(100, 20);
        return;
    }

    // Use default font for measurement
    // TODO: Use cell's specific font if available

    // Measure text
    CSize textSize;
    pRT->MeasureText(str, -1, &textSize);

    // Add margins
    textSize.cx += 2 * GetMargin();
    textSize.cy += 2 * GetMargin();

    // Handle multiline text if DT_WORDBREAK is set
    DWORD nFormat = GetFormat();
    if ((nFormat & DT_WORDBREAK) && !(nFormat & DT_SINGLELINE))
    {
        // For multiline text, we need to calculate based on word wrapping
        // This is a simplified implementation
        SStringT strText = str;
        int nMaxWidth = 0;
        int nLines = 1;

        // Count lines and find max width
        int nPos = 0;
        while ((nPos = strText.Find(_T("\n"), nPos)) >= 0)
        {
            nLines++;
            nPos++;
        }

        textSize.cy = textSize.cy * nLines;
    }

    *pSize = textSize;
}


IMPLMETHOD(void, SGridCell::GetCellExtent)(IRenderTarget* pRT, SIZE* pSize)
{
    if (!pSize)
        return;

    // Get text extent first
    SIZE textSize;
    GetTextExtent(GetText(), &textSize);

    CSize cellSize = textSize;
    CSize imageSize(0, 0);

    // Add image size if present
    int nImage = GetImage();
    if (nImage >= 0)
    {
        if (m_pGrid && m_pGrid->GetImageList())
        {
            // Get actual image size from the image list
            ISkinObj* pImageList = m_pGrid->GetImageList();
            imageSize = pImageList->GetSkinSize();
        }
        else
        {
            // Fallback to standard icon size
            imageSize = CSize(16, 16);
        }

        if (cellSize.cx > 2 * (int)GetMargin())
            imageSize.cx += GetMargin();
        imageSize.cy += 2 * (int)GetMargin();
    }

    // Combine text and image sizes
    cellSize.cx += imageSize.cx + 1;
    cellSize.cy = smax(cellSize.cy, imageSize.cy) + 1;

    // Add extra space for fixed cells (they have borders)
    if (IsFixed())
    {
        cellSize.cx++;
        cellSize.cy++;
    }

    *pSize = cellSize;
}

IMPLMETHOD(BOOL, SGridCell::Edit)(int nRow, int nCol, RECT rect, POINT point, UINT nID, UINT nChar)
{
    // Check if cell is read-only
    if (IsReadOnly())
        return FALSE;

    // Check if we have a valid grid
    if (!m_pGrid)
        return FALSE;

    // Start editing this cell
    SCellID cellID(nRow, nCol);
    m_pGrid->StartEdit(cellID);

    // If a character was provided, it will be handled by the grid's keyboard processing
    // We don't need to do anything special here

    return m_pGrid->IsInEdit();
}

IMPLMETHOD(void, SGridCell::EndEdit)()
{
    // Let the grid handle ending the edit
    if (m_pGrid)
    {
        m_pGrid->EndEdit(TRUE); // Save changes
    }
}

IMPLMETHOD(BOOL, SGridCell::ValidateEdit)(LPCTSTR str)
{
    // Default implementation accepts all input
    return TRUE;
}

IMPLMETHOD(BOOL, SGridCell::PrintCell)(IRenderTarget* pRT, int nRow, int nCol, RECT rect)
{
    // Default implementation uses the same drawing as screen
    return Draw(pRT, nRow, nCol, rect, TRUE);
}

IMPLMETHOD(IGridInplaceWnd *, SGridCell::CreateInplaceWnd)(CTHIS_ int nRow,int nCol) SCONST{
    TplGridInplaceWnd<SEdit>* pEdit = new TplGridInplaceWnd<SEdit>(m_pGrid, nRow, nCol);
    if (!pEdit)
        return NULL;
    // Set edit properties
    pEdit->SetAttribute(L"colorBkgnd", L"#FFFFFF", TRUE);
    pEdit->SetAttribute(L"colorText", L"#000000", TRUE);
    pEdit->SetAttribute(L"autoWordSel", L"1", TRUE);

    return pEdit;
}

IMPLMETHOD(IGridInplaceWnd *, SGridCheckBoxCell::CreateInplaceWnd)(CTHIS_ int nRow,int nCol) SCONST{
    return NULL;
}


IMPLMETHOD(IGridInplaceWnd *, SGridURLCell::CreateInplaceWnd)(CTHIS_ int nRow,int nCol) SCONST{
    return NULL;
}

//------------------------------------------------------------------------

//////////////////////////////////////////////////////////////////////////
// SGridCtrl implementation
//////////////////////////////////////////////////////////////////////////
// SGridURLCell implementation

SGridURLCell::SGridURLCell()
    : m_crURLColor(RGBA(0, 0, 255,255))  // Blue color for URLs
    , m_bAutoLaunch(TRUE)
    , m_rcCell(0, 0, 0, 0)
{
}

SGridURLCell::~SGridURLCell()
{
}

IMPLMETHOD(BOOL, SGridURLCell::Draw)(IRenderTarget* pRT, int nRow, int nCol, RECT rect, BOOL bEraseBkgnd)
{
    if (!pRT)
        return FALSE;

    // Store cell rect for hit testing
    m_rcCell = rect;

    // Check if text contains URL
    if (HasURL(GetText()))
    {
        // Temporarily change text color to URL color
        COLORREF oldColor = GetTextClr();
        SetTextClr(m_crURLColor);

        // Call base class draw
        BOOL result = SGridCell::Draw(pRT, nRow, nCol, rect, bEraseBkgnd);

        // Restore original color
        SetTextClr(oldColor);

        return result;
    }
    else
    {
        // Draw as normal cell
        return SGridCell::Draw(pRT, nRow, nCol, rect, bEraseBkgnd);
    }
}

IMPLMETHOD(void, SGridURLCell::OnClickUp)(POINT PointCellRelative)
{
    if (!m_bAutoLaunch)
        return;

    SStringT strURL;
    CPoint pt(PointCellRelative.x, PointCellRelative.y);

    if (OverURL(pt, strURL))
    {
        // Launch URL
        ShellExecute(NULL, _T("open"), strURL, NULL, NULL, SW_SHOW);
    }
}

IMPLMETHOD(BOOL, SGridURLCell::OnSetCursor)(POINT PointCellRelative)
{
    if (m_pGrid)
    {
        SStringT strURL;
        if (OverURL(PointCellRelative, strURL))
        {
            HCURSOR hCursor = GETRESPROVIDER->LoadCursor(IDC_HAND);
            ::SetCursor(hCursor);
            return TRUE;
        }
    }

    return FALSE;
}

BOOL SGridURLCell::HasURL(const SStringT& str) const
{
    // Simple URL detection - check for common prefixes
    static LPCTSTR urlPrefixes[] = {
        _T("http://"), _T("https://"), _T("ftp://"), _T("www."),
        _T("mailto:"), _T("file://"), _T("news://"), _T("gopher://")
    };

    SStringT lowerStr = str;
    lowerStr.MakeLower();

    for (int i = 0; i < ARRAYSIZE(urlPrefixes); i++)
    {
        if (lowerStr.Find(urlPrefixes[i]) >= 0)
            return TRUE;
    }

    return FALSE;
}

BOOL SGridURLCell::OverURL(const CPoint& pt, SStringT& strURL) const
{
    // Simple implementation - if cell contains URL, entire cell is clickable
    if (HasURL(GetText()))
    {
        strURL = m_strURL.IsEmpty() ? SStringT(GetText()) : m_strURL;
        return TRUE;
    }

    return FALSE;
}

//////////////////////////////////////////////////////////////////////////
// SGridCheckBoxCell implementation

SGridCheckBoxCell::SGridCheckBoxCell()
    : m_bChecked(FALSE)
    , m_nCheckBoxSize(16)
{
}

SGridCheckBoxCell::~SGridCheckBoxCell()
{
}

IMPLMETHOD(BOOL, SGridCheckBoxCell::Draw)(IRenderTarget* pRT, int nRow, int nCol, RECT rect, BOOL bEraseBkgnd)
{
    if (!pRT)
        return FALSE;

    // Draw background first
    if (bEraseBkgnd)
    {
        COLORREF crBk = GetBackClr();
        if (crBk == CLR_DEFAULT)
        {
            if (IsSelected())
                crBk = RGBA(51, 153, 255,255);
            else if (IsFixed())
                crBk = RGBA(240, 240, 240,255);
            else
                crBk = RGBA(255, 255, 255,255);
        }
        pRT->FillSolidRect(&rect, crBk);
    }

    // Calculate checkbox position (centered)
    CRect rcCheck;
    rcCheck.left = rect.left + 4;
    rcCheck.top = rect.top + (rect.bottom - rect.top - m_nCheckBoxSize) / 2;
    rcCheck.right = rcCheck.left + m_nCheckBoxSize;
    rcCheck.bottom = rcCheck.top + m_nCheckBoxSize;
    // Fill checkbox background
    pRT->FillSolidRect(&rcCheck, RGBA(255, 255, 255,255));
    // Draw checkbox border
    SAutoRefPtr<IPenS> pPen, pOldPen;
    pRT->CreatePen(PS_SOLID, RGBA(128, 128, 128,255), 1, &pPen);
    if (pPen)
    {
        pRT->SelectObject(pPen, (IRenderObj**)&pOldPen);
        pRT->DrawRectangle(&rcCheck);
        pRT->SelectObject(pOldPen, NULL);
    }
    // Draw check mark if checked
    if (m_bChecked)
    {
        SAutoRefPtr<IPenS> pCheckPen, pOldCheckPen;
        pRT->CreatePen(PS_SOLID, RGBA(0, 128, 0,255), 2, &pCheckPen);
        if (pCheckPen)
        {
            pRT->SelectObject(pCheckPen, (IRenderObj**)&pOldCheckPen);

            // Draw check mark as lines
            pRT->DrawLine(CPoint(rcCheck.left + 3, rcCheck.top + m_nCheckBoxSize / 2),
                         CPoint(rcCheck.left + m_nCheckBoxSize / 2, rcCheck.bottom - 3));
            pRT->DrawLine(CPoint(rcCheck.left + m_nCheckBoxSize / 2, rcCheck.bottom - 3),
                         CPoint(rcCheck.right - 2, rcCheck.top + 2));

            pRT->SelectObject(pOldCheckPen, NULL);
        }
    }

    // Draw text if any (to the right of checkbox)
    SStringT strText = GetText();
    if (!strText.IsEmpty())
    {
        CRect rcText = rect;
        rcText.left = rcCheck.right + 4;

        if (rcText.left < rcText.right)
        {
            COLORREF crText = GetTextClr();
            if (crText == CLR_DEFAULT)
                crText = RGBA(0, 0, 0,255);

            pRT->DrawText(strText, strText.GetLength(), &rcText,
                         GetFormat() | DT_VCENTER | DT_SINGLELINE);
        }
    }

    return TRUE;
}

IMPLMETHOD(void, SGridCheckBoxCell::OnClickUp)(POINT PointCellRelative)
{
    if (IsReadOnly())
        return;

    // Toggle checkbox state
    m_bChecked = !m_bChecked;

    // Redraw cell
    if (m_pGrid)
    {
        m_pGrid->RedrawCell(m_nRow, m_nCol);
    }
}

//------------------------------------------------------------------------
SGridOptionCell::SGridOptionCell(){}

SGridOptionCell::~SGridOptionCell(){}

IMPLMETHOD(BOOL, SGridOptionCell::Draw)(IRenderTarget* pRT, int nRow, int nCol, RECT rect, BOOL bEraseBkgnd)
{
    // Draw background first
    if (bEraseBkgnd)
    {
        COLORREF crBk = GetBackClr();
        if (crBk == CLR_DEFAULT)
        {
            if (IsSelected())
                crBk = RGBA(51, 153, 255,255);
            else if (IsFixed())
                crBk = RGBA(240, 240, 240,255);
            else
                crBk = RGBA(255, 255, 255,255);
        }
        pRT->FillSolidRect(&rect, crBk);
    }

    // Draw text if any (to the right of checkbox)
    SStringT strText = GetText();
    if (!strText.IsEmpty())
    {
        CRect rcText = rect;
        rcText.left = rect.left + 4;

        if (rcText.left < rcText.right)
        {
            COLORREF crText = GetTextClr();
            if (crText == CLR_DEFAULT)
                crText = RGBA(0, 0, 0,255);

            pRT->DrawText(strText, strText.GetLength(), &rcText,
                         GetFormat() | DT_VCENTER | DT_SINGLELINE);
        }
    }
    //Draw arrow
    CRect rcArrow = rect;
    rcArrow.left = rect.right - 16;
    ISkinObj *pComboArrow = GETBUILTINSKIN2(SKIN_SYS_DROPBTN, m_pGrid->ToIWindow()->GetScale());
    if(pComboArrow){
        pComboArrow->DrawByIndex(pRT, rcArrow, 0);
    }
    return TRUE;
}

IMPLMETHOD(void, SGridOptionCell::OnDblClick)(THIS_ POINT PointCellRelative)
{
    if (GetGrid()->IsEditable() && !IsReadOnly())
    {
        SCellID cellID(GetRow(), GetCol() );
        GetGrid()->StartEdit(cellID);
        IGridInplaceWnd * inplaceWnd= GetGrid()->GetInplaceWnd();
        if(inplaceWnd){
            IWindow * pWnd = inplaceWnd->GetIWindow();
            SAutoRefPtr<IComboBox> pCombo;
            pWnd->QueryInterface(IID_IComboBox,(IObjRef**)&pCombo);
            if(pCombo && pCombo->IsDropdown()){//dropdwon combo list.
                pCombo->DropDown();
            }
        }
    }
}

IMPLMETHOD(void, SGridOptionCell::OnClickUp)(THIS_ POINT PointCellRelative){
    if (GetGrid()->IsEditable() && !IsReadOnly()){
        CRect rcCell;
        GetGrid()->GetCellRectEx(GetRow(), GetCol(),&rcCell);
        rcCell.MoveToXY(0,0);
        rcCell.left = rcCell.right - 16;
        if(rcCell.PtInRect(PointCellRelative)){
            SCellID cellID (GetRow(), GetCol() );
            GetGrid()->StartEdit(cellID);
            IGridInplaceWnd * inplaceWnd= GetGrid()->GetInplaceWnd();
            if(inplaceWnd){
                IWindow * pWnd = inplaceWnd->GetIWindow();
                SAutoRefPtr<IComboBox> pCombo;
                pWnd->QueryInterface(IID_IComboBox,(IObjRef**)&pCombo);
                if(pCombo){//dropdwon combo list.
                    pCombo->DropDown();
                }
            }
        }
    }
}

IMPLMETHOD(IGridInplaceWnd *, SGridOptionCell::CreateInplaceWnd)(CTHIS_ int nRow,int nCol) SCONST{
    TplGridInplaceWnd<SComboBox>* pCombo = new TplGridInplaceWnd<SComboBox>(m_pGrid, nRow, nCol);
    if (!pCombo)
        return NULL;

    // Set combo properties
    pCombo->SetAttribute(L"dropDown", L"1", TRUE);
    pCombo->SetAttribute(L"colorBkgnd", L"#FFFFFF", TRUE);
    pCombo->SetAttribute(L"drawFocusRect", L"0", TRUE);
    return pCombo;
}

//////////////////////////////////////////////////////////////////////////
// SGridCtrl implementation
SGridCtrl::SGridCtrl()
    : m_nRows(0)
    , m_nCols(0)
    , m_nFixedRows(1)
    , m_nFixedCols(1)
    , m_nDefCellWidth(80)
    , m_nDefCellHeight(20)
    , m_nGridLines(SGVL_BOTH)
    , m_crGridLineColor(RGBA(192, 192, 192,255))
    , m_crGridBkColor(RGBA(255, 255, 255,255))
    , m_bEditable(TRUE)
    , m_bListMode(FALSE)
    , m_bSingleRowSelection(FALSE)
    , m_bSingleColSelection(FALSE)
    , m_cellFocus(-1, -1)
    , m_bInEdit(FALSE)
    , m_cellEdit(-1, -1)
    , m_pInplaceWnd(NULL)
    , m_nSortColumn(-1)
    , m_nSortOrder(SORT_NONE)
    , m_bSortOnClick(TRUE)
    , m_pImageList(NULL)
    , m_MouseMode(MOUSE_NOTHING)
    , m_bLMouseButtonDown(FALSE)
    , m_bRMouseButtonDown(FALSE)
    , m_LeftClickDownPoint(0, 0)
    , m_LastMousePoint(0, 0)
    , m_LeftClickDownCell(-1, -1)
    , m_SelectionStartCell(-1, -1)
    , m_lastHoverCell(-1,-1)
    , m_LastDragOverCell(-1, -1)
    , m_bAllowDragAndDrop(FALSE)
    , m_bDragRowMode(FALSE)
    , m_bAllowRowResize(TRUE)
    , m_bAllowColumnResize(TRUE)
    , m_nResizeCaptureRange(3)
    , m_sizeGrid(0, 0)
{
    m_bClipClient = TRUE;
    m_bFocusable = TRUE;
    
    RegisterCellFactory<SGridCell>();
    RegisterCellFactory<SGridCheckBoxCell>();
    RegisterCellFactory<SGridURLCell>();
    RegisterCellFactory<SGridOptionCell>();
    
    // Create default cell implementations
    m_pDefaultCell.Attach(CreateCellByType(CELL_TYPE_TEXT));
    m_pDefaultFixedCell.Attach(CreateCellByType(CELL_TYPE_TEXT));
    
    // Set default properties for fixed cells
    m_pDefaultFixedCell->SetBackClr(RGBA(240, 240, 240,255));
    m_pDefaultFixedCell->SetFormat(DT_CENTER | DT_VCENTER | DT_SINGLELINE);

    GetEventSet()->addEvent(EVENTID(EventGridInitInplaceWnd));
    GetEventSet()->addEvent(EVENTID(EventGridGetInplaceStyle));
}

SGridCtrl::~SGridCtrl()
{
    ClearCells();
    
    if (m_pDefaultCell)
    {
        m_pDefaultCell = NULL;
    }
    
    if (m_pDefaultFixedCell)
    {
        m_pDefaultFixedCell = NULL;
    }
}

// Grid dimensions implementation
int SGridCtrl::GetRowCount() const
{
    return m_nRows;
}

int SGridCtrl::GetColumnCount() const
{
    return m_nCols;
}

int SGridCtrl::GetFixedRowCount() const
{
    return m_nFixedRows;
}

int SGridCtrl::GetFixedColumnCount() const
{
    return m_nFixedCols;
}

BOOL SGridCtrl::SetRowCount(int nRows)
{
    if (nRows < 0)
        return FALSE;

    if (nRows == m_nRows)
        return TRUE;

    // Resize row heights array
    m_arRowHeights.SetCount(nRows);
    for (int i = m_nRows; i < nRows; i++)
    {
        m_arRowHeights[i] = m_nDefCellHeight;
    }

    // Resize cell data
    if (nRows > m_nRows)
    {
        // Add new rows
        for (int i = m_nRows; i < nRows; i++)
        {
            SArray<IGridCell*>* pRow = new SArray<IGridCell*>();
            pRow->SetCount(m_nCols);
            for (int j = 0; j < m_nCols; j++)
            {
                (*pRow)[j] = NULL;
            }
            m_RowData.Add(pRow);
        }
    }
    else
    {
        // Remove rows
        for (int i = nRows; i < m_nRows; i++)
        {
            SArray<IGridCell*>* pRow = m_RowData[i];
            if (pRow)
            {
                for (int j = 0; j < m_nCols; j++)
                {
                    if ((*pRow)[j])
                    {
                        (*pRow)[j]->Release();
                        (*pRow)[j] = NULL;
                    }
                }
                delete pRow;
            }
        }
        m_RowData.SetCount(nRows);
    }

    m_nRows = nRows;

    // Update focus and selection if necessary
    if (m_cellFocus.row >= m_nRows)
        m_cellFocus.row = -1;
    // Trim selection ranges that exceed new row count
    SCellRange bound = m_SelectedCellRange.GetBoundingRange();
    if (bound.maxRow >= m_nRows)
    {
        // remove ranges beyond new row count
        SList<SCellRange> newRanges;
        SPOSITION pos = m_SelectedCellRange.GetRanges().GetHeadPosition();
        while (pos)
        {
            SCellRange r = m_SelectedCellRange.GetRanges().GetNext(pos);
            if (r.minRow < m_nRows)
            {
                r.maxRow = smin(r.maxRow, m_nRows - 1);
                newRanges.AddTail(r);
            }
        }
        m_SelectedCellRange.Clear();
        SPOSITION pos2 = newRanges.GetHeadPosition();
        while(pos2){ m_SelectedCellRange.AddRange(newRanges.GetNext(pos2)); }
    }

    RecalcLayout();
    Invalidate();
    return TRUE;
}

BOOL SGridCtrl::SetColumnCount(int nCols)
{
    if (nCols < 0)
        return FALSE;

    if (nCols == m_nCols)
        return TRUE;

    // Resize column widths array
    m_arColWidths.SetCount(nCols);
    for (int i = m_nCols; i < nCols; i++)
    {
        m_arColWidths[i] = m_nDefCellWidth;
    }

    // Resize cell data
    for (int i = 0; i < m_nRows; i++)
    {
        SArray<IGridCell*>* pRow = m_RowData[i];
        if (pRow)
        {
            if (nCols > m_nCols)
            {
                // Add new columns
                int oldCount = pRow->GetCount();
                pRow->SetCount(nCols);
                for (int j = oldCount; j < nCols; j++)
                {
                    (*pRow)[j] = NULL;
                }
            }
            else
            {
                // Remove columns
                for (int j = nCols; j < m_nCols; j++)
                {
                    if ((*pRow)[j])
                    {
                        (*pRow)[j]->Release();
                        (*pRow)[j] = NULL;
                    }
                }
                pRow->SetCount(nCols);
            }
        }
    }

    m_nCols = nCols;

    // Update focus and selection if necessary
    if (m_cellFocus.col >= m_nCols)
        m_cellFocus.col = -1;
    // Trim selection ranges that exceed new col count
    SCellRange bound = m_SelectedCellRange.GetBoundingRange();
    if (bound.maxCol >= m_nCols)
    {
        SList<SCellRange> newRanges;
        SPOSITION pos = m_SelectedCellRange.GetRanges().GetHeadPosition();
        while (pos)
        {
            SCellRange r = m_SelectedCellRange.GetRanges().GetNext(pos);
            if (r.minCol < m_nCols)
            {
                r.maxCol = smin(r.maxCol, m_nCols - 1);
                newRanges.AddTail(r);
            }
        }
        m_SelectedCellRange.Clear();
        SPOSITION pos2 = newRanges.GetHeadPosition();
        while(pos2){ m_SelectedCellRange.AddRange(newRanges.GetNext(pos2)); }
    }

    RecalcLayout();
    Invalidate();
    return TRUE;
}

BOOL SGridCtrl::SetFixedRowCount(int nFixedRows)
{
    if (nFixedRows < 0 || nFixedRows > m_nRows)
        return FALSE;

    m_nFixedRows = nFixedRows;
    Invalidate();
    return TRUE;
}

BOOL SGridCtrl::SetFixedColumnCount(int nFixedCols)
{
    if (nFixedCols < 0 || nFixedCols > m_nCols)
        return FALSE;

    m_nFixedCols = nFixedCols;
    Invalidate();
    return TRUE;
}

// Grid appearance implementation
int SGridCtrl::GetRowHeight(int nRow) const
{
    if (nRow < 0 || nRow >= m_nRows)
        return -1;

    return m_arRowHeights[nRow];
}

int SGridCtrl::GetColumnWidth(int nCol) const
{
    if (nCol < 0 || nCol >= m_nCols)
        return -1;

    return m_arColWidths[nCol];
}

BOOL SGridCtrl::SetRowHeight(int nRow, int height)
{
    if (nRow < 0 || nRow >= m_nRows || height < 0)
        return FALSE;

    m_arRowHeights[nRow] = height;
    RecalcLayout();
    Invalidate();
    return TRUE;
}

BOOL SGridCtrl::SetColumnWidth(int nCol, int width)
{
    if (nCol < 0 || nCol >= m_nCols || width < 0)
        return FALSE;

    m_arColWidths[nCol] = width;
    RecalcLayout();
    Invalidate();
    return TRUE;
}

int SGridCtrl::GetDefCellHeight() const
{
    return m_nDefCellHeight;
}

int SGridCtrl::GetDefCellWidth() const
{
    return m_nDefCellWidth;
}

void SGridCtrl::SetDefCellHeight(int nHeight)
{
    if (nHeight > 0)
    {
        m_nDefCellHeight = nHeight;
    }
}

void SGridCtrl::SetDefCellWidth(int nWidth)
{
    if (nWidth > 0)
    {
        m_nDefCellWidth = nWidth;
    }
}

// Image list implementation
void SGridCtrl::SetImageList(ISkinObj* pSkin)
{
    m_pImageList = pSkin;
    Invalidate();
}

ISkinObj* SGridCtrl::GetImageList() const
{
    return m_pImageList;
}

// Grid lines and appearance implementation
int SGridCtrl::GetGridLines() const
{
    return m_nGridLines;
}

void SGridCtrl::SetGridLines(int nWhichLines)
{
    m_nGridLines = nWhichLines;
    Invalidate();
}

COLORREF SGridCtrl::GetGridLineColor() const
{
    return m_crGridLineColor;
}

void SGridCtrl::SetGridLineColor(COLORREF clr)
{
    m_crGridLineColor = clr;
    Invalidate();
}

COLORREF SGridCtrl::GetGridBkColor() const
{
    return m_crGridBkColor;
}

void SGridCtrl::SetGridBkColor(COLORREF clr)
{
    m_crGridBkColor = clr;
    Invalidate();
}

// Grid behavior implementation
BOOL SGridCtrl::IsEditable() const
{
    return m_bEditable;
}

void SGridCtrl::SetEditable(BOOL bEditable)
{
    m_bEditable = bEditable;
}

BOOL SGridCtrl::GetListMode() const
{
    return m_bListMode;
}

void SGridCtrl::SetListMode(BOOL bEnableListMode)
{
    SelectNone();
    m_bListMode = bEnableListMode;
}

BOOL SGridCtrl::GetSingleRowSelection() const
{
    return m_bSingleRowSelection;
}

void SGridCtrl::SetSingleRowSelection(BOOL bSing)
{
    m_bSingleRowSelection = bSing;
}

BOOL SGridCtrl::GetSingleColSelection() const
{
    return m_bSingleColSelection;
}

void SGridCtrl::SetSingleColSelection(BOOL bSing)
{
    m_bSingleColSelection = bSing;
}

// Cell operations implementation
IGridCell* SGridCtrl::GetCell(int nRow, int nCol) const
{
    if (!IsValid(nRow, nCol))
        return NULL;

    SArray<IGridCell*>* pRow = m_RowData[nRow];
    if (!pRow)
        return NULL;

    IGridCell* pCell = (*pRow)[nCol];
    if (!pCell)
    {
        // Create cell on demand
        pCell = const_cast<SGridCtrl*>(this)->CreateCellByType(CELL_TYPE_TEXT);
        if (pCell)
        {
            pCell->Init(const_cast<SGridCtrl*>(this),nRow, nCol);
            // Copy properties from default fixed cell
            if(IsCellFixed(nRow, nCol)){
                if (m_pDefaultFixedCell)
                {
                    pCell->SetBackClr(m_pDefaultFixedCell->GetBackClr());
                    pCell->SetTextClr(m_pDefaultFixedCell->GetTextClr());
                    pCell->SetFormat(m_pDefaultFixedCell->GetFormat());
                }
            }
            else
            {
                // Copy properties from default cell
                if (m_pDefaultCell)
                {
                    pCell->SetBackClr(m_pDefaultCell->GetBackClr());
                    pCell->SetTextClr(m_pDefaultCell->GetTextClr());
                    pCell->SetFormat(m_pDefaultCell->GetFormat());
                }
            }
            (*pRow)[nCol] = pCell;
        }
    }

    return pCell;
}

BOOL SGridCtrl::SetItem(const SGV_ITEM* pItem)
{
    if (!pItem || !IsValid(pItem->row, pItem->col))
        return FALSE;

    IGridCell* pCell = GetCell(pItem->row, pItem->col);
    if (!pCell)
        return FALSE;

    if (pItem->mask & SGVIF_TEXT)
        pCell->SetText(pItem->szText);
    if (pItem->mask & SGVIF_IMAGE)
        pCell->SetImage(pItem->iImage);
    if (pItem->mask & SGVIF_PARAM)
        pCell->SetData(pItem->lParam);
    if (pItem->mask & SGVIF_STATE)
        pCell->SetState(pItem->state);
    if (pItem->mask & SGVIF_FORMAT)
        pCell->SetFormat(pItem->nFormat);
    if (pItem->mask & SGVIF_BKCLR)
        pCell->SetBackClr(pItem->crBkClr);
    if (pItem->mask & SGVIF_FGCLR)
        pCell->SetTextClr(pItem->crFgClr);
    if (pItem->mask & SGVIF_FONT)
        pCell->SetFont(&pItem->lfFont);

    return TRUE;
}

BOOL SGridCtrl::GetItem(SGV_ITEM* pItem) const
{
    if (!pItem || !IsValid(pItem->row, pItem->col))
        return FALSE;

    IGridCell* pCell = GetCell(pItem->row, pItem->col);
    if (!pCell)
        return FALSE;

    if (pItem->mask & SGVIF_TEXT)
        pItem->szText = pCell->GetText();
    if (pItem->mask & SGVIF_IMAGE)
        pItem->iImage = pCell->GetImage();
    if (pItem->mask & SGVIF_PARAM)
        pItem->lParam = pCell->GetData();
    if (pItem->mask & SGVIF_STATE)
        pItem->state = pCell->GetState();
    if (pItem->mask & SGVIF_FORMAT)
        pItem->nFormat = pCell->GetFormat();
    if (pItem->mask & SGVIF_BKCLR)
        pItem->crBkClr = pCell->GetBackClr();
    if (pItem->mask & SGVIF_FGCLR)
        pItem->crFgClr = pCell->GetTextClr();
    if (pItem->mask & SGVIF_FONT)
    {
        LOGFONT* pFont = pCell->GetFont();
        if (pFont)
            memcpy(&pItem->lfFont, pFont, sizeof(LOGFONT));
        else
            memset(&pItem->lfFont, 0, sizeof(LOGFONT));
    }

    return TRUE;
}

BOOL SGridCtrl::SetItemText(int nRow, int nCol, LPCTSTR str)
{
    IGridCell* pCell = GetCell(nRow, nCol);
    if (!pCell)
        return FALSE;

    pCell->SetText(str);
    RedrawCell(nRow, nCol);
    return TRUE;
}

BOOL SGridCtrl::GetCellRange(SCELLRANGE *pRange) const
{
    if (!pRange)
        return FALSE;
        
    pRange->minRow = 0;
    pRange->minCol = 0;
    pRange->maxRow = m_nRows - 1;
    pRange->maxCol = m_nCols - 1;
    
    return TRUE;
}

BOOL SGridCtrl::GetItemText(int nRow, int nCol, IStringT *str) const
{
    if (!str)
        return FALSE;

    IGridCell* pCell = GetCell(nRow, nCol);
    if (!pCell)
        return FALSE;

    str->Assign(pCell->GetText());
    return TRUE;
}

BOOL SGridCtrl::SetItemData(int nRow, int nCol, LPARAM lParam)
{
    IGridCell* pCell = GetCell(nRow, nCol);
    if (!pCell)
        return FALSE;

    pCell->SetData(lParam);
    return TRUE;
}

LPARAM SGridCtrl::GetItemData(int nRow, int nCol) const
{
    IGridCell* pCell = GetCell(nRow, nCol);
    if (!pCell)
        return 0;

    return pCell->GetData();
}

// Selection implementation
BOOL SGridCtrl::GetFocusCell(SCELLID *cell) const
{
    *cell = m_cellFocus;
    return TRUE;
}

BOOL SGridCtrl::SetFocusCell2(SCELLID cell)
{
    if (m_cellFocus == cell)
        return TRUE;
    
    SCellID oldFocus = m_cellFocus;
    // Remove focus from old cell
    if (IsValid2(oldFocus))
    {
        IGridCell* pOldCell = GetCell(oldFocus.row, oldFocus.col);
        if (pOldCell)
        {
            DWORD state = pOldCell->GetState();
            state &= ~SGVIS_FOCUSED;
            pOldCell->SetState(state);
            RedrawCell(oldFocus.row, oldFocus.col);
        }
    }

    if (IsValid2(cell))
    {
        // Set focus to new cell
        m_cellFocus = cell;
        IGridCell* pNewCell = GetCell(cell.row, cell.col);
        if (pNewCell)
        {
            DWORD state = pNewCell->GetState();
            state |= SGVIS_FOCUSED;
            pNewCell->SetState(state);
            RedrawCell(cell.row, cell.col);
        }
        // Ensure the focused cell is visible
        EnsureVisible2(cell);
    }else{
        m_cellFocus = SCellID(-1, -1);
    }
    return TRUE;
}

BOOL SGridCtrl::SetFocusCell(int nRow, int nCol)
{
    return SetFocusCell2(SCellID(nRow, nCol));
}

int SGridCtrl::GetSelectedCount() const
{
    int nCount = 0;
    for (int row = 0; row < m_nRows; row++)
    {
        for (int col = 0; col < m_nCols; col++)
        {
            if (IsCellSelected(SCellID(row, col)))
                nCount++;
        }
    }
    return nCount;
}

BOOL SGridCtrl::GetSelectedCellRange(SCELLRANGE *pRange) const
{
    if (!pRange) return FALSE;
    *pRange = m_SelectedCellRange.GetBoundingRange();
    return TRUE;
}

void SGridCtrl::SetSelectedRange2(const SCELLRANGE *pRange, BOOL bForceRepaint)
{
    SetSelectedRange(pRange->minRow, pRange->minCol, pRange->maxRow, pRange->maxCol, bForceRepaint);
}

void SGridCtrl::SetSelectedRange(int nMinRow, int nMinCol, int nMaxRow, int nMaxCol, BOOL bForceRepaint)
{
    // Clear old selection
    // Clear old selection highlights
    SPOSITION posOld = m_SelectedCellRange.GetRanges().GetHeadPosition();
    while(posOld)
    {
        SCellRange r = m_SelectedCellRange.GetRanges().GetNext(posOld);
        for (int row = r.minRow; row <= r.maxRow; row++)
        {
            for (int col = r.minCol; col <= r.maxCol; col++)
            {
                IGridCell* pCell = GetCell(row, col);
                if (pCell)
                {
                    DWORD state = pCell->GetState();
                    state &= ~SGVIS_SELECTED;
                    pCell->SetState(state);
                    if (bForceRepaint)
                        RedrawCell(row, col);
                }
            }
        }
    }

    // Set new selection
    m_SelectedCellRange.Clear();
    SCellRange newRange(nMinRow, nMinCol, nMaxRow, nMaxCol);
    if (newRange.IsValid() && IsRangeValid(&newRange))
    {
        m_SelectedCellRange.AddRange(newRange);
        // Highlight new selection
        SPOSITION pos = m_SelectedCellRange.GetRanges().GetHeadPosition();
        while(pos)
        {
            SCellRange r = m_SelectedCellRange.GetRanges().GetNext(pos);
            for (int row = r.minRow; row <= r.maxRow; row++)
            {
                for (int col = r.minCol; col <= r.maxCol; col++)
                {
                    IGridCell* pCell = GetCell(row, col);
                    if (pCell)
                    {
                        DWORD state = pCell->GetState();
                        state |= SGVIS_SELECTED;
                        pCell->SetState(state);
                        if (bForceRepaint)
                            RedrawCell(row, col);
                    }
                }
            }
        }
    }
}

BOOL SGridCtrl::IsCellVisible(SCELLID cell) const
{
    if (!IsValid2(cell))
        return FALSE;

    CRect rcCell = GetCellRect(cell.row, cell.col);
    CRect rcClient = GetClientRect();

    CRect rcIntersect;
    return rcIntersect.IntersectRect(rcCell, rcClient);
}

BOOL SGridCtrl::IsCellEditable(SCELLID cell) const
{
    if (!IsValid2(cell) || !m_bEditable)
        return FALSE;

    IGridCell* pCell = GetCell(cell.row, cell.col);
    if (!pCell)
        return TRUE; // Default to editable if no cell exists

    return !pCell->IsReadOnly();
}

BOOL SGridCtrl::SetCellEditable(THIS_ SCELLID cell, BOOL bEditable){
    if (!IsValid2(cell))
        return FALSE;
    if(bEditable && !m_bEditable)
        return FALSE;
    IGridCell* pCell = GetCell(cell.row, cell.col);
    if (!pCell)
        return FALSE;
    pCell->SetReadOnly(!bEditable);
    return TRUE;
}

BOOL SGridCtrl::IsCellSelected(SCELLID cell) const
{
    if (!IsValid2(cell))
        return FALSE;

    IGridCell* pCell = GetCell(cell.row, cell.col);
    if (!pCell)
        return FALSE;

    return pCell->IsSelected();
}

void SGridCtrl::EnsureVisible2(SCELLID cell)
{
    EnsureVisible(cell.row, cell.col);
}

int SGridCtrl::GetFixedColsWidth() const{
    int ret = 0;
    for (int i = 0; i < m_nFixedCols; i++)
        ret += GetColumnWidth(i);
    return ret;
}
int SGridCtrl::GetFixedRowsHeight() const{
    int ret = 0;
    for (int i = 0; i < m_nFixedRows; i++)
        ret += GetRowHeight(i);
    return ret;
}

void SGridCtrl::EnsureVisible(int nRow, int nCol)
{
    if (!IsValid(nRow, nCol))
        return;

    CRect rcCell = GetCellRectEx(nRow, nCol);
    CRect rcClient = GetClientRect();

    int nFixedColsWidth = GetFixedColsWidth();
    int nFixedRowsHeight = GetFixedRowsHeight();

    // Calculate scroll adjustments
    int nScrollX = 0, nScrollY = 0;
    if(nCol >= m_nFixedCols){
        if(rcCell.left < rcClient.left+nFixedColsWidth)
            nScrollX = rcClient.left+nFixedColsWidth - rcCell.left;
        else if(rcCell.right > rcClient.right)
            nScrollX = rcClient.right - rcCell.right;
    }
    if(nRow >= m_nFixedRows){
        if(rcCell.top < rcClient.top+nFixedRowsHeight)
            nScrollY = rcClient.top+nFixedRowsHeight - rcCell.top;
        else if(rcCell.bottom > rcClient.bottom)
            nScrollY =  rcClient.bottom - rcCell.bottom;
    }
    // Apply scroll adjustments
    if (nScrollX != 0)
    {
        SetScrollPos(FALSE,m_siHoz.nPos-nScrollX,TRUE);
    }

    if (nScrollY != 0)
    {
        SetScrollPos(TRUE,m_siVer.nPos-nScrollY,TRUE);
    }
}

BOOL SGridCtrl::GetVisibleCellRange(SCELLRANGE *pRange) const
{
    if (!pRange)
        return FALSE;

    CRect rcClient = GetClientRect();

    // Calculate fixed area sizes
    int nFixedWidth = 0;
    for (int i = 0; i < m_nFixedCols; i++)
    {
        nFixedWidth += GetColumnWidth(i);
    }

    int nFixedHeight = 0;
    for (int i = 0; i < m_nFixedRows; i++)
    {
        nFixedHeight += GetRowHeight(i);
    }

    // Find visible row range
    int nStartRow = m_nFixedRows;
    int nEndRow = m_nRows - 1;

    if (m_nFixedRows < m_nRows)
    {
        int y = nFixedHeight;
        int scrollY = m_siVer.nPos;

        // Find first visible row
        for (int i = m_nFixedRows; i < m_nRows; i++)
        {
            int nHeight = GetRowHeight(i);
            if (y - scrollY + nHeight > nFixedHeight)
            {
                nStartRow = i;
                break;
            }
            y += nHeight;
        }

        // Find last visible row
        y = nFixedHeight;
        for (int i = m_nFixedRows; i < m_nRows; i++)
        {
            int nHeight = GetRowHeight(i);
            if (y - scrollY > rcClient.Height())
            {
                nEndRow = smax(m_nFixedRows, i - 1);
                break;
            }
            y += nHeight;
        }
    }

    // Find visible column range
    int nStartCol = m_nFixedCols;
    int nEndCol = m_nCols - 1;

    if (m_nFixedCols < m_nCols)
    {
        int x = nFixedWidth;
        int scrollX = m_siHoz.nPos;

        // Find first visible column
        for (int i = m_nFixedCols; i < m_nCols; i++)
        {
            int nWidth = GetColumnWidth(i);
            if (x - scrollX + nWidth > nFixedWidth)
            {
                nStartCol = i;
                break;
            }
            x += nWidth;
        }

        // Find last visible column
        x = nFixedWidth;
        for (int i = m_nFixedCols; i < m_nCols; i++)
        {
            int nWidth = GetColumnWidth(i);
            if (x - scrollX > rcClient.Width())
            {
                nEndCol = smax(m_nFixedCols, i - 1);
                break;
            }
            x += nWidth;
        }
    }

    pRange->minRow = nStartRow;
    pRange->minCol = nStartCol;
    pRange->maxRow = nEndRow;
    pRange->maxCol = nEndCol;
    
    return TRUE;
}

BOOL SGridCtrl::GetCellFromPt(POINT point, BOOL bAllowFixedCells, SCELLID *pCell) const
{
    if (!pCell)
        return FALSE;

    CRect rcClient = GetClientRect();

    // Convert point to client coordinates
    point.x -= rcClient.left;
    point.y -= rcClient.top;

    // Calculate fixed area sizes
    int nFixedWidth = GetFixedColsWidth();
    int nFixedHeight = GetFixedRowsHeight();

    // Find row
    int nRow = -1;
    int y = 0;

    // Check fixed rows first
    for (int i = 0; i < m_nFixedRows && i < m_nRows; i++)
    {
        int nHeight = GetRowHeight(i);
        if (point.y >= y && point.y < y + nHeight)
        {
            nRow = i;
            break;
        }
        y += nHeight;
    }

    // If not in fixed rows, check scrollable rows
    if (nRow == -1 && point.y >= nFixedHeight)
    {
        int scrollY = point.y - nFixedHeight + m_siVer.nPos;
        y = nFixedHeight;
        for (int i = m_nFixedRows; i < m_nRows; i++)
        {
            int nHeight = GetRowHeight(i);
            if (scrollY >= y - nFixedHeight && scrollY < y - nFixedHeight + nHeight)
            {
                nRow = i;
                break;
            }
            y += nHeight;
        }
        if(nRow == -1){
            nRow = kMaxRow;
        }
    }

    // Find column
    int nCol = -1;
    int x = 0;

    // Check fixed columns first
    for (int i = 0; i < m_nFixedCols && i < m_nCols; i++)
    {
        int nWidth = GetColumnWidth(i);
        if (point.x >= x && point.x < x + nWidth)
        {
            nCol = i;
            break;
        }
        x += nWidth;
    }

    // If not in fixed columns, check scrollable columns
    if (nCol == -1 && point.x >= nFixedWidth)
    {
        int scrollX = point.x - nFixedWidth + m_siHoz.nPos;
        x = nFixedWidth;
        for (int i = m_nFixedCols; i < m_nCols; i++)
        {
            int nWidth = GetColumnWidth(i);
            if (scrollX >= x - nFixedWidth && scrollX < x - nFixedWidth + nWidth)
            {
                nCol = i;
                break;
            }
            x += nWidth;
        }
        if(nCol == -1){
            nCol = kMaxCol;
        }
    }

    // Check if fixed cells are allowed
    if (!bAllowFixedCells && IsCellFixed(nRow, nCol))
    {
        pCell->row = -1;
        pCell->col = -1;
        return FALSE;
    }

    pCell->row = nRow;
    pCell->col = nCol;
    return TRUE;
}

SCellID SGridCtrl::GetCellFromPt(POINT point, BOOL bAllowFixedCells) const
{
    SCellID cell;
    GetCellFromPt(point, bAllowFixedCells, &cell);
    return cell;
}

HRESULT SGridCtrl::OnAttrRowCount(const SStringW &strValue, BOOL bLoading)
{
    int nValue = Str2IntW(strValue, TRUE);   
    SetRowCount(nValue);
    return bLoading?S_OK:S_FALSE;
}

HRESULT SGridCtrl::OnAttrColCount(const SStringW &strValue, BOOL bLoading)
{
    int nValue = Str2IntW(strValue, TRUE);
    SetColumnCount(nValue);
    return bLoading ? S_OK : S_FALSE;
}

HRESULT SGridCtrl::OnAttrFixedRows(const SStringW &strValue, BOOL bLoading)
{
    int nValue = Str2IntW(strValue, TRUE);
    SetFixedRowCount(nValue);
    return bLoading ? S_OK : S_FALSE;
}

HRESULT SGridCtrl::OnAttrFixedCols(const SStringW &strValue, BOOL bLoading)
{
    int nValue = Str2IntW(strValue, TRUE);
    SetFixedColumnCount(nValue);
    return bLoading ? S_OK : S_FALSE;
}

// Operations implementation
int SGridCtrl::InsertColumn(LPCTSTR strHeading, UINT nFormat, int nColumn)
{
    if (nColumn < 0)
        nColumn = m_nCols;
    if (nColumn > m_nCols)
        return -1;

    // Insert column width
    m_arColWidths.InsertAt(nColumn, m_nDefCellWidth);

    // Insert cells in each row
    for (int i = 0; i < m_nRows; i++)
    {
        SArray<IGridCell*>* pRow = m_RowData[i];
        if (pRow)
        {
            pRow->InsertAt(nColumn, (IGridCell*)NULL);
        }
    }

    m_nCols++;

    // Set column header if provided
    if (strHeading && m_nFixedRows > 0)
    {
        SetItemText(0, nColumn, strHeading);
        IGridCell* pCell = GetCell(0, nColumn);
        if (pCell)
        {
            pCell->SetFormat(nFormat);
        }
    }

    RecalcLayout();
    Invalidate();
    return nColumn;
}

int SGridCtrl::InsertRow(LPCTSTR strHeading, int nRow)
{
    if (nRow < 0)
        nRow = m_nRows;
    if (nRow > m_nRows)
        return -1;

    // Insert row height
    m_arRowHeights.InsertAt(nRow, m_nDefCellHeight);

    // Create new row
    SArray<IGridCell*>* pNewRow = new SArray<IGridCell*>();
    pNewRow->SetCount(m_nCols);
    for (int i = 0; i < m_nCols; i++)
    {
        (*pNewRow)[i] = NULL;
    }

    m_RowData.InsertAt(nRow, pNewRow);
    m_nRows++;

    // Set row header if provided
    if (strHeading && m_nFixedCols > 0)
    {
        SetItemText(nRow, 0, strHeading);
    }

    RecalcLayout();
    Invalidate();
    return nRow;
}

BOOL SGridCtrl::DeleteColumn(int nColumn)
{
    if (nColumn < 0 || nColumn >= m_nCols)
        return FALSE;

    // Delete cells in each row
    for (int i = 0; i < m_nRows; i++)
    {
        SArray<IGridCell*>* pRow = m_RowData[i];
        if (pRow && nColumn < pRow->GetCount())
        {
            if ((*pRow)[nColumn])
            {
                (*pRow)[nColumn]->Release();
            }
            pRow->RemoveAt(nColumn);
        }
    }

    // Remove column width
    m_arColWidths.RemoveAt(nColumn);
    m_nCols--;

    // Adjust focus and selection
    if (m_cellFocus.col == nColumn)
        m_cellFocus.col = -1;
    else if (m_cellFocus.col > nColumn)
        m_cellFocus.col--;

    // Adjust selection ranges
    SList<SCellRange> newRanges;
    SPOSITION pos = m_SelectedCellRange.GetRanges().GetHeadPosition();
    while (pos)
    {
        SCellRange r = m_SelectedCellRange.GetRanges().GetNext(pos);
        // remove single-column ranges matching this column
        if (r.minCol == nColumn && r.maxCol == nColumn)
            continue;
        if (r.minCol > nColumn) r.minCol--;
        if (r.maxCol >= nColumn) r.maxCol--;
        if (r.IsValid()) newRanges.AddTail(r);
    }
    m_SelectedCellRange.Clear();
    SPOSITION pos2 = newRanges.GetHeadPosition();
    while(pos2){ m_SelectedCellRange.AddRange(newRanges.GetNext(pos2)); }

    RecalcLayout();
    Invalidate();
    return TRUE;
}

BOOL SGridCtrl::DeleteRow(int nRow)
{
    if (nRow < 0 || nRow >= m_nRows)
        return FALSE;

    // Delete row data
    SArray<IGridCell*>* pRow = m_RowData[nRow];
    if (pRow)
    {
        for (int i = 0; i < pRow->GetCount(); i++)
        {
            if ((*pRow)[i])
            {
                (*pRow)[i]->Release();
            }
        }
        delete pRow;
    }

    m_RowData.RemoveAt(nRow);
    m_arRowHeights.RemoveAt(nRow);
    m_nRows--;

    // Adjust focus and selection
    if (m_cellFocus.row == nRow)
        m_cellFocus.row = -1;
    else if (m_cellFocus.row > nRow)
        m_cellFocus.row--;

    // Adjust selection ranges
    SList<SCellRange> newRanges;
    SPOSITION pos = m_SelectedCellRange.GetRanges().GetHeadPosition();
    while (pos)
    {
        SCellRange r = m_SelectedCellRange.GetRanges().GetNext(pos);
        if (r.minRow == nRow && r.maxRow == nRow)
            continue;
        if (r.minRow > nRow) r.minRow--;
        if (r.maxRow >= nRow) r.maxRow--;
        if (r.IsValid()) newRanges.AddTail(r);
    }
    m_SelectedCellRange.Clear();
    SPOSITION pos2 = newRanges.GetHeadPosition();
    while(pos2){ m_SelectedCellRange.AddRange(newRanges.GetNext(pos2)); }

    RecalcLayout();
    Invalidate();
    return TRUE;
}

BOOL SGridCtrl::DeleteAllItems()
{
    ClearCells();
    m_nRows = 0;
    m_nCols = 0;
    m_arRowHeights.RemoveAll();
    m_arColWidths.RemoveAll();
    m_cellFocus = SCellID(-1, -1);
    m_SelectedCellRange.Clear();

    RecalcLayout();
    Invalidate();
    return TRUE;
}

BOOL SGridCtrl::DeleteNonFixedRows()
{
    if (m_nFixedRows >= m_nRows)
        return TRUE;

    // Delete non-fixed rows
    for (int i = m_nRows - 1; i >= m_nFixedRows; i--)
    {
        DeleteRow(i);
    }

    return TRUE;
}

// Auto-sizing implementation
BOOL SGridCtrl::AutoSizeRow(int nRow, BOOL bResetScroll)
{
    if (nRow < 0 || nRow >= m_nRows)
        return FALSE;

    int nMaxHeight = m_nDefCellHeight;

    // Find the maximum height needed for this row
    for (int col = 0; col < m_nCols; col++)
    {
        IGridCell* pCell = GetCell(nRow, col);
        if (pCell)
        {
            // TODO: Calculate actual text height based on cell content
            // For now, use default height
            nMaxHeight = smax(nMaxHeight, m_nDefCellHeight);
        }
    }

    SetRowHeight(nRow, nMaxHeight);

    if (bResetScroll)
    {
        UpdateScrollBars();
    }

    return TRUE;
}

BOOL SGridCtrl::AutoSizeColumn(int nCol, UINT nAutoSizeStyle, BOOL bResetScroll)
{
    if (nCol < 0 || nCol >= m_nCols)
        return FALSE;

    int nMaxWidth = m_nDefCellWidth;

    // Determine which rows to check based on style
    int nStartRow = 0;
    int nEndRow = m_nRows - 1;

    if (nAutoSizeStyle == SGVS_HEADER)
    {
        nEndRow = m_nFixedRows - 1;
    }
    else if (nAutoSizeStyle == SGVS_DATA)
    {
        nStartRow = m_nFixedRows;
    }

    // Find the maximum width needed for this column
    for (int row = nStartRow; row <= nEndRow; row++)
    {
        IGridCell* pCell = GetCell(row, nCol);
        if (pCell)
        {
            // TODO: Calculate actual text width based on cell content
            // For now, estimate based on text length
            SStringT strText = pCell->GetText();
            if (!strText.IsEmpty())
            {
                int nEstimatedWidth = strText.GetLength() * 8 + 20; // Rough estimate
                nMaxWidth = smax(nMaxWidth, nEstimatedWidth);
            }
        }
    }

    SetColumnWidth(nCol, nMaxWidth);

    if (bResetScroll)
    {
        UpdateScrollBars();
    }

    return TRUE;
}

void SGridCtrl::AutoSizeRows()
{
    for (int row = 0; row < m_nRows; row++)
    {
        AutoSizeRow(row, FALSE);
    }
    UpdateScrollBars();
}

void SGridCtrl::AutoSizeColumns(UINT nAutoSizeStyle)
{
    for (int col = 0; col < m_nCols; col++)
    {
        AutoSizeColumn(col, nAutoSizeStyle, FALSE);
    }
    UpdateScrollBars();
}

void SGridCtrl::AutoSize(UINT nAutoSizeStyle)
{
    AutoSizeRows();
    AutoSizeColumns(nAutoSizeStyle);
}

// Validation and utilities implementation
BOOL SGridCtrl::IsValid(int nRow, int nCol) const
{
    return (nRow >= 0 && nRow < m_nRows && nCol >= 0 && nCol < m_nCols);
}

BOOL SGridCtrl::IsValid2(SCELLID cell) const
{
    return IsValid(cell.row, cell.col);
}

BOOL SGridCtrl::IsRangeValid(const SCELLRANGE* range) const
{
    return (range->minRow >= 0 && range->minCol >= 0 &&
            range->maxRow < m_nRows && range->maxCol < m_nCols &&
            range->minRow <= range->maxRow && range->minCol <= range->maxCol);
}

BOOL SGridCtrl::IsCellFixed(int nRow, int nCol) const
{
    return ((nRow>=0 && nRow < m_nFixedRows) || (nCol>=0 && nCol < m_nFixedCols));
}

BOOL SGridCtrl::IsCellFixed2(SCELLID cell) const
{
    return IsCellFixed(cell.row, cell.col);
}

void SGridCtrl::DestroyCell(int nRow, int nCol)
{
    if (!IsValid(nRow, nCol))
        return;

    SArray<IGridCell*>* pRow = m_RowData[nRow];
    if (pRow && (*pRow)[nCol])
    {
        (*pRow)[nCol]->Release();
        (*pRow)[nCol] = NULL;
    }
}

IGridCell* SGridCtrl::GetDefaultCell(BOOL bFixedRow, BOOL bFixedCol) const
{
    if (bFixedRow || bFixedCol)
        return m_pDefaultFixedCell;
    else
        return m_pDefaultCell;
}

// GetCellRectEx is used to get the cell rectangle of a cell including merged cells
CRect SGridCtrl::GetCellRectEx(const SCellID &cell) const{
    return GetCellRectEx(cell.row, cell.col);
}

CRect SGridCtrl::GetCellRectEx(int nRow, int nCol) const{
    const SMergedCell * pMergedCell = GetMergedCell(nRow, nCol);
    if(pMergedCell){
        SCellRange range = pMergedCell->range;
        return GetRangeRect(range);
    }else{
        return GetCellRect(nRow, nCol);
    }
}

BOOL SGridCtrl:: GetCellRect(THIS_ int nRow, int nCol, RECT *pRect) SCONST{
    if(!IsValid(nRow, nCol))
        return FALSE;
    *pRect = GetCellRect(nRow, nCol);
    return TRUE;
}
BOOL SGridCtrl::GetCellRectEx(THIS_ int nRow, int nCol, RECT *pRect) SCONST{
    if(!IsValid(nRow, nCol))
        return FALSE;
    *pRect = GetCellRectEx(nRow, nCol);
    return TRUE;    
}

CRect SGridCtrl::GetCellRect(const SCellID& cell) const{
    return GetCellRect(cell.row, cell.col);
}

// Layout and positioning
CRect SGridCtrl::GetCellRect(int nRow, int nCol) const
{
    if (!IsValid(nRow, nCol))
        return CRect(0, 0, 0, 0);

    CRect rcClient = GetClientRect();
    CRect rcCell;

    // Calculate fixed area sizes
    int nFixedWidth = GetFixedColsWidth();
    int nFixedHeight = GetFixedRowsHeight();

    // Calculate left position
    rcCell.left = 0;
    for (int i = 0; i < nCol; i++)
    {
        rcCell.left += GetColumnWidth(i);
    }

    // Apply horizontal scrolling for non-fixed columns
    if (nCol >= m_nFixedCols)
    {
        rcCell.left -= m_siHoz.nPos;
    }

    // Calculate top position
    rcCell.top = 0;
    for (int i = 0; i < nRow; i++)
    {
        rcCell.top += GetRowHeight(i);
    }

    // Apply vertical scrolling for non-fixed rows
    if (nRow >= m_nFixedRows)
    {
        rcCell.top -= m_siVer.nPos;
    }

    // Calculate right and bottom
    rcCell.right = rcCell.left + GetColumnWidth(nCol);
    rcCell.bottom = rcCell.top + GetRowHeight(nRow);

    // Offset by client area
    rcCell.OffsetRect(rcClient.TopLeft());

    return rcCell;
}

void SGridCtrl::RecalcLayout()
{
    // Calculate total grid size
    m_sizeGrid.cx = 0;
    for (int i = 0; i < m_nCols; i++)
    {
        m_sizeGrid.cx += GetColumnWidth(i);
    }

    m_sizeGrid.cy = 0;
    for (int i = 0; i < m_nRows; i++)
    {
        m_sizeGrid.cy += GetRowHeight(i);
    }

    UpdateScrollBars();
}

void SGridCtrl::UpdateScrollBars()
{
    // Get client area size
    CRect rcClient = SWindow::GetClientRect();
    //  关闭滚动条
    m_wBarVisible = SSB_NULL;

    // Calculate fixed area sizes
    int nFixedWidth = GetFixedColsWidth();
    int nFixedHeight = GetFixedRowsHeight();

    // Calculate scrollable area
    int nScrollableWidth = m_sizeGrid.cx - nFixedWidth;
    int nScrollableHeight = m_sizeGrid.cy - nFixedHeight;

    int nClientScrollableWidth = rcClient.Width() - nFixedWidth;
    int nClientScrollableHeight = rcClient.Height() - nFixedHeight;

    SCROLLINFO si = {0};
    si.cbSize = sizeof(SCROLLINFO);
    si.fMask = SIF_RANGE | SIF_PAGE | SIF_POS;
    
    if(nScrollableHeight > nClientScrollableHeight)
    {
        nClientScrollableWidth -= GetSbWidth();
        m_wBarVisible |= SSB_VERT;
        // Update vertical scrollbar
        si.nMax = smax(0, nScrollableHeight);
        if(nScrollableWidth > nClientScrollableWidth)
        {
            nClientScrollableHeight -= GetSbWidth();
        }
        si.nPage = smax(0, nClientScrollableHeight);
        si.nPos = m_siVer.nPos;
        SetScrollInfo(si,TRUE);
        if(nScrollableWidth > nClientScrollableWidth)
        {
            m_wBarVisible |= SSB_HORZ;
            // Update horizontal scrollbar
            si.nMax = smax(0, nScrollableWidth);
            si.nPage = smax(0, nClientScrollableWidth);
            si.nPos = m_siHoz.nPos;
            SetScrollInfo(si,FALSE);
        }
    }else if(nScrollableWidth > nClientScrollableWidth)
    {
        m_wBarVisible |= SSB_HORZ;
        // Update horizontal scrollbar
        si.nMin = 0;
        si.nMax = smax(0, nScrollableWidth);
        si.nPage = smax(0, nClientScrollableWidth);
        si.nPos = m_siHoz.nPos;
        SetScrollInfo(si,FALSE);
    }
    SSendMessage(WM_NCCALCSIZE);
}

void SGridCtrl::ClearCells()
{
    for (int i = 0; i < m_RowData.GetCount(); i++)
    {
        SArray<IGridCell*>* pRow = m_RowData[i];
        if (pRow)
        {
            for (int j = 0; j < pRow->GetCount(); j++)
            {
                if ((*pRow)[j])
                {
                    (*pRow)[j]->Release();
                    (*pRow)[j] = NULL;
                }
            }
            delete pRow;
        }
    }
    m_RowData.RemoveAll();
}

// Drawing implementation
void SGridCtrl::OnPaint(IRenderTarget *pRT)
{
    SPainter painter;
    BeforePaint(pRT, painter);

    CRect rcClient = GetClientRect();

    // Fill background
    pRT->FillSolidRect(&rcClient, m_crGridBkColor);

    // Draw cells
    DrawCells(pRT, rcClient);

    // Draw grid lines
    if (m_nGridLines != SGVL_NONE)
    {
        DrawGridLines(pRT, rcClient);
    }
    DrawMergedCells(pRT,rcClient);

    AfterPaint(pRT, painter);
}

void SGridCtrl::DrawCells(IRenderTarget* pRT, CRect rc)
{
    if (m_nRows == 0 || m_nCols == 0)
        return;
    // Calculate visible cell range
    SCellRange visibleRange;
    GetVisibleCellRange(&visibleRange);
    int nFixedWidth = GetFixedColsWidth();
    int nFixedHeight = GetFixedRowsHeight();
    CRect rcFixed(rc.TopLeft(), CSize(nFixedWidth, nFixedHeight));
    pRT->PushClipRect(&rcFixed, RGN_DIFF);
    // Draw fixed row headers (scroll horizontally)
    for (int nRow = 0; nRow < m_nFixedRows; nRow++)
    {
        for (int nCol = smax(m_nFixedCols, visibleRange.minCol);
             nCol <= smin(m_nCols - 1, visibleRange.maxCol); nCol++)
        {
            CRect rcCell = GetCellRect(nRow, nCol);
            CRect rcIntersect;
            if (rcIntersect.IntersectRect(rcCell, rc))
            {
                DrawFixedCell(pRT, nRow, nCol, rcCell);
            }
        }
    }

    // Draw fixed column headers (scroll vertically)
    for (int nRow = smax(m_nFixedRows, visibleRange.minRow);
         nRow <= smin(m_nRows - 1, visibleRange.maxRow); nRow++)
    {
        for (int nCol = 0; nCol < m_nFixedCols; nCol++)
        {
            CRect rcCell = GetCellRect(nRow, nCol);
            CRect rcIntersect;
            if (rcIntersect.IntersectRect(rcCell, rc))
            {
                DrawFixedCell(pRT, nRow, nCol, rcCell);
            }
        }
    }
    pRT->PopClip();
    // Draw fixed corner
    for (int nRow = 0; nRow < m_nFixedRows; nRow++)
    {
        for (int nCol = 0; nCol < m_nFixedCols; nCol++)
        {
            CRect rcCell = GetCellRect(nRow, nCol);
            CRect rcIntersect;
            if (rcIntersect.IntersectRect(rcCell, rc))
            {
                DrawFixedCell(pRT, nRow, nCol, rcCell);
            }
        }
    }


    rc.left += nFixedWidth;
    rc.top += nFixedHeight;
    pRT->PushClipRect(&rc, RGN_AND);
    // Draw scrollable cells
    for (int nRow = smax(m_nFixedRows, visibleRange.minRow);
         nRow <= smin(m_nRows - 1, visibleRange.maxRow); nRow++)
    {
        for (int nCol = smax(m_nFixedCols, visibleRange.minCol);
             nCol <= smin(m_nCols - 1, visibleRange.maxCol); nCol++)
        {
            if(IsCellMerged(nRow, nCol))//ignore merged cell
                continue;
            CRect rcCell = GetCellRect(nRow, nCol);
            CRect rcIntersect;
            if (rcIntersect.IntersectRect(rcCell, rc))
            {
                DrawCell(pRT, nRow, nCol, rcCell);
            }
        }
    }
    pRT->PopClip();
}

void SGridCtrl::DrawFixedCell(IRenderTarget* pRT, int nRow, int nCol, const CRect& rect)
{
    if (!pRT)
        return;

    IGridCell* pCell = GetCell(nRow, nCol);
    if (pCell)
    {
        pCell->Draw(pRT, nRow, nCol, rect, TRUE);
    }
    else
    {
        // Draw default fixed cell with 3D effect
        COLORREF crBk = RGBA(240, 240, 240,255); // Default fixed cell background
        pRT->FillSolidRect(&rect, crBk);

        // Draw 3D border effect
        SAutoRefPtr<IPenS> pPenLight, pPenDark;
        pRT->CreatePen(PS_SOLID, RGBA(255, 255, 255,255), 1, &pPenLight); // Light edge
        pRT->CreatePen(PS_SOLID, RGBA(128, 128, 128,255), 1, &pPenDark);  // Dark edge

        if (pPenLight && pPenDark)
        {
            SAutoRefPtr<IPenS> pOldPen;

            // Draw light edges (top and left)
            pRT->SelectObject(pPenLight, (IRenderObj**)&pOldPen);
            {
                POINT pt[2]={{rect.left,rect.bottom-1},{rect.left,rect.top}};
                pRT->DrawLines(pt,2);
            }
            {
                POINT pt[2]={{rect.left,rect.top},{rect.right-1,rect.top}};
                pRT->DrawLines(pt,2);
            }

            // Draw dark edges (bottom and right)
            pRT->SelectObject(pPenDark, NULL);
            {
                POINT pt[2]={{rect.right-1,rect.top},{rect.right-1, rect.bottom-1}};
                pRT->DrawLines(pt,2);
            }
            {
                //todo:hjx
                POINT pt[2]={{rect.right-1, rect.bottom-1},{rect.left, rect.bottom-1}};
                pRT->DrawLines(pt,2);
            }
            pRT->SelectObject(pOldPen, NULL);
        }

        // Draw sort arrow if this is the sorted column
        if (nRow < m_nFixedRows && nCol == m_nSortColumn && m_nSortOrder != SORT_NONE)
        {
            DrawSortArrow(pRT, rect, (int)m_nSortOrder);
        }
    }
}

void SGridCtrl::DrawMergedCells(IRenderTarget* pRT, CRect rc){
    int nFixedWidth = GetFixedColsWidth();
    int nFixedHeight = GetFixedRowsHeight();
    rc.left += nFixedWidth;
    rc.top += nFixedHeight;
    pRT->PushClipRect(&rc, RGN_AND);

    for(int i=0;i<m_MergedCells.GetCount();i++){
        // Calculate merged cell rectangle
        SMergedCell & mergedCell = m_MergedCells.GetAt(i);
        CRect rcMerged = GetRangeRect(mergedCell.range);
        rcMerged.left++;
        rcMerged.top++;
        // Draw the merged cell using master cell's data
        IGridCell* pCell = GetCell(mergedCell.masterCell.row, mergedCell.masterCell.col);
        if (pCell)
        {
            pCell->Draw(pRT, mergedCell.masterCell.row, mergedCell.masterCell.col, rcMerged, TRUE);
        }
        else
        {
            // Draw default merged cell
            COLORREF crBk = RGBA(255, 255, 255,255);
            pRT->FillSolidRect(&rcMerged, crBk);
        }
    }
    pRT->PopClip();
}

void SGridCtrl::DrawCell(IRenderTarget* pRT, int nRow, int nCol, const CRect& rect)
{
    // Check if this cell is part of a merged cell
    const SMergedCell* pMergedCell = GetMergedCell(nRow, nCol);
    if (pMergedCell)
    {
        // If this is not the master cell, don't draw anything
        if (pMergedCell->masterCell.row != nRow || pMergedCell->masterCell.col != nCol)
            return;

        // Calculate merged cell rectangle
        CRect rcMerged = GetRangeRect(pMergedCell->range);

        // Draw the merged cell using master cell's data
        IGridCell* pCell = GetCell(pMergedCell->masterCell.row, pMergedCell->masterCell.col);
        if (pCell)
        {
            pCell->Draw(pRT, nRow, nCol, rcMerged, TRUE);
        }
        else
        {
            // Draw default merged cell
            COLORREF crBk = RGBA(255, 255, 255,255);
            pRT->FillSolidRect(&rcMerged, crBk);
        }
    }
    else
    {
        // Draw normal cell
        IGridCell* pCell = GetCell(nRow, nCol);
        if (pCell)
        {
            pCell->Draw(pRT, nRow, nCol, rect, TRUE);
        }
        else
        {
            // Draw default cell
            COLORREF crBk = RGBA(255, 255, 255,255); // Default cell background
            pRT->FillSolidRect(&rect, crBk);
        }
    }
}

void SGridCtrl::DrawGridLines(IRenderTarget* pRT, const CRect& rect)
{
    if (!pRT || m_nGridLines == SGVL_NONE)
        return;

    SAutoRefPtr<IPenS> pPen;
    pRT->CreatePen(PS_SOLID, m_crGridLineColor, 1, &pPen);

    if (!pPen)
        return;

    SAutoRefPtr<IPenS> pOldPen;
    pRT->SelectObject(pPen, (IRenderObj**)&pOldPen);

    // Draw vertical lines
    if (m_nGridLines & SGVL_VERT)
    {
        int nCol =0;
        int x = rect.left;
        for(;nCol<m_nFixedCols;nCol++)
        {
            x += GetColumnWidth(nCol);
            if (x >= rect.left && x <= rect.right)
            {
                POINT pt[2]={{x,rect.top},{x,rect.bottom}};
                pRT->DrawLines(pt,2);
            }
        }
        int xMin = x;
        x -= m_siHoz.nPos;
        for (; nCol <= m_nCols; nCol++)
        {
            if (x >= xMin && x <= rect.right)
            {
                POINT pt[2]={{x,rect.top},{x,rect.bottom}};
                pRT->DrawLines(pt,2);
            }
            if (nCol < m_nCols)
                x += GetColumnWidth(nCol);
        }
    }

    // Draw horizontal lines
    if (m_nGridLines & SGVL_HORZ)
    {
        int nRow =0;
        int y = rect.top;
        for(;nRow<m_nFixedRows;nRow++)
        {
            y += GetRowHeight(nRow);
            if (y >= rect.top && y <= rect.bottom)
            {
                POINT pt[2]={{rect.left,y},{rect.right,y}};
                pRT->DrawLines(pt,2);
            }
        }
        int yMin = y;
        y -= m_siVer.nPos;
        for (; nRow <= m_nRows; nRow++)
        {
            if (y >= yMin && y <= rect.bottom)
            {
                POINT pt[2]={{rect.left,y},{rect.right,y}};
                pRT->DrawLines(pt,2);
            }
            if (nRow < m_nRows)
                y += GetRowHeight(nRow);
        }
    }

    if (pOldPen)
        pRT->SelectObject(pOldPen, NULL);
}

// Event handlers implementation
void SGridCtrl::OnSize(UINT nType, CSize size)
{
    __baseCls::OnSize(nType, size);
    UpdateScrollBars();
}

BOOL SGridCtrl::OnScroll(BOOL bVertical, UINT uCode, int nPos)
{
    BOOL ret = __baseCls::OnScroll(bVertical, uCode, nPos);
    if(ret && IsInplaceEditing()){
        EndInplaceEdit(TRUE);
    }
    Invalidate();
    return ret;
}

void SGridCtrl::OnLButtonDown(UINT nFlags, CPoint pt)
{
    // End any existing edit
    if (m_bInEdit)
    {
        EndEdit(TRUE);
    }
    __baseCls::OnLButtonDown(nFlags, pt);

    // Store mouse state
    m_bLMouseButtonDown = TRUE;
    m_LeftClickDownPoint = pt;
    m_LastMousePoint = pt;

    SCellID cell = GetCellFromPt(pt, TRUE);
    m_LeftClickDownCell = cell;

    // Check for resize operations first
    if (m_MouseMode == MOUSE_OVER_COL_DIVIDE)
    {
        // Start column resize
        SCellID resizeCell = GetCellFromPt(pt, TRUE);
        if (resizeCell.IsValid())
        {
            // Determine which column to resize
            CRect cellRect = GetCellRect(resizeCell.row, resizeCell.col);
            int nCol = resizeCell.col;
            if (pt.x - cellRect.left < m_nResizeCaptureRange && nCol > 0)
                nCol--;

            StartColumnResize(nCol, pt);
        }
        return;
    }
    else if (m_MouseMode == MOUSE_OVER_ROW_DIVIDE)
    {
        // Start row resize
        SCellID resizeCell = GetCellFromPt(pt, TRUE);
        if (resizeCell.IsValid())
        {
            // Determine which row to resize
            CRect cellRect = GetCellRect(resizeCell.row, resizeCell.col);
            int nRow = resizeCell.row;
            if (pt.y - cellRect.top < m_nResizeCaptureRange && nRow > 0)
                nRow--;

            StartRowResize(nRow, pt);
        }
        return;
    }

    if (cell.IsValid())
    {
        // Handle fixed cell clicks
        if (IsCellFixed(cell.row, cell.col))
        {
            OnFixedCellClick(cell);
        }
        else
        {
            const SMergedCell *pMergedCell = GetMergedCell(cell);
            if(pMergedCell){
                cell = pMergedCell->masterCell;
            }
            // Check if clicking on already selected cell (for drag preparation)
            if (IsCellSelected(cell))
            {
                SetFocusCell(cell.row, cell.col);

                // If control is pressed, unselect the cell
                if (nFlags & MK_CONTROL)
                {
                    ToggleCellSelection(cell);
                    return;
                }

                // Prepare for potential drag operation
                if (m_bAllowDragAndDrop)
                {
                    m_MouseMode = MOUSE_PREPARE_DRAG;
                }
            }
            else
            {
                // Handle selection based on mode and modifiers
                if (nFlags & MK_CONTROL)
                {
                    // Toggle selection of this cell
                    ToggleCellSelection(cell);
                }
                else if (nFlags & MK_SHIFT)
                {
                    // Extend selection from focus cell to this cell
                    ExtendSelection(cell);
                }
                else
                {
                    // Start new selection
                    m_SelectionStartCell = cell;

                    if (m_bListMode && m_bSingleRowSelection)
                    {
                        m_MouseMode = MOUSE_SELECT_ROW;
                    }
                    else if (m_bSingleColSelection)
                    {
                        m_MouseMode = MOUSE_SELECT_COL;
                    }
                    else
                    {
                        m_MouseMode = MOUSE_SELECT_CELLS;
                    }

                    OnSelecting(cell);
                    IGridCell *pCell = GetCell(cell.row, cell.col);
                    if(pCell){
                        CRect rcCell = GetCellRect(cell.row, cell.col);
                        CPoint pt2 = pt-rcCell.TopLeft();
                        pCell->OnClickDown(pt2);
                    }
                }

                SetFocusCell(cell.row, cell.col);
            }
        }
    }
}

void SGridCtrl::OnLButtonUp(UINT nFlags, CPoint pt)
{
    m_bLMouseButtonDown = FALSE;
    // Handle mouse mode cleanup
    switch (m_MouseMode)
    {
    case MOUSE_PREPARE_DRAG:
        // If we were preparing to drag but didn't actually drag,
        // treat it as a normal click
        m_MouseMode = MOUSE_NOTHING;
        break;

    case MOUSE_DRAGGING:
        // End drag operation
        m_MouseMode = MOUSE_NOTHING;
        break;

    case MOUSE_SELECT_CELLS:
    case MOUSE_SELECT_ROW:
    case MOUSE_SELECT_COL:
        // End selection operation
        m_MouseMode = MOUSE_NOTHING;
        break;

    case MOUSE_SIZING_ROW:
    case MOUSE_SIZING_COL:
        // End resize operation
        EndResize();
        break;

    default:
        m_MouseMode = MOUSE_NOTHING;
        break;
    }
    if(m_MouseMode == MOUSE_NOTHING){
        IGridCell* pCell = GetCell(m_LeftClickDownCell.row, m_LeftClickDownCell.col);
        if (pCell){
            CRect rcCell = GetCellRect(m_LeftClickDownCell.row, m_LeftClickDownCell.col);
            CPoint pt2 = pt-rcCell.TopLeft();
            pCell->OnClickUp(pt2);
        }

    }
    __baseCls::OnLButtonUp(nFlags, pt);
}

void SGridCtrl::OnLButtonDbClick(UINT nFlags, CPoint pt)
{
    __baseCls::OnLButtonDbClick(nFlags, pt);
    SCellID cell;
    GetCellFromPt(pt, FALSE, &cell);

    if (cell.IsValid() && !IsCellFixed(cell.row, cell.col))
    {
        OnCellDblClick(cell, nFlags,pt);
    }
}

void SGridCtrl::OnRButtonDown(UINT nFlags, CPoint pt)
{
    SCellID cell;
    GetCellFromPt(pt, TRUE, &cell);

    if (cell.IsValid())
    {
        OnCellRClick(cell, nFlags,pt);
    }

    __baseCls::OnRButtonDown(nFlags, pt);
}

void SGridCtrl::OnMouseMove(UINT nFlags, CPoint pt)
{
    SCellID cell = GetCellFromPt(pt, TRUE);
    // Handle mouse operations when left button is down
    if (m_bLMouseButtonDown)
    {
        switch (m_MouseMode)
        {
        case MOUSE_PREPARE_DRAG:
            // Check if we've moved far enough to start dragging
            {
                CSize dragSize(abs(pt.x - m_LeftClickDownPoint.x),
                              abs(pt.y - m_LeftClickDownPoint.y));
                if (dragSize.cx > 3 || dragSize.cy > 3) // Drag threshold
                {
                    OnBeginDrag();
                }
            }
            break;

        case MOUSE_SELECT_CELLS:
        case MOUSE_SELECT_ROW:
        case MOUSE_SELECT_COL:
            // Continue selection
            if(cell.row < m_nFixedRows )
                cell.row = m_nFixedRows;
            if(cell.col < m_nFixedCols )
                cell.col = m_nFixedCols;
            if(cell.row == kMaxRow)
                cell.row = m_nRows - 1;
            if(cell.col == kMaxCol)
                cell.col = m_nCols - 1;
            if (cell.IsValid())
            {
                OnSelecting(cell);
            }
            break;

        case MOUSE_DRAGGING:
            // Handle drag over
            OnDragOver(pt);
            break;

        case MOUSE_SIZING_ROW:
            // Handle row resizing
            DoRowResize(pt);
            break;

        case MOUSE_SIZING_COL:
            // Handle column resizing
            DoColumnResize(pt);
            break;
        }
    }
    else
    {
        // Handle cursor changes when not dragging
        if (m_bAllowColumnResize && MouseOverResizeArea(pt,TRUE))
        {
            if (m_MouseMode != MOUSE_OVER_COL_DIVIDE)
            {
                m_MouseMode = MOUSE_OVER_COL_DIVIDE;
            }
        }
        else if (m_bAllowRowResize && MouseOverResizeArea(pt,FALSE))
        {
            if (m_MouseMode != MOUSE_OVER_ROW_DIVIDE)
            {
                m_MouseMode = MOUSE_OVER_ROW_DIVIDE;
            }
        }
        else if (m_MouseMode == MOUSE_OVER_COL_DIVIDE || m_MouseMode == MOUSE_OVER_ROW_DIVIDE)
        {
            m_MouseMode = MOUSE_NOTHING;
        }
    }

    // Handle hover effects
    if (cell != m_lastHoverCell)
    {
        // Mouse left previous cell
        if (m_lastHoverCell.IsValid())
        {
            OnCellMouseLeave(m_lastHoverCell);
        }

        // Mouse entered new cell
        if (cell.IsValid())
        {
            OnCellMouseEnter(cell);
        }

        m_lastHoverCell = cell;
    }

    if (cell.IsValid())
    {
        OnCellMouseOver(cell);
    }

    m_LastMousePoint = pt;
}

BOOL SGridCtrl::OnSetCursor(const CPoint &pt)
{
    switch(m_MouseMode){
    case MOUSE_OVER_COL_DIVIDE:
        SetCursor(GETRESPROVIDER->LoadCursor(IDC_SIZEWE));
        return TRUE;
    case MOUSE_OVER_ROW_DIVIDE:
        SetCursor(GETRESPROVIDER->LoadCursor(IDC_SIZENS));
        return TRUE;
    default:
    {
        IGridCell *pCell = GetCell(m_lastHoverCell.row, m_lastHoverCell.col);
        if (!pCell)
            return FALSE;
        CRect rcCell = GetCellRect(m_lastHoverCell.row, m_lastHoverCell.col);
        CPoint pt2 = pt - rcCell.TopLeft();
        if (pCell->OnSetCursor(pt2))
            return TRUE;
        break;
    }
    }
    return FALSE;
}

void SGridCtrl::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
    // Handle edit mode first
    if (m_bInEdit)
    {
        OnEditKeyDown(nChar, nRepCnt, nFlags);
        return;
    }

    if (!m_cellFocus.IsValid())
    {
        SetMsgHandled(FALSE);
        return;
    }

    SCellID newFocus = m_cellFocus;
    BOOL bShift = (GetKeyState(VK_SHIFT) & 0x8000) != 0;
    BOOL bCtrl = (GetKeyState(VK_CONTROL) & 0x8000) != 0;

    switch (nChar)
    {
        case VK_LEFT:
            if (OnKeyLeft(newFocus, bShift, bCtrl))
                return;
            break;

        case VK_RIGHT:
            if (OnKeyRight(newFocus, bShift, bCtrl))
                return;
            break;

        case VK_UP:
            if (OnKeyUp(newFocus, bShift, bCtrl))
                return;
            break;

        case VK_DOWN:
            if (OnKeyDownNav(newFocus, bShift, bCtrl))
                return;
            break;

        case VK_HOME:
            if (OnKeyHome(newFocus, bShift, bCtrl))
                return;
            break;

        case VK_END:
            if (OnKeyEnd(newFocus, bShift, bCtrl))
                return;
            break;

        case VK_PRIOR: // Page Up
            if (OnKeyPageUp(newFocus, bShift, bCtrl))
                return;
            break;

        case VK_NEXT: // Page Down
            if (OnKeyPageDown(newFocus, bShift, bCtrl))
                return;
            break;

        case VK_TAB:
            if (OnKeyTab(newFocus, bShift, bCtrl))
                return;
            break;

        case VK_RETURN:
            if (OnKeyReturn(newFocus, bShift, bCtrl))
                return;
            break;

        case VK_ESCAPE:
            if (OnKeyEscape(newFocus, bShift, bCtrl))
                return;
            break;

        case VK_DELETE:
            if (OnKeyDelete(newFocus, bShift, bCtrl))
                return;
            break;

        case VK_F2:
            if (OnKeyF2(newFocus, bShift, bCtrl))
                return;
            break;

        default:
            // Handle clipboard shortcuts
            if (bCtrl)
            {
                switch (nChar)
                {
                    case 'C':
                    case 'c':
                        if (Copy())
                            return;
                        break;

                    case 'X':
                    case 'x':
                        if (Cut())
                            return;
                        break;

                    case 'V':
                    case 'v':
                        if (Paste())
                            return;
                        break;

                    case 'A':
                    case 'a':
                        SelectAll();
                        return;
                        break;
                }
            }

            // Handle character input for editing
            if (nChar >= 32 && nChar <= 126) // Printable characters
            {
                if (OnKeyChar(nChar, newFocus, bShift, bCtrl))
                    return;
            }
            break;
    }

    //__baseCls::OnKeyDown(nChar, nRepCnt, nFlags);
}

// Cell event handlers
void SGridCtrl::OnFixedCellClick(SCellID cell)
{
    if (!cell.IsValid())
        return;
    // In list mode, clicking on column header sorts
    if (cell.row < m_nFixedRows && cell.col >= m_nFixedCols)
    {
        OnHeaderClick(cell.col);
    }
    // Clicking on row header selects entire row
    else if (cell.col < m_nFixedCols && cell.row >= m_nFixedRows)
    {
        OnRowHeaderClick(cell.row);
    }
    if (m_bListMode)
    {

    }
}

void SGridCtrl::OnCellClick(SCellID cell, UINT nFlags,CPoint pt)
{
    if (!cell.IsValid())
        return;

    // Set focus to clicked cell
    SetFocusCell2(cell);

    // Handle selection
    if (nFlags & MK_CONTROL)
    {
        // Toggle selection of this cell
        ToggleCellSelection(cell);
    }
    else if (nFlags & MK_SHIFT)
    {
        // Extend selection from focus cell to this cell
        ExtendSelection(cell);
    }
    else
    {
        // Select only this cell
        if (m_bListMode && m_bSingleRowSelection)
        {
            // Select entire row
            SetSelectedRange(cell.row, 0, cell.row, m_nCols - 1, TRUE);
        }
        else if (m_bSingleColSelection)
        {
            // Select entire column
            SetSelectedRange(0, cell.col, m_nRows - 1, cell.col, TRUE);
        }
        else
        {
            // Select single cell
            SetSelectedRange(cell.row, cell.col, cell.row, cell.col, TRUE);
        }
    }
}

void SGridCtrl::OnCellDblClick(SCellID cell, UINT nFlags,CPoint pt)
{
    if (!cell.IsValid())
        return;

    // Start editing if cell is editable
    const SMergedCell* pMergedCell = GetMergedCell(cell);
    if(pMergedCell){
        cell = pMergedCell->masterCell;
    }
    IGridCell *pCell = GetCell(cell.row, cell.col);
    if(pCell){
        CRect rcCell = GetCellRect(cell.row, cell.col);
        CPoint pt2 = pt-rcCell.CenterPoint();
        pCell->OnDblClick(pt2);
    }
}

void SGridCtrl::OnCellRClick(SCellID cell, UINT nFlags,CPoint pt)
{
    if (!cell.IsValid())
        return;

    // Set focus to right-clicked cell
    SetFocusCell2(cell);
    CRect rcCell = GetCellRect(cell.row, cell.col);
    IGridCell *pCell = GetCell(cell.row, cell.col);
    if(pCell){
        CPoint pt2 = pt-rcCell.TopLeft();
        pCell->OnRClick(pt2);
    }
    // TODO: Show context menu
}

void SGridCtrl::OnCellMouseEnter(SCellID cell)
{
    if (!cell.IsValid())
        return;

    IGridCell* pCell = GetCell(cell.row, cell.col);
    if (pCell)
    {
        pCell->OnMouseEnter();
    }
}

void SGridCtrl::OnCellMouseLeave(SCellID cell)
{
    if (!cell.IsValid())
        return;

    IGridCell* pCell = GetCell(cell.row, cell.col);
    if (pCell)
    {
        pCell->OnMouseLeave();
    }
}

void SGridCtrl::OnCellMouseOver(SCellID cell)
{
    if (!cell.IsValid())
        return;

    IGridCell* pCell = GetCell(cell.row, cell.col);
    if (pCell)
    {
        pCell->OnMouseOver();
    }
}

// Refresh and redraw implementation
BOOL SGridCtrl::RedrawCell(int nRow, int nCol)
{
    if (!IsValid(nRow, nCol))
        return FALSE;

    CRect rcCell = GetCellRectEx(nRow, nCol);
    InvalidateRect(rcCell);
    return TRUE;
}

BOOL SGridCtrl::RedrawCell2(SCELLID cell)
{
    return RedrawCell(cell.row, cell.col);
}

BOOL SGridCtrl::RedrawRow(int row)
{
    if (row < 0 || row >= m_nRows)
        return FALSE;

    for (int col = 0; col < m_nCols; col++)
    {
        RedrawCell(row, col);
    }
    return TRUE;
}

BOOL SGridCtrl::RedrawColumn(int col)
{
    if (col < 0 || col >= m_nCols)
        return FALSE;

    for (int row = 0; row < m_nRows; row++)
    {
        RedrawCell(row, col);
    }
    return TRUE;
}

BOOL SGridCtrl::Refresh()
{
    Invalidate();
    return TRUE;
}

void SGridCtrl::OnHeaderClick(int nCol)
{
    if (m_bListMode)
    {
        if (!m_bSortOnClick || nCol < m_nFixedCols)
            return;

        // Determine sort order
        ESortOrder newOrder = SORT_ASCENDING;
        if (m_nSortColumn == nCol)
        {
            // Same column - toggle sort order
            if (m_nSortOrder == SORT_ASCENDING)
                newOrder = SORT_DESCENDING;
            else if (m_nSortOrder == SORT_DESCENDING)
                newOrder = SORT_NONE;
            else
                newOrder = SORT_ASCENDING;
        }

        // Perform sort
        if (newOrder == SORT_NONE)
        {
            m_nSortColumn = -1;
            m_nSortOrder = SORT_NONE;
            // TODO: Restore original order
        }
        else
        {
            SortItems(nCol, newOrder);
        }

        // Redraw header to show sort indicator
        RedrawRow(0);
    }
    else
    {
        //select entire column
        SetSelectedRange(m_nFixedRows, nCol, m_nRows-1, nCol, TRUE);
        SetFocusCell(m_nFixedRows, nCol);
    }
}

void SGridCtrl::OnRowHeaderClick(int nRow)
{
    // Select entire row
    SetSelectedRange(nRow, m_nFixedCols, nRow, m_nCols - 1, TRUE);
    SetFocusCell(nRow, m_nFixedCols);
}

void SGridCtrl::ToggleCellSelection(SCellID cell)
{
    if (!cell.IsValid())
        return;

    IGridCell* pCell = GetCell(cell.row, cell.col);
    if (pCell)
    {
        SCellRange range(cell.row, cell.col,cell.row, cell.col);
        DWORD state = pCell->GetState();
        if (state & SGVIS_SELECTED)
        {
            state &= ~SGVIS_SELECTED;
            m_SelectedCellRange.RemoveRange(range);
        }    
        else{
            state |= SGVIS_SELECTED;
            m_SelectedCellRange.AddRange(range);
        }
        pCell->SetState(state);
        RedrawCell(cell.row, cell.col);
    }
}

void SGridCtrl::ExtendSelection(SCellID cell)
{
    if (!cell.IsValid() || !m_cellFocus.IsValid())
        return;

    int minRow = smin(m_cellFocus.row, cell.row);
    int maxRow = smax(m_cellFocus.row, cell.row);
    int minCol = smin(m_cellFocus.col, cell.col);
    int maxCol = smax(m_cellFocus.col, cell.col);

    SetSelectedRange(minRow, minCol, maxRow, maxCol, TRUE);
}

void SGridCtrl::StartEdit(SCELLID _cell)
{
    SCellID cell=_cell;
    if (!cell.IsValid() || !IsCellEditable(cell) || m_bInEdit)
        return;
    // Start inplace editing with appropriate type
    StartInplaceEdit(cell.row, cell.col);
}

// Multi-selection implementation
void SGridCtrl::SelectCells(SCellID currentCell, BOOL bForceRedraw)
{
    if (!IsValid(currentCell.row, currentCell.col))
        return;

    int row = currentCell.row;
    int col = currentCell.col;
    if (row < m_nFixedRows || col < m_nFixedCols)
        return;

    // Calculate selection range
    int minRow = smin(m_SelectionStartCell.row, row);
    int maxRow = smax(m_SelectionStartCell.row, row);
    int minCol = smin(m_SelectionStartCell.col, col);
    int maxCol = smax(m_SelectionStartCell.col, col);

    SetSelectedRange(minRow, minCol, maxRow, maxCol, bForceRedraw);
}

void SGridCtrl::SelectRows(SCellID currentCell, BOOL bForceRedraw)
{
    if (!IsValid(currentCell.row, currentCell.col))
        return;

    if (currentCell.row < m_nFixedRows)
        return;

    int minRow, maxRow;
    if (m_bSingleRowSelection)
    {
        minRow = maxRow = currentCell.row;
    }
    else
    {
        minRow = smin(m_SelectionStartCell.row, currentCell.row);
        maxRow = smax(m_SelectionStartCell.row, currentCell.row);
    }

    int leftCol = m_nFixedCols;
    int rightCol = m_nCols - 1;

    SetSelectedRange(minRow, leftCol, maxRow, rightCol, bForceRedraw);
}

void SGridCtrl::SelectColumns(SCellID currentCell, BOOL bForceRedraw)
{
    if (!IsValid(currentCell.row, currentCell.col))
        return;

    if (currentCell.col < m_nFixedCols)
        return;

    int minCol, maxCol;
    if (m_bSingleColSelection)
    {
        minCol = maxCol = currentCell.col;
    }
    else
    {
        minCol = smin(m_SelectionStartCell.col, currentCell.col);
        maxCol = smax(m_SelectionStartCell.col, currentCell.col);
    }

    int topRow = m_nFixedRows;
    int bottomRow = m_nRows - 1;

    SetSelectedRange(topRow, minCol, bottomRow, maxCol, bForceRedraw);
}

void SGridCtrl::OnSelecting(const SCellID& currentCell)
{
    switch (m_MouseMode)
    {
    case MOUSE_SELECT_ALL:
        SelectAll();
        break;
    case MOUSE_SELECT_COL:
        SelectColumns(currentCell, TRUE);
        break;
    case MOUSE_SELECT_ROW:
        SelectRows(currentCell, TRUE);
        break;
    case MOUSE_SELECT_CELLS:
        SelectCells(currentCell, TRUE);
        break;
    }
}

// Drag and drop implementation
void SGridCtrl::OnBeginDrag()
{
    if (!m_bAllowDragAndDrop)
        return;
    // Get selected text for drag operation
    SStringT strText = FormatCellsAsText(m_SelectedCellRange);
    if (strText.IsEmpty())
        return;

    // Set drag mode
    m_MouseMode = MOUSE_DRAGGING;
    m_bLMouseButtonDown = FALSE;

    // Store drag data
    m_strDragData = strText;

    // Copy to clipboard as fallback
    SetClipboardText(strText);
}

void SGridCtrl::OnDragEnter()
{
    // TODO: Implement drag enter handling
}

void SGridCtrl::OnDragOver(CPoint point)
{
    if (!m_bAllowDragAndDrop)
        return;

    // Find which cell we are over
    SCellID cell = GetCellFromPt(point, TRUE);
    if (!IsValid(cell.row, cell.col))
        return;

    // Highlight the drop target cell
    if (m_LastDragOverCell != cell)
    {
        // Unhighlight previous cell
        if (m_LastDragOverCell.IsValid())
        {
            IGridCell* pPrevCell = GetCell(m_LastDragOverCell.row, m_LastDragOverCell.col);
            if (pPrevCell)
            {
                DWORD state = pPrevCell->GetState();
                state &= ~SGVIS_DROPHILITED;
                pPrevCell->SetState(state);
                RedrawCell(m_LastDragOverCell.row, m_LastDragOverCell.col);
            }
        }

        // Highlight new cell
        IGridCell* pCell = GetCell(cell.row, cell.col);
        if (pCell && IsCellEditable(cell))
        {
            DWORD state = pCell->GetState();
            state |= SGVIS_DROPHILITED;
            pCell->SetState(state);
            RedrawCell(cell.row, cell.col);
        }

        m_LastDragOverCell = cell;
    }
}

void SGridCtrl::OnDragLeave()
{
    // Unhighlight drop target cell
    if (m_LastDragOverCell.IsValid())
    {
        IGridCell* pCell = GetCell(m_LastDragOverCell.row, m_LastDragOverCell.col);
        if (pCell)
        {
            DWORD state = pCell->GetState();
            state &= ~SGVIS_DROPHILITED;
            pCell->SetState(state);
            RedrawCell(m_LastDragOverCell.row, m_LastDragOverCell.col);
        }
        m_LastDragOverCell = SCellID(-1, -1);
    }
}

BOOL SGridCtrl::OnDrop(CPoint point, LPCTSTR lpszText)
{
    m_MouseMode = MOUSE_NOTHING;

    if (!m_bAllowDragAndDrop)
        return FALSE;

    SCellID cell = GetCellFromPt(point, TRUE);
    if (!IsValid(cell.row, cell.col))
        return FALSE;

    OnDragLeave(); // Clean up highlighting

    // Use drag data if available, otherwise use provided text
    SStringT strData = m_strDragData.IsEmpty() ?
                      SStringT(lpszText ? lpszText : _T("")) :
                      m_strDragData;

    if (strData.IsEmpty())
        return FALSE;

    // Check if this is a multi-cell paste operation
    if (strData.Find(_T("\t")) >= 0 || strData.Find(_T("\r")) >= 0 || strData.Find(_T("\n")) >= 0)
    {
        return PasteMultiCellData(cell, strData);
    }
    else
    {
        // Single cell paste
        if (!IsCellEditable(cell))
            return FALSE;
        return PasteTextToCell(cell, strData);
    }
}

BOOL SGridCtrl::PasteTextToCell(SCellID cell, LPCTSTR lpszText)
{
    if (!IsValid(cell.row, cell.col) || !IsCellEditable(cell) || !lpszText)
        return FALSE;

    // For simple implementation, just set the text to the target cell
    SetItemText(cell.row, cell.col, lpszText);

    // TODO: Parse multi-line, tab-delimited text for multi-cell paste
    // This would involve splitting the text by \r\n and \t and pasting
    // to multiple cells starting from the target cell

    return TRUE;
}

BOOL SGridCtrl::PasteMultiCellData(SCellID startCell, LPCTSTR lpszData)
{
    if (!IsValid(startCell.row, startCell.col) || !lpszData)
        return FALSE;

    SStringT strData = lpszData;

    // Split data into rows
    SArray<SStringT> rows;
    SplitString(strData, _T("\r\n"), rows);
    if (rows.GetCount() == 0)
        SplitString(strData, _T("\n"), rows);

    int nStartRow = startCell.row;
    int nStartCol = startCell.col;
    int nCols = 0;
    for (int i = 0; i < rows.GetCount(); i++)
    {
        int nRow = nStartRow + i;
        if (nRow >= GetRowCount())
            break;

        // Split row into columns
        SArray<SStringT> cols;
        SplitString(rows[i], _T("\t"), cols);

        for (int j = 0; j < cols.GetCount(); j++)
        {
            int nCol = nStartCol + j;
            if (nCol >= GetColumnCount())
                break;

            if (!IsCellFixed(nRow, nCol) && IsCellEditable(SCellID(nRow, nCol)))
            {
                SetItemText(nRow, nCol, cols[j]);
            }
        }
        nCols = smax(nCols, cols.GetCount());
    }

    // Refresh the affected area
    SCellRange affectedRange(nStartRow, nStartCol,
                           smin(nStartRow + rows.GetCount() - 1, GetRowCount() - 1),
                           smin(nStartCol + nCols, GetColumnCount() - 1));
    CRect rc1 = GetCellRect(affectedRange.minRow, affectedRange.minCol);
    CRect rc2 = GetCellRect(affectedRange.maxRow, affectedRange.maxCol);
    CRect rc3 = rc1 | rc2;
    InvalidateRect(rc3);
    return TRUE;
}

void SGridCtrl::SplitString(const SStringT& str, LPCTSTR delimiter, SArray<SStringT>& result)
{
    result.RemoveAll();

    if (str.IsEmpty())
        return;

    SStringT temp = str;
    int pos = 0;
    SStringT token;

    while ((pos = temp.Find(delimiter)) >= 0)
    {
        token = temp.Left(pos);
        result.Add(token);
        temp = temp.Mid(pos + _tcslen(delimiter));
    }

    // Add the last part
    if (!temp.IsEmpty())
        result.Add(temp);
}

BOOL SGridCtrl::MouseOverResizeArea(const CPoint& point, BOOL bColumn)
{
    SCellID idCurrentCell = GetCellFromPt(point, TRUE);
    if (!idCurrentCell.IsValid())
        return FALSE;

    CRect cellRect = GetCellRect(idCurrentCell.row, idCurrentCell.col);
    if (cellRect.IsRectEmpty())
        return FALSE;

    if (bColumn)
    {
        if (idCurrentCell.row < m_nFixedRows)
        {
            // Check if we're near the left or right edge of the cell
            if ((point.x - cellRect.left < m_nResizeCaptureRange && idCurrentCell.col != 0) || (cellRect.right - point.x < m_nResizeCaptureRange))
            {
                return TRUE;
            }
        }
    }
    else
    {
        if (idCurrentCell.col < m_nFixedCols)
        {
            // Check if we're near the top or bottom edge of the cell
            if ((point.y - cellRect.top < m_nResizeCaptureRange && idCurrentCell.row != 0) || (cellRect.bottom - point.y < m_nResizeCaptureRange))
            {
                return TRUE;
            }
        }
    }
    return FALSE;
}

void SGridCtrl::StartRowResize(int nRow, CPoint point)
{
    m_MouseMode = MOUSE_SIZING_ROW;
    m_LeftClickDownCell.row = nRow;
    m_LeftClickDownCell.col = 0;
    m_LeftClickDownPoint = point;

    // Set cursor
    SetCursor(LoadCursor(NULL, IDC_SIZENS));

    // Capture mouse
    SetCapture();
}

void SGridCtrl::StartColumnResize(int nCol, CPoint point)
{
    m_MouseMode = MOUSE_SIZING_COL;
    m_LeftClickDownCell.row = 0;
    m_LeftClickDownCell.col = nCol;
    m_LeftClickDownPoint = point;

    // Set cursor
    SetCursor(LoadCursor(NULL, IDC_SIZEWE));

    // Capture mouse
    SetCapture();
}

void SGridCtrl::DoRowResize(CPoint point)
{
    if (m_LeftClickDownPoint == point)
        return;

    CRect cellRect = GetCellRect(m_LeftClickDownCell.row, 0);
    if (cellRect.IsRectEmpty())
        return;

    int nNewHeight = smax(point.y - cellRect.top, 1);
    SetRowHeight(m_LeftClickDownCell.row, nNewHeight);

    // Update layout and redraw
    Invalidate();
}

void SGridCtrl::DoColumnResize(CPoint point)
{
    if (m_LeftClickDownPoint == point)
        return;

    CRect cellRect = GetCellRect(0, m_LeftClickDownCell.col);
    if (cellRect.IsRectEmpty())
        return;

    int nNewWidth = smax(point.x - cellRect.left, 1);
    SetColumnWidth(m_LeftClickDownCell.col, nNewWidth);

    // Update layout and redraw
    Invalidate();
}

void SGridCtrl::EndResize()
{
    m_MouseMode = MOUSE_NOTHING;
    ReleaseCapture();
    SetCursor(LoadCursor(NULL, IDC_ARROW));
}

// Cell merging implementation
BOOL SGridCtrl::MergeCells(int nStartRow, int nStartCol, int nEndRow, int nEndCol)
{
    if (!IsValid(nStartRow, nStartCol) || !IsValid(nEndRow, nEndCol))
        return FALSE;
    if (nEndRow - nStartRow <=0 &&  nEndCol - nStartCol <=0)
        return FALSE;
    if(nStartRow < m_nFixedRows || nStartCol < m_nFixedCols)
        return FALSE;
    // Check if any cells in the range are already merged
    for (int row = nStartRow; row <= nEndRow; row++)
    {
        for (int col = nStartCol; col <= nEndCol; col++)
        {
            if (IsCellMerged(row, col))
            {
                // Unmerge existing merged cells first
                UnmergeCells(row, col);
            }
        }
    }

    // Create new merged cell
    SCellRange range(nStartRow, nStartCol, nEndRow, nEndCol);
    SMergedCell mergedCell(range);
    m_MergedCells.Add(mergedCell);
    // Invalidate the merged area
    CRect rcRange = GetRangeRect(range);
    InvalidateRect(rcRange);

    return TRUE;
}

BOOL SGridCtrl::MergeCells(const SCELLRANGE* range)
{
    SASSERT(range);
    return MergeCells(range->minRow, range->minCol, range->maxRow, range->maxCol);
}

BOOL SGridCtrl::UnmergeCells(int nRow, int nCol)
{
    return UnmergeCells(SCellID(nRow, nCol));
}

BOOL SGridCtrl::UnmergeCells(const SCellID& cell)
{
    for (int i = 0; i < m_MergedCells.GetCount(); i++)
    {
        if (m_MergedCells[i].Contains(cell))
        {
            CRect rcRange = GetRangeRect(m_MergedCells[i].range);
            m_MergedCells.RemoveAt(i);
            InvalidateRect(rcRange);
            return TRUE;
        }
    }
    return FALSE;
}

BOOL SGridCtrl::IsCellMerged(int nRow, int nCol) const
{
    return IsCellMerged(SCellID(nRow, nCol));
}

BOOL SGridCtrl::IsCellMerged(const SCellID& cell) const
{
    for (int i = 0; i < m_MergedCells.GetCount(); i++)
    {
        if (m_MergedCells[i].Contains(cell))
            return TRUE;
    }
    return FALSE;
}

const SMergedCell* SGridCtrl::GetMergedCell(int nRow, int nCol) const
{
    return GetMergedCell(SCellID(nRow, nCol));
}

const SMergedCell* SGridCtrl::GetMergedCell(const SCellID& cell) const
{
    for (int i = 0; i < m_MergedCells.GetCount(); i++)
    {
        if (m_MergedCells[i].Contains(cell))
            return &m_MergedCells[i];
    }
    return NULL;
}

SCellID SGridCtrl::GetMergedCellMaster(int nRow, int nCol) const
{
    return GetMergedCellMaster(SCellID(nRow, nCol));
}

SCellID SGridCtrl::GetMergedCellMaster(const SCellID& cell) const
{
    for (int i = 0; i < m_MergedCells.GetCount(); i++)
    {
        if (m_MergedCells[i].Contains(cell))
            return m_MergedCells[i].masterCell;
    }
    return cell; // Return original cell if not merged
}

void SGridCtrl::ClearAllMergedCells()
{
    m_MergedCells.RemoveAll();
    Invalidate();
}

// Advanced cell types implementation
BOOL SGridCtrl::SetCellType(int nRow, int nCol, ECellType cellType)
{
    return SetCellType(SCellID(nRow, nCol), cellType);
}

BOOL SGridCtrl::RegisterCellFactory(IGridCellFactory *pFactory)
{
    for(int i = 0; i < m_cellFactories.GetCount(); i++){
        if(m_cellFactories[i]->GetCellType() == pFactory->GetCellType()){
            m_cellFactories.SetAt(i, pFactory->Clone());
            return FALSE;
        }
    }
    m_cellFactories.Add(pFactory->Clone());
    return TRUE;
}

BOOL SGridCtrl::UnregisterCellFactory(IGridCellFactory *pFactory)
{
    for(int i = 0; i < m_cellFactories.GetCount(); i++){
        if(m_cellFactories[i] == pFactory){
            m_cellFactories.RemoveAt(i);
            return TRUE;
        }
    }
    return FALSE;
}

IGridCell * SGridCtrl::CreateCellByType(ECellType cellType){
    for (int i = 0; i < m_cellFactories.GetCount(); i++)
    {
        if (m_cellFactories[i]->GetCellType() == cellType)
        {
            return m_cellFactories[i]->CreateCell();
        } 
    }
    return NULL;
}

BOOL SGridCtrl::SetCellType(const SCellID& cell, ECellType cellType)
{
    if (!IsValid(cell.row, cell.col))
        return FALSE;

    IGridCell* pOldCell = GetCell(cell.row, cell.col);
    if(pOldCell && pOldCell->GetType() == cellType)
        return TRUE;
    IGridCell* pNewCell = CreateCellByType(cellType);
    if (!pNewCell)
    {
        return FALSE;
    }
    // Copy properties from old cell if it exists
    if (pOldCell)
    {
        pNewCell->SetText(pOldCell->GetText());
        pNewCell->SetTextClr(pOldCell->GetTextClr());
        pNewCell->SetBackClr(pOldCell->GetBackClr());
        pNewCell->SetFormat(pOldCell->GetFormat());
        pNewCell->SetState(pOldCell->GetState());
        pNewCell->SetData(pOldCell->GetData());
        pNewCell->SetImage(pOldCell->GetImage());
        pNewCell->SetMargin(pOldCell->GetMargin());
    }else{
        // Copy properties from default fixed cell
        if(IsCellFixed2(cell)){
            if (m_pDefaultFixedCell)
            {
                pNewCell->SetBackClr(m_pDefaultFixedCell->GetBackClr());
                pNewCell->SetTextClr(m_pDefaultFixedCell->GetTextClr());
                pNewCell->SetFormat(m_pDefaultFixedCell->GetFormat());
            }
        }
        else
        {
            // Copy properties from default cell
            if (m_pDefaultCell)
            {
                pNewCell->SetBackClr(m_pDefaultCell->GetBackClr());
                pNewCell->SetTextClr(m_pDefaultCell->GetTextClr());
                pNewCell->SetFormat(m_pDefaultCell->GetFormat());
            }
        }
    }

    // Set the new cell
    if (cell.row < m_RowData.GetCount())
    {
        SArray<IGridCell*>* pRow = m_RowData[cell.row];
        if (pRow && cell.col < pRow->GetCount())
        {
            // Delete old cell
            if ((*pRow)[cell.col])
             (*pRow)[cell.col]->Release();

            // Set new cell
            (*pRow)[cell.col] = pNewCell;
            pNewCell->Init(this, cell.row, cell.col);
        }
    }

    return TRUE;
}

ECellType SGridCtrl::GetCellType(int nRow, int nCol) const
{
    return GetCellType(SCellID(nRow, nCol));
}

ECellType SGridCtrl::GetCellType(const SCellID& cell) const
{
    IGridCell* pCell = GetCell(cell.row, cell.col);
    if (!pCell)
        return CELL_TYPE_TEXT;
    return (ECellType)pCell->GetType();
}

// Keyboard navigation implementation
BOOL SGridCtrl::OnKeyLeft(SCellID& newFocus, BOOL bShift, BOOL bCtrl)
{
    if (bCtrl)
    {
        // Ctrl+Left: Move to first non-fixed column
        newFocus.col = m_nFixedCols;
    }
    else
    {
        SCellID cell = newFocus;
        const SMergedCell* mergedCell = GetMergedCell(cell);
        if(mergedCell){
            cell.col = mergedCell->range.minCol-1;
        }else{
            cell.col--;
        }
        if (cell.col < m_nFixedCols)
        {
            // Can't move further right
            return FALSE;
        }
        mergedCell = GetMergedCell(cell);
        if(mergedCell){
            cell = mergedCell->masterCell;
        }
        newFocus = cell;
    }

    return MoveFocusAndSelection(newFocus, bShift);
}

BOOL SGridCtrl::OnKeyRight(SCellID& newFocus, BOOL bShift, BOOL bCtrl)
{
    if (bCtrl)
    {
        // Ctrl+Right: Move to last column
        newFocus.col = m_nCols - 1;
    }
    else
    {
        SCellID cell = newFocus;
        const SMergedCell* mergedCell = GetMergedCell(cell);
        if(mergedCell){
            cell.col = mergedCell->range.maxCol+1;
        }else{
            cell.col++;
        }
        if (cell.col >= m_nCols)
        {
            // Can't move further right
            return FALSE;
        }
        mergedCell = GetMergedCell(cell);
        if(mergedCell){
            cell = mergedCell->masterCell;
        }
        newFocus = cell;
    }

    return MoveFocusAndSelection(newFocus, bShift);
}

BOOL SGridCtrl::OnKeyUp(SCellID& newFocus, BOOL bShift, BOOL bCtrl)
{
    if (bCtrl)
    {
        // Ctrl+Up: Move to first non-fixed row
        newFocus.row = m_nFixedRows;
    }
    else
    {
        SCellID cell = newFocus;
        const SMergedCell* mergedCell = GetMergedCell(cell);
        if(mergedCell){
            cell.row = mergedCell->range.minRow-1;
        }else{
            cell.row--;
        }
        if (cell.row < m_nFixedRows)
        {
            // Can't move further up
            return FALSE;
        }
        mergedCell = GetMergedCell(cell);
        if(mergedCell){
            cell = mergedCell->masterCell;
        }
        newFocus = cell;
    }

    return MoveFocusAndSelection(newFocus, bShift);
}

BOOL SGridCtrl::OnKeyDownNav(SCellID& newFocus, BOOL bShift, BOOL bCtrl)
{
    if (bCtrl)
    {
        // Ctrl+Down: Move to last row
        newFocus.row = m_nRows - 1;
    }
    else
    {
        SCellID cell = newFocus;
        const SMergedCell* mergedCell = GetMergedCell(cell);
        if(mergedCell){
            cell.row = mergedCell->range.maxRow+1;
        }else{
            cell.row++;
        }
        if (cell.row >= m_nRows)
        {
            // Can't move further down
            return FALSE;
        }
        mergedCell = GetMergedCell(cell);
        if(mergedCell){
            cell = mergedCell->masterCell;
        }
        newFocus = cell;
    }

    return MoveFocusAndSelection(newFocus, bShift);
}

BOOL SGridCtrl::OnKeyHome(SCellID& newFocus, BOOL bShift, BOOL bCtrl)
{
    if (bCtrl)
    {
        // Ctrl+Home: Move to top-left data cell
        newFocus.row = m_nFixedRows;
        newFocus.col = m_nFixedCols;
    }
    else
    {
        // Home: Move to first column of current row
        newFocus.col = m_nFixedCols;
    }

    return MoveFocusAndSelection(newFocus, bShift);
}

BOOL SGridCtrl::OnKeyEnd(SCellID& newFocus, BOOL bShift, BOOL bCtrl)
{
    if (bCtrl)
    {
        // Ctrl+End: Move to bottom-right cell
        newFocus.row = m_nRows - 1;
        newFocus.col = m_nCols - 1;
    }
    else
    {
        // End: Move to last column of current row
        newFocus.col = m_nCols - 1;
    }

    return MoveFocusAndSelection(newFocus, bShift);
}

BOOL SGridCtrl::OnKeyPageUp(SCellID& newFocus, BOOL bShift, BOOL bCtrl)
{
    CRect rcClient = GetClientRect();
    int nVisibleRows = rcClient.Height() / m_nDefCellHeight;

    newFocus.row = smax(m_nFixedRows, newFocus.row - nVisibleRows);

    return MoveFocusAndSelection(newFocus, bShift);
}

BOOL SGridCtrl::OnKeyPageDown(SCellID& newFocus, BOOL bShift, BOOL bCtrl)
{
    CRect rcClient = GetClientRect();
    int nVisibleRows = rcClient.Height() / m_nDefCellHeight;

    newFocus.row = smin(m_nRows - 1, newFocus.row + nVisibleRows);

    return MoveFocusAndSelection(newFocus, bShift);
}

BOOL SGridCtrl::OnKeyTab(SCellID& newFocus, BOOL bShift, BOOL bCtrl)
{
    if (bShift)
    {
        // Shift+Tab: Move to previous cell
        if (newFocus.col > m_nFixedCols)
        {
            newFocus.col--;
        }
        else if (newFocus.row > m_nFixedRows)
        {
            newFocus.row--;
            newFocus.col = m_nCols - 1;
        }
        else
        {
            return FALSE; // Can't move further
        }
    }
    else
    {
        // Tab: Move to next cell
        if (newFocus.col < m_nCols - 1)
        {
            newFocus.col++;
        }
        else if (newFocus.row < m_nRows - 1)
        {
            newFocus.row++;
            newFocus.col = m_nFixedCols;
        }
        else
        {
            return FALSE; // Can't move further
        }
    }

    return MoveFocusAndSelection(newFocus, FALSE); // Tab doesn't extend selection
}

BOOL SGridCtrl::OnKeyReturn(SCellID& newFocus, BOOL bShift, BOOL bCtrl)
{
    if (IsCellEditable(m_cellFocus))
    {
        // Start editing current cell
        StartEdit(m_cellFocus);
        return TRUE;
    }
    else
    {
        // Move down one row
        if (m_cellFocus.row < m_nRows - 1)
        {
            newFocus.row = m_cellFocus.row + 1;
            return MoveFocusAndSelection(newFocus, FALSE);
        }
    }

    return FALSE;
}

BOOL SGridCtrl::OnKeyEscape(SCellID& newFocus, BOOL bShift, BOOL bCtrl)
{
    // TODO: Cancel editing if in edit mode
    // For now, just clear selection
    SetSelectedRange(-1, -1, -1, -1, TRUE);
    return TRUE;
}

BOOL SGridCtrl::OnKeyDelete(SCellID& newFocus, BOOL bShift, BOOL bCtrl)
{
    // Delete content of selected cells
    SPOSITION pos = m_SelectedCellRange.GetRanges().GetHeadPosition();
    bool any = false;
    while (pos)
    {
        SCellRange r = m_SelectedCellRange.GetRanges().GetNext(pos);
        for (int row = r.minRow; row <= r.maxRow; row++)
        {
            for (int col = r.minCol; col <= r.maxCol; col++)
            {
                if (!IsCellFixed(row, col) && IsCellEditable(SCellID(row, col)))
                {
                    SetItemText(row, col, _T(""));
                    any = true;
                }
            }
        }
    }
    if (any)
    {
        Invalidate();
        return TRUE;
    }

    return FALSE;
}

BOOL SGridCtrl::OnKeyF2(SCellID& newFocus, BOOL bShift, BOOL bCtrl)
{
    // F2: Start editing current cell
    if (IsCellEditable(m_cellFocus))
    {
        StartEdit(m_cellFocus);
        return TRUE;
    }

    return FALSE;
}

BOOL SGridCtrl::OnKeyChar(UINT nChar, SCellID& newFocus, BOOL bShift, BOOL bCtrl)
{
    // If already in edit mode, forward to edit control
    if (m_bInEdit)
    {
        OnEditChar(nChar);
        return TRUE;
    }

    // Start editing with the typed character
    if (IsCellEditable(m_cellFocus))
    {
        StartEdit(m_cellFocus);
        // Send the character to the edit control
        if (m_bInEdit && m_pInplaceWnd)
        {
            // Clear existing text and insert the new character
            m_pInplaceWnd->GetIWindow()->SetWindowText(_T(""));
        }
        return TRUE;
    }

    return FALSE;
}

BOOL SGridCtrl::MoveFocusAndSelection(const SCellID& newFocus, BOOL bExtendSelection)
{
    if (!IsValid2(newFocus) || newFocus == m_cellFocus)
        return FALSE;

    // Set new focus
    SetFocusCell2(newFocus);

    // Handle selection
    if (bExtendSelection)
    {
        // Extend selection from original focus to new focus
        ExtendSelection(newFocus);
    }
    else
    {
        // Select only the new focus cell
        if (m_bListMode && m_bSingleRowSelection)
        {
            // Select entire row
            SetSelectedRange(newFocus.row, m_nFixedCols, newFocus.row, m_nCols - 1, TRUE);
        }
        else if (m_bSingleColSelection)
        {
            // Select entire column
            SetSelectedRange(m_nFixedRows, newFocus.col, m_nRows - 1, newFocus.col, TRUE);
        }
        else
        {
            // Select single cell
            SetSelectedRange(newFocus.row, newFocus.col, newFocus.row, newFocus.col, TRUE);
        }
    }

    return TRUE;
}

// Cell editing implementation
BOOL SGridCtrl::IsInEdit() const
{
    return m_bInEdit;
}

BOOL SGridCtrl::GetEditCell(SCELLID *pCell) const
{
    if (pCell)
        *pCell = m_cellEdit;
    return m_bInEdit;
}

BOOL SGridCtrl::EndEdit(BOOL bSaveChanges)
{
    if (!IsInplaceEditing())
        return FALSE;

    // Use new inplace editing system
    EndInplaceEdit(bSaveChanges);
    return TRUE;
}

void SGridCtrl::OnEditChar(UINT nChar)
{
    // Handle special characters in edit mode
    if (m_bInEdit && m_pInplaceWnd)
    {
        // Forward to edit control
        m_pInplaceWnd->GetIWindow()->SSendMessage(WM_CHAR, nChar, 0,NULL);
    }
}

void SGridCtrl::OnEditKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
    if (!m_bInEdit || !m_pInplaceWnd)
        return;

    switch (nChar)
    {
        case VK_RETURN:
            // Save and end edit
            EndEdit(TRUE);
            break;

        case VK_ESCAPE:
            // Cancel edit
            EndEdit(FALSE);
            break;

        case VK_TAB:
            // Save and move to next cell
            if (EndEdit(TRUE))
            {
                SCellID newFocus = m_cellFocus;
                if (GetKeyState(VK_SHIFT) & 0x8000)
                {
                    OnKeyTab(newFocus, TRUE, FALSE);
                }
                else
                {
                    OnKeyTab(newFocus, FALSE, FALSE);
                }
            }
            break;

        default:
            // Forward to edit control
            m_pInplaceWnd->GetIWindow()->SSendMessage(WM_KEYDOWN, nChar, MAKELPARAM(nRepCnt, nFlags),NULL);
            break;
    }
}

BOOL SGridCtrl::ValidateEdit(SCellID cell, LPCTSTR lpszText)
{
    // Default validation - always accept
    // Override this method to implement custom validation
    return TRUE;
}

// Selection and highlighting helpers
void SGridCtrl::InvalidateSelection()
{
    SPOSITION pos = m_SelectedCellRange.GetRanges().GetHeadPosition();
    while (pos)
    {
        SCellRange r = m_SelectedCellRange.GetRanges().GetNext(pos);
        for (int row = r.minRow; row <= r.maxRow; row++)
        {
            for (int col = r.minCol; col <= r.maxCol; col++)
            {
                RedrawCell(row, col);
            }
        }
    }
}

void SGridCtrl::HighlightCell(int nRow, int nCol, BOOL bHighlight)
{
    IGridCell* pCell = GetCell(nRow, nCol);
    if (pCell)
    {
        DWORD state = pCell->GetState();
        if (bHighlight)
            state |= SGVIS_SELECTED;
        else
            state &= ~SGVIS_SELECTED;
        pCell->SetState(state);
        RedrawCell(nRow, nCol);
    }
}

void SGridCtrl::HighlightRange(const SCellRange& range, BOOL bHighlight)
{
    if (!range.IsValid())
        return;

    for (int row = range.minRow; row <= range.maxRow; row++)
    {
        for (int col = range.minCol; col <= range.maxCol; col++)
        {
            if (IsValid(row, col))
            {
                IGridCell* pCell = GetCell(row, col);
                if (pCell)
                {
                    DWORD state = pCell->GetState();
                    if (bHighlight)
                        state |= SGVIS_SELECTED;
                    else
                        state &= ~SGVIS_SELECTED;
                    pCell->SetState(state);
                }
            }
        }
    }

    // Redraw the range
    CRect rcRange = GetRangeRect(range);
    InvalidateRect(rcRange);
}

CRect SGridCtrl::GetRangeRect(const SCellRange& range) const
{
    if (!range.IsValid())
        return CRect(0, 0, 0, 0);

    CRect rcTopLeft = GetCellRect(range.minRow, range.minCol);
    CRect rcBottomRight = GetCellRect(range.maxRow, range.maxCol);

    return CRect(rcTopLeft.left, rcTopLeft.top,
                rcBottomRight.right, rcBottomRight.bottom);
}

BOOL SGridCtrl::IsRangeVisible(const SCellRange& range) const
{
    if (!range.IsValid())
        return FALSE;

    SCellRange visibleRange;
    GetVisibleCellRange(&visibleRange);

    // Check if ranges intersect
    return !(range.maxRow < visibleRange.minRow ||
             range.minRow > visibleRange.maxRow ||
             range.maxCol < visibleRange.minCol ||
             range.minCol > visibleRange.maxCol);
}

void SGridCtrl::SelectAll()
{
    if (m_nRows > 0 && m_nCols > 0)
    {
        SetSelectedRange(m_nFixedRows, m_nFixedCols,
                        m_nRows - 1, m_nCols - 1, TRUE);
    }
}

void SGridCtrl::SelectNone()
{
    SetSelectedRange(-1, -1, -1, -1, TRUE);
}

void SGridCtrl::SelectRow(int nRow, BOOL bAdd)
{
    if (nRow < m_nFixedRows || nRow >= m_nRows)
        return;

    if (!bAdd)
    {
        // Clear existing selection
        SetSelectedRange(-1, -1, -1, -1, FALSE);
    }

    // Select the row
    SetSelectedRange(nRow, m_nFixedCols, nRow, m_nCols - 1, TRUE);
}

void SGridCtrl::SelectColumn(int nCol, BOOL bAdd)
{
    if (nCol < m_nFixedCols || nCol >= m_nCols)
        return;

    if (!bAdd)
    {
        // Clear existing selection
        SetSelectedRange(-1, -1, -1, -1, FALSE);
    }

    // Select the column
    SetSelectedRange(m_nFixedRows, nCol, m_nRows - 1, nCol, TRUE);
}

void SGridCtrl::ResetSelectedRange()
{
    SetSelectedRange(-1,-1,-1,-1,FALSE);
    SetFocusCell(-1,-1);
}

// Sorting implementation
BOOL SGridCtrl::SortItems(int nCol, int order)
{
    if (nCol < 0 || nCol >= m_nCols || m_nRows <= m_nFixedRows)
        return FALSE;
    ResetSelectedRange();
    // Default to text sorting
    return SortTextItems(nCol, (ESortOrder)order);
}

BOOL SGridCtrl::SortTextItems(int nCol, ESortOrder order)
{
    if (nCol < 0 || nCol >= m_nCols || m_nRows <= m_nFixedRows)
        return FALSE;

    // Create array of row data for sorting
    struct RowData
    {
        int nRow;
        SStringT strText;
        SArray<IGridCell*>* pRowCells;
        int nHeight;
    };

    SArray<RowData> sortData;
    sortData.SetCount(m_nRows - m_nFixedRows);

    // Collect data from non-fixed rows
    for (int i = 0; i < m_nRows - m_nFixedRows; i++)
    {
        int nRow = i + m_nFixedRows;
        sortData[i].nRow = nRow;
        sortData[i].pRowCells = m_RowData[nRow];
        sortData[i].nHeight = GetRowHeight(nRow);

        // Get sort key text
        IGridCell* pCell = GetCell(nRow, nCol);
        if (pCell)
            sortData[i].strText = pCell->GetText();
        else
            sortData[i].strText = _T("");
    }

    // Sort the data
    for (int i = 0; i < sortData.GetCount() - 1; i++)
    {
        for (int j = i + 1; j < sortData.GetCount(); j++)
        {
            int nCompare = sortData[i].strText.CompareNoCase(sortData[j].strText);

            BOOL bSwap = FALSE;
            if (order == SORT_ASCENDING && nCompare > 0)
                bSwap = TRUE;
            else if (order == SORT_DESCENDING && nCompare < 0)
                bSwap = TRUE;

            if (bSwap)
            {
                RowData temp = sortData[i];
                sortData[i] = sortData[j];
                sortData[j] = temp;
            }
        }
    }

    // Rearrange the actual grid data
    for (int i = 0; i < sortData.GetCount(); i++)
    {
        int nRow = i + m_nFixedRows;
        m_RowData[nRow] = sortData[i].pRowCells;
        SetRowHeight(nRow, sortData[i].nHeight);
    }

    // Update sort state
    m_nSortColumn = nCol;
    m_nSortOrder = order;

    // Redraw the grid
    Invalidate();

    return TRUE;
}

BOOL SGridCtrl::SortNumericItems(int nCol, ESortOrder order)
{
    if (nCol < 0 || nCol >= m_nCols || m_nRows <= m_nFixedRows)
        return FALSE;

    // Create array of row data for sorting
    struct RowData
    {
        int nRow;
        double dValue;
        SArray<IGridCell*>* pRowCells;
        int nHeight;
    };

    SArray<RowData> sortData;
    sortData.SetCount(m_nRows - m_nFixedRows);

    // Collect data from non-fixed rows
    for (int i = 0; i < m_nRows - m_nFixedRows; i++)
    {
        int nRow = i + m_nFixedRows;
        sortData[i].nRow = nRow;
        sortData[i].pRowCells = m_RowData[nRow];
        sortData[i].nHeight = GetRowHeight(nRow);

        // Get numeric value
        IGridCell* pCell = GetCell(nRow, nCol);
        if (pCell)
        {
            SStringT strText = pCell->GetText();
            sortData[i].dValue = _tstof(strText);
        }
        else
        {
            sortData[i].dValue = 0.0;
        }
    }

    // Sort the data
    for (int i = 0; i < sortData.GetCount() - 1; i++)
    {
        for (int j = i + 1; j < sortData.GetCount(); j++)
        {
            BOOL bSwap = FALSE;
            if (order == SORT_ASCENDING && sortData[i].dValue > sortData[j].dValue)
                bSwap = TRUE;
            else if (order == SORT_DESCENDING && sortData[i].dValue < sortData[j].dValue)
                bSwap = TRUE;

            if (bSwap)
            {
                RowData temp = sortData[i];
                sortData[i] = sortData[j];
                sortData[j] = temp;
            }
        }
    }

    // Rearrange the actual grid data
    for (int i = 0; i < sortData.GetCount(); i++)
    {
        int nRow = i + m_nFixedRows;
        m_RowData[nRow] = sortData[i].pRowCells;
        SetRowHeight(nRow, sortData[i].nHeight);
    }

    // Update sort state
    m_nSortColumn = nCol;
    m_nSortOrder = order;

    // Redraw the grid
    Invalidate();

    return TRUE;
}

int SGridCtrl::GetSortColumn() const
{
    return m_nSortColumn;
}

int SGridCtrl::GetSortOrder() const
{
    return (int)m_nSortOrder;
}

void SGridCtrl::SetSortOnClick(BOOL bSort)
{
    m_bSortOnClick = bSort;
}

BOOL SGridCtrl::GetSortOnClick() const
{
    return m_bSortOnClick;
}

void SGridCtrl::DrawSortArrow(IRenderTarget* pRT, CRect rect, int order)
{
    if (!pRT || order == SORT_NONE)
        return;

    // Calculate arrow position (right side of cell)
    int nArrowSize = 6;
    CPoint ptCenter(rect.right - nArrowSize - 4, rect.CenterPoint().y);

    // Create arrow points
    CPoint pts[3];
    if (order == SORT_ASCENDING)
    {
        // Up arrow
        pts[0] = CPoint(ptCenter.x, ptCenter.y - nArrowSize/2);      // Top
        pts[1] = CPoint(ptCenter.x - nArrowSize/2, ptCenter.y + nArrowSize/2); // Bottom left
        pts[2] = CPoint(ptCenter.x + nArrowSize/2, ptCenter.y + nArrowSize/2); // Bottom right
    }
    else
    {
        // Down arrow
        pts[0] = CPoint(ptCenter.x, ptCenter.y + nArrowSize/2);      // Bottom
        pts[1] = CPoint(ptCenter.x - nArrowSize/2, ptCenter.y - nArrowSize/2); // Top left
        pts[2] = CPoint(ptCenter.x + nArrowSize/2, ptCenter.y - nArrowSize/2); // Top right
    }

    // Draw the arrow
    SAutoRefPtr<IBrushS> pBrush;
    pRT->CreateSolidColorBrush(RGBA(64, 64, 64,255), &pBrush);
    if (pBrush)
    {
        SAutoRefPtr<IBrushS> pOldBrush;
        pRT->SelectObject(pBrush, (IRenderObj**)&pOldBrush);
        // Draw filled triangle using lines (fallback)
        pRT->DrawLine(pts[0], pts[1]);
        pRT->DrawLine(pts[1], pts[2]);
        pRT->DrawLine(pts[2], pts[0]);
        pRT->SelectObject(pOldBrush, NULL);
    }
}

// Clipboard operations implementation
BOOL SGridCtrl::Copy()
{
    if (!CanCopy())
        return FALSE;
    SStringT strText = FormatCellsAsText(m_SelectedCellRange);
    return SetClipboardText(strText);
}

BOOL SGridCtrl::Cut()
{
    if (!CanCut())
        return FALSE;

    // Copy first
    if (!Copy())
        return FALSE;

    // Then clear the selected cells
    SPOSITION pos = m_SelectedCellRange.GetRanges().GetHeadPosition();
    while(pos)
    {
        SCellRange r = m_SelectedCellRange.GetRanges().GetNext(pos);
        for (int row = r.minRow; row <= r.maxRow; row++)
        {
            for (int col = r.minCol; col <= r.maxCol; col++)
            {
                if (!IsCellFixed(row, col) && IsCellEditable(SCellID(row, col)))
                {
                    SetItemText(row, col, _T(""));
                }
            }
        }
    }
    Invalidate();

    return TRUE;
}

BOOL SGridCtrl::Paste()
{
    if (!CanPaste())
        return FALSE;

    SStringT strText = GetClipboardText();
    if (strText.IsEmpty())
        return FALSE;

    if (!m_cellFocus.IsValid())
        return FALSE;

    return PasteMultiCellData(m_cellFocus, strText);
}

BOOL SGridCtrl::CanCopy() const
{
    // True if any selection ranges exist
    return !m_SelectedCellRange.GetRanges().IsEmpty();
}

BOOL SGridCtrl::CanCut() const
{
    // Check if any selected range contains an editable cell
    SPOSITION pos = m_SelectedCellRange.GetRanges().GetHeadPosition();
    while (pos)
    {
        SCellRange r = m_SelectedCellRange.GetRanges().GetNext(pos);
        for (int row = r.minRow; row <= r.maxRow; row++)
        {
            for (int col = r.minCol; col <= r.maxCol; col++)
            {
                if (!IsCellFixed(row, col) && IsCellEditable(SCellID(row, col)))
                    return TRUE;
            }
        }
    }
    return FALSE;
}

BOOL SGridCtrl::CanPaste() const
{
    // Check if we have a valid paste location
    if (!m_SelectedCellRange.GetRanges().IsEmpty() || m_cellFocus.IsValid())
    {
        // Check if clipboard has text
        SStringT strText = GetClipboardText();
        return !strText.IsEmpty();
    }

    return FALSE;
}

SStringT SGridCtrl::GetClipboardText() const
{
    SStringT strText;

    if (OpenClipboard(GetContainer()->GetHostHwnd()))
    {
        HANDLE hData = GetClipboardData(CF_UNICODETEXT);
        if (hData)
        {
            LPCTSTR lpszText = (LPCTSTR)GlobalLock(hData);
            if (lpszText)
            {
                strText = lpszText;
                GlobalUnlock(hData);
            }
        }
        CloseClipboard();
    }

    return strText;
}

BOOL SGridCtrl::SetClipboardText(LPCTSTR lpszText)
{
    if (!lpszText)
        return FALSE;

    BOOL bResult = FALSE;

    if (OpenClipboard(GetContainer()->GetHostHwnd()))
    {
        EmptyClipboard();

        int nLen = _tcslen(lpszText);
        HGLOBAL hData = GlobalAlloc(GMEM_MOVEABLE, (nLen + 1) * sizeof(TCHAR));
        if (hData)
        {
            LPCTSTR lpszBuffer = (LPCTSTR)GlobalLock(hData);
            if (lpszBuffer)
            {
                _tcscpy((LPTSTR)lpszBuffer, lpszText);
                GlobalUnlock(hData);

                if (SetClipboardData(CF_UNICODETEXT, hData))
                    bResult = TRUE;
                else
                    GlobalFree(hData);
            }
            else
            {
                GlobalFree(hData);
            }
        }

        CloseClipboard();
    }

    return bResult;
}

SStringT SGridCtrl::FormatCellsAsText(const SCellRange& range) const
{
    if (!range.IsValid())
        return _T("");

    SStringT strResult;

    for (int row = range.minRow; row <= range.maxRow; row++)
    {
        if (row > range.minRow)
            strResult += _T("\r\n"); // New line for each row

        for (int col = range.minCol; col <= range.maxCol; col++)
        {
            if (col > range.minCol)
                strResult += _T("\t"); // Tab between columns

            IGridCell* pCell = GetCell(row, col);
            if (pCell)
            {
                SStringT strText = pCell->GetText();
                // Escape tabs and newlines in cell text
                strText.Replace(_T("\t"), _T(" "));
                strText.Replace(_T("\r\n"), _T(" "));
                strText.Replace(_T("\n"), _T(" "));
                strResult += strText;
            }
        }
    }

    return strResult;
}

SStringT SGridCtrl::FormatCellsAsText(const SSelRangeMgr& ranges) const
{
    SStringT strResult;
    SPOSITION pos = ranges.GetRanges().GetHeadPosition();
    bool firstRange = true;
    while (pos)
    {
        SCellRange r = ranges.GetRanges().GetNext(pos);
        if (!r.IsValid()) continue;
        // For multiple ranges, separate ranges by a blank line
        if (!firstRange)
            strResult += _T("\r\n");
        firstRange = false;

        for (int row = r.minRow; row <= r.maxRow; row++)
        {
            if (row > r.minRow)
                strResult += _T("\r\n");

            for (int col = r.minCol; col <= r.maxCol; col++)
            {
                if (col > r.minCol)
                    strResult += _T("\t");

                IGridCell* pCell = GetCell(row, col);
                if (pCell)
                {
                    SStringT strText = pCell->GetText();
                    strText.Replace(_T("\t"), _T(" "));
                    strText.Replace(_T("\r\n"), _T(" "));
                    strText.Replace(_T("\n"), _T(" "));
                    strResult += strText;
                }
            }
        }
    }
    return strResult;
}

BOOL SGridCtrl::ParseTextIntoCells(LPCTSTR lpszText, const SCellRange& range)
{
    if (!lpszText || !range.IsValid())
        return FALSE;

    SStringT strText = lpszText;
    SArray<SStringT> lines;

    // Split into lines
    int nStart = 0;
    int nPos = 0;
    while (nPos < strText.GetLength())
    {
        if (strText[nPos] == _T('\n') || strText[nPos] == _T('\r'))
        {
            if (nPos > nStart)
            {
                lines.Add(strText.Mid(nStart, nPos - nStart));
            }

            // Skip \r\n sequence
            if (strText[nPos] == _T('\r') && nPos + 1 < strText.GetLength() && strText[nPos + 1] == _T('\n'))
                nPos++;

            nStart = nPos + 1;
        }
        nPos++;
    }

    // Add last line if any
    if (nStart < strText.GetLength())
    {
        lines.Add(strText.Mid(nStart));
    }

    // Parse each line and paste into cells
    int nRow = range.minRow;
    for (int i = 0; i < lines.GetCount() && nRow <= range.maxRow && nRow < m_nRows; i++, nRow++)
    {
        SStringT strLine = lines[i];
        SArray<SStringT> cells;

        // Split line into cells by tabs
        nStart = 0;
        nPos = 0;
        while (nPos < strLine.GetLength())
        {
            if (strLine[nPos] == _T('\t'))
            {
                cells.Add(strLine.Mid(nStart, nPos - nStart));
                nStart = nPos + 1;
            }
            nPos++;
        }

        // Add last cell
        if (nStart <= strLine.GetLength())
        {
            cells.Add(strLine.Mid(nStart));
        }

        // Paste cells into row
        int nCol = range.minCol;
        for (int j = 0; j < cells.GetCount() && nCol <= range.maxCol && nCol < m_nCols; j++, nCol++)
        {
            if (!IsCellFixed(nRow, nCol) && IsCellEditable(SCellID(nRow, nCol)))
            {
                SetItemText(nRow, nCol, cells[j]);
            }
        }
    }

    Invalidate();
    return TRUE;
}

//////////////////////////////////////////////////////////////////////////
// Inplace editing implementation

BOOL SGridCtrl::StartInplaceEdit(int nRow, int nCol)
{
    if (!IsValid(nRow, nCol) || !IsCellEditable(SCellID(nRow, nCol)))
        return FALSE;

    // End any existing edit
    if (IsInplaceEditing())
        EndInplaceEdit(TRUE);
    
    SASSERT(!m_pInplaceWnd);
    // Create appropriate inplace editor based on cell type
    IGridCell* gridCell = GetCell(nRow, nCol);

    m_pInplaceWnd = gridCell->CreateInplaceWnd(nRow, nCol);
    if (!m_pInplaceWnd)
        return FALSE;
    IWindow *pInplaceWnd = m_pInplaceWnd->GetIWindow();
    SStringW strXmlStyle;
    EventGridGetInplaceStyle evGetInplaceStyle(this);
    evGetInplaceStyle.pstrXmlStyle = &strXmlStyle;
    evGetInplaceStyle.pCell = gridCell;
    FireEvent(evGetInplaceStyle);
    if(strXmlStyle.IsEmpty()){
        pInplaceWnd->InitFromXml(NULL);
    }else{
        SXmlDoc xmlDoc;
        xmlDoc.LoadBufferInplace((void*)strXmlStyle.c_str(), strXmlStyle.GetLength() * sizeof(wchar_t), xml_parse_default, sizeof(wchar_t) == 2 ? enc_utf16 : enc_utf32);
        pInplaceWnd->InitFromXml(xmlDoc.Root()->FirstChild());
    }

    EventGridInitInplaceWnd evInitInplaceWnd(this);
    evInitInplaceWnd.pCell = gridCell;
    evInitInplaceWnd.pInplaceWnd = m_pInplaceWnd;
    evInitInplaceWnd.bInited = FALSE;
    FireEvent(&evInitInplaceWnd);

    if(!evInitInplaceWnd.bInited){
        // Initialize with current cell text
        SStringT strText = gridCell ? gridCell->GetText() : _T("");
        m_pInplaceWnd->InitData(strText);
    }

    // Position the inplace window
    CRect rcCell = GetCellRectEx(nRow, nCol);
    rcCell.DeflateRect(1, 1);
    pInplaceWnd->Move(rcCell);


    // Show and focus the inplace window
    pInplaceWnd->SetVisible(TRUE, TRUE);
    pInplaceWnd->SetFocus();

    // Update state
    m_bInEdit = TRUE;
    m_cellEdit = SCellID(nRow, nCol);

    return TRUE;
}

void SGridCtrl::EndInplaceEdit(BOOL bSave)
{
    if (!IsInplaceEditing())
        return;

    if (bSave)
    {
        // Update cell data
        if (m_pInplaceWnd)
            m_pInplaceWnd->UpdateData();
    }

    // Destroy inplace window
    if (m_pInplaceWnd)
    {
        SAutoRefPtr<IGridInplaceWnd> pInplaceWnd = m_pInplaceWnd;
        m_pInplaceWnd = NULL;
        pInplaceWnd->GetIWindow()->SetVisible(FALSE, TRUE);
        pInplaceWnd->GetIWindow()->Destroy();
    }

    // Reset state
    m_bInEdit = FALSE;
    m_cellEdit = SCellID(-1, -1);

    // Redraw the cell
    if (m_cellEdit.IsValid())
    {
        RedrawCell(m_cellEdit.row, m_cellEdit.col);
    }
    SetFocus();
}



SNSEND
