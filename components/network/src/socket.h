#pragma once

#include <stdint.h>
#include <vector>
#include <set>
#include <string>

#define WIN32_LEAN_AND_MEAN
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#ifdef _WIN32
#include <winsock2.h>
#endif
#include "common_defs.h"
#include <helper/obj-ref-impl.hpp>
#include <interface/network-i.h>



SNSBEGIN

/**
 * Socket base class.
 */
class Socket
{
public:
	
    static bool fillSockAddr(const std::string &ip, uint16_t port, void *address);
	
    /**
     * Wiat multiple socket for data untile timeout.
     * @param msec the timeout value.
     */
    static std::set<Socket *> waitSockets(const std::set<Socket *> &socket, unsigned int msec);
	
    /**
     * Default constructor.
     */
    Socket();

    /**
     * Constructor with a socket.
     * @param sock existing socket.
     */
    Socket(int sock);

    /**
     * Copy constructor.
     */
    Socket(const Socket &socket);

    /**
     * Destructor.
     */
    virtual ~Socket();

    /**
     * Create a socket.
     * @param ip local IP address to bind.
     * @param port local port to bind.
     * @return true if successfully.
     */
    virtual bool create(const std::string &ip, uint16_t port) = 0;

    /**
     * Close socket.
     */
    virtual void close();

    /**
     * Select a socket synchronized. For client socket, it will wait for data receiving;
     * for server socket, it will wait for connecting.
     * @return true if successfully.
     */
    int selectRead() const;
    int selectWrite() const;

    /**
     * Select a socket synchronized during a time.
     * @param msec waiting time in milliseconds.
     * @return true if successfully.
     */
    int selectRead(unsigned int msec) const;
    int selectWrite(unsigned int msec) const;

    /**
     * Get local IP address bound.
     * @return local IP address.
     */
    std::string address() const;

    /**
     * Get local port bound.
     * @return local port.
     */
    uint16_t port() const;

    /**
     * Set the socket blocking mode.
     * @param blocking true for block and false for no-blocking mode.
     */
    virtual int setBlocking(bool blocking)
    {
        return -1;
    }

    /**
     * Set the max length for receiving data  and send data.
     * @param len the length of the buffer.
     */
    virtual int setBufLength(unsigned int len)
    {
        return -1;
    }

    int getSocket()
    {
        return _socket;
    }
#if !defined(_WIN32) && !defined(__APPLE__)
	int enableIpTos();
#endif
    static bool closeSocket(int socket);
protected:
    // return success or not.
    static bool isIPv6Address(const std::string &ip, bool &isIPv6);
    static void prepareSocket();

    static int getError();

    int createSocket(const std::string &ip, uint16_t port, bool isTCP, bool isReusable);
    int createSocket(bool isIpV4, bool isTCP, bool isReusable);

    int select(void *readSet, void *writeSet, void *errorSet, int timeout) const;

#ifndef WIN32
    static const int INVALID_SOCKET = -1;
#endif
    int _socket;
    bool _enableLog;
};

#ifdef WIN32
//inet_ntop_win is same as inet_ntop().inet_pton_win is same as inet_pton().
//there are not supported in WinXP. So we implement them myself.
const char *inet_ntop_win(int family, void *src , char *dst, unsigned int len);
int inet_pton_win(int family, const char *src, void *dst);
#endif

SNSEND
