#include "../../system_util.h"
#include <winsock2.h>
#include <windows.h>
#include "../../socket.h"
#include <Ws2tcpip.h>
#include <iphlpapi.h>
#include <stdlib.h>
#include <helper/slog.h>
#define kLogTag "SystemUtil"

SNSBEGIN

void SystemUtil::sleep(unsigned int msec)
{
    ::Sleep(msec);
}

uint64_t SystemUtil::getCPUTime()
{
    LARGE_INTEGER t;
    LARGE_INTEGER f;
    QueryPerformanceCounter(&t);
    QueryPerformanceFrequency (&f);
    double fdou = f.QuadPart / (double)1000;
    return (uint64_t)(t.QuadPart / fdou);
}

uint64_t SystemUtil::getCPUTimeInMicroseconds()
{
    LARGE_INTEGER t;
    LARGE_INTEGER f;
    QueryPerformanceCounter(&t);
    QueryPerformanceFrequency (&f);
    double fdou = f.QuadPart / (double)1000000;
    return (uint64_t)(t.QuadPart / fdou);
}

uint64_t SystemUtil::getCPUTimeIn100Nanoseconds()
{
    LARGE_INTEGER t;
    LARGE_INTEGER f;
    QueryPerformanceCounter(&t);
    QueryPerformanceFrequency (&f);
    double fdou = f.QuadPart / (double)10000000;
    return (uint64_t)(t.QuadPart / fdou);
}

std::vector<LocalIPInfo> SystemUtil::getLocalIPList()
{
    std::vector<LocalIPInfo> ipInfoList;

    ULONG family = AF_INET;
    ULONG flags = GAA_FLAG_INCLUDE_PREFIX;
    ULONG outputBufferLength = 0;

    PIP_ADAPTER_ADDRESSES localAddresses = NULL;
    PIP_ADAPTER_ADDRESSES currentAddress = NULL;
    PIP_ADAPTER_UNICAST_ADDRESS pUnicast = NULL;


    if (GetAdaptersAddresses(family, flags, NULL, NULL, &outputBufferLength) == ERROR_BUFFER_OVERFLOW)
    {
        localAddresses = new IP_ADAPTER_ADDRESSES[outputBufferLength];
    }

    if (NULL == localAddresses)
    {
        SLOG_FMT(kLogTag,LOG_ERROR, "can't allocate memory for localAddresses!");
        return ipInfoList;
    }

    if (NO_ERROR != GetAdaptersAddresses(family, flags, NULL, localAddresses, &outputBufferLength))
    {
        SLOG_FMT(kLogTag,LOG_ERROR, "get error when GetAdaptersAddresses()!");
        return ipInfoList;
    }

    currentAddress = localAddresses;

    int totalInterface = 0;

    while (NULL != currentAddress)
    {
        ++totalInterface;
        currentAddress = currentAddress->Next;
    }

    currentAddress = localAddresses;

    sockaddr_in *addr;

    for (int index = 0; ((index < totalInterface) && (NULL != currentAddress)); index++)
    {
        bool interfaceRunning = (currentAddress->OperStatus == IfOperStatusUp);

        if (false == interfaceRunning )
        {
            currentAddress = currentAddress->Next;
            continue;
        }

        std::string  interfaceType;

        switch (currentAddress->IfType)
        {
        case IF_TYPE_SOFTWARE_LOOPBACK:
        {
            currentAddress = currentAddress->Next;
            continue;
        }
        break;

        case IF_TYPE_ETHERNET_CSMACD:
            interfaceType = "en0";
            break;

        case IF_TYPE_IEEE80211:
            interfaceType = "en1";
            break;

        case IF_TYPE_TUNNEL:
            interfaceType = "tunnel";
            break;

        default:
            interfaceType = "";
            break;
        }

        pUnicast = currentAddress->FirstUnicastAddress;

        while ((pUnicast != NULL) )
        {
            LocalIPInfo ipInfo;

            while ((pUnicast != NULL) && (pUnicast->Address.lpSockaddr->sa_family != AF_INET))
            {
                pUnicast = pUnicast->Next;
            }

            addr = (sockaddr_in *)pUnicast->Address.lpSockaddr;

            char ipStr[256] = {0};
            inet_ntop_win(addr->sin_family, &addr->sin_addr, ipStr, sizeof(ipStr));

            ipInfo.IP = ipStr;
            ipInfo.name = interfaceType;


            if (((addr->sin_addr.s_addr & htonl(0xFFFFFF00)) != htonl(0x7F000000))
                    && ((addr->sin_addr.s_addr & htonl(0xFFFF0000)) != htonl(0xA9FE0000)))
            {
                ipInfoList.push_back(ipInfo);
            }

            pUnicast = pUnicast->Next;
        }

        currentAddress = currentAddress->Next;
    }

    return ipInfoList;
}

std::string SystemUtil::getIPAddrFromDomain(std::string domain)
{
    WORD wVersionRequested;
    WSADATA wsaData;

    wVersionRequested = MAKEWORD( 2, 2 );

    if ( WSAStartup( wVersionRequested, &wsaData ) != 0 )
    {
        SLOG_FMT(kLogTag,LOG_ERROR, "WSAStartup failed!\n");
        return "";
    }

    struct addrinfo *res0 = NULL, *res = NULL;

    const struct sockaddr_in *v4addr = NULL;

    int rc = getaddrinfo(domain.c_str(), NULL, NULL, &res0);

    std::string strAddr = "";

    if (rc == 0 && res0)
    {
        for (res = res0; res; res = res->ai_next)
        {
            if (!res->ai_addr )
            {
                continue;
            }

            if (res->ai_addr->sa_family != AF_INET)
            {
                continue;
            }

            v4addr = reinterpret_cast <const struct sockaddr_in *> (res->ai_addr);
            char szAddr[32] = {0};

            if (inet_ntop_win(AF_INET, (void *)(&v4addr->sin_addr), szAddr, sizeof(szAddr)))
            {
                strAddr = szAddr;
                break;
            }
        }

        freeaddrinfo(res0);
    }

    WSACleanup();
    SLOG_FMT(kLogTag,LOG_DEBUG, "get IP Address=%s\n", strAddr.c_str());
    return strAddr;
}

bool SystemUtil::getDeviceInfo(std::string &manufacture, std::string &deviceId)
{
    return false;
}

std::string SystemUtil::getAppDataFolder()
{
    return ".";
}

std::vector<FileInfo> SystemUtil::getFileList(std::string inputDir, bool recursive)
{
    // TODO
    std::vector<FileInfo> files;
    return files;
}

bool SystemUtil::removeFile(std::string fileName)
{
    // TODO
    return true;
}

std::string SystemUtil::getDefaultGateway()
{
    // TODO
    return "";
}

SNSEND
