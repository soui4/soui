#pragma once
#ifdef _WIN32
#include <InitGuid.h>
#endif // _WIN32
#include "../CPP/7zip/IStream.h"
#include "../CPP/7zip/IDecl.h"
#include "../CPP/Common/MyWindows.h"

namespace SevenZip
{
#ifndef _WIN32
	typedef struct tagSTATSTG {
		ULARGE_INTEGER cbSize;
	} STATSTG;
#endif

	//STREAM_INTERFACE(IMyStream, 0x0A)
	DEFINE_GUID(IID_IMyStream, k_7zip_GUID_Data1, k_7zip_GUID_Data2, k_7zip_GUID_Data3_Common, 0, 0, 0, (3), 0, (0x0A), 0, 0);
	struct IMyStream : public IUnknown
	{
		// IUnknown
		STDMETHOD_(ULONG, AddRef)(void) PURE;
		STDMETHOD_(ULONG, Release)(void) PURE;

		// ISequentialStream
		STDMETHOD(Read)(void *pv, ULONG cb, ULONG *pcbRead) PURE;
		STDMETHOD(Write)(const void *pv, ULONG cb, ULONG *pcbWritten) PURE;

		// IStream
		STDMETHOD(Seek)(LARGE_INTEGER dlibMove, DWORD dwOrigin, ULARGE_INTEGER *plibNewPosition) PURE;
		STDMETHOD(Commit)(DWORD grfCommitFlags) PURE;
		STDMETHOD(Stat)(STATSTG *pstatstg, DWORD grfStatFlag) PURE;
	};
}

