/**
 * @file WsServer.hpp
 * @author paul
 * @date 10.03.19
 * @brief Declares the WsServer class
 */

#ifndef WsServer_H
#define WsServer_H

#include <interface/ws-i.h>
#include <helper/obj-ref-impl.hpp>
#include <string>
#include "Connection.h"
#include <condition_variable>
#include <thread>

SNSBEGIN

/**
 * @brief WebSocket服务器配置
 */
class WsCfg{
  public:
  WsCfg(){
    pingIntervalSeconds = 5;
    nHeartbeatSeconds = 10;
    nPingTimeoutCount = 3;
  }
  int pingIntervalSeconds;  // ping间隔时间，单位秒
  int nHeartbeatSeconds;   // 心跳间隔时间，单位秒，ping间隔时间的2倍即可
  int nPingTimeoutCount;   // ping超时次数，超过此次数则关闭连接
};

class WsServer : public TObjRefImpl<IWsServer> {
    friend class SvrConnection;

  public:
    WsServer(ISvrListener *pListener, const WsCfg &cfg = WsCfg());
    ~WsServer();
  public:
    STDMETHODIMP_(int) start(THIS_ uint16_t port, const char *protocolName, SvrOption option) OVERRIDE;
    STDMETHODIMP_(BOOL) wait(THIS_ int timeoutMs) OVERRIDE;
    STDMETHODIMP_(void) quit(THIS) OVERRIDE;
  private:
    void run();
    int handler(lws *websocket, lws_callback_reasons reasons, void *id, void *data, size_t len);

    ISvrListener *m_pListener;

    std::condition_variable m_cvQuit;
    std::mutex m_mutex;
    bool m_finished;

    std::thread m_worker;

    std::string m_protocolName;
    WsCfg m_cfg;

    lws_context *m_context;

    static int cb_lws(lws *websocket, lws_callback_reasons reasons, void *userData, void *data, std::size_t len);

};
SNSEND
#endif // WsServer_H