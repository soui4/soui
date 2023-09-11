#include "souistd.h"
#include "res.mgr/SSkinPool.h"
#include "core/Sskin.h"
#include "SApp.h"
#include "helper/SDpiScale.h"

SNSBEGIN

//////////////////////////////////////////////////////////////////////////
// SSkinPool

SSkinPool::SSkinPool()
{
    m_pFunOnKeyRemoved = OnKeyRemoved;
}

SSkinPool::~SSkinPool()
{
#ifdef _DEBUG
    //查询哪些皮肤运行过程中没有使用过,将结果用输出到Output
    SSLOGD() << "####Detecting Defined Skin Usage BEGIN";
    SPOSITION pos = m_mapNamedObj->GetStartPosition();
    while (pos)
    {
        SkinKey skinKey = m_mapNamedObj->GetNextKey(pos);
        if (!m_mapSkinUseCount.Lookup(skinKey))
        {
            SSLOGFMTD(L"skin of [%s.%d] was not used.", (LPCWSTR)skinKey.strName, skinKey.scale);
        }
    }
    SSLOGD() << "!!!!Detecting Defined Skin Usage END";
#endif
}

ISkinObj *SSkinPool::_LoadSkin(SXmlNode xmlSkin, int nScale)
{
    SStringW strTypeName = xmlSkin.name();
    SStringW strSkinName = xmlSkin.attribute(L"name").value();

    if (strSkinName.IsEmpty() || strTypeName.IsEmpty())
    {
        return NULL;
    }

    ISkinObj *pSkin = SApplication::getSingleton().CreateSkinByName(strTypeName);
    if (pSkin)
    {
        pSkin->InitFromXml(&xmlSkin);
        if (nScale != -1 && pSkin->GetScale() == 100)
        {
            pSkin->SetScale(nScale);
        }
        SkinKey key = { strSkinName, pSkin->GetScale() };
        SASSERT(!HasKey(key));
        AddKeyObject(key, pSkin);
    }
    else
    {
        SASSERT_FMTW(FALSE, L"load skin error,type=%s,name=%s", strTypeName.c_str(), strSkinName.c_str());
    }
    return pSkin;
}

int SSkinPool::_LoadSkins(SXmlNode xmlNode)
{
    int nLoaded = 0;
    SStringW strSkinName, strTypeName;

    int scale = xmlNode.attribute(L"scale").as_int(-1);
    SXmlNode xmlSkin = xmlNode.first_child();
    while (xmlSkin)
    {
        SStringW strName = xmlSkin.name();
        if (strName.CompareNoCase(L"skin") == 0)
        {
            nLoaded += _LoadSkins(xmlSkin);
        }
        else
        {
            ISkinObj *pSkin = _LoadSkin(xmlSkin, scale);
            if (pSkin)
                nLoaded++;
        }
        xmlSkin = xmlSkin.next_sibling();
    }
    return nLoaded;
}

int SSkinPool::LoadSkins(IXmlNode *xmlNode)
{
    if (!xmlNode)
        return 0;
    // loadSkins前把this加入到poolmgr,便于在skin中引用其它skin
    GETUIDEF->PushSkinPool(this);
    int nLoaded = _LoadSkins(SXmlNode(xmlNode));
    GETUIDEF->PopSkinPool(this);
    return nLoaded;
}

BOOL SSkinPool::AddSkin(ISkinObj *pSkin)
{
    SkinKey key = { pSkin->GetName(), pSkin->GetScale() };
    if (HasKey(key))
        return FALSE;
    AddKeyObject(key, pSkin);
    pSkin->AddRef();
    return TRUE;
}

BOOL SSkinPool::RemoveSkin(THIS_ ISkinObj *pSkin)
{
    SkinKey key = { pSkin->GetName(), pSkin->GetScale() };
    return !!RemoveKeyObject(key);
}

ISkinObj *SSkinPool::GetSkin(LPCWSTR strSkinName, int nScale)
{
    SkinKey key = { strSkinName, nScale };

    if (!HasKey(key))
    {
        nScale = SDpiScale::NormalizeScale(nScale);
        key.scale = nScale;
        if (!HasKey(key))
        {
            bool bFind = false;
            for (int i = 0; i < SDpiScale::GetBuiltinScaleCount(); i++)
            {
                key.scale = SDpiScale::GetBuiltinScales()[i];
                bFind = HasKey(key);
                if (bFind)
                    break;
            }
            if (!bFind)
                return NULL;

            ISkinObj *pSkinSrc = GetKeyObject(key);
            ISkinObj *pSkin = pSkinSrc->Scale(nScale);
            if (pSkin)
            {
                key.scale = nScale;
                AddKeyObject(key, pSkin);
            }
        }
    }
#ifdef _DEBUG
    m_mapSkinUseCount[key]++;
#endif
    return GetKeyObject(key);
}

void SSkinPool::OnKeyRemoved(const SSkinPtr &obj)
{
    obj->Release();
}

void SSkinPool::RemoveAll(THIS)
{
    SCmnMap<SSkinPtr, SkinKey>::RemoveAll();
}

SNSEND