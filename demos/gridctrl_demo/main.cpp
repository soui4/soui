#include <souistd.h>
#include <SouiFactory.h>
#include <com-cfg.h>

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
        pGrid->SetGridLineColor(RGB(200, 200, 200));
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

    EVENT_MAP_BEGIN()
        EVENT_NAME_COMMAND(L"btn_close", OnClose)
        EVENT_NAME_COMMAND(L"btn_max", OnMaximize)
        EVENT_NAME_COMMAND(L"btn_restore", OnRestore)
        EVENT_NAME_COMMAND(L"btn_min", OnMinimize)
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

    SComMgr* pComMgr = new SComMgr;
    SouiFactory souiFac;
    {
        BOOL bLoaded = FALSE;
        CAutoRefPtr<SOUI::IImgDecoderFactory> pImgDecoderFactory;
        CAutoRefPtr<SOUI::IRenderFactory> pRenderFactory;
        bLoaded = pComMgr->CreateRender_GDI((IObjRef**)&pRenderFactory);
        SASSERT_FMT(bLoaded, _T("load interface [render] failed!"));
        bLoaded = pComMgr->CreateImgDecoder((IObjRef**)&pImgDecoderFactory);
        SASSERT_FMT(bLoaded, _T("load interface [%s] failed!"), _T("imgdecoder"));

        pRenderFactory->SetImgDecoderFactory(pImgDecoderFactory);
        SApplication* theApp = new SApplication(pRenderFactory, hInstance);

        SAutoRefPtr<IResProvider> sysResouce(souiFac.CreateResProvider(RES_FILE), FALSE);
        SStringT srcDir = getSourceDir();
        SStringT sysRes = srcDir + kPath_SysRes;
        sysResouce->Init((LPARAM)sysRes.c_str(), 0);
        theApp->LoadSystemNamedResource(sysResouce);

        SAutoRefPtr<IResProvider> pResProvider;
        pResProvider.Attach(souiFac.CreateResProvider(RES_FILE));
        SStringT appRes = srcDir + kPath_UiRes;
        bLoaded = pResProvider->Init((LPARAM)appRes.c_str(), 0);
        if (!bLoaded)
        {
            pResProvider.Attach(souiFac.CreateResProvider(RES_PE));
            bLoaded = pResProvider->Init((WPARAM)theApp->GetModule(), 0);
        }
        SASSERT_FMT(bLoaded, _T("load app resource failed"));
        theApp->AddResProvider(pResProvider);

        theApp->AddResProvider(pResProvider);

        // Show main dialog
        {
            CMainDlg dlgMain;
            dlgMain.Create(GetActiveWindow());
            dlgMain.SendMessage(WM_INITDIALOG);
            dlgMain.CenterWindow(dlgMain.m_hWnd);
            dlgMain.ShowWindow(SW_SHOWNORMAL);
            nRet = theApp->Run(dlgMain.m_hWnd);
        }

        delete theApp;
    }

    delete pComMgr;
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