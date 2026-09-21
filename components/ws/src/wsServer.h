/**
 * @file WsServer.hpp
 * @author paul
 * @date 10.03.19
 * @brief Declares the WsServer class
 */

#ifndef WsServer_H
#define WsServer_H

#include "Connection.h"
#include "TimerGenerator.h"
#include <interface/ws-i.h>
#include <helper/obj-ref-impl.hpp>
#include <string>
#include <condition_variable>
#include <thread>
#include <mutex>
#include <deque>

SNSBEGIN

class WsServer : public TObjRefImpl<IWsServer>{
    friend class SvrConnection;

  public:
    WsServer(ISvrListener *pListener);
    ~WsServer();
  public:
    STDMETHODIMP_(int) start(THIS_ uint16_t port, const char *protocolName, SvrOption option, SvrPingCfg pingCfg) OVERRIDE;
    STDMETHODIMP_(BOOL) wait(THIS_ int timeoutMs) OVERRIDE;
    STDMETHODIMP_(void) quit(THIS) OVERRIDE;
    STDMETHODIMP_(void) postServiceTask(THIS_ const IRunnable * task) OVERRIDE;
  private:
    void run();
    void DrainServiceQueue();
    int handler(lws *websocket, lws_callback_reasons reasons, void *id, void *data, size_t len);
    // join 工作线程; 若当前就在工作线程内(如监听器回调里调用了 quit),
    // 则 detach 以避免 std::terminate, 并等待 run() 收尾完成再返回, 防止 UAF。
    void joinOrDetachWorker();

    ISvrListener *m_pListener;

    std::condition_variable m_cvQuit;
    std::mutex m_mutex;
    bool m_finished;

    // run() 收尾同步: 当 quit() 因"从工作线程调用"而 detach 时, 工作线程仍在跑
    // run() 的收尾(含 lws_context_destroy), 析构/二次 quit 必须等其真正结束。
    std::mutex m_teardownMutex;
    std::condition_variable m_teardownCv;
    bool m_teardownDone = true;

    std::thread m_worker;

    std::string m_protocolName;
    SvrPingCfg m_cfg;

    lws_context *m_context;

    std::mutex m_serviceMutex;
    std::deque<SAutoRefPtr<IRunnable> > m_serviceQueue;

    static int cb_lws(lws *websocket, lws_callback_reasons reasons, void *userData, void *data, std::size_t len);

};
SNSEND
#endif // WsServer_H