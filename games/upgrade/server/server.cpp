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
#include "WebSocketGame.h"
#include "Upgrade.h"
#include "PropBag.h"
#define kLogTag "UpgradeServer"

int run(LPCTSTR pszCfg){
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