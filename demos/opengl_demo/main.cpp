// opengl_demo.cpp : 程序入口
//
// 演示 <realwnd/> + 自定义 IRealWndHandler + SNativeWnd 子窗口 + OpenGL：
//   SOUI 宿主窗口
//     └─ DUI 控件（按钮、文本）
//     └─ <realwnd wndclass="SOUI_GLWnd"/>  ->  CGLWnd : SNativeWnd（带 OpenGL 上下文）
//
// 命令行：
//   无参数     正常交互运行
//   --selftest 无人值守自检：渲染若干帧后把画面 dump 成 BMP 并写出探针报告，然后退出
//

#include "stdafx.h"
#include <SAppCfg.h>
#include "MainDlg.h"
#include "GLRealWndHandler.h"

using namespace SNS;

/**
 * @brief 取源码目录（__FILE__ 所在目录）。
 *
 * 注意：MSVC 的 __FILE__ 用反斜杠，MinGW 用正斜杠，这里两种都兼容，
 * 否则资源路径会被拼坏、DoConfig 直接失败。
 */
static SStringT GetSourceDir()
{
    SStringA file(__FILE__);
    int nPos = file.ReverseFind('/');
    int nPos2 = file.ReverseFind('\\');
    if (nPos2 > nPos)
        nPos = nPos2;
    if (nPos > 0)
        file = file.Left(nPos);
    return S_CA2T(file);
}

int WINAPI _tWinMain(HINSTANCE hInstance, HINSTANCE /*hPrevInstance*/, LPTSTR lpstrCmdLine, int /*nCmdShow*/)
{
    HRESULT hRes = ::OleInitialize(NULL);
    SASSERT(SUCCEEDED(hRes));

    int nRet = 0;
    {
        SApplication app(hInstance);

        SAppCfg cfg;
        SStringT srcDir = GetSourceDir();
        cfg.SetRender(Render_Skia)
            .SetImgDecoder(ImgDecoder_Stb)
            .SetAppDir(srcDir)
            .SetSysResFile(srcDir + _T("/../../soui-sys-resource"))
            .SetAppResFile(srcDir + _T("/uires"))
            .SetLog(FALSE);
        if (!cfg.DoConfig(&app))
        {
            // Skia 组件缺失时退回 GDI：本 demo 的重点是原生 GL 子窗口，
            // DUI 部分只有标题栏/按钮/文本，两种渲染器都够用。
            SAppCfg cfgGdi;
            cfgGdi.SetRender(Render_Gdi)
                .SetImgDecoder(ImgDecoder_Stb)
                .SetAppDir(srcDir)
                .SetSysResFile(srcDir + _T("/../../soui-sys-resource"))
                .SetAppResFile(srcDir + _T("/uires"))
                .SetLog(FALSE);
            if (!cfgGdi.DoConfig(&app))
            {
                ::OleUninitialize();
                return -1;
            }
        }

        // 用自定义 handler 接管 <realwnd/>：它在里面创建 CGLWnd（SNativeWnd 派生）
        IRealWndHandler *pHandler = new CGLRealWndHandler();
        app.SetRealWndHandler(pHandler);
        pHandler->Release(); // 引用交给 SApplication 持有

        // 用 GetCommandLine() 而不是 lpstrCmdLine：MinGW 下入口走 main() 转发时
        // lpstrCmdLine 可能是 NULL，直接读系统命令行最稳。
        BOOL bSelfTest = FALSE;
        LPCTSTR pszCmdLine = ::GetCommandLine();
        if (pszCmdLine)
        {
            SStringT strCmd(pszCmdLine);
            bSelfTest = strCmd.Find(_T("--selftest")) >= 0;
        }

        TCHAR szDir[MAX_PATH] = {0};
        ::GetCurrentDirectory(MAX_PATH, szDir);

        {
            CMainDlg dlgMain;
            dlgMain.EnableSelfTest(bSelfTest, szDir);
            nRet = (int)dlgMain.DoModal();
        }

        app.SetRealWndHandler(NULL); // Release -> 释放 handler
    }

    ::OleUninitialize();
    return nRet;
}

#if !defined(_WIN32) || defined(__MINGW32__)
int main(int argc, char **argv)
{
    HINSTANCE hInst = GetModuleHandle(NULL);
    return _tWinMain(hInst, 0, NULL, SW_SHOWNORMAL);
}
#endif //_WIN32
