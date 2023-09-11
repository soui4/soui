#include "souistd.h"
#include "res.mgr/SObjDefAttr.h"
#include "SApp.h"

SNSBEGIN

BOOL SObjDefAttr::Init(SXmlNode xmlNode)
{
    if (!xmlNode)
        return FALSE;
    // clear old data
    RemoveAll();
    m_xmlRoot.root().RemoveAllChilden();

    m_xmlRoot.root().append_copy(xmlNode);

    SXmlNode xmlObjAttr = m_xmlRoot.root().child(L"objattr").first_child();
    while (xmlObjAttr)
    {
        AddKeyObject(xmlObjAttr.name(), xmlObjAttr);
        xmlObjAttr = xmlObjAttr.next_sibling();
    }

    SPOSITION pos = m_mapNamedObj->GetStartPosition();
    while (pos)
    {
        SMap<SStringW, SXmlNode>::CPair *p = m_mapNamedObj->GetNext(pos);
        BuildClassAttribute(p->m_value, p->m_key);
    }

    return TRUE;
}

void SObjDefAttr::BuildClassAttribute(SXmlNode &xmlNode, LPCWSTR pszClassName)
{
    SObjectInfo baseClassInfo = SApplication::getSingleton().BaseObjectInfoFromObjectInfo(ObjInfo_New(pszClassName, Window));
    if (!ObjInfo_IsValid(&baseClassInfo))
        return;

    if (HasKey(baseClassInfo.szName))
    {
        SXmlNode xmlNodeAttrs = GetKeyObject(baseClassInfo.szName);
        SXmlAttr attr = xmlNodeAttrs.first_attribute();
        while (attr)
        {
            if (!xmlNode.attribute(attr.name()))
                xmlNode.append_attribute(attr.name()).set_value(attr.value());
            attr = attr.next_attribute();
        }
    }
    BuildClassAttribute(xmlNode, baseClassInfo.szName);
}

SXmlNode SObjDefAttr::GetDefAttribute(LPCWSTR pszClassName)
{
    SASSERT(pszClassName);
    if (!HasKey(pszClassName))
    {
        SObjectInfo baseClassInfo = SApplication::getSingleton().BaseObjectInfoFromObjectInfo(ObjInfo_New(pszClassName, Window));
        if (!ObjInfo_IsValid(&baseClassInfo))
            return SXmlNode();

        return GetDefAttribute(baseClassInfo.szName);
    }
    else
    {
        return GetKeyObject(pszClassName);
    }
}

SNSEND
