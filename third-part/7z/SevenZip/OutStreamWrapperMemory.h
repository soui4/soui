#pragma once

#include "../CPP/7zip/IStream.h"
#include "../CPP/Common/MyCom.h"
#include "FileStreamMemory.h"
#include <atomic>

namespace SevenZip
{
namespace intl
{
	class OutStreamWrapperMemory : public IOutStream
	{
	private:

		std::atomic<long>		m_refCount;
		CMyComPtr< IMyStream >	m_baseStream;

	public:

		OutStreamWrapperMemory( const CMyComPtr< IMyStream >& baseStream );
		virtual ~OutStreamWrapperMemory();

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
