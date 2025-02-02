/********************************************************************
    created:	2014/11/03
    created:	3:11:2014   16:13
    filename: 	SIPAddressCtrl.h
    author:		冰

    purpose:	SOUI版的IP控件
*********************************************************************/
#pragma once

namespace SOUI
{

class SEditIP;

class SIPAddressCtrl : public SWindow {
    DEF_SOBJECT(SWindow, L"ipctrl")
  public:
    SIPAddressCtrl(void);
    ~SIPAddressCtrl(void);

    BOOL IsBlank() const;
    void ClearAddress();
    int GetAddress(BYTE &nField0, BYTE &nField1, BYTE &nField2, BYTE &nField3) const;
    int GetAddress(DWORD &dwAddress) const;

    void SetAddress(DWORD dwAddress);
    void SetAddress(BYTE nField0, BYTE nField1, BYTE nField2, BYTE nField3);

    void SetFieldFocus(WORD nField);
  protected:
    void OnPaint(IRenderTarget *pRT);
    LRESULT OnCreate(LPVOID);
    void OnSize(UINT nType, CSize size);

    SOUI_MSG_MAP_BEGIN()
    MSG_WM_CREATE(OnCreate)
    MSG_WM_SIZE(OnSize)
    MSG_WM_PAINT_EX(OnPaint)
    SOUI_MSG_MAP_END()

    HRESULT OnAttrField(int iField,const SStringW &strValue, BOOL bLoading);

    HRESULT OnAttrField0(const SStringW &strValue, BOOL bLoading);
    HRESULT OnAttrField1(const SStringW &strValue, BOOL bLoading);
    HRESULT OnAttrField2(const SStringW &strValue, BOOL bLoading);
    HRESULT OnAttrField3(const SStringW &strValue, BOOL bLoading);

    HRESULT OnAttrIP(const SStringW &strValue, BOOL bLoading);

    SOUI_ATTRS_BEGIN()
        ATTR_CUSTOM(L"field0", OnAttrField0)
        ATTR_CUSTOM(L"field1", OnAttrField1)
        ATTR_CUSTOM(L"field2", OnAttrField2)
        ATTR_CUSTOM(L"field3", OnAttrField3)
        ATTR_CUSTOM(L"ip", OnAttrIP)
    SOUI_ATTRS_END()
  private:
    SEditIP *m_editFields[4];
    BYTE m_ipFields[4];
};
} // namespace SOUI
