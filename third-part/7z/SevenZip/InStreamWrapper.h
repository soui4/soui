#pragma once

#if _WIN32
#include <io.h>
#endif
#include <stdio.h>
#include <stdint.h>
#include <atomic>
#include "../CPP/7zip/IStream.h"
#include "../CPP/Common/MyCom.h"

namespace SevenZip
{
namespace intl
{
	class InStreamWrapper : public IInStream, public IStreamGetSize, private CMyUnknownImp
	{
	private:

		std::atomic<long>	m_refCount;
		FILE*				m_pFile;
		int64_t				m_nFileSize;

	public:

		InStreamWrapper(FILE* fp);
		virtual ~InStreamWrapper();

		STDMETHOD(QueryInterface)(REFIID iid, void** ppvObject);
		STDMETHOD_(ULONG, AddRef)();
		STDMETHOD_(ULONG, Release)();

		// ISequentialInStream
		STDMETHOD(Read)(void* data, UInt32 size, UInt32* processedSize);

		// IInStream
		STDMETHOD(Seek)(Int64 offset, UInt32 seekOrigin, UInt64* newPosition);

		// IStreamGetSize
		STDMETHOD(GetSize)(UInt64* size);

		//MY_UNKNOWN_IMP2(IInStream, IStreamGetSize)
	};
}
}
