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
int CBaseAdapterFix::IniTemplateNames(SXmlNode xmlTemplate)
{
	for (xmlTemplate = xmlTemplate.first_child(); xmlTemplate; xmlTemplate = xmlTemplate.next_sibling())
	{
		if (node_element == xmlTemplate.type())
			m_TemplateNames.Add(xmlTemplate.name());
	}
	return m_TemplateNames.GetCount();
}

void CBaseAdapterFix::InitByTemplate(SXmlNode xmlTemplate)
{
	if (IniTemplateNames(xmlTemplate) > 0)
	{
		//此处名字是自定义的，view的灵活性也就体现在这些地方。
		m_nItemHeight[0] = xmlTemplate.attribute(KAttrName_Height[0]).as_int(50);
		m_nItemHeight[1] = xmlTemplate.attribute(KAttrName_Height[1]).as_int(60);
		m_nItemHeight[2] = xmlTemplate.attribute(KAttrName_Height[2]).as_int(70);
	}
}

int CBaseAdapterFix::getViewTypeCount() 
{ 
	return m_TemplateNames.GetCount() == 0 ? 1 : m_TemplateNames.GetCount(); 
}

int CBaseAdapterFix::getItemViewType(int position, DWORD dwState)
{
	if (m_TemplateNames.GetCount() > 1)
	{
		if (position % 2 == 0)
			return 0;//1,3,5,... odd lines
		else if (dwState & WndState_Hover)
			return 2;//even lines with check state
		else
			return 1;//even lines 
	}
	return SAdapterBase::getItemViewType(position, dwState);
}

SIZE CBaseAdapterFix::getViewDesiredSize(int position, SItemPanel *pItem, int wid,int hei)
{
	DWORD dwState = pItem->GetState();
	int viewType = getItemViewType(position, dwState);
	return CSize(0, m_nItemHeight[viewType]);//cx在listview，mclistview中没有使用，不需要计算
}

void CBaseAdapterFix::getView(int position, SItemPanel * pItem, SXmlNode xmlTemplate)
{
	if (pItem->GetChildrenCount() == 0)
	{
		if (m_TemplateNames.GetCount() == 0)
			pItem->InitFromXml(&xmlTemplate);
		else
		{
			int nViewType = getItemViewType(position, pItem->GetState());
			SXmlNode xmlNode = xmlTemplate.child(m_TemplateNames[nViewType < m_TemplateNames.GetCount() - 1 ? nViewType : m_TemplateNames.GetCount() - 1]);
			pItem->InitFromXml(&xmlNode);
		}
	}
}



