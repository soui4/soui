#include "InStreamWrapper.h"


namespace SevenZip
{
namespace intl
{
    InStreamWrapper::InStreamWrapper(FILE* fp)
        : m_refCount(1), m_pFile(fp)
    {
		if (m_pFile) {
			fseek(m_pFile, 0, SEEK_END);
			m_nFileSize = ftell(m_pFile);
			fseek(m_pFile, 0, SEEK_SET);
		}
    }

    InStreamWrapper::~InStreamWrapper()
    {
		if (m_pFile) {
			fclose(m_pFile);
		}
    }

    STDMETHODIMP InStreamWrapper::QueryInterface(REFIID iid, void** ppvObject)
    {
        if (iid == IID_IUnknown ||
            iid == IID_ISequentialInStream ||
            iid == IID_IInStream ||
            iid == IID_IStreamGetSize)
        {
            *ppvObject = static_cast<void*>(this);
            AddRef();
            return S_OK;
        }
        else
        {
            *ppvObject = NULL;
            return E_NOINTERFACE;
        }
    }

    STDMETHODIMP_(ULONG) InStreamWrapper::AddRef()
    {
		return InterlockedIncrement(&m_refCount);
    }

    STDMETHODIMP_(ULONG) InStreamWrapper::Release()
    {
		ULONG newRefCount = InterlockedDecrement(&m_refCount);
        if (newRefCount == 0)
        {
            delete this;
            return 0;
        }
        return newRefCount;
    }

    STDMETHODIMP InStreamWrapper::Read(void* data, UInt32 size, UInt32* processedSize)
    {
        if (processedSize) {
            *processedSize = 0;
        }

		size_t count = fread(data, 1, size, m_pFile);

		if (count >= 0) {
			if (processedSize != NULL)
				*processedSize = count;
		}

        if (processedSize) {
            *processedSize = static_cast<UInt32>(count);
        }

        return count ? S_OK : E_FAIL;
    }

    STDMETHODIMP InStreamWrapper::Seek(Int64 offset, UInt32 seekOrigin, UInt64* newPosition)
    {
#if _WIN32
		int result = _fseeki64(m_pFile, (long)offset, seekOrigin);
#else
		int result = fseek(m_pFile, (long)offset, seekOrigin);
#endif

		if (!result) {
			if (newPosition)
				*newPosition = ftell(m_pFile);

			return S_OK;
		}

        return E_FAIL;
    }

    STDMETHODIMP InStreamWrapper::GetSize(UInt64* size)
    {
        if (!size)
        {
            return E_FAIL;
        }

		if (size)
			*size = m_nFileSize;
        return S_OK;
    }

}
}
