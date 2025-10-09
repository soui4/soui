﻿#include "ArchiveExtractCallbackMemory.h"
#include "PropVariant2.h"
#include "FileSys.h"
#include "OutStreamWrapperMemory.h"
#include "FileStreamMemory.h"
#include "SevenString.h"

namespace SevenZip
{
	namespace intl
	{

		const TString EmptyFileAlias = _T("[Content]");


		ArchiveExtractCallbackMemory::ArchiveExtractCallbackMemory(
			const CMyComPtr< IInArchive >& archiveHandler,
			ProgressCallback* callback,
			CFileStream &fileStreams)
			: m_refCount(0)
			, m_archiveHandler(archiveHandler)
			, m_callback(callback)
			, m_totalSize(0)
			, PasswordIsDefined(false)
			, _fileStreams(fileStreams)
		{ 

		}

		ArchiveExtractCallbackMemory::~ArchiveExtractCallbackMemory()
		{
		}

		STDMETHODIMP ArchiveExtractCallbackMemory::QueryInterface(REFIID iid, void** ppvObject)
		{
			if (iid == IID_IUnknown)
			{
				*ppvObject = reinterpret_cast<IUnknown*>(this);
				AddRef();
				return S_OK;
			}

			if (iid == IID_IArchiveExtractCallback)
			{
				*ppvObject = static_cast<IArchiveExtractCallback*>(this);
				AddRef();
				return S_OK;
			}

			if (iid == IID_ICryptoGetTextPassword)
			{
				*ppvObject = static_cast<ICryptoGetTextPassword*>(this);
				AddRef();
				return S_OK;
			}

			if (iid == IID_ICompressProgressInfo)
			{
				*ppvObject = static_cast<ICompressProgressInfo*>(this);
				AddRef();
				return S_OK;
			}

			return E_NOINTERFACE;
		}

		STDMETHODIMP_(ULONG) ArchiveExtractCallbackMemory::AddRef()
		{
			return InterlockedIncrement(&m_refCount);
		}

		STDMETHODIMP_(ULONG) ArchiveExtractCallbackMemory::Release()
		{
			ULONG newRefCount = InterlockedDecrement(&m_refCount);
			if (newRefCount == 0)
			{
				delete this;
				return 0;
			}
			return newRefCount;
		}

		STDMETHODIMP ArchiveExtractCallbackMemory::SetTotal(UInt64 size)
		{
#ifdef _DEBUG
			//wprintf(L"SetTotal:%llu\n", size);
#endif // _DEBUG
			m_totalSize = size;
			//	- SetTotal is never called for ZIP and 7z formats
			if (m_callback)
			{
				m_callback->OnStart(m_absPath, size);
			}
			return S_OK;
		}

		STDMETHODIMP ArchiveExtractCallbackMemory::SetCompleted(const UInt64* completeValue)
		{
#ifdef _DEBUG
			//wprintf(L"SetCompleted:%llu\n", *completeValue);
#endif // _DEBUG
			completeValue;
			//Callback Event calls
			/*
			NB:
			- For ZIP format SetCompleted only called once per 1000 files in central directory and once per 100 in local ones.
			- For 7Z format SetCompleted is never called.
			*/
			if (m_callback)
			{
				//Don't call this directly, it will be called per file which is more consistent across archive types
				//TODO: incorporate better progress tracking
				//m_callback->OnProgress(m_absPath, *completeValue);
			}
			return S_OK;
		}

		STDMETHODIMP ArchiveExtractCallbackMemory::SetRatioInfo(const UInt64 *inSize, const UInt64 *outSize)
		{
#ifdef _DEBUG
			//wprintf(L"SetRatioInfo:%llu-%llu\n", *inSize, *outSize);
#endif // _DEBUG
			if (m_callback)
				m_callback->OnRadio(*inSize, *outSize);
			return S_OK;
		}


		STDMETHODIMP ArchiveExtractCallbackMemory::GetStream(UInt32 index, ISequentialOutStream** outStream, Int32 askExtractMode)
		{
			try
			{
				// Retrieve all the various properties for the file at this index.
				GetPropertyFilePath(index);
				if (askExtractMode != NArchive::NExtract::NAskMode::kExtract)
				{
					return S_OK;
				}

				GetPropertyAttributes(index);
				GetPropertyIsDir(index);
				GetPropertyModifiedTime(index);
				GetPropertyCreatedTime(index);
				GetPropertyAccessedTime(index);
				GetPropertySize(index);
			}
			catch (/*_com_error& ex*/...)
			{
				//return ex.Error();
				return S_FALSE;
			}

			// TODO: m_directory could be a relative path as "..\"
			m_absPath = m_relPath;

			if (m_isDir)
			{
				*outStream = NULL;
				return S_OK;
			}

			if (m_callback)
			{
				if (!m_callback->OnFileBegin(_T(""), m_relPath))
				{
					//stop decompress
					return E_FAIL;
				}
				//Skip file
				if (m_relPath.empty())
				{
					*outStream = NULL;
					return S_OK;
				}
			}
			 
			CMyComPtr< IMyStream > fileStream;
#ifdef _UNICODE
			std::wstring path = m_absPath;
#else
			std::wstring path = ToWstring(m_absPath);
#endif//_UNICODE
			if (S_OK != FileStreamMemory::OpenFile(path, _fileStreams, &fileStream) || fileStream == NULL){
                m_absPath.clear();
                return HRESULT_FROM_WIN32(GetLastError());
            }

			OutStreamWrapperMemory * stream = new OutStreamWrapperMemory(fileStream);
			if (!stream){
				return E_OUTOFMEMORY;
			}

			CMyComPtr< OutStreamWrapperMemory > wrapperStream = stream;
			*outStream = wrapperStream.Detach();

            return S_OK;
        }

        STDMETHODIMP ArchiveExtractCallbackMemory::PrepareOperation(Int32 /*askExtractMode*/)
        {
            return S_OK;
        }

        STDMETHODIMP ArchiveExtractCallbackMemory::SetOperationResult(Int32 operationResult)
        {
			m_outMemStream.Release();

            if (operationResult != S_OK)
            {
                //_ASSERT_EXPR(FALSE,L"begin rollback");
                bool succ = ProcessRollBack(); succ;
                //_ASSERT_EXPR(succ, L"rollback error!");
                return E_FAIL;
            }
            

            if (m_absPath.empty())
            {
                if (m_callback)
                    m_callback->OnEnd(_T(""));
                return S_OK;
            }
			  

            if (m_callback)
            {
                if(!m_callback->OnFileDone(m_absPath, m_newFileSize))
                    return E_FAIL;
                m_callback->OnProgress(m_absPath, m_newFileSize);
            }
            return S_OK;
        }

        STDMETHODIMP ArchiveExtractCallbackMemory::CryptoGetTextPassword(BSTR* password)
        {
			if (!PasswordIsDefined)
			{
				// You can ask real password here from user
				// Password = GetPassword(OutStream);
				// PasswordIsDefined = true;
				printf("Password is not defined");
				return E_ABORT;
			}
			return StringToBstr(Password, password);
        }

        void ArchiveExtractCallbackMemory::GetPropertyFilePath(UInt32 index)
        {
            CPropVariant prop;
            HRESULT hr = m_archiveHandler->GetProperty(index, kpidPath, &prop);
            if (hr != S_OK)
            {
                // _com_issue_error(hr);
            }

            if ( prop.vt == VT_EMPTY )
            {
                m_relPath = EmptyFileAlias;
            }
            else if (prop.vt != VT_BSTR)
            {
                // _com_issue_error(E_FAIL);
            }
            else
            {
                _bstr_t bstr = prop.bstrVal;
#ifdef _UNICODE
                m_relPath = bstr;
#else
                char relPath[MAX_PATH];
                int size = WideCharToMultiByte(CP_UTF8, 0, bstr, bstr.length(), relPath, MAX_PATH, NULL, NULL);
                m_relPath.assign(relPath, size);
#endif
            }
        }

        void ArchiveExtractCallbackMemory::GetPropertyAttributes(UInt32 index)
        {
            CPropVariant prop;
            HRESULT hr = m_archiveHandler->GetProperty(index, kpidAttrib, &prop);
            if (hr != S_OK)
            {
                // _com_issue_error(hr);
            }

            if (prop.vt == VT_EMPTY)
            {
                m_attrib = 0;
                m_hasAttrib = false;
            }
            else if (prop.vt != VT_UI4)
            {
                // _com_issue_error(E_FAIL);
            }
            else
            {
                m_attrib = prop.ulVal;
                m_hasAttrib = true;
            }
        }

        void ArchiveExtractCallbackMemory::GetPropertyIsDir(UInt32 index)
        {
            CPropVariant prop;
            HRESULT hr = m_archiveHandler->GetProperty(index, kpidIsDir, &prop);
            if (hr != S_OK)
            {
                // _com_issue_error(hr);
            }

            if (prop.vt == VT_EMPTY)
            {
                m_isDir = false;
            }
            else if (prop.vt != VT_BOOL)
            {
                // _com_issue_error(E_FAIL);
            }
            else
            {
                m_isDir = prop.boolVal != VARIANT_FALSE;
            }
        }

        void ArchiveExtractCallbackMemory::GetPropertyModifiedTime(UInt32 index)
        {
            CPropVariant prop;
            HRESULT hr = m_archiveHandler->GetProperty(index, kpidMTime, &prop);
            if (hr != S_OK)
            {
                // _com_issue_error(hr);
            }

            if (prop.vt == VT_EMPTY)
            {
//                m_hasModifiedTime = false;
            }
            else if (prop.vt != VT_FILETIME)
            {
                // _com_issue_error(E_FAIL);
            }
            else
            {
 //               m_modifiedTime = prop.filetime;
//                m_hasModifiedTime = true;
            }
        }
        void ArchiveExtractCallbackMemory::GetPropertyAccessedTime(UInt32 index)
        {
            CPropVariant prop;
            HRESULT hr = m_archiveHandler->GetProperty(index, kpidATime, &prop);
            if (hr != S_OK)
            {
                // _com_issue_error(hr);
            }

            if (prop.vt == VT_EMPTY)
            {
                //m_hasAccessedTime = false;
            }
            else if (prop.vt != VT_FILETIME)
            {
                // _com_issue_error(E_FAIL);
            }
            else
            {
//                m_accessedTime = prop.filetime;
//                m_hasAccessedTime = true;
            }
        }

        void ArchiveExtractCallbackMemory::GetPropertyCreatedTime(UInt32 index)
        {
            CPropVariant prop;
            HRESULT hr = m_archiveHandler->GetProperty(index, kpidCTime, &prop);
            if (hr != S_OK)
            {
                // _com_issue_error(hr);
            }

            if (prop.vt == VT_EMPTY)
            {
//                m_hasCreatedTime = false;
            }
            else if (prop.vt != VT_FILETIME)
            {
                // _com_issue_error(E_FAIL);
            }
            else
            {
  //              m_createdTime = prop.filetime;
  //              m_hasCreatedTime = true;
            }
        }

        void ArchiveExtractCallbackMemory::GetPropertySize(UInt32 index)
        {
            CPropVariant prop;
            HRESULT hr = m_archiveHandler->GetProperty(index, kpidSize, &prop);
            if (hr != S_OK)
            {
                // _com_issue_error(hr);
            }

            switch (prop.vt)
            {
            case VT_EMPTY:
                m_hasNewFileSize = false;
                return;
            case VT_UI1:
                m_newFileSize = prop.bVal;
                break;
            case VT_UI2:
                m_newFileSize = prop.uiVal;
                break;
            case VT_UI4:
                m_newFileSize = prop.ulVal;
                break;
            case VT_UI8:
                m_newFileSize = (UInt64)prop.uhVal.QuadPart;
                break;
            default:
				;// _com_issue_error(E_FAIL);
            }

            m_hasNewFileSize = true;
        }


        bool ArchiveExtractCallbackMemory::ProcessRollBack()
        {
            bool succ = true;
            for (size_t i = 0; i< m_rollbacks.size(); ++i)
            {
                RollBack_Info& a = m_rollbacks[0];
                if (m_callback)
                    m_callback->OnRollBack(a.original_path);

                if (a.backup_path.empty())
                    FileSys::RemovePath(a.original_path);
                else
                    succ = FileSys::RenameFile(a.backup_path, a.original_path);
            }
            return succ;
        }
    }
}
