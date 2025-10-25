#ifndef __AUTOCOMPLETEADAPTER_H__
#define __AUTOCOMPLETEADAPTER_H__

#include <helper/SAdapterBase.h>
#include <vector>

SNSBEGIN

class CAutoCompleteAdapter : public SAdapterBase
{
public:
	CAutoCompleteAdapter(){}

	virtual ~CAutoCompleteAdapter()
	{
	}
	virtual SStringT getItemText(int position) const =0;
};

class SAutoCompleteImageSrcAdapter : public CAutoCompleteAdapter
{
    struct ImageInfo
    {
        SStringT strDesc;
        SStringT strPath;
    };
    std::vector<ImageInfo> m_vecImage;
    
public:
    SAutoCompleteImageSrcAdapter()
    {
    }
    ~SAutoCompleteImageSrcAdapter()
    {
    }

    void addItem(SStringT strDesc, SStringT strPath)
    {
        ImageInfo info;
        info.strDesc = strDesc;
        info.strPath = strPath;
        m_vecImage.push_back(info);
    }
    virtual SStringT getItemText(int position) const
    {
        return m_vecImage[position].strDesc;
    }
    virtual int WINAPI getCount()
    {
        return m_vecImage.size();
    }
    virtual void WINAPI getView(int position, SItemPanel *pItem, SXmlNode xmlTemplate)
    {
        if(pItem->GetChildrenCount()==0){
            pItem->InitFromXml(&xmlTemplate);
        }
        SWindow *pTxtDesc = pItem->FindChildByID(R.id.txt_desc);
        pTxtDesc->SetWindowText(m_vecImage[position].strDesc);
        
        SWindow *pTxtPath = pItem->FindChildByID(R.id.txt_path);
        pTxtPath->SetWindowText(m_vecImage[position].strPath);

        //SImageWnd *pImgPreview = pItem->FindChildByID2<SImageWnd>(R.id.skin_preview);
        //pImgPreview->SetAttribute(L"src",m_vecImage[position].strImage);
    }
};

class CAutoCompleteSkinAdapter : public CAutoCompleteAdapter
{
	struct SkinInfo
	{
		SStringT name;
		SAutoRefPtr<ISkinObj> pSkin;
	};
	std::vector<SkinInfo> m_vecSkin;
public:
	CAutoCompleteSkinAdapter()
	{
	}

	~CAutoCompleteSkinAdapter()
	{
	}

	void AddSkin(const SStringT & strSkin,ISkinObj *pSkin)
	{
		SkinInfo data;
		data.name = strSkin;
		data.pSkin = pSkin;
		m_vecSkin.push_back(data);
	}

	virtual int WINAPI getCount()
	{
		return m_vecSkin.size();
	}

	virtual SStringT getItemText(int iItem) const
	{
		return m_vecSkin[iItem].name;
	}

	STDMETHOD_(void,getView)(int position, SItemPanel *pItem, SXmlNode xmlTemplate)
	{
		if(pItem->GetChildrenCount()==0){
			pItem->InitFromXml(&xmlTemplate);
		}
		SWindow *pTxtDesc = pItem->FindChildByID(R.id.txt_desc);
		pTxtDesc->SetWindowText(m_vecSkin[position].name);
		SImageWnd *pImgPreview = pItem->FindChildByID2<SImageWnd>(R.id.skin_preview);
		pImgPreview->SetSkin(m_vecSkin[position].pSkin);
	}
};

//---------------------------------------------------------------------------------------------------
class CAutoCompleteColorAdapter : public CAutoCompleteAdapter
{
	struct ColorInfo
	{
		SStringT name;
		COLORREF color;
	};
	std::vector<ColorInfo> m_vecColor;
public:
	CAutoCompleteColorAdapter()
	{
	}

	~CAutoCompleteColorAdapter()
	{
	}

	void AddColor(const SStringT & strDesc,COLORREF cr)
	{
		ColorInfo data;
		data.name = strDesc;
		data.color = cr;
		m_vecColor.push_back(data);
	}

	COLORREF GetColor(int iItem)
	{
		return m_vecColor[iItem].color;
	}
	virtual int WINAPI getCount()
	{
		return m_vecColor.size();
	}

	virtual SStringT getItemText(int iItem) const
	{
		return m_vecColor[iItem].name;
	}

	STDMETHOD_(void,getView)(int position, SItemPanel *pItem, SXmlNode xmlTemplate)
	{
		if(pItem->GetChildrenCount()==0){
			pItem->InitFromXml(&xmlTemplate);
		}
		SWindow *pTxtDesc = pItem->FindChildByID(R.id.txt_desc);
		pTxtDesc->SetWindowText(m_vecColor[position].name);
		SWindow *pColorPreview = pItem->FindChildByID(R.id.color_preview);
		pColorPreview->GetStyle().m_crBg = m_vecColor[position].color;
	}
};

//---------------------------------------------------------------------------------------------------
class CAutoCompleteStringAdapter : public SAdapterBase
{
	struct StringInfo
	{
		SStringT name;
		SStringT value;
	};
	std::vector<StringInfo> m_vecString;
public:
	CAutoCompleteStringAdapter()
	{
	}

	~CAutoCompleteStringAdapter()
	{
	}

	void AddString(const SStringT & strName,const SStringT & strDesc)
	{
		StringInfo data;
		data.name = strName;
		data.value = strDesc;
		m_vecString.push_back(data);
	}

	virtual int WINAPI getCount()
	{
		return m_vecString.size();
	}

	virtual SStringT getItemText(int iItem)
	{
		return m_vecString[iItem].name;
	}

	STDMETHOD_(void,getView)(int position, SItemPanel *pItem, SXmlNode xmlTemplate)
	{
		if(pItem->GetChildrenCount()==0){
			pItem->InitFromXml(&xmlTemplate);
		}
		SWindow *pTxtDesc = pItem->FindChildByID(R.id.txt_desc);
		pTxtDesc->SetWindowText(m_vecString[position].name);
		SWindow *pTxtValue = pItem->FindChildByID(R.id.txt_value);
		pTxtValue->SetWindowText(m_vecString[position].value);
	}
};


SNSEND
#endif//__AUTOCOMPLETEADAPTER_H__