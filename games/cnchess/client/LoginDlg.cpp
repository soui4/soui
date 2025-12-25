#include "stdafx.h"
#include "LoginDlg.h"

#define kConfigFile _T("cnchess_cfg.xml")

CLoginDlg::CLoginDlg() : SHostDialog(_T("LAYOUT:XML_LOGIN"))
{
}

BOOL CLoginDlg::OnInitDialog(HWND wndFocus, LPARAM lInitParam)
{
    SStringT strCfg = SApplication::getSingleton().GetAppDir() + kConfigFile;
    if(GetFileAttributes(strCfg) != INVALID_FILE_ATTRIBUTES)
    {
        SXmlDoc doc;
        if(doc.load_file(strCfg))
        {
            SXmlNode node = doc.root().child(L"config");
            if(node)
            {
                SWindow * pEdtSvr = FindChildByName2<SEdit>(L"edt_svr");
                pEdtSvr->SetWindowText(S_CW2T(node.attribute(L"svr").as_string()));

                SWindow * pEdtName = FindChildByName2<SEdit>(L"edt_name");
                pEdtName->SetWindowText(S_CW2T(node.attribute(L"name").as_string()));

                SComboBox * pComboSex = FindChildByName2<SComboBox>(L"cbx_sex");
                pComboSex->SetCurSel(node.attribute(L"sex").as_int(0));
            }
        }
    }
    return TRUE;
}

void CLoginDlg::OnClose()
{
    EndDialog(IDCANCEL);
}

void CLoginDlg::OnLogin()
{
    SWindow * pEdtSvr = FindChildByName2<SEdit>(L"edt_svr");
    SWindow * pEdtName = FindChildByName2<SEdit>(L"edt_name");
    m_strSvr = pEdtSvr->GetWindowText();
    m_strName = pEdtName->GetWindowText();
    SComboBox * pComboSex = FindChildByName2<SComboBox>(L"cbx_sex");
    int iSel = pComboSex->GetCurSel();
    m_cSex = pComboSex->GetItemData(iSel);

    SXmlDoc doc;
    SXmlNode node = doc.root().child2(L"config");
    if (node)
    {
        node.attribute2(L"svr").set_value(S_CT2W(m_strSvr));
        node.attribute2(L"name").set_value(S_CT2W(m_strName));
        node.attribute2(L"sex").set_value(m_cSex);
    }
    SStringT strCfg = SApplication::getSingleton().GetAppDir() + kConfigFile;
    doc.save_file(strCfg);
    EndDialog(IDOK);
}

void CLoginDlg::OnBtnLocal()
{
    SWindow * pEdtSvr = FindChildByName2<SEdit>(L"edt_svr");
    pEdtSvr->SetWindowText(_T("127.0.0.1:3311"));
}