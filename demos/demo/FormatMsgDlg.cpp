#include "stdafx.h"
#include "FormatMsgDlg.h"

namespace SOUI
{
    CFormatMsgDlg::CFormatMsgDlg(void):SHostDialog(_T("layout:dlg_formatmsg"))
    {
        m_nRepeat=1;
    }

    CFormatMsgDlg::~CFormatMsgDlg(void)
    {
    }

    void CFormatMsgDlg::OnOK()
    {
        SEdit *pEdit = FindChildByName2<SEdit>(L"re_xmlinput");
        if (pEdit) {
            m_strMsg = pEdit->GetWindowText();
            SEdit* pRepeat = FindChildByName2<SEdit>(L"edit_repeat");
            SStringT strRepeat = pRepeat->GetWindowText();
            m_nRepeat = _tstoi(strRepeat);
        }
        EndDialog(IDOK);
    }
}
