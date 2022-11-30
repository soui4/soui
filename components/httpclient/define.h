#pragma once


//定义每次读取数据的缓冲区大小 4KB
#define READ_BUFFER_SIZE		4096

//内存中设置的缓冲区大小 8M
#define DOWNLOAD_BUFFER_SIZE	(8*1024*1024)


//定义常用http头名称
#define HEADER_USER_AGENT			"User-Agent"
#define HEADER_CONNECTION			"Connection"
#define HEADER_ACCEPT				"Accept"
#define HEADER_ACCEPT_ENCODING		"Accept-Encoding"
#define HEADER_ACCEPT_LANGUAGE		"Accept-Language"
#define HEADER_CONTENT_TYPE			"Content-Type"
#define HEADER_HOST					"Host"
#define HEADER_RANGE				"Range"
#define HEADER_LOCATION				"Location"
#define HEADER_CONTENT_LENGTH		"Content-Length"


//默认HTTP版本
static const char default_http_version[] = "HTTP/1.1";

//定义默认的userAgent
static const char default_user_agent[] = "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/80.0.4044.92 Safari/537.36";

//默认接收类型
static const char default_accept[] = "*/*";

//默认保持连接
static const char default_connection[] = "Keep-Alive";

//默认语言
static const char default_language[] = "zh-CN,zh;q=0.9";
 
//HTTP协议换行符
static const char http_newline[] = "\r\n";
