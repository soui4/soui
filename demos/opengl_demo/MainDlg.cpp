// MainDlg.cpp : 主对话框实现
//

#include "stdafx.h"
#include "MainDlg.h"
#include "GLWnd.h"

#define TIMER_STATUS 100 /**< 刷新状态文本 */
#define TIMER_SELFTEST 101 /**< 自检：等待渲染若干帧后 dump */

#ifdef _WIN32
#define GL_PATH_SEP _T("\\")
#else
#define GL_PATH_SEP _T("/")
#endif

/** 以 UTF-8(带 BOM) 写文本，方便用记事本/编辑器直接看 */
static BOOL SaveTextUtf8(LPCTSTR pszPath, const SStringT &strText)
{
    FILE *pFile = _tfopen(pszPath, _T("wb"));
    if (!pFile)
        return FALSE;
    const BYTE bom[3] = {0xEF, 0xBB, 0xBF};
    fwrite(bom, 1, 3, pFile);
#ifdef _WIN32
    SStringA strUtf8 = S_CW2A(strText, CP_UTF8);
#else
    // 非 Windows 平台 SStringT 即 UTF-8 编码的 SStringA，直接写出
    SStringA strUtf8 = strText;
#endif
    const char *psz = strUtf8.c_str();
    fwrite(psz, 1, strlen(psz), pFile);
    fclose(pFile);
    return TRUE;
}

CMainDlg::CMainDlg()
    : SHostDialog(_T("LAYOUT:opengl.demo"))
    , m_pGLWnd(NULL)
    , m_bSelfTest(FALSE)
{
}

CMainDlg::~CMainDlg()
{
}

void CMainDlg::EnableSelfTest(BOOL bSelfTest, LPCTSTR pszOutDir)
{
    m_bSelfTest = bSelfTest;
    m_strOutDir = pszOutDir;
}

BOOL CMainDlg::OnInitDialog(HWND wndFocus, LPARAM lInitParam)
{
    // 主动取一次：<realwnd/> 的 init 属性为 0 时是懒创建的，
    // 这里触发后就能立刻在状态栏里显示驱动信息。
    GetGLWnd();
    UpdateStatus();

    SetTimer(TIMER_STATUS, 200, NULL);
    if (m_bSelfTest)
        SetTimer(TIMER_SELFTEST, 1200, NULL);
    return TRUE;
}

CGLWnd *CMainDlg::GetGLWnd()
{
    if (m_pGLWnd && ::IsWindow(m_pGLWnd->m_hWnd))
        return m_pGLWnd;

    IWindow *pReal = FindChildByName(L"glwnd");
    if (!pReal)
        return NULL;

    SAutoRefPtr<IRealWnd> pRealWnd;
    pReal->QueryInterface(IID_IRealWnd, (IObjRef **)&pRealWnd);
    if (!pRealWnd)
        return NULL;

    pRealWnd->GetRealHwnd(TRUE); // 触发 SRealWnd::InitRealWnd -> IRealWndHandler
    m_pGLWnd = (CGLWnd *)pRealWnd->GetData();
    return m_pGLWnd;
}

void CMainDlg::UpdateStatus()
{
    SWindow *pText = FindChildByName(L"txt_info");
    if (!pText)
        return;

    SStringT str;
    if (m_pGLWnd)
    {
        str = m_pGLWnd->GetSummary();
    }
    else
    {
        str = _T("OpenGL 子窗口未创建（检查驱动是否支持 OpenGL，或看日志）");
    }
    str.AppendFormat(_T("\n操作: 在 GL 区域内 左键拖动旋转 / 滚轮缩放 / 双击暂停或继续"));

    pText->SetWindowText(str);

    SWindow *pBtnAnim = FindChildByName(L"btn_anim");
    if (pBtnAnim && m_pGLWnd)
        pBtnAnim->SetWindowText(m_pGLWnd->IsAnimating() ? _T("暂停动画") : _T("继续动画"));

    SWindow *pBtnMode = FindChildByName(L"btn_mode");
    if (pBtnMode && m_pGLWnd)
    {
        pBtnMode->SetWindowText(m_pGLWnd->GetRenderMode() == CGLWnd::Mode_Shader
                                    ? _T("切到固定管线")
                                    : _T("切到 GLSL 着色器"));
    }
}

void CMainDlg::OnTimer(UINT_PTR nIDEvent)
{
    if (nIDEvent == TIMER_STATUS)
    {
        GetGLWnd();
        UpdateStatus();
        return;
    }
    if (nIDEvent == TIMER_SELFTEST)
    {
        KillTimer(TIMER_SELFTEST);
        RunSelfTest();
        return;
    }
    SetMsgHandled(FALSE);
}

void CMainDlg::RunSelfTest()
{
    CGLWnd *pGL = GetGLWnd();

    // WGL 入口契约测试放在抓图之前：它内部会临时创建/切换/销毁上下文，
    // 一旦它把线程级状态弄坏，后面三张截图会立刻失败，不会把问题掩盖过去。
    int nApiPass = 0, nApiFail = 0, nApiSkip = 0;
    SStringT strApiTest;
    if (pGL)
        strApiTest = pGL->RunApiTests(&nApiPass, &nApiFail, &nApiSkip);

    SStringT strBmp = m_strOutDir + GL_PATH_SEP + _T("opengl_selftest.bmp");
    SStringT strBmpShader = m_strOutDir + GL_PATH_SEP + _T("opengl_selftest_shader.bmp");
    SStringT strTxt = m_strOutDir + GL_PATH_SEP + _T("opengl_selftest.txt");

    BOOL bShot = pGL ? pGL->SaveSnapshot(strBmp) : FALSE;

    // 再切到 GLSL 着色器 + VBO 路径抓一张，确认两条渲染路径都真的出图
    BOOL bShotShader = FALSE;
    if (pGL && pGL->GetRenderMode() == CGLWnd::Mode_FixedFunction)
    {
        if (pGL->ToggleRenderMode() == CGLWnd::Mode_Shader)
            bShotShader = pGL->SaveSnapshot(strBmpShader);
    }

    // 最后验证上下文重建（销毁 HGLRC 后重建上下文、VBO 与着色器程序）
    BOOL bShotRecreate = FALSE;
    SStringT strBmpRecreate = m_strOutDir + GL_PATH_SEP + _T("opengl_selftest_recreated.bmp");
    if (pGL)
    {
        if (pGL->RecreateContext())
            bShotRecreate = pGL->SaveSnapshot(strBmpRecreate);
    }

    BOOL bAllOK = bShot && bShotShader && bShotRecreate && nApiFail == 0;

    SStringT strReport;
    strReport.Format(_T("selftest: %s\n"), bAllOK ? _T("PASS") : _T("FAIL"));
    strReport.AppendFormat(_T("snapshot(fixed pipeline): %s -> %s\n"), strBmp.c_str(),
                           bShot ? _T("OK") : _T("FAIL"));
    strReport.AppendFormat(_T("snapshot(GLSL shader)   : %s -> %s\n"), strBmpShader.c_str(),
                           bShotShader ? _T("OK") : _T("FAIL"));
    strReport.AppendFormat(_T("snapshot(recreate ctx) : %s -> %s\n"), strBmpRecreate.c_str(),
                           bShotRecreate ? _T("OK") : _T("FAIL"));
    strReport.AppendFormat(_T("WGL 入口测试 (PASS/FAIL/SKIP): %d/%d/%d\n"), nApiPass, nApiFail,
                           nApiSkip);
    if (pGL)
    {
        SStringT strSummary = pGL->GetSummary();
        SStringT strProbe = pGL->GetProbeReport();
        strReport.AppendFormat(_T("%s\n\n"), strSummary.c_str());
        strReport.AppendFormat(_T("%s"), strProbe.c_str());
        if (!strApiTest.IsEmpty())
            strReport.AppendFormat(_T("\n%s"), strApiTest.c_str());
    }
    else
    {
        strReport.AppendFormat(_T("GL 子窗口未创建\n"));
    }

    // 几何自检：realwnd 控件的 DUI 矩形应当与原生子窗口的矩形重合
    CRect rcCtrl(0, 0, 0, 0);
    SWindow *pReal = FindChildByName(L"glwnd");
    if (pReal)
        pReal->GetWindowRect(&rcCtrl);
    strReport.AppendFormat(_T("\n[几何]\n"));
    SWindow *pHost = FindChildByName(L"gl_host");
    if (pHost)
    {
        CRect rcHost(0, 0, 0, 0);
        pHost->GetWindowRect(&rcHost);
        strReport.AppendFormat(_T("  容器 gl_host    (宿主坐标): %d,%d,%d,%d (%dx%d)\n"), rcHost.left,
                               rcHost.top, rcHost.right, rcHost.bottom, rcHost.Width(), rcHost.Height());
    }
    strReport.AppendFormat(_T("  realwnd 控件 (宿主坐标): %d,%d,%d,%d (%dx%d)\n"), rcCtrl.left,
                           rcCtrl.top, rcCtrl.right, rcCtrl.bottom, rcCtrl.Width(), rcCtrl.Height());
    if (pGL && ::IsWindow(pGL->m_hWnd))
    {
        RECT rcWnd = {};
        ::GetWindowRect(pGL->m_hWnd, &rcWnd);
        ::MapWindowPoints(NULL, GetHostHwnd(), (LPPOINT)&rcWnd, 2);
        strReport.AppendFormat(_T("  GL 子窗口   (宿主坐标): %d,%d,%d,%d (%dx%d)\n"), rcWnd.left,
                               rcWnd.top, rcWnd.right, rcWnd.bottom, rcWnd.right - rcWnd.left,
                               rcWnd.bottom - rcWnd.top);
    }
    SaveTextUtf8(strTxt, strReport);

    EndDialog(bAllOK ? 0 : 1);
}

void CMainDlg::OnToggleAnim()
{
    CGLWnd *pGL = GetGLWnd();
    if (pGL)
        pGL->ToggleAnimating();
    UpdateStatus();
}

void CMainDlg::OnSwitchMode()
{
    CGLWnd *pGL = GetGLWnd();
    if (pGL)
        pGL->ToggleRenderMode();
    UpdateStatus();
}

void CMainDlg::OnRecreate()
{
    CGLWnd *pGL = GetGLWnd();
    if (pGL)
        pGL->RecreateContext();
    UpdateStatus();
}

void CMainDlg::OnProbe()
{
    CGLWnd *pGL = GetGLWnd();
    SStringT strReport = pGL ? pGL->GetProbeReport() : SStringT(_T("GL 子窗口未创建"));
    SStringT strPath = m_strOutDir.IsEmpty() ? SStringT(_T("opengl_probe.txt"))
                                             : (m_strOutDir + GL_PATH_SEP + _T("opengl_probe.txt"));
    SaveTextUtf8(strPath, strReport);

    SStringT strMsg;
    strMsg.Format(_T("%s\n\n完整报告已写入:\n%s"), strReport.c_str(), strPath.c_str());
    ::MessageBox(m_hWnd, strMsg, _T("OpenGL API 探针"), MB_OK | MB_ICONINFORMATION);
}

void CMainDlg::OnApiTest()
{
    CGLWnd *pGL = GetGLWnd();
    int nPass = 0, nFail = 0, nSkip = 0;
    SStringT strReport = pGL ? pGL->RunApiTests(&nPass, &nFail, &nSkip)
                             : SStringT(_T("GL 子窗口未创建，无法测试 WGL 入口"));
    if (!pGL)
        nFail = 1;

    SStringT strPath = m_strOutDir.IsEmpty() ? SStringT(_T("opengl_apitest.txt"))
                                             : (m_strOutDir + GL_PATH_SEP + _T("opengl_apitest.txt"));
    SaveTextUtf8(strPath, strReport);

    SStringT strMsg;
    strMsg.Format(_T("PASS %d / FAIL %d / SKIP %d\n\n%s\n完整报告已写入:\n%s"), nPass, nFail, nSkip,
                  strReport.c_str(), strPath.c_str());
    ::MessageBox(m_hWnd, strMsg, _T("WGL API 测试"),
                 MB_OK | (nFail ? MB_ICONWARNING : MB_ICONINFORMATION));
}

void CMainDlg::OnClose()
{
    KillTimer(TIMER_STATUS);
    EndDialog(0);
}
