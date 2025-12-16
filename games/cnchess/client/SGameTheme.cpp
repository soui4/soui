#include "stdafx.h"
#include "SGameTheme.h"

SNSBEGIN


SGameTheme::SGameTheme()
{
}

SGameTheme::~SGameTheme()
{
}

BOOL SGameTheme::Load(const SStringT & strPath)
{
    //set file prefix, load from file will use this prefix to build file path.
    SApplication::getSingletonPtr()->SetFilePrefix(strPath);
    SXmlDoc xmlDoc;
    if (!SApplication::getSingletonPtr()->LoadXmlDocment(xmlDoc, _T("file:config.xml")))
    {
        return FALSE;
    }
    
    SXmlNode xmlSkin = xmlDoc.root().child(L"theme").child(L"skin");
    int ret = SSkinPool::getSingletonPtr()->LoadSkins(&xmlSkin);

    //load layouts from xml file.
    SXmlNode xmlLayouts = xmlDoc.root().child(L"theme").child(L"layouts");
    SXmlNode xmlLayout = xmlLayouts.first_child();
    while(xmlLayout)
    {
        SStringW clsName = xmlLayout.name();
        SStringW strName = xmlLayout.attribute(L"name").as_string();
        SASSERT(m_mapLayouts.find(strName) == m_mapLayouts.end());
        ILayoutParam *pLayout = SApplication::getSingletonPtr()->CreateLayoutParamByName(clsName);
        if(pLayout)
        {
            pLayout->InitFromXml(&xmlLayout);
            m_mapLayouts[strName] = pLayout;
            pLayout->Release();
        }
        xmlLayout = xmlLayout.next_sibling();
    }

    //load animations from xml file.
    SXmlNode xmlAnims = xmlDoc.root().child(L"theme").child(L"animations");
    SXmlNode xmlAnim = xmlAnims.first_child();
    while(xmlAnim)
    {
        SStringW clsName = xmlAnim.name();
        SStringW strName = xmlAnim.attribute(L"name").as_string();
        SASSERT(m_mapAnims.find(strName) == m_mapAnims.end());
        IAnimation *pAnim = SApplication::getSingletonPtr()->CreateAnimationByName(clsName);
        if(pAnim)
        {
            pAnim->InitFromXml(&xmlAnim);
            m_mapAnims[strName] = pAnim;
            pAnim->Release();
        }
        xmlAnim = xmlAnim.next_sibling();
    }

    //load sprites from xml file.
    SXmlNode xmlSprites = xmlDoc.root().child(L"theme").child(L"sprites");
    SXmlNode xmlSprite = xmlSprites.first_child();
    while(xmlSprite)
    {
        SStringW clsName = xmlSprite.name();
        SStringW strName = xmlSprite.attribute(L"name").as_string();
        SASSERT(m_mapSprites.find(strName) == m_mapSprites.end());
        ISprite *pSprite = SApplication::getSingletonPtr()->CreateSpriteByName(clsName);
        if(pSprite)
        {
            pSprite->InitFromXml(&xmlSprite);
            m_mapSprites[strName] = pSprite;
            pSprite->Release();
        }
        xmlSprite = xmlSprite.next_sibling();
    }

    //load sounds from xml file.
    SXmlNode xmlSounds = xmlDoc.root().child(L"theme").child(L"sounds");
    SXmlNode xmlSound = xmlSounds.first_child();
    while(xmlSound)
    {
        SStringW clsName = xmlSound.name();
        SStringW strName = xmlSound.attribute(L"name").as_string();
        SASSERT(m_mapSounds.find(strName) == m_mapSounds.end());
        IAudioPlayer *pSound = SApplication::getSingletonPtr()->CreateAudioPlayerByName(clsName);
        if(pSound)
        {
            pSound->InitFromXml(&xmlSound);
            m_mapSounds[strName] = pSound;
            pSound->Release();
        }
        xmlSound = xmlSound.next_sibling();
    }

    return TRUE;
}

ISkinObj * SGameTheme::GetSkin(const SStringW & name)
{
    return SSkinPool::getSingletonPtr()->GetSkin(name);
}

IAnimation * SGameTheme::GetAnimation(const SStringW & name)
{
    ANIMATION_MAP::const_iterator it = m_mapAnims.find(name);
    if(it!=m_mapAnims.end())
        return it->second;
    return NULL;
}

ILayoutParam * SGameTheme::GetLayout(const SStringW & name)
{
    LAYOUT_MAP::const_iterator it = m_mapLayouts.find(name);
    if(it!=m_mapLayouts.end())
        return it->second;
    return NULL;
}

ISprite * SGameTheme::GetSprite(const SStringW & name)
{
    SPRITE_MAP::const_iterator it = m_mapSprites.find(name);
    if(it!=m_mapSprites.end())
        return it->second;
    return NULL;
}

IAudioPlayer * SGameTheme::GetSound(const SStringW & name)
{
    SOUND_MAP::const_iterator it = m_mapSounds.find(name);
    if(it!=m_mapSounds.end())
        return it->second;
    return NULL;
}

SNSEND