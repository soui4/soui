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
#include "ChessServer.h"
#define kLogTag "ChessServer"

#ifdef _WIN32
#include <windows.h>
#include <signal.h>
#else
#include <signal.h>
#endif

static ILogMgr *s_logMgr = NULL;
static CChessServer* g_game = nullptr;

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
            Log::setLogCallback(SouiLog_Callback);
            log->setLoggerName("ChessServer");
            log->start();
        }
        
        int nPort = 8080; // 默认端口
        SLOGI() << "start chess server on port " << nPort;
        CChessServer game;
        g_game = &game;
        
#ifdef _WIN32
        SetConsoleCtrlHandler(ConsoleHandler, TRUE);
#else
        signal(SIGINT, signalHandler);
        signal(SIGTERM, signalHandler);
#endif
        
        BOOL bRet = game.GameStart(nPort);
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