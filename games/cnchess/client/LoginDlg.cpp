#include "stdafx.h"
#include "LoginDlg.h"
#include <protocol.h>
#include <commdlg.h>
#include <gif/SGifPlayer.h>
#include <vector>

#define kConfigFile _T("/cnchess_cfg.xml")

CLoginDlg::CLoginDlg() : SHostDialog(_T("LAYOUT:XML_LOGIN"))
{
    m_nAvatarId = 1;
    m_cSex = SEX_MALE;
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
                int iSex = node.attribute(L"sex").as_int(0);
                pComboSex->SetCurSel(iSex);
                m_cSex = (char)pComboSex->GetItemData(pComboSex->GetCurSel());

                int nId = node.attribute(L"avatar_id").as_int(1);
                SStringT strPath = S_CW2T(node.attribute(L"avatar_path").as_string(L""));
                if (nId < 0 || nId > 4) nId = 1;
                if (!strPath.IsEmpty() && GetFileAttributes(strPath) != INVALID_FILE_ATTRIBUTES)
                {
                    m_nAvatarId = -1;
                    m_strAvatarPath = strPath;
                }
                else
                {
                    m_nAvatarId = nId;
                    m_strAvatarPath.Empty();
                }
            }
        }
    }
    UpdateAvatarPreview();
    SetAvatarSelectionBorder(m_nAvatarId);
    return TRUE;
}

void CLoginDlg::OnClose()
{
    EndDialog(IDCANCEL);
}

void CLoginDlg::OnSelectBuiltinAvatar(int nId)
{
    m_nAvatarId = nId;
    m_strAvatarPath.Empty();
    UpdateAvatarPreview();
    SetAvatarSelectionBorder(nId);
}

void CLoginDlg::OnAvatarExt()
{
#ifdef _WIN32
    OPENFILENAME ofn;
    TCHAR szFile[MAX_PATH] = {0};
    ZeroMemory(&ofn, sizeof(ofn));
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = m_hWnd;
    ofn.lpstrFile = szFile;
    ofn.nMaxFile = _countof(szFile);
    ofn.lpstrFilter = _T("Images (*.png;*.jpg;*.jpeg;*.gif;*.bmp)\0*.png;*.jpg;*.jpeg;*.gif;*.bmp\0All Files (*.*)\0*.*\0");
    ofn.nFilterIndex = 1;
    ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_HIDEREADONLY;
    if (GetOpenFileName(&ofn))
    {
        m_strAvatarPath = szFile;
        m_nAvatarId = -1;
        UpdateAvatarPreview();
        SetAvatarSelectionBorder(-1);
    }
#else
    SMessageBox(m_hWnd, _T("File picker for non-Windows platforms not implemented yet.\nPlease use the Windows build to browse external images."), _T("Notice"), MB_OK | MB_ICONINFORMATION);
#endif
}

void CLoginDlg::UpdateAvatarPreview()
{
    SGifPlayer *pGpPreview = FindChildByName2<SGifPlayer>(L"gp_preview");
    if (!pGpPreview) return;

    if (m_nAvatarId >= 0 && m_strAvatarPath.IsEmpty())
    {
        SStringW strSkinName;
        strSkinName.Format(L"gif_avatar_builtin_%d", m_nAvatarId);
        ISkinObj *pSkin = GETSKIN(strSkinName, GetScale());
        SSkinAni *pAniSkin = sobj_cast<SSkinAni>(pSkin);
        if (pAniSkin) pGpPreview->SetAniSkin(pAniSkin);
    }
    else if (!m_strAvatarPath.IsEmpty())
    {
        FILE *f = _tfopen(m_strAvatarPath, _T("rb"));
        if (!f) return;
        fseek(f, 0, SEEK_END);
        long sz = ftell(f);
        if (sz <= 0 || sz > 4*1024*1024) { fclose(f); return; }
        fseek(f, 0, SEEK_SET);
        std::vector<BYTE> buf((size_t)sz);
        size_t rd = fread(buf.data(), 1, (size_t)sz, f);
        fclose(f);
        if (rd == (size_t)sz)
        {
            pGpPreview->LoadFromMemory(buf.data(), buf.size());
        }
    }
}

void CLoginDlg::SetAvatarSelectionBorder(int nId)
{
    for (int i = 1; i <= 5; ++i)
    {
        SStringT strName;
        strName.Format(_T("btn_avatar_%d"), i);
        SWindow *pBtn = FindChildByName(strName);
        if (!pBtn) continue;
        bool selected = (nId == i);
        pBtn->SetAlpha(selected ? 255 : 160);
    }
}

void CLoginDlg::OnLogin()
{
    SWindow * pEdtSvr = FindChildByName2<SEdit>(L"edt_svr");
    SWindow * pEdtName = FindChildByName2<SEdit>(L"edt_name");
    m_strSvr = pEdtSvr->GetWindowText();
    m_strName = pEdtName->GetWindowText();
    SComboBox * pComboSex = FindChildByName2<SComboBox>(L"cbx_sex");
    int iSel = pComboSex->GetCurSel();
    m_cSex = (char)pComboSex->GetItemData(iSel);

    SXmlDoc doc;
    SXmlNode node = doc.root().child2(L"config");
    if (node)
    {
        node.attribute2(L"svr").set_value(S_CT2W(m_strSvr));
        node.attribute2(L"name").set_value(S_CT2W(m_strName));
        node.attribute2(L"sex").set_value(m_cSex);
        if (!m_strAvatarPath.IsEmpty())
        {
            node.attribute2(L"avatar_id").set_value(-1);
            node.attribute2(L"avatar_path").set_value(S_CT2W(m_strAvatarPath));
        }
        else
        {
            int nId = (m_nAvatarId >= 0 && m_nAvatarId <= 4) ? m_nAvatarId : 1;
            node.attribute2(L"avatar_id").set_value(nId);
            node.attribute2(L"avatar_path").set_value(L"");
        }
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
