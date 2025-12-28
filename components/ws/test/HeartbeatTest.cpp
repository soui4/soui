/**
 * @file HeartbeatTest.cpp
 * @brief WebSocket心跳机制测试程序
 * 
 * 测试场景：
 * 1. 正常通信 - 客户端定期发送数据
 * 2. 空闲但响应ping - 客户端不发送数据但响应ping
 * 3. 完全无响应 - 客户端不发送数据也不响应ping
 */

#include <iostream>
#include <thread>
#include <chrono>
#include <interface/ws-i.h>
#include <helper/obj-ref-impl.hpp>
#include <com-loader.hpp>

using namespace SOUI;

// 服务器监听器
class TestSvrListener : public TObjRefImpl<ISvrListener>
{
public:
    STDMETHODIMP_(BOOL) onConnected(ISvrConnection* pConn, const char* uriPath, const char* uriArgs) override
    {
        std::cout << "[Server] Client connected: " << pConn << std::endl;
        std::cout << "  URI Path: " << uriPath << std::endl;
        std::cout << "  URI Args: " << uriArgs << std::endl;
        return TRUE;
    }

    STDMETHODIMP_(void) onConnError(ISvrConnection* pConn, const char* errStr) override
    {
        std::cout << "[Server] Connection error: " << errStr << std::endl;
    }

    STDMETHODIMP_(void) onDisconnect(ISvrConnection* pConn) override
    {
        std::cout << "[Server] Client disconnected: " << pConn << std::endl;
    }

    STDMETHODIMP_(void) onDataSent(ISvrConnection* pConn, int nMsgId) override
    {
        // std::cout << "[Server] Data sent, msgId=" << nMsgId << std::endl;
    }

    STDMETHODIMP_(void) onDataRecv(ISvrConnection* pConn, const void* data, int len, BOOL bBinary) override
    {
        std::string msg((const char*)data, len);
        std::cout << "[Server] Received: " << msg << std::endl;
        
        // 回显消息
        pConn->sendText(msg.c_str(), len);
    }
};

// 客户端监听器
class TestConnListener : public TObjRefImpl<IConnListener>
{
public:
    STDMETHODIMP_(void) onConnected() override
    {
        std::cout << "[Client] Connected to server" << std::endl;
    }

    STDMETHODIMP_(void) onConnError(const char* errStr) override
    {
        std::cout << "[Client] Connection error: " << errStr << std::endl;
    }

    STDMETHODIMP_(void) onDisconnect() override
    {
        std::cout << "[Client] Disconnected from server" << std::endl;
    }

    STDMETHODIMP_(void) onDataSent(int nMsgId) override
    {
        // std::cout << "[Client] Data sent, msgId=" << nMsgId << std::endl;
    }

    STDMETHODIMP_(void) onDataRecv(const void* data, int len, BOOL bBinary) override
    {
        std::string msg((const char*)data, len);
        std::cout << "[Client] Received: " << msg << std::endl;
    }
};

void printUsage()
{
    std::cout << "WebSocket Heartbeat Test" << std::endl;
    std::cout << "Usage:" << std::endl;
    std::cout << "  HeartbeatTest server <port> <ping_interval> <heartbeat_timeout> <ping_timeout_count>" << std::endl;
    std::cout << "  HeartbeatTest client <host> <port>" << std::endl;
    std::cout << std::endl;
    std::cout << "Example:" << std::endl;
    std::cout << "  HeartbeatTest server 3310 30 120 3" << std::endl;
    std::cout << "  HeartbeatTest client 127.0.0.1 3310" << std::endl;
}

int main(int argc, const char** argv)
{
    if (argc < 3) {
        printUsage();
        return 1;
    }

    SComLoader comLoader;
    IWebsocket* pWs = NULL;
    comLoader.CreateInstance(_T("ws"), (IObjRef**)&pWs);

    if (!pWs) {
        std::cerr << "Failed to create WebSocket instance" << std::endl;
        return 1;
    }

    std::string mode = argv[1];

    if (mode == "server") {
        if (argc < 6) {
            printUsage();
            return 1;
        }

        uint16_t port = atoi(argv[2]);
        int pingInterval = atoi(argv[3]);
        int heartbeatTimeout = atoi(argv[4]);
        int pingTimeoutCount = atoi(argv[5]);

        std::cout << "Starting server on port " << port << std::endl;
        std::cout << "  Ping interval: " << pingInterval << "s" << std::endl;
        std::cout << "  Heartbeat timeout: " << heartbeatTimeout << "s" << std::endl;
        std::cout << "  Ping timeout count: " << pingTimeoutCount << std::endl;

        TestSvrListener listener;
        IWsServer* pServer = pWs->CreateWsServer(&listener);

        SvrOption option = { FALSE, NULL, NULL };
        SvrPingCfg pingCfg = { pingInterval, heartbeatTimeout, pingTimeoutCount };

        int ret = pServer->start(port, "test", option, pingCfg);
        if (ret != 0) {
            std::cerr << "Failed to start server, error code: " << ret << std::endl;
            return 1;
        }

        std::cout << "Server started, press Ctrl+C to stop..." << std::endl;
        pServer->wait(-1);

        pServer->Release();
    }
    else if (mode == "client") {
        if (argc < 4) {
            printUsage();
            return 1;
        }

        const char* host = argv[2];
        uint16_t port = atoi(argv[3]);

        std::cout << "Connecting to " << host << ":" << port << std::endl;

        TestConnListener listener;
        IWsClient* pClient = pWs->CreateWsClient(&listener);

        ClientOption option = { FALSE, NULL, FALSE, FALSE, FALSE };
        int ret = pClient->connectTo(host, "/test/", port, "", option);
        if (ret != 0) {
            std::cerr << "Failed to connect, error code: " << ret << std::endl;
            return 1;
        }

        // 等待连接建立
        std::this_thread::sleep_for(std::chrono::seconds(1));

        // 发送测试消息
        for (int i = 0; i < 5; i++) {
            char msg[64];
            sprintf(msg, "Test message %d", i);
            pClient->sendText(msg);
            std::this_thread::sleep_for(std::chrono::seconds(2));
        }

        std::cout << "Messages sent, waiting for server response..." << std::endl;
        pClient->wait(10000);

        pClient->Release();
    }
    else {
        printUsage();
        return 1;
    }

    pWs->Release();
    return 0;
}

