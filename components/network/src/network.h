#pragma once

#include <interface/network-i.h>
#include <helper/obj-ref-impl.hpp>
SNSBEGIN

class SNetwork : public TObjRefImpl<INetwork>
{
public:
    SNetwork();
    ~SNetwork();

    STDMETHOD_(ITcpClientSocket *, CreateTcpClientSocket)(THIS) OVERRIDE;
    STDMETHOD_(IUdpSocket *, CreateUdpSocket)(THIS) OVERRIDE;
    STDMETHOD_(ITcpServerSocket *, CreateTcpServerSocket)(THIS) OVERRIDE;
};

namespace NETWORK
{
	SOUI_COM_C BOOL SOUI_COM_API SCreateInstance(IObjRef **ppObj);
}

SNSEND

EXTERN_C BOOL SOUI_COM_API Network_SCreateInstance(IObjRef **ppObj);
