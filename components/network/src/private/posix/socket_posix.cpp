#include "../../socket.h"
#include <unistd.h>
#include <cerrno>

SNSBEGIN
void Socket::prepareSocket()
{
}

bool Socket::closeSocket(int socket)
{
    return ::close(socket) == 0;
}

int Socket::getError()
{
    return errno;
}

SNSEND
