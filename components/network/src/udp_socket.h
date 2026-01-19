#pragma once

#include "socket.h"

struct sockaddr_in6;

SNSBEGIN

/**
 * UDP socket class.
 */
class UDPSocket : public Socket, public TObjRefImpl<IUdpSocket>
{
public:
    /**
     * Default constructor.
     */
    UDPSocket();

    /**
     * Destructor.
     */
    virtual ~UDPSocket();

    // IUdpSocket interface
    STDMETHOD_(bool, Create)(THIS_ const char *ip, uint16_t port) OVERRIDE;
    STDMETHOD_(bool, SendTo)(THIS_ const char *ip, uint16_t port, const void *data, unsigned int length, int *sentlen) OVERRIDE;
    STDMETHOD_(bool, RecvFrom)(THIS_ char *ip, uint16_t &port, void *data, unsigned int &length) OVERRIDE;
    STDMETHOD_(int, SetBlocking)(THIS_ bool blocking) OVERRIDE;
    STDMETHOD_(int, SetBufLength)(THIS_ unsigned int len) OVERRIDE;
    STDMETHOD_(void, Close)(THIS) OVERRIDE;

    /**
     * Create UDP socket.
     * @param ip local IP address to bind.
     * @param port local port to bind.
     * @return true if successfully.
     */
    virtual bool create(const std::string &ip, uint16_t port);

    /**
     * Send data to remote.
     * @param ip remote IP address.
     * @param port remote port.
     * @param data data to be sent.
     * @param length length of data.
     * @param sentlen length of data being sent.
     * @return true if successfully.
     */
    bool sendto(const std::string &ip,
                uint16_t port,
                const void *data,
                unsigned int length,
                int *sentlen);

    bool sendto(const sockaddr_in6 &addr,
                const void *data,
                unsigned int length,
                unsigned int *sentlen);

    /**
     * Receive data from remote.
     * @param ip remote IP address.
     * @param port remote port address.
     * @param data buffer for received data.
     * @param length length of data received.
     * @return true if successfully.
     */
    bool recvfrom(std::string &ip,
                  uint16_t &port,
                  void *data,
                  unsigned int &length);

    bool recvfrom(sockaddr_in6 &addr,
                  void *data,
                  unsigned int &length);

    /**
     * Detail can be find in base class.
     */
    int setBlocking(bool blocking);

    /**
     * Detail can be find in base class.
     */
    int setBufLength(unsigned int len);

    int connect(const std::string& ip, uint16_t port);

    bool sendto(const void *data,
                unsigned int length,
                int *sentlen);

private:
    uint32_t _continueRecvError;
    uint32_t _continueSendError;
    bool _connected;
    std::string _remoteIP;
    uint16_t _remotePort;
};

SNSEND
