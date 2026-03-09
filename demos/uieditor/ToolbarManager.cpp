#include "stdafx.h"
#include "ToolbarManager.h"
#include "SysdataMgr.h"
#include "XmlEditor.h"
#include "Dialog/DlgInsertXmlElement.h"
#include "ResManger.h"
SNSBEGIN
CToolbarManager::CToolbarManager(SSkinPool* pSkinPool)
    : m_pSkinPool(pSkinPool)
    , m_tbWidget(NULL)
    , m_tbSkin(NULL)
{
}

CToolbarManager::~CToolbarManager()
{
}

void CToolbarManager::LoadToolbarIcons(SXmlNode xmlIconSkin, ISkinObj*& pSkin)
{
    pSkin = SApplication::getSingleton().CreateSkinByName(xmlIconSkin.attribute(L"class_name").as_string(SSkinImgList::GetClassName()));
    if (pSkin)
    {
        pSkin->InitFromXml(&xmlIconSkin);
        m_pSkinPool->AddSkin(pSkin);
        pSkin->Release();
    }
}

void CToolbarManager::InitWidgetToolbar(SWindow* pParent, CXmlEditor* pXmlEditor)
{
    SXmlNode xmlNode = CSysDataMgr::getSingleton().getCtrlDefNode();
    SXmlNode xmlIconSkin = xmlNode.child(L"toolbar").child(L"icons");
    ISkinObj *pSkin = NULL;
    LoadToolbarIcons(xmlIconSkin, pSkin);
    
    SArray<IconInfo> arrIcons;
    SXmlNode xmlWidget = xmlNode.child(L"toolbar").child(L"items").first_child();
    while (xmlWidget)
    {
        IconInfo info;
        info.iIcon = xmlWidget.attribute(L"iconIndex").as_int(0);
        SStringW strTxt = xmlWidget.attribute(L"name").as_string();
        info.strTxt = S_CW2T(strTxt);
        info.strTip = S_CW2T(xmlWidget.attribute(L"tip").as_string());
        SStringW strContent;
        xmlWidget.child(strTxt).ToString(&strContent);
        arrIcons.Add(info);
        m_mapWidget[info.strTxt] = strContent;
        xmlWidget = xmlWidget.next_sibling();
    }
    
    SToolBar *pToolBar = pParent->FindChildByID2<SToolBar>(R.id.tb_widget);
    if (pToolBar && pSkin)
    {
        pToolBar->SetIconsSkin(pSkin);
        for(int i = 0; i < arrIcons.GetCount(); i++)
        {
            pToolBar->AddButton(i, arrIcons[i].iIcon, arrIcons[i].strTxt);
        }
        m_tbWidget = pToolBar;
    }
}

void CToolbarManager::InitSkinToolbar(SWindow* pParent)
{
    SXmlNode xmlNode = CSysDataMgr::getSingleton().getSkinDefNode();
    SXmlNode xmlIconSkin = xmlNode.child(L"toolbar").child(L"icons");
    ISkinObj *pSkin = NULL;
    LoadToolbarIcons(xmlIconSkin, pSkin);
    
    SArray<IconInfo> arrIcons;
    SXmlNode xmlSkin = xmlNode.child(L"toolbar").child(L"items").first_child();
    while (xmlSkin)
    {
        IconInfo info;
        info.iIcon = xmlSkin.attribute(L"iconIndex").as_int(0);
        info.strTxt = S_CW2T(xmlSkin.attribute(L"name").as_string());
        info.strTip = S_CW2T(xmlSkin.attribute(L"tip").as_string());
        arrIcons.Add(info);
        xmlSkin = xmlSkin.next_sibling();
    }
    
    SToolBar *pToolBar = pParent->FindChildByID2<SToolBar>(R.id.tb_skin);
    if (pToolBar && pSkin)
    {
        pToolBar->SetIconsSkin(pSkin);
        for (int i = 0; i < arrIcons.GetCount(); i++)
        {
            pToolBar->AddButton(i, arrIcons[i].iIcon, arrIcons[i].strTxt);
        }
        m_tbSkin = pToolBar;
    }
}

void CToolbarManager::UpdateEditorToolbar(int editXmlType)
{
    if (m_tbWidget)
    {
        m_tbWidget->SetVisible(editXmlType == FT_LAYOUT_XML, TRUE);
    }
    if (m_tbSkin)
    {
        m_tbSkin->SetVisible(editXmlType == FT_SKIN, TRUE);
    }
}

void CToolbarManager::OnTbWidgetClick(IEvtArgs *e, CXmlEditor* pXmlEditor)
{
    if (!m_tbWidget || !pXmlEditor)
        return;
    
    EventToolBarCmd *e2 = sobj_cast<EventToolBarCmd>(e);
    if (!e2)
        return;
    
    ToolBarItem item;
    m_tbWidget->GetItemInfo(e2->iItem, &item);
    if (SMap<SStringT, SStringW>::CPair *pair = m_mapWidget.Lookup(item.strText))
    {
        pXmlEditor->InsertWidget(S_CW2A(pair->m_value, CP_UTF8));
    }
}

void CToolbarManager::OnTbSkinClick(IEvtArgs *e, ResManger* pResManger, CXmlEditor* pXmlEditor)
{
    if (!m_tbSkin || !pResManger || !pXmlEditor)
        return;
    
    EventToolBarCmd *e2 = sobj_cast<EventToolBarCmd>(e);
    if (!e2)
        return;
    
    ToolBarItem item;
    m_tbSkin->GetItemInfo(e2->iItem, &item);
    DlgInsertXmlElement dlg(pResManger, CSysDataMgr::getSingleton().getSkinDefNode().child(L"skins"), S_CT2W(item.strText));
    if(IDOK == dlg.DoModal(NULL))
    {
        pXmlEditor->InsertElement(dlg.GetXml());
    }
}

SNSEND