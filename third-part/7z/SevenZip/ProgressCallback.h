#pragma once
#include "CompressionFormat.h"
#include "SevenString.h"
#include <vector>
#include <stdint.h>

namespace SevenZip
{
    class ProgressCallback
    {
    public:

        virtual bool OnFileCount(uint32_t /*fileCount*/){ return false; };

        virtual bool OnFileItems(std::vector<std::wstring>&/*itemNames*/, std::vector<uint64_t>&/*itemSizes*/){ return true; };
        /*
        开始: 压缩/解压
        totalSize : 待压缩文件总大小  /   压缩包大小? or 待解压文件总大小?
        */
        virtual void OnStart(const TString& /*filePath*/, uint64_t /*totalSize*/) {}

        /*
        数值进度 : 压缩/解压
        inSize   : 已处理文件总大小  / 已处理压缩包大小
        outSize  : 生成压缩包大小    / 解压出来文件总大小
        */
        virtual void OnRadio(uint64_t  /*inSize*/, uint64_t /*outSize*/) {}

        /*
        文件进度 : 压缩/解压
        filePath       : 文件
        bytesCompleted : 已处理大小
        */
        virtual void OnProgress(const TString& /*filePath*/, uint64_t /*bytesCompleted*/) {}
        
        /*
        压缩/解压完成
        ArchiveFilePath : 压缩包路径
        */
        virtual void OnEnd(const TString& /*ArchiveFilePath*/) {}


        /*
        解压文件开始
        destFolder : 目标文件夹
        ItemPath   : 相对路径
        return     : 是否继续解压
        */
        virtual bool OnFileBegin(const TString& /*destFolder*/, TString&  /*ItemPath*/) { return true; };

        /*
        解压文件结束
        filePath       : 文件路径
        bytesCompleted : 文件尺寸
        return         : 是否继续解压
        */
        virtual bool OnFileDone(const TString& /*filePath*/, uint64_t /*bytesCompleted*/) { return true; }
        
        /*
        回滚文件进度
        filePath       : 文件路径
        return         : 是否继续回滚
        */
        virtual void OnRollBack(const TString& /*filePath*/){}

    };
}
