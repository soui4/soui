#include "udp_socket.h"
#include <cassert>
#include <cstring>

#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#else
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <sys/ioctl.h>
#endif
#include <helper/slog.h>
#define kLogTag "UDPSocket"


#define CHECK_SOCKET_OR_RETURN(socket,ret) \
    if (_socket == INVALID_SOCKET) { \
        SLOGE()<<"socket not created, udp operation failed"; \
        return ret;\
    }

SNSBEGIN


UDPSocket::UDPSocket() :
    Socket(),
    TObjRefImpl<IUdpSocket>(),
    _continueRecvError(0),
    _continueSendError(0),
    _connected(false),
    _remotePort(0)
{
}

UDPSocket::~UDPSocket()
{
}

bool UDPSocket::create(const std::string &ip, uint16_t port)
{
    if (_socket >= 0)
    {
        SLOG_FMT(kLogTag,LOG_WARN,
                         "create udp socket failed, because exists ip=%s, port=%d",
                         this->address().c_str(),
                         this->port());
        return false;
    }

    _socket = createSocket(ip, port, false, false);

    SLOG_FMT(kLogTag,LOG_DEBUG,
                     "create udp socket success, use ip=%s, port=%d, result=%d, socket=%d",
                     this->address().c_str(),
                     this->port(),
                     _socket != INVALID_SOCKET,
                     _socket);

    return (_socket != INVALID_SOCKET);
}

bool UDPSocket::sendto(const std::string &ip,
                       uint16_t port,
                       const void *data,
                       unsigned int length,
                       int *sentlen)
{
    if (_connected) {
        return this->sendto(data, length, sentlen);
    }

    CHECK_SOCKET_OR_RETURN(_socket, false);

    assert(data);

    // we using v6 structure to make sure it is larger enough.
    sockaddr_in6 address;

    if (!fillSockAddr(ip, port, &address))
    {
        SLOG_FMT(kLogTag,LOG_ERROR,
                         "send udp data failed, invalid address, ip=%s, port=%d",
                         ip.c_str(),
                         port);
        return false;
    }

    int sent = static_cast<int>(::sendto(_socket,
                                         (const char *)data,
                                         length,
                                         0,
                                         (sockaddr *)&address,
                                         sizeof(address)));

    if (sent == -1)
    {
        if (_continueSendError++ < 5)
        {
            SLOG_FMT(kLogTag,LOG_ERROR,
                             "send udp data failed, socket=%d, data=%p, length=%u, local ip=%s, local port=%u, remote ip=%s, remote port=%u, error=%d",
                             _socket,
                             data,
                             length,
                             this->address().c_str(),
                             this->port(),
                             ip.c_str(),
                             port,
                             getError());
        }


        if (sentlen)
        {
            *sentlen = 0;
        }

        return false;
    }

    _continueSendError = 0;

    if (sentlen)
    {
        *sentlen = sent;
    }

    return true;
}

bool UDPSocket::sendto(const sockaddr_in6 &address,
                       const void *data,
                       unsigned int length,
                       unsigned int *sentlen)
{
    CHECK_SOCKET_OR_RETURN(_socket, false);

    assert(data);

    int sent = static_cast<int>(::sendto(_socket,
                                         (const char *)data,
                                         length,
                                         0,
                                         (sockaddr *)&address,
                                         sizeof(address)));

    if (sent == -1)
    {
        if (_continueSendError++ < 5)
        {
            SLOG_FMT(kLogTag,LOG_ERROR,
                             "send udp data failed, socket=%d, data=%p, length=%u, local ip=%s, local port=%u, error=%d",
                             _socket,
                             data,
                             length,
                             this->address().c_str(),
                             this->port(),
                             getError());
        }


        if (sentlen)
        {
            *sentlen = 0;
        }

        return false;
    }

    _continueSendError = 0;

    if (sentlen)
    {
        *sentlen = sent;
    }

    return true;
}

bool UDPSocket::recvfrom(std::string &ip,
                         uint16_t &port,
                         void *data,
                         unsigned int &length)
{
    CHECK_SOCKET_OR_RETURN(_socket, false);

    assert(data);
    assert(length > 0);

    if (_connected) {
        int recv = static_cast<int>(::recv(_socket, (char *)data, length, 0));
        if (recv == -1) {
            if (_continueRecvError++ < 20) {
                SLOG_FMT(kLogTag,LOG_ERROR,
                                "receive udp data failed, socket=%d, error=%d",
                                _socket,
                                getError());
            }
            return false;
        }

        if (recv == 0) {
            // gracefully close
            if (_continueRecvError++ < 20) {
                SLOG_FMT(kLogTag,LOG_INFO,
                                "20 consecutive error data received, close socket=%d",
                                _socket);
            }
            return false;
        }

        _continueRecvError = 0;

        ip = _remoteIP;
        port = _remotePort;

        length = recv;

        return true;
    } else {
        sockaddr_in6 address = {0};
        socklen_t addressSize = sizeof(address);
        int recv = static_cast<int>(::recvfrom(_socket,
                                            (char *)data,
                                            length,
                                            0,
                                            (sockaddr *)&address,
                                            &addressSize));

        if (recv == -1)
        {
            if (_continueRecvError++ < 20)
            {
                SLOG_FMT(kLogTag,LOG_ERROR,
                                "receive udp data failed, socket=%d, error=%d",
                                _socket,
                                getError());
            }

            return false;
        }

        if (recv == 0)
        {
            // gracefully close
            if (_continueRecvError++ < 20)
            {
                SLOG_FMT(kLogTag,LOG_INFO,
                                "20 consecutive error data received, close socket=%d",
                                _socket);
            }

            return false;
        }

        _continueRecvError = 0;

        char ipStr[256];

        if (((sockaddr *)&address)->sa_family == AF_INET6)
        {
    #ifdef WIN32
            inet_ntop_win(address.sin6_family, &address.sin6_addr, ipStr, sizeof(ipStr));
    #else
            inet_ntop(address.sin6_family, &address.sin6_addr, ipStr, sizeof(ipStr));
    #endif
            port = ntohs(address.sin6_port);
        }
        else
        {
            sockaddr_in *addr_in = (sockaddr_in *)&address;
    #ifdef WIN32
            inet_ntop_win(addr_in->sin_family, &addr_in->sin_addr, ipStr, sizeof(ipStr));
    #else
            inet_ntop(addr_in->sin_family, &addr_in->sin_addr, ipStr, sizeof(ipStr));
    #endif
            port = ntohs(addr_in->sin_port);
        }

        ip = std::string(ipStr);

        length = recv;
        return true;
    }
}

bool UDPSocket::recvfrom(sockaddr_in6 &address,
                         void *data,
                         unsigned int &length)
{
    CHECK_SOCKET_OR_RETURN(_socket, false);

    assert(data);
    assert(length > 0);

    memset(&address, 0, sizeof(sockaddr_in6));

    socklen_t addressSize = sizeof(address);
    int recv = static_cast<int>(::recvfrom(_socket,
                                           (char *)data,
                                           length,
                                           0,
                                           (sockaddr *)&address,
                                           &addressSize));

    if (recv == -1)
    {
        if (_continueRecvError++ < 20)
        {
            SLOG_FMT(kLogTag,LOG_ERROR,
                             "receive udp data failed, socket=%d, error=%d",
                             _socket,
                             getError());
        }

        return false;
    }

    if (recv == 0)
    {
        // gracefully close
        if (_continueRecvError++ < 20)
        {
            SLOG_FMT(kLogTag,LOG_INFO,
                             "20 consecutive error data received, close socket=%d",
                             _socket);
        }

        return false;
    }

    _continueRecvError = 0;

    length = recv;
    return true;
}

int UDPSocket::setBlocking(bool blocking)
{
    int nonblocking = blocking ? 0 : 1;
    //TODO by zijwu, we need call ioctlsocket in windows platform.
#ifdef WIN32
    int ret = ioctlsocket(_socket, FIONBIO, (u_long *)&nonblocking);
#else
    int ret = ioctl(_socket, FIONBIO, &nonblocking);
#endif
    return ret;
}

int UDPSocket::setBufLength(unsigned int len)
{
    CHECK_SOCKET_OR_RETURN(_socket, 0);
    int ret = setsockopt(_socket, SOL_SOCKET, SO_SNDBUF, (const char *)(&len), sizeof(len));

    if (ret == -1)
    {
        return ret;
    }

    ret = setsockopt(_socket, SOL_SOCKET, SO_RCVBUF, (const char *)(&len), sizeof(len));
    return ret;
}

int UDPSocket::connect(const std::string& ip, uint16_t port)
{
    CHECK_SOCKET_OR_RETURN(_socket, 0);
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(struct sockaddr_in));
    addr.sin_family = AF_INET;
    ::inet_pton(AF_INET, ip.c_str(), &(addr.sin_addr));
    addr.sin_port = htons(port);
    int ret = ::connect(_socket, (struct sockaddr*)&addr, sizeof(struct sockaddr_in));
    if (ret == 0) {
        _connected = true;
        _remoteIP = ip;
        _remotePort = port;
    }
    return ret;
}

bool UDPSocket::sendto(const void *data, unsigned int length, int *sentlen)
{
    if (!_connected) {
        return false;
    }

    CHECK_SOCKET_OR_RETURN(_socket, false);

    assert(data);

    int sent = static_cast<int>(::send(_socket,
                                         (const char *)data,
                                         length,
                                         0));

    if (sent == -1)
    {
        if (_continueSendError++ < 5)
        {
            SLOG_FMT(kLogTag,LOG_ERROR,
                                 "send udp data failed, socket=%d, data=%p, length=%u, local ip=%s, local port=%u, error=%d",
                                 _socket,
                                 data,
                                 length,
                                 this->address().c_str(),
                                 this->port(),
                                 getError());
        }


        if (sentlen)
        {
            *sentlen = 0;
        }

        return false;
    }

    _continueSendError = 0;

    if (sentlen)
    {
        *sentlen = sent;
    }

    return true;
}

// IUdpSocket interface implementation
STDMETHODIMP_(bool) UDPSocket::Create(THIS_ const char *ip, uint16_t port)
{
    std::string strIp(ip);
    return create(strIp, port);
}

STDMETHODIMP_(bool) UDPSocket::SendTo(THIS_ const char *ip, uint16_t port, const void *data, unsigned int length, int *sentlen)
{
    std::string strIp(ip);
    return sendto(strIp, port, data, length, sentlen);
}

STDMETHODIMP_(bool) UDPSocket::RecvFrom(THIS_ char *ip, uint16_t &port, void *data, unsigned int &length)
{
    std::string strIp;
    bool result = recvfrom(strIp, port, data, length);
    if (result && ip)
    {
        strcpy(ip, strIp.c_str());
    }
    return result;
}

STDMETHODIMP_(int) UDPSocket::SetBlocking(THIS_ bool blocking)
{
    return setBlocking(blocking);
}

STDMETHODIMP_(int) UDPSocket::SetBufLength(THIS_ unsigned int len)
{
    return setBufLength(len);
}

STDMETHODIMP_(void) UDPSocket::Close(THIS)
{
    Socket::close();
}

SNSEND
