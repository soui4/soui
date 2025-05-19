#pragma once
#include "SevenZipArchive.h"
#include "CompressionFormat.h"
#include "ProgressCallback.h"
#include "SevenZipPwd.h"
#include "FileStream.h"
#include "../CPP/7zip/IStream.h"

namespace SevenZip
{

    class SevenZipExtractorMemory : public SevenZipArchive
    {
    public: 
        SevenZipExtractorMemory();
        virtual ~SevenZipExtractorMemory();

		virtual HRESULT ExtractArchive(CFileStream &fileStreams, ProgressCallback* callback, SevenZipPassword *pSevenZipPassword = NULL);
        const TString& GetErrorString(); 
    private:

		HRESULT ExtractArchive(CFileStream &fileStreams, FILE* archiveStream,  ProgressCallback* callback, SevenZipPassword *pSevenZipPassword);
		 
        TString m_message;
    };
}
