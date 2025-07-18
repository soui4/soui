#ifndef _SIECTRL_H_
#define _SIECTRL_H_

#include <control/SActiveX.h>
#include <ExDisp.h>
#include <ExDispid.h>
#include <atl.mini/SComCli.h>

SNSBEGIN

struct IWebEvent
{
    virtual void BeforeNavigate2(IDispatch *pDisp,
                                 VARIANT *&url,
                                 VARIANT *&Flags,
                                 VARIANT *&TargetFrameName,
                                 VARIANT *&PostData,
                                 VARIANT *&Headers,
                                 VARIANT_BOOL *&Cancel)
    {
    }
    virtual void NavigateError(IDispatch *pDisp,
                               VARIANT *&url,
                               VARIANT *&TargetFrameName,
                               VARIANT *&StatusCode,
                               VARIANT_BOOL *&Cancel)
    {
    }
    virtual void NavigateComplete2(IDispatch *pDisp, VARIANT *&url)
    {
    }
    virtual void ProgressChange(LONG nProgress, LONG nProgressMax)
    {
    }
    virtual void CommandStateChange(long Command, VARIANT_BOOL Enable)
    {
    }
    virtual void DocumentComplete(IDispatch *pDisp, VARIANT *&url)
    {
    }
    virtual void StatusTextChange(IDispatch *pDisp, VARIANT *&url)
    {
    }
};

class SIECtrlEvtDispatch : public IDispatch {
  public:
    SIECtrlEvtDispatch(IWebEvent *pEventHandler)
        : m_pEventHandler(pEventHandler)
        , m_cRef(1)
    {
    }

    void SetEventHandler(IWebEvent *pEventHandler)
    {
        m_pEventHandler = pEventHandler;
    }

  public: // IUnknown
    STDMETHOD_(ULONG, AddRef)()
    {
        return ++m_cRef;
    }
    STDMETHOD_(ULONG, Release)()
    {
        return --m_cRef;
    }

    STDMETHOD(QueryInterface)(REFIID riid, LPVOID *ppvObject)
    {
        if (IsEqualGUID(riid, __uuidof(IUnknown)) || IsEqualGUID(riid, __uuidof(IDispatch)))
        {
            *ppvObject = this;
            AddRef();
            return S_OK;
        }
        return E_NOINTERFACE;
    }

  public: // IDispatch
    virtual HRESULT STDMETHODCALLTYPE GetTypeInfoCount(
        /* [out] */ UINT *pctinfo)
    {
        return E_NOTIMPL;
    }

    virtual HRESULT STDMETHODCALLTYPE GetTypeInfo(
        /* [in] */ UINT iTInfo,
        /* [in] */ LCID lcid,
        /* [out] */ ITypeInfo **ppTInfo)
    {
        return E_NOTIMPL;
    }

    virtual HRESULT STDMETHODCALLTYPE GetIDsOfNames(
        /* [in] */ REFIID riid,
        /* [size_is][in] */ LPOLESTR *rgszNames,
        /* [range][in] */ UINT cNames,
        /* [in] */ LCID lcid,
        /* [size_is][out] */ DISPID *rgDispId)
    {
        return E_NOTIMPL;
    }

    virtual /* [local] */ HRESULT STDMETHODCALLTYPE Invoke(
        /* [in] */ DISPID dispIdMember,
        /* [in] */ REFIID riid,
        /* [in] */ LCID lcid,
        /* [in] */ WORD wFlags,
        /* [out][in] */ DISPPARAMS *pDispParams,
        /* [out] */ VARIANT *pVarResult,
        /* [out] */ EXCEPINFO *pExcepInfo,
        /* [out] */ UINT *puArgErr);

  protected:
    IWebEvent *m_pEventHandler;
    ULONG m_cRef;
};

class SIECtrl
    : public SActiveX
    , public IMsgFilter {
    DEF_SOBJECT(SActiveX, L"iectrl")
  public:
    SIECtrl(void);
    ~SIECtrl(void);

    IWebBrowser2 *GetIEObject()
    {
        return m_pIE;
    }

    SIECtrlEvtDispatch *GetEventDispatch()
    {
        return &m_eventDispatch;
    }

    BOOL Navigate(const SStringW &strUrl);

  protected:
    virtual void OnAxActivate(IUnknown *pUnknwn);
    virtual BOOL WINAPI PreTranslateMessage(MSG *pMsg);

    int OnCreate(LPVOID);
    void OnDestroy();

    SOUI_MSG_MAP_BEGIN()
    MSG_WM_CREATE(OnCreate)
    MSG_WM_DESTROY(OnDestroy)
    SOUI_MSG_MAP_END()
  protected:
    HRESULT RegisterEventHandler(BOOL inAdvise);

    SOUI_ATTRS_BEGIN()
    ATTR_STRINGW(L"url", m_strUrl, FALSE)
    ATTR_INT(L"disableScrollbar", m_bDisableScrollbar, FALSE)
    ATTR_INT(L"disable3DBorder", m_bDisable3DBorder, FALSE)
    ATTR_INT(L"disableContextMenu", m_bDisableContextMenu, FALSE)
    ATTR_INT(L"disableTextSelect", m_bDisableTextSelect, FALSE)
    SOUI_ATTRS_END()

    SStringW m_strUrl;

    DWORD m_dwCookie;

    SIECtrlEvtDispatch m_eventDispatch;
    SComPtr<IWebBrowser2> m_pIE;

    BOOL m_bDisableScrollbar;
    BOOL m_bDisable3DBorder;
    BOOL m_bDisableContextMenu;
    BOOL m_bDisableTextSelect;
    HWND m_hIEWnd;
};

SNSEND

#endif//_SIECTRL_H_