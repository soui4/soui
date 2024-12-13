#pragma once
#include <vector>

#include <helper/SAdapterBase.h>
#include "CGlobalUnits.h"

class CGZHTileViewAdapter : public SAdapterBase {
public:
	struct ItemData
	{
		std::string m_strID;
        std::string m_strName;
		std::string m_strAvatar;
	};

	struct IListen
	{
		//virtual void OnEmotionItemClick(const std::string& strID) = 0;
	};

public:
    CGZHTileViewAdapter(IListen *pListen)
		:m_pListen(pListen)
	{
	}
    ~CGZHTileViewAdapter()
    {
    }

protected:
	virtual int getCount() {
		return static_cast<int>(m_vecItems.size());
	}

	virtual void getView(int position, SItemPanel* pItem, SXmlNode xmlTemplate)
	{
		if (0 == pItem->GetChildrenCount())
		{
			pItem->InitFromXml(&xmlTemplate);
			//pItem->GetEventSet()->subscribeEvent(EventItemPanelClick::EventID, Subscriber(&CGZHTileViewAdapter::OnEvtItemPanelClick, this));
        }

		size_t sPos = static_cast<size_t>(position);
		if (sPos >= m_vecItems.size()) return;

		ItemData* pItemData = m_vecItems[sPos];
		if (NULL == pItemData) return;

		SImageWnd* pImg = pItem->FindChildByName2<SImageWnd>(L"item_avatar");
        SASSERT(pImg);
        if (pItemData->m_strAvatar == "")
		{
            pImg->SetAttribute(L"skin", L"skin_default_gzh32");
		}

		SStatic *pName = pItem->FindChildByName2<SStatic>(L"item_name");
        SASSERT(pName);
        pName->SetWindowText(S_CA2T(pItemData->m_strName.c_str()));
	}

	BOOL OnEvtItemPanelClick(EventArgs* e)
	{
		EventItemPanelClick* pItemClickEvt = sobj_cast<EventItemPanelClick>(e);
		if (NULL == pItemClickEvt)
			return FALSE;

		SItemPanel* pItem = sobj_cast<SItemPanel>(pItemClickEvt->sender);
		if (pItem)
		{
			int nItem = static_cast<int>(pItem->GetItemIndex());
			ItemData* pData = m_vecItems[nItem];
			if (NULL != pData)
			{
				//m_pListener->OnEmotionItemClick(pData->m_strID);
				return TRUE;
			}
		}

		return TRUE;
	}

public:
	void AddItem(std::string& strID, std::string& strName, std::string& strAvatar)
	{
		ItemData* pItemData = new ItemData;
        pItemData->m_strID = strID;
		pItemData->m_strName = strName;
		pItemData->m_strAvatar = strAvatar;

		m_vecItems.push_back(pItemData);
		notifyDataSetChanged();
	}

private:
	IListen* m_pListen;

	std::vector<ItemData*>	m_vecItems;				// 数据
};