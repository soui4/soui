#include "Zip7Archive.h"
#include <assert.h>
#include <string/sstringa.h>
#include <string/sstringw.h>
#include <string/strcpcvt.h>

#include "SevenZip/SevenZipExtractor.h"
#include "SevenZip/SevenZipExtractorMemory.h"
#include "SevenZip/SevenZipLister.h" 

#include <shlwapi.h>
#pragma comment(lib, "shlwapi.lib")
#include <crtdbg.h>
#include <tchar.h>
#include <malloc.h>

using namespace SOUI;

namespace SevenZip{

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
	 
	BOOL CZipArchive::Open(LPCTSTR pszFileName,LPCSTR pszPassword)
	{
		SStringW strPsw = S_CA2W(pszPassword,CP_UTF8);
		TString s_pwd = strPsw.c_str();
		SevenZip::SevenZipPassword pwd(true, s_pwd);
		SevenZip::SevenZipExtractorMemory decompress;
		SStringW strFilename = S_CT2W(pszFileName);
		decompress.SetArchivePath(strFilename.c_str());
		 
		return (S_OK == decompress.ExtractArchive(m_fileStreams, NULL, &pwd));
	}

	BOOL CZipArchive::Open(HMODULE hModule, LPCTSTR pszName, LPCSTR pszPassword, LPCTSTR pszType)
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

		SStringW strPsw = S_CA2W(pszPassword,CP_UTF8);
		TString s_pwd = strPsw.c_str();
		SevenZip::SevenZipPassword pwd(true, s_pwd);
		SevenZip::SevenZipExtractorMemory decompress;
		decompress.SetArchiveData(hResData,dwLength);

		return (S_OK == decompress.ExtractArchive(m_fileStreams, NULL, &pwd));
	}

	void CZipArchive::CloseFile()
	{ 
	}

	DWORD CZipArchive::ReadFile(void* pBuffer, DWORD dwBytes)
	{
		DWORD dwRead = 0;
		 

		return dwRead;
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