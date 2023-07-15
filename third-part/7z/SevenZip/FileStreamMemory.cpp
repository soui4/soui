#include "FileStreamMemory.h"

namespace SevenZip
{
	static std::string WStringToString(const wchar_t* pStr, int len, unsigned int CodePage)
	{
		std::string buf;

		if (pStr == NULL)
		{
			return buf;
		}

		if (len < 0 && len != -1)
		{
			return buf;
		}

		// figure out how many narrow characters we are going to get
		int nChars = WideCharToMultiByte(CodePage, 0,
			pStr, len, NULL, 0, NULL, NULL);
		if (len == -1)
			--nChars;
		if (nChars == 0)
			return "";

		// convert the wide string to a narrow string
		// nb: slightly naughty to write directly into the string like this
		buf.resize(nChars);
		WideCharToMultiByte(CodePage, 0, pStr, len,
			const_cast<char*>(buf.c_str()), nChars, NULL, NULL);

		return buf;
	}


    FileStreamMemory::FileStreamMemory(const std::wstring &fileName,CFileStream &streams)
		:_streams(streams), _fileName(fileName), _refcount(1), _blob_read_pos(0)
    {   
    }  
  
    FileStreamMemory::~FileStreamMemory()  
    {  
		if (_fileName.length()>0)
		{
			std::string file_name = WStringToString(_fileName.c_str(), _fileName.length(), CP_ACP);
			_streams.PutFile(file_name.c_str(), _blob); 			
		}
		else
		{
			_fileName = _fileName;
		}
    }  
    
    STDMETHODIMP FileStreamMemory::QueryInterface(REFIID iid, void ** ppvObject)  
    {   
        if (iid == __uuidof(IUnknown)  
            || iid == __uuidof(IStream)  
            || iid == __uuidof(ISequentialStream))  
        {  
            *ppvObject = static_cast<IStream*>(this);  
            AddRef();  
            return S_OK;  
        } else  
            return E_NOINTERFACE;   
    }  
  
    STDMETHODIMP_(ULONG) FileStreamMemory::AddRef(void)   
    {   
        return (ULONG)InterlockedIncrement(&_refcount);   
    }  
  
    STDMETHODIMP_(ULONG) FileStreamMemory::Release(void)   
    {  
        ULONG res = (ULONG) InterlockedDecrement(&_refcount);  
        if (res == 0)   
            delete this;  
        return res;  
    }  
     
    STDMETHODIMP FileStreamMemory::Read(void* pv, ULONG cb, ULONG* pcbRead)  
    { 
		if (cb + _blob_read_pos >= _blob.GetBlobLength())
			cb = _blob.GetBlobLength() - _blob_read_pos;

		*pcbRead = cb;

        memcpy((unsigned char*)pv,_blob.GetBlobPtr(),cb);  

        return S_OK;  
    }  
  
    STDMETHODIMP FileStreamMemory::Write(void const* pv, ULONG cb, ULONG* pcbWritten)  
    {   
		_blob.AppendBlobContent((const unsigned char *)pv,cb);
		*pcbWritten = cb; 

		_blob_read_pos += cb;
        return S_OK;  
    }  
   
    STDMETHODIMP FileStreamMemory::SetSize(ULARGE_INTEGER)  
    {   
        return E_NOTIMPL;     
    }  
  
    STDMETHODIMP FileStreamMemory::CopyTo(IStream*, ULARGE_INTEGER, ULARGE_INTEGER*, ULARGE_INTEGER*)   
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
  
    STDMETHODIMP FileStreamMemory::Clone(IStream **)                                    
    {   
        return E_NOTIMPL;     
    }  
  
    STDMETHODIMP FileStreamMemory::Seek(LARGE_INTEGER liDistanceToMove, DWORD dwOrigin, ULARGE_INTEGER* lpNewFilePointer)  
    {   
        DWORD dwMoveMethod;  
  
        switch(dwOrigin)  
        {  
        case STREAM_SEEK_SET:  
            dwMoveMethod = FILE_BEGIN;  
			_blob_read_pos = 0; 
            break;  
        case STREAM_SEEK_CUR:  
            dwMoveMethod = FILE_CURRENT;  
			break;  
        case STREAM_SEEK_END:  
			dwMoveMethod = FILE_END;
			_blob_read_pos = _blob.GetBlobLength();
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
		size.QuadPart = _blob.GetBlobLength();
		pStatstg->cbSize = size;

        return S_OK;  
    } 
  
}