#include "stdafx.h"
#include "SGameTheme.h"

SNSBEGIN


SGameTheme::SGameTheme()
{
    m_pSkinPool.Attach(SUiDef::CreateSkinPool(TRUE));
    GETUIDEF->PushSkinPool(m_pSkinPool);
}

SGameTheme::~SGameTheme()
{
    GETUIDEF->PopSkinPool(m_pSkinPool);
}

BOOL SGameTheme::Load(LPCTSTR pszThemeDir)
{
    //set file prefix, load from file will use this prefix to build file path.
    SApplication::getSingletonPtr()->SetFilePrefix(pszThemeDir);
    SXmlDoc xmlDoc;
    if (!SApplication::getSingletonPtr()->LoadXmlDocment(xmlDoc, _T("file:config.xml")))
    {
        return FALSE;
    }
    SXmlNode xmlSkin = xmlDoc.root().child(L"theme").child(L"skin");
    int ret = m_pSkinPool->LoadSkins(&xmlSkin);

    //load widgets from xml file.
    SXmlNode xmlWidgets = xmlDoc.root().child(L"theme").child(L"widgets");
    SXmlNode xmlWidget = xmlWidgets.first_child();
    while(xmlWidget)
    {
        SStringW clsName = xmlWidget.name();
        SStringW strName = xmlWidget.attribute(L"name").as_string();
        SASSERT(m_mapWidgets.find(strName) == m_mapWidgets.end());
        IWindow *pWidget = SApplication::getSingletonPtr()->CreateWindowByName(clsName);
        if(pWidget)
        {
            pWidget->InitFromXml(&xmlWidget);
            m_mapWidgets[strName] = pWidget;
            pWidget->Release();
        }
        xmlWidget = xmlWidget.next_sibling();
    }

    //load anchors from xml file.
    SASSERT(AnchorName::kCount == ARRAYSIZE(AnchorName::kElementNames));
    SXmlNode xmlAnchors = xmlDoc.root().child(L"theme").child(L"anchors");
    for(int i=0;i<ARRAYSIZE(AnchorName::kElementNames);i++)
    {
        SStringW strName = AnchorName::kElementNames[i];
        AnchorPos pos;
        SXmlNode xmlEle = xmlAnchors.child(strName);
        if(xmlEle){
            SAnchorLayoutParam::ParsePosition(xmlEle.attribute(L"pos").value(), pos);
            pos.fOffsetX = xmlEle.attribute(L"offsetX").as_float();
            pos.fOffsetY = xmlEle.attribute(L"offsetY").as_float();
        }
        m_mapLayout[strName] = pos;
    }

    //load animation
    SXmlNode xmlAnim = xmlDoc.root().child(L"theme").child(L"anims");
    SXmlNode xmlAni = xmlAnim.first_child();
    while(xmlAni)
    {
        SStringW strName = xmlAni.attribute(L"name").as_string();
        SStringT strSrc = S_CW2T(xmlAni.attribute(L"src").as_string());
        SASSERT(m_mapAnimations.find(strName) == m_mapAnimations.end());
        SXmlDoc xml;
        if (SApplication::getSingletonPtr()->LoadXmlDocment(xml, strSrc)){
            SAutoRefPtr<IAnimation> pAni(SApplication::getSingletonPtr()->CreateAnimationByName(xml.root().first_child().name()),false);
            if (pAni){
                SXmlNode xmlNode = xml.root().first_child();
                pAni->InitFromXml(&xmlNode);
                m_mapAnimations[strName] = pAni;
            }
        }
        xmlAni = xmlAni.next_sibling();
    }

    //copy template
    SXmlNode xmlTemplate = xmlDoc.root().child(L"theme").child(L"template");
    SASSERT(xmlTemplate);
    m_docTemplate.root().append_copy(xmlTemplate);

    SXmlNode xmlSounds = xmlDoc.root().child(L"theme").child(L"sounds");
    SASSERT(xmlSounds);
    m_docSounds.root().append_copy(xmlSounds);

    return ret > 0 ;
}

IWindow *SGameTheme::GetWidget(LPCWSTR pszName)
{
    std::map<SStringW, SAutoRefPtr<IWindow> >::iterator it = m_mapWidgets.find(pszName);
    if(it != m_mapWidgets.end())
    {
        return it->second;
    }   
    return nullptr;
}

SXmlNode SGameTheme::GetTemplate(LPCWSTR pszName)
{
    return m_docTemplate.root().first_child().child(pszName);
}

IAnimation *SGameTheme::GetAnimation(LPCWSTR pszName)
{
    std::map<SStringW, SAutoRefPtr<IAnimation> >::iterator it = m_mapAnimations.find(pszName);
    if(it != m_mapAnimations.end())
    {
        return it->second->clone(); // as animation change its state, we need clone it
    }   
    return nullptr;
}


SStringW SGameTheme::GetEffectSoundFile(LPCWSTR pszName)
{
    SXmlNode xmlEffect = m_docSounds.root().first_child().child(L"effects").first_child();
    while (xmlEffect)
    {
        SStringW name = xmlEffect.attribute(L"name").as_string();
        if (name == pszName)
        {
            // 找到匹配的效果音效，获取src属性
            SStringW src = xmlEffect.attribute(L"src").as_string();
            if (!src.IsEmpty())
            {
                return src;
            }
            break;
        }
        xmlEffect = xmlEffect.next_sibling();
    }
    return L"";
}

SNSEND