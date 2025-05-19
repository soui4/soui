﻿#ifndef __ZIPARCHIVE_H__
#define __ZIPARCHIVE_H__

#pragma once

#ifdef _WIN32
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif // !WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <tchar.h>
#else
#include "SevenZip/../CPP/Common/MyWindows.h"

#define CopyMemory(Destination, Source, Length)  memcpy((Destination), (Source), (Length))

#define FILE_BEGIN   0
#define FILE_CURRENT 1
#define FILE_END     2
typedef DWORD *LPDWORD;
typedef BYTE *LPBYTE;

#endif
#include "SevenZip/FileStream.h"

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

	BOOL Open(LPCTSTR pszFileName, LPCSTR pszPassword);
#ifdef _WIN32
	BOOL Open(HMODULE hModule, LPCTSTR pszName, LPCSTR pszPassword, LPCTSTR pszType = _T("7Z"));
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
private:
	CFileStream m_fileStreams;
};

}//end of ns
#endif	//	__ZIP7ARCHIVE_H__
