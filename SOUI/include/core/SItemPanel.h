//////////////////////////////////////////////////////////////////////////
//  Class Name: SOsrPanel
// Description: A Framework wrapping frame to be used in a swindow.
//     Creator: Huang Jianxiong
//     Version: 2011.10.20 - 1.0 - Create
//////////////////////////////////////////////////////////////////////////

#pragma once

#include "SwndContainerImpl.h"

SNSBEGIN

class SOsrPanel;

struct IItemContainer
{
    virtual void OnItemSetCapture(SOsrPanel *pItem, BOOL bCapture) = 0; //设置or释放鼠标捕获
    virtual BOOL OnItemGetRect(const SOsrPanel *pItem,
                               CRect &rcItem) const = 0; //获得表项的显示位置
    virtual BOOL IsItemRedrawDelay() const = 0;          //指示表项的更新方式
};

struct IHostProxy
{
    virtual IObject *GetHost() = 0;
    virtual BOOL OnHostFireEvent(IEvtArgs *e) = 0;
    virtual BOOL IsHostUpdateLocked() const = 0;
    virtual BOOL IsHostVisible() const = 0;
    virtual CRect GetHostRect() const = 0;
    virtual void InvalidateHostRect(LPCRECT pRc,BOOL bClip) = 0;
    virtual ISwndContainer *GetHostContainer() = 0;
	virtual void OnLayoutDirty() = 0;
};

class SHostProxy : public IHostProxy {
  public:
    explicit SHostProxy(SWindow *pHost)
        : m_pHost(pHost)
    {
    }

  public:
    virtual IObject *GetHost()
    {
        return m_pHost;
    }
    virtual BOOL OnHostFireEvent(IEvtArgs *e)
    {
        return m_pHost->FireEvent(e);
    }
    virtual BOOL IsHostUpdateLocked() const
    {
        return m_pHost->IsUpdateLocked();
    }
    virtual BOOL IsHostVisible() const
    {
        return m_pHost->IsVisible(TRUE);
    }
    virtual CRect GetHostRect() const
    {
        return m_pHost->GetClientRect();
    }
    virtual void InvalidateHostRect(LPCRECT pRc,BOOL bClip)
    {
        m_pHost->InvalidateRect(pRc,TRUE,bClip);
    }
    virtual ISwndContainer *GetHostContainer()
    {
        return m_pHost->GetContainer();
    }

	virtual void OnLayoutDirty(){}
  protected:
    SWindow *m_pHost;
};

class SOUI_EXP SOsrPanel
    : public TWindowProxy<IOsrPanel>
    , public SwndContainerImpl {
    DEF_SOBJECT(SWindow, L"osrPanel")
  public:
    SOsrPanel(IHostProxy *pFrameHost, IItemContainer *pItemContainer);
    virtual ~SOsrPanel()
    {
    }

    static BOOL IsItemInClip(const SMatrix &mtx, const CRect &rcClip, const IRegionS *clipRgn, const CRect &rcItem);

  public:
    STDMETHOD_(void, SetItemIndex)(THIS_ LPARAM lp) OVERRIDE;
    STDMETHOD_(LPARAM, GetItemIndex)(THIS) SCONST OVERRIDE;
    STDMETHOD_(void, SetItemData)(THIS_ LPARAM dwData) OVERRIDE;
    STDMETHOD_(LPARAM, GetItemData)(THIS) SCONST OVERRIDE;
	STDMETHOD_(void,PtToHost)(THIS_ POINT *pt) SCONST OVERRIDE;
  public:
    STDMETHOD_(void, OnFinalRelease)(THIS) OVERRIDE;
    STDMETHOD_(BOOL, InitFromXml)(THIS_ IXmlNode *pNode) OVERRIDE;

  public: // SwndContainerImpl
    STDMETHOD_(BOOL, OnFireEvent)(IEvtArgs *evt) OVERRIDE;

    STDMETHOD_(void, GetContainerRect)(RECT *ret) const OVERRIDE;

	STDMETHOD_(void, UpdateRegion)(IRegionS *rgn) OVERRIDE;

    STDMETHOD_(void, OnRedraw)(LPCRECT rc,BOOL bClip) OVERRIDE;

    STDMETHOD_(BOOL, OnReleaseSwndCapture)() OVERRIDE;

    STDMETHOD_(SWND, OnSetSwndCapture)(SWND swnd) OVERRIDE;
    STDMETHOD_(HWND, GetHostHwnd)() OVERRIDE;
    STDMETHOD_(LPCWSTR, GetTranslatorContext)() const OVERRIDE;
    STDMETHOD_(void, FrameToHost)(RECT *rc) const OVERRIDE;
    STDMETHOD_(BOOL, IsTranslucent)() const OVERRIDE;
    STDMETHOD_(BOOL, IsSendWheel2Hover)() const OVERRIDE;

    STDMETHOD_(BOOL, UpdateWindow)() OVERRIDE;

    STDMETHOD_(void, UpdateTooltip)() OVERRIDE;

    STDMETHOD_(IMessageLoop *, GetMsgLoop)() OVERRIDE;

    STDMETHOD_(IScriptModule *, GetScriptModule)() OVERRIDE;

    STDMETHOD_(int, GetScale)() const OVERRIDE;

    STDMETHOD_(void, EnableIME)(BOOL bEnable) OVERRIDE;

    STDMETHOD_(void, OnUpdateCursor)() OVERRIDE;

  public: // SWindow
    virtual LRESULT DoFrameEvent(UINT uMsg, WPARAM wParam, LPARAM lParam);
    virtual void ModifyItemState(DWORD dwStateAdd, DWORD dwStateRemove);

    virtual SWND SwndFromPoint(CPoint &pt, bool bIncludeMsgTransparent = false);

    virtual void Draw(IRenderTarget *pRT, const CRect &rc);

    virtual void BeforePaint(IRenderTarget *pRT, SPainter &painter);

    virtual BOOL NeedRedrawWhenStateChange();
    virtual BOOL UpdateToolTip(CPoint pt, SwndToolTipInfo &tipInfo);
	virtual void RequestRelayout(SWND hSource, BOOL bSourceResizable);

    CRect GetItemRect() const;
    void SetItemCapture(BOOL bCapture);

  protected:
    void OnShowWindow(BOOL bShow, UINT nStatus);
    void OnDestroy();
    SOUI_MSG_MAP_BEGIN()
        MSG_WM_DESTROY(OnDestroy)
        MSG_WM_SHOWWINDOW(OnShowWindow)
    SOUI_MSG_MAP_END()
  protected:
    IHostProxy *m_pHostProxy;
    IItemContainer *m_pItemContainer;
    LPARAM m_lpItemIndex;
    LPARAM m_dwData;
};

template <class T>
class TOsrPanelProxy
    : public T
    , public SOsrPanel {
  public:
    TOsrPanelProxy(IHostProxy *pFrameHost, IItemContainer *pItemContainer)
        : SOsrPanel(pFrameHost, pItemContainer)
    {
    }

    STDMETHOD_(long, AddRef)(THIS) OVERRIDE
    {
        return SOsrPanel::AddRef();
    }
    STDMETHOD_(long, Release)(THIS) OVERRIDE
    {
        return SOsrPanel::Release();
    }
    STDMETHOD_(void, OnFinalRelease)(THIS) OVERRIDE
    {
        SOsrPanel::OnFinalRelease();
    }

    STDMETHOD_(IWindow *, ToIWindow)(THIS) OVERRIDE
    {
        return this;
    }

    STDMETHOD_(HRESULT, QueryInterface)(REFGUID id, IObjRef **ppRet) OVERRIDE
    {
        if (id == __uuidof(T))
        {
            *ppRet = (T *)this;
            AddRef();
            return S_OK;
        }
        else
        {
            return SOsrPanel::QueryInterface(id, ppRet);
        }
    }

    STDMETHOD_(void, SetItemIndex)(THIS_ LPARAM lp) OVERRIDE
    {
        return SOsrPanel::SetItemIndex(lp);
    }
    STDMETHOD_(LPARAM, GetItemIndex)(THIS) SCONST OVERRIDE
    {
        return SOsrPanel::GetItemIndex();
    }
    STDMETHOD_(void, SetItemData)(THIS_ LPARAM dwData) OVERRIDE
    {
        return SOsrPanel::SetItemData(dwData);
    }
    STDMETHOD_(LPARAM, GetItemData)(THIS) SCONST OVERRIDE
    {
        return SOsrPanel::GetItemData();
    }
	STDMETHOD_(void,PtToHost)(THIS_ POINT *pt) SCONST OVERRIDE{
		return SOsrPanel::PtToHost(pt);
	}

};

class SOUI_EXP SItemPanel : public TOsrPanelProxy<IItemPanel> {
    DEF_SOBJECT(SOsrPanel, L"ItemPanel")
  public:
    static SItemPanel *Create(IHostProxy *pFrameHost, SXmlNode xmlNode, IItemContainer *pItemContainer);

  public:
    SItemPanel(IHostProxy *pFrameHost, IItemContainer *pItemContainer);
    virtual ~SItemPanel()
    {
    }

  public:
    STDMETHOD_(void, SetSkin)(THIS_ ISkinObj *pSkin) OVERRIDE;
    STDMETHOD_(void, SetColor)(THIS_ COLORREF crBk, COLORREF crSelBk) OVERRIDE;

  public:
    SOUI_ATTRS_BEGIN()
        ATTR_COLOR(L"colorNormal", m_crBk, FALSE)
        ATTR_COLOR(L"colorSelected", m_crSelBk, FALSE)
        ATTR_COLOR(L"colorHover", m_crHover, FALSE)
    SOUI_ATTRS_END()

  protected:
    STDMETHOD_(COLORREF, GetBkgndColor)(THIS) SCONST OVERRIDE;
    STDMETHOD_(BOOL, OnFireEvent)(IEvtArgs *evt);

  protected:
    COLORREF m_crBk, m_crSelBk, m_crHover;
};

SNSEND