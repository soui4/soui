#include "stdafx.h"
#include "helper/SAdapterBase.h"
#include "Adapter.h"
#include "PreviewHost.h"

void CBaseMcAdapterFix::getView(int position, SItemPanel * pItem, SXmlNode xmlTemplate)
{
	if (pItem->GetChildrenCount() == 0)
	{
		pItem->InitFromXml(&xmlTemplate);
	}
}

SStringW CBaseMcAdapterFix::GetColumnName(int iCol) const {
	if (iCol >= m_colNames.GetCount())
		iCol = 0;

	return m_colNames[iCol];
}

void CBaseMcAdapterFix::InitColNames(SXmlNode xmlTemplate)
{
	for (xmlTemplate = xmlTemplate.first_child(); xmlTemplate; xmlTemplate = xmlTemplate.next_sibling())
	{
		if (node_element != xmlTemplate.type())
			continue;

		while (xmlTemplate && !xmlTemplate.attribute(L"name"))
		{
			if (node_element != xmlTemplate.type())
			{
				xmlTemplate = xmlTemplate.next_sibling();
				continue;
			}
			xmlTemplate = xmlTemplate.first_child();
		}
		m_colNames.Add(xmlTemplate.attribute(L"name").value());
	}
}

void CBaseMcAdapterFix::InitByTemplate(SXmlNode xmlTemplate)
{
	InitColNames(xmlTemplate);
}


//////////////////////////////////////////////////////////////////////////
void CBaseAdapterFix::getView(int position, SItemPanel * pItem, SXmlNode xmlTemplate)
{
	if (pItem->GetChildrenCount() == 0)
	{
        pItem->InitFromXml(&xmlTemplate);
	}
}



