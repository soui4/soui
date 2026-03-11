#ifndef _SPROPTYITEM_TEXT_H_
#define _SPROPTYITEM_TEXT_H_

#include "../SPropertyItemBase.h"

SNSBEGIN

class SPropertyItemText : public SPropertyItemBase {
    friend class SPropertyGrid;
    DEF_SOBJECT(SPropertyItemBase, L"proptext")
  public:
    static LPCWSTR GetInplaceItemStyleName();

    virtual PropType GetType() const OVERRIDE
    {
        return PT_TEXT;
    }

    virtual void SetValue(const SStringT &strValue) OVERRIDE;
    virtual SStringT GetValue() const OVERRIDE
    {
        return m_strValue;
    }
    virtual BOOL HasButton() const OVERRIDE;

    virtual void DrawItem(IRenderTarget *pRT, CRect rc) OVERRIDE;
    virtual BOOL HasValue() const OVERRIDE;
    virtual void ClearValue() OVERRIDE;
    virtual BOOL IsAutoComplete() const;
    void OnValueChanged() OVERRIDE;

    SOUI_ATTRS_BEGIN()
        ATTR_STRINGT(L"value", m_strValue, TRUE)
        ATTR_BOOL(L"hasButton", m_hasButton, TRUE)
        ATTR_BOOL(L"isNumber", m_isNumber, TRUE)
        ATTR_BOOL(L"autoComplete", m_bAutoComplete, FALSE)
    SOUI_ATTRS_END()

  protected:
    virtual void OnInplaceActive(BOOL bActive);

  protected:
    SStringT m_strValue;
    BOOL m_hasButton;
    BOOL m_isNumber;
    BOOL m_bAutoComplete;
    SEdit *m_pEdit;

  protected:
    SPropertyItemText(SPropertyGrid *pOwner);
};
SNSEND

#endif //_SPROPTYITEM_TEXT_H_
