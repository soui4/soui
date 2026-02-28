#ifndef __SFRAMELAYOUT__H__
#define __SFRAMELAYOUT__H__

#include <interface/slayout-i.h>
#include <layout/SFrameLayoutParamStruct.h>
#include <layout/SLinearLayout.h>
#include <sobject/Sobject.hpp>

SNSBEGIN

namespace FrameLayout_Dock{
  static const LPCWSTR kDock_None = L"none";
  static const LPCWSTR kDock_Left = L"left";
  static const LPCWSTR kDock_Top = L"top";
  static const LPCWSTR kDock_Right = L"right";
  static const LPCWSTR kDock_Bottom = L"bottom";
  static const LPCWSTR kDock_MainView = L"mainview";
  static const LPCWSTR kDock_Main = L"main";
}

namespace FrameLayout_DockMode{
  static const LPCWSTR kDockMode_None = L"none";
  static const LPCWSTR kDockMode_Left = L"left";
  static const LPCWSTR kDockMode_Top = L"top";
  static const LPCWSTR kDockMode_Right = L"right";
  static const LPCWSTR kDockMode_Bottom = L"bottom";
  static const LPCWSTR kDockMode_All = L"all";
}

class SOUI_EXP SFrameLayoutParam
    : public TObjRefImpl<SObjectImpl<ILayoutParam>>
    , public SFrameLayoutParamStruct {
    DEF_SOBJECT(SObjectImpl<ILayoutParam>, L"FrameLayoutParam")

    friend class SFrameLayout;

  public:
    static DockPosition parseDockPosition(const SStringW &strValue);
  public:
    SFrameLayoutParam();

    STDMETHOD_(void, Clear)(THIS) OVERRIDE;
    STDMETHOD_(BOOL, IsMatchParent)(THIS_ ORIENTATION orientation) SCONST OVERRIDE;
    STDMETHOD_(BOOL, IsWrapContent)(THIS_ ORIENTATION orientation) SCONST OVERRIDE;
    STDMETHOD_(BOOL, IsSpecifiedSize)(THIS_ ORIENTATION orientation) SCONST OVERRIDE;
    STDMETHOD_(BOOL, GetSpecifiedSize)(THIS_ ORIENTATION orientation, LAYOUTSIZE *pLayoutSize) SCONST OVERRIDE;
    STDMETHOD_(void, SetMatchParent)(THIS_ ORIENTATION orientation) OVERRIDE;
    STDMETHOD_(void, SetWrapContent)(THIS_ ORIENTATION orientation) OVERRIDE;
    STDMETHOD_(void, SetSpecifiedSize)(THIS_ ORIENTATION orientation, const LAYOUTSIZE *pLayoutSize) OVERRIDE;
    STDMETHOD_(void *, GetRawData)(THIS) OVERRIDE;
    STDMETHOD_(ILayoutParam *, Clone)(THIS) SCONST OVERRIDE;
    STDMETHOD_(BOOL, SetAnimatorValue)(THIS_ IPropertyValuesHolder *pHolder, float fraction, ANI_STATE state) OVERRIDE;

  public:
    SOUI_ATTRS_BEGIN()
        ATTR_LAYOUTSIZE(L"width", width,TRUE)
        ATTR_LAYOUTSIZE(L"height", height,TRUE)
        ATTR_CUSTOM(L"size", OnAttrSize)
        ATTR_CUSTOM(L"dock", OnAttrDockPosition)
        ATTR_STRINGW(L"dockRelativeTo", strDockRelativeTo, FALSE)
        ATTR_FLOAT(L"weight", weight, FALSE)
        ATTR_GRAVITY(L"layout_gravity", gravity, FALSE)  
        ATTR_LAYOUTSIZE(L"extend_left", extend_left, FALSE)
        ATTR_LAYOUTSIZE(L"extend_top", extend_top, FALSE)
        ATTR_LAYOUTSIZE(L"extend_right", extend_right, FALSE)
        ATTR_LAYOUTSIZE(L"extend_bottom", extend_bottom, FALSE)
    SOUI_ATTRS_BREAK()

  protected:
    HRESULT OnAttrSize(const SStringW &strValue, BOOL bLoading);
    HRESULT OnAttrDockPosition(const SStringW &strValue, BOOL bLoading);
};

class SOUI_EXP SFrameLayout : public TObjRefImpl<SObjectImpl<ILayout>> {
    DEF_SOBJECT_EX(SObjectImpl<ILayout>, L"frameLayout",L"frame")

  public:
    SFrameLayout(void);
    ~SFrameLayout(void);
    
    void SetEnableDockMode(DockMode mode);
    DockMode GetEnableDockMode() const;
  protected:
    static DockMode parseDockMode(const SStringW &strValue);
    DockMode m_enableDockMode;
  public:
    SOUI_ATTRS_BEGIN()
        ATTR_CUSTOM(L"enableDockMode", OnAttrEnableDockMode)
    SOUI_ATTRS_END()

  protected:
    HRESULT OnAttrEnableDockMode(const SStringW &strValue, BOOL bLoading);

    STDMETHOD_(BOOL, IsParamAcceptable)(THIS_ const ILayoutParam *pLayoutParam) SCONST OVERRIDE;
    STDMETHOD_(void, LayoutChildren)(THIS_ IWindow *pParent) OVERRIDE;
    STDMETHOD_(ILayoutParam *, CreateLayoutParam)(CTHIS) SCONST OVERRIDE;
    STDMETHOD_(SIZE, MeasureChildren)
    (THIS_ const IWindow *pParent, int nWidth, int nHeight) SCONST OVERRIDE;

  protected:
    struct ChildInfo
    {
        IWindow *pWnd;
        SFrameLayoutParam *pParam;
        CSize szDesired;
        CRect rcExtend;
    };

    void CollectChildren(const IWindow *pParent, SList<ChildInfo> &lstChildren) const;
    void CalcExtendRect(const ChildInfo &info, CRect &rcExtend, int nScale) const;
    CSize MeasureChild(const ChildInfo &info, int nWidth, int nHeight) const;
    void LayoutDockTopBottom(IWindow *pParent, SList<ChildInfo *> &lstChildren, CRect &rcAvailable, BOOL bIsTop, const CRect &rcParent);
    void LayoutDockLeftRight(IWindow *pParent, SList<ChildInfo *> &lstChildren, CRect &rcAvailable, BOOL bIsLeft, const CRect &rcParent);
};

SNSEND

#endif
