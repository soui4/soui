#include "../../system_util.h"
#include <time.h>
#include <netdb.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <unistd.h>
#include <cerrno>
#include <dirent.h>
#include <arpa/inet.h>
#include "ifaddrs.h"
#include "../../common_defs.h"
#include <helper/slog.h>
#define kLogTag "SystemUtil"

SNSBEGIN

void SystemUtil::sleep(unsigned int msec)
{
    struct timespec delay, remain;
    delay.tv_sec = msec / 1000;
    delay.tv_nsec = (msec % 1000) * 1000000ll;

    int ret = nanosleep(&delay, &remain);

    while (ret != 0)
    {
        delay = remain;
        ret = nanosleep(&delay, &remain);
    }
}

std::vector<LocalIPInfo> SystemUtil::getLocalIPList()
{
    std::vector<LocalIPInfo> ret;
    struct ifaddrs *interfaces = NULL;
    struct ifaddrs *temp_addr = NULL;
    int success = 0;

    // retrieve the current interfaces - returns 0 on success
    success = getifaddrs(&interfaces);

    if (0 == success)
    {
        // Loop through linked list of interfaces
        temp_addr = interfaces;

        while (temp_addr != NULL)
        {
            if (temp_addr->ifa_addr->sa_family == AF_INET)
            {
                // Check if interface is en0 which is the wifi connection on the iPhone
                char ipStr[256] = {0};
                struct sockaddr_in *addr = (struct sockaddr_in *)temp_addr->ifa_addr;
                inet_ntop(addr->sin_family, &addr->sin_addr, ipStr, sizeof(ipStr));

                LocalIPInfo info;
                info.name = temp_addr->ifa_name;
                info.IP = ipStr;
                ret.push_back(info);
            }

            temp_addr = temp_addr->ifa_next;
        }
    }

    // Free memory
    freeifaddrs(interfaces);

    return ret;
}

std::string SystemUtil::getLocalIpByExternalIp(std::string externalIp, uint16_t port)
{
    struct sockaddr_in dstAddr = {0};
    struct sockaddr_in srcAddr = {0};
    int sProbe = 0;
    int lenOfSrcAddr = sizeof(srcAddr);
    char *pSrcIp = NULL;
    int on = 1;

    sProbe = socket(AF_INET, SOCK_DGRAM, 0);

    if (-1 == sProbe)
    {
        return "";
    }

    dstAddr.sin_family = AF_INET;
    dstAddr.sin_port = htons(port);
    dstAddr.sin_addr.s_addr = inet_addr(externalIp.c_str());

    if (setsockopt(sProbe, SOL_SOCKET, SO_BROADCAST, &on, sizeof(on)) == -1)
    {
        ::close (sProbe);
        return "";
    }

    if (connect(sProbe, (struct sockaddr *)&dstAddr, sizeof(struct sockaddr_in)) == -1)
    {
        ::close(sProbe);
        return "";
    }

    if (-1 == getsockname(
                sProbe,
                (struct sockaddr *)&srcAddr,
                (socklen_t *)&lenOfSrcAddr
            ))
    {
        ::close(sProbe);
        return "";
    }

    pSrcIp = inet_ntoa(srcAddr.sin_addr);

    if (!pSrcIp)
    {
        ::close(sProbe);
        return "";
    }

    ::close(sProbe);

    return pSrcIp;
}

std::string SystemUtil::getIPAddrFromDomain(std::string domain)
{
    if (true == domain.empty())
    {
        SLOG_FMT(kLogTag,LOG_ERROR, "SYS domain name resolution failed, it is empty");
        return "";
    }

    // Structure for return value of gethostbyname.
    struct hostent *hptr;

    // Try to call gethostbyname
    if (NULL == (hptr = gethostbyname(domain.c_str())))
    {
        SLOG_FMT(kLogTag,LOG_ERROR, "SYS domain name resolution failed, can not get ip address");
        return "";
    }

    char szAddr[32] = {0};

    switch (hptr->h_addrtype)
    {
    case AF_INET:
    {
        inet_ntop(hptr->h_addrtype, *(hptr->h_addr_list), szAddr, sizeof(szAddr));
        SLOG_FMT(kLogTag,LOG_DEBUG, "SYS domain name resolution result=%s", szAddr);
        std::string strAddr = szAddr;
        return (strAddr.compare("0.0.0.0") == 0 ? "" : strAddr);
    }

    default:
        break;
    }

    return "";
}

std::vector<FileInfo> SystemUtil::getFileList(std::string inputDir, bool recursive)
{
    std::vector<FileInfo> ret;

    DIR *dir;
    struct dirent *ptr;
    dir = opendir(inputDir.c_str());

    if (dir == NULL)
    {
        SLOG_FMT(kLogTag,LOG_ERROR, "SYS open dir=%s failed", inputDir.c_str());
        return ret;
    }

    errno = 0;

    while ( (ptr = readdir (dir)) != NULL)
    {
        if (strcmp (ptr->d_name, ".") == 0 || strcmp (ptr->d_name, "..") == 0)
        {
            continue;
        }

        if (ptr->d_type == DT_DIR && recursive)
        {
            std::vector<FileInfo> retRec = getFileList(inputDir, recursive);
            ret.insert(ret.end(), retRec.begin(), retRec.end());
        }

        std::string fullName = inputDir + "/" + std::string(ptr->d_name);

        struct stat statbuf;

        if (stat(fullName.c_str(), &statbuf) == -1)
        {
            printf("Get stat on %s Error：%s\n", fullName.c_str(), strerror(errno));
            break;
        }

        if (S_ISDIR(statbuf.st_mode))
        {
            continue;
        }

        if (S_ISREG(statbuf.st_mode))
        {
            FileInfo info;
            info.fileName = std::string(ptr->d_name);
            info.fileSize = (int)statbuf.st_size;
            info.createTime = statbuf.st_ctime;
            info.modifyTime = statbuf.st_mtime;
            ret.push_back(info);
        }

    }

    if (errno != 0)
    {
        SLOG_FMT(kLogTag,LOG_ERROR, "SYS read dir=%s failed", inputDir.c_str());
        closedir (dir);
        return std::vector<FileInfo>();
    }

    closedir (dir);

    return ret;
}

bool SystemUtil::removeFile(std::string fileName)
{
    bool isFile = false;

    struct stat statbuf;

    if (lstat(fileName.c_str(), &statbuf) == 0)
    {
        isFile = S_ISREG(statbuf.st_mode) != 0;
    }

    if (!isFile)
    {
        return false;
    }

    int ret = remove(fileName.c_str());

    return ret == 0;
}

SNSEND
