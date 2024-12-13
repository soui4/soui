#pragma once
#include "CGlobalUnits.h"
#include <helper/SAdapterBase.h>

struct TreeItemData
{
	TreeItemData() :bGroup(false) {}
	std::string strID;
	INT32 gid;			//用户的组ID
	SStringT strImg;	//用户图像
	SStringT strName;	//用户名
	bool bGroup;

};

class CTvContactAdapter : public STreeAdapterBase<TreeItemData>
{
public:
	struct IListen
	{
		virtual void ContactTVItemClick(int nGID, const std::string& strID) = 0;
		virtual void ContactTVItemDBClick(int nGID, const std::string& strID) = 0;
		virtual void ContactTVItemRClick(int nGID, const std::string& strID) = 0;
	};

public:
	CTvContactAdapter(IListen* pListen)
	{
		m_pListen = pListen;

		{
			TreeItemData data;
			data.strName = L"新的朋友";
			data.gid = 1;
			data.bGroup = true;
			HSTREEITEM hRoot = InsertItem(data);
			SetItemExpanded(hRoot, TRUE);
			data.bGroup = FALSE;
			data.strID = "new_friends";
			data.strName = L"新的朋友";
			data.strImg = L"skin_default_personal32";
			InsertItem(data, hRoot);
		}

		{
			TreeItemData data1;
			data1.strName = L"公众号";
			data1.gid = 2;
			data1.bGroup = true;
			HSTREEITEM hRoot1 = InsertItem(data1);
			SetItemExpanded(hRoot1, TRUE);
			data1.bGroup = false;
			data1.strID = "page_gzh";
			data1.strName = L"公众号测试";
			data1.strImg = L"skin_default_gzh32";
			InsertItem(data1, hRoot1);
		}

		{
			TreeItemData data2;
			data2.strName = L"订阅号";
			data2.gid = 3;
			data2.bGroup = TRUE;
			HSTREEITEM hRoot2 = InsertItem(data2);
			SetItemExpanded(hRoot2, TRUE);

			data2.bGroup = false;
			data2.strID = "page_dyh";
			data2.strName = L"订阅号测试";
			data2.strImg = L"skin_default_dyh32";
			InsertItem(data2, hRoot2);
		}

		{
			TreeItemData data3;
			data3.strName = L"群聊";
			data3.gid = 4;
			data3.bGroup = TRUE;
			HSTREEITEM hRoot3 = InsertItem(data3);
			SetItemExpanded(hRoot3, TRUE);

			auto iter = CGlobalUnits::instance()->m_mapGroups.begin();
			for (; iter != CGlobalUnits::instance()->m_mapGroups.end(); iter++)
			{
				data3.bGroup = false;
				data3.strID = iter->second.m_strGroupID;
				SStringW sstrGroupName = S_CA2W(iter->second.m_strGroupName.c_str());
				data3.strName = sstrGroupName;
				data3.strImg = L"skin_default_group32";
				InsertItem(data3, hRoot3);
			}
		}

		{//此处可根据用户名首字符做分类处理
			TreeItemData data4;
			data4.strName = L"好友";
			data4.gid = 5;
			data4.bGroup = TRUE;
			HSTREEITEM hRoot4 = InsertItem(data4);
			SetItemExpanded(hRoot4, TRUE);
			auto iter = CGlobalUnits::instance()->m_mapPersonals.begin();
			for (; iter != CGlobalUnits::instance()->m_mapPersonals.end(); iter++)
			{
				data4.bGroup = false;
				data4.strID = iter->second.m_strID;
				SStringW sstrName = S_CA2W(iter->second.m_strName.c_str());
				data4.strName = sstrName;
				data4.strImg = L"skin_default_personal32";
				InsertItem(data4, hRoot4);
			}
		}
	}

	~CTvContactAdapter() {}

	virtual void getView(HSTREEITEM loc, SItemPanel* pItem, SXmlNode xmlTemplate)
	{
		ItemInfo& ii = m_tree.GetItemRef((HSTREEITEM)loc);
		int itemType = getViewType(loc);

		if (pItem->GetChildrenCount() == 0)
		{
			switch (itemType)
			{
			case 0:xmlTemplate = xmlTemplate.child(L"item_group");
				break;
			case 1:xmlTemplate = xmlTemplate.child(L"item_data");
				break;
			default:
				break;
			}
			pItem->InitFromXml(&xmlTemplate);
			if (itemType == 0)
			{//不让点	
				pItem->GetEventSet()->setMutedState(true);
			}
		}

		if (itemType == 1)
		{
			//关联事件
			pItem->GetEventSet()->subscribeEvent(EVT_ITEMPANEL_CLICK,
				Subscriber(&CTvContactAdapter::OnItemPanelClick, this));
			pItem->GetEventSet()->subscribeEvent(EVT_ITEMPANEL_DBCLICK,
				Subscriber(&CTvContactAdapter::OnItemPanelDBClick, this));
			pItem->GetEventSet()->subscribeEvent(EVT_ITEMPANEL_RCLICK,
				Subscriber(&CTvContactAdapter::OnItemPanelRClick, this));

			//设置值
			SStatic* pName = pItem->FindChildByName2<SStatic>(L"name");
			SASSERT(pName);
			pName->SetWindowText(ii.data.strName);

			SImageWnd* pFace = pItem->FindChildByName2<SImageWnd>(L"face");
			SASSERT(pFace);
			pItem->FindChildByName(L"face")->SetAttribute(L"skin", ii.data.strImg);
		}
		else
		{
			pItem->FindChildByName(L"hr")->SetVisible(ii.data.gid != 1);

			SStatic* pName = pItem->FindChildByName2<SStatic>(L"name");
			SASSERT(pName);
			pName->SetWindowText(ii.data.strName);
		}
	}

	virtual int getViewType(HSTREEITEM hItem) const
	{
		ItemInfo& ii = m_tree.GetItemRef((HSTREEITEM)hItem);
		if (ii.data.bGroup) return 0;
		else return 1;
	}

	virtual int getViewTypeCount() const
	{
		return 2;
	}

	BOOL OnItemPanelClick(EventArgs* e)
	{
		EventItemPanelClick* pEvt = sobj_cast<EventItemPanelClick>(e);
		if (NULL == pEvt) return true;

		SItemPanel* pItem = sobj_cast<SItemPanel>(pEvt->sender);
		if (NULL == pItem) return true;

		HSTREEITEM loc = (HSTREEITEM)pItem->GetItemIndex();
		ItemInfo& ii = m_tree.GetItemRef(loc);
		if (!ii.data.bGroup)
		{
			m_pListen->ContactTVItemClick(ii.data.gid, ii.data.strID);
		}

		return true;
	}

	BOOL OnItemPanelDBClick(EventArgs* e)
	{
		EventItemPanelDbclick* pEvt = sobj_cast<EventItemPanelDbclick>(e);
		if (NULL == pEvt) return true;

		SItemPanel* pItem = sobj_cast<SItemPanel>(pEvt->sender);
		if (NULL == pItem) return true;

		HSTREEITEM loc = (HSTREEITEM)pItem->GetItemIndex();
		ItemInfo& ii = m_tree.GetItemRef(loc);
		if (!ii.data.bGroup)
		{
			m_pListen->ContactTVItemDBClick(ii.data.gid, ii.data.strID);
		}

		return true;
	}

	BOOL OnItemPanelRClick(EventArgs* e)
	{
		EventItemPanelRclick* pEvt = sobj_cast<EventItemPanelRclick>(e);
		if (NULL == pEvt) return true;

		SItemPanel* pItem = sobj_cast<SItemPanel>(pEvt->sender);
		if (NULL == pItem) return true;

		HSTREEITEM loc = (HSTREEITEM)pItem->GetItemIndex();
		ItemInfo& ii = m_tree.GetItemRef(loc);
		if (!ii.data.bGroup)
		{
			m_pListen->ContactTVItemRClick(ii.data.gid, ii.data.strID);
		}

		return true;
	}

private:
	IListen* m_pListen;
};

