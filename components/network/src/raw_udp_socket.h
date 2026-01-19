#pragma once
#include <set>
#include <map>
#include <string>
#include <memory>
#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#else
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <sys/ioctl.h>
#endif
#include "socket.h"
#include "udp_socket.h"

SNSBEGIN

class UDPAddress
{
public:
    UDPAddress(const sockaddr_in6 *addr);
    UDPAddress(std::string ip, uint16_t port);

    bool operator==(const UDPAddress &other) const;
    bool operator!=(const UDPAddress &other) const;
    const UDPAddress &operator=(const UDPAddress &other);

    const sockaddr_in6 *getRawSocketAddr() const;
private:
    sockaddr_in6 _addr;
};

class RawUdpSocketDataHandler
{
public:
    virtual void onDataReady(const UDPAddress *addr, const char *data, uint32_t len) = 0;
};

class RawUdpSocket: public UDPSocket
{
public:
    RawUdpSocket(std::string localIP, int localPort, uint32_t bufferSize);
    ~RawUdpSocket();

    void setDataHandler(RawUdpSocketDataHandler *handler);
    bool tryReceive(int maxWaitTime);
    int sendPacket(const char *data, int len, const UDPAddress &addr);
    bool createSuccess();
private:
    RawUdpSocketDataHandler *_handler;
    bool _createSuccess;
};

SNSEND
