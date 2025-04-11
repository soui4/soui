#ifndef __ZIPARCHIVE_H__
#define __ZIPARCHIVE_H__

#pragma once
#include <string>
#include <vector>
#include <bit7z/bit7zlibrary.hpp>
#include <bit7z/bitarchivereader.hpp>
#include <bit7z/bitmemextractor.hpp>
#include <bit7z/bittypes.hpp>
#include "SevenZip/FileStream.h"
#include <tchar.h>
#include <malloc.h>

namespace SevenZip{

class CZipFile;
class CZipArchive;

//	ZIP file wrapper from zip archive
class CZipFile
{
	friend CZipArchive;
protected:
	DWORD	m_dwPos;
public:
	CZipFile(DWORD dwSize=0);
	~CZipFile();
	BOOL Read(void* pBuffer, DWORD dwSize, LPDWORD pdwRead = NULL);
	BOOL Close();
	BOOL IsOpen() const ;
	BYTE* GetData() ;
	DWORD GetSize() const;
	DWORD GetPosition() const;
	DWORD Seek(DWORD dwOffset, UINT nFrom);

	BOOL Attach(LPBYTE pData, DWORD dwSize);
	void Detach();
	BlobBuffer &getBlob();
protected: 
	BlobBuffer m_blob;
};

//	ZIP Archive class, load files from a zip archive
class CZipArchive
{
public:
	CZipArchive();
	~CZipArchive();

	BOOL Open(LPCTSTR pszFileName, LPCTSTR pszPassword);
#ifdef _WIN32
	BOOL Open(HMODULE hModule, LPCTSTR pszName, LPCTSTR pszPassword, LPCTSTR pszType = _T("7Z"));
#endif // _WIN32

	void Close();
	BOOL IsOpen() const;
		 
	BOOL GetFile(LPCTSTR pszFileName, CZipFile& file); 
	DWORD GetFileSize(LPCTSTR pszFileName);
	BOOL IsFileExist( LPCTSTR pszFileName );
	
	int GetFileCount();
	unsigned int GetFirstFilePos();
	unsigned int GetNextFilePos(unsigned int pos);
	bool Eof(unsigned int pos);

	std::string GetFileName(unsigned int pos);
protected:
	void CloseFile();

	DWORD ReadFile(void* pBuffer, DWORD dwBytes);

	bit7z::Bit7zLibrary& Get7zLibrary();
private:
	CFileStream m_fileStreams;
	bit7z::Bit7zLibrary m_lib;
};

}//end of ns
#endif	//	__ZIP7ARCHIVE_H__
