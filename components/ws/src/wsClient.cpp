#include "wsClient.h"
#include <vector>

SNSBEGIN
WsClient::WsClient(IConnListener *pGroup)
    : m_pListener(pGroup)
    , m_msgId(0)
    , m_finished(true)
    , m_connected(false)
    , m_context(nullptr)
    , m_wsi(nullptr)
{

}

WsClient::~WsClient()
{
    quit();
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

int WsClient::connectTo(const char *server_, const char *path_, uint16_t port_, const char *protocolName_, ClientOption option)
{
    m_server = server_;
    m_path = path_;
    m_port = port_;
    m_protocolName = protocolName_;

    lws_protocols protocols[] = { { this->m_protocolName.c_str(), &WsClient::cb_lws, 0, kSocketBufSize, 0, nullptr, kSocketBufSize },
                                  {
                                      nullptr, nullptr, 0, 0, 0, nullptr, 0 // Quasi null terminator
                                  } };
    std::string ca;
    if (option.bSecure)
    {
#ifdef _WIN32
        if (!load_file(option.ca_u8, ca))
            return -2;
#endif
    }
    lws_context_creation_info contextCreationInfo = { 0 };
    contextCreationInfo.port = CONTEXT_PORT_NO_LISTEN;
    contextCreationInfo.protocols = protocols;
    contextCreationInfo.gid = -1;
    contextCreationInfo.uid = -1;
    contextCreationInfo.user = this;
#ifdef _WIN32
    contextCreationInfo.client_ssl_ca_mem = ca.c_str();
    contextCreationInfo.client_ssl_ca_mem_len = ca.length();
#else
    contextCreationInfo.client_ssl_ca_filepath = option.ca_u8;
#endif
    if (option.bSecure)
    {
        contextCreationInfo.options = LWS_SERVER_OPTION_DO_SSL_GLOBAL_INIT;
    }
    m_finished = false;
    this->m_context = lws_create_context(&contextCreationInfo);

    if (!this->m_context)
    {
        lwsl_err("%s: create lws context failed!\n", __func__);
        m_finished = true;
        return -1;
    }

    lws_client_connect_info clientConnectInfo = { 0 };
    clientConnectInfo.context = m_context;
    clientConnectInfo.port = this->m_port;
    clientConnectInfo.address = this->m_server.c_str();
    clientConnectInfo.path = this->m_path.c_str();
    clientConnectInfo.host = this->m_server.c_str();
    clientConnectInfo.origin = this->m_server.c_str();
    clientConnectInfo.protocol = this->m_protocolName.c_str();
    clientConnectInfo.local_protocol_name = this->m_protocolName.c_str();
    clientConnectInfo.pwsi = &this->m_wsi;

    if (option.bSecure)
    {
        clientConnectInfo.ssl_connection = LCCSCF_USE_SSL;
        if (option.allowSelfSigned)
            clientConnectInfo.ssl_connection |= LCCSCF_ALLOW_SELFSIGNED;
        if (option.skipServerCertHostnameCheck)
            clientConnectInfo.ssl_connection |= LCCSCF_SKIP_SERVER_CERT_HOSTNAME_CHECK;
        if (option.allowExpired)
            clientConnectInfo.ssl_connection |= LCCSCF_ALLOW_EXPIRED;
    }
    if (!lws_client_connect_via_info(&clientConnectInfo))
    {
        lwsl_err("%s: Could not connect!\n", __func__);
        m_finished = true;
        return -1;
    }
    this->m_worker = std::thread{ &WsClient::run, this };
    return 0;
}

void WsClient::quit()
{
    {
        std::lock_guard<std::mutex> lockGuard(m_mutex);
        if (m_finished) {
            if (this->m_worker.joinable())
                this->m_worker.detach();
            return;
        }
        else {
            this->m_finished = true;
            lws_cancel_service(m_context);
        }
    }
    this->m_worker.join();
}

BOOL WsClient::wait(int timeout)
{
    std::unique_lock<std::mutex> lock(m_mutex);
    auto waitPred = [&] { return m_finished.load(); };
    if (timeout < 0)
    {
        m_cvQuit.wait(lock, waitPred);
        return true;
    }
    else
        return m_cvQuit.wait_for(lock, std::chrono::milliseconds(timeout), waitPred);
}

int WsClient::send(const std::string &text, bool bBinary)
{
    std::lock_guard<std::mutex> lockGuard(m_mutex);
    if (this->m_finished)
    {
        return -1;
    }
    if (m_sendingBuf.empty())
        lws_callback_on_writable(m_wsi);
    MsgData msgData = { text, bBinary, genMsgId() };
    lwsl_notice("add msg to send queue msgid=%d, data=%s",msgData.msgId,msgData.buf.c_str());
    m_sendingBuf.push_back(msgData);
    lws_cancel_service(this->m_context);
    return msgData.msgId;
}

void WsClient::run()
{
    while (!m_finished)
    {
        lws_service(this->m_context, 100);
    }
    lws_context_destroy(m_context);
    m_context = nullptr;
    m_cvQuit.notify_all();
}

int WsClient::handler(lws_callback_reasons reasons, void *user, const void *data, std::size_t len)
{
    std::lock_guard<std::mutex> lockGuard(m_mutex);
    if (this->m_finished)
    {
        return -1;
    }
    lwsl_info("%s,reaon=%d\n", __func__, reasons);
    switch (reasons)
    {
    case LWS_CALLBACK_CLIENT_ESTABLISHED:
    {
        this->m_connected = true;
        if (m_pListener)
        {
            m_pListener->onConnected();
        }
    }
    break;
    case LWS_CALLBACK_CLIENT_RECEIVE:
    {
        std::size_t remaining = lws_remaining_packet_payload(this->m_wsi);
        bool isFinalFragment = !!lws_is_final_fragment(this->m_wsi);
        bool isBinary = lws_frame_is_binary(this->m_wsi);
        std::string recv((const char *)data, len);
        this->m_receiveStream << recv;

        lwsl_notice("remain=%d,final frag=%d,recv : %s",(int)remaining,isFinalFragment,recv.c_str());
        if (!remaining && isFinalFragment)
        {
            if (m_pListener)
            {
                m_pListener->onDataRecv(m_receiveStream.str().c_str(), m_receiveStream.str().length(), isBinary);
            }
            m_receiveStream.str(std::string{});
        }
    }
    break;
    case LWS_CALLBACK_CLIENT_WRITEABLE:
    {
        while (!m_sendingBuf.empty())
        {
            MsgData msgData = m_sendingBuf.front();
            m_sendingBuf.pop_front();
            lwsl_notice("call lws_write for msgid=%d, data=%s",msgData.msgId,msgData.buf.c_str());
            std::vector<unsigned char> buf;
            buf.resize(msgData.buf.length() + LWS_PRE);
            memcpy(buf.data() + LWS_PRE, msgData.buf.data(), msgData.buf.length());
            lws_write(m_wsi, buf.data() + LWS_PRE, msgData.buf.length(), msgData.bBinary ? LWS_WRITE_BINARY : LWS_WRITE_TEXT);
            if (m_pListener)
            {
                m_pListener->onDataSent(msgData.msgId);
            }
            if (lws_send_pipe_choked(m_wsi))
            {
                lws_callback_on_writable(m_wsi);
                break;
            }
        };
    }
    break;
    case LWS_CALLBACK_CLIENT_CONNECTION_ERROR:
    {
        const char *err = (char *)data;
        this->m_connected = false;
        if (m_pListener)
        {
            m_pListener->onConnError(err);
        }
        lws_cancel_service(m_context);
        this->m_finished = true;
    }
    break;
    case LWS_CALLBACK_CLIENT_CLOSED:
    {
        this->m_connected = false;
        if (m_pListener)
        {
            m_pListener->onDisconnect();
        }
        lws_cancel_service(m_context);
        this->m_finished = true;
    }
    break;
    default:
        break;
    }
    return 0;
}

int WsClient::cb_lws(lws *websocket, lws_callback_reasons reasons, void *user, void *data, std::size_t len)
{

    lws_context *ctx = lws_get_context(websocket);
    WsClient *_this = (WsClient *)lws_context_user(ctx);
    return _this->handler(reasons, user, data, len);
}

void WsClient::disconnect()
{
    quit();
}

SNSEND