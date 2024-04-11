#ifndef __SHOSTWNDPROXY__H__
#define __SHOSTWNDPROXY__H__

SNSBEGIN


template <class T>
class THostWndProxy
	: public T
	, public SHostWnd {
public:
	THostWndProxy(LPCWSTR pszResId)
		: SHostWnd(pszResId)
	{
	}
	THostWndProxy(LPCSTR pszResId)
		: SHostWnd(pszResId)
	{
	}

public:
	STDMETHOD_(long, AddRef)(THIS)
	{
		return SHostWnd::AddRef();
	}
	STDMETHOD_(long, Release)(THIS)
	{
		return SHostWnd::Release();
	}
	STDMETHOD_(void, OnFinalRelease)(THIS)
	{
		SHostWnd::OnFinalRelease();
	}

	STDMETHOD_(INativeWnd*, GetNative)(THIS) OVERRIDE {
		return SHostWnd::GetNative();
	}

	STDMETHOD_(HWND, CreateEx)
		(THIS_ HWND hWndParent, DWORD dwStyle, DWORD dwExStyle, int x, int y, int nWidth, int nHeight)
	{
		return SHostWnd::CreateEx(hWndParent, dwStyle, dwExStyle, x, y, nWidth, nHeight);
	}
	STDMETHOD_(HWND, Create)
		(THIS_ HWND hWndParent, int x = 0, int y = 0, int nWidth = 0, int nHeight = 0)
	{
		return SHostWnd::Create(hWndParent, x, y, nWidth, nHeight);
	}

	STDMETHOD_(BOOL, InitFromXml)(THIS_ IXmlNode *pNode) OVERRIDE{
		return SHostWnd::InitFromXml(pNode);
	}

	STDMETHOD_(void, SetLayoutId)(THIS_ LPCTSTR pszLayoutId) OVERRIDE
	{
		SHostWnd::SetLayoutId(pszLayoutId);
	}

	STDMETHOD_(IWindow *, GetIRoot)(THIS) OVERRIDE
	{
		return SHostWnd::GetIRoot();
	}

	STDMETHOD_(BOOL,IsTranslucent)(CTHIS) SCONST OVERRIDE{
		return SHostWnd::IsTranslucent();
	}
	STDMETHOD_(IHostPresenter*,GetPresenter)(THIS) OVERRIDE{
		return SHostWnd::GetPresenter();
	}

	STDMETHOD_(void,SetPresenter)(THIS_ IHostPresenter* pPresenter) OVERRIDE{
		return SHostWnd::SetPresenter(pPresenter);
	}

	STDMETHOD_(IMessageLoop*,GetMsgLoop)(THIS) OVERRIDE{
		return SHostWnd::GetMsgLoop();
	}

	STDMETHOD_(IWindow *, FindIChildByID)(THIS_ int nId) OVERRIDE{
		return SHostWnd::FindIChildByID(nId);
	}

	STDMETHOD_(IWindow *, FindIChildByName)(THIS_ LPCWSTR pszName) OVERRIDE{
		return SHostWnd::FindIChildByName(pszName);
	}

	STDMETHOD_(IWindow *, FindIChildByNameA)(THIS_ LPCSTR pszName) OVERRIDE{
		return SHostWnd::FindIChildByNameA(pszName);
	}

	STDMETHOD_(INcPainter*,GetNcPainter)(THIS) OVERRIDE
	{
		return SHostWnd::GetNcPainter();
	}

	STDMETHOD_(void, SetEventHandler)(THIS_ FunCallback fun, void *ctx) OVERRIDE{
		return SHostWnd::SetEventHandler(fun,ctx);
	}

	STDMETHOD_(EventHandlerInfo*, GetEventHandler)(THIS) OVERRIDE{
		return SHostWnd::GetEventHandler();
	}

	STDMETHOD_(BOOL, AnimateHostWindow)(THIS_ DWORD dwTime, DWORD dwFlags) OVERRIDE{
		return SHostWnd::AnimateHostWindow(dwTime,dwFlags);
	}

	STDMETHOD_(void,EnableDragDrop)(THIS) OVERRIDE{
		return SHostWnd::EnableDragDrop();
	}
	STDMETHOD_(void,ShowHostWnd)(THIS_ int uShowCmd,BOOL bWaitAniDone) OVERRIDE{
		return SHostWnd::ShowHostWnd(uShowCmd,bWaitAniDone);
	}
	STDMETHOD_(void, EnablePrivateUiDef)(THIS_ BOOL bEnable) OVERRIDE{
		return SHostWnd::EnablePrivateUiDef(bEnable);
	}
	STDMETHOD_(void, SetScale)(THIS_ int nScale,LPCRECT pDestRect){
		return SHostWnd::SetScale(nScale,pDestRect);
	}

};

SNSEND
#endif // __SHOSTWNDPROXY__H__