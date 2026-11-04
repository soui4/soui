#ifndef _HTTP_HEADER_H_
#define _HTTP_HEADER_H_

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

// Cookie 解析工具：IHttpClient::GetCookie 的 Windows(WinHTTP) 与其它平台(libcurl)
// 两套实现共用同一份解析逻辑，避免两边行为不一致。
//
// 响应头里的 Set-Cookie 形如：
//   Set-Cookie: ASP.NET_SessionId=abc123; path=/; HttpOnly
// 一个响应可以携带多条 Set-Cookie。这里统一解析成可直接放进请求头
// "Cookie: xxx" 的 "name=value; name2=value2" 串，只保留 name=value，
// 剔除 Path / Expires / Domain / Secure / HttpOnly / SameSite / Max-Age 等属性项。
namespace CookieUtil
{
	// 把一条 Set-Cookie 的"值"部分（不含 "Set-Cookie:" 前缀）追加到 outCookie。
	void AppendSetCookie(const std::string& strValue, std::string& outCookie);

	// 扫描整块原始响应头文本（每行 "Name: Value"），提取全部 Set-Cookie 后拼接返回。
	std::string ParseFromRawHeaders(const std::string& strRawHeaders);
}

SNSEND

#endif//_HTTP_HEADER_H_