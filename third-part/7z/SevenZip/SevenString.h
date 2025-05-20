#pragma once

#include <string>

#ifdef _WIN32
#include <Windows.h>
#include <comdef.h>
#include <tchar.h>
#else
#include "../CPP/Common/MyCom.h"
typedef std::wstring _bstr_t;
#endif

#if defined(_UNICODE)
#if !defined(_WIN32)
#define _T(x) L##x
#endif
#else
#define _T(x) x
#endif

namespace SevenZip
{
#ifdef _UNICODE
	typedef std::wstring TString;
#else
	typedef std::string TString;
#endif
}
namespace SevenZip
{
	std::string ToPathNativeStr(const std::wstring& str);

	std::string ToString(const std::wstring& wstring, int cp=CP_ACP);
	std::wstring ToWstring(const std::string& string, int cp=CP_ACP);
	std::string ToU8string(const std::wstring &wstring);
}
