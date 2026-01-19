#ifndef NETWORK_INTERFACE_H
#define NETWORK_INTERFACE_H

#include <interface/obj-ref-i.h>
#include <stdint.h>

SNSBEGIN

#undef INTERFACE
#define INTERFACE ITcpClientSocket
/**
 * @brief TCP client socket interface
 * 
 * Defines the core operations of TCP client socket, including creating socket, connecting to server, 
 * sending and receiving data, etc. This interface is the core of TCP client communication and provides 
 * complete TCP client functionality.
 */
DECLARE_INTERFACE_(ITcpClientSocket, IObjRef)
{
    STDMETHOD_(long, AddRef)(THIS) PURE;
    STDMETHOD_(long, Release)(THIS) PURE;
    STDMETHOD_(void, OnFinalRelease)(THIS) PURE;

    /**
     * @brief Create and connect TCP client socket
     * @param ip Server IP address
     * @param port Server port
     * @return TRUE for success, FALSE for failure
     */
    STDMETHOD_(bool, Create)(THIS_ const char *ip, uint16_t port) PURE;
    
    /**
     * @brief Create IPv4 TCP client socket
     * @return TRUE for success, FALSE for failure
     */
    STDMETHOD_(bool, CreateIpV4)(THIS) PURE;
    
    /**
     * @brief Connect to specified TCP server
     * @param ip Server IP address
     * @param port Server port
     * @return TRUE for success, FALSE for failure
     */
    STDMETHOD_(bool, Connect)(THIS_ const char *ip, uint16_t port) PURE;
    
    /**
     * @brief Connect to TCP server with timeout
     * @param ip Server IP address
     * @param port Server port
     * @param timeout Timeout in milliseconds
     * @return TRUE for success, FALSE for failure
     */
    STDMETHOD_(bool, ConnectTimeout)(THIS_ const char *ip, uint16_t port, uint32_t timeout) PURE;
    
    /**
     * @brief Asynchronously connect to TCP server
     * @param ip Server IP address
     * @param port Server port
     * @return TRUE for success, FALSE for failure
     */
    STDMETHOD_(bool, ConnectAsync)(THIS_ const char *ip, uint16_t port) PURE;
    
    /**
     * @brief Check if connection is established
     * @return TRUE if connected, FALSE if not connected
     */
    STDMETHOD_(bool, IsConnected)(CTHIS) SCONST PURE;
    
    /**
     * @brief Send data to TCP server
     * @param data Data to send
     * @param length Data length
     * @param sentlen Actual number of bytes sent (output parameter)
     * @return TRUE for success, FALSE for failure
     */
    STDMETHOD_(bool, Send)(THIS_ const void *data, unsigned int length, unsigned int *sentlen) PURE;
    
    /**
     * @brief Receive data from TCP server
     * @param data Buffer to receive data
     * @param length Buffer size, returns actual number of bytes received
     * @return TRUE for success, FALSE for failure
     */
    STDMETHOD_(bool, Recv)(THIS_ void *data, unsigned int &length) PURE;
    
    /**
     * @brief Set socket blocking mode
     * @param blocking TRUE for blocking mode, FALSE for non-blocking mode
     * @return 0 for success, non-zero for failure
     */
    STDMETHOD_(int, SetBlocking)(THIS_ bool blocking) PURE;
    
    /**
     * @brief Close TCP client socket
     */
    STDMETHOD_(void, Close)(THIS) PURE;
    
    /**
     * @brief Get remote server IP address
     * @return Remote server IP address string
     */
    STDMETHOD_(const char *, GetRemoteIP)(CTHIS) SCONST PURE;
    
    /**
     * @brief Get remote server port
     * @return Remote server port number
     */
    STDMETHOD_(uint16_t, GetRemotePort)(CTHIS) SCONST PURE;
};

#undef INTERFACE
#define INTERFACE IUdpSocket
/**
 * @brief UDP socket interface
 * 
 * Defines the core operations of UDP socket, including creating socket, sending and receiving UDP datagrams, etc.
 * This interface is the core of UDP communication and provides complete UDP socket functionality.
 */
DECLARE_INTERFACE_(IUdpSocket, IObjRef)
{
    STDMETHOD_(long, AddRef)(THIS) PURE;
    STDMETHOD_(long, Release)(THIS) PURE;
    STDMETHOD_(void, OnFinalRelease)(THIS) PURE;

    /**
     * @brief Create UDP socket and bind to specified address
     * @param ip Local IP address
     * @param port Local port
     * @return TRUE for success, FALSE for failure
     */
    STDMETHOD_(bool, Create)(THIS_ const char *ip, uint16_t port) PURE;
    
    /**
     * @brief Send UDP datagram to specified address
     * @param ip Destination IP address
     * @param port Destination port
     * @param data Data to send
     * @param length Data length
     * @param sentlen Actual number of bytes sent (output parameter)
     * @return TRUE for success, FALSE for failure
     */
    STDMETHOD_(bool, SendTo)(THIS_ const char *ip, uint16_t port, const void *data, unsigned int length, int *sentlen) PURE;
    
    /**
     * @brief Receive datagram from UDP socket
     * @param ip Sender IP address (output parameter)
     * @param port Sender port (output parameter)
     * @param data Buffer to receive data
     * @param length Buffer size, returns actual number of bytes received
     * @return TRUE for success, FALSE for failure
     */
    STDMETHOD_(bool, RecvFrom)(THIS_ char *ip, uint16_t &port, void *data, unsigned int &length) PURE;
    
    /**
     * @brief Set socket blocking mode
     * @param blocking TRUE for blocking mode, FALSE for non-blocking mode
     * @return 0 for success, non-zero for failure
     */
    STDMETHOD_(int, SetBlocking)(THIS_ bool blocking) PURE;
    
    /**
     * @brief Set socket buffer length
     * @param len Buffer length
     * @return 0 for success, non-zero for failure
     */
    STDMETHOD_(int, SetBufLength)(THIS_ unsigned int len) PURE;
    
    /**
     * @brief Close UDP socket
     */
    STDMETHOD_(void, Close)(THIS) PURE;
};

#undef INTERFACE
#define INTERFACE ITcpServerSocket
/**
 * @brief TCP server socket interface
 * 
 * Defines the core operations of TCP server socket, including creating socket, listening for connections, accepting client connections, etc.
 * This interface is the core of TCP server communication and provides complete TCP server functionality.
 */
DECLARE_INTERFACE_(ITcpServerSocket, IObjRef)
{
    STDMETHOD_(long, AddRef)(THIS) PURE;
    STDMETHOD_(long, Release)(THIS) PURE;
    STDMETHOD_(void, OnFinalRelease)(THIS) PURE;

    /**
     * @brief Create TCP server socket and bind to specified address
     * @param ip Local IP address
     * @param port Local port
     * @return TRUE for success, FALSE for failure
     */
    STDMETHOD_(bool, Create)(THIS_ const char *ip, uint16_t port) PURE;
    
    /**
     * @brief Close TCP server socket
     */
    STDMETHOD_(void, Close)(THIS) PURE;
    
    /**
     * @brief Start listening for client connections
     * @param count Maximum number of pending connections
     * @return TRUE for success, FALSE for failure
     */
    STDMETHOD_(bool, Listen)(THIS_ unsigned int count) PURE;
    
    /**
     * @brief Accept client connection
     * @return Client socket object, NULL for failure
     */
    STDMETHOD_(ITcpClientSocket *, Accept)(THIS) PURE;
    
    /**
     * @brief Get current number of connected clients
     * @return Number of clients
     */
    STDMETHOD_(int, GetClientSocketCount)(CTHIS) SCONST PURE;
    
    /**
     * @brief Get client socket reference list
     * @param sockets Array to store client socket pointers
     * @param count Array size
     * @return Actual number of clients obtained
     */
    STDMETHOD_(int, GetClientSocketsRef)(THIS_ ITcpClientSocket **sockets, unsigned int count) SCONST PURE;
};

#undef INTERFACE
#define INTERFACE INetwork
/**
 * @brief Network module core interface
 * 
 * Defines the core functionality of the network module, including creating various types of socket objects.
 * This interface is the entry point of the network module, through which TCP clients, UDP sockets,
 * and TCP server sockets can be created and managed. It is the foundation of the entire network functionality.
 */
DECLARE_INTERFACE_(INetwork, IObjRef)
{
    STDMETHOD_(long, AddRef)(THIS) PURE;
    STDMETHOD_(long, Release)(THIS) PURE;
    STDMETHOD_(void, OnFinalRelease)(THIS) PURE;

    /**
     * @brief Create TCP client socket
     * @return TCP client socket object, NULL for failure
     */
    STDMETHOD_(ITcpClientSocket *, CreateTcpClientSocket)(THIS) PURE;
    
    /**
     * @brief Create UDP socket
     * @return UDP socket object, NULL for failure
     */
    STDMETHOD_(IUdpSocket *, CreateUdpSocket)(THIS) PURE;
    
    /**
     * @brief Create TCP server socket
     * @return TCP server socket object, NULL for failure
     */
    STDMETHOD_(ITcpServerSocket *, CreateTcpServerSocket)(THIS) PURE;
};

SNSEND

#endif // NETWORK_INTERFACE_H
