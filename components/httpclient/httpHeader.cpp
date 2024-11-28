#include <Windows.h>

#include "httpHeader.h"
#include "define.h"

SNSBEGIN

//回调参数信息
struct HttpParamsData
{
	void *lpparam;//自定义数据
	IHttpCallback *callback;//回调函数
	HttpError errcode;//错误码
};

CHttpHeader::CHttpHeader()
	: http_code_(0)
{

}

CHttpHeader::CHttpHeader(const char* pHeader)
	: http_code_(0)
{
	Revolse(std::string(pHeader));
}

CHttpHeader::CHttpHeader(const std::string& strHeader)
	: http_code_(0)
{
	Revolse(strHeader);
}


CHttpHeader::~CHttpHeader(void)
{
}

std::string CHttpHeader::GetValue(const std::string& strKey)
{
	std::string strResult;
	std::map<std::string, std::string>::const_iterator itor;
	itor = http_headers.find(strKey);
	if (itor != http_headers.end())
		strResult = itor->second;
	return strResult;
}

void CHttpHeader::addHeader(const std::string& key, const std::string& value)
{
	if (key.empty() || value.empty()) {
		return;
	}
	http_headers.insert(std::make_pair(key, value));
}

void CHttpHeader::setUserAgent(const std::string& userAgent)
{
	if (userAgent.empty()) {
		return;
	}
	http_headers.insert(std::make_pair(HEADER_USER_AGENT, userAgent));
}

void CHttpHeader::setHost(const std::string& host)
{
	if (host.empty()) {
		return;
	}
	http_headers.insert(std::make_pair(HEADER_HOST, host));
}

void CHttpHeader::setRange(__int64 range)
{
	if (range < 0) {
		return;
	}
	char buffer[64] = { 0 };
	sprintf_s(buffer, "bytes=%lld-", range);
	http_headers.insert(std::make_pair(HEADER_RANGE, std::string(buffer)));
}

std::string CHttpHeader::toString(RequestType type)
{
	//填充默认值
	if (http_version_.empty()) {
		http_version_.assign(default_http_version);
	}
	if (http_headers.find(HEADER_USER_AGENT) == http_headers.end()) {
		http_headers.insert(std::make_pair(HEADER_USER_AGENT, default_user_agent));
	}
	if (http_headers.find(HEADER_CONNECTION) == http_headers.end()) {
		http_headers.insert(std::make_pair(HEADER_USER_AGENT, default_http_version));
	}
	if (http_headers.find(HEADER_ACCEPT) == http_headers.end()) {
		http_headers.insert(std::make_pair(HEADER_ACCEPT, default_accept));
	}
	if (http_headers.find(HEADER_CONNECTION) == http_headers.end()) {
		http_headers.insert(std::make_pair(HEADER_CONNECTION, default_connection));
	}
	if (http_headers.find(HEADER_ACCEPT_LANGUAGE) == http_headers.end()) {
		http_headers.insert(std::make_pair(HEADER_ACCEPT_LANGUAGE, default_language));
	}
	std::string header((type == Hr_Post) ? "POST " : "GET ");
	header += request_page_;
	header.append(" ");
	header += http_version_;
	header.append(http_newline);
	for (KEYMAP::iterator itor = http_headers.begin(); itor != http_headers.end(); ++itor) {
		header += itor->first;
		header.append(": ");
		header += itor->second;
		header.append(http_newline);
	}
	header.append(http_newline);
	header.append(http_newline);
	return header;
}

std::string CHttpHeader::toHttpHeaders()
{
	if (http_headers.find(HEADER_USER_AGENT) == http_headers.end()) {
		http_headers.insert(std::make_pair(HEADER_USER_AGENT, default_user_agent));
	}
	if (http_headers.find(HEADER_CONNECTION) == http_headers.end()) {
		http_headers.insert(std::make_pair(HEADER_USER_AGENT, default_http_version));
	}
	if (http_headers.find(HEADER_ACCEPT) == http_headers.end()) {
		http_headers.insert(std::make_pair(HEADER_ACCEPT, default_accept));
	}
	if (http_headers.find(HEADER_CONNECTION) == http_headers.end()) {
		http_headers.insert(std::make_pair(HEADER_CONNECTION, default_connection));
	}
	if (http_headers.find(HEADER_ACCEPT_LANGUAGE) == http_headers.end()) {
		http_headers.insert(std::make_pair(HEADER_ACCEPT_LANGUAGE, default_language));
	}
	std::string header;
	for (KEYMAP::iterator itor = http_headers.begin(); itor != http_headers.end(); ++itor) {
		header += itor->first;
		header.append(": ");
		header += itor->second;
		header.append(http_newline);
	}
	return header;
}

bool CHttpHeader::Revolse(const std::string& strHeader)
{
	int nStartPos = 0, nFindPos = 0, nLineIndex = 0;
	std::string strLine, strKey, strValue;
	do
	{
		nFindPos = (int)strHeader.find("\r\n", nStartPos);
		if (-1 == nFindPos)
			strLine = strHeader.substr(nStartPos, strHeader.size() - nStartPos);
		else
		{
			strLine = strHeader.substr(nStartPos, nFindPos - nStartPos);
			nStartPos = nFindPos + 2;
		}
		if (0 == nLineIndex)//第一行
		{
			http_version_ = strLine.substr(0, 8);
			size_t nSpace1 = strLine.find(" ");
			size_t nSpace2 = strLine.find(" ", nSpace1 + 1);
			http_code_ = atoi(strLine.substr(nSpace1 + 1, nSpace2 - nSpace1 - 1).c_str());
			http_response_ = strLine.substr(nSpace2 + 1, strLine.size() - nSpace2 - 1);
			nLineIndex++;
			continue;
		}
		size_t nSplit = strLine.find(": ");
		strKey = strLine.substr(0, nSplit);
		strValue = strLine.substr(nSplit + 2, strLine.size() - nSplit - 2);
		std::pair<std::string, std::string> data;
		data.first = strKey;
		data.second = strValue;
		http_headers.insert(data);
		nLineIndex++;
	} while (nFindPos != -1);
	return true;
}

SNSEND