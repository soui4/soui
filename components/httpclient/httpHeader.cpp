
#include "httpHeader.h"
#include "define.h"
#include <cctype>
#include <cstring>

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
	#if defined(__linux__)
	sprintf(buffer, "bytes=%ld-", range);
	#else
	sprintf(buffer, "bytes=%lld-", range);
	#endif//_WIN32
	http_headers.insert(std::make_pair(HEADER_RANGE, std::string(buffer)));
}

std::string CHttpHeader::toString(RequestType type)
{
	std::string header((type == Hr_Post) ? "POST " : "GET ");
	header += toHttpHeaders();
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

// ------------------------------------------------------------------ Cookie 解析
namespace CookieUtil
{

// 比较 name 与全小写的字面量 a 是否相等（大小写不敏感）
static bool EqualCI(const std::string& strName, const char* a)
{
	size_t n = strlen(a);
	if (strName.size() != n)
		return false;
	for (size_t i = 0; i < n; ++i)
	{
		if (tolower((unsigned char)strName[i]) != a[i])
			return false;
	}
	return true;
}

// Set-Cookie 里的修饰属性，不属于 Cookie 本身
static bool IsCookieAttrName(const std::string& strName)
{
	static const char* kAttrs[] = {
		"path", "expires", "domain", "secure", "httponly",
		"samesite", "max-age", "version", "comment", "priority",
	};
	const size_t kCount = sizeof(kAttrs) / sizeof(kAttrs[0]);
	for (size_t i = 0; i < kCount; ++i)
	{
		if (EqualCI(strName, kAttrs[i]))
			return true;
	}
	return false;
}

static std::string Trim(const std::string& s)
{
	size_t a = s.find_first_not_of(" \t\r\n");
	if (a == std::string::npos)
		return std::string();
	size_t b = s.find_last_not_of(" \t\r\n");
	return s.substr(a, b - a + 1);
}

void AppendSetCookie(const std::string& strValue, std::string& outCookie)
{
	const size_t n = strValue.size();
	size_t pos = 0;
	while (pos <= n)
	{
		size_t semi = strValue.find(';', pos);
		std::string part = (semi == std::string::npos) ? strValue.substr(pos)
			                                          : strValue.substr(pos, semi - pos);
		part = Trim(part);
		if (!part.empty())
		{
			size_t eq = part.find('=');
			// eq > 0：跳过 "=value" 这种没有名字的脏数据
			if (eq != std::string::npos && eq > 0)
			{
				std::string name = Trim(part.substr(0, eq));
				// Cookie 名不允许出现空格；属性项（Path/Expires/...）直接丢弃
				if (!name.empty() && name.find(' ') == std::string::npos && !IsCookieAttrName(name))
				{
					if (!outCookie.empty())
						outCookie += "; ";
					outCookie += part;
				}
			}
		}
		if (semi == std::string::npos)
			break;
		pos = semi + 1;
	}
}

std::string ParseFromRawHeaders(const std::string& strRawHeaders)
{
	std::string outCookie;
	const size_t kKeyLen = 11; // strlen("set-cookie:")
	const size_t n = strRawHeaders.size();
	size_t pos = 0;
	while (pos < n)
	{
		size_t eol = strRawHeaders.find('\n', pos);
		std::string line = (eol == std::string::npos) ? strRawHeaders.substr(pos)
		                                             : strRawHeaders.substr(pos, eol - pos);
		pos = (eol == std::string::npos) ? n : eol + 1;
		if (!line.empty() && line[line.size() - 1] == '\r')
			line.erase(line.size() - 1);
		if (line.size() <= kKeyLen)
			continue;
		// 前缀 "Set-Cookie:"，大小写不敏感
		if (!EqualCI(line.substr(0, kKeyLen), "set-cookie:"))
			continue;
		AppendSetCookie(line.substr(kKeyLen), outCookie);
	}
	return outCookie;
}

} // namespace CookieUtil

SNSEND