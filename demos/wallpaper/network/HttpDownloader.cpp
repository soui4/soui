#include "stdafx.h"
#include "HttpDownloader.h"
#include <helper/SFunctor.hpp>
#include <interface/SHttpClient-i.h>

#include "MD5.h"
#define kLogTag "CHttpDownloader"

template<>
CHttpDownloader * SSingleton<CHttpDownloader>::ms_Singleton = NULL;

CHttpDownloader::CHttpDownloader(void)
{
	SNotifyCenter::getSingletonPtr()->addEvent(EVENTID(EventDownloadFinish));


	TCHAR szCurDir[MAX_PATH];
	GetCurrentDirectory(MAX_PATH, szCurDir);
	SStringT path = szCurDir;
	path += _T("/image_cache");
	CreateDirectory(path.c_str(),NULL);
	m_u8CachePath = S_CT2A(szCurDir, CP_UTF8).c_str();
	for(int i=0;i<MAX_DOWNLOADER;i++)
	{
		m_ComMgr.CreateTaskLoop((IObjRef **)&m_taskLoops[i]);
		char szName[30];
		sprintf(szName,"downloader_%d",i+1);
		m_taskLoops[i]->start(szName,Low);
	}
}

CHttpDownloader::~CHttpDownloader(void)
{
	for(int i=0;i<MAX_DOWNLOADER;i++)
	{
		int nPendingTasks = m_taskLoops[i]->getTaskCount();
		SLOGI()<<"downloader "<<i<<" remain "<<nPendingTasks<<" tasks";
		m_taskLoops[i]->stop();
		m_taskLoops[i] = NULL;
	}
}

void CHttpDownloader::download(const std::string & url,long type, long catetory)
{
	int nMinTasks = 10000;
	int iLoop = -1;
	{
		SAutoLock lock(m_cs);
		if(m_pendingTasks.find(url)!=m_pendingTasks.end())
			return;
		m_pendingTasks.insert(url);
	}
	for(int i=0;i<MAX_DOWNLOADER;i++)
	{
		int nTasks = m_taskLoops[i]->getTaskCount();
		if(nMinTasks>nTasks)
		{
			nMinTasks = nTasks;
			iLoop = i;
		}
	}
	STaskHelper::post(m_taskLoops[iLoop],this,&CHttpDownloader::_download,url,type,catetory,iLoop,false);
}


string CHttpDownloader::uri2md5(const string & uri)
{
	MD5_CTX ctx;
	MD5Init(&ctx);
	MD5Update(&ctx, (unsigned char *)uri.c_str(), uri.length());
	unsigned char digest[16];
	MD5Final(&ctx, digest);

	char szMD5[16 * 2 + 1];
	for (int i = 0; i<16; i++)
	{
		sprintf(szMD5 + i * 2, "%x", (digest[i] & 0xF0) >> 4);
		sprintf(szMD5 + i * 2 + 1, "%x", digest[i] & 0x0F);
	}
	return string(szMD5, 32);
}

void CHttpDownloader::_download(const std::string & url,long type, long category,int iLoop)
{
	SLOGI()<<"_download, url:"<<url.c_str()<<" type:"<<type<<" category";

	std::string strUriMd5 = uri2md5(url);
	std::string strCacheFile = m_u8CachePath + "/" + strUriMd5 + ".cache";

	SSharedPtr<SStringA> data;

	//check local cache
	bool isFoundCache = false;
	if (type == URL_IMG)
	{
		SStringW strCacheFileW = S_CA2W(strCacheFile.c_str(),CP_UTF8);
		FILE *f = _wfopen(strCacheFileW.c_str(), L"rb");
		if (f)
		{
			fseek(f, 0, SEEK_END);
			int nLen = ftell(f);
			fseek(f, 0, SEEK_SET);

			data.reset(new SStringA('\0',nLen));
			fread((char*)data->c_str(), 1, nLen, f);
			fclose(f);
			isFoundCache = true;
			SLOGI()<<"load image from cache uri: " << url.c_str();
		}
	}

	if(!isFoundCache)
	{
		SLOGI() << "<<<<<start download " << url.c_str();
		SAutoRefPtr<IHttpClient> httpClient;
		BOOL bLoaded = m_comMgr2.CreateHttpClient((IObjRef**)&httpClient);
		SASSERT(bLoaded);
		httpClient->SetTimeOut(5000, 5000, 5000);
		SStringA outBuf;
		BOOL bDownload = httpClient->Request(&outBuf,url.c_str(), Hr_Get);
		if (bDownload)
		{
			data.reset(new SStringA(outBuf));
		}
		else {
			SLOGE() << "download " << url.c_str() << " failed"<<" err="<<httpClient->GetErrorCode()<<" http code="<< httpClient->GetHttpCode();
		}
		SLOGI()<<">>>>> end download "<<url.c_str();
	}
	{
		SAutoLock lock(m_cs);
		m_pendingTasks.erase(url);
	}

	if(data){
		EventDownloadFinish *evt = new EventDownloadFinish(NULL);
		evt->url = url;
		evt->data = data;
		evt->type = type;
		evt->catetory = category;
		if(type == URL_IMG)
		{//try to decode image.
			evt->pImg.Attach(SResLoadFromMemory::LoadImage((LPVOID)data->c_str(),data->GetLength()));
			if(evt->pImg && !isFoundCache)
			{
				SStringW strCacheFileW = S_CA2W(strCacheFile.c_str(), CP_UTF8);
				FILE* f = _wfopen(strCacheFileW.c_str(), L"rb");
				if (f)
				{
					fwrite(data->c_str(), 1, data->GetLength(), f);
					fclose(f);
				}
			}
		}else
		{
			SLOGI()<<"download form "<<url.c_str()<<" result:"<<data->c_str();
		}
		SNotifyCenter::getSingletonPtr()->FireEventAsync(evt);
		evt->Release();
	}
	else {
		SLOGE() << "download " << url.c_str() << " failed";
	}
}
