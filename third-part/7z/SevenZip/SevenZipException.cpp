#include "SevenZipException.h"
#include "SevenString.h"
#include <vector>

namespace SevenZip
{
	std::string StrFmt(const char* format, ...) {
		std::string result;
		va_list args;
		va_start(args, format);

#ifdef _WIN32
		int sz = _vscprintf(format, args) + 1; // 计算所需缓冲区大小（含终止符）
		std::vector<char> buffer(sz);
		_vsnprintf_s(&buffer[0], sz, _TRUNCATE, format, args);
#else
		va_list args_copy;
		va_copy(args_copy, args);

		int sz = vsnprintf(nullptr, 0, format, args_copy);
		if (sz < 0) {
			va_end(args_copy);
			va_end(args);
			return {};
		}

		std::vector<char> buffer(sz + 1); // +1 为终止符预留空间
		vsnprintf(buffer.data(), buffer.size(), format, args);
		va_end(args_copy);
#endif
		result = &buffer[0];

		va_end(args);
		return result;
	}

TString GetWinErrMsg( const TString& contextMessage, DWORD lastError )
{
	// TODO: use FormatMessage to get the appropriate message from the 
#ifdef _UNICODE
	return ToWstring(StrFmt("%s: GetLastError = %lu", contextMessage.c_str(), lastError),CP_ACP);
#else
	return StrFmt("%s: GetLastError = %lu", contextMessage.c_str(), lastError);
#endif

}

TString GetCOMErrMsg( const TString& contextMessage, HRESULT lastError )
{
	// TODO: use FormatMessage to get the appropriate message from the 
#ifdef _UNICODE
	return ToWstring(StrFmt("%s: HRESULT = 0x%08X", contextMessage.c_str(), lastError ),CP_ACP);
#else
	return StrFmt("%s: HRESULT = 0x%08X", contextMessage.c_str(), lastError);
#endif
}

SevenZipException::SevenZipException()
{
}

SevenZipException::SevenZipException( const TString& message )
	: m_message( message )
{
}

SevenZipException::~SevenZipException()
{
}

const TString& SevenZipException::GetMessage() const
{
	return m_message;
}

}
