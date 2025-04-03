// MainDlg.cpp : implementation of the CMainDlg class
//
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "MainDlg.h"	
#define kLogTag "maindlg"

#pragma warning(disable:4996)

CMainDlg::CMainDlg() : SHostWnd(_T("LAYOUT:XML_MAINWND"))
{
}

CMainDlg::~CMainDlg()
{
}

int CMainDlg::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	SetMsgHandled(FALSE);
	return 0;
}

BOOL CMainDlg::OnInitDialog(HWND hWnd, LPARAM lParam)
{
	SListView *pLvImg = FindChildByID2<SListView>(R.id.lv_thumbnail);
	m_imgAdapter.Attach(new CImageAdapter);
	pLvImg->SetAdapter(m_imgAdapter);

	CHttpDownloader::getSingletonPtr()->download("http://cdn.apc.360.cn/index.php?c=WallPaper&a=getAllCategoriesV2&from=360chrome",URL_CATEGORIES,-1);
	return 0;
}

//TODO:消息映射
void CMainDlg::OnClose()
{
	SNativeWnd::DestroyWindow();
}

void CMainDlg::OnMaximize()
{
	SendMessage(WM_SYSCOMMAND, SC_MAXIMIZE);
}
void CMainDlg::OnRestore()
{
	SendMessage(WM_SYSCOMMAND, SC_RESTORE);
}
void CMainDlg::OnMinimize()
{
	SendMessage(WM_SYSCOMMAND, SC_MINIMIZE);
}

void CMainDlg::OnSize(UINT nType, CSize size)
{
	SetMsgHandled(FALSE);
	
	SWindow *pBtnMax = FindChildByName(L"btn_max");
	SWindow *pBtnRestore = FindChildByName(L"btn_restore");
	if(!pBtnMax || !pBtnRestore) return;
	
	if (nType == SIZE_MAXIMIZED)
	{
		pBtnRestore->SetVisible(TRUE);
		pBtnMax->SetVisible(FALSE);
	}
	else if (nType == SIZE_RESTORED)
	{
		pBtnRestore->SetVisible(FALSE);
		pBtnMax->SetVisible(TRUE);
	}
}

void CMainDlg::OnDownloadFinish(EventArgs *e)
{
	EventDownloadFinish *e2 = sobj_cast<EventDownloadFinish>(e);
	if (!e2->data)
		return;
	if(e2->type == URL_CATEGORIES)
	{//
//#define MYERRORDEBUG
#ifdef MYERRORDEBUG
		std::string errorData = R"({"errno1":"0","errmsg":"\u6b63\u5e38","consume":"1","total":"18","data":[[],[],[],[],[],[],[],[],[],[],[],[],[],[],[],[],[],[]]})";
#endif
		Json::Reader reader;
		Json::Value  value;
#ifdef MYERRORDEBUG	
		if(reader.parse(errorData,value))
#else
		if (reader.parse(e2->data->c_str(), value))
#endif
		{
			if (value["errno"].asString() == "0")
			{
				SListBox *pLbCategory = FindChildByID2<SListBox>(R.id.lb_catetory);
				SASSERT(pLbCategory);

				Json::Value categories = value["data"];
				for(unsigned int i=0;i<categories.size();i++)
				{					
					try
					{
						string id = categories[i]["id"].asString();
						string name = categories[i]["name"].asString();

						pLbCategory->AddString(S_CA2T(name.c_str(), CP_UTF8), 0, atoi(id.c_str()));
					}
					catch (std::exception)
					{
						break;
					}
				}
			}else
			{
				SLOGE()<<"fatch categories error code:"<<value["error"].asString().c_str();
			}
		}else
		{
			SLOGE()<<"parse categories failed";
		}
	}else
	{
		m_imgAdapter->OnDownloadFinish(e2->url,*e2->data,e2->type,e2->catetory,e2->pImg);
	}
}

void CMainDlg::OnCategorySelChange(EventArgs *e)
{
	EventLBSelChanged *e2 = sobj_cast<EventLBSelChanged>(e);
	SListBox *pLbCategory = sobj_cast<SListBox>(e->Sender());
	int category = (int)pLbCategory->GetItemData(e2->nNewSel);
	
	m_imgAdapter->setCategory(category);
}

