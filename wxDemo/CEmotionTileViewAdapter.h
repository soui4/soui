#pragma once
#include <vector>

#include <helper/SAdapterBase.h>
#include "CGlobalUnits.h"

class CEmotionTileViewAdapter : public SAdapterBase
{
public:
	struct ItemData
	{
		std::string m_strID;
	};

	struct IListener
	{
		virtual void OnEmotionItemClick(const std::string& strID) = 0;
	};

public:
	CEmotionTileViewAdapter(IListener* pListener)
		:m_pListener(pListener)
	{

	}
	~CEmotionTileViewAdapter() {}

protected:
	virtual int getCount() {
		return static_cast<int>(m_vecItems.size());
	}

	virtual void getView(int position, SItemPanel* pItem, SXmlNode xmlTemplate)
	{
		if (0 == pItem->GetChildrenCount())
		{
			pItem->InitFromXml(&xmlTemplate);
			pItem->GetEventSet()->
				subscribeEvent(EventItemPanelClick::EventID,
					Subscriber(&CEmotionTileViewAdapter::OnEvtItemPanelClick, this));
		}

		size_t sPos = static_cast<size_t>(position);
		if (sPos >= m_vecItems.size()) return;

		ItemData* pItemData = m_vecItems[sPos];
		if (NULL == pItemData) return;

		SImageWnd* pImg = pItem->FindChildByName2<SImageWnd>(L"emotion");
		if (NULL != pImg)
		{


			std::map<std::string, IBitmap*>::iterator iter =
				CGlobalUnits::instance()->m_mapFace.find(pItemData->m_strID);
			if (iter != CGlobalUnits::instance()->m_mapFace.end())
			{
				pImg->SetImage(iter->second);
			}
		}
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
				m_pListener->OnEmotionItemClick(pData->m_strID);
				return TRUE;
			}
		}

		return TRUE;
	}

public:
	void AddItem(const std::string& strID)
	{
		ItemData* pItemFaceData = new ItemData;
		pItemFaceData->m_strID = strID;

		m_vecItems.push_back(pItemFaceData);
		notifyDataSetChanged();
	}

private:
	IListener* m_pListener;

	std::vector<ItemData*>	m_vecItems;				// Êý¾Ý
};