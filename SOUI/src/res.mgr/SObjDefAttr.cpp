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

    return TRUE;
}

SXmlNode SObjDefAttr::GetDefAttribute(LPCWSTR pszClassName)
{
    SASSERT(pszClassName);
    return GetKeyObject(pszClassName);
}

SNSEND
