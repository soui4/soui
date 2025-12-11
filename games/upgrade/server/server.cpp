// server.cpp : Defines the entry point for the application.
//

#include <iostream>
#include <chrono>
#include <map>
#include <interface/ws-i.h>
#include <helper/obj-ref-impl.hpp>
#include <tchar.h>
#include <com-loader.hpp>
#include <helper/slog.h>
#include "WebSocketGame.h"
#include "Upgrade.h"
#include "PropBag.h"
#define kLogTag "UpgradeServer"

int _tmain(int argc, TCHAR **argv)
{
    PropBag *propBag = new PropBag;
    if (argc>1)
        propBag->Init(argv[1]);
    int nPort = propBag->GetPort();
    SLOGI() << "start upgrade server on port " << nPort;
    CWebSocketGame game;
    BOOL bRet = game.GameStart(nPort);
    delete propBag;
    SLOGI() << " upgrade server quit, ret=" << bRet;
    return bRet ? 0 : 1;
}


#if !defined(_WIN32) || defined(__MINGW32__) 
int main(int argc, char **argv)
{
    return _tmain(argc, argv);
}
#endif //_WIN32