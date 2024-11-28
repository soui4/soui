#ifndef __SPANELPROXY__H__
#define __SPANELPROXY__H__

SNSBEGIN

template <class T>
class TPanelProxy
	: public T
	, public SPanel {
public:
	STDMETHOD_(long, AddRef)(THIS) OVERRIDE
	{
		return SPanel::AddRef();
	}
	STDMETHOD_(long, Release)(THIS) OVERRIDE
	{
		return SPanel::Release();
	}
	STDMETHOD_(void, OnFinalRelease)(THIS) OVERRIDE
	{
		SPanel::OnFinalRelease();
	}

	STDMETHOD_(IWindow *, ToIWindow)(THIS) OVERRIDE
	{
		return (IWindow *)this;
	}

	STDMETHOD_(BOOL, ShowScrollBar)(THIS_ int wBar, BOOL bShow) OVERRIDE
	{
		return SPanel::ShowScrollBar(wBar, bShow);
	}

	STDMETHOD_(BOOL, EnableScrollBar)(THIS_ int wBar, BOOL bEnable) OVERRIDE
	{
		return SPanel::EnableScrollBar(wBar, bEnable);
	}

	STDMETHOD_(BOOL, IsScrollBarEnable)(THIS_ BOOL bVertical) SCONST OVERRIDE
	{
		return SPanel::IsScrollBarEnable(bVertical);
	}

	STDMETHOD_(void, SetScrollInfo)(THIS_ SCROLLINFO si, BOOL bVertical) OVERRIDE
	{
		return SPanel::SetScrollInfo(si, bVertical);
	}

	STDMETHOD_(BOOL, SetScrollPos)(THIS_ BOOL bVertical, int nNewPos, BOOL bRedraw) OVERRIDE
	{
		return SPanel::SetScrollPos(bVertical, nNewPos, bRedraw);
	}

	STDMETHOD_(int, GetScrollPos)(THIS_ BOOL bVertical) SCONST OVERRIDE
	{
		return SPanel::GetScrollPos(bVertical);
	}

	STDMETHOD_(BOOL, SetScrollRange)
		(THIS_ BOOL bVertical, int nMinPos, int nMaxPos, BOOL bRedraw) OVERRIDE
	{
		return SPanel::SetScrollRange(bVertical, nMinPos, nMaxPos, bRedraw);
	}

	STDMETHOD_(BOOL, GetScrollRange)
		(THIS_ BOOL bVertical, LPINT lpMinPos, LPINT lpMaxPos) SCONST OVERRIDE
	{
		return SPanel::GetScrollRange(bVertical, lpMinPos, lpMaxPos);
	}

	STDMETHOD_(BOOL, HasScrollBar)(THIS_ BOOL bVertical) SCONST OVERRIDE
	{
		return SPanel::HasScrollBar(bVertical);
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
			return SPanel::QueryInterface(id, ppRet);
		}
	}
};

SNSEND
#endif // __SPANELPROXY__H__