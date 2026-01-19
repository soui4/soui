//
//  system_util.cpp
//  dugon
//
//  Created by Andy F on 8/7/14.
//  Copyright (c) 2014 Fan William. All rights reserved.
//

#include "system_util.h"
#include <iostream>
#include <fstream>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#ifndef WIN32
#include <unistd.h>
#endif

#include <stdint.h>
#include <stdarg.h>
#include <string.h>

#if TARGET_OS_MAC
#include <unistd.h>
#endif

using namespace std;

SNSBEGIN
bool SystemUtil::copyFile(std::string sourceFile, std::string newFile)
{
#ifdef WIN32
    ifstream in;
    ofstream out;
    in.open(sourceFile.c_str(), ios::binary);

    if (in.fail())
    {
        cout << "Error 1: Fail to open the source file." << endl;
        in.close();
        out.close();
        return false;
    }

    out.open(newFile.c_str(), ios::binary);

    if (out.fail())
    {
        cout << "Error 2: Fail to create the new file." << endl;
        out.close();
        in.close();
        return false;
    }
    else
    {
        const int bufferSize = 1024 * 1024;
        char *buffer = new char[bufferSize];
        memset(buffer, 0, bufferSize);

        while (in.read(buffer, bufferSize))
        {
            out.write(buffer, bufferSize);
        }

        out.write(buffer, in.gcount());

        in.close();
        out.close();
        delete [] buffer;
        return true;
    }

#else
    int fpIn = 0;
    int fpOut = 0;

    fpIn = open(sourceFile.c_str(), O_RDWR);

    if (fpIn == -1)
    {
        cout << "Error 1: Fail to open the source file." << endl;
        return false;
    }
#ifndef LINUX // ivan
    fpOut = open(newFile.c_str(), O_RDWR | O_CREAT);
#else
    fpOut = open(newFile.c_str(), O_RDWR); 
#endif

    if (fpOut == -1)
    {
        cout << "Error 2: Fail to create the new file." << endl;
        close(fpIn);
        return false;
    }

    const int bufferSize = 10 * 1024 * 1024;
    char *buffer = new char[bufferSize];
    memset(buffer, 0, bufferSize);

    int num = 0;

    while ((num = read(fpIn, buffer, bufferSize)) > 0)
    {
        write(fpOut, buffer, num);
    }

    close(fpIn);
    close(fpOut);

    delete [] buffer;
    return true;
#endif
}

    bool SystemUtil::isIPv4(const char *addr) {
        bool bRet = true;
        int nTmp1, nTmp2, nTmp3, nTmp4, nCount;

        do {
            nCount = sscanf(addr, "%d.%d.%d.%d", &nTmp1, &nTmp2, &nTmp3, &nTmp4);

            if (nCount != 4) {
                bRet = false;
                break;
            }

            if ((nTmp1 > 255) || (nTmp2 > 255) || (nTmp3 > 255) || (nTmp4 > 255)) {
                bRet = false;
                break;
            }

            for (; *addr != 0; addr++) {
                if ((*addr != '.') && ((*addr < '0') || (*addr > '9'))) {
                    bRet = false;
                    break;
                }
            }
        } while (0);

        return bRet;
    }
SNSEND
