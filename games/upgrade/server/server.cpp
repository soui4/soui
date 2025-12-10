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

int WINAPI _tWinMain(HINSTANCE hInstance, HINSTANCE /*hPrevInstance*/, LPTSTR /*lpstrCmdLine*/, int /*nCmdShow*/)
{
    LPTSTR pCmd = GetCommandLine();
    if (pCmd[0] == _T('\"'))
    {
        pCmd = _tcsstr(pCmd, _T("\" "));
        if (pCmd)
            pCmd += 2;
    }
    PropBag * propBag = new PropBag;
    propBag->Init(pCmd);
    int nPort = propBag->GetPort();

    SLOGI() << "start upgrade server on port " << nPort;
    CWebSocketGame game;
    BOOL bRet = game.GameStart(nPort);
    delete propBag;
    SLOGI() << " upgrade server quit, ret="<<bRet;
    return bRet?0:1;
}



#if !defined(_WIN32) || defined(__MINGW32__) 
int main(int argc, char **argv)
{
    HINSTANCE hInst = GetModuleHandle(NULL);
    return _tWinMain(hInst, 0, NULL, SW_SHOWNORMAL);
}
#endif //_WIN32