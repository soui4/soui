/*****************************************************************
*HTTP处理类，主要用于HTTP GET/POST、下载（支持重定向）功能
*Author：	JelinYao
*Date：		2015/2/14 12:11
*Email：	mailto://jelinyao@163.com
*/
/*****************************************************************
*/
#pragma once
#include <interface/obj-ref-i.h>
#include <interface/sstring-i.h>
#include <stdint.h>

SNSBEGIN

typedef enum RequestType
{
	Hr_Post,
	Hr_Get,
}RequestType;
//枚举下载状态
typedef enum DownloadState
{
	DS_Start = 0,
	DS_Loading,
	DS_Finished,
	DS_Failed,
}DownloadState;

/******************************************************
*定义错误信息
*
******************************************************/
typedef enum HttpError
{
	Hir_Success = 0,		//成功
	Hir_InitErr,			//初始化失败
	Hir_ConnectErr,			//连接HTTP服务器失败
	Hir_SendErr,			//发送请求失败
	Hir_QueryErr,			//查询HTTP请求头失败
	Hir_404,				//页面不存在
	Hir_IllegalUrl,			//无效的URL
	Hir_CreateFileErr,		//创建文件失败
	Hir_DownloadErr,		//下载失败
	Hir_QueryIPErr,			//获取域名对应的地址失败
	Hir_SocketErr,			//套接字错误
	Hir_UserCancel,			//用户取消下载
	Hir_BufferErr,			//文件太大，缓冲区不足
	Hir_HeaderErr,			//HTTP请求头错误
	Hir_ParamErr,			//参数错误，空指针，空字符……
	Hir_UnknowErr,

}HttpError;


#undef INTERFACE
#define INTERFACE IHttpCallback
DECLARE_INTERFACE_(IHttpCallback, IObjRef){
   /**
     * @brief 增加引用计数
     * @return 新引用计数
    */
    STDMETHOD_(long,AddRef) (THIS) PURE;

    /**
     * @brief 减少引用计数
     * @return 新引用计数
    */
    STDMETHOD_(long,Release) (THIS) PURE;

	/**
	 * @brief 释放对象
	 * @return void
	*/
	STDMETHOD_(void,OnFinalRelease) (THIS) PURE;
	//////////////////////////////////////////////////////////////////////////
	STDMETHOD_(BOOL,OnDownloadCallback)(THIS_ DownloadState state, uint64_t nTotalSize, uint64_t nLoadSize) PURE;
};

#undef INTERFACE
#define INTERFACE IHttpClient
DECLARE_INTERFACE_(IHttpClient, IObjRef){
   /**
     * @brief 增加引用计数
     * @return 新引用计数
    */
    STDMETHOD_(long,AddRef) (THIS) PURE;

    /**
     * @brief 减少引用计数
     * @return 新引用计数
    */
    STDMETHOD_(long,Release) (THIS) PURE;

	/**
	 * @brief 释放对象
	 * @return void
	*/
	STDMETHOD_(void,OnFinalRelease) (THIS) PURE;
	//////////////////////////////////////////////////////////////////////////
	STDMETHOD_(void,SetHeader)(THIS_ LPCSTR pszKey,LPCSTR pszValue) PURE;
	STDMETHOD_(void,SetDownloadCallback)(THIS_ IHttpCallback* pCallback) PURE;
	STDMETHOD_(BOOL,DownloadFile)(THIS_ LPCSTR lpUrl, LPCSTR lpFilePath) PURE;
	STDMETHOD_(BOOL,DownloadToMem)(THIS_ LPCSTR lpUrl, OUT void** ppBuffer, OUT int* nSize) PURE;
	STDMETHOD_(HttpError,GetErrorCode)(CTHIS) SCONST PURE;
	STDMETHOD_(BOOL,Request)(THIS_ IStringA *result,LPCSTR lpUrl, RequestType type, LPCSTR lpPostData DEF_VAL(NULL), LPCSTR lpHeader DEF_VAL(NULL)) PURE;
	STDMETHOD_(void,SetTimeOut)(THIS_ int dwConnectTime,  int dwSendTime, int dwRecvTime) PURE;		
	STDMETHOD_(int,GetHttpCode)(CTHIS) SCONST PURE;
};

SNSEND



