#include "stdafx.h"
#include "SIPAddressCtrl.h"
#define SEP_EDIT 2

namespace SOUI
{
class SEditIP : public SEdit {
    DEF_SOBJECT(SEdit, L"ipedit")
  public:
    SEditIP();
    ~SEditIP();
    inline const BYTE GetFiled()
    {
        SStringT str = GetWindowText();
        return (BYTE)_ttoi(str);
    }
    inline void SetField(BYTE nField)
    {
        SStringT str;
        str.Format(_T("%u"), nField);
        SetWindowText(str);
    }
    inline void ClearEdit()
    {
        SetField(0);
    }

  protected:
    //重载OnFinalRelease来保证new和delete在同一个模块。这样就不需要向系统的窗口类厂注册SEditIP类了。
    virtual void WINAPI OnFinalRelease()
    {
        delete this;
    }
    void OnChar(UINT nChar, UINT nRepCnt, UINT nFlags);
    void OnSetFocus(SWND wndOld);
    void OnKillFocus(SWND wndFocus);
    SOUI_MSG_MAP_BEGIN()
    MSG_WM_CHAR(OnChar)
    MSG_WM_SETFOCUS_EX(OnSetFocus)
    MSG_WM_KILLFOCUS_EX(OnKillFocus)
    SOUI_MSG_MAP_END()
  private:
    HIMC m_hImcCopy;
};

SEditIP::SEditIP()
    : m_hImcCopy(0)
{
}

SEditIP::~SEditIP()
{
}

void SEditIP::OnChar(UINT nChar, UINT nRepCnt, UINT nFlags)
{
    if (nChar == '.' || isdigit(nChar))
    {
        if (nChar != '.')
            __baseCls::OnChar(nChar, nRepCnt, nFlags);

        if (GetWindowTextLength() > 2 || nChar == '.')
        {
            SWindow *pSwnd = GetWindow(GSW_NEXTSIBLING);
            if (NULL != pSwnd && pSwnd->IsClass(SEditIP::GetClassName()))
            {
                pSwnd->SetFocus();
            }
        }
    }
}

void SEditIP::OnSetFocus(SWND wndOld)
{
    __baseCls::OnSetFocus(wndOld);
    HWND hHost = GetContainer()->GetHostHwnd();
    m_hImcCopy = ImmGetContext(hHost);
    ImmAssociateContext(hHost, 0);
}

void SEditIP::OnKillFocus(SWND wndFocus)
{
    __baseCls::OnKillFocus(wndFocus);
    HWND hHost = GetContainer()->GetHostHwnd();
    ImmAssociateContext(hHost, m_hImcCopy);
    m_hImcCopy = 0;

    SStringT strValue = GetWindowText();
    UINT uiValue = _ttoi(strValue);
    if (uiValue > 255)
    {
        SetField(255);
    }
}

//////////////////////////////////////////////////////////////////////////
//
SIPAddressCtrl::SIPAddressCtrl(void)
{
    memset(m_editFields, 0, sizeof(m_editFields));
    memset(m_ipFields, 0, sizeof(m_ipFields));
}

SIPAddressCtrl::~SIPAddressCtrl(void)
{
}

void SIPAddressCtrl::OnSize(UINT nType, CSize size)
{
    __baseCls::OnSize(nType, size);

    if (!m_editFields[0])
        return;

    CRect rcClient;
    GetClientRect(&rcClient);

    int nEditWid = (rcClient.Width() - SEP_EDIT * 3) / 4;
    CRect rcEdit = rcClient;
    rcEdit.right = rcEdit.left + nEditWid;
    for (int i = 0; i < 4; i++)
    {
        m_editFields[i]->Move(&rcEdit);
        rcEdit.OffsetRect(nEditWid + SEP_EDIT, 0);
    }
}

HRESULT SIPAddressCtrl::OnAttrField(int iField, const SStringW &strValue, BOOL bLoading)
{
    BYTE nField = (BYTE)_wtoi(strValue);
    if (nField > 255)
        nField = 255;
    if (m_editFields[iField])
        m_editFields[iField]->SetField(nField);
    else
        m_ipFields[iField] = nField;
    return bLoading ? S_FALSE : S_OK;
}

HRESULT SIPAddressCtrl::OnAttrField0(const SStringW &strValue, BOOL bLoading)
{
    return OnAttrField(0, strValue, bLoading);
}

HRESULT SIPAddressCtrl::OnAttrField1(const SStringW &strValue, BOOL bLoading)
{
    return OnAttrField(1, strValue, bLoading);
}
HRESULT SIPAddressCtrl::OnAttrField2(const SStringW &strValue, BOOL bLoading)
{
    return OnAttrField(2, strValue, bLoading);
}
HRESULT SIPAddressCtrl::OnAttrField3(const SStringW &strValue, BOOL bLoading)
{
    return OnAttrField(3, strValue, bLoading);
}

HRESULT SIPAddressCtrl::OnAttrIP(const SStringW &strValue, BOOL bLoading)
{
    SStringWList strList;
    SplitString(strValue, L'.', strList);
    if (strList.GetCount() != 4)
        return E_INVALIDARG;
    for (int i = 0; i < 4; i++)
    {
        OnAttrField(i, strList.GetAt(i), bLoading);
    }
    return bLoading ? S_FALSE : S_OK;
}

LRESULT SIPAddressCtrl::OnCreate(LPVOID)
{
    wchar_t szEditAttr[]
        = L"<ipedit margin=\"0\" number=\"1\" transparent=\"1\" align=\"center\" maxBuf=\"3\" mouseRelay=\"1\"/>";
    SXmlDoc xmlDoc;
    xmlDoc.load_buffer(szEditAttr, sizeof(szEditAttr));
    for (int i = 0; i < 4; i++)
    {
        m_editFields[i] = new SEditIP; //直接new出来。
        InsertChild(m_editFields[i]);
        SXmlNode xmlNode = xmlDoc.root().first_child();
        m_editFields[i]->InitFromXml(&xmlNode);
        m_editFields[i]->SetField(m_ipFields[i]);
    }
    return 0;
}

void SIPAddressCtrl::OnPaint(IRenderTarget *pRT)
{
    CRect rcClient;
    GetClientRect(&rcClient);
    int nEditWid = (rcClient.Width() - SEP_EDIT * 3) / 4;
    CRect rcSep = rcClient;
    rcSep.left += nEditWid;
    rcSep.right = rcSep.left + SEP_EDIT;
    for (int i = 0; i < 3; i++)
    {
        CRect rcDot = rcSep;
        rcDot.top += (rcSep.Height() - SEP_EDIT) / 2;
        rcDot.bottom = rcDot.top + SEP_EDIT + 1;
        rcDot.right += 1;
        pRT->FillSolidRect(rcDot, RGBA(153, 153, 153, 255));
        rcSep.OffsetRect(nEditWid + SEP_EDIT, 0);
    }
}

BOOL SIPAddressCtrl::IsBlank() const
{
    DWORD empty = 0;
    if (!m_editFields[0])
        return memcmp(m_ipFields,&empty,4)==0;
    if (m_editFields[0]->GetWindowText().GetLength() == 0
        && m_editFields[1]->GetWindowText().GetLength() == 0
        && m_editFields[2]->GetWindowText().GetLength() == 0
        && m_editFields[3]->GetWindowText().GetLength() == 0)
    {
        return TRUE;
    }
    return FALSE;
}

int SIPAddressCtrl::GetAddress(BYTE &nField0, BYTE &nField1, BYTE &nField2, BYTE &nField3) const
{
    if (m_editFields[0])
    {
        nField0 = m_editFields[0]->GetFiled();
        nField1 = m_editFields[1]->GetFiled();
        nField2 = m_editFields[2]->GetFiled();
        nField3 = m_editFields[3]->GetFiled();
    }
    else
    {
        nField0 = m_ipFields[0];
        nField1 = m_ipFields[1];
        nField2 = m_ipFields[2];
        nField3 = m_ipFields[3];
    }
    return 0;
}

int SIPAddressCtrl::GetAddress(DWORD &dwAddress) const
{
    if (m_editFields[0])
    {
        BYTE nField0 = m_editFields[0]->GetFiled();
        BYTE nField1 = m_editFields[1]->GetFiled();
        BYTE nField2 = m_editFields[2]->GetFiled();
        BYTE nField3 = m_editFields[3]->GetFiled();
        dwAddress = MAKELONG(MAKEWORD(nField0, nField1), MAKEWORD(nField2, nField3));
    }
    else
    {
        dwAddress = MAKELONG(MAKEWORD(m_ipFields[0], m_ipFields[1]), MAKEWORD(m_ipFields[2], m_ipFields[3]));
    }
    return 0;
}

void SIPAddressCtrl::SetAddress(DWORD dwAddress)
{
    const BYTE *pbyAddr = (const BYTE *)&dwAddress;
    SetAddress(pbyAddr[0], pbyAddr[1], pbyAddr[2], pbyAddr[3]);
}

void SIPAddressCtrl::SetAddress(BYTE nField0, BYTE nField1, BYTE nField2, BYTE nField3)
{
    if (m_editFields[0])
    {
        m_editFields[0]->SetField(nField0);
        m_editFields[1]->SetField(nField1);
        m_editFields[2]->SetField(nField2);
        m_editFields[3]->SetField(nField3);
    }
    else
    {
        m_ipFields[0] = nField0;
        m_ipFields[1] = nField1;
        m_ipFields[2] = nField2;
        m_ipFields[3] = nField3;
    }
}

void SIPAddressCtrl::SetFieldFocus(WORD nField)
{
    if (nField < 4 && m_editFields[0])
    {
        m_editFields[nField]->SetFocus();
    }
}

void SIPAddressCtrl::ClearAddress()
{
    if (m_editFields[0])
    {
        m_editFields[0]->ClearEdit();
        m_editFields[1]->ClearEdit();
        m_editFields[2]->ClearEdit();
        m_editFields[3]->ClearEdit();
    }
    else
    {
        m_ipFields[0] = 0;
        m_ipFields[1] = 0;
        m_ipFields[2] = 0;
        m_ipFields[3] = 0;
    }
}
} // namespace SOUI
