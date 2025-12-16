#include "stdafx.h"
#include "PropBag.h"
#include <xml/SXml.h>

SNSBEGIN

template<>
PropBag * SSingleton<PropBag>::ms_Singleton = NULL;

PropBag::PropBag()
{
    m_wPort=3310;	//默认端口号
    
	memset(m_dwProps,0,sizeof(m_dwProps));
	m_dwProps[PI_TIME_REPORT]=10;
	m_dwProps[PI_TIME_PUTBOTTOM]=60;
	m_dwProps[PI_TIME_REBEL]=10;
	m_dwProps[PI_TIME_PUTCARD]=20;
	m_dwProps[PI_RULE_REBEL]=1;
	m_dwProps[PI_RULE_LEVEL_5_10_K]=1;	//只打5，10，K
	m_dwProps[PI_RULE_2_MAIN]=1;
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
    m_dwProps[PI_TIME_REPORT] = xmlNode.attribute(L"report").as_int(10);
    m_dwProps[PI_TIME_PUTBOTTOM] = xmlNode.attribute(L"putbottom").as_int(60);
    m_dwProps[PI_TIME_REBEL] = xmlNode.attribute(L"rebel").as_int(10);
    m_dwProps[PI_TIME_PUTCARD] = xmlNode.attribute(L"putcard").as_int(20);
    m_dwProps[PI_RULE_REBEL] = xmlNode.attribute(L"rebel").as_int(1);
    m_dwProps[PI_RULE_LEVEL_5_10_K] = xmlNode.attribute(L"510k").as_int(0);
    m_dwProps[PI_RULE_2_MAIN] = xmlNode.attribute(L"2main").as_int(1);
    m_wPort = xmlNode.attribute(L"port").as_int(3310);     //默认端口号
}
WORD PropBag::GetPort() const
{
    return m_wPort;
}
SNSEND