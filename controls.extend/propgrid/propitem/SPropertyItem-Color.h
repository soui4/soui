#ifndef _SPROPTYITEM_COLOR_H_
#define _SPROPTYITEM_COLOR_H_

#include "SPropertyItem-Text.h"
#include "../colorpicker/SColorPicker.h"
#include "../colorpicker/ColourPopup.h"

SNSBEGIN

    class SPropertyItemColor : public SPropertyItemText,public IColorPicker
    {
		friend class SPropertyGrid;
        friend class SPropColorEdit;
        DEF_SOBJECT(SPropertyItemText,L"propcolor")
    public:
		static LPCWSTR GetInplaceItemStyleName(){return NULL;}

		virtual PropType GetType() const {return PT_COLOR;}
        virtual BOOL HasButton() const {return TRUE;}
        virtual void DrawItem(IRenderTarget *pRT,CRect rc);
        virtual void AdjustInplaceActiveWndRect(CRect & rc);
        
        virtual void SetValue(const SStringT & strValue);
        virtual SStringT GetValue() const;
		COLORREF GetValue2() const{return m_crValue;}
		virtual BOOL HasValue() const ;
		virtual void ClearValue() ;


        SOUI_ATTRS_BEGIN()
            ATTR_STRINGT(L"format",m_strFormat,TRUE)
            ATTR_COLOR(L"value",m_crValue,TRUE)
        SOUI_ATTRS_END()

    protected:
        virtual void OnInplaceActive(BOOL bActive);
        virtual BOOL OnButtonClick();
    protected:
        SStringT m_strFormat;
        COLORREF    m_crValue;
        
    protected:
        SPropertyItemColor(SPropertyGrid *pOwner);

	protected://IColorPicker
		virtual void OnColorChanged(COLORREF cr);
		virtual void OnColorEnd(BOOL bCancel,COLORREF cr);
		virtual IMessageLoop * GetMsgLoop();
    };
SNSEND

#endif//_SPROPTYITEM_COLOR_H_
