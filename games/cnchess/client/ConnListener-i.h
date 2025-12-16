#ifndef CONNLISTENER_I_H
#define CONNLISTENER_I_H

struct IConnListener2
{
    virtual void OnConnected() = 0;
    virtual void OnDisconnected() = 0;
};

#endif//CONNLISTENER_I_H