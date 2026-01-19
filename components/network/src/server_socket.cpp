#include "server_socket.h"
#include <cassert>

#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#else
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#endif

#include "tcp_client_socket.h"
#include <helper/slog.h>
#define kLogTag "ServerSocket"


#define CHECK_SOCKET_OR_RETURN(socket,ret) \
    if (_socket == INVALID_SOCKET) { \
        SLOGE()<<"server socket not created, operation failed"; \
        return ret;\
    }

SNSBEGIN

ServerSocket::ServerSocket() :
    Socket(),
    _clientSockets()
{
}

ServerSocket::~ServerSocket()
{
    clearTcpClientSockets();
}

bool ServerSocket::create(const std::string &ip, uint16_t port)
{
    if (_socket >= 0)
    {
        SLOG_FMT("ServerSocket", LOG_WARN,
                         "server socket is already created: ip=%s, port=%d",
                         this->address().c_str(),
                         this->port());
        return false;
    }

    _socket = createSocket(ip, port, true, false);
    return (_socket != INVALID_SOCKET);
}

void ServerSocket::close()
{
    clearTcpClientSockets();
    Socket::close();
}

bool ServerSocket::listen(unsigned int count) const
{
    CHECK_SOCKET_OR_RETURN(_socket, false);

    assert(count > 0);

    int ret = ::listen(_socket, count);

    if (ret != 0)
    {
        SLOG_FMT("ServerSocket", LOG_ERROR,
                         "server socket=%d listen failed, ip=%s, port=%d, error=%d",
                         _socket,
                         address().c_str(),
                         port(),
                         getError());
        return false;
    }

    return true;
}

TcpClientSocket *ServerSocket::accept()
{
    CHECK_SOCKET_OR_RETURN(_socket, NULL);

    clearClosedTcpClientSockets();

    sockaddr_in clientAddr;
    socklen_t addrLen = sizeof(clientAddr);
    int clientSock = ::accept(_socket, (sockaddr *)&clientAddr, &addrLen);

    if (clientSock == INVALID_SOCKET)
    {
        SLOG_FMT("ServerSocket", LOG_ERROR,
                         "server socket=%d accept failed, ip=%s, port=%d, error=%d",
                         _socket,
                         address().c_str(),
                         port(),
                         getError());
        return NULL;
    }

    char *clientIP = inet_ntoa(clientAddr.sin_addr);
    uint16_t clientPort = ntohs(clientAddr.sin_port);
    TcpClientSocket *newClientSock = new TcpClientSocket(clientSock,
                                 clientIP,
                                 clientPort);

    _clientSockets.push_back(newClientSock);
    newClientSock->Release();
    return newClientSock;
}

void ServerSocket::clearClosedTcpClientSockets()
{
    std::list<SAutoRefPtr<TcpClientSocket>>::iterator iter = _clientSockets.begin();

    while (iter != _clientSockets.end())
    {
        std::list<SAutoRefPtr<TcpClientSocket>>::iterator cur = iter;
        SAutoRefPtr<TcpClientSocket> client = *iter;
        ++ iter;

        if (client->address() == "")
        {
            _clientSockets.erase(cur);
        }
    }
}

void ServerSocket::clearTcpClientSockets()
{
    std::list<SAutoRefPtr<TcpClientSocket>>::iterator iter = _clientSockets.begin();

    for (; iter != _clientSockets.end(); ++ iter)
    {
        (*iter)->close();
    }

    _clientSockets.clear();
}

std::vector<SAutoRefPtr<TcpClientSocket>> ServerSocket::clients() const
{
    std::vector<SAutoRefPtr<TcpClientSocket>> clients;
    std::list<SAutoRefPtr<TcpClientSocket>>::const_iterator iter = _clientSockets.begin();

    for (; iter != _clientSockets.end(); ++ iter)
    {
        SAutoRefPtr<TcpClientSocket> client = *iter;

        if (client->address() != "")
        {
            clients.push_back(client);
        }
    }

    return clients;
}

// ITcpServerSocket interface implementation
STDMETHODIMP_(bool) ServerSocket::Create(THIS_ const char *ip, uint16_t port)
{
    std::string strIp(ip);
    return create(strIp, port);
}

STDMETHODIMP_(void) ServerSocket::Close(THIS)
{
    close();
}

STDMETHODIMP_(bool) ServerSocket::Listen(THIS_ unsigned int count)
{
    return listen(count);
}

STDMETHODIMP_(ITcpClientSocket *) ServerSocket::Accept(THIS)
{
    return accept();
}

STDMETHODIMP_(int) ServerSocket::GetClientSocketCount(CTHIS) SCONST
{
    return _clientSockets.size();
}

STDMETHODIMP_(int) ServerSocket::GetClientSocketsRef(THIS_ ITcpClientSocket **sockets, unsigned int count) const
{
    std::vector<SAutoRefPtr<TcpClientSocket>> vec = clients();
    if(count > vec.size())
    {
        count = vec.size();
    }

    std::vector<SAutoRefPtr<TcpClientSocket>>::iterator iter = vec.begin();
    for(unsigned int i = 0; i < count; ++i,++iter)
    {
        sockets[i] = iter->Get();
        sockets[i]->AddRef();
    }

    return count;
}
SNSEND
