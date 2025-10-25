#ifndef __SSEARCHDROPDOWNLIST__H__
#define __SSEARCHDROPDOWNLIST__H__

#include <control/SDropDown.h>
#include <control/SListView.h>
#include <proxy/SWindowProxy.h>

SNSBEGIN

class SDropdownList;
class SOUI_EXP SSearchDropdownList
    : public TWindowProxy<ISearchDropdownList>
    , public ISDropDownOwner {
    typedef enum _DropAlign
    {
        DROPALIGN_LEFT=0,
        DROPALIGN_RIGHT,
    }DropAlign;
    DEF_SOBJECT(SWindow, L"searchdropdownlist")

  protected:
    SDropdownList *m_pDropDownWnd; /**< DropDown指针 */
    DropAlign m_nDropAlign;
    SLayoutSize m_nMaxDropHeight;
    SAutoRefPtr<SListView> m_pListBox;
    SXmlDoc m_xmlDropdownStyle;   /**< Dropdown style XML document */
  public:
    SSearchDropdownList(void);
    ~SSearchDropdownList(void);

    STDMETHOD_(void, DropDown)(THIS_ const IStringT *pText) OVERRIDE;

    STDMETHOD_(void, CloseUp)(THIS_ int code DEF_VAL(IDCANCEL)) OVERRIDE;

    STDMETHOD_(BOOL, IsDropdown)(THIS) SCONST OVERRIDE;
    
  public:
    SOUI_ATTRS_BEGIN()
        ATTR_ENUM_BEGIN(L"dropAlign", DropAlign, FALSE)
          ATTR_ENUM_VALUE(L"left", DROPALIGN_LEFT)
          ATTR_ENUM_VALUE(L"right", DROPALIGN_RIGHT)
        ATTR_ENUM_END(m_nDropAlign)
        ATTR_LAYOUTSIZE(L"maxDropHeight", m_nMaxDropHeight, FALSE)
    SOUI_ATTRS_END()

  protected:
    CRect GetClientRect() const OVERRIDE {return CRect();}
    CRect GetWindowRect() const OVERRIDE {return CRect();}
    BOOL CreateChildren(SXmlNode xmlNode) OVERRIDE;

    BOOL WINAPI FireEvent(IEvtArgs *evt) OVERRIDE;

    SWindow *GetDropDownOwner() OVERRIDE;

    void OnCreateDropDown(SDropDownWnd *pDropDown) OVERRIDE;

    void OnDestroyDropDown(SDropDownWnd *pDropDown) OVERRIDE;

    STDMETHOD_(void,GetDesiredSize)(SIZE *psz, int nParentWid, int nParentHei) OVERRIDE{
        psz->cx = 0;
        psz->cy = 0;
    }
    
  protected:
    BOOL CreateListBox(SXmlNode xmlNode);
    void AdjustDropdownList();
};

SNSEND

#endif //__SSEARCHDROPDOWNLIST__H__