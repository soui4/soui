#ifndef NETWORK_INTERFACE_H
#define NETWORK_INTERFACE_H

#include <interface/obj-ref-i.h>
#include <stdint.h>

SNSBEGIN

#undef INTERFACE
#define INTERFACE ITcpClientSocket
DECLARE_INTERFACE_(ITcpClientSocket, IObjRef)
{
    STDMETHOD_(long, AddRef)(THIS) PURE;
    STDMETHOD_(long, Release)(THIS) PURE;
    STDMETHOD_(void, OnFinalRelease)(THIS) PURE;

    STDMETHOD_(bool, Create)(THIS_ const char *ip, uint16_t port) PURE;
    STDMETHOD_(bool, CreateIpV4)(THIS) PURE;
    STDMETHOD_(bool, Connect)(THIS_ const char *ip, uint16_t port) PURE;
    STDMETHOD_(bool, ConnectTimeout)(THIS_ const char *ip, uint16_t port, uint32_t timeout) PURE;
    STDMETHOD_(bool, ConnectAsync)(THIS_ const char *ip, uint16_t port) PURE;
    STDMETHOD_(bool, IsConnected)(CTHIS) SCONST PURE;
    STDMETHOD_(bool, Send)(THIS_ const void *data, unsigned int length, unsigned int *sentlen) PURE;
    STDMETHOD_(bool, Recv)(THIS_ void *data, unsigned int &length) PURE;
    STDMETHOD_(int, SetBlocking)(THIS_ bool blocking) PURE;
    STDMETHOD_(void, Close)(THIS) PURE;
    STDMETHOD_(const char *, GetRemoteIP)(CTHIS) SCONST PURE;
    STDMETHOD_(uint16_t, GetRemotePort)(CTHIS) SCONST PURE;
};

#undef INTERFACE
#define INTERFACE IUdpSocket
DECLARE_INTERFACE_(IUdpSocket, IObjRef)
{
    STDMETHOD_(long, AddRef)(THIS) PURE;
    STDMETHOD_(long, Release)(THIS) PURE;
    STDMETHOD_(void, OnFinalRelease)(THIS) PURE;

    STDMETHOD_(bool, Create)(THIS_ const char *ip, uint16_t port) PURE;
    STDMETHOD_(bool, SendTo)(THIS_ const char *ip, uint16_t port, const void *data, unsigned int length, int *sentlen) PURE;
    STDMETHOD_(bool, RecvFrom)(THIS_ char *ip, uint16_t &port, void *data, unsigned int &length) PURE;
    STDMETHOD_(int, SetBlocking)(THIS_ bool blocking) PURE;
    STDMETHOD_(int, SetBufLength)(THIS_ unsigned int len) PURE;
    STDMETHOD_(void, Close)(THIS) PURE;
};

#undef INTERFACE
#define INTERFACE ITcpServerSocket
DECLARE_INTERFACE_(ITcpServerSocket, IObjRef)
{
    STDMETHOD_(long, AddRef)(THIS) PURE;
    STDMETHOD_(long, Release)(THIS) PURE;
    STDMETHOD_(void, OnFinalRelease)(THIS) PURE;

    STDMETHOD_(bool, Create)(THIS_ const char *ip, uint16_t port) PURE;
    STDMETHOD_(void, Close)(THIS) PURE;
    STDMETHOD_(bool, Listen)(THIS_ unsigned int count) PURE;
    STDMETHOD_(ITcpClientSocket *, Accept)(THIS) PURE;
    STDMETHOD_(int, GetClientSocketCount)(CTHIS) SCONST PURE;
    STDMETHOD_(int, GetClientSocketsRef)(THIS_ ITcpClientSocket **sockets, unsigned int count) SCONST PURE;
};

#undef INTERFACE
#define INTERFACE INetwork
DECLARE_INTERFACE_(INetwork, IObjRef)
{
    STDMETHOD_(long, AddRef)(THIS) PURE;
    STDMETHOD_(long, Release)(THIS) PURE;
    STDMETHOD_(void, OnFinalRelease)(THIS) PURE;

    STDMETHOD_(ITcpClientSocket *, CreateTcpClientSocket)(THIS) PURE;
    STDMETHOD_(IUdpSocket *, CreateUdpSocket)(THIS) PURE;
    STDMETHOD_(ITcpServerSocket *, CreateTcpServerSocket)(THIS) PURE;
};

SNSEND

#endif // NETWORK_INTERFACE_H
