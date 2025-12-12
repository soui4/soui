#pragma once
#include <interface/STaskLoop-i.h>
#include <string>
#include <helper/SCriticalSection.h>
#include <helper/SSharedPtr.hpp>
#include <commgr2.h>
#include <memory>

#define MAX_DOWNLOADER 10


DEF_EVT_EXT(EventDownloadFinish,EVT_DOWNLOAD_FINISH,{
	std::string url;
	SSharedPtr<SStringA> data;
	long   type;
	long   catetory;
	SAutoRefPtr<IBitmapS> pImg;
}
)

enum{
	URL_CATEGORIES=1,
	URL_IMGINFO,
	URL_IMG,
};

class CHttpDownloader : public SSingleton<CHttpDownloader>
{
public:
	CHttpDownloader(void);
	~CHttpDownloader(void);

	void download(const std::string & url,long type, long catetory);

private:
	void _download(const std::string & url,long type, long catetory,int iLoop);
	string uri2md5(const string & uri);
private:
	SComMgr m_ComMgr;
	SAutoRefPtr<ITaskLoop> m_taskLoops[MAX_DOWNLOADER];
	
	SCriticalSection	m_cs;
	set<string>			m_pendingTasks;
	SComMgr2			m_comMgr2;
	string m_u8CachePath;
};
