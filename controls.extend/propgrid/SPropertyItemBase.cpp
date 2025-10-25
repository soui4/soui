﻿#include "stdafx.h"
#include "SPropertyItemBase.h"
#include "SPropertyGrid.h"

SNSBEGIN

int SPropertyItemBase::GetLevel() const
{
    int iLevel = 0;
    IPropertyItem *pParent = GetParent();
    while (pParent)
    {
        iLevel++;
        pParent = pParent->GetParent();
    }
    return iLevel;
}

BOOL SPropertyItemBase::IsExpand() const
{
    return m_bExpanded;
}

void SPropertyItemBase::Expand(BOOL bExpend)
{
    m_bExpanded = bExpend;
    GetOwner()->OnItemExpanded(this);
}

IPropertyItem *SPropertyItemBase::GetParent() const
{
    return m_pParent;
}

void SPropertyItemBase::SetParent(IPropertyItem *pParent)
{
    m_pParent = pParent;
}

IPropertyItem *SPropertyItemBase::GetItem(PROPITEMTYPE type) const
{
    switch (type)
    {
    case GPI_PARENT:
        return GetParent();
        break;
    case GPI_FIRSTCHILD:
        if (m_childs.IsEmpty())
            return NULL;
        else
            return m_childs.GetHead();
        break;
    case GPI_LASTCHILD:
        if (m_childs.IsEmpty())
            return NULL;
        else
            return m_childs.GetTail();
        break;
    case GPI_NEXTSIBLING:
    {
        SPropertyItemBase *pParent = (SPropertyItemBase *)GetParent();
        if (!pParent)
            return NULL;

        SPOSITION pos = pParent->m_childs.Find((IPropertyItemPtr)this);
        SASSERT(pos);
        pParent->m_childs.GetNext(pos);
        if (!pos)
            return NULL;
        return pParent->m_childs.GetAt(pos);
    }
    break;
    case GPI_PREVSIBLINIG:
    {
        SPropertyItemBase *pParent = (SPropertyItemBase *)GetParent();
        if (!pParent)
            return NULL;
        SPOSITION pos = pParent->m_childs.Find((IPropertyItemPtr)this);
        SASSERT(pos);
        pParent->m_childs.GetPrev(pos);
        if (!pos)
            return NULL;
        return pParent->m_childs.GetAt(pos);
    }
    break;
    }
    return NULL;
}

IPropertyItem *SPropertyItemBase::GetChildById(int nID) const
{
    SPOSITION pos = m_childs.GetHeadPosition();
    while (pos)
    {
        IPropertyItem *pItem = m_childs.GetNext(pos);
        if (pItem->GetID() == nID)
            return pItem;
    }
    return NULL;
}

SPropertyGrid *SPropertyItemBase::GetOwner() const
{
    return m_pOwner;
}

BOOL SPropertyItemBase::InsertChild(IPropertyItem *pChild, IPropertyItem *pInsertAfter /*=IC_LAST*/)
{
    if (pInsertAfter == IC_LAST)
    {
        m_childs.InsertAfter(NULL, pChild);
    }
    else if (pInsertAfter == IC_FIRST)
    {
        m_childs.InsertBefore(NULL, pChild);
    }
    else
    {
        SPOSITION pos = m_childs.Find(pInsertAfter);
        if (!pos)
            return FALSE;

        m_childs.InsertAfter(pos, pChild);
    }
    pChild->SetParent(this);
    pChild->AddRef();
    return TRUE;
}

int SPropertyItemBase::ChildrenCount() const
{
    return (int)m_childs.GetCount();
}

BOOL SPropertyItemBase::RemoveChild(IPropertyItem *pChild)
{
    SPOSITION pos = m_childs.Find(pChild);
    if (!pos)
        return FALSE;
    m_childs.RemoveAt(pos);
    pChild->Release();
    return TRUE;
}

SPropertyItemBase::~SPropertyItemBase()
{
    SPOSITION pos = m_childs.GetHeadPosition();
    while (pos)
    {
        IPropertyItemPtr pChild = m_childs.GetNext(pos);
        pChild->Release();
    }
    m_childs.RemoveAll();
}

BOOL SPropertyItemBase::InitFromXml(IXmlNode *pNode)
{
    SObject::InitFromXml(pNode);
    SXmlNode xmlNode(pNode);
    SXmlNode xmlProp = xmlNode.first_child();
    while (xmlProp)
    {
        IPropertyItem *pItem = GetOwner()->CreateItem(xmlProp.name());
        if (pItem)
        {
            SPropertyItemBase *pItem2 = static_cast<SPropertyItemBase *>(pItem);
            if (pItem2)
            {
                pItem2->InitFromXml(&xmlProp);
                InsertChild(pItem2);
                pItem2->Release();
            }
        }
        xmlProp = xmlProp.next_sibling();
    }
    return TRUE;
}

HRESULT SPropertyItemBase::OnAttrExpanded(const SStringW &strValue, BOOL bLoading)
{
    BOOL bExpanded = STRINGASBOOL(strValue);
    if (!bLoading)
        Expand(bExpanded);
    else
        m_bExpanded = bExpanded;
    return S_FALSE;
}

void SPropertyItemBase::OnValueChanged()
{
    if (GetParent())
        GetParent()->OnChildValueChanged(this);
    else
        GetOwner()->OnItemValueChanged(this);
}

SStringW SPropertyItemBase::GetExtendType() const{
    if(m_strExType.IsEmpty() && GetParent())
        return GetParent()->GetExtendType();
    return m_strExType;
}
IPropertyItem *SPropertyItemBase::FindChildByName(LPCWSTR pszName) const
{
    SPOSITION pos = m_childs.GetHeadPosition();
    while (pos)
    {
        IPropertyItem *pChild = m_childs.GetNext(pos);
        if (pChild->GetName2() == pszName)
            return pChild;
        IPropertyItem *pFind = pChild->FindChildByName(pszName);
        if (pFind)
            return pFind;
    }
    return NULL;
}

IPropertyItem *SPropertyItemBase::FindChildById(int nID)
{
    SPOSITION pos = m_childs.GetHeadPosition();
    while (pos)
    {
        IPropertyItem *pChild = m_childs.GetNext(pos);
        if (pChild->GetID() == nID)
            return pChild;
        IPropertyItem *pFind = pChild->FindChildById(nID);
        if (pFind)
            return pFind;
    }
    return NULL;
}

void SPropertyItemBase::SetReadOnly(BOOL bReadOnly)
{
    m_bReadOnly = bReadOnly;
    GetOwner()->OnItemInvalidate(this);
}

SNSEND
