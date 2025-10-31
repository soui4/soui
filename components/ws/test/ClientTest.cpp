/**
 * @file ClientTest.cpp
 * @author paul
 * @date 13.03.19
 * @brief Small test/example program for the WebSocketClient
 */
#include <iostream>
#include <chrono>
#include <thread>
#include <string>
#include <interface/ws-i.h>
#include <helper/obj-ref-impl.hpp>
#include <tchar.h>
#include <com-loader.hpp>

using namespace SOUI;

class ConnListener : public TObjRefImpl<IConnListener> {
  public:
    // 通过 IConnListener 继承
    STDMETHODIMP_(void) onConnected() override
    {
        std::cout << "onConnected\n";
    }

    STDMETHODIMP_(void) onDisconnect() override
    {
        std::cout << "onDisconnect\n";
    }

    STDMETHODIMP_(void) onDataSent(int nMsgId) override
    {
        // std::cout << "onDataSent msgId="<<nMsgId<<"\n";
    }

    STDMETHODIMP_(void) onDataRecv(const void *data, int len, BOOL bBinary) override
    {
        std::cout << "onDataRecv msgId=" << std::string((const char *)data, len) << "\n";
    }

    STDMETHODIMP_(void) onConnError(const char *errStr) override
    {
        std::cout << "connection error:" << errStr << "\n";
    }
};

int main(int cArgs, const char **argv)
{
    SComLoader comLoader;
    ConnListener listener;
    IWebsocket *pWs = NULL;
    IWsClient *pClient = NULL;
    comLoader.CreateInstance(_T("ws"), (IObjRef **)&pWs);
    pClient = pWs->CreateWsClient(&listener);

    char szUri[1024] = "/test/";
    if (cArgs >= 2)
    {
        sprintf(szUri, "/test/?%s", argv[1]);
    }
    ClientOption optoin = { true, ".\\server.crt", true, true, true };
    int nRet = pClient->connectTo("localhost", szUri, 4043, "", optoin);

    for (int i = 0; i < 10; i++)
    {
        pClient->wait(1000);
        char szMsg[50];
        sprintf(szMsg, "hello, msgid=%d", i);
        pClient->sendText(szMsg);
    }
    pClient->wait(-1);

    pClient->Release();
    pWs->Release();

    std::cout << "connection lose" << std::endl;
    system("pause");
}
