// server.cpp : Defines the entry point for the application.
//

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
#include "WebSocketGame.h"
#include "Upgrade.h"
#include "PropBag.h"
#define kLogTag "UpgradeServer"

static ILogMgr *s_logMgr = NULL;
static void SouiLog_Callback(const char *tag, const char *pLogStr, int level, const char *file, int line, const char *fun, void *retAddr)
{
    SASSERT(s_logMgr);
    if (s_logMgr->prePushLog(level))
    {
        s_logMgr->pushLog(level, tag, pLogStr, file, line, fun, retAddr);
    }
}

int run(LPCTSTR pszCfg){
    SComLoader loader;
    SAutoRefPtr<ILogMgr> log;
    loader.CreateInstance(_T("log4z"),(IObjRef**)&log);
    if(log){
        s_logMgr = log;
        Log::setLogCallback(SouiLog_Callback);
        log->setLoggerName("UpgradeServer");
        log->start();
    }
    PropBag *propBag = new PropBag;
    propBag->Init(pszCfg);
    int nPort = propBag->GetPort();
    SLOGI() << "start upgrade server on port " << nPort;
    CWebSocketGame game;
    BOOL bRet = game.GameStart(nPort);
    delete propBag;
    SLOGI() << " upgrade server quit, ret=" << bRet;
    return bRet ? 0 : 1;
}
int _tmain(int argc, TCHAR **argv)
{
    LPCTSTR pszCfg = argc>1?argv[1]:NULL;
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