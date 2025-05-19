#include "OutStreamWrapper.h"


namespace SevenZip
{
namespace intl
{

OutStreamWrapper::OutStreamWrapper(FILE* fp)
	: m_refCount( 0 )
	, m_pFile( fp )
{
}

OutStreamWrapper::~OutStreamWrapper()
{
	if (m_pFile) {
		fclose(m_pFile);
	}
}

HRESULT STDMETHODCALLTYPE OutStreamWrapper::QueryInterface( REFIID iid, void** ppvObject )
{ 
	if ( iid == IID_IUnknown )
	{
		*ppvObject = static_cast< IUnknown* >( this );
		AddRef();
		return S_OK;
	}

	if ( iid == IID_ISequentialOutStream )
	{
		*ppvObject = static_cast< ISequentialOutStream* >( this );
		AddRef();
		return S_OK;
	}

	if ( iid == IID_IOutStream )
	{
		*ppvObject = static_cast< IOutStream* >( this );
		AddRef();
		return S_OK;
	}
	
	return E_NOINTERFACE;
}

ULONG STDMETHODCALLTYPE OutStreamWrapper::AddRef()
{
	return ++m_refCount;
}

ULONG STDMETHODCALLTYPE OutStreamWrapper::Release()
{
	ULONG newRefCount = --m_refCount;
	if (newRefCount == 0)
	{
		delete this;
		return 0;
	}
	return newRefCount;
}

STDMETHODIMP OutStreamWrapper::Write( const void* data, UInt32 size, UInt32* processedSize )
{
	size_t count = fwrite(data, size, 1, m_pFile);
	if (count >= 0) {
		if (processedSize != NULL)
		{
			*processedSize = count;
		}
	}
	return count ? S_OK : E_FAIL;
}

STDMETHODIMP OutStreamWrapper::Seek( Int64 offset, UInt32 seekOrigin, UInt64* newPosition )
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

STDMETHODIMP OutStreamWrapper::SetSize( UInt64 newSize )
{
#ifdef _WIN32
	int fd = _fileno(m_pFile);
	HANDLE hFile = (HANDLE)_get_osfhandle(fd);
	if (hFile == INVALID_HANDLE_VALUE) {
		return E_FAIL;
	}
	else {
		SetFilePointer(hFile, newSize, NULL, FILE_BEGIN);
		if (!SetEndOfFile(hFile)) {
			return E_FAIL;
		}
	}
#else
	int fd = fileno(m_pFile);
	if (ftruncate(fd, newSize)) {
		return E_FAIL;
	}
#endif

	return S_OK;
}

}
}
