#pragma once

#include <stdint.h>
#include <string>
#include <vector>
#include <sdef.h>
SNSBEGIN
struct LocalIPInfo
{
    std::string name;
    std::string IP;
};

struct FileInfo
{
    std::string fileName;
    int fileSize;
    uint64_t createTime;
    uint64_t modifyTime;
};

/**
 * System API utility.
 */
class SystemUtil
{
public:
    /**
     * Sleep in current thread for a while.
     * @param msec sleep time in milliseconds.
     */
    static void sleep(unsigned int msec);

    /**
     * Get milliseconds from CPU is started. This time should not change with
     * BIOS time or current time zone.
     * @return milliseconds from CPU is started.
     */
    static uint64_t getCPUTime();

    /**
     * Get microseconds from CPU is started. This time should not change with
     * BIOS time or current time zone.
     * @return microseconds from CPU is started.
     */
    static uint64_t getCPUTimeInMicroseconds();

    /**
     * Get 100 nanoseconds from CPU is started. This time should not change with
     * BIOS time or current time zone.
     * @return 100 nanoseconds from CPU is started.
     */
    static uint64_t getCPUTimeIn100Nanoseconds();

    /**
     * Get the application data folder.
     */
    static std::string getAppDataFolder();

    /**
     * Get the interface name and ip for all interface
     */
    static std::vector<LocalIPInfo> getLocalIPList();

    /**
     * Get local IP as for the external IP.
     * this function will find the proper local ip used for
     * sending data to the "externalIp" and port "port"
     */
    static std::string getLocalIpByExternalIp(std::string externalIp, uint16_t port);

    static std::string getIPAddrFromDomain(std::string domain);

    static std::string getDefaultGateway();

    static bool isIPv4(const char *addr);

    /**
     * Get the device manufacture and device id.
     */
    static bool getDeviceInfo(std::string &manufacture, std::string &deviceId);

    static bool copyFile(std::string SourceFile, std::string NewFile);

    static std::vector<FileInfo> getFileList(std::string inputDir, bool recursive);

    static bool removeFile(std::string fileName);

private:
    SystemUtil();
};

SNSEND
