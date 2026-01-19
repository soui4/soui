#pragma once

#include <list>
#include <vector>
#include "socket.h"
#include <helper/obj-ref-impl.hpp>

SNSBEGIN

class TcpClientSocket;

/**
 * ServerSocket represents a TCP server socket.
 */
class ServerSocket : public Socket, public TObjRefImpl<ITcpServerSocket>
{
public:
    /**
     * Default constructor.
     */
    ServerSocket();

    /**
     * Destructor.
     */
    virtual ~ServerSocket();

    // ITcpServerSocket interface
    STDMETHOD_(bool, Create)(THIS_ const char *ip, uint16_t port) OVERRIDE;
    STDMETHOD_(void, Close)(THIS) OVERRIDE;
    STDMETHOD_(bool, Listen)(THIS_ unsigned int count) OVERRIDE;
    STDMETHOD_(ITcpClientSocket *, Accept)(THIS) OVERRIDE;
    STDMETHOD_(int, GetClientSocketsRef)(THIS_ ITcpClientSocket **sockets, unsigned int count) SCONST OVERRIDE;
    STDMETHOD_(int, GetClientSocketCount)(CTHIS) SCONST OVERRIDE;

    /**
     * Create TCP server socket.
     * @param ip local IP address.
     * @param port local port.
     * @return true if successfully.
     */
    virtual bool create(const std::string &ip, uint16_t port);

    /**
     * Listen for TCP client connecting.
     * @param count socket count of listening.
     * @return true if successfully.
     */
    bool listen(unsigned int count) const;

    /**
     * Accept for TCP client connecting synchronized.
     * @return a TCP client socket.
     */
    TcpClientSocket *accept();

    /**
     * Get TCP client sockets for all connections.
     * @return TCP client socket list.
     */
    std::vector<SAutoRefPtr<TcpClientSocket>> clients() const;

    /**
     * Close TCP server socket.
     */
    void close();
private:
    void clearClosedTcpClientSockets();
    void clearTcpClientSockets();

    std::list<SAutoRefPtr<TcpClientSocket> > _clientSockets;
};

SNSEND
