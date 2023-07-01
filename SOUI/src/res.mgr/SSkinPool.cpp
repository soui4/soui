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

int SSkinPool::LoadSkins(SXmlNode xmlNode)
{
    if (!xmlNode)
        return 0;

    int nLoaded = 0;
    SStringW strSkinName, strTypeName;

    // loadSkins前把this加入到poolmgr,便于在skin中引用其它skin
    GETUIDEF->PushSkinPool(this);

    SXmlNode xmlSkin = xmlNode.first_child();
    while (xmlSkin)
    {
        strTypeName = xmlSkin.name();
        strSkinName = xmlSkin.attribute(L"name").value();

        if (strSkinName.IsEmpty() || strTypeName.IsEmpty())
        {
            xmlSkin = xmlSkin.next_sibling();
            continue;
        }

        ISkinObj *pSkin = SApplication::getSingleton().CreateSkinByName(strTypeName);
        if (pSkin)
        {
            pSkin->InitFromXml(&xmlSkin);
            SkinKey key = { strSkinName, pSkin->GetScale() };
            SASSERT(!HasKey(key));
            AddKeyObject(key, pSkin);
            nLoaded++;
        }
        else
        {
            SASSERT_FMTW(FALSE, L"load skin error,type=%s,name=%s", strTypeName.c_str(), strSkinName.c_str());
        }
        xmlSkin = xmlSkin.next_sibling();
    }

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

ISkinObj *SSkinPool::GetSkin(const SStringW &strSkinName, int nScale)
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

SNSEND