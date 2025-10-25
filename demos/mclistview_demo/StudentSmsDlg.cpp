#include "stdafx.h"
#include "StudentSmsDlg.h"
#include <helper/SAdapterBase.h>
#include <helper/STime.h>

SNSBEGIN

class SStudentSearchAdapter : public SAdapterBase
{
friend class CStudentAdapter;

public:
    struct StudentInfo2
    {
        SStringT strText;
        int      bExpired;
        int      nPos;
    };

    StudentInfo2 GetItem(int i){return m_searchResult[i];}
protected:
    virtual int WINAPI getCount() override
    {
        return m_searchResult.GetCount();
    }

    STDMETHOD_(void, getView)(int position, SItemPanel *pItem, SXmlNode xmlTemplate)  override
    {
        if(pItem->GetChildrenCount() == 0)
        {
            pItem->InitFromXml(&xmlTemplate);
        }
        pItem->FindChildByID(R.id.txt_key)->SetWindowText(m_searchResult[position].strText);
    }

protected:
    SArray<StudentInfo2> m_searchResult;
};

class CStudentAdapter : public SMcAdapterBase
{
protected:
    enum{
        VT_GROUP,
        VT_DATA,
    };
    struct StudentInfo{
        SStringT strNick;
        UINT     imid;//yy 号
        SOUI::STime tm1;
        SOUI::STime tm2;
        int         nLoyalDegree;
        BOOL        bChecked;
    };
    
    SArray<StudentInfo> m_stuCurrent,m_stuExpired;
    BOOL                m_bCurrentExpand, m_bExpiredExpand;
    BOOL                m_bAllCurrentChecked, m_bAllExpiredChecked;
    
    IStudentCheckChanged    * m_pSelChangedHandler;
public:
    
    CStudentAdapter(IStudentCheckChanged * pSelChangedHandler)
    :m_pSelChangedHandler(pSelChangedHandler)
    ,m_bCurrentExpand(TRUE),m_bExpiredExpand(FALSE)
    ,m_bAllCurrentChecked(FALSE),m_bAllExpiredChecked(FALSE)
    {
        StudentInfo data[] =
        {
            {
            _T("sb no.1"),100,
            SOUI::STime(2015,9,1,0,0,0),
            SOUI::STime(2016,9,1,0,0,0),
            10000,0
            },
            {
            _T("sb no.2"),200,
            SOUI::STime(2015,9,2,0,0,0),
            SOUI::STime(2016,9,2,0,0,0),
            100000,0
            },
            {
            _T("sb no.3"),300,
            SOUI::STime(2015,9,3,0,0,0),
            SOUI::STime(2016,9,3,0,0,0),
            100000,0
            },
        };
        for(int i=0;i<ARRAYSIZE(data);i++)
        {
            m_stuCurrent.Add(data[i]);
        }
        
        StudentInfo data2[] =
        {
            {
            _T("nb no.1"),400,
            SOUI::STime(2015,9,1,0,0,0),
            SOUI::STime(2016,9,1,0,0,0),
            0,0
            },
            {
            _T("nb no.2"),500,
            SOUI::STime(2015,9,2,0,0,0),
            SOUI::STime(2016,9,2,0,0,0),
            0,0
            },
            {
            _T("nb no.3"),600,
            SOUI::STime(2015,9,3,0,0,0),
            SOUI::STime(2016,9,3,0,0,0),
            0,0
            },
        };
        for(int i=0;i<ARRAYSIZE(data2);i++)
        {
            m_stuExpired.Add(data2[i]);
        }
    }
    
    int Search(const SStringT & strKey,SStudentSearchAdapter *pSearchAdapter)
    {
        if(strKey.IsEmpty()) 
            return 0;
        for(UINT i=0;i<m_stuCurrent.GetCount();i++)
        {
            SStringT str = SStringT().Format(_T("%s(%u)"),m_stuCurrent[i].strNick.c_str(),m_stuCurrent[i].imid);
            if(str.Find(strKey)!=-1)
            {
                SStudentSearchAdapter::StudentInfo2 info;
                info.strText = str;
                info.bExpired = FALSE;
                info.nPos = i;
                pSearchAdapter->m_searchResult.Add(info);
            }
        }
        for(UINT i=0;i<m_stuExpired.GetCount();i++)
        {
            SStringT str = SStringT().Format(_T("%s(%u)"),m_stuExpired[i].strNick.c_str(),m_stuExpired[i].imid);

            if(str.Find(strKey)!=-1)
            {
                SStudentSearchAdapter::StudentInfo2 info;
                info.strText = str;
                info.bExpired = TRUE;
                info.nPos = i;
                pSearchAdapter->m_searchResult.Add(info);
            }
        }
        return pSearchAdapter->m_searchResult.GetCount();
    }
    
    int EnsureKeyVisible(BOOL bExpired,int iItem)
    {
        if(!bExpired)
        {
            if(!m_bCurrentExpand)
            {
                m_bCurrentExpand = TRUE;
                notifyDataSetChanged();
            }
            return  1+iItem;
        }else
        {
            if(!m_bExpiredExpand)
            {
                m_bExpiredExpand = TRUE;
                notifyDataSetChanged();
            }
            return 2 + iItem + (m_bCurrentExpand?m_stuCurrent.GetCount():0);
        }
    }
protected:
    
    virtual int WINAPI getCount()
    {
        return 2 + (m_bCurrentExpand?m_stuCurrent.GetCount():0) + (m_bExpiredExpand?m_stuExpired.GetCount():0);
    }

    virtual int  WINAPI getItemViewType(int position)
    {
        if(position == 0 || position == 1+ (m_bCurrentExpand?m_stuCurrent.GetCount():0))
            return VT_GROUP;
        else
            return VT_DATA;
    }

    virtual int  WINAPI getViewTypeCount()
    {
        return 2;
    }

    virtual SStringW WINAPI GetColumnName(int iCol) const
    {
        SStringW colNames[] = {L"col_nick",L"col_time_span",L"col_loyal_degree"};
        return colNames[iCol];
    }

    STDMETHOD_(void, getView)(int position, SItemPanel *pItem, SXmlNode xmlTemplate)
    {
        int nViewType = getItemViewType(position);
        if(pItem->GetChildrenCount() == 0)
        {
            SXmlNode xmlItem;
            switch(nViewType)
            {
            case VT_GROUP: xmlItem = xmlTemplate.child(L"item_group");break;
            case VT_DATA: xmlItem = xmlTemplate.child(L"item_data");break;
            }
            pItem->InitFromXml(&xmlItem);
        }
        if(nViewType == VT_GROUP)
        {
            SToggle * pSwitch=pItem->FindChildByID2<SToggle>(R.id.tgl_tv_expand);
            pSwitch->SetToggle(position==0?m_bCurrentExpand:m_bExpiredExpand);
            pSwitch->GetEventSet()->subscribeEvent(EVT_CMD,Subscriber(&CStudentAdapter::OnBtnGroupExpand,this));
            pItem->FindChildByID(R.id.txt_group)->SetWindowText(S_CW2T(TR(GETSTRING(position==0?R.string.current_student:R.string.expired_student),L"")));
            pItem->GetEventSet()->subscribeEvent(EVT_ITEMPANEL_DBCLICK,Subscriber(&CStudentAdapter::OnGroupDblClick,this));
            SCheckBox *pGroupCheck = pItem->FindChildByID2<SCheckBox>(R.id.chk_select_group);
            pGroupCheck->GetEventSet()->subscribeEvent(EVT_STATECHANGED,Subscriber(&CStudentAdapter::OnGroupSelectCheckChanged,this));
            pGroupCheck->GetEventSet()->setMutedState(true);
            pGroupCheck->SetCheck(position==0?m_bAllCurrentChecked:m_bAllExpiredChecked);
            pGroupCheck->GetEventSet()->setMutedState(false);
        }else
        {
            int nBaseCurrent = 1;
            int nBaseExpired = 1 + (m_bCurrentExpand?m_stuCurrent.GetCount():0) + 1;
            if(position< nBaseExpired)
            {//current student
                StudentInfo & stuInfo = m_stuCurrent[position-nBaseCurrent];
                SCheckBox *pCheckBox = pItem->FindChildByID2<SCheckBox>(R.id.txt_nick);
                pCheckBox->GetEventSet()->subscribeEvent(EVT_STATECHANGED,Subscriber(&CStudentAdapter::OnStudentCheckChanged,this));
                pCheckBox->GetEventSet()->setMutedState(true);
                pCheckBox->SetCheck(stuInfo.bChecked);
                pCheckBox->GetEventSet()->setMutedState(false);
                pCheckBox->SetWindowText(SStringT().Format(_T("%s(%u)"),stuInfo.strNick.c_str(),stuInfo.imid));
                pItem->FindChildByID(R.id.txt_loyal_degree)->SetWindowText(SStringT().Format(_T("%d"),stuInfo.nLoyalDegree));
                pItem->FindChildByID(R.id.txt_time_span)->SetWindowText(stuInfo.tm1.Format(_T("%Y/%m/%d") + SStringT(_T(" - ")) + stuInfo.tm2.Format(_T("%Y/%m/%d"))));
            }else
            {//expired student
                StudentInfo & stuInfo = m_stuExpired[position-nBaseExpired];
                SCheckBox *pCheckBox = pItem->FindChildByID2<SCheckBox>(R.id.txt_nick);
                pCheckBox->GetEventSet()->subscribeEvent(EVT_STATECHANGED,Subscriber(&CStudentAdapter::OnStudentCheckChanged,this));
                pCheckBox->GetEventSet()->setMutedState(true);
                pCheckBox->SetCheck(stuInfo.bChecked);
                pCheckBox->GetEventSet()->setMutedState(false);
                pCheckBox->SetWindowText(SStringT().Format(_T("%s(%u)"),stuInfo.strNick.c_str(),stuInfo.imid));
                pItem->FindChildByID(R.id.txt_loyal_degree)->SetWindowText(SStringT().Format(_T("%d"),stuInfo.nLoyalDegree));
                pItem->FindChildByID(R.id.txt_time_span)->SetWindowText(stuInfo.tm1.Format(_T("%Y/%m/%d") + SStringT(_T(" - ")) + stuInfo.tm2.Format(_T("%Y/%m/%d"))));
            }
        }
    }
    
    BOOL OnBtnGroupExpand(IEvtArgs *e)
    {
        SToggle *pTgl = sobj_cast<SToggle>(e->Sender());
        SASSERT(pTgl);
        SItemPanel * pItem = sobj_cast<SItemPanel>(pTgl->GetRoot());
        SASSERT(pItem);
        int position = (int)pItem->GetItemIndex();
        
        if(position == 0)
            m_bCurrentExpand = pTgl->GetToggle();
        else
            m_bExpiredExpand = pTgl->GetToggle();        
        
        notifyDataSetChanged();
        
        return true;
    }
    
    BOOL OnGroupDblClick(IEvtArgs *e)
    {
        SItemPanel *pItem = sobj_cast<SItemPanel>(e->Sender());
        SASSERT(pItem);
        int position = (int)pItem->GetItemIndex();
        if(position == 0)   m_bCurrentExpand = !m_bCurrentExpand;
        else m_bExpiredExpand = !m_bExpiredExpand;
        notifyDataSetChanged();
        return true;
    }
    
    BOOL OnGroupSelectCheckChanged(IEvtArgs *e)
    {
        EventSwndStateChanged *e2 = sobj_cast<EventSwndStateChanged>(e);
        SASSERT(e2);
        if(!EventSwndStateChanged_CheckState(e2,WndState_Check))
            return false;
            
        SCheckBox *pCheckBox = sobj_cast<SCheckBox>(e->Sender());
        SASSERT(pCheckBox);
        SItemPanel * pItem = sobj_cast<SItemPanel>(pCheckBox->GetRoot());
        int position = (int)pItem->GetItemIndex();
        
        int nSelCur = 0, nSelExp=0;

        if(position == 0)
        {
            for(UINT i=0;i<m_stuCurrent.GetCount();i++)
            {
                m_stuCurrent[i].bChecked = (e2->dwNewState & WndState_Check)!=0;
                nSelCur += m_stuCurrent[i].bChecked;
            }
            
            for(UINT i=0;i<m_stuExpired.GetCount();i++)
            {
                nSelExp += m_stuExpired[i].bChecked;
            }

        }else
        {
            for(UINT i=0;i<m_stuCurrent.GetCount();i++)
            {
                nSelCur += m_stuCurrent[i].bChecked;
            }

            for(UINT i=0;i<m_stuExpired.GetCount();i++)
            {
                m_stuExpired[i].bChecked = (e2->dwNewState & WndState_Check)!=0;;
                nSelExp += m_stuExpired[i].bChecked;
            }
        }
        m_bAllCurrentChecked = nSelCur == m_stuCurrent.GetCount();
        m_bAllExpiredChecked = nSelExp == m_stuExpired.GetCount();
        
        notifyDataSetChanged();
        
        if(m_pSelChangedHandler)
            m_pSelChangedHandler->OnStudentCheckChanged(nSelCur,nSelExp);
        return true;
    }
    
    BOOL OnStudentCheckChanged(IEvtArgs *e)
    {
        EventSwndStateChanged *e2 = sobj_cast<EventSwndStateChanged>(e);
        
        if(!EventSwndStateChanged_CheckState(e2,WndState_Check))
            return false;
            
        SCheckBox *pCheck = sobj_cast<SCheckBox>(e->Sender());
        SASSERT(pCheck);
        SItemPanel * pItem = sobj_cast<SItemPanel>(pCheck->GetRoot());
        int position = (int)pItem->GetItemIndex();
        
        int nBaseCurrent = 1;
        int nBaseExpired = 1 + (m_bCurrentExpand?m_stuCurrent.GetCount():0) + 1;
        
        if(position < nBaseExpired)
        {
            m_stuCurrent[position - nBaseCurrent].bChecked = pCheck->IsChecked();
        }else
        {
            m_stuExpired[position - nBaseExpired].bChecked = pCheck->IsChecked();
        }
        
        if(m_pSelChangedHandler)
        {
            int nSelCur = 0, nSelExp=0;
            for(UINT i=0;i<m_stuCurrent.GetCount();i++)
            {
                nSelCur += m_stuCurrent[i].bChecked;
            }
            for(UINT i=0;i<m_stuExpired.GetCount();i++)
            {
                nSelExp += m_stuExpired[i].bChecked;
            }
            BOOL bAllCurrentChecked = nSelCur == m_stuCurrent.GetCount();
            BOOL bAllExpiredChecked = nSelExp == m_stuExpired.GetCount();
            if(bAllCurrentChecked!=m_bAllCurrentChecked || bAllExpiredChecked != m_bAllExpiredChecked)
            {
                m_bAllCurrentChecked = bAllCurrentChecked;
                m_bAllExpiredChecked = bAllExpiredChecked;
                notifyDataSetChanged();
            }
            m_pSelChangedHandler->OnStudentCheckChanged(nSelCur,nSelExp);
        }
        return true;
    }
};


class CSmsRecordAdapter : public SAdapterBase
{
protected:
    struct RecordInfo
    {
        SOUI::STime tmSend;
        SStringT strReceiver;
        SStringT strContent;
    };

    SArray<RecordInfo> m_smsRecord;

public:
    CSmsRecordAdapter()
    {
        RecordInfo recs[] =
        {
            {
            SOUI::STime(2015,9,1,0,0,0),
            _T("sb no.1"),
            _T("A股明天暴跌，有钱没钱使劲买。"),
            },
            {
            SOUI::STime(2015,9,1,0,0,0),
            _T("sb no.2"),
            _T("A股明天暴跌，有钱没钱使劲买。"),
            },
            {
            SOUI::STime(2015,9,1,0,0,0),
            _T("sb no.3"),
            _T("A股明天暴跌，有钱没钱使劲买。"),
            },
            {
            SOUI::STime(2015,9,1,0,0,0),
            _T("sb no.4"),
            _T("A股明天暴跌，有钱没钱使劲买。"),
            },
            {
            SOUI::STime(2015,9,1,0,0,0),
            _T("sb no.5"),
            _T("A股明天暴跌，有钱没钱使劲买。"),
            },
        };
        
        for(int i=0;i<ARRAYSIZE(recs);i++)
        {
            m_smsRecord.Add(recs[i]);
        }
    }

protected:
    virtual int WINAPI getCount()
    {
        return m_smsRecord.GetCount();
    }

    STDMETHOD_(void, getView)(int position, SItemPanel * pItem, SXmlNode xmlTemplate)
    {
        if(pItem->GetChildrenCount() == 0)
        {
            pItem->InitFromXml(&xmlTemplate);
        }
        RecordInfo & rec = m_smsRecord[position];
        pItem->FindChildByID(R.id.txt_send_time)->SetWindowText(rec.tmSend.Format(_T("%Y/%m/%d %H:%M:%S")));
        pItem->FindChildByID(R.id.txt_sms_receiver)->SetWindowText(rec.strReceiver);
        pItem->FindChildByID(R.id.txt_sms_content)->SetWindowText(rec.strContent);
    }

};

CStudentSmsDlg::CStudentSmsDlg(void)
: SOUI::SHostDialog(UIRES.layout.dlg_student_sms)
{
}

CStudentSmsDlg::~CStudentSmsDlg(void)
{
}

BOOL CStudentSmsDlg::OnInitDialog(HWND wndFocus, LPARAM lInitParam)
{
    m_wndSmsRecord = FindChildByID(R.id.wnd_record_frame);
    m_tvStudent = FindChildByID2<SMCListView>(R.id.tv_student);
    
    IMcAdapter *pTvAdapter = new CStudentAdapter(this);
    m_tvStudent->SetAdapter(pTvAdapter);
    pTvAdapter->Release();
    
    m_lvSmsRecord = FindChildByID2<SListView>(R.id.lv_sms_record);
    
    ILvAdapter *pLvAdapter = new CSmsRecordAdapter;
    m_lvSmsRecord->SetAdapter(pLvAdapter);
    pLvAdapter->Release();

	return TRUE;
}


void CStudentSmsDlg::OnBtnSmsRecord()
{
    m_wndSmsRecord->SetVisible(TRUE,TRUE);
}

void CStudentSmsDlg::OnSearchGetBuddyRect(IEvtArgs *e)
{
    EventDropdownListGetBuddyRect *e2 = sobj_cast<EventDropdownListGetBuddyRect>(e);
    SWindow *pEvtSrc = sobj_cast<SWindow>(e->Sender());
    pEvtSrc->GetParent()->GetWindowRect(&e2->rcBuddy);
}

void CStudentSmsDlg::OnLButtonDown(UINT nFlags, CPoint point)
{
    if(m_wndSmsRecord->IsVisible(TRUE) && !m_wndSmsRecord->GetWindowRect().PtInRect(point))
    {
        m_wndSmsRecord->SetVisible(FALSE,TRUE);
    }
    SetMsgHandled(FALSE);
}

void CStudentSmsDlg::OnStudentCheckChanged(int nSelCurrent, int nSelExpired)
{
    FindChildByID(R.id.txt_sms_checked_num)->SetWindowText(SStringT().Format(_T("%d"),nSelCurrent + nSelExpired));
}

void CStudentSmsDlg::OnSearchFillList(IEvtArgs *e)
{
    EventDropdownListFill * e2 = sobj_cast<EventDropdownListFill>(e);
    SASSERT(e2);
    CStudentAdapter * pStudentAdapter = (CStudentAdapter*)m_tvStudent->GetAdapter();
    SStudentSearchAdapter * pSearchAdapter = new SStudentSearchAdapter;
    if(pStudentAdapter->Search(e2->strKey,pSearchAdapter)>0)
    {
        IListView *pLvSearch = e2->pListView;
        pLvSearch->SetAdapter(pSearchAdapter);
        pLvSearch->SetSel(0);
        e2->bPopup = true;
    }else{
        e2->bPopup = false;
    }
    pSearchAdapter->Release();
}

void CStudentSmsDlg::OnSearchValue(IEvtArgs *e)
{
    EventDropdownListSelected * e2 = sobj_cast<EventDropdownListSelected>(e);
    IListView *pListView = e2->pListView;
    SStudentSearchAdapter *pAdapter = (SStudentSearchAdapter*)pListView->GetAdapter();
    if(e2->nValue == -1) return;
    
    SStudentSearchAdapter::StudentInfo2 searchResult = pAdapter->GetItem(e2->nValue);
    CStudentAdapter * pStudentAdapter = (CStudentAdapter*)m_tvStudent->GetAdapter();
    int iItem = pStudentAdapter->EnsureKeyVisible(searchResult.bExpired,searchResult.nPos);
    if(iItem != -1)
    {
        m_tvStudent->SetSel(iItem);
        m_tvStudent->EnsureVisible(iItem);
    }
}

void CStudentSmsDlg::OnSmsInputNotify(IEvtArgs *e)
{
    EventRENotify *e2 = sobj_cast<EventRENotify>(e);
    if(e2->iNotify != EN_CHANGE) return;
    SEdit *pEdit = sobj_cast<SEdit>(e->Sender());
    int nLen = pEdit->GetWindowTextLength();
    SWindow *pWordCount = FindChildByID(R.id.txt_wordcount); 
    pWordCount->SetWindowText(SStringT().Format(_T("%d"),nLen));
    pWordCount->EnableWindow(nLen<100,TRUE);
}

void CStudentSmsDlg::OnSearchInputNotify(IEvtArgs *e)
{
    EventRENotify *e2 = sobj_cast<EventRENotify>(e);
    if(e2->iNotify != EN_CHANGE) return;
    SEdit *pEdit = sobj_cast<SEdit>(e->Sender());
    SStringT strText = pEdit->GetWindowText();
    pEdit->GetParent()->FindChildByClass<SSearchDropdownList>()->DropDown(&strText);
}

void CStudentSmsDlg::OnScale(int nID)
{
	int nScale = 100;
	if(nID == R.id.scale_100)
    nScale = 100;
    else if(nID == R.id.scale_150)
    nScale = 150;
    else if(nID == R.id.scale_200)
    nScale = 200;


	int nCurScale = GetScale();
	CRect rcWnd;
    GetWindowRect(rcWnd);
	int OriWid = rcWnd.Width() * 100 / nCurScale;
	int OriHei = rcWnd.Height() * 100/ nCurScale;

	int nNewWid = OriWid * nScale / 100;
	int nNewHei = OriHei * nScale / 100;

	GetRoot()->SDispatchMessage(UM_SETSCALE,nScale,0);

	if(!IsZoomed()){
		SetWindowPos(0, 0, 0, nNewWid, nNewHei, SWP_NOZORDER | SWP_NOMOVE);
	}
}

SNSEND