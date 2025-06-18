#include "FileStreamMemory.h"
#include "SevenString.h"

namespace SevenZip
{
    FileStreamMemory::FileStreamMemory(const std::wstring &fileName,CFileStream &streams)
		:m_streams(streams), m_fileName(fileName), m_refCount(1), m_blobReadPos(0)
    {   
    }  
  
    FileStreamMemory::~FileStreamMemory()  
    {  
		if (m_fileName.length()>0)
		{
			std::string file_name = ToPathNativeStr(m_fileName);
			m_streams.PutFile(file_name.c_str(), m_blob); 			
		}
    }  
    
    STDMETHODIMP FileStreamMemory::QueryInterface(REFIID iid, void ** ppvObject)  
    {   
        if (iid == IID_IUnknown
            || iid == IID_IMyStream)
        {  
            *ppvObject = static_cast<IMyStream*>(this);  
            AddRef();  
            return S_OK;  
        } else  
            return E_NOINTERFACE;   
    }  
  
    STDMETHODIMP_(ULONG) FileStreamMemory::AddRef(void)   
    {   
		return InterlockedIncrement(&m_refCount);
    }  
  
    STDMETHODIMP_(ULONG) FileStreamMemory::Release(void)   
    {  
		ULONG newRefCount = InterlockedDecrement(&m_refCount);
		if (newRefCount == 0)
		{
			delete this;
			return 0;
		}
		return newRefCount;
    }  
     
    STDMETHODIMP FileStreamMemory::Read(void* pv, ULONG cb, ULONG* pcbRead)  
    { 
		if (cb + m_blobReadPos >= m_blob.GetBlobLength())
			cb = m_blob.GetBlobLength() - m_blobReadPos;

		*pcbRead = cb;

        memcpy((unsigned char*)pv,m_blob.GetBlobPtr(),cb);  

        return S_OK;  
    }  
  
    STDMETHODIMP FileStreamMemory::Write(void const* pv, ULONG cb, ULONG* pcbWritten)  
    {   
		m_blob.AppendBlobContent((const unsigned char *)pv,cb);
		*pcbWritten = cb; 

		m_blobReadPos += cb;
        return S_OK;  
    }  
   
    STDMETHODIMP FileStreamMemory::SetSize(ULARGE_INTEGER)  
    {   
        return E_NOTIMPL;     
    }  
  
    STDMETHODIMP FileStreamMemory::CopyTo(IMyStream*, ULARGE_INTEGER, ULARGE_INTEGER*, ULARGE_INTEGER*)
    {   
        return E_NOTIMPL;     
    }  
  
    STDMETHODIMP FileStreamMemory::Commit(DWORD)                                        
    {   
        return E_NOTIMPL;     
    }  
  
    STDMETHODIMP FileStreamMemory::Revert(void)                                         
    {   
        return E_NOTIMPL;     
    }  
  
    STDMETHODIMP FileStreamMemory::LockRegion(ULARGE_INTEGER, ULARGE_INTEGER, DWORD)                
    {   
        return E_NOTIMPL;     
    }  
  
    STDMETHODIMP FileStreamMemory::UnlockRegion(ULARGE_INTEGER, ULARGE_INTEGER, DWORD)              
    {   
        return E_NOTIMPL;     
    }  
  
    STDMETHODIMP FileStreamMemory::Clone(IMyStream **)
    {   
        return E_NOTIMPL;     
    }  
  
    STDMETHODIMP FileStreamMemory::Seek(LARGE_INTEGER liDistanceToMove, DWORD dwOrigin, ULARGE_INTEGER* lpNewFilePointer)  
    {   
        switch(dwOrigin)  
        {  
        case STREAM_SEEK_SET:  
			m_blobReadPos = 0; 
            break;  
        case STREAM_SEEK_CUR:  
			break;  
        case STREAM_SEEK_END:  
			m_blobReadPos = m_blob.GetBlobLength();
            break;  
        default:     
            return STG_E_INVALIDFUNCTION;  
            break;  
        }  
  
        return S_OK;  
    }  
  
    STDMETHODIMP FileStreamMemory::Stat(STATSTG* pStatstg, DWORD grfStatFlag)   
    {  
		ULARGE_INTEGER size;
		size.QuadPart = m_blob.GetBlobLength();
		pStatstg->cbSize = size;

        return S_OK;  
    } 
  
}