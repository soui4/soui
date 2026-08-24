
#include <iostream>
#include <chrono>
#include <map>
#include <interface/ws-i.h>
#include <helper/obj-ref-impl.hpp>
#include <string/strcpcvt.h>
#include <tchar.h>
#include <com-loader.hpp>
#include <helper/slog.h>
#include <com-loader.hpp>
#include <interface/slog-i.h>
#define  SCOM_MASK scom_mask_log4z
#include <commgr2.h>
#include "WebSocketGame.h"

#include "PropBag.h"
#include "ThemeResourceProvider.h"
#define kLogTag "CnChessServer"

#ifdef _WIN32
#include <windows.h>
#include <signal.h>
#else
#include <signal.h>
#endif

static ILogMgr *s_logMgr = NULL;
static CWebSocketGame* g_game = nullptr;

static void SouiLog_Callback(const char *tag, const char *pLogStr, int level, const char *file, int line, const char *fun, void *retAddr)
{
    SASSERT(s_logMgr);
    if (s_logMgr->prePushLog(level))
    {
        s_logMgr->pushLog(level, tag, pLogStr, file, line, fun, retAddr);
    }
}

#ifdef _WIN32
static BOOL WINAPI ConsoleHandler(DWORD dwType)
{
    switch (dwType)
    {
    case CTRL_C_EVENT:
    case CTRL_CLOSE_EVENT:
    case CTRL_LOGOFF_EVENT:
    case CTRL_SHUTDOWN_EVENT:
        if (g_game) {
            SLOGI() << "Received exit signal, stopping game...";
            g_game->GameStop();
        }
        return TRUE;
    default:
        return FALSE;
    }
}
#else
static void signalHandler(int signum)
{
    if (g_game) {
        g_game->GameStop();
    }
    exit(signum);
}
#endif

int run(LPCTSTR pszCfg){
    SComMgr2 comMgr;
    {
        SAutoRefPtr<ILogMgr> log;
        comMgr.CreateLog4z((IObjRef**)&log);
        if(log){
            s_logMgr = log;
            SLog::setLogCallback(SouiLog_Callback);
            log->setLoggerName("ChessServer");
            log->start();
        }
        
        PropBag *propBag = new PropBag;
        propBag->Init(pszCfg);
        int nPort = propBag->GetPort();
        SLOGI() << "start chess server on port " << nPort;

        // 加载多平台主题资源包
        ThemeResourceProvider* pThemeProv = ThemeResourceProvider::GetInstance();
        char szExePath[MAX_PATH];
        GetModuleFileNameA(NULL, szExePath, MAX_PATH);
        char* pSlash = strrchr(szExePath, '\\');
        if (!pSlash) pSlash = strrchr(szExePath, '/');

        // 桌面端主题（OS_ID_WINDOWS）
        char szPcThemeZip[MAX_PATH];
        if (pSlash) {
            strcpy(pSlash + 1, "pc_theme.zip");
            strcpy(szPcThemeZip, szExePath);
        } else {
            strcpy(szPcThemeZip, "pc_theme.zip");
        }
        if (pThemeProv->Init(OS_ID_WINDOWS, szPcThemeZip)) {
            char szMD5[33] = { 0 };
            MD5_ToHexString(pThemeProv->GetMD5(OS_ID_WINDOWS), szMD5);
            SLOGI() << "PC theme loaded: " << szPcThemeZip << " md5=" << szMD5;
        } else {
            SLOGW() << "PC theme not found: " << szPcThemeZip;
        }

        // 移动端主题（OS_ID_ANDROID / OS_ID_IOS 共用同一资源包）
        char szMobileThemeZip[MAX_PATH];
        if (pSlash) {
            strcpy(pSlash + 1, "mobile_theme.zip");
            strcpy(szMobileThemeZip, szExePath);
        } else {
            strcpy(szMobileThemeZip, "mobile_theme.zip");
        }
        if (pThemeProv->Init(OS_ID_ANDROID, szMobileThemeZip)) {
            char szMD5[33] = { 0 };
            MD5_ToHexString(pThemeProv->GetMD5(OS_ID_ANDROID), szMD5);
            SLOGI() << "Mobile theme loaded: " << szMobileThemeZip << " md5=" << szMD5;
        } else {
            SLOGW() << "Mobile theme not found: " << szMobileThemeZip;
        }
        // iOS 复用 Android 主题（同一资源包）
        pThemeProv->Init(OS_ID_IOS, szMobileThemeZip);
        CWebSocketGame game;
        g_game = &game;
        
#ifdef _WIN32
        SetConsoleCtrlHandler(ConsoleHandler, TRUE);
#else
        signal(SIGINT, signalHandler);
        signal(SIGTERM, signalHandler);
#endif
        BOOL bRet = game.GameStart(nPort);
        delete propBag;
        SLOGI() << " chess server quit, ret=" << bRet;
        return bRet ? 0 : 1;
    }
}

int _tmain(int argc, TCHAR **argv)
{
    LPCTSTR pszCfg = _T("./config.ini");
    if(argc > 1)
        pszCfg = argv[1];
    return run(pszCfg);
}


#if !defined(_WIN32) || defined(__MINGW32__) 
int main(int argc, char **argv)
{
    if(argc>1)
    {
        #ifdef _UNICODE
        SStringW strArg=S_CA2W(argv[1]);
        return run(strArg);
        #else
        return run(argv[1]);
        #endif
    }else{
        return run(NULL);
    }
}
#endif //_WIN32