#pragma once

#include <string>
#if defined(_LIBCPP_VERSION) || defined(_MSC_VER)
////GCC 5.0 add codecvt
#include <codecvt>
#elif defined(__GNUC__) && __GNUC__ >= 5
#include <codecvt>
#else
#include <bits/codecvt.h>
#endif

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

	std::string ToString(const std::wstring& wstring, const std::locale& locale = std::locale{ "" });
	std::wstring ToWstring(const std::string& string, const std::locale& locale = std::locale{ "" });
	std::string ToU8string(const std::wstring &wstring, const std::locale& locale = std::locale{ "" });
}
