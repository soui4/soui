#ifndef __ADAPTER_H__
#define __ADAPTER_H__


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
public:

	CBaseAdapterFix()
	{
	}

	~CBaseAdapterFix()
	{	
	}

	virtual int WINAPI getCount() override
	{
		return DEFAULT_LINE;
	}
	
	virtual void WINAPI getView(int position, SItemPanel * pItem, SXmlNode xmlTemplate);
};

class  SEmptyTreeAdapter : public STreeAdapterBase<void*> {
public:
    SEmptyTreeAdapter() {
    }

    ~SEmptyTreeAdapter() {
    }

public:
    STDMETHOD_(void, getView)(THIS_ HSTREEITEM hItem, SItemPanel *pItem, SXmlNode xmlTemplate) OVERRIDE {
        if (pItem->GetChildrenCount() == 0) {
            pItem->InitFromXml(&xmlTemplate);
        }
    }
};
#endif//__ADAPTER_H__