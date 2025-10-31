#include "wsServer.h"

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
int WsServer::start(uint16_t port, const char *protocolName_, SvrOption option)
{
    m_protocolName = protocolName_;

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
    this->m_worker = std::thread{ &WsServer::run, this };
    return 0;
}

void WsServer::run()
{
    while (!m_finished)
    {
        lws_service(m_context, 50);
    }
}

int WsServer::handler(lws *websocket, lws_callback_reasons reasons, void *userData, void *data, size_t len)
{
    std::unique_lock<std::mutex> lock(m_mutex);
    if (m_finished)
        return -1;
    int ret = 0;
    switch (reasons)
    {
    case LWS_CALLBACK_ESTABLISHED:
    {
        static const int kMaxArgs = 1024, kMaxPath = 100;
        char uriArgs[kMaxArgs] = { 0 }, uriPath[kMaxPath] = { 0 };
        lws_hdr_copy(websocket, uriArgs, kMaxArgs, WSI_TOKEN_HTTP_URI_ARGS);
        lws_hdr_copy(websocket, uriPath, kMaxPath, WSI_TOKEN_GET_URI);

        SvrConnection *connection  = new SvrConnection(m_context, websocket, m_pListener);
        if (!m_pListener->onConnected(connection, uriPath, uriArgs)) {
            lwsl_info("invalid args");
            lws_close_reason(websocket, LWS_CLOSE_STATUS_PROTOCOL_ERR, NULL, 0);
            connection->Release();
            *(SvrConnection**)userData = nullptr;
            ret = -1;
        }
        else {
            *(SvrConnection**)userData = (SvrConnection*)connection;
        }
        break;
    }
    case LWS_CALLBACK_CLOSED:
    {
        SvrConnection*conn = *(SvrConnection**)userData;
        if (!conn)
            break;
        m_pListener->onDisconnect(conn); 
        conn->Release();
        break;
    }
    case LWS_CALLBACK_RECEIVE:
    {
        SvrConnection* conn = *(SvrConnection**)userData;
        if (!conn)
            break;
        const std::size_t remaining = lws_remaining_packet_payload(websocket);
        const bool isFinalFragment = lws_is_final_fragment(websocket);
        const bool isBinary = lws_frame_is_binary(websocket);
        conn->onRecv(std::string((const char*)data, len), !remaining && isFinalFragment, isBinary);
    }
    break;
    case LWS_CALLBACK_SERVER_WRITEABLE:
    {
        SvrConnection* conn = *(SvrConnection**)userData;
        if (conn)
        {
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