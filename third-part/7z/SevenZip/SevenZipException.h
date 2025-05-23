#pragma once

#include <stdarg.h>
#include <exception>
#include "SevenString.h"
#ifdef _WIN32
#include <windows.h>
#endif

namespace SevenZip
{
	std::string StrFmt( const TCHAR* format, ... );
	TString GetWinErrMsg( const TString& contextMessage, DWORD lastError );
	TString GetCOMErrMsg( const TString& contextMessage, HRESULT lastError );

	class SevenZipException
	{
	protected:

		TString m_message;

	public:

		SevenZipException();
		SevenZipException( const TString& message );
		virtual ~SevenZipException();

		const TString& GetMessage() const;
	};
}
