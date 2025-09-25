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

        // Set column headers with specific names for different cell types
        pGrid->SetItemText(0, 0, _T(""));
        pGrid->SetItemText(0, 1, _T("Name"));
        pGrid->SetItemText(0, 2, _T("Number"));
        pGrid->SetItemText(0, 3, _T("Date"));
        pGrid->SetItemText(0, 4, _T("Color"));
        pGrid->SetItemText(0, 5, _T("DateTime"));
        pGrid->SetItemText(0, 6, _T("Checkbox"));
        pGrid->SetItemText(0, 7, _T("URL"));
        for (int col = 8; col < kNumCols; col++)
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

        // Fill with sample data and set different cell types
        for (int row = 1; row < kNumRows; row++)
        {
            for (int col = 1; col < kNumCols; col++)
            {
                SStringT strText;
                if (col == 1)
                {
                    // First column: Names (Text cells)
                    LPCTSTR names[] = {_T("Alice"), _T("Bob"), _T("Charlie"), _T("David"), _T("Eve"),
                                      _T("Frank"), _T("Grace"), _T("Henry"), _T("Ivy"), _T("Jack"),
                                      _T("Kate"), _T("Liam"), _T("Mia"), _T("Noah"), _T("Olivia"),
                                      _T("Paul"), _T("Quinn"), _T("Ruby"), _T("Sam")};
                    strText = names[(row - 1) % 19];
                    pGrid->SetItemText(row, col, strText);
                }
                else if (col == 2)
                {
                    // Second column: Numbers (Text cells)
                    strText.Format(_T("%d"), (row * 7 + col * 3) % 100);
                    pGrid->SetItemText(row, col, strText);
                }
                else if (col == 3)
                {
                    // Third column: Dates (Text cells for now)
                    strText.Format(_T("2024-%02d-%02d"), (row % 12) + 1, (row % 28) + 1);
                    pGrid->SetItemText(row, col, strText);
                }
                else if (col == 4)
                {
                    // Fourth column: Color cells
                    pGrid->SetCellType(row, col, CELL_TYPE_COLOR);
                    // Set different colors for demonstration
                    COLORREF colors[] = {RGB(255, 0, 0), RGB(0, 255, 0), RGB(0, 0, 255),
                                       RGB(255, 255, 0), RGB(255, 0, 255), RGB(0, 255, 255),
                                       RGB(128, 128, 128), RGB(255, 128, 0), RGB(128, 255, 128)};
                    COLORREF cr = colors[(row - 1) % 9];
                    strText.Format(_T("#%02X%02X%02X"), GetRValue(cr), GetGValue(cr), GetBValue(cr));
                    pGrid->SetItemText(row, col, strText);
                }
                else if (col == 5)
                {
                    // Fifth column: DateTime cells
                    pGrid->SetCellType(row, col, CELL_TYPE_DATETIMEPICKER);
                    strText.Format(_T("2024-%02d-%02d %02d:%02d:00"),
                                  (row % 12) + 1, (row % 28) + 1,
                                  (row % 24), (row * 15) % 60);
                    pGrid->SetItemText(row, col, strText);
                }
                else if (col == 6)
                {
                    // Sixth column: Checkbox cells
                    pGrid->SetCellType(row, col, CELL_TYPE_CHECKBOX);
                    strText = (row % 2 == 0) ? _T("1") : _T("0");
                    pGrid->SetItemText(row, col, strText);
                }
                else if (col == 7)
                {
                    // Seventh column: URL cells
                    pGrid->SetCellType(row, col, CELL_TYPE_URL);
                    strText.Format(_T("https://example.com/user%d"), row);
                    pGrid->SetItemText(row, col, strText);
                }
                else
                {
                    // Other columns: Mixed data (Text cells)
                    strText.Format(_T("Cell(%d,%d)"), row, col);
                    pGrid->SetItemText(row, col, strText);
                }
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
		int cellType = pCell->GetType();
        if(cellType == CELL_TYPE_OPTION){
            const wchar_t * kOptionStyle = L"<combobox dropDown=\"0\" dropHeight=\"300\" class=\"cls_edit\" dotted=\"0\" animateTime=\"200\" curSel=\"2\"> \
						<dropdownStyle>\
							<root colorBkgnd=\"#ffffff\"/>\
						</dropdownStyle>\
						<liststyle itemHeight=\"30\" colorText=\"#000000\" colorSelText=\"#FFFFFF\" colorItemBkgnd=\"#FFFFFF\" colorItemSelBkgnd=\"#000088\" />\
						<editstyle inset=\"5,0,5,0\" margin=\"0\" colorText=\"#000000\" align=\"left\" colorBkgnd=\"#FFFFFF\" />\
					</combobox>";
            pEvt->pstrXmlStyle->Assign(kOptionStyle);
        }else if(cellType == CELL_TYPE_DATETIMEPICKER){
			const wchar_t * kDatePickerStyle =  L"<dateTimePicker  padding=\"4,0,4,0\" dropWidth=\"300\" timeEnable=\"1\"> \
				<calstyle ncSkin=\"_skin.sys.border\" daySkin=\"_skin.sys.btn.normal\" margin=\"1\" font=\"face:微软雅黑,size=14\" />\
				</dateTimePicker>";
			pEvt->pstrXmlStyle->Assign(kDatePickerStyle);
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