#include <ws2tcpip.h>
#include "../../socket.h"
#include <winsock2.h>
#include <cerrno>

SNSBEGIN

void Socket::prepareSocket()
{
    WORD wVersionRequested = MAKEWORD(2, 2);
    WSADATA wsaData = {0};
    WSAStartup(wVersionRequested, &wsaData);
}

bool Socket::closeSocket(int socket)
{
    bool ret = ::closesocket(socket) == 0;
    WSACleanup();

    return ret;
}

int Socket::getError()
{
    return WSAGetLastError();
}

//inet_ntop_win is same as inet_ntop().
//inet_ntop() is not supported in WinXP. So we implement it myself.
const char *inet_ntop_win(int family, void *src , char *dst, unsigned int len)
{
    struct sockaddr_storage addr;
    memset(&addr, 0, sizeof(addr));
    int addr_len = sizeof(addr);
    DWORD address_string_len = len;
    addr.ss_family = family;

    if (family == AF_INET)
    {
        memcpy(&((struct sockaddr_in *) &addr)->sin_addr, src, sizeof(struct in_addr));
    }
    else if (family == AF_INET6)
    {
        memcpy(&((struct sockaddr_in6 *)&addr)->sin6_addr, src, sizeof(struct in6_addr));
    }

    int rc = WSAAddressToStringA((struct sockaddr *)&addr, addr_len, NULL, dst, &address_string_len);

    if (0 == rc && address_string_len <= len)
    {
        return dst;
    }
    else
    {
        return NULL;
    }
}

//inet_ntop_win is same as inet_ntop().
//inet_ntop() is not supported in WinXP. So we implement it myself.
int inet_pton_win(int family, const char *src, void *dst)
{
    struct sockaddr_storage addr;
    int addr_len = sizeof(addr);
    addr.ss_family = family;

    int rc = WSAStringToAddressA((char *) src, family, NULL,
                                 (struct sockaddr *) &addr, &addr_len);

    if (rc != 0)
    {
        return -1;
    }

    if (family == AF_INET)
    {
        memcpy(dst, &((struct sockaddr_in *) &addr)->sin_addr,
               sizeof(struct in_addr));
    }
    else if (family == AF_INET6)
    {
        memcpy(dst, &((struct sockaddr_in6 *)&addr)->sin6_addr,
               sizeof(struct in6_addr));
    }

    return 1;
}

SNSEND
