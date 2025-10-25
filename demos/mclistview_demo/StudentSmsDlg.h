#ifndef __STUDENT_SMS_DLG_H__
#define __STUDENT_SMS_DLG_H__

SNSBEGIN
struct IStudentCheckChanged
{
    virtual void OnStudentCheckChanged(int nSelCurrent, int nSelExpired) PURE;
};

class CStudentSmsDlg
	: public SHostDialog
	, public IStudentCheckChanged
{
public:
    CStudentSmsDlg(void);
    ~CStudentSmsDlg(void);
    
protected:
    void OnBtnSmsRecord();
    void OnSearchGetBuddyRect(IEvtArgs *e);
    void OnSearchFillList(IEvtArgs *e);
    void OnSearchValue(IEvtArgs *e);
    void OnSmsInputNotify(IEvtArgs *e);
    void OnSearchInputNotify(IEvtArgs *e);
	void OnScale(int nID);
    EVENT_MAP_BEGIN()
        EVENT_ID_HANDLER(R.id.edit_sms_input,EventRENotify::EventID,OnSmsInputNotify)
        EVENT_ID_HANDLER(R.id.edit_input,EventRENotify::EventID,OnSearchInputNotify)
        EVENT_ID_COMMAND(R.id.btn_sms_record,OnBtnSmsRecord);
        EVENT_ID_HANDLER(R.id.search_dropdown,EventDropdownListGetBuddyRect::EventID,OnSearchGetBuddyRect)
        EVENT_ID_HANDLER(R.id.search_dropdown,EventDropdownListFill::EventID,OnSearchFillList)
        EVENT_ID_HANDLER(R.id.search_dropdown,EventDropdownListSelected::EventID,OnSearchValue)
		EVENT_ID_COMMAND_RANGE(R.id.scale_100,R.id.scale_200,OnScale)
    EVENT_MAP_END2(SHostDialog)

protected:
    BOOL OnInitDialog(HWND wndFocus, LPARAM lInitParam);
    void OnLButtonDown(UINT nFlags, CPoint point);

    BEGIN_MSG_MAP_EX(CStudentSmsDlg)
        MSG_WM_LBUTTONDOWN(OnLButtonDown)
        MSG_WM_INITDIALOG(OnInitDialog)
        CHAIN_MSG_MAP(SOUI::SHostDialog)
    END_MSG_MAP()

protected:
    virtual void OnStudentCheckChanged(int nSelCurrent, int nSelExpired);

protected:
	
    SWindow     * m_wndSmsRecord;
	SMCListView * m_tvStudent;
	SListView   * m_lvSmsRecord;
};

SNSEND
#endif//__STUDENT_SMS_DLG_H__
