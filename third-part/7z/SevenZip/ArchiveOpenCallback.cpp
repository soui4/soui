#include "ArchiveOpenCallback.h"
#include "../CPP/Common/MyCom.h"

namespace SevenZip
{
namespace intl
{

ArchiveOpenCallback::ArchiveOpenCallback()
	: m_refCount(0), PasswordIsDefined(false)
{
}

ArchiveOpenCallback::~ArchiveOpenCallback()
{
}

STDMETHODIMP ArchiveOpenCallback::QueryInterface( REFIID iid, void** ppvObject )
{
	if ( iid == IID_IUnknown )
	{
		*ppvObject = reinterpret_cast< IUnknown* >( this );
		AddRef();
		return S_OK;
	}

	if ( iid == IID_IArchiveOpenCallback )
	{
		*ppvObject = static_cast< IArchiveOpenCallback* >( this );
		AddRef();
		return S_OK;
	}

	if ( iid == IID_ICryptoGetTextPassword )
	{
		*ppvObject = static_cast< ICryptoGetTextPassword* >( this );
		AddRef();
		return S_OK;
	}

	return E_NOINTERFACE;
}

STDMETHODIMP_(ULONG) ArchiveOpenCallback::AddRef()
{
	return ++m_refCount;
}

STDMETHODIMP_(ULONG) ArchiveOpenCallback::Release()
{
	ULONG newRefCount = --m_refCount;
	if (newRefCount == 0)
	{
		delete this;
		return 0;
	}
	return newRefCount;
}

STDMETHODIMP ArchiveOpenCallback::SetTotal( const UInt64* /*files*/, const UInt64* /*bytes*/ )
{
	return S_OK;
}

STDMETHODIMP ArchiveOpenCallback::SetCompleted( const UInt64* /*files*/, const UInt64* /*bytes*/ )
{
	return S_OK;
}

STDMETHODIMP ArchiveOpenCallback::CryptoGetTextPassword( BSTR* password)
{
	if (!PasswordIsDefined)
	{
		// You can ask real password here from user
		// Password = GetPassword(OutStream);
		// PasswordIsDefined = true;
		//printf("Password is not defined");
		return E_ABORT;
	}

	return StringToBstr(Password, password); 
}

}
}
