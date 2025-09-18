﻿#include "souistd.h"
#include "control/SComboBox.h"

SNSBEGIN

//////////////////////////////////////////////////////////////////////////
SComboBox::SComboBox()
    : m_pListBox(NULL)
{
}

SComboBox::~SComboBox()
{
    if (m_pListBox)
    {
        m_pListBox->SetOwner(NULL);
        m_pListBox->SSendMessage(WM_DESTROY);
        m_pListBox->Release();
    }
}

BOOL SComboBox::CreateListBox(SXmlNode xmlNode)
{
    //创建列表控件
    SXmlNode listStyle = xmlNode.child(L"listStyle");
    SStringW strListClass = listStyle.attribute(L"wndclass").as_string(SListBox::GetClassName());
    SListBox *pListBox = sobj_cast<SListBox>(CreateChildByName(strListClass));
    if (!pListBox)
        return FALSE;
    m_pListBox = pListBox;

    m_pListBox->SetContainer(GetContainer());
    m_pListBox->InitFromXml(&listStyle);
    m_pListBox->SetAttribute(L"pos", L"0,0,-0,-0", TRUE);
    m_pListBox->SetAttribute(L"hotTrack", L"1", TRUE);
    m_pListBox->SetOwner(this); // chain notify message to combobox
    m_pListBox->SetVisible(FALSE);
    m_pListBox->SetID(IDC_DROPDOWN_LIST);
    m_pListBox->SSendMessage(UM_SETSCALE, GetScale());

    //初始化列表数据
    SXmlNode xmlNode_Items = xmlNode.child(L"items");
    if (xmlNode_Items)
    {
        SXmlNode xmlNode_Item = xmlNode_Items.child(L"item");
        while (xmlNode_Item)
        {

            SStringW strText = xmlNode_Item.attribute(L"text").value();
            if (strText.IsEmpty())
                strText = GetXmlText(xmlNode_Item);
            int iIcon = xmlNode_Item.attribute(L"icon").as_int(0);
            LPARAM lParam = xmlNode_Item.attribute(L"data").as_int(0);
            m_pListBox->AddString(S_CW2T(GETSTRING(strText)), iIcon, lParam);
            xmlNode_Item = xmlNode_Item.next_sibling(L"item");
        }
    }

    if (m_iInitSel != -1)
    {
        SetCurSel(m_iInitSel);
    }
    return TRUE;
}

int SComboBox::GetListBoxHeight()
{
    int nDropHeight = m_nDropHeight.toPixelSize(GetScale());
    if (GetCount())
    {
        int nItemHeight = m_pListBox->GetItemHeight();
        CRect rcMargin = m_pListBox->GetStyle().GetMargin();
        nDropHeight = smin(nDropHeight, (int)(nItemHeight * GetCount() + rcMargin.top + rcMargin.bottom));
    }
    return nDropHeight;
}

void SComboBox::OnCreateDropDown(SDropDownWnd *pDropDown)
{
    __baseCls::OnCreateDropDown(pDropDown);
    pDropDown->GetRoot()->InsertChild(m_pListBox);
    pDropDown->GetRoot()->UpdateChildrenPosition();
    pDropDown->GetRoot()->SDispatchMessage(UM_SETSCALE, GetScale(), 0);
    pDropDown->GetRoot()->SDispatchMessage(UM_SETCOLORIZE, m_crColorize, 0);

    m_pListBox->SetVisible(TRUE);
    m_pListBox->SetFocus();
    m_pListBox->EnsureVisible(GetCurSel());
}

void SComboBox::OnDestroyDropDown(SDropDownWnd *pDropDown)
{
    pDropDown->GetRoot()->RemoveChild(m_pListBox);
    m_pListBox->SetVisible(FALSE);
    m_pListBox->SetContainer(GetContainer());
    __baseCls::OnDestroyDropDown(pDropDown);
}

void SComboBox::OnSelChanged()
{
    m_pListBox->GetCurSel();
    if (m_pEdit && !m_pEdit->GetEventSet()->isMuted())
    {
        SStringT strText = GetLBText(m_pListBox->GetCurSel());
        m_pEdit->GetEventSet()->setMutedState(true);
        SComboBase::SetWindowText(strText);
        m_pEdit->GetEventSet()->setMutedState(false);
    }
    Invalidate();
    __baseCls::OnSelChanged();
}

BOOL SComboBox::FireEvent(IEvtArgs *evt)
{
    if (evt->IdFrom() == IDC_DROPDOWN_LIST && m_pDropDownWnd)
    {
        if (evt->GetID() == EventLBSelChanged::EventID)
        {
            OnSelChanged();
            return TRUE;
        }
        if (evt->GetID() == EventCmd::EventID)
        {
            CloseUp();
            return TRUE;
        }
    }
    return SComboBase::FireEvent(evt);
}

void SComboBox::OnScaleChanged(int nScale)
{
    __baseCls::OnScaleChanged(nScale);
    if (m_pListBox)
        m_pListBox->SSendMessage(UM_SETSCALE, GetScale());
}

HRESULT SComboBox::OnLanguageChanged()
{
    HRESULT hr = __baseCls::OnLanguageChanged();
    if (m_pListBox)
        m_pListBox->SSendMessage(UM_SETLANGUAGE);
    return hr;
}

BOOL SComboBox::SetCurSel(int iSel)
{
    if (m_pListBox->SetCurSel(iSel))
    {
        m_pListBox->EnsureVisible(iSel);
        OnSelChanged();
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}

int SComboBox::GetCurSel() const
{
    return m_pListBox->GetCurSel();
}

int SComboBox::GetCount() const
{
    return m_pListBox->GetCount();
}

LPARAM SComboBox::GetItemData(UINT iItem) const
{
    return m_pListBox->GetItemData(iItem);
}

BOOL SComboBox::SetItemData(UINT iItem, LPARAM lParam)
{
    return m_pListBox->SetItemData(iItem, lParam);
}

int SComboBox::InsertItem(int iPos, LPCTSTR pszText, int iIcon, LPARAM lParam)
{
    return m_pListBox->InsertString(iPos, pszText, iIcon, lParam);
}

BOOL SComboBox::DeleteString(int iPos)
{
    return m_pListBox->DeleteString(iPos);
}

void SComboBox::ResetContent()
{
    SetCurSel(-1);
    return m_pListBox->DeleteAll();
}

BOOL SComboBox::GetItemText(int iItem, BOOL bRawText, IStringT *str) const
{
    if (iItem < 0 || iItem >= GetCount())
        return FALSE;

    SStringT strRet = m_pListBox->GetText(iItem, bRawText);
    str->Copy(&strRet);
    return TRUE;
}

IListBox *SComboBox::GetIListBox(THIS)
{
    return GetListBox();
}

SNSEND
