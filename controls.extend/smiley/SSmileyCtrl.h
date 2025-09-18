﻿#ifndef SoSmileyCtrl_h__
#define SoSmileyCtrl_h__

// SoSmileyCtrl.h : Declaration of the CSoSmileyCtrl
#include <richedit.h>
#include <richole.h>

#if !defined(_WIN32) || defined(__MINGW32__)
// ITimerHandler
DEFINE_SGUID(IID_ITimerHandler,
    0x8A0E5678, 0x792F, 0x439b, 0xAE, 0xDD, 0xE8, 0xD4, 0xAB, 0x60, 0x20, 0x41);
#undef INTERFACE
#define INTERFACE ITimerHandler
DECLARE_INTERFACE_(ITimerHandler, IUnknown)
{
    DECLARE_CLASS_SIID(IID_ITimerHandler)
#else
interface __declspec(uuid("8A0E5678-792F-439b-AEDD-E8D4AB602041")) ITimerHandler : IUnknown
{
#endif
    virtual HRESULT STDMETHODCALLTYPE OnTimer(HDC hdc) PURE;
    virtual HRESULT STDMETHODCALLTYPE Clear() PURE;
    virtual HRESULT STDMETHODCALLTYPE GetRect(LPRECT pRect) PURE;
};

// IReOleSource
#if !defined(_WIN32) || defined(__MINGW32__)
DEFINE_SGUID(IID_ISmileySource,
    0xE9FFF8D9, 0x7585, 0x42ce, 0xB6, 0xCE, 0x33, 0x33, 0x62, 0x83, 0x99, 0x4E);
#undef INTERFACE
#define INTERFACE ISmileySource
DECLARE_INTERFACE_(ISmileySource, IUnknown)
{
    DECLARE_CLASS_SIID(IID_ISmileySource)
#else
interface __declspec(uuid("E9FFF8D9-7585-42ce-B6CE-33336283994E")) ISmileySource : IUnknown
{
#endif
    virtual HRESULT STDMETHODCALLTYPE Stream_Load(LPSTREAM pStm) PURE;
    virtual HRESULT STDMETHODCALLTYPE Stream_Save(LPSTREAM pStm) PURE;
    virtual HRESULT STDMETHODCALLTYPE LoadFromID(UINT uID) PURE;
    virtual HRESULT STDMETHODCALLTYPE LoadFromFile(LPCWSTR pszFilePath) PURE;
    virtual HRESULT STDMETHODCALLTYPE GetID(UINT * pID) PURE;
    virtual HRESULT STDMETHODCALLTYPE GetFile(BSTR * bstrFile) PURE;
    virtual HRESULT STDMETHODCALLTYPE GetFrameCount(int *pFrameCount) PURE;
    virtual HRESULT STDMETHODCALLTYPE GetFrameDelay(int iFrame, int *pFrameDelay) PURE;
    virtual HRESULT STDMETHODCALLTYPE GetSize(LPSIZE pSize) PURE;
    virtual HRESULT STDMETHODCALLTYPE Draw(HDC hdc, LPCRECT pRect, int iFrame) PURE;
};

// IReOleHost
#if !defined(_WIN32) || defined(__MINGW32__)
DEFINE_SGUID(IID_ISmileyHost,
    0x0F3687B9, 0x333F, 0x48a4, 0x90, 0x01, 0xC9, 0x94, 0x45, 0x5B, 0x43, 0x0D);
#undef INTERFACE
#define INTERFACE ISmileyHost
DECLARE_INTERFACE_(ISmileyHost, IUnknown)
{
    DECLARE_CLASS_SIID(IID_ISmileyHost)
#else
interface __declspec(uuid("0F3687B9-333F-48a4-9001-C994455B430D")) ISmileyHost : IUnknown
{
#endif
    virtual HRESULT STDMETHODCALLTYPE SendMessage(UINT uMsg, WPARAM wParam, LPARAM lParam,
                                                  LRESULT * pRet) PURE;
    virtual HRESULT STDMETHODCALLTYPE GetHostRect(LPRECT prcHost) PURE;
    virtual HRESULT STDMETHODCALLTYPE InvalidateRect(LPCRECT pRect) PURE;
    virtual HRESULT STDMETHODCALLTYPE CreateSource(ISmileySource * *ppSource) PURE;
    virtual HRESULT STDMETHODCALLTYPE SetTimer(ITimerHandler * pTimerHander, int nInterval) PURE;
    virtual HRESULT STDMETHODCALLTYPE KillTimer(ITimerHandler * pTimerHander) PURE;
    virtual HRESULT STDMETHODCALLTYPE OnTimer(int nInterval) PURE;
    virtual HRESULT STDMETHODCALLTYPE ClearTimer() PURE;
};

// IReOleObject
#if !defined(_WIN32) || defined(__MINGW32__)
DEFINE_SGUID(IID_ISmileyCtrl,
    0x3286141B, 0xC87F, 0x4052, 0xB6, 0xA2, 0x37, 0x63, 0x91, 0xDC, 0xDA, 0xF7);
#undef INTERFACE
#define INTERFACE ISmileyCtrl
DECLARE_INTERFACE_(ISmileyCtrl, IUnknown)
{
    DECLARE_CLASS_SIID(IID_ISmileyCtrl)
#else
interface __declspec(uuid("3286141B-C87F-4052-B6A2-376391DCDAF7")) ISmileyCtrl : IUnknown
{
#endif
    virtual HRESULT STDMETHODCALLTYPE Insert2Richedit(IRichEditOle * ole) PURE;
    virtual HRESULT STDMETHODCALLTYPE SetSource(ISmileySource * pSource) PURE;
    virtual HRESULT STDMETHODCALLTYPE GetSource(ISmileySource * *ppSource) PURE;
};

EXTERN_C const CLSID CLSID_SSmileyCtrl;

template <class Base>
class SComObject : public Base {
  public:
    typedef Base _BaseClass;
    SComObject(void * = NULL) throw()
    {
    }
    // Set refcount to -(LONG_MAX/2) to protect destruction and
    // also catch mismatched Release in debug builds
    virtual ~SComObject() throw()
    {
        Base::m_dwRef = -(LONG_MAX / 2);
        Base::FinalRelease();
    }
    // If InternalAddRef or InternalRelease is undefined then your class
    // doesn't derive from CComObjectRoot
    STDMETHOD_(ULONG, AddRef)()
    {
        return Base::InternalAddRef();
    }
    STDMETHOD_(ULONG, Release)()
    {
        ULONG l = Base::InternalRelease();
        if (l == 0)
            delete this;
        return l;
    }
    // if _InternalQueryInterface is undefined then you forgot BEGIN_COM_MAP
    STDMETHOD(QueryInterface)(REFIID iid, void **ppvObject) throw()
    {
        HRESULT hr = Base::_InternalQueryInterface(iid, ppvObject);
        if (iid == IID_IOleObject)
        {
            IOleObject *pOle = *(IOleObject **)ppvObject;
            pOle->SetClientSite(NULL);
        }
        else if (iid == IID_IPersistStorage)
        {
            IPersistStorage *pStg = *(IPersistStorage **)ppvObject;
            (pStg);
        }
        return hr;
    }
    template <class Q>
    HRESULT STDMETHODCALLTYPE QueryInterface(Q **pp) throw()
    {
        return QueryInterface(__uuidof(Q), (void **)pp);
    }
};

HRESULT CreateSmiley(REFIID riid, void **ppUnk);

#endif // SoSmileyCtrl_h__