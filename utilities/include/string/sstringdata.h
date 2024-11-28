#ifndef __SSTRINGDATA__H__
#define __SSTRINGDATA__H__
#include "utilities-def.h"

SNSBEGIN

struct TStringData
{
	int nRefs;            // Reference count: negative == locked
	int nDataLength;    // Length of currently used data in XCHARs (not including terminating null)
	int nAllocLength;    // Length of allocated data in XCHARs (not including terminating null)

	void* data() const;

	void AddRef();
	void Release();
	bool IsShared() const;
	bool IsLocked() const;

	void Lock();
	void Unlock();

	static TStringData* InitDataNil();
	static const void*  InitPszNil();
};

SNSEND
#endif // __SSTRINGDATA__H__