#pragma once
#include <stdio.h>
#include <stdint.h>
#include <atomic>
#include "../CPP/7zip/IStream.h"
#include "../CPP/Common/MyCom.h"
#ifdef _WIN32
#include <io.h>
#include <windows.h>
#else
#include <unistd.h>
#endif

namespace SevenZip
{
namespace intl
{
	class OutStreamWrapper : public IOutStream
	{
	private:

		std::atomic<long>	m_refCount;
		FILE*				m_pFile;

	public:

		OutStreamWrapper(FILE* fp);
		virtual ~OutStreamWrapper();

		STDMETHOD(QueryInterface)( REFIID iid, void** ppvObject );
		STDMETHOD_(ULONG, AddRef)();
		STDMETHOD_(ULONG, Release)();

		// ISequentialOutStream
		STDMETHOD(Write)( const void* data, UInt32 size, UInt32* processedSize );

		// IOutStream
		STDMETHOD(Seek)( Int64 offset, UInt32 seekOrigin, UInt64* newPosition );
		STDMETHOD(SetSize)( UInt64 newSize );
	};
}
}
