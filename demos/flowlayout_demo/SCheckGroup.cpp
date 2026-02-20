#include "stdafx.h"
#include "SCheckGroup.h"

SNSBEGIN

static const int GroupCheck_ID_Start = 100000;

SCheckGroup::SCheckGroup(void):m_checkId(-1)
{
    m_evtSet.addEvent(EVENTID(EventGroupCheck));
}

SCheckGroup::~SCheckGroup(void)
{
}

void SCheckGroup::SetOptions(const SCheckItem * vOptions, size_t nCount)
{
    SXmlNode xmlItem = m_xmlTemplate.root().first_child();
    for (size_t i = 0; i < nCount; i++)
    {
        SWindow *pItem = (SWindow*)SApplication::getSingleton().CreateWindowByName(xmlItem.attribute(SCheckGroup_style::kStyle_wndclass).as_string(SWindow::GetClassName()));
        pItem->InitFromXml(&xmlItem);
        InsertChild(pItem);
        if(m_checkId != -1)
            pItem = pItem->FindChildByID(m_checkId);
        pItem->SetID(GroupCheck_ID_Start + i);
        pItem->SetWindowText(vOptions[i].strText);
        pItem->GetEventSet()->setMutedState(TRUE);
        pItem->SetCheck(vOptions[i].bChecked);
        pItem->GetEventSet()->setMutedState(FALSE);
        pItem->GetEventSet()->subscribeEvent(EventSwndStateChanged::EventID, Subscriber(&SCheckGroup::OnCheckChange, this));
    }
}

BOOL SCheckGroup::CreateChildren(SXmlNode xmlNode)
{
    SXmlNode xmlTemplate = xmlNode.child(SCheckGroup_style::kStyle_template);
    if (!xmlTemplate)
        return FALSE;
    m_xmlTemplate.Reset();
    m_xmlTemplate.root().append_copy(xmlTemplate);
    m_checkId = xmlTemplate.attribute(L"checkid").AsInt(-1);
    return TRUE;
}

BOOL SCheckGroup::OnCheckChange(IEvtArgs *e)
{
    EventSwndStateChanged *e2 = sobj_cast<EventSwndStateChanged>(e);
    if (!EventSwndStateChanged_CheckState(e2, WndState_Check))
        return FALSE;
    EventGroupCheck evt(this);
    evt.iItem = e2->sender->GetID() - GroupCheck_ID_Start;
    FireEvent(evt);
    return TRUE;
}

SNSEND
