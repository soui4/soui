#include "wsServer.h"
#include <helper/slog.h>
#define kLogTag "WsServer"

SNSBEGIN
WsServer::WsServer(ISvrListener *pListener)
    : m_pListener(pListener)
    , m_context(nullptr)
    , m_finished(true)
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
    if (m_finished)
        return -1;
    int ret = 0;
    switch (reasons)
    {
    case LWS_CALLBACK_TIMER:
        {
            SvrConnection* conn = *(SvrConnection**)userData;
            if (!conn) {
                lwsl_err("invalid connection in TIMER callback");
                break;  // 使用break而不是return -1，避免影响其他连接
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

                    // 发送ping
                    lws_send_ping(websocket);
                    conn->last_ping = now;
                    conn->ping_timeout_count++;  // 假设会超时，收到pong时会清零
                    SLOGI() << "send ping, count=" << conn->ping_timeout_count << ", conn=" << conn;
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
        if (m_pListener)
        {
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
    m_cvQuit.notify_all();
}

int WsServer::cb_lws(lws *websocket, lws_callback_reasons reasons, void *userData, void *data, size_t len)
{
    lws_context *ctx = lws_get_context(websocket);
    WsServer *_this = (WsServer *)lws_context_user(ctx);
    return _this->handler(websocket, reasons, userData, data, len);
}

SNSEND