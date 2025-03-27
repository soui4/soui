#ifndef _IPCDEMO_PROTOCOL_H_
#define _IPCDEMO_PROTOCOL_H_

#include <string>
#include <sstream>
#include <interface/sipcobj-i.h>
#include <helper/SIpcParamHelper.hpp>

#define DEMO_SHARE_BUF_NAME_FMT _T("demo2_share_buffer_8085395F-E2FA-4F96-8BD0-FE5D7412CD22_%08x_2_%08x")
#define SVR_NAME _T("sipc_demo_svr")
#ifdef _UNICODE
typedef std::wstring tstring;
#else
typedef std::string tstring;
#endif


//////////////////////////////////////////////////////////////////
SNSBEGIN
	template<>
	inline SParamStream & SParamStream::operator<<(const tstring & str)
	{
		int nSize = (int)str.size();
		GetBuffer()->Write(&nSize, sizeof(int));
		GetBuffer()->Write(str.c_str(), nSize*sizeof(TCHAR));
		return *this;
	}
	template<>
	inline SParamStream & SParamStream::operator >> (tstring & str)
	{
		int nSize = 0;
		GetBuffer()->Read(&nSize, sizeof(int));
		TCHAR *pBuf = new TCHAR[nSize];
		GetBuffer()->Read(pBuf, nSize*sizeof(TCHAR));
		str = tstring(pBuf, nSize);
		delete[]pBuf;
		return *this;
	}
SNSEND

struct FunParams_Base : SOUI::IFunParams
{
	virtual void ToStream4Input(SOUI::SParamStream &  ps) {}
	virtual void ToStream4Output(SOUI::SParamStream &  ps) {}
	virtual void FromStream4Input(SOUI::SParamStream &  ps) {}
	virtual void FromStream4Output(SOUI::SParamStream &  ps) {}
};


struct IClient
{
	virtual int Add(int a, int b) = 0;
	virtual tstring Add(const tstring & a, const tstring & b) = 0;
	virtual int Sum(int n) = 0;
};

struct IServer
{
	virtual void SayHello(const tstring & name) = 0;
};

enum {
	CID_AddInt = SOUI::FUN_ID_START,
	CID_AddString,

	SID_Hello,

	BID_Sum,
};

struct Param_AddInt : FunParams_Base
{
	int a, b;
	int ret;
	FUNID(CID_AddInt)
		PARAMS2(Input, a,b)
		PARAMS1(Output,ret)
};

struct Param_AddString : FunParams_Base
{
	tstring a, b;
	tstring ret;
	FUNID(CID_AddString)
		PARAMS2(Input, a, b)
		PARAMS1(Output, ret)
};

struct Param_Hello : FunParams_Base
{
	tstring text;
	FUNID(SID_Hello)
		PARAMS1(Input, text)
};

struct Param_Sum : FunParams_Base
{
	int n;
	int nRet;
	FUNID(BID_Sum)
		PARAMS1(Input, n)
		PARAMS1(Output, nRet)
};


#endif//_IPCDEMO_PROTOCOL_H_