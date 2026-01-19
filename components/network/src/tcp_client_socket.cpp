#include "tcp_client_socket.h"
#include <cassert>

#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#else
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <sys/ioctl.h>
#include <errno.h>
#endif
#include <helper/slog.h>
#define kLogTag "Socket"


#define CHECK_SOCKET_OR_RETURN(socket,ret) \
    if (_socket == INVALID_SOCKET) { \
        SLOGE()<<"socket not created, operation failed"; \
        return ret;\
    }

#define CHECK_CONNECT_OR_RETURN(ip,port,ret) \
    if (ip == "" || port == 0) { \
        SLOGE()<<"socket not connected, operation failed"; \
        return ret;\
    }

SNSBEGIN


TcpClientSocket::TcpClientSocket() :
    Socket(),
    TObjRefImpl<ITcpClientSocket>(),
    _remoteIP(),
    _remotePort(0),
    _isBlocking(true)
{
}

TcpClientSocket::TcpClientSocket(int sock,
                           const std::string &remoteIP,
                           uint16_t remotePort) :
    Socket(sock),
    TObjRefImpl<ITcpClientSocket>(),
    _remoteIP(remoteIP),
    _remotePort(remotePort)
{
}

TcpClientSocket::TcpClientSocket(const TcpClientSocket &socket) :
    Socket(socket),
    TObjRefImpl<ITcpClientSocket>(),
    _remoteIP(socket._remoteIP),
    _remotePort(socket._remotePort)
{
}

TcpClientSocket::~TcpClientSocket()
{
}

std::string TcpClientSocket::getRemoteIP() const
{
    return _remoteIP;
}

uint16_t TcpClientSocket::getRemotePort() const
{
    return _remotePort;
}

int TcpClientSocket::setBlocking(bool blocking)
{
    int nonblocking = blocking ? 0 : 1;
    //TODO by zijwu, we need call ioctlsocket in windows platform.
#ifdef WIN32
    int ret = ioctlsocket(_socket, FIONBIO, (u_long *)&nonblocking);
#else
    int ret = ioctl(_socket, FIONBIO, &nonblocking);
#endif

    if (ret == 0)
    {
        _isBlocking = blocking;
    }

    SLOG_FMT(kLogTag, LOG_INFO,
                     "socket set blocking %d, result=%d",
                     blocking, ret);

    return ret;
}

bool TcpClientSocket::create(const std::string &ip, uint16_t port)
{
    if (_socket >= 0)
    {
        SLOG_FMT(kLogTag,LOG_WARN,
                         "socket is already created: ip=%s, port=%d",
                         this->address().c_str(),
                         this->port());
        return false;
    }

    _socket = createSocket(ip, port, true, false);
    return (_socket != INVALID_SOCKET);
}

bool TcpClientSocket::create(bool isIpV4)
{
    if (_socket >= 0)
    {
        SLOG_FMT(kLogTag,LOG_WARN,
                         "create socket failed, because exists ip=%s, port=%d",
                         this->address().c_str(),
                         this->port());
        return false;
    }

    _socket = createSocket(isIpV4, true, false);
    return (_socket != INVALID_SOCKET);
}


bool TcpClientSocket::connect(const std::string &ip, uint16_t port)
{
    CHECK_SOCKET_OR_RETURN(_socket, false);

    if (_remoteIP != "" || _remotePort != 0)
    {
        SLOG_FMT(kLogTag,LOG_WARN,
                         "connect socket failed, because exists connected to ip=%s, port=%d",
                         _remoteIP.c_str(),
                         _remotePort);
        return false;
    }

    sockaddr_in address;

    if (!fillSockAddr(ip, port, &address))
    {
        SLOG_FMT(kLogTag,LOG_ERROR,
                         "invalid address, ip=%s, port=%d",
                         ip.c_str(),
                         port);
        return false;
    }

    if (_isBlocking)
    {
        int ret = ::connect(_socket, (sockaddr *)&address, sizeof(address));

        if (ret != 0)
        {
            SLOG_FMT(kLogTag,LOG_ERROR,
                             "blocking remote connect failed, socket=%d,  ip=%s, port=%d, error=%d",
                             _socket,
                             ip.c_str(),
                             port,
                             getError());
            return false;
        }
    }
    else
    {
        ::connect(_socket, (sockaddr *)&address, sizeof(address));

        if (selectWrite() != RETURN_OK)
        {
            SLOG_FMT(kLogTag,LOG_ERROR,
                             "none-blocking remote connect failed, socket=%d, ip=%s, port=%d, error=%d",
                             _socket,
                             ip.c_str(),
                             port,
                             getError());
            return false;
        }
    }


    _remoteIP = ip;
    _remotePort = port;
    return true;
}

bool TcpClientSocket::connect(const std::string &ip, uint16_t port, uint32_t timeout)
{
    CHECK_SOCKET_OR_RETURN(_socket, false);

    if (_remoteIP != "" || _remotePort != 0)
    {
        SLOG_FMT(kLogTag,LOG_WARN,
                         "connect socket failed, because exists connected to ip=%s, port=%d",
                         _remoteIP.c_str(),
                         _remotePort);
        return false;
    }

    sockaddr_in address;

    if (!fillSockAddr(ip, port, &address))
    {
        SLOG_FMT(kLogTag,LOG_ERROR,
                         "invalid address, ip=%s, port=%d",
                         ip.c_str(),
                         port);
        return false;
    }

    bool isBlocking = _isBlocking;
    bool ret = false;

    if (isBlocking)
    {
        setBlocking(false);
    }

    ::connect(_socket, (sockaddr *)&address, sizeof(address));

    if (selectWrite(timeout) == RETURN_OK)
    {
        _remoteIP = ip;
        _remotePort = port;
        ret = true;
    }
    else
    {
        SLOG_FMT(kLogTag,LOG_ERROR,
                         "none-blocking remote connect failed, socket=%d, ip=%s, port=%d, error=%d",
                         _socket,
                         ip.c_str(),
                         port,
                         getError());
    }

    if (isBlocking != _isBlocking)
    {
        setBlocking(isBlocking);
    }

    return ret;
}

bool TcpClientSocket::connectAsync(const std::string &ip, uint16_t port)
{
    CHECK_SOCKET_OR_RETURN(_socket, false);

    assert(_isBlocking == false);

    if (_remoteIP != "" || _remotePort != 0)
    {
        SLOG_FMT(kLogTag,LOG_WARN,
                         "connect async socket failed, because exists connected to ip=%s, port=%d",
                         _remoteIP.c_str(),
                         _remotePort);
        return false;
    }

    sockaddr_in address;

    if (!fillSockAddr(ip, port, &address))
    {
        SLOG_FMT(kLogTag,LOG_ERROR,
                         "connect async socket failed, because invalid address ip=%s, port=%d",
                         ip.c_str(),
                         port);
        return false;
    }

    ::connect(_socket, (sockaddr *)&address, sizeof(address));

    _remoteIP = ip;
    _remotePort = port;
    return true;
}

bool TcpClientSocket::isConnected() const
{
    if (const_cast<TcpClientSocket*>(this)->selectWrite(0) == RETURN_OK)
    {
        return true;
    }
    return false;
}

bool TcpClientSocket::send(const void *data,
                        unsigned int length,
                        unsigned int *sentlen) const
{
    CHECK_SOCKET_OR_RETURN(_socket, false);
    CHECK_CONNECT_OR_RETURN(_remoteIP, _remotePort, false);

    assert(data);
#if defined(__linux__) || defined (ANDROID)
    int sent = static_cast<int> (::send(_socket, (const char *)data, length, MSG_NOSIGNAL));
#else
    int sent = static_cast<int> (::send(_socket, (const char *)data, length, 0));
#endif

    if (sent == -1)
    {
        SLOG_FMT(kLogTag,LOG_ERROR,
                         "send data failed, socket=%d, error=%d",
                         _socket,
                         getError());
        return false;
    }

    if (sentlen)
    {
        *sentlen = sent;
    }

    return true;
}

bool TcpClientSocket::recv(void *data, unsigned int &length) const
{
    CHECK_SOCKET_OR_RETURN(_socket, false);
    CHECK_CONNECT_OR_RETURN(_remoteIP, _remotePort, false);

    assert(data);

    int recv = static_cast<int> (::recv(_socket, ( char *)data, length, 0));

    if (recv == -1)
    {
        SLOG_FMT(kLogTag,LOG_ERROR,
                         "receive data failed, socket=%d, error=%d",
                         _socket,
                         getError());
        return false;
    }

    if (recv == 0)
    {
        // gracefully close
        SLOG_FMT(kLogTag,LOG_INFO,
                         "socket closed when receive close message, socket=%d",
                         _socket);
        return false;
    }

    length = recv;
    return true;
}

// ITcpClientSocket interface implementation
STDMETHODIMP_(bool) TcpClientSocket::Create(THIS_ const char *ip, uint16_t port)
{
    std::string strIp(ip);
    return create(strIp, port);
}

STDMETHODIMP_(bool) TcpClientSocket::CreateIpV4(THIS)
{
    return create(true);
}

STDMETHODIMP_(bool) TcpClientSocket::Connect(THIS_ const char *ip, uint16_t port)
{
    std::string strIp(ip);
    return connect(strIp, port);
}

STDMETHODIMP_(bool) TcpClientSocket::ConnectTimeout(THIS_ const char *ip, uint16_t port, uint32_t timeout)
{
    std::string strIp(ip);
    return connect(strIp, port, timeout);
}

STDMETHODIMP_(bool) TcpClientSocket::ConnectAsync(THIS_ const char *ip, uint16_t port)
{
    std::string strIp(ip);
    return connectAsync(strIp, port);
}

STDMETHODIMP_(bool) TcpClientSocket::IsConnected(CTHIS) SCONST
{
    return isConnected();
}

STDMETHODIMP_(bool) TcpClientSocket::Send(THIS_ const void *data, unsigned int length, unsigned int *sentlen)
{
    return send(data, length, sentlen);
}

STDMETHODIMP_(bool) TcpClientSocket::Recv(THIS_ void *data, unsigned int &length)
{
    return recv(data, length);
}

STDMETHODIMP_(int) TcpClientSocket::SetBlocking(THIS_ bool blocking)
{
    return setBlocking(blocking);
}

STDMETHODIMP_(void) TcpClientSocket::Close(THIS)
{
    Socket::close();
    _remoteIP = "";
    _remotePort = 0;
}

STDMETHODIMP_(const char *) TcpClientSocket::GetRemoteIP(CTHIS) SCONST
{
    return _remoteIP.c_str();
}

STDMETHODIMP_(uint16_t) TcpClientSocket::GetRemotePort(CTHIS) SCONST
{
    return _remotePort;
}

SNSEND
