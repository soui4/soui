// MainDlg.h : 主对话框
//
// 布局里只有一个 <realwnd/>，真正的 OpenGL 渲染发生在它创建出来的
// SNativeWnd 子窗口里（见 GLWnd.h）；这个对话框负责按钮、状态文本和自检。
//
#pragma once

class CGLWnd;

class CMainDlg : public SHostDialog
{
  public:
    CMainDlg();
    ~CMainDlg();

    /** @brief 打开无人值守自检：延时dump一帧 BMP + 探针报告，然后自动退出 */
    void EnableSelfTest(BOOL bSelfTest, LPCTSTR pszOutDir);

  protected:
    BOOL OnInitDialog(HWND wndFocus, LPARAM lInitParam);
    void OnClose();
    void OnTimer(UINT_PTR nIDEvent);
    void OnToggleAnim();
    void OnSwitchMode();
    void OnRecreate();
    void OnProbe();
    void OnApiTest();

    /** @brief 取布局里 realwnd 上的 CGLWnd（必要时触发创建） */
    CGLWnd *GetGLWnd();
    void UpdateStatus();
    void RunSelfTest();

    EVENT_MAP_BEGIN()
        EVENT_NAME_COMMAND(L"btn_close", OnClose)
        EVENT_NAME_COMMAND(L"btn_anim", OnToggleAnim)
        EVENT_NAME_COMMAND(L"btn_mode", OnSwitchMode)
        EVENT_NAME_COMMAND(L"btn_recreate", OnRecreate)
        EVENT_NAME_COMMAND(L"btn_probe", OnProbe)
        EVENT_NAME_COMMAND(L"btn_apitest", OnApiTest)
    EVENT_MAP_END2(SHostDialog)

    // HostWnd 真实窗口消息处理
    BEGIN_MSG_MAP_EX(CMainDlg)
        MSG_WM_INITDIALOG(OnInitDialog)
        MSG_WM_CLOSE(OnClose)
        MSG_WM_TIMER(OnTimer)
        CHAIN_MSG_MAP(SHostDialog)
    END_MSG_MAP()

  protected:
    CGLWnd *m_pGLWnd;

    BOOL m_bSelfTest;
    SStringT m_strOutDir;
};
