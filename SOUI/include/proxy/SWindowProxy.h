#ifndef __SWINDOWPROXY__H__
#define __SWINDOWPROXY__H__

SNSBEGIN

template <class T>
class TWindowProxy
	: public T
	, public SWindow {
public:

	STDMETHOD_(long, AddRef)(THIS) OVERRIDE
	{
		return SWindow::AddRef();
	}
	STDMETHOD_(long, Release)(THIS) OVERRIDE
	{
		return SWindow::Release();
	}
	STDMETHOD_(void, OnFinalRelease)(THIS) OVERRIDE
	{
		SWindow::OnFinalRelease();
	}

	STDMETHOD_(IWindow *, ToIWindow)(THIS) OVERRIDE
	{
		return this;
	}

	STDMETHOD_(HRESULT, QueryInterface)(REFGUID id, IObjRef **ppRet) OVERRIDE
	{
		if (id == __suidof(T))
		{
			*ppRet = (T *)this;
			AddRef();
			return S_OK;
		}
		else
		{
			return E_NOTIMPL;
		}
	}
};

SNSEND
#endif // __SWINDOWPROXY__H__