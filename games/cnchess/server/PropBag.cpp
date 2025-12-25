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
    m_dwProps[PROPID_REGRET] = 2;
    m_dwProps[PROPID_TIME_STEP] = 60;
    m_dwProps[PROPID_TIME_ROUND] = 0;
    m_dwProps[PROPID_MAX_STP_JIANG] = 10;
    m_dwProps[PROPID_MAX_STP_CATCH] = 10;
    m_dwProps[PROPID_MAX_STP_PEACE] = 100;
    m_dwProps[PROPID_MIN_STEPS] = 10;
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
    m_dwProps[PROPID_REGRET] = xmlNode.attribute(L"regret").as_int(1);	//默认悔棋次数为1次
    m_dwProps[PROPID_TIME_ROUND] = xmlNode.attribute(L"round").as_int(0);	//默认无局时
    m_dwProps[PROPID_TIME_STEP] = xmlNode.attribute(L"step").as_int(0);	//默认无步时
    m_dwProps[PROPID_MAX_STP_JIANG] = xmlNode.attribute(L"max_jiang").as_int(10);	//默认长将步数为10步
    m_dwProps[PROPID_MAX_STP_CATCH] = xmlNode.attribute(L"max_catch").as_int(10);	//默认长捉步数为10步
    m_dwProps[PROPID_MAX_STP_PEACE] = xmlNode.attribute(L"max_peace").as_int(100);	//默认不吃子步数为100步
    m_dwProps[PROPID_MIN_STEPS] = xmlNode.attribute(L"min_steps").as_int(10);	//默认最小步数为10步

    m_wPort = xmlNode.attribute(L"port").as_int(DEF_PORT);     //默认端口号
}
WORD PropBag::GetPort() const
{
    return m_wPort;
}
SNSEND