#ifndef _FILE_STREAM_MEMORY_H
#define _FILE_STREAM_MEMORY_H

#include "FileSys.h"
#include "FileStream.h"

namespace SevenZip
{

class FileStreamMemory : public IStream  
{  
private:
	FileStreamMemory(const std::wstring &fileName, CFileStream &streams);
	~FileStreamMemory();
public:
  
	HRESULT static OpenFile(const std::wstring &fileName, CFileStream &streams, IStream ** ppStream)
	{
		*ppStream = new FileStreamMemory(fileName, streams);
		 
		return S_OK;
	}
public:
    STDMETHOD(QueryInterface)(REFIID iid, void ** ppvObject)  ;
    STDMETHOD_(ULONG, AddRef)();
    STDMETHOD_(ULONG, Release)();
    
    // ISequentialStream Interface  
public:  
    STDMETHOD(Read)(void* pv, ULONG cb, ULONG* pcbRead);  
    STDMETHOD(Write)(void const* pv, ULONG cb, ULONG* pcbWritten)  ;
  
    // IStream Interface  
public:  
    STDMETHOD(SetSize)(ULARGE_INTEGER);  
    STDMETHOD(CopyTo)(IStream*, ULARGE_INTEGER, ULARGE_INTEGER*, ULARGE_INTEGER*);  
    STDMETHOD(Commit)(DWORD);
    STDMETHOD(Revert)(void);  
    STDMETHOD(LockRegion)(ULARGE_INTEGER, ULARGE_INTEGER, DWORD);
    STDMETHOD(UnlockRegion)(ULARGE_INTEGER, ULARGE_INTEGER, DWORD);
    STDMETHOD(Clone)(IStream **); 
    STDMETHOD(Seek)(LARGE_INTEGER liDistanceToMove, DWORD dwOrigin, ULARGE_INTEGER* lpNewFilePointer); 
    STDMETHOD(Stat)(STATSTG* pStatstg, DWORD grfStatFlag);  
private:  
    LONG _refcount;  
    BlobBuffer _blob;
	unsigned long _blob_read_pos;
    CFileStream &_streams;
	std::wstring _fileName;
};  

}
#endif