#include "wsServer.h"
#include <helper/slog.h>
#include <helper/SFunctor.hpp>
#define kLogTag "WsServer"

SNSBEGIN
WsServer::WsServer(ISvrListener *pListener)
    : m_pListener(pListener)
    , m_finished(true)
    , m_context(nullptr)
{
}

#ifdef _WIN32
static bool load_file(const char *pszPath_u8, std::string &buf)
{
    wchar_t path[1024] = { 0 };
    MultiByteToWideChar(CP_UTF8, 0, pszPath_u8, -1, path, 1024);
    FILE *f = _wfopen(path, L"r");
    if (!f)
        return false;
    fseek(f, 0, SEEK_END);
    int len = ftell(f);
    fseek(f, 0, SEEK_SET);
    buf.resize(len);
    len = fread((void *)buf.data(), 1, len, f);
    buf.resize(len);
    fclose(f);
    return true;
}
#endif
int WsServer::start(uint16_t port, const char *protocolName_, SvrOption option, SvrPingCfg pingCfg)
{
    m_protocolName = protocolName_;
    m_cfg = pingCfg;
    if(m_cfg.nHeartbeatSeconds<5)
        m_cfg.nHeartbeatSeconds = 5;
    if(m_cfg.nPingTimeoutCount<2)
        m_cfg.nPingTimeoutCount = 2;
    if(m_cfg.pingIntervalSeconds>m_cfg.nHeartbeatSeconds/2){
        m_cfg.pingIntervalSeconds = m_cfg.nHeartbeatSeconds/2;
    }
    lws_protocols protocols[] = { { m_protocolName.c_str(), &WsServer::cb_lws, sizeof(void*), kSocketBufSize, 0, nullptr, kSocketBufSize },
                                  {
                                      nullptr, nullptr, 0, 0, 0, nullptr, 0 // Quasi null terminator
                                  } };
    lws_context_creation_info contextCreationInfo = { 0 };
    contextCreationInfo.port = port;
    contextCreationInfo.protocols = protocols;
    contextCreationInfo.gid = -1;
    contextCreationInfo.uid = -1;

    contextCreationInfo.options = LWS_SERVER_OPTION_HTTP_HEADERS_SECURITY_BEST_PRACTICES_ENFORCE;

    std::string cert, priv_key;
    if (option.bSecure)
    {
        if (!option.cert_u8 || !option.priv_key_u8)
            return -2;
        contextCreationInfo.options |= LWS_SERVER_OPTION_DO_SSL_GLOBAL_INIT;
#ifdef _WIN32
        if (!load_file(option.cert_u8, cert))
            return -3;
        if (!load_file(option.priv_key_u8, priv_key))
            return -4;
        contextCreationInfo.server_ssl_cert_mem = cert.data();
        contextCreationInfo.server_ssl_cert_mem_len = cert.length();
        contextCreationInfo.server_ssl_private_key_mem = priv_key.data();
        contextCreationInfo.server_ssl_private_key_mem_len = priv_key.length();
#else
        contextCreationInfo.ssl_cert_filepath = option.cert_u8;
        contextCreationInfo.ssl_private_key_filepath = option.priv_key_u8;
#endif
    }

    contextCreationInfo.user = this;
    m_finished = false;
    this->m_context = lws_create_context(&contextCreationInfo);

    if (!this->m_context)
    {
        m_finished = true;
        lwsl_err("%s: Could not initialize websocket\n", __func__);
        return -1;
    }
    this->m_worker = std::thread(&WsServer::run, this);
    return 0;
}

void WsServer::run()
{
    while (!m_finished)
    {
        lws_service(m_context, 50);
        DrainServiceQueue();
    }
}

void WsServer::DrainServiceQueue()
{
    // 服务队列在 LWS 事件线程(LWS线程)上排空, 确保回调与其他游戏消息串行执行
    std::deque<SAutoRefPtr<IRunnable> > tasks;
    {
        std::lock_guard<std::mutex> lock(m_serviceMutex);
        tasks.swap(m_serviceQueue);
    }
    for (size_t i = 0; i < tasks.size(); ++i)
        tasks[i]->run();
}

void WsServer::postServiceTask(IRunnable * task)
{
    // clone 使调用方栈上的 IRunnable 可在返回后安全销毁(与 ITaskLoop::postTask 一致)
    SAutoRefPtr<IRunnable> pClone;
    pClone.Attach(task->clone());
    bool bWake = false;
    {
        std::lock_guard<std::mutex> lock(m_serviceMutex);
        // 仅当队列"从空变非空"时才唤醒, 避免打断已在进行的中断唤醒
        bWake = m_serviceQueue.empty();
        m_serviceQueue.push_back(pClone);
    }
    if (bWake)
    {
        // 关键修复: LWS 事件线程当前正阻塞在 lws_service/poll 等待网络事件,
        // 单纯入队不会打断该阻塞。若这里不唤醒, 服务任务(如机器人AI结果)会一直被
        // 拖延到下一次网络事件才在 run() 中被 DrainServiceQueue 执行,
        // 导致"搜索已完成(0ms)但结果迟迟不落子"的严重延迟(实测可达20s+)。
        // lws_cancel_service 是该场景的标准跨线程唤醒机制(quit() 亦用它退出阻塞),
        // 会令 poll 立即返回, run() 随即循环回来执行 DrainServiceQueue。
        lws_context *ctx = m_context;
        if (ctx)
            lws_cancel_service(ctx);
    }
}

static void lws_send_ping(struct lws *wsi) {
    unsigned char ping[LWS_PRE + 125];
    int len = lws_snprintf((char *)ping + LWS_PRE, 125, "ping data");
    lws_write(wsi, ping + LWS_PRE, len, LWS_WRITE_PING);
}

int WsServer::handler(lws *websocket, lws_callback_reasons reasons,
    void *userData, void *data, size_t len)
{
    std::unique_lock<std::mutex> lock(m_mutex);
    // After quit() the lws_context_destroy below still fires LWS_CALLBACK_CLOSED
    // for every live connection: it must NOT be short-circuited, otherwise each
    // SvrConnection leaks and the listener never sees onDisconnect.
    if (m_finished && reasons != LWS_CALLBACK_CLOSED)
        return -1;
    int ret = 0;
    switch (reasons)
    {
    case LWS_CALLBACK_TIMER:
        {
            SvrConnection* conn = *(SvrConnection**)userData;
            if (!conn) {
                // 非连接 wsi(如 vhost 的 wsi): 无心跳可维护
                break;
            }
            time_t now = time(NULL);

            // 1. 检查心跳超时（基于最后活动时间）
            // 如果客户端在nHeartbeatSeconds时间内没有任何活动（包括发送数据和pong），则关闭连接
            if (now - conn->last_activity > m_cfg.nHeartbeatSeconds) {
                SLOGI() << "heartbeat timeout, close connection! timeout=" << m_cfg.nHeartbeatSeconds
                       << "s, inactive=" << (now - conn->last_activity) << "s, conn=" << conn;
                lws_close_reason(websocket, LWS_CLOSE_STATUS_GOINGAWAY,
                                (unsigned char *)"heartbeat timeout", 17);
                return -1;  // 关闭连接，不需要重设定时器
            }

            // 2. 检查是否需要发送ping
            // 只有在客户端空闲（距离上次活动超过ping间隔）时才需要发送ping来检测连接
            // 如果客户端一直有活动，说明连接正常，不需要发送ping
            if (now - conn->last_activity >= m_cfg.pingIntervalSeconds) {
                // 检查是否到了发送ping的时间
                if (now - conn->last_ping >= m_cfg.pingIntervalSeconds) {
                    // 检查上一次ping是否超时（发送ping后一直没收到pong）
                    if (conn->ping_timeout_count >= m_cfg.nPingTimeoutCount) {
                        SLOGI() << "ping timeout count exceeded, close connection! count="
                               << conn->ping_timeout_count << ", max=" << m_cfg.nPingTimeoutCount
                               << ", conn=" << conn;
                        lws_close_reason(websocket, LWS_CLOSE_STATUS_POLICY_VIOLATION,
                                        (unsigned char *)"ping timeout", 12);
                        return -1;  // 关闭连接
                    }

                    // request a ping: the actual write must happen in the
                    // WRITEABLE callback (lws only allows writes there)
                    conn->bPingPending = true;
                    lws_callback_on_writable(websocket);
                }
            }

            // 3. 重设定时器
            // 使用ping间隔和心跳超时中较小的值作为定时器间隔
            // 这样可以确保及时发送ping和检查心跳超时
            uint32_t timer_interval = smin(m_cfg.pingIntervalSeconds, m_cfg.nHeartbeatSeconds);
            if (timer_interval < 1) {
                timer_interval = 1;  // 最小1秒
            }
            lws_set_timer_usecs(websocket, timer_interval * LWS_USEC_PER_SEC);
        }
    break;
    case LWS_CALLBACK_RECEIVE_PONG:
    {
        SvrConnection* conn = *(SvrConnection**)userData;
        if (conn) {
            conn->ping_timeout_count = 0;  // 收到pong，清零超时计数器
            conn->last_activity = time(NULL);  // 更新最后活动时间
            SLOGI() << "recv pong, reset ping timeout counter, conn=" << conn;
        } else {
            lwsl_err("invalid connection in RECEIVE_PONG callback");
        }
    }
    break;
    case LWS_CALLBACK_ESTABLISHED:
    {
        static const int kMaxArgs = 1024, kMaxPath = 100;
        char uriArgs[kMaxArgs] = { 0 }, uriPath[kMaxPath] = { 0 };
        lws_hdr_copy(websocket, uriArgs, kMaxArgs, WSI_TOKEN_HTTP_URI_ARGS);
        lws_hdr_copy(websocket, uriPath, kMaxPath, WSI_TOKEN_GET_URI);

        if(m_pListener){
            SvrConnection *connection  = new SvrConnection(m_context, websocket, m_pListener);
            lock_guard_rev rev(m_mutex);
            if (!m_pListener->onConnected(connection, uriPath, uriArgs))
            {
                lwsl_info("invalid args");
                lws_close_reason(websocket, LWS_CLOSE_STATUS_PROTOCOL_ERR, NULL, 0);
                connection->Release();
                *(SvrConnection **)userData = nullptr;
                ret = -1;
            }
            else
            {
                *(SvrConnection **)userData = (SvrConnection *)connection;
                connection->last_activity = time(NULL);
                // 心跳数据已在SvrConnection构造函数中初始化
                // 设置定时器
                uint32_t timer_interval = smin(m_cfg.pingIntervalSeconds, m_cfg.nHeartbeatSeconds);
                if (timer_interval < 1) {
                    timer_interval = 1;
                }
                lws_set_timer_usecs(websocket, timer_interval * LWS_USEC_PER_SEC);
                SLOGI() << "connection established, timer_interval=" << timer_interval << "s, conn=" << connection;
            }
        }
        break;
    }
    case LWS_CALLBACK_CLOSED:
    {
        SvrConnection*conn = *(SvrConnection**)userData;
        if (!conn)
            break;
        {
            // invalidate the socket BEFORE the wsi is recycled, so isValid()
            // returns 0 and send() fails fast instead of touching a dangling lws*
            std::lock_guard<std::mutex> connLock(conn->m_mutex);
            conn->m_socket = nullptr;
        }
        if (m_pListener)
        {
            SLOGI() << "connection closed, conn=" << conn;
            lock_guard_rev rev(m_mutex);
            m_pListener->onDisconnect(conn);
        }
        conn->Release();
        break;
    }
    case LWS_CALLBACK_RECEIVE:
    {
        SvrConnection* conn = *(SvrConnection**)userData;
        if (!conn)
            break;
        conn->last_activity = time(NULL);
        conn->ping_timeout_count = 0; // 重置计数器
        const std::size_t remaining = lws_remaining_packet_payload(websocket);
        const bool isFinalFragment = lws_is_final_fragment(websocket);
        const bool isBinary = lws_frame_is_binary(websocket);
        lock_guard_rev rev(m_mutex);
        conn->onRecv(std::string((const char*)data, len), !remaining && isFinalFragment, isBinary);
    }
    break;
    case LWS_CALLBACK_SERVER_WRITEABLE:
    {
        SvrConnection* conn = *(SvrConnection**)userData;
        if (conn)
        {
            lock_guard_rev rev(m_mutex);
            conn->sendBuf();
            std::lock_guard<std::mutex> connLock(conn->m_mutex);
            if (conn->bWantClose)
            {
                lws_close_reason(websocket, LWS_CLOSE_STATUS_UNEXPECTED_CONDITION, NULL, 0);
                ret = -1; // kill the connection after a write error
            }
            else if (conn->bPingPending && conn->sendingBuf.empty())
            {
                // send the ping requested by the TIMER here, where lws allows writes
                conn->bPingPending = false;
                conn->last_ping = time(NULL);
                conn->ping_timeout_count++; // assume timeout, reset on pong
                lws_send_ping(websocket);
                SLOGI() << "send ping, count=" << conn->ping_timeout_count << ", conn=" << conn;
            }
        }
    }
    break;
    default:
        break;
    }

    return ret;
}

WsServer::~WsServer()
{
    quit();
}

BOOL WsServer::wait(int timeout)
{
    std::unique_lock<std::mutex> lock(m_mutex);
    auto waitPred = [&] { return m_finished; };
    if (timeout < 0)
    {
        m_cvQuit.wait(lock, waitPred);
        return true;
    }
    else
        return m_cvQuit.wait_for(lock, std::chrono::milliseconds(timeout), waitPred);
}

void WsServer::quit()
{
    {
        std::unique_lock<std::mutex> lock(m_mutex);
        if (this->m_finished)
            return;
        this->m_finished = true;
    }
    lws_cancel_service(m_context);
    this->m_worker.join();
    lws_context_destroy(m_context);
    m_context = nullptr;
    {
        // LWS 事件线程已退出, 队列不会再被排空: 丢弃可能残留的定时器任务
        std::lock_guard<std::mutex> lock(m_serviceMutex);
        m_serviceQueue.clear();
    }
    m_cvQuit.notify_all();
}

int WsServer::cb_lws(lws *websocket, lws_callback_reasons reasons, void *userData, void *data, size_t len)
{
    lws_context *ctx = lws_get_context(websocket);
    WsServer *_this = (WsServer *)lws_context_user(ctx);
    return _this->handler(websocket, reasons, userData, data, len);
}

SNSEND