#include "stdafx.h"
#include "PropBag.h"
#include <xml/SXml.h>

SNSBEGIN

template<>
PropBag * SSingleton<PropBag>::ms_Singleton = NULL;

PropBag::PropBag()
{
    m_wPort=DEF_PORT;	//默认端口号

	memset(m_dwProps,0,sizeof(m_dwProps));
    m_dwProps[PROPID_BOMBFLY] = 0;
    m_dwProps[PROPID_MINEASBOMB] = 0;
    m_dwProps[PROPID_PEACESTEPS] = 10;
    m_dwProps[PROPID_RULE] = RULE_SMS;  //默认双明
    m_dwProps[PROPID_STEPTICK] = 60;
    m_dwProps[PROPID_PASS] = 3;
    m_dwProps[PROPID_LAYOUTTICK] = 60;
    m_dwProps[PROPID_GETFLASH] = 1;
    m_dwProps[PROPID_AUTOSAVE] = 1;
}

PropBag::~PropBag()
{

}

DWORD PropBag::GetProp(DWORD dwProp) const
{
	return m_dwProps[dwProp];
}

void PropBag::Init(LPCTSTR pszPropXml)
{
    if (!pszPropXml)
        return;
	SXmlDoc xmlDoc;
    if (!xmlDoc.load_file(pszPropXml))
    {
        return;
    }
    SXmlNode xmlNode = xmlDoc.root().child(L"prop");
    m_dwProps[PROPID_BOMBFLY] = xmlNode.attribute(L"bombfly").as_int(0);
    m_dwProps[PROPID_MINEASBOMB] = xmlNode.attribute(L"mineasbomb").as_int(0);
    m_dwProps[PROPID_PEACESTEPS] = xmlNode.attribute(L"peacesteps").as_int(10);
    m_dwProps[PROPID_RULE] = xmlNode.attribute(L"rule").as_int(0);
    m_dwProps[PROPID_STEPTICK] = xmlNode.attribute(L"steptick").as_int(20);
    m_dwProps[PROPID_PASS] = xmlNode.attribute(L"pass").as_int(3);
    m_dwProps[PROPID_LAYOUTTICK] = xmlNode.attribute(L"layouttick").as_int(60);
    m_dwProps[PROPID_GETFLASH] = xmlNode.attribute(L"getflash").as_int(1);
    m_dwProps[PROPID_AUTOSAVE] = xmlNode.attribute(L"autosave").as_int(0);

    m_wPort = xmlNode.attribute(L"port").as_int(DEF_PORT);     //默认端口号
}
WORD PropBag::GetPort() const
{
    return m_wPort;
}
SNSEND