#pragma once
#include <helper/SAdapterBase.h>
class CTestAdapterFix : public SAdapterBase
{
	int* m_pCbxSel;
public:

	CTestAdapterFix()
	{
		m_pCbxSel = new int[getCount()];
		memset(m_pCbxSel, 0, sizeof(int) * getCount());
	}

	~CTestAdapterFix()
	{
		delete[]m_pCbxSel;
	}

	virtual int WINAPI getCount()
	{
		return 50000;
	}

	virtual void WINAPI getView(int position, SItemPanel* pItem, SXmlNode xmlTemplate)
	{
		if (pItem->GetChildrenCount() == 0)
		{
			pItem->InitFromXml(&xmlTemplate);
		}

		SButton* pBtn = pItem->FindChildByName2<SButton>(L"btn_test");
		pBtn->SetWindowText(SStringT().Format(_T("button %d"), position));
		pBtn->GetRoot()->SetUserData(position);

		//由网友“从未来过” 修改的新事件订阅方式,采用模板函数从响应函数中自动提取事件类型，2016.12.13
		pBtn->GetEventSet()->subscribeEvent(EventCmd::EventID, Subscriber(&CTestAdapterFix::OnButtonClick, this));

		SComboBox* pCbx = pItem->FindChildByName2<SComboBox>(L"cbx_in_lv");
		if (pCbx)
		{
			pCbx->SetCurSel(m_pCbxSel[position]);
			pCbx->GetEventSet()->subscribeEvent(EventCBSelChange::EventID, Subscriber(&CTestAdapterFix::OnCbxSelChange, this));
		}
	}

	BOOL OnCbxSelChange(EventCBSelChange* pEvt)
	{
		SComboBox* pCbx = sobj_cast<SComboBox>(pEvt->Sender());
		int iItem = pCbx->GetRoot()->GetUserData();
		m_pCbxSel[iItem] = pCbx->GetCurSel();
		return true;
	}

	BOOL OnButtonClick(EventCmd* pEvt)
	{
		SButton* pBtn = sobj_cast<SButton>(pEvt->Sender());
		int iItem = pBtn->GetRoot()->GetUserData();
		SMessageBox(0, SStringT().Format(_T("button of %d item was clicked"), iItem), _T("haha"), MB_OK);
		return true;
	}
};


class CTestAdapterFixHorz : public SAdapterBase
{
public:

	CTestAdapterFixHorz()
	{
	}

	~CTestAdapterFixHorz()
	{
	}

	virtual int WINAPI getCount()
	{
		return 300;
	}

	virtual void WINAPI getView(int position, SItemPanel* pItem, SXmlNode xmlTemplate)
	{
		if (pItem->GetChildrenCount() == 0)
		{
			pItem->InitFromXml(&xmlTemplate);
		}

		SImageWnd* pImg = pItem->FindChildByName2<SImageWnd>(L"btn_icon");
		pImg->SetIcon(position % 9);
		SWindow* pText = pItem->FindChildByName(L"btn_text");
		pText->SetWindowText(SStringT().Format(_T("item_%d"), position + 1));

		pImg->GetParent()->GetEventSet()->subscribeEvent(EventCmd::EventID, Subscriber(&CTestAdapterFixHorz::OnButtonClick, this));
	}


	BOOL OnButtonClick(EventCmd* pEvt)
	{
		SWindow* pBtn = sobj_cast<SWindow>(pEvt->sender);
		SItemPanel* pItem = (SItemPanel*)pBtn->GetRoot();
		int iItem = pItem->GetItemIndex();
		SMessageBox(0, SStringT().Format(_T("button of %d item was clicked"), iItem), _T("haha"), MB_OK);
		return true;
	}
};

const wchar_t* KAttrName_Height[] = {
	L"oddHeight",
	L"evenHeight",
	L"evenSelHeight"
};

const wchar_t* KNodeName_Item[] = {
	L"itemOdd",
	L"itemEven",
	L"itemEvenHover"
};
class CTestAdapterFlex : public SAdapterBase
{
public:
	SLayoutSize m_nItemHeight[3];


	CTestAdapterFlex()
	{

	}

	virtual void WINAPI InitByTemplate(SXmlNode xmlTemplate)
	{
        m_nItemHeight[0] = GETLAYOUTSIZE(xmlTemplate.attribute(KAttrName_Height[0]).as_string());
        m_nItemHeight[1] = GETLAYOUTSIZE(xmlTemplate.attribute(KAttrName_Height[1]).as_string());
        m_nItemHeight[2] = GETLAYOUTSIZE(xmlTemplate.attribute(KAttrName_Height[2]).as_string());
	}

	virtual int WINAPI getCount() override
	{
		return 12340;
	}

	virtual int WINAPI getViewTypeCount() override { return 3; }

	virtual int WINAPI getItemViewType(int position, DWORD dwState) override
	{
		if (position % 2 == 0)
			return 0;//1,3,5,... odd lines
		else if (dwState & WndState_Hover)
			return 2;//even lines with check state
		else
			return 1;//even lines 
	}

	virtual void WINAPI getViewDesiredSize(SIZE *retSize,int position, SItemPanel *pItem, int nWid, int nHei) override
	{
		DWORD dwState = pItem->GetState();
		int viewType = getItemViewType(position, dwState);
        *retSize = CSize(0, m_nItemHeight[viewType].toPixelSize(pItem->GetScale())); // cx在listview，mclistview中没有使用，不需要计算
	}

	virtual void WINAPI getView(int position, SItemPanel* pItem, SXmlNode xmlTemplate)
	{
		if (pItem->GetChildrenCount() == 0)
		{
			int nViewType = getItemViewType(position, pItem->GetState());
			SXmlNode xmlNode = xmlTemplate.child(KNodeName_Item[nViewType]);
			pItem->InitFromXml(&xmlNode);
		}
		pItem->GetEventSet()->subscribeEvent(EventSwndStateChanged::EventID, Subscriber(&CTestAdapterFlex::OnItemStateChanged, this));

		SButton* pBtn = pItem->FindChildByName2<SButton>(L"btn_test");
		pBtn->SetWindowText(SStringT().Format(_T("button %d"), position));
		pBtn->SetUserData(position);
		pBtn->GetEventSet()->subscribeEvent(EVT_CMD, Subscriber(&CTestAdapterFlex::OnButtonClick, this));
	}

	BOOL OnItemStateChanged(IEvtArgs* e)
	{
		EventSwndStateChanged* e2 = sobj_cast<EventSwndStateChanged>(e);
		if (!EventSwndStateChanged_CheckState(e2, WndState_Hover)) return false;
		//通知界面重绘
		notifyDataSetInvalidated();
		return true;
	}

	BOOL OnButtonClick(IEvtArgs* pEvt)
	{
		SButton* pBtn = sobj_cast<SButton>(pEvt->Sender());
		int iItem = pBtn->GetUserData();
		SMessageBox(0, SStringT().Format(_T("button of %d item was clicked"), iItem), _T("haha"), MB_OK);
		return true;
	}

};

class CTestMcAdapterFix : public SMcAdapterBase
{
#define NUMSCALE 100000
public:
	struct SOFTINFO
	{
		const wchar_t* pszSkinName;
		const wchar_t* pszName;
		const wchar_t* pszDesc;
		float     fScore;
		DWORD     dwSize;
		const wchar_t* pszInstallTime;
		const wchar_t* pszUseTime;
	};

	SArray<SOFTINFO> m_softInfo;

public:
	CTestMcAdapterFix()
	{
		SOFTINFO info[] =
		{
			{
				L"skin_icon1",
				L"鲁大师",
				L"鲁大师是一款专业的硬件检测，驱动安装工具",
				5.4f,
				15 * (1 << 20),
				L"2015-8-5",
				L"今天"
			},
			{
				L"skin_icon2",
				L"PhotoShop",
				L"强大的图片处理工具",
				9.0f,
				150 * (1 << 20),
				L"2015-8-5",
				L"今天"
			},
			{
				L"skin_icon3",
				L"QQ7.0",
				L"腾讯公司出品的即时聊天工具",
				8.0f,
				40 * (1 << 20),
				L"2015-8-5",
				L"今天"
			},
			{
				L"skin_icon4",
				L"Visual Studio 2008",
				L"Microsoft公司的程序开发套件",
				9.0f,
				40 * (1 << 20),
				L"2015-8-5",
				L"今天"
			},
			{
				L"skin_icon5",
				L"YY8",
				L"YY语音",
				9.0f,
				20 * (1 << 20),
				L"2015-8-5",
				L"今天"
			},
			{
				L"skin_icon6",
				L"火狐浏览器",
				L"速度最快的浏览器",
				8.5f,
				35 * (1 << 20),
				L"2015-8-5",
				L"今天"
			},
			{
				L"skin_icon7",
				L"迅雷",
				L"迅雷下载软件",
				7.3f,
				17 * (1 << 20),
				L"2015-8-5",
				L"今天"
			}
		};


		for (int i = 0; i < ARRAYSIZE(info); i++)
		{
			m_softInfo.Add(info[i]);
		}
	}

	virtual int WINAPI getCount()
	{
		return m_softInfo.GetCount() * NUMSCALE;
	}

	SStringT getSizeText(DWORD dwSize)
	{
		int num1 = dwSize / (1 << 20);
		dwSize -= num1 * (1 << 20);
		int num2 = dwSize * 100 / (1 << 20);
		return SStringT().Format(_T("%d.%02dM"), num1, num2);
	}

	virtual void WINAPI getView(int position, SItemPanel* pItem, SXmlNode xmlTemplate)
	{
		if (pItem->GetChildrenCount() == 0)
		{
			pItem->InitFromXml(&xmlTemplate);
		}

		SOFTINFO* psi = m_softInfo.GetData() + position % m_softInfo.GetCount();
		pItem->FindChildByName(L"img_icon")->SetAttribute(L"skin", psi->pszSkinName);
		pItem->FindChildByName(L"txt_name")->SetWindowText(S_CW2T(psi->pszName));
		pItem->FindChildByName(L"txt_desc")->SetWindowText(S_CW2T(psi->pszDesc));
		pItem->FindChildByName(L"txt_score")->SetWindowText(SStringT().Format(_T("%1.2f 分"), psi->fScore));
		pItem->FindChildByName(L"txt_installtime")->SetWindowText(S_CW2T(psi->pszInstallTime));
		pItem->FindChildByName(L"txt_usetime")->SetWindowText(S_CW2T(psi->pszUseTime));
		pItem->FindChildByName(L"txt_size")->SetWindowText(getSizeText(psi->dwSize));
		SRatingBar* pRatingBar = pItem->FindChildByName2<SRatingBar>(L"rating_score");
		pRatingBar->SetValue(psi->fScore / 2);
		pItem->FindChildByName(L"txt_index")->SetWindowText(SStringT().Format(_T("第%d行"), position + 1));

		SButton* pBtnUninstall = pItem->FindChildByName2<SButton>(L"btn_uninstall");
		CRect rcBtn = pBtnUninstall->GetWindowRect();
		SMatrix mtx;
		//mtx.setTranslate(0,10);
		mtx.setRotate(10);
		mtx.preTranslate(-rcBtn.Width() / 2, -rcBtn.Height() / 2);
		mtx.postTranslate(rcBtn.Width() / 2, rcBtn.Height() / 2);
		pBtnUninstall->SetMatrix(mtx);
		pBtnUninstall->SetUserData(position);
		pBtnUninstall->GetEventSet()->subscribeEvent(EVT_CMD, Subscriber(&CTestMcAdapterFix::OnButtonClick, this));
	}

	BOOL OnButtonClick(IEvtArgs* pEvt)
	{
		SButton* pBtn = sobj_cast<SButton>(pEvt->Sender());
		int iItem = pBtn->GetUserData();

		if (SMessageBox(0, SStringT().Format(_T("Are you sure to uninstall the selected [%d] software?"), iItem), _T("uninstall"), MB_OKCANCEL | MB_ICONQUESTION) == IDOK)
		{//删除一条记录
			DeleteItem(iItem);
		}
		return true;
	}

	//删除一行，提供外部调用。
	void DeleteItem(int iPosition)
	{
		if (iPosition >= 0 && iPosition < getCount())
		{
			int iItem = iPosition % m_softInfo.GetCount();
			m_softInfo.RemoveAt(iItem);
			notifyDataSetChanged();
		}
	}

	SStringW WINAPI GetColumnName(int iCol) const {
		return SStringW().Format(L"col%d", iCol + 1);
	}

	struct SORTCTX
	{
		int iCol;
		UINT fmt;
	};

	BOOL WINAPI OnSort(int iCol, UINT* pFmts, int nCols)
	{
		if (iCol == 5) //最后一列“操作”不支持排序
			return FALSE;

		UINT fmt = pFmts[iCol];
		switch (fmt & SORT_MASK)
		{
		case 0:fmt = HDF_SORTUP; break;
		case HDF_SORTDOWN:fmt = HDF_SORTUP; break;
		case HDF_SORTUP:fmt = HDF_SORTDOWN; break;
		}
		for (int i = 0; i < nCols; i++)
		{
			pFmts[i] &= ~SORT_MASK;
		}
		pFmts[iCol] |= fmt;

		SORTCTX ctx = { iCol,fmt };
		qsort_s(m_softInfo.GetData(), m_softInfo.GetCount(), sizeof(SOFTINFO), SortCmp, &ctx);
		return TRUE;
	}

	static int __cdecl SortCmp(void* context, const void* p1, const void* p2)
	{
		SORTCTX* pctx = (SORTCTX*)context;
		const SOFTINFO* pSI1 = (const SOFTINFO*)p1;
		const SOFTINFO* pSI2 = (const SOFTINFO*)p2;
		int nRet = 0;
		switch (pctx->iCol)
		{
		case 0://name
			nRet = wcscmp(pSI1->pszName, pSI2->pszName);
			break;
		case 1://score
		{
			float fCmp = (pSI1->fScore - pSI2->fScore);
			if (fabs(fCmp) < 0.0000001) nRet = 0;
			else if (fCmp > 0.0f) nRet = 1;
			else nRet = -1;
		}
		break;
		case 2://size
			nRet = (int)(pSI1->dwSize - pSI2->dwSize);
			break;
		case 3://install time
			nRet = wcscmp(pSI1->pszInstallTime, pSI2->pszInstallTime);
			break;
		case 4://user time
			nRet = wcscmp(pSI1->pszUseTime, pSI2->pszUseTime);
			break;

		}
		if (pctx->fmt & HDF_SORTUP)
			nRet = -nRet;
		return nRet;
	}
};



SStringW skins[5] = {
	L"skin_icon1",
	L"skin_icon2",
	L"skin_icon3",
	L"skin_icon4",
	L"skin_icon5"
};

class CTestTileAdapter : public SAdapterBase
{
public:
	CTestTileAdapter()
	{

	}
	virtual int WINAPI getCount()
	{
		return 50000;
	}

	virtual void WINAPI getView(int position, SItemPanel* pItem, SXmlNode xmlTemplate)
	{
		if (pItem->GetChildrenCount() == 0)
		{
			pItem->InitFromXml(&xmlTemplate);
		}
		SImageWnd* pImg = pItem->FindChildByName2<SImageWnd>(L"img_file_icon");
		pImg->SetSkin(GETSKIN(skins[position % 5], pImg->GetScale()));
		SButton* pBtn = pItem->FindChildByName2<SButton>(L"btn_test");
		pBtn->SetWindowText(SStringT().Format(_T("btn %d"), position));
		pBtn->GetRoot()->SetUserData(position);
		pBtn->GetEventSet()->subscribeEvent(EVT_CMD, Subscriber(&CTestTileAdapter::OnButtonClick, this));
	}

	BOOL OnButtonClick(IEvtArgs* pEvt)
	{
		SButton* pBtn = sobj_cast<SButton>(pEvt->Sender());
		int iItem = pBtn->GetRoot()->GetUserData();
		SMessageBox(0, SStringT().Format(_T("button of %d item was clicked"), iItem), _T("haha"), MB_OK);
		return true;
	}

};


struct TviBase
{
    SStringT strName;
    BOOL bHot;
    virtual ~TviBase()
    {
    }
};

struct TreeItemData1 : TviBase
{
    SStringT strUrl;
    SStringT strPlatform;
};

struct TreeItemData2 : TviBase
{
};

struct TreeItemData3 : TviBase
{
    int id;
};

enum NodeLevel
{
	Level1=0,
	Level2,
	Level3,
};

struct TreeItemData
{
    NodeLevel nLevel;
    TviBase *data;
    ~TreeItemData()
    {
        delete data;
    }
};

static void OnTvItemDataFree(TreeItemData *data)
{
    delete data;
}

class CTreeViewAdapter : public STreeAdapterBase<TreeItemData *> {
public:

	CTreeViewAdapter() {
        SetDataFreer(OnTvItemDataFree);
	}

	~CTreeViewAdapter() {}

	STDMETHOD_(void, InitByTemplate)(SXmlNode xmlTemplate)
    {
        SStringW strSrc = xmlTemplate.attribute(L"data").as_string();
        SXmlDoc xmlDoc;
        if (LOADXML(xmlDoc, S_CW2T(strSrc)))
        {
            SXmlNode root = xmlDoc.root().child(L"roomlist");
            SXmlNode nodePlatform = root.child(L"platform");
            HSTREEITEM hParent = STVI_ROOT;
            while (nodePlatform)
            {
                TreeItemData *pData = new TreeItemData;
                pData->nLevel = Level1;
                TreeItemData1 *pNode = new TreeItemData1;
                pNode->strPlatform = S_CW2T(nodePlatform.attribute(L"platform").as_string());
                pNode->strName = S_CW2T(nodePlatform.attribute(L"name").as_string());
                pNode->strUrl = S_CW2T(nodePlatform.attribute(L"url").as_string());
                pNode->bHot = nodePlatform.attribute(L"hot").as_bool();
                pData->data = pNode;
                HSTREEITEM h1=InsertItem(pData, hParent);
                BOOL bExpand = nodePlatform.attribute(L"expand").as_bool();
                SetItemExpanded(h1, bExpand);
                SXmlNode nodeType = nodePlatform.child(L"type");
                while (nodeType)
                {
                    TreeItemData *pData = new TreeItemData;
                    pData->nLevel = Level2;
                    TreeItemData2 *pNode = new TreeItemData2;
                    pData->data = pNode;
                    pNode->bHot = nodeType.attribute(L"hot").as_bool();
                    pNode->strName = S_CW2T(nodeType.attribute(L"name").as_string());
                    HSTREEITEM h2 = InsertItem(pData, h1);
                    BOOL bExpand = nodeType.attribute(L"expand").as_bool();
                    SetItemExpanded(h2, bExpand);
                    SXmlNode nodeRoom = nodeType.child(L"room");
                    while (nodeRoom)
                    {
                        TreeItemData *pData = new TreeItemData;
                        pData->nLevel = Level3;
                        TreeItemData3 *pNode = new TreeItemData3;
                        pData->data = pNode;
                        pNode->bHot = nodeRoom.attribute(L"hot").as_bool();
                        pNode->id = nodeRoom.attribute(L"id").as_int();
                        pNode->strName = S_CW2T(nodeRoom.attribute(L"desc").as_string());
                        InsertItem(pData, h2);
                        nodeRoom = nodeRoom.next_sibling();
                    }
                    nodeType = nodeType.next_sibling();
                }
                nodePlatform = nodePlatform.next_sibling(L"platform");
            }
        }
    }

	virtual void WINAPI getView(HSTREEITEM loc, SItemPanel* pItem, SXmlNode xmlTemplate) {
		if (pItem->GetChildrenCount() == 0)
		{
			pItem->InitFromXml(&xmlTemplate);
		}
        TreeItemData *pData = GetItemData(loc);
        SWindow * txt = pItem->FindChildByName("txt_label");
        SWindow *btnUrl = pItem->FindChildByName("btn_go_offcial_url");
        btnUrl->SetVisible(false, false);
        txt->SetWindowText(pData->data->strName);

        if (pData->nLevel == Level1)
        {
            TreeItemData1 *pNode = (TreeItemData1 *)pData->data;
			if (!pNode->strUrl.IsEmpty())
            {
                btnUrl->SetVisible(true, false);
                btnUrl->SetAttribute(L"url", S_CT2W(pNode->strUrl), 0);
                btnUrl->GetEventSet()->subscribeEvent(EventCmd::EventID, Subscriber(&CTreeViewAdapter::OnBtnUrl, this));
            }
        }
        else if (pData->nLevel == Level2)
        {
            TreeItemData2 *pNode = (TreeItemData2 *)pData->data;
        }
        else if (pData->nLevel == Level3)
        {
            TreeItemData3 *pNode = (TreeItemData3 *)pData->data;
        }

        SImageWnd *img = pItem->FindChildByName2<SImageWnd>("img_state");

        if (pData->nLevel!= Level3)
        {
            BOOL expland = IsItemExpanded(loc);
            img->SetIcon(expland ? 1 : 0);
        }
        else
        {
            img->SetIcon(2);
        }

        SWindow * aniHot = pItem->FindChildByName("ani_hot");
        aniHot->SetVisible(pData->data->bHot, true);
	}

	BOOL OnBtnUrl(IEvtArgs *e)
    {
        SWindow *pSender = sobj_cast<SWindow>(e->Sender());
        SItemPanel *pItem = sobj_cast<SItemPanel>(pSender->GetRoot());
        HSTREEITEM hItem = (HSTREEITEM)pItem->GetItemIndex();
        const TreeItemData *data = GetItemData(hItem);
        SASSERT(data->nLevel == Level1);
        TreeItemData1 *pdata1 = (TreeItemData1 *)data->data;
        ShellExecute(0, _T("open"), pdata1->strUrl, NULL, NULL, SW_SHOWDEFAULT);
        return TRUE;
    }
};
