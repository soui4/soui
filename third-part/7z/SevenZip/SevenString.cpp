
#include "SevenString.h"
#include <vector>

#define GHC_USE_WCHAR_T

namespace SevenZip
{
	std::string ToPathNativeStr(const std::wstring& str)
	{
		return ToString(str,CP_ACP);
	}

	std::string ToString(const std::wstring& wstring, int cp)
	{
		int len = WideCharToMultiByte(cp,0,wstring.c_str(),wstring.length(),NULL,0,NULL,NULL);
		std::string ret;
		ret.resize(len);
		WideCharToMultiByte(cp,0,wstring.c_str(),wstring.length(),(char*)ret.c_str(),len+1,NULL,NULL);
		return ret;
	}

	std::wstring ToWstring(const std::string& string, int cp)
	{
		int len = MultiByteToWideChar(cp,0,string.c_str(),string.length(),NULL,0);
		std::wstring ret;
		ret.resize(len);
		MultiByteToWideChar(cp,0,string.c_str(),string.length(),(wchar_t*)ret.c_str(),len+1);
		return ret;
	}

	std::string ToU8string(const std::wstring &wstring)
	{
		return ToString(wstring,CP_UTF8);
	}
}