#include "Zip7Archive.h"
#include <assert.h>
#include <memory>
#include <string/sstringa.h>
#include <string/sstringw.h>
#include <string/strcpcvt.h>

#if ((defined(_MSVC_LANG) && _MSVC_LANG >= 201703L) || (defined(__cplusplus) && __cplusplus >= 201703L)) && defined(__has_include)
#if __has_include(<filesystem>) && (!defined(__MAC_OS_X_VERSION_MIN_REQUIRED) || __MAC_OS_X_VERSION_MIN_REQUIRED >= 101500)
#define GHC_USE_STD_FS
#include <filesystem>
namespace fs = std::filesystem;
#endif
#endif
#ifndef GHC_USE_STD_FS
#include <ghc/filesystem.hpp>
namespace fs = ghc::filesystem;
#endif

#ifndef ZIP_DISABLE_WILDCARD
#include <shlwapi.h>
#pragma comment(lib, "shlwapi.lib")
#endif

#ifndef _WIN32
#define _ASSERTE assert
#endif // _WIN32

using namespace SOUI;

namespace SevenZip{

	inline fs::path GetExePath()
	{
#ifdef _WIN32
		std::array< wchar_t, MAX_PATH > path{ 0 };
		GetModuleFileNameW(nullptr, path.data(), MAX_PATH);
		return fs::path{ path.data() };
#elif defined( __APPLE__ )
		std::array< char, PROC_PIDPATHINFO_MAXSIZE > result{ 0 };
		ssize_t result_size = proc_pidpath(getpid(), result.data(), result.size());
		return (result_size > 0) ? std::string(result.data(), result_size) : "";
#else
		std::error_code error;
		const fs::path result = fs::read_symlink("/proc/self/exe", error);
		return error ? "" : result;
#endif
	}

    CZipFile::CZipFile(DWORD dwSize/*=0*/)
		: m_dwPos(0)
	{

	}

	CZipFile::~CZipFile()
	{
		Close();
	}

	BlobBuffer &CZipFile::getBlob()
	{
		return m_blob;
	}

	BOOL CZipFile::Read(void* pBuffer, DWORD dwSize, LPDWORD pdwRead/* = NULL*/)
	{
		_ASSERTE(IsOpen());

		if (pdwRead != NULL)
			*pdwRead = 0;

		if (m_blob.GetBlobLength()==0)
			return FALSE;

		if (m_dwPos + dwSize > m_blob.GetBlobLength())
			dwSize = m_blob.GetBlobLength() - m_dwPos;

		::CopyMemory(pBuffer, m_blob.GetBlobPtr() + m_dwPos, dwSize);
		m_dwPos += dwSize;
		if (pdwRead != NULL)
			*pdwRead = dwSize;

		return TRUE;
	}
	BOOL CZipFile::Close()
	{
		m_blob.ClearContent();
		m_dwPos = 0;

		return TRUE;
	}
	BOOL CZipFile::IsOpen() const 
	{
		return (m_blob.GetBlobLength() > 0);
	}
	BYTE* CZipFile::GetData() 
	{
		_ASSERTE(IsOpen());
		return m_blob.GetBlobRealPtr();
	}
	DWORD CZipFile::GetSize() const
	{
		_ASSERTE(IsOpen());
		return m_blob.GetBlobLength();
	}
	DWORD CZipFile::GetPosition() const
	{
		_ASSERTE(IsOpen());
		return m_dwPos;
	}
	DWORD CZipFile::Seek(DWORD dwOffset, UINT nFrom)	//	return old pos
	{
		_ASSERTE(IsOpen());
		DWORD dwPos = m_dwPos;
		switch (nFrom)
		{
		case FILE_BEGIN:
			m_dwPos = dwOffset;
			break;
		case FILE_END:
			m_dwPos = m_blob.GetBlobLength() + dwOffset;
			break;
		case FILE_CURRENT:
			m_dwPos += dwOffset;
			break;
		}
		if (m_dwPos < 0)
			m_dwPos = 0;
		if (m_dwPos >= m_blob.GetBlobLength())
			m_dwPos = m_blob.GetBlobLength();
		return dwPos;
	} 

	BOOL CZipFile::Attach(LPBYTE pData, DWORD dwSize)
	{
		_ASSERTE(pData);
		_ASSERTE(!::IsBadReadPtr(pData,dwSize));

		m_blob.SetBlobContent(pData, dwSize);
		return TRUE;
	}

	void CZipFile::Detach()
	{ 
		m_blob.ClearContent();
		m_dwPos = 0;
	}
	//////////////////////////////////////////////////////////////////////////
	//CZipArchive
	//////////////////////////////////////////////////////////////////////////
	
	CZipArchive::CZipArchive()
	{
	}
	CZipArchive::~CZipArchive()
	{
		Close();
	}

	void CZipArchive::Close()
	{
		CloseFile(); 
	}
	BOOL CZipArchive::IsOpen() const
	{
		return TRUE;
	} 

	// ZIP File API

	BOOL CZipArchive::GetFile(LPCTSTR pszFileName, CZipFile& file)
	{
		SStringA fileName = S_CT2A(pszFileName);
		if (m_fileStreams.GetFile(fileName.c_str(),file.getBlob()))
			return TRUE;

		return FALSE;
	}
	 
	BOOL CZipArchive::Open(LPCTSTR pszFileName,LPCTSTR pszPassword)
	{
		try {
			fs::path zipPath(pszFileName);
			if (!fs::is_regular_file(zipPath)) {
				zipPath = GetExePath().parent_path() / zipPath;
			}
			std::unique_ptr<bit7z::BitArchiveReader> archivereader = std::make_unique<bit7z::BitArchiveReader>(Get7zLibrary(), zipPath.native(), bit7z::BitFormat::Auto);
			archivereader->setPassword(pszPassword);
			for (const auto& item : archivereader->items()) {
				if (item.isDir()) continue;
				std::vector<bit7z::byte_t> buffer;
				archivereader->extractTo(buffer, item.index());
				CZipFile zipFile;
				zipFile.Attach(buffer.data(), buffer.size());
#ifdef _WIN32
				m_fileStreams.PutFile(S_CW2A(item.path().c_str()), zipFile.getBlob());
#else
				m_fileStreams.PutFile(item.path().c_str(), zipFile.getBlob());
#endif // _WIN32	
			}
		}
		catch (const bit7z::BitException& e) {
			std::cerr << "Error opening archive: " << e.what() << std::endl;
			return FALSE;
		}

		return TRUE;
	}

#ifdef _WIN32
	BOOL CZipArchive::Open(HMODULE hModule, LPCTSTR pszName, LPCTSTR pszPassword, LPCTSTR pszType)
	{
		HRSRC hResInfo = ::FindResource(hModule, pszName, pszType);
		if (hResInfo == NULL)
			return FALSE;

		DWORD dwLength = ::SizeofResource(hModule, hResInfo);
		if (dwLength == 0)
			return FALSE;

		HGLOBAL hResData = ::LoadResource(hModule, hResInfo);
		if (hResData == NULL)
			return FALSE;

		try {
			std::vector<bit7z::byte_t> data(static_cast<size_t>(dwLength));
			void* dataPtr = LockResource(hResData);
			if (dataPtr != NULL) {
				memcpy(data.data(), dataPtr, dwLength);
			}

			std::unique_ptr<bit7z::BitArchiveReader> archivereader = std::make_unique<bit7z::BitArchiveReader>(Get7zLibrary(), data, bit7z::BitFormat::Auto);
			archivereader->setPassword(pszPassword);
			for (const auto& item : archivereader->items()) {
				if (item.isDir()) continue;
				std::vector<bit7z::byte_t> buffer;
				archivereader->extractTo(buffer, item.index());
				CZipFile zipFile;
				zipFile.Attach(buffer.data(), buffer.size());
				m_fileStreams.PutFile(S_CW2A(item.path().c_str()), zipFile.getBlob());
			}
		}
		catch (const bit7z::BitException& e) {
			std::cerr << "Error opening archive: " << e.what() << std::endl;
			return FALSE;
		}

		return TRUE;
	}
#endif // _WIN32

	void CZipArchive::CloseFile()
	{ 
	}

	DWORD CZipArchive::ReadFile(void* pBuffer, DWORD dwBytes)
	{
		DWORD dwRead = 0;
		 

		return dwRead;
	}

	bit7z::Bit7zLibrary & CZipArchive::Get7zLibrary()
	{
#if defined(_WIN32)
		static bit7z::Bit7zLibrary lib((GetExePath().parent_path() / "7z.dll").string< bit7z::tchar >());
#else
		static bit7z::Bit7zLibrary lib((GetExePath().parent_path() / "7z.so").string());
#endif
		return lib;
	}

	DWORD CZipArchive::GetFileSize( LPCTSTR pszFileName )
	{
		SStringA fileName = S_CT2A(pszFileName);
		return m_fileStreams.GetFileSize(fileName.c_str());
	} 

	BOOL CZipArchive::IsFileExist( LPCTSTR pszFileName )
	{
		SStringA fileName = S_CT2A(pszFileName);
		return m_fileStreams.GetFilePtr(fileName.c_str())!=NULL;
	} 

	int CZipArchive::GetFileCount()
	{
		return m_fileStreams.GetFileCount();
	}

	unsigned int CZipArchive::GetFirstFilePos()
	{
		return m_fileStreams.First();
	}

	unsigned int CZipArchive::GetNextFilePos(unsigned int pos)
	{
		return m_fileStreams.Next(pos);
	}

	bool CZipArchive::Eof(unsigned int pos)
	{
		return m_fileStreams.Eof(pos);
	}

	std::string CZipArchive::GetFileName(unsigned int pos)
	{
		return m_fileStreams.getParamName(pos);
	}


	}//end of ns