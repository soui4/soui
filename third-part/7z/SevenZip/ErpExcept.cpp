/********************************************************************
  Class Name: ERPException
  Created Date:20080905
  Author: wuding
  Definition file: ErpExcept.cpp
  Last Version Date: 
  Version Date:
********************************************************************/
#include "ErpExcept.h"

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifdef _WIN32
#include <winsock2.h>
#include <windows.h>
#endif
ERPException::ERPException(EGrade nGrade,const char *pFormat,...)
              :mGrade(nGrade),
              nErrCode(0) 
{
      va_list pArg;
      va_start(pArg, pFormat);
#ifdef _WIN32
      _vsnprintf(m_str,MAX_EXCEPTION_LEN, pFormat, pArg);
#else
	  vsnprintf(m_str, MAX_EXCEPTION_LEN, pFormat, pArg);
#endif
      va_end(pArg);
}

ERPException::ERPException(EGrade nGrade, int nCode, const char *pFormat,...)
                :mGrade(nGrade),
                nErrCode(nCode)
{
      va_list pArg;
      va_start(pArg, pFormat);
#ifdef _WIN32
      _vsnprintf(m_str,MAX_EXCEPTION_LEN, pFormat, pArg);
#else
	  vsnprintf(m_str, MAX_EXCEPTION_LEN, pFormat, pArg);
#endif
      va_end(pArg);
}
ERPException::~ERPException()
{
}


const char *ERPException::GetErrorMsg()
{ 
     return m_str;
}
 
const std::wstring ERPException::GetErrorMsgW()
{
#ifdef _WIN32
		const char *pszSrc = m_str;
		int textlen ;
		wchar_t * result;
		textlen = MultiByteToWideChar( CP_ACP, 0, pszSrc,-1, NULL,0 ); 
		result = (wchar_t *)malloc((textlen+1)*sizeof(wchar_t)); 
		memset(result,0,(textlen+1)*sizeof(wchar_t)); 
		MultiByteToWideChar(CP_ACP, 0,pszSrc,-1,(wchar_t *)result,textlen ); 
		std::wstring msg(result);
		free(result);
		return msg;
#else 
	return L"";
#endif
} 
const std::string ERPException::GetErrorMsgA()
{ 
     return m_str;
}

int ERPException::GetErrorCode()
{
     return nErrCode;
}

EGrade ERPException::GetErrorGrade()
{
     return mGrade;
}
