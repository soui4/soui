#include "SevenZipExtractorMemory.h"
#include "GUIDs.h"
#include "FileSys.h"
#include "ArchiveOpenCallback.h"
#include "ArchiveExtractCallbackMemory.h"
#include "InStreamWrapper.h"
#include "PropVariant2.h"
#include "UsefulFunctions.h"


namespace SevenZip
{

    using namespace intl;


	//////////////////////////////////////////////////////////////////////////

	class CResStream : public IStream{
	protected:
		int m_nRef;
		const BYTE * m_pBufPtr;
		UINT		 m_szBuf;
		UINT			 m_nPos;

		CResStream(HGLOBAL hRes,DWORD dwSize):m_nRef(0){
			m_pBufPtr = (const BYTE *)LockResource(hRes);
			m_szBuf = dwSize;
			m_nPos = 0;
		}
	public:
		static IStream * CreateResStream(HGLOBAL hRes,DWORD dwSize){
			return new CResStream(hRes,dwSize);
		}
	public:
		virtual HRESULT STDMETHODCALLTYPE QueryInterface( 
			/* [in] */ REFIID riid,
			/* [iid_is][out] */ __RPC__deref_out void __RPC_FAR *__RPC_FAR *ppvObject) {
				HRESULT hRet = S_OK;
				if(riid == IID_IUnknown){
					*ppvObject = this;
					AddRef();
				}else if(riid == IID_ISequentialInStream)
				{
					*ppvObject = this;
					AddRef();
				}else if(riid == IID_IStream){
					*ppvObject = this;
					AddRef();
				}else{
					hRet = E_NOINTERFACE;
				}
				return hRet;
		}

		virtual ULONG STDMETHODCALLTYPE AddRef( void){
			return ++m_nRef;
		}

		virtual ULONG STDMETHODCALLTYPE Release( void){
			if(--m_nRef==0){
				delete this;
			}
			return m_nRef;
		}


		virtual /* [local] */ HRESULT STDMETHODCALLTYPE Read( 
			/* [length_is][size_is][out] */ void *pv,
			/* [in] */ ULONG cb,
			/* [out] */ ULONG *pcbRead){
				UINT remain = m_szBuf-m_nPos;
				if(cb>remain) cb = remain;
				memcpy(pv,m_pBufPtr+m_nPos,cb);
				m_nPos+=cb;
				if(pcbRead) *pcbRead = cb;
				return S_OK;
		}

		virtual /* [local] */ HRESULT STDMETHODCALLTYPE Write( 
			/* [size_is][in] */ const void *pv,
			/* [in] */ ULONG cb,
			/* [out] */ ULONG *pcbWritten){
				return E_FAIL;
		}


		virtual /* [local] */ HRESULT STDMETHODCALLTYPE Seek( 
			/* [in] */ LARGE_INTEGER dlibMove,
			/* [in] */ DWORD dwOrigin,
			/* [out] */ ULARGE_INTEGER *plibNewPosition){
				switch(dwOrigin){
					case STREAM_SEEK_SET:
						m_nPos = (UINT)dlibMove.QuadPart;
						break;
					case STREAM_SEEK_CUR:
						m_nPos += (UINT)dlibMove.QuadPart;
						break;
					case STREAM_SEEK_END:
						m_nPos = m_szBuf+(UINT)dlibMove.QuadPart;
						break;
				}
				if(plibNewPosition){
					plibNewPosition->QuadPart = m_nPos;
				}
				return S_OK;
		}

		virtual HRESULT STDMETHODCALLTYPE SetSize( 
			/* [in] */ ULARGE_INTEGER libNewSize){
				return E_FAIL;
		}

		virtual /* [local] */ HRESULT STDMETHODCALLTYPE CopyTo( 
			/* [unique][in] */ IStream *pstm,
			/* [in] */ ULARGE_INTEGER cb,
			/* [out] */ ULARGE_INTEGER *pcbRead,
			/* [out] */ ULARGE_INTEGER *pcbWritten){
				return E_FAIL;
		}
		virtual HRESULT STDMETHODCALLTYPE Commit( 
			/* [in] */ DWORD grfCommitFlags){
				return E_FAIL;
		}

		virtual HRESULT STDMETHODCALLTYPE Revert( void) {
			return E_FAIL;
		}

		virtual HRESULT STDMETHODCALLTYPE LockRegion( 
			/* [in] */ ULARGE_INTEGER libOffset,
			/* [in] */ ULARGE_INTEGER cb,
			/* [in] */ DWORD dwLockType) {
				return E_FAIL;
		}

		virtual HRESULT STDMETHODCALLTYPE UnlockRegion( 
			/* [in] */ ULARGE_INTEGER libOffset,
			/* [in] */ ULARGE_INTEGER cb,
			/* [in] */ DWORD dwLockType) {
				return E_FAIL;
		}

		virtual HRESULT STDMETHODCALLTYPE Stat( 
			/* [out] */ __RPC__out STATSTG *pstatstg,
			/* [in] */ DWORD grfStatFlag) {
				pstatstg->cbSize.QuadPart = m_szBuf;
				return S_OK;
		}

		virtual HRESULT STDMETHODCALLTYPE Clone( 
			/* [out] */ __RPC__deref_out_opt IStream **ppstm) {
				return E_FAIL;
		}

	};



    SevenZipExtractorMemory::SevenZipExtractorMemory()
        : SevenZipArchive()        
    {
    }

    SevenZipExtractorMemory::~SevenZipExtractorMemory()
    {
    }


	HRESULT SevenZipExtractorMemory::ExtractArchive(CFileStream &fileStreams, ProgressCallback* callback, SevenZipPassword *pSevenZipPassword)
    {
        DetectCompressionFormat();
		
        CMyComPtr< IStream > fileStream;
		if(m_hData)
		{
			fileStream = CResStream::CreateResStream(m_hData,m_dwDataSize);
		}
		else{
			fileStream = FileSys::OpenFileToRead(m_archivePath);
		}
        if (fileStream == NULL)
        {
            LPVOID msgBuf;
            if (::FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER |
                FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
                NULL, ERROR_OPEN_FAILED, 0, (LPTSTR)&msgBuf, 0, NULL) == 0)
            {
                m_message = (LPCTSTR)msgBuf;
                ::LocalFree(msgBuf);
            }
            return ERROR_OPEN_FAILED;	//Could not open archive
        }

		return ExtractArchive(fileStreams,fileStream, callback, pSevenZipPassword);
    }

	HRESULT SevenZipExtractorMemory::ExtractArchive(CFileStream &fileStreams,const CMyComPtr< IStream >& archiveStream, ProgressCallback* callback, SevenZipPassword *pSevenZipPassword)
    {
        CMyComPtr< IInArchive > archive = UsefulFunctions::GetArchiveReader(m_compressionFormat);
        CMyComPtr< InStreamWrapper > inFile = new InStreamWrapper(archiveStream);
        CMyComPtr< ArchiveOpenCallback > openCallback = new ArchiveOpenCallback();
		 
		if (NULL != pSevenZipPassword)
		{ 
			openCallback->PasswordIsDefined = pSevenZipPassword->PasswordIsDefined;
			openCallback->Password = pSevenZipPassword->Password.c_str();
		}

        HRESULT hr = archive->Open(inFile, 0, openCallback);
        if (hr != S_OK)
        {
            LPVOID msgBuf;
            if (::FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER |
                FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
                NULL, hr, 0, (LPTSTR)&msgBuf, 0, NULL) == 0)
            {
                m_message = (LPCTSTR)msgBuf;
                ::LocalFree(msgBuf);
            }
            return hr;	//Open archive error
        }

        if (callback)
        {
            UInt32 mynumofitems = 0;
            hr = archive->GetNumberOfItems(&mynumofitems);
            if (callback->OnFileCount(mynumofitems))
            {
                size_t numberofitems = size_t(mynumofitems);
                
                std::vector<std::wstring> itemnames;
                itemnames.reserve(numberofitems);

                std::vector<unsigned __int64> origsizes;
                origsizes.reserve(numberofitems);

                bool succ = true;
                for (UInt32 i = 0; i < numberofitems; ++i)
                {
                    // Get uncompressed size of file
                    CPropVariant prop;
                    hr = archive->GetProperty(i, kpidSize, &prop);
                    if (hr != S_OK)
                    {
                        succ = false;
                        break;
                    }

                    unsigned __int64 size = prop.uhVal.QuadPart;
                    origsizes.push_back(size);

                    // Get name of file
                    hr = archive->GetProperty(i, kpidPath, &prop);
                    if (hr != S_OK)
                    {
                        succ = false;
                        break;
                    }
                    itemnames.push_back(prop.vt == VT_BSTR?prop.bstrVal:L"");
                }

                if (!succ)
                {
                    archive->Close();
                    return E_FAIL;
                    
                }

                if (!callback->OnFileItems(itemnames, origsizes))
                {
                    //只为了取得文件信息,所以直接返回.
                    archive->Close();
                    return S_OK;
                }
            }
        }

		CMyComPtr< ArchiveExtractCallbackMemory > extractCallback = new ArchiveExtractCallbackMemory(archive, callback,fileStreams);
		if (NULL != pSevenZipPassword)
		{
			extractCallback->PasswordIsDefined = pSevenZipPassword->PasswordIsDefined;
			extractCallback->Password = pSevenZipPassword->Password.c_str();
		}

        hr = archive->Extract(NULL, UInt32(-1), false, extractCallback);
        if (hr != S_OK)
        {
            LPVOID msgBuf;
            if (::FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER |
                FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
                NULL, hr, 0, (LPTSTR)&msgBuf, 0, NULL) == 0)
            {
                m_message = (LPCTSTR)msgBuf;
                ::LocalFree(msgBuf);
            }
        }
        if (callback)
        {
            callback->OnEnd(m_archivePath);
        }
        archive->Close();
        return hr;
    }

    const TString& SevenZipExtractorMemory::GetErrorString()
    {
        return m_message;
    }


}
