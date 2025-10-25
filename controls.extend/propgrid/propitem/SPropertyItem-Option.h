#ifndef _SPROPTYITEM_OPTION_H_
#define _SPROPTYITEM_OPTION_H_

#include "../SPropertyItemBase.h"
#include <souicoll.h>

SNSBEGIN

    class SPropertyItemOption : public SPropertyItemBase
    {
		friend class SPropertyGrid;
        DEF_SOBJECT(SPropertyItemBase,L"propoption")
    public:
		static LPCWSTR GetInplaceItemStyleName();
        
		virtual PropType GetType() const {return PT_OPTION;}
        virtual void SetValue(const SStringT & strValue);

        virtual SStringT GetValue() const;

		virtual BOOL HasValue() const ;
		virtual void ClearValue() ;

        SStringT Option2Value(const SStringT& value) const;

		SStringT Value2Option(const SStringT& value) const;

        void ClearOptions();
        void AddOption(const SStringT & strOption);
        int GetOptionCount() const;
        SStringT GetOption(int iIndex) const;

    public:
        SOUI_ATTRS_BEGIN()
            ATTR_CUSTOM(L"options",OnAttrOptions)
			ATTR_CUSTOM(L"value",OnAttrValue)
            ATTR_INT(L"dropHeight",m_nDropHeight,FALSE)
			ATTR_BOOL(L"dropDown",m_bDropDown,FALSE)
        SOUI_ATTRS_END()

    protected:
		virtual void DrawItem(IRenderTarget *pRT,CRect rc);
        virtual void OnInplaceActive(BOOL bActive);
    protected:
        HRESULT OnAttrOptions(const SStringW & strValue,BOOL bLoading);
		HRESULT OnAttrValue(const SStringW & strValue,BOOL bLoading);


        int      m_nDropHeight;
		bool	 m_bDropDown;
        SStringT m_strValue;

        SArray<SStringT>    m_options;//value options   
		SMap<SStringT, SStringT> m_option2value;//text to option map.

        SComboBox  * m_pCombobox;
        
    public:
		BOOL OnCBSelChange(IEvtArgs *pEvt);
    protected:
        SPropertyItemOption(SPropertyGrid *pOwner);
    };
SNSEND
#endif//_SPROPTYITEM_OPTION_H_
