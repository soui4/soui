#pragma once
#include <string>
#include <map>
#include <interface/SHttpClient-i.h>

SNSBEGIN

class CHttpHeader
{
public:
	CHttpHeader();
	CHttpHeader(const char* pHeader);
	CHttpHeader(const std::string& strHeader);
	virtual	~CHttpHeader(void);


	const std::string& getHttpVersion()const { return http_version_; }
	void setHttpVersion(const std::string& version) { http_version_ = version; }
	void setRequestPath(const std::string& path) { request_page_ = path; }

	const int GetHttpCode()const { return http_code_; }

	const char*	GetContent()const { return http_response_.c_str(); }

	std::string	GetValue(const std::string& strKey);

	void addHeader(const std::string& key, const std::string& value);
	void setUserAgent(const std::string& userAgent);
	void setHost(const std::string& host);
	void setRange(__int64 range);

	//转成http请求头
	std::string toString(RequestType type);
	//转成http请求头列表
	std::string toHttpHeaders();

protected:
	//解析HTTP头结构
	bool Revolse(const std::string& strHeader);

private:
	int	 http_code_;
	std::string http_version_;
	std::string	http_response_;
	std::string request_page_;
	typedef std::map<std::string, std::string> KEYMAP;
	KEYMAP	http_headers;
};

SNSEND