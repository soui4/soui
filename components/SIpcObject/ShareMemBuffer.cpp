// MemMapFile.cpp: implementation of the CMemMapFile class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "ShareMemBuffer.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
SNSBEGIN

CShareMemBuffer::CShareMemBuffer()
	:m_pMemBuf(NULL)
	,m_pHeader(NULL)
	,m_pBuffer(NULL)
	,m_hMap(NULL)
	,m_hMutex(NULL)
{
}

CShareMemBuffer::~CShareMemBuffer()
{
	Close();
}

BOOL CShareMemBuffer::OpenMemFile(LPCTSTR pszName,DWORD dwMaximumSize , void * pSecurityAttr)
{
	if(m_hMap) return FALSE;
	SECURITY_ATTRIBUTES *psa = (SECURITY_ATTRIBUTES*)pSecurityAttr;
	TCHAR szMutex[MAX_PATH];
	_stprintf(szMutex, _T("%s_mutex"), pszName);
	if (dwMaximumSize == 0)
	{
		m_hMap = OpenFileMapping(FILE_MAP_READ | FILE_MAP_WRITE, FALSE, pszName);
		m_hMutex = OpenEvent(SYNCHRONIZE, FALSE, szMutex);
	}
	else
	{
		m_hMap = CreateFileMapping(INVALID_HANDLE_VALUE, psa, PAGE_READWRITE, 0, dwMaximumSize + sizeof(BufHeader), pszName);
		m_hMutex = CreateEvent(psa, FALSE, TRUE, szMutex);
	}
	if (!m_hMap || !m_hMutex)	goto error;
	m_pMemBuf=::MapViewOfFile(m_hMap, FILE_MAP_READ | FILE_MAP_WRITE,0,0,0);//map whole file
	if (!m_pMemBuf) goto error;
	m_pHeader = (BufHeader*)m_pMemBuf;
	m_pBuffer = (LPBYTE)(m_pHeader + 1);
	if (dwMaximumSize != 0)
	{//init header.
		BufHeader *pHeader = GetHeader();
		pHeader->dwSize = dwMaximumSize;
		pHeader->dwTailPos = 0;
		pHeader->dwPos = 0;
	}
	return TRUE;
error:
	if (m_hMap)
	{
		::CloseHandle(m_hMap);
		m_hMap = 0;
	}
	if(m_hMutex)
	{
		CloseHandle(m_hMutex);
		m_hMutex=0;
	}
	return FALSE;
}

void CShareMemBuffer::Close(){
	if(m_pMemBuf)
	{
		::UnmapViewOfFile(m_pMemBuf);
		m_pBuffer = NULL;
		m_pHeader = NULL;
		m_pMemBuf = NULL;
		::CloseHandle(m_hMap);
		m_hMap = 0;
	}
	if (m_hMutex)
	{
		CloseHandle(m_hMutex);
		m_hMutex = NULL;
	}
}

int CShareMemBuffer::Write(const void * pBuf, UINT nLen)
{
	assert(GetBuffer());
	BufHeader * pHeader = GetHeader();

	UINT nRemain = pHeader->dwSize - pHeader->dwPos;
	if (nLen > nRemain) nLen = nRemain;
	memcpy(GetBuffer() + pHeader->dwPos, pBuf, nLen);
	pHeader->dwPos += nLen;
	if (pHeader->dwTailPos < pHeader->dwPos)
		pHeader->dwTailPos = pHeader->dwPos;
	return nLen;
}

int CShareMemBuffer::Read(void * pBuf, UINT nLen)
{
	assert(GetBuffer());
	BufHeader * pHeader = GetHeader();
	UINT nRemain = (pHeader->dwTailPos - pHeader->dwPos);
	if (nLen > nRemain) nLen = nRemain;
	memcpy(pBuf, GetBuffer() + pHeader->dwPos, nLen);
	pHeader->dwPos += nLen;
	return nLen;
}

UINT CShareMemBuffer::Tell() const
{
	return GetHeader()->dwPos;
}

UINT CShareMemBuffer::Seek(SEEK mode, int nOffset)
{
	BufHeader *pHeader = GetHeader();
	switch (mode)
	{
	case seek_cur:
		nOffset += pHeader->dwPos;
		break;
	case seek_end:
		nOffset += pHeader->dwTailPos;
		break;
	case seek_set:
	default:
		break;
	}
	assert(nOffset >= 0 && nOffset < (int)pHeader->dwSize);
	if (nOffset > (int)pHeader->dwTailPos)
	{//auto expend buffer used size.
		pHeader->dwTailPos = nOffset;
	}
	pHeader->dwPos = nOffset;
	return pHeader->dwPos;
}

void CShareMemBuffer::SetTail(UINT uPos)
{
	BufHeader *pHeader = GetHeader();
	assert(uPos <= pHeader->dwSize);
#ifdef _DEBUG
	if(uPos<pHeader->dwTailPos)//
		memset(GetBuffer() + uPos,0, pHeader->dwTailPos-uPos);
	else
		memset(GetBuffer() + pHeader->dwTailPos, 0, uPos - pHeader->dwTailPos);
#endif
	pHeader->dwTailPos = uPos;
	if (pHeader->dwPos > uPos) pHeader->dwPos = uPos;
}

BOOL CShareMemBuffer::Lock(DWORD timeout)
{
	return WaitForSingleObject(m_hMutex, timeout) == WAIT_OBJECT_0;
}

void CShareMemBuffer::Unlock()
{
	SetEvent(m_hMutex);//make mutex waitable.
}

SNSEND
