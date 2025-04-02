#ifndef _SPROPTYITEM_GROUP_H_
#define _SPROPTYITEM_GROUP_H_

#include "../SPropertyItemBase.h"


SNSBEGIN

class SPropertyGroup : public SPropertyItemBase
{
	friend class SPropertyGrid;
	DEF_SOBJECT(SPropertyItemBase,L"propgroup")
public:
	static LPCWSTR GetInplaceItemStyleName(){return NULL;}
	virtual PropType GetType() const {return PT_GROUP;}
	virtual void DrawItem(IRenderTarget *pRT,CRect rc);
	virtual BOOL HasValue() const ;
	virtual void ClearValue() ;
	virtual void OnChildValueChanged( IPropertyItem *pChild );
protected:
	SPropertyGroup(SPropertyGrid *pOwner);
};

SNSEND
#endif//_SPROPTYITEM_GROUP_H_