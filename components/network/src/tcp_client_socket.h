#pragma once

#include "socket.h"

SNSBEGIN

/**
 * TcpClientSocket class is a class for TCP client socket.
 */
class TcpClientSocket : public Socket, public TObjRefImpl<ITcpClientSocket>
{
public:
    /**
     * Default constructor.
     */
    TcpClientSocket();

    /**
     * Constructor for existing socket.
     * @param sock existing TCP client socket.
     * @param remoteIP remote IP address for this socket.
     * @param remotePort remote port address for this socket.
     */
    TcpClientSocket(int sock, const std::string &remoteIP, uint16_t remotePort);

    /**
     * Copy constructor.
     * @param socket TcpClientSocket object to copy.
     */
    TcpClientSocket(const TcpClientSocket &socket);

    /**
     * Destructor.
     */
    virtual ~TcpClientSocket();

    // ITcpClientSocket interface
    STDMETHOD_(bool, Create)(THIS_ const char *ip, uint16_t port) OVERRIDE;
    STDMETHOD_(bool, CreateIpV4)(THIS) OVERRIDE;
    STDMETHOD_(bool, Connect)(THIS_ const char *ip, uint16_t port) OVERRIDE;
    STDMETHOD_(bool, ConnectTimeout)(THIS_ const char *ip, uint16_t port, uint32_t timeout) OVERRIDE;
    STDMETHOD_(bool, ConnectAsync)(THIS_ const char *ip, uint16_t port) OVERRIDE;
    STDMETHOD_(bool, IsConnected)(CTHIS) SCONST OVERRIDE;
    STDMETHOD_(bool, Send)(THIS_ const void *data, unsigned int length, unsigned int *sentlen) OVERRIDE;
    STDMETHOD_(bool, Recv)(THIS_ void *data, unsigned int &length) OVERRIDE;
    STDMETHOD_(int, SetBlocking)(THIS_ bool blocking) OVERRIDE;
    STDMETHOD_(void, Close)(THIS) OVERRIDE;
    STDMETHOD_(const char *, GetRemoteIP)(CTHIS) SCONST OVERRIDE;
    STDMETHOD_(uint16_t, GetRemotePort)(CTHIS) SCONST OVERRIDE;

    /**
     * Get connected remote IP address.
     * @return remote IP address.
     */
    std::string getRemoteIP() const;

    /**
     * Get connected remote port.
     * @return remote port.
     */
    uint16_t getRemotePort() const;

    virtual int setBlocking(bool blocking);	

    /**
     * Create socket with IP address and port.
     * @param ip local IP address.
     * @param port local IP port. Port could be chosen by system automatically
     * if port is 0.
     * @return true if create socket successfully.
     */
    virtual bool create(const std::string &ip, uint16_t port);

    /**
     * Create socket .
     * @return true if create socket successfully.
     */
    virtual bool create(bool isIpV4);

    /**
     * Connect socket to remote TCP server socket.
     * @param ip remote IP address.
     * @param port remote port.
     * @return true if connect successfully.
     */
    bool connect(const std::string &ip, uint16_t port);
    bool connect(const std::string &ip, uint16_t port, uint32_t timeout);
    bool connectAsync(const std::string &ip, uint16_t port);
    bool isConnected() const;


    /**
     * Send data to remote socket.
     * @param data data to be sent. Cannot be NULL.
     * @param length data length.
     * @param sentlen returning data length that is sent.
     * @return true if data is sent successfully.
     */
    bool send(const void *data, unsigned int length, unsigned int *sentlen) const;

    /**
     * Receive data from remote synchronized.
     * @param data data buffer that will filled in with received data.
     * @param length data length received.
     * @return true if data is received successfully.
     */
    bool recv(void *data, unsigned int &length) const;


private:
    std::string _remoteIP;
    uint16_t _remotePort;
    bool _isBlocking;
};

SNSEND
