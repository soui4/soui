#include "souistd.h"
#include "res.mgr/SStylePool.h"

SNSBEGIN

//////////////////////////////////////////////////////////////////////////
// SStylePool

// Get style object from pool by class name
SXmlNode SStylePool::GetStyle(const SStringW &strName)
{
    if (!HasKey(strName))
        return SXmlNode();
    return GetKeyObject(strName);
}

// Load style-pool from xml tree
BOOL SStylePool::Init(SXmlNode xmlStyleRoot)
{
    if (!xmlStyleRoot)
        return FALSE;

    xmlStyleRoot = m_xmlDoc.root().append_copy(xmlStyleRoot);

    for (SXmlNode xmlChild = xmlStyleRoot.first_child(); xmlChild; xmlChild = xmlChild.next_sibling())
    {
        SStringW strClsName = xmlChild.name();
        if (strClsName.CompareNoCase(L"class") == 0)
        {
            strClsName = xmlChild.attribute(L"name").value();
            if (strClsName.IsEmpty())
                continue;
            xmlChild.remove_attribute(L"name"); //删除name属性，防止该属性被处理
        }
        SASSERT(!xmlChild.attribute(L"name"));
        AddKeyObject(strClsName, xmlChild);
    }

    return TRUE;
}

/////////////////////////////////////////////////////////////////////
BOOL STemplatePool::Init(SXmlNode xmlNode)
{
    if (!xmlNode)
        return FALSE;
    for (SXmlNode xmlChild = xmlNode.first_child(); xmlChild; xmlChild = xmlChild.next_sibling())
    {
        SStringW strTempName = xmlChild.name();
        SXmlNode xmlNode = xmlChild.first_child();
        SStringW strValue;
        while (xmlNode)
        {
            SStringW strXml;
            xmlNode.ToString(&strXml);
            strValue += strXml;
            xmlNode = xmlNode.next_sibling();
        }
        AddKeyObject(strTempName, strValue);
    }
    return TRUE;
}

SStringW STemplatePool::GetTemplateString(const SStringW &strName) const
{
    SStringW strRet;
    GetKeyObject(strName, strRet);
    return strRet;
}

SNSEND