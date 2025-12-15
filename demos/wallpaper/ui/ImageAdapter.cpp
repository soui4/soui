#include "stdafx.h"
#include "ImageAdapter.h"
#define kLogTag "CImageAdapter"
#pragma warning(disable:4996)

CImageAdapter::CImageAdapter(void):m_category(-1)
{
}

CImageAdapter::~CImageAdapter(void)
{
}

int CImageAdapter::getCount()
{
	DATAMAP::iterator it = m_dataMap.find(m_category);
	if(it == m_dataMap.end())
		return 0;
	else
		return (int)(it->second->size()+4)/5;
}

void CImageAdapter::getView(int position, SItemPanel * pItem, SXmlNode xmlTemplate)
{
	if(pItem->GetChildrenCount()==0)
	{
		pItem->InitFromXml(&xmlTemplate);
	}

	DATAMAP::iterator it = m_dataMap.find(m_category);
	SASSERT(it!=m_dataMap.end());

	int iImg = position*5;
	for(int i=R.id.img_1;i<=R.id.img_5;i++,iImg++)
	{
		SImageWnd *pImg = pItem->FindChildByID2<SImageWnd>(i);
		if(iImg < it->second->size())
		{
			IMGINFO & imgInfo = it->second->at(iImg);
			URL2IMGMAP::iterator it2 = m_imgMap.find(imgInfo.uriThumb);
			if(it2 == m_imgMap.end())
			{//download the image
				pImg->SetImage(NULL);
				CHttpDownloader::getSingletonPtr()->download(imgInfo.uriThumb,URL_IMG,m_category);
			}else
			{
				pImg->SetImage(it2->second);
			}
		}else
		{
			pImg->SetImage(NULL);
		}
	}
}

void CImageAdapter::OnDownloadFinish(const std::string &uri, const SStringA &data, long id,long category, IBitmapS * pImg)
{
	if(id == URL_IMGINFO)
	{
		Json::Reader reader;
		Json::Value  value;
		if(reader.parse(data.c_str(), value))
		{
			if(value["errno"].asString() == "0")
			{
				Json::Value imgs = value["data"];
				for(unsigned int i=0;i<imgs.size();i++)
				{
					IMGINFO info;
					string id = imgs[i]["id"].asString();
					info.uriThumb = imgs[i]["url_thumb"].asString();
					info.uriBig = imgs[i]["url"].asString();
					info.id = atoi(id.c_str());
					string clsId = imgs[i]["class_id"].asString();
					int nClsId = atoi(clsId.c_str());

					DATAMAP::iterator it = m_dataMap.find(nClsId);
					if(it == m_dataMap.end())
					{
						m_dataMap[nClsId] = new IMGS;
					}
					m_dataMap[nClsId]->push_back(info);
				}
				notifyDataSetChanged();
			}else
			{
				SLOGE()<<"fatch categories error code:"<<value["error"].asString().c_str();
			}
		}
	}else
	{//img
		m_imgMap[uri] = pImg;
		m_cacheList.push_back(uri);
		if(m_cacheList.size()>MAX_CACHE_SIZE)
		{
			string discardUri = m_cacheList.front();
			m_cacheList.pop_front();
			m_imgMap.erase(discardUri);
		}
		int idx = url2index(uri,category);
		SLOGW()<<"url "<<uri.c_str()<<" index is "<<idx;
	
		if(idx!=-1)
		{
			idx /= 5;
			notifyItemDataChanged(idx);
		}else
		{
			SLOGW()<<"url "<<uri.c_str()<<" index is -1";
		}
	}
}

void CImageAdapter::setCategory(int catetory)
{
	m_category = catetory;
	notifyDataSetChanged();
	if(getCount() == 0)
	{
		SStringA strUrl=SStringA().Format("http://wallpaper.apc.360.cn/index.php?%%20c=WallPaper&a=getAppsByCategory&cid=%d&start=0&count=50&from=360chrome",m_category);
		CHttpDownloader::getSingletonPtr()->download((LPCSTR)strUrl,URL_IMGINFO,-1);
	}
}


int CImageAdapter::url2index(const string &url,int category) const
{
	if(category != m_category)
		return -1;
	DATAMAP::const_iterator it = m_dataMap.find(category);
	if(it == m_dataMap.end())
		return -1;

	int iRet=0;
	IMGS::const_iterator it2 = it->second->begin();
	while(it2 != it->second->end())
	{
		if(it2->uriThumb == url)
			return iRet;

		iRet ++;
		it2++;
	}
	return -1;
}