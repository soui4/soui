#include "souistd.h"
#include "res.mgr/SGradientPool.h"
#include <core/SGradient.h>

SNSBEGIN

//////////////////////////////////////////////////////////////////////////
// SGradientPool
IGradient *SGradientPool::GetGradient(const SStringW &strName)
{
    if (!HasKey(strName))
        return NULL;
    return GetKeyObject(strName);
}

// Load SGradientPool from xml tree
BOOL SGradientPool::Init(SXmlNode xmlRoot)
{
    if (!xmlRoot)
        return FALSE;
    for (SXmlNode xmlChild = xmlRoot.first_child(); xmlChild; xmlChild = xmlChild.next_sibling())
    {
        SGradient *pGradient = new SGradient();
        pGradient->SetName(xmlChild.name());
        pGradient->InitFromXml(&xmlChild);
        AddKeyObject(xmlChild.name(), pGradient);
        pGradient->Release();
    }

    return TRUE;
}

SNSEND