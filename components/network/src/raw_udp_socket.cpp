#include "raw_udp_socket.h"
#include <algorithm>
#include <map>
#include <assert.h>

#ifndef _WIN32
#include <arpa/inet.h>
#include <unistd.h>
#else
#include <WinSock2.h>
#endif
#include "system_util.h"

#include <helper/slog.h>
#define kLogTag "RawUdpSocket"


// for UDP, max support 16 bits len payload
#define MAX_MSG (96*1024)


using namespace std;

SNSBEGIN

UDPAddress::UDPAddress(const sockaddr_in6 *addr):
    _addr(*addr)
{
    assert(addr != NULL);
}

UDPAddress::UDPAddress(std::string ip, uint16_t port)
{
    if (!Socket::fillSockAddr(ip, port, &_addr))
    {
        SLOG_FMT(kLogTag,LOG_ERROR, "create udp address failed");
    }
}


bool UDPAddress::operator==(const UDPAddress &other) const
{
    if (((sockaddr *)&_addr)->sa_family == AF_INET6)
    {
        if (((sockaddr *)&other._addr)->sa_family != AF_INET6)
        {
            return false;
        }

        return memcmp(&(_addr.sin6_addr), &(other._addr.sin6_addr), sizeof(other._addr.sin6_addr)) == 0;
    }
    else
    {
        if (((sockaddr *)&other._addr)->sa_family == AF_INET6)
        {
            return false;
        }

        sockaddr_in *addrIn = (sockaddr_in *)&_addr;
        sockaddr_in *otherAddiIn = (sockaddr_in *)&other._addr;
        return memcmp(&(addrIn->sin_addr), &(otherAddiIn->sin_addr), sizeof(sockaddr_in)) == 0;
    }
}
bool UDPAddress::operator!=(const UDPAddress &other) const
{
    return (*this) == other;
}

const UDPAddress &UDPAddress::operator=(const UDPAddress &other)
{
    memcpy(&_addr, &other._addr, sizeof(_addr));

    return *this;
}

const sockaddr_in6 *UDPAddress::getRawSocketAddr() const
{
    return &_addr;
}


RawUdpSocket::RawUdpSocket(std::string localIP, int localPort, uint32_t bufferSize):
    UDPSocket(),
    _handler(NULL),
    _createSuccess(false)
{
    _createSuccess = this->create(localIP, localPort);
    this->setBlocking(false);
    int res = this->setBufLength(bufferSize);

    if (res == -1)
    {
        _createSuccess = false;
        SLOG_FMT(kLogTag,LOG_ERROR, "create raw udp socket failed, because set buffer size failed");
    }
}

RawUdpSocket::~RawUdpSocket()
{
    this->close();
}

bool RawUdpSocket::tryReceive(int maxWaitTime)
{
    if (selectRead(maxWaitTime) != RETURN_OK)
    {
        return false;
    }

    // get buffer and release buffer.
    sockaddr_in6 addr = {0};
    static char data[MAX_MSG];
    unsigned int length = sizeof(data);
    bool ret = this->recvfrom(addr, data, length);

    if (!ret)
    {
        return false;
    }

    if (_handler == NULL)
    {
        return true;
    }

    UDPAddress address(&addr);

    _handler->onDataReady(&address, data, length);

    return true;
}

void RawUdpSocket::setDataHandler(RawUdpSocketDataHandler *handler)
{
    _handler = handler;
}

int RawUdpSocket::sendPacket(const char *data, int len, const UDPAddress &addr)
{
    assert(len < MAX_MSG);
    unsigned int sentLen = 0;

    bool ret = sendto(*addr.getRawSocketAddr(), data, (unsigned int)len, &sentLen);

    if (!ret || ((int)sentLen) != len)
    {
        return -1;
    }

    return len;
}

bool RawUdpSocket::createSuccess()
{
    return _createSuccess;
}


SNSEND
