﻿#ifndef _SPROPTYITEM_TEXT_H_
#define _SPROPTYITEM_TEXT_H_


#include "../SPropertyItemBase.h"

SNSBEGIN

    class SPropertyItemText : public SPropertyItemBase
    {
		friend class SPropertyGrid;
        DEF_SOBJECT(SPropertyItemBase,L"proptext")
    public:
		static LPCWSTR GetInplaceItemStyleName();

		virtual PropType GetType() const {return PT_TEXT;}
        
        virtual void SetValue(const SStringT & strValue);
        virtual SStringT GetValue() const {return m_strValue;}
		virtual BOOL HasButton() const;

		virtual void DrawItem(IRenderTarget *pRT,CRect rc);
		virtual BOOL HasValue() const ;
		virtual void ClearValue() ;
        virtual BOOL IsAutoComplete() const;
        void OnValueChanged() override;

        SOUI_ATTRS_BEGIN()
            ATTR_STRINGT(L"value",m_strValue,TRUE)
			ATTR_BOOL(L"hasButton",m_hasButton,TRUE)
            ATTR_BOOL(L"autoComplete",m_bAutoComplete,FALSE)
        SOUI_ATTRS_END()

    protected:
        virtual void OnInplaceActive(BOOL bActive);
    protected:
        SStringT m_strValue;
        BOOL	m_hasButton;
        BOOL    m_bAutoComplete;
        SEdit  * m_pEdit;
    protected:
        SPropertyItemText(SPropertyGrid *pOwner);
    };
SNSEND

#endif//_SPROPTYITEM_TEXT_H_
