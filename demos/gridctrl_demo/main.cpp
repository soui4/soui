#include <souistd.h>
#include <SouiFactory.h>
#include <com-cfg.h>
#include <SGridCtrl.h>
#include <SAppCfg.h>

static const TCHAR * kPath_SysRes = _T("/../../soui-sys-resource");
static const TCHAR *kPath_UiRes = _T("/uires");

using namespace SOUI;

static SStringT getSourceDir()
{
    SStringA file(__FILE__);
    file = file.Left(file.ReverseFind(PATH_SLASH));
    return S_CA2T(file);
}

class CMainDlg : public SHostWnd
{
public:
    CMainDlg() : SHostWnd("LAYOUT:dlg_main") {}

protected:
    BOOL OnInitDialog(HWND hWnd,LPARAM lParam)
    {
        // Get the grid control
        SGridCtrl* pGrid = FindChildByName2<SGridCtrl>(L"grid_test");
        if (pGrid)
        {
            InitializeGrid(pGrid);
        }
        return TRUE;
    }

    void InitializeGrid(SGridCtrl* pGrid)
    {
        // Set grid dimensions - create a larger grid to test scrolling
        const int kNumRows = 20;
        const int kNumCols = 12;
        pGrid->SetRowCount(kNumRows);
        pGrid->SetColumnCount(kNumCols);
        pGrid->SetFixedRowCount(1);
        pGrid->SetFixedColumnCount(1);

        // Set column headers
        pGrid->SetItemText(0, 0, _T(""));
        for (int col = 1; col < kNumCols; col++)
        {
            SStringT strHeader;
            strHeader.Format(_T("Column %d"), col);
            pGrid->SetItemText(0, col, strHeader);
        }

        // Set row headers
        for (int row = 1; row < kNumRows; row++)
        {
            SStringT strRow;
            strRow.Format(_T("Row %d"), row);
            pGrid->SetItemText(row, 0, strRow);
        }

        // Fill with sample data - mix of text and numbers for sorting demo
        for (int row = 1; row < kNumRows; row++)
        {
            for (int col = 1; col < kNumCols; col++)
            {
                SStringT strText;
                if (col == 1)
                {
                    // First column: Names
                    LPCTSTR names[] = {_T("Alice"), _T("Bob"), _T("Charlie"), _T("David"), _T("Eve"),
                                      _T("Frank"), _T("Grace"), _T("Henry"), _T("Ivy"), _T("Jack"),
                                      _T("Kate"), _T("Liam"), _T("Mia"), _T("Noah"), _T("Olivia"),
                                      _T("Paul"), _T("Quinn"), _T("Ruby"), _T("Sam")};
                    strText = names[(row - 1) % 19];
                }
                else if (col == 2)
                {
                    // Second column: Numbers
                    strText.Format(_T("%d"), (row * 7 + col * 3) % 100);
                }
                else if (col == 3)
                {
                    // Third column: Dates
                    strText.Format(_T("2024-%02d-%02d"), (row % 12) + 1, (row % 28) + 1);
                }
                else
                {
                    // Other columns: Mixed data
                    strText.Format(_T("Cell(%d,%d)"), row, col);
                }
                pGrid->SetItemText(row, col, strText);
            }
        }

        // Set some cell properties
        pGrid->SetDefCellHeight(25);
        pGrid->SetDefCellWidth(80);
        pGrid->SetGridLines(SGVL_BOTH);
        pGrid->SetGridLineColor(RGBA(200, 200, 200,255));
        pGrid->SetEditable(TRUE);
        //pGrid->SetListMode(TRUE);
        //pGrid->SetSingleRowSelection(TRUE);

        // Select first row to demonstrate selection
        pGrid->SetSelectedRange(1, 1, 1, 7, TRUE);
        // Set focus to first data cell
        pGrid->SetFocus();
        pGrid->SetFocusCell(1, 1);

        // Test data for demonstrating features
        pGrid->SetItemText(1, 1, _T("Multi-select"));
        pGrid->SetItemText(1, 2, _T("Drag & Drop"));
        pGrid->SetItemText(1, 3, _T("Row/Col Resize"));
        pGrid->SetItemText(1, 4, _T("Features"));
        pGrid->SetItemText(2, 1, _T("Use Ctrl+Click"));
        pGrid->SetItemText(2, 2, _T("for multi-select"));
        pGrid->SetItemText(2, 3, _T("Drag fixed edges"));
        pGrid->SetItemText(2, 4, _T("to resize"));
        pGrid->SetItemText(3, 1, _T("Use Shift+Click"));
        pGrid->SetItemText(3, 2, _T("for range select"));
        pGrid->SetItemText(3, 3, _T("Drag cells"));
        pGrid->SetItemText(3, 4, _T("to move data"));

        // Add some sample data for testing
        for (int row = 4; row < 8; row++)
        {
            for (int col = 1; col < 6; col++)
            {
                SStringT text;
                text.Format(_T("R%d-C%d"), row, col);
                pGrid->SetItemText(row, col, text);
            }
        }

        // Demonstrate advanced cell types
        // Add URL cell
        pGrid->SetItemText(4, 1, _T("https://www.soui.com.cn"));
        pGrid->SetCellType(4, 1, CELL_TYPE_URL);

        // Add checkbox cells
        pGrid->SetItemText(5, 1, _T("Enable Feature"));
        pGrid->SetCellType(5, 1, CELL_TYPE_CHECKBOX);
        SAutoRefPtr<IGridCellCheck> pCheckCell = pGrid->GetCellOp<IGridCellCheck>(5,1);
        pCheckCell->SetCheck(TRUE);

        pGrid->SetItemText(6, 1, _T("Auto Save"));
        pGrid->SetCellType(6, 1, CELL_TYPE_CHECKBOX);
        pCheckCell = pGrid->GetCellOp<IGridCellCheck>(6,1);
        pCheckCell->SetCheck(FALSE);


        // Demonstrate cell merging
        pGrid->SetItemText(7, 1, _T("Merged Cell Example"));
        pGrid->MergeCells(7, 1, 7, 3);  // Merge cells (7,1) to (7,3)

        pGrid->SetItemText(8, 1, _T("Large Merged Area"));
        pGrid->MergeCells(8, 1, 9, 2);  // Merge 2x2 area

        // Demonstrate different inplace editing types
        pGrid->SetItemText(10, 1, _T("Text Editor"));
        pGrid->SetCellType(10, 1, CELL_TYPE_TEXT);

        pGrid->SetItemText(11, 1, _T("Dropdown List"));
        pGrid->SetCellType(11, 1, CELL_TYPE_OPTION);

        pGrid->SetItemText(12, 1, _T("Checkbox"));
        pGrid->SetCellType(12, 1, CELL_TYPE_CHECKBOX);
        pCheckCell = pGrid->GetCellOp<IGridCellCheck>(12,1);
        pCheckCell->SetCheck(TRUE);

        // Add instructions
        pGrid->SetItemText(1, 6, _T("Double-click cells to edit"));
        pGrid->SetItemText(2, 6, _T("F2 to start editing"));
        pGrid->SetItemText(3, 6, _T("Tab/Enter to navigate"));
        pGrid->SetItemText(4, 6, _T("ESC to cancel editing"));
    }

    void OnClose()
    {
        DestroyWindow();
    }

    void OnMaximize()
    {
        SendMessage(WM_SYSCOMMAND, SC_MAXIMIZE);
    }

    void OnRestore()
    {
        SendMessage(WM_SYSCOMMAND, SC_RESTORE);
    }

    void OnMinimize()
    {
        SendMessage(WM_SYSCOMMAND, SC_MINIMIZE);
    }

    void OnSize(UINT nType, CSize size)
    {
        if(nType == SIZE_MAXIMIZED){
            FindChildByName2<SWindow>(L"btn_max")->SetVisible(FALSE);
            FindChildByName2<SWindow>(L"btn_restore")->SetVisible(TRUE);
        }else{
            FindChildByName2<SWindow>(L"btn_max")->SetVisible(TRUE);
            FindChildByName2<SWindow>(L"btn_restore")->SetVisible(FALSE);
        }
        SetMsgHandled(FALSE);
    }

    void OnBtnMerge()
    {
        SGridCtrl* pGrid = FindChildByName2<SGridCtrl>(L"grid_test");
        SCELLRANGE range;
        pGrid->GetSelectedCellRange(&range);
        pGrid->MergeCells(&range);
    }

    void OnBtnUnmerge()
    {
        SGridCtrl* pGrid = FindChildByName2<SGridCtrl>(L"grid_test");
        SCELLRANGE range;
        pGrid->GetSelectedCellRange(&range);
        pGrid->UnmergeCells(range.minRow, range.minCol);
    }

    void OnGridInitInplaceWnd(IEvtArgs* e)
    {
        EventGridInitInplaceWnd* pEvt = sobj_cast<EventGridInitInplaceWnd>(e);
        IGridCell * pCell = pEvt->pCell;
        IGridInplaceWnd * pInplaceWnd = pEvt->pInplaceWnd;
        if(pCell->GetType() == CELL_TYPE_OPTION){
            SComboBox * pCombo = (SComboBox *)pInplaceWnd->GetIWindow();
            for(int i=0;i<10;i++){
                pCombo->InsertItem(i,SStringT().Format(_T("option %d"),i),0,0);
            }
            SStringT text = pCell->GetText();
            int nSel = pCombo->FindString(text);
            pCombo->SetCurSel(nSel);
            pInplaceWnd->InitData(pCell->GetText());
            pEvt->bInited = TRUE;
        }
    }

    void OnGridGetInplaceStyle(IEvtArgs *e){
        EventGridGetInplaceStyle* pEvt = sobj_cast<EventGridGetInplaceStyle>(e);
        IGridCell * pCell = pEvt->pCell;
        if(pCell->GetType() == CELL_TYPE_OPTION){
            const wchar_t * kOptionStyle = L"<combobox dropDown=\"0\" dropHeight=\"300\" class=\"cls_edit\" dotted=\"0\" animateTime=\"200\" curSel=\"2\"> \
						<dropdownStyle>\
							<root colorBkgnd=\"#ffffff\"/>\
						</dropdownStyle>\
						<liststyle itemHeight=\"30\" colorText=\"#000000\" colorSelText=\"#FFFFFF\" colorItemBkgnd=\"#FFFFFF\" colorItemSelBkgnd=\"#000088\" />\
						<editstyle inset=\"5,0,5,0\" margin=\"0\" colorText=\"#000000\" align=\"left\" colorBkgnd=\"#FFFFFF\" />\
					</combobox>";
            pEvt->pstrXmlStyle->Assign(kOptionStyle);
        }
    }

    EVENT_MAP_BEGIN()
        EVENT_NAME_COMMAND(L"btn_close", OnClose)
        EVENT_NAME_COMMAND(L"btn_max", OnMaximize)
        EVENT_NAME_COMMAND(L"btn_restore", OnRestore)
        EVENT_NAME_COMMAND(L"btn_min", OnMinimize)
        EVENT_NAME_COMMAND(L"btn_merge", OnBtnMerge)
        EVENT_NAME_COMMAND(L"btn_unmerge", OnBtnUnmerge)
        EVENT_HANDLER(EventGridInitInplaceWnd::EventID, OnGridInitInplaceWnd)
        EVENT_HANDLER(EventGridGetInplaceStyle::EventID, OnGridGetInplaceStyle)
    EVENT_MAP_END2(SHostWnd)

    BEGIN_MSG_MAP_EX(CMainDlg)
        MSG_WM_INITDIALOG(OnInitDialog)
        MSG_WM_CLOSE(OnClose)
        MSG_WM_SIZE(OnSize)
        CHAIN_MSG_MAP(SHostWnd)
    END_MSG_MAP()
};

int WINAPI _tWinMain(HINSTANCE hInstance, HINSTANCE /*hPrevInstance*/, LPTSTR lpstrCmdLine, int /*nCmdShow*/)
{
    HRESULT hRes = OleInitialize(NULL);
    SASSERT(SUCCEEDED(hRes));

    int nRet = 0;
    SApplication app(hInstance);
    SAppCfg cfg;
    SStringT srcDir = getSourceDir();
    cfg.SetRender(Render_Skia)
        .SetImgDecoder(ImgDecoder_Stb)
        .SetAppDir(srcDir)
        .SetSysResFile(srcDir + kPath_SysRes)
        .SetAppResFile(srcDir + kPath_UiRes)
        .SetLog(TRUE);
    if (!cfg.DoConfig(&app))
    {
        return -1;
    }
    // Register external window class
    app.RegisterWindowClass<SGridCtrl>();
    // Show main dialog
    {
        CMainDlg dlgMain;
        dlgMain.Create(GetActiveWindow());
        dlgMain.SendMessage(WM_INITDIALOG);
        dlgMain.CenterWindow(dlgMain.m_hWnd);
        dlgMain.ShowWindow(SW_SHOWNORMAL);
        nRet = app.Run(dlgMain.m_hWnd);
    }

    OleUninitialize();
    return nRet;
}


#if !defined(_WIN32) || defined(__MINGW32__) 
int main(int argc, char **argv)
{
    HINSTANCE hInst = GetModuleHandle(NULL);
    return _tWinMain(hInst, 0, NULL, SW_SHOWNORMAL);
}
#endif //_WIN32