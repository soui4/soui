#pragma once
#include "helper/SAdapterBase.h"


#define DEFAULT_LINE 10
class CBaseMcAdapterFix : public SMcAdapterBase
{
	SArray<SStringW> m_colNames;
public:
	CBaseMcAdapterFix()
	{		
	}

	int WINAPI getCount() override
	{
		return DEFAULT_LINE;
	}

	void WINAPI getView(int position, SItemPanel * pItem, SXmlNode xmlTemplate) override;

	SStringW WINAPI GetColumnName(int iCol) const override;

	void WINAPI InitByTemplate(SXmlNode xmlTemplate) override;

	void InitColNames(SXmlNode xmlTemplate);
};


class CBaseAdapterFix : public SAdapterBase
{	
	const wchar_t*  KAttrName_Height[3];
	SArray<SStringW> m_TemplateNames;
	int m_nItemHeight[3];
public:

	CBaseAdapterFix()
	{
		KAttrName_Height[0] = L"oddHeight";
		KAttrName_Height[1] = L"evenHeight";
		KAttrName_Height[2] = L"evenSelHeight";
	}

	~CBaseAdapterFix()
	{	
	}

	virtual int WINAPI getCount() override
	{
		return DEFAULT_LINE;
	}
	
	int IniTemplateNames(SXmlNode xmlTemplate);

	virtual void WINAPI InitByTemplate(SXmlNode xmlTemplate);

	virtual int WINAPI getViewTypeCount();

	virtual int WINAPI getItemViewType(int position, DWORD dwState);

	virtual SIZE WINAPI getViewDesiredSize(int position, SItemPanel *pItem, int wid,int hei);

	virtual void WINAPI getView(int position, SItemPanel * pItem, SXmlNode xmlTemplate);
};


