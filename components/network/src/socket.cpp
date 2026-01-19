#include "socket.h"
#include <cstring>
#include <cassert>

#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#include <algorithm>
#else
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <netdb.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <unistd.h>
#include <errno.h>
#include <netinet/ip.h> 

#endif
#include <helper/slog.h>
#define kLogTag "Socket"

#undef INVALID_SOCKET
#define INVALID_SOCKET -1

#define CHECK_SOCKET_OR_RETURN(socket,ret) \
    if (_socket == INVALID_SOCKET) { \
        SLOGE()<<"socket not created, operation failed"; \
        return ret;\
    }

SNSBEGIN
    
bool Socket::fillSockAddr(const std::string &ip, uint16_t port, void *address)
{
    bool isIPv6Address = false;
    bool ret = Socket::isIPv6Address(ip, isIPv6Address);

    if (!ret)
    {
        return ret;
    }

    if (isIPv6Address)
    {
        sockaddr_in6 *addr = static_cast<sockaddr_in6 *> (address);
        memset(addr, 0, sizeof(sockaddr_in6));
#ifdef WIN32
        int res = inet_pton_win(AF_INET6, ip.c_str(), &(addr->sin6_addr));
#else
        int res = inet_pton(AF_INET6, ip.c_str(), &(addr->sin6_addr));
#endif

        if (res != 1)
        {
            return false;
        }

        addr->sin6_family = AF_INET6;
        addr->sin6_port = htons(port);
    }
    else
    {
        sockaddr_in *addr = static_cast<sockaddr_in *> (address);
        memset(addr, 0, sizeof(sockaddr_in));
#ifdef WIN32
        int res = inet_pton_win(AF_INET, ip.c_str(), &(addr->sin_addr));
#else
        int res = inet_pton(AF_INET, ip.c_str(), &(addr->sin_addr));
#endif

        if (res != 1)
        {
            return false;
        }

        addr->sin_family = AF_INET;
        addr->sin_port = htons(port);
    }

    return true;
}

std::set<Socket *> Socket::waitSockets(const std::set<Socket *> &sockets, unsigned int msec)
{
    // There socket should no more than FD_SETSIZE
    assert(sockets.size() < FD_SETSIZE);

    std::set<Socket *> result;
    timeval waiting = {0};
    waiting.tv_sec = msec / 1000;
    waiting.tv_usec = msec % 1000 * 1000;

    int maxfdp = 0;
    fd_set fs;
    FD_ZERO(&fs);

    for (std::set<Socket *>::const_iterator it = sockets.begin(); it != sockets.end(); it++)
    {
        FD_SET((*it)->_socket, &fs);
        maxfdp = std::max(maxfdp, (*it)->_socket);
    }

    int val = 0;
    val = ::select(maxfdp + 1, &fs, NULL, NULL, &waiting);

    if (val > 0)
    {
        for (std::set<Socket *>::const_iterator it = sockets.begin(); it != sockets.end(); it++)
        {
            if (FD_ISSET((*it)->_socket, &fs))
            {
                result.insert(*it);
            }
        }
    }

    return result;
}

Socket::Socket() :
    _socket(INVALID_SOCKET),
    _enableLog(true)
{
}

Socket::Socket(int sock) :
    _socket(sock),
    _enableLog(true)
{
}

Socket::Socket(const Socket &socket) :
    _socket(socket._socket),
    _enableLog(true)
{
}

Socket::~Socket()
{
}

void Socket::close()
{
    if (_socket != INVALID_SOCKET)
    {
        SLOG_FMT(kLogTag,LOG_DEBUG, "socket=%d closed", _socket);

        if (!closeSocket(_socket))
        {
            SLOG_FMT(kLogTag,LOG_DEBUG, "socket=%d close failed, error=%d", _socket, getError());
        }

        _socket = INVALID_SOCKET;
    }
}

int Socket::selectRead() const
{
    CHECK_SOCKET_OR_RETURN(_socket, RETURN_ERROR);

    fd_set fs;
    FD_ZERO(&fs);
    FD_SET(_socket, &fs);

    return select(&fs, NULL, NULL, -1);
}

int Socket::selectWrite() const
{
    CHECK_SOCKET_OR_RETURN(_socket, RETURN_ERROR);

    fd_set fs;
    FD_ZERO(&fs);
    FD_SET(_socket, &fs);

    return select(NULL, &fs, NULL, -1);
}

int Socket::selectRead(unsigned int msec) const
{
    CHECK_SOCKET_OR_RETURN(_socket, RETURN_ERROR);

    fd_set fs;
    FD_ZERO(&fs);
    FD_SET(_socket, &fs);

    return select(&fs, NULL, NULL, msec);
}

int Socket::selectWrite(unsigned int msec) const
{
    CHECK_SOCKET_OR_RETURN(_socket, RETURN_ERROR);

    fd_set fs;
    FD_ZERO(&fs);
    FD_SET(_socket, &fs);

    return select(NULL, &fs, NULL, msec);
}

int Socket::select(void *readSet, void *writeSet, void *errorSet, int timeout) const
{
    int val = 0;

    if (timeout == -1)
    {
        val = ::select(_socket + 1, (fd_set *)readSet, (fd_set *)writeSet, (fd_set *)errorSet, NULL);
    }
    else
    {
        timeval waiting = {0};
        waiting.tv_sec = timeout / 1000;
        waiting.tv_usec = timeout % 1000 * 1000;

        val = ::select(_socket + 1, (fd_set *)readSet, (fd_set *)writeSet, (fd_set *)errorSet, &waiting);
    }

    if (val < 0)
    {
        SLOG_FMT(kLogTag,LOG_INFO,
                         "socket=%d select failed, error=%d",
                         _socket,
                         getError());
        return RETURN_ERROR;
    }
    else if (val == 0)
    {
        return RETURN_TIMEOUT;
    }

    return RETURN_OK;
}

std::string Socket::address() const
{
    CHECK_SOCKET_OR_RETURN(_socket, "");

    sockaddr_in6 addr_inv6;
    sockaddr *addr = (sockaddr *)&addr_inv6;
    sockaddr_in *addr_in = (sockaddr_in *)&addr_inv6;

    socklen_t len = sizeof(addr_inv6);
    int ret = getsockname(_socket, addr, &len);

    if (ret == -1)
    {
        return "";
    }

    char ipStr[256];

    if (addr->sa_family == AF_INET6)
    {
#ifdef WIN32
        inet_ntop_win(addr_inv6.sin6_family, &addr_inv6.sin6_addr, ipStr, sizeof(sockaddr_in6));
#else
        inet_ntop(addr_inv6.sin6_family, &addr_inv6.sin6_addr, ipStr, sizeof(sockaddr_in6));
#endif
    }
    else
    {
#ifdef WIN32
        inet_ntop_win(addr_in->sin_family, &addr_in->sin_addr, ipStr, sizeof(sockaddr_in));
#else
        inet_ntop(addr_in->sin_family, &addr_in->sin_addr, ipStr, sizeof(sockaddr_in));
#endif
    }

    return std::string(ipStr);
}

uint16_t Socket::port() const
{
    CHECK_SOCKET_OR_RETURN(_socket, 0);

    sockaddr_in6 addr_inv6;
    sockaddr *addr = (sockaddr *)&addr_inv6;
    socklen_t len = sizeof(addr_inv6);
    int ret = getsockname(_socket, addr, &len);

    if (ret == -1)
    {
        return 0;
    }

    if (addr->sa_family == AF_INET6)
    {
        uint16_t port = ntohs(((sockaddr_in6 *)addr)->sin6_port);
        return port;
    }
    else
    {
        uint16_t port = ntohs(((sockaddr_in *)addr)->sin_port);
        return port;
    }
}

bool Socket::isIPv6Address(const std::string &ip, bool &isIPv6)
{
    static bool lastResult = false;
    static std::string lastIp = "";

    if (lastIp == ip)
    {
        isIPv6 = lastResult;
        return true;
    }

    lastIp = ip;

    struct addrinfo hint, *info = 0;
    memset(&hint, 0, sizeof(hint));
    hint.ai_family = AF_UNSPEC;
    int ret = getaddrinfo(ip.c_str(), 0, &hint, &info);

    if (ret != 0)
    {
        lastIp = "";
        lastResult = false;
        return false;
    }

    int result = info->ai_family;
    freeaddrinfo(info);
    isIPv6 = (result == AF_INET6);
    lastResult = isIPv6;
    return true;
}

int Socket::createSocket(const std::string &ip, uint16_t port, bool isTCP, bool isReusable)
{
    prepareSocket();
    sockaddr_in6 addrStorage;
    sockaddr *address = (sockaddr *)(void *)(&addrStorage);

    if (!fillSockAddr(ip, port, address))
    {
        SLOG_FMT(kLogTag,LOG_ERROR,
                         "create socket failed,invalid ip=%s, port=%d",
                         ip.c_str(),
                         port);
        return INVALID_SOCKET;
    }

    int type = SOCK_DGRAM;
    int protocol = IPPROTO_UDP;

    if (isTCP)
    {
        type = SOCK_STREAM;
        protocol = IPPROTO_TCP;
    }

    int sock = socket(address->sa_family, type, protocol);

    if (sock == INVALID_SOCKET)
    {
        SLOG_FMT(kLogTag,LOG_ERROR,
                         "socket create failed, error=%d",
                         getError());
        return INVALID_SOCKET;
    }

    if (isReusable)
    {
        struct  linger ling;
        ling.l_onoff = 1;
        ling.l_linger = 0;

        if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, (const char *)&ling, sizeof(ling)) == -1)
        {
            SLOG_FMT(kLogTag,LOG_ERROR,
                             "socket create failed, set address reuse error=%d",
                             getError());
            return INVALID_SOCKET;
        }
    }

#ifdef __APPLE__
    int set = 1;
    setsockopt(sock, SOL_SOCKET, SO_NOSIGPIPE, (void *)&set, sizeof(set));
#endif


    int addressSize = sizeof(sockaddr_in);

    if (address->sa_family == AF_INET6)
    {
        addressSize = sizeof(sockaddr_in6);
    }

    int ret = ::bind(sock, address, addressSize);

    if (ret == -1)
    {
        SLOG_FMT(kLogTag,LOG_ERROR,
                         "socket create failed, bind error=%d",
                         getError());
        closeSocket(sock);
        return INVALID_SOCKET;
    }

    return sock;
}

int Socket::createSocket(bool isIpV4, bool isTCP, bool isReusable)
{
    prepareSocket();

    int type = SOCK_DGRAM;
    int protocol = IPPROTO_UDP;
    int ipFamily = isIpV4 ? AF_INET : AF_INET6;

    if (isTCP)
    {
        type = SOCK_STREAM;
        protocol = IPPROTO_TCP;
    }

    int sock = socket(ipFamily, type, protocol);

    if (sock == INVALID_SOCKET)
    {
        SLOG_FMT(kLogTag,LOG_ERROR,
                         "socket create failed, error=%d",
                         getError());
        return INVALID_SOCKET;
    }

    if (isReusable)
    {
        struct  linger ling;
        ling.l_onoff = 1;
        ling.l_linger = 0;

        if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, (const char *)&ling, sizeof(ling)) == -1)
        {
            SLOG_FMT(kLogTag,LOG_ERROR,
                             "socket create failed, set address reuse error=%d",
                             getError());
            return INVALID_SOCKET;
        }
    }

#ifdef __APPLE__
    int set = 1;
    setsockopt(sock, SOL_SOCKET, SO_NOSIGPIPE, (void *)&set, sizeof(set));
#endif

    return sock;
}

#if !defined(_WIN32) && !defined(__APPLE__)
int Socket::enableIpTos()
{
	unsigned char  service_type = 0x98;

	if (setsockopt(_socket, SOL_IP, IP_TOS, (const char *)&service_type, sizeof(service_type)) == -1)
	{
	    SLOG_FMT(kLogTag,LOG_ERROR,
	                     "setsockopt set IP_TOS error=%d",
	                     getError());
	    return INVALID_SOCKET;
	}

	return 0;
}
#endif
SNSEND
