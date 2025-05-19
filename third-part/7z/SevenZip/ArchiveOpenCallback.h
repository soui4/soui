#pragma once


#include "../CPP/7zip/Archive/IArchive.h"
#include "../CPP/7zip/IPassword.h"
#include "../CPP/Common/MyString.h"
#include <atomic>

namespace SevenZip
{
namespace intl
{
	class ArchiveOpenCallback : public IArchiveOpenCallback, public ICryptoGetTextPassword
	{
	public: 
		bool PasswordIsDefined;
		UString Password;
	private:

		std::atomic<long> m_refCount;
	public:

		ArchiveOpenCallback();
		virtual ~ArchiveOpenCallback();

		STDMETHOD(QueryInterface)( REFIID iid, void** ppvObject );
		STDMETHOD_(ULONG, AddRef)();
		STDMETHOD_(ULONG, Release)();

		// IArchiveOpenCallback
		STDMETHOD(SetTotal)( const UInt64* files, const UInt64* bytes );
		STDMETHOD(SetCompleted)( const UInt64* files, const UInt64* bytes );

		// ICryptoGetTextPassword
		STDMETHOD(CryptoGetTextPassword)( BSTR* password );
	};
}
}
