#pragma once


class CImageAdapter : public SAdapterBase
{
public:
	CImageAdapter(void);
	~CImageAdapter(void);

	void setCategory(int nCategory);
	void OnDownloadFinish(const std::string &uri, const SStringA &data, long type, long category, IBitmap * pImg);
protected:
	STDMETHOD_(int, getCount)() override;

	STDMETHOD_(void, getView)(int position, SItemPanel *pItem, SXmlNode xmlTemplate);

private:
	int url2index(const string &url,int category) const;
private:
	struct IMGINFO
	{
		int id;
		string uriThumb;
		string uriBig;
	};

	typedef map<string,SAutoRefPtr<IBitmap> > URL2IMGMAP;

	typedef vector<IMGINFO> IMGS;
	typedef map<int,IMGS * > DATAMAP;
	DATAMAP m_dataMap;
	URL2IMGMAP m_imgMap;

	enum {
		MAX_CACHE_SIZE = 40
	};
	list<string> m_cacheList;

	int    m_category;
};
