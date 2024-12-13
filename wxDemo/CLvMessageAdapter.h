#pragma once
#include <helper/SAdapterBase.h>
#include <vector>


class CLvMessageAdapter : public SAdapterBase
{
public:
	struct ItemData
	{
		int m_nType;
		SStringW m_sstrID;
		SStringW m_sstrAvatar;
		SStringW m_sstrName;
		SStringW m_sstrContent;
		SStringW m_sstrTime;
		bool	 m_bReminder;
	};

	struct IListen
	{
		virtual void OnMessageItemClick(int& nIndex) = 0;
	};

protected:
	BOOL OnEventLvSelChangeing(EventLVSelChanging* pEvt)
	{
		if (NULL == pEvt)
			return true;

		pEvt->bubbleUp = false;
		if (-1 == pEvt->iNewSel)				// 不能设置 -1 
			pEvt->bCancel = TRUE;

		return TRUE;
	}

	BOOL OnEventItemPanelClick(EventArgs* e)
	{
		EventItemPanelClick* pEvt = sobj_cast<EventItemPanelClick>(e);
		if (!pEvt) return false;

		SItemPanel* pItem = sobj_cast<SItemPanel>(pEvt->sender);
		if (!pItem) return false;

		int nIndex = static_cast<int>(pItem->GetItemIndex());
		ItemData* pItemData = m_vecItemDatas[nIndex];

		m_pListen->OnMessageItemClick(nIndex);
		return TRUE;
	}

public:
	CLvMessageAdapter(SListView* pOwner, IListen* pListen)
	{
		m_pOwner = pOwner;
		SASSERT(m_pOwner);
		m_pOwner->GetEventSet()->subscribeEvent(&CLvMessageAdapter::OnEventLvSelChangeing, this);

		m_pListen = pListen;
	}

	virtual int getCount()
	{
		return static_cast<int>(m_vecItemDatas.size());
	}

	virtual void getView(int position, SItemPanel* pItem, SXmlNode xmlTemplate)
	{
		if (0 == pItem->GetChildrenCount())
		{
			pItem->InitFromXml(&xmlTemplate);
			pItem->GetEventSet()->
				subscribeEvent(EventItemPanelClick::EventID,
					Subscriber(&CLvMessageAdapter::OnEventItemPanelClick, this));
		}

		size_t sPos = static_cast<size_t>(position);
		if (sPos >= m_vecItemDatas.size()) return;

		ItemData* pItemData = m_vecItemDatas[sPos];
		if (NULL == pItemData) return;

		//avatar
		SImageWnd* pAvatar = pItem->FindChildByName2<SImageWnd>(L"item_avatar");
		SASSERT(pAvatar);
		switch (pItemData->m_nType)
		{
		case -1:
		{
			//gzh
			pAvatar->SetAttribute(L"skin", L"skin_default_gzh32");
		}
		break;
		case -2:
		{
			//dyh
			pAvatar->SetAttribute(L"skin", L"skin_default_dyh32");
		}
		break;
		case -3:
		{
			//news
			pAvatar->SetAttribute(L"skin", L"skin_default_news32");
		}
		break;
		case 0:
		{
			//file helper
			pAvatar->SetAttribute(L"skin", L"skin_default_filehelper32");
		}
		break;
		case 1:
		{
			//personal 
			if (pItemData->m_sstrAvatar == L"")
			{
				//use default avatar
				pAvatar->SetAttribute(L"skin", L"skin_default_personal32");
			}
			else
			{
				//use default avatar
			}
		}
		break;
		case 2:
		{
			//group
			if (pItemData->m_sstrAvatar == L"")
			{
				//use default avatar
				pAvatar->SetAttribute(L"skin", L"skin_default_group32");
			}
			else
			{
				//use default avatar
			}
		}
		break;
		default:
			break;
		}

		//name
		SStatic* pName = pItem->FindChildByName2<SStatic>(L"item_name");
		SASSERT(pName);
		pName->SetWindowTextW(pItemData->m_sstrName);

		//content
		SStatic* pContent = pItem->FindChildByName2<SStatic>(L"item_content");
		SASSERT(pContent);
		pContent->SetWindowTextW(pItemData->m_sstrContent);

		//time
		//根据定义的时间类型处理展示，如果是时间戳则需要处理，如果是文本则直接展示
		SStatic* pTime = pItem->FindChildByName2<SStatic>(L"item_time");
		SASSERT(pTime);
		pTime->SetWindowTextW(pItemData->m_sstrTime);

		//reminder
		SImageWnd* pReminder = pItem->FindChildByName2<SImageWnd>(L"item_reminder");
		SASSERT(pReminder);
		if (pItemData->m_bReminder) 
		{
			//展示
			pReminder->SetAttribute(L"show", L"1");
		}
		else
		{
			//不展示
			pReminder->SetAttribute(L"show", L"0");
		}
	}

	void AddItem(SStringW& sstrID, int& nType, SStringW& sstrAvatar, SStringW& sstrName, SStringW& sstrContent, SStringW& sstrTime, bool& bReminder)
	{
		ItemData* data = new ItemData;
		data->m_nType = nType;
		data->m_sstrID = sstrID;
		data->m_sstrAvatar = sstrAvatar;
		data->m_sstrName = sstrName;
		data->m_sstrContent = sstrContent;
		data->m_sstrTime = sstrTime;
		data->m_bReminder = bReminder;
		m_vecItemDatas.push_back(data);

		notifyDataSetChanged();
	}

	void DeleteAllItem()
	{
		m_vecItemDatas.clear();
		notifyDataSetChanged();
	}

	ItemData* GetItemData(int& nIndex)
	{
		return m_vecItemDatas[nIndex];
	}

	void EnsureVisible(int& nIndex)
	{
		m_pOwner->EnsureVisible(nIndex);
		notifyDataSetChanged();
	}

private:
	std::vector<ItemData*>				m_vecItemDatas;				// 数据 

	SListView* m_pOwner;
	IListen* m_pListen;
};

