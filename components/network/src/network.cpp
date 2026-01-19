#include "socket.h"
#include "network.h"
#include "tcp_client_socket.h"
#include "udp_socket.h"
#include "server_socket.h"
SNSBEGIN

SNetwork::SNetwork()
{
}

SNetwork::~SNetwork()
{
}

STDMETHODIMP_(ITcpClientSocket *) SNetwork::CreateTcpClientSocket(THIS)
{
    return new TcpClientSocket();
}

STDMETHODIMP_(IUdpSocket *) SNetwork::CreateUdpSocket(THIS)
{
    return new UDPSocket();
}

STDMETHODIMP_(ITcpServerSocket *) SNetwork::CreateTcpServerSocket(THIS)
{
    return new ServerSocket();
}

namespace NETWORK
{
	SOUI_COM_C BOOL SOUI_COM_API SCreateInstance(IObjRef **ppObj)
	{
		*ppObj = new SNetwork();
		return TRUE;
	}
}

SNSEND

EXTERN_C BOOL Network_SCreateInstance(IObjRef **ppObj)
{
    *ppObj = new SNS::SNetwork();
    return TRUE;
}
