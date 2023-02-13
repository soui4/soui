#include "StdAfx.h"
#include "SRealWndHandler_Scintilla.h"
#include "ScintillaWnd.h"

namespace SOUI
{
	SRealWndHandler_Scintilla::SRealWndHandler_Scintilla(void)
	{
	}

	SRealWndHandler_Scintilla::~SRealWndHandler_Scintilla(void)
	{
	}

	HWND SRealWndHandler_Scintilla::OnRealWndCreate( IWindow *pReal )
	{
		SAutoRefPtr<IRealWnd> pRealWnd;
		pReal->QueryInterface(__uuidof(IRealWnd),(IObjRef**)&pRealWnd);
		if(pRealWnd->GetRealClassName()->CompareNoCase(CScintillaWnd::GetScintillaWndClass())==0)
		{
			CScintillaWnd *pWnd=new CScintillaWnd();
			BOOL bOK=pWnd->Create(pRealWnd->GetRealWindowName()->c_str(),WS_CHILD,CRect(0,0,0,0),pReal->GetContainer()->GetHostHwnd(),pReal->GetID(),SApplication::getSingleton().GetModule());
			if(!bOK)
			{
				SASSERT(FALSE);
				delete pWnd;
				return 0;
			}
			pRealWnd->SetData(pWnd);
			return pWnd->m_hWnd;
		}else
		{
			return 0;
		}
	}

	void SRealWndHandler_Scintilla::OnRealWndDestroy( IWindow *pWnd )
	{
		SAutoRefPtr<IRealWnd> pRealWnd;
		pWnd->QueryInterface(__uuidof(IRealWnd),(IObjRef**)&pRealWnd);
		if(pRealWnd->GetRealClassName()->CompareNoCase(CScintillaWnd::GetScintillaWndClass())==0)
		{
			CScintillaWnd *pWnd=(CScintillaWnd *)pRealWnd->GetData();
			if(pWnd)
			{
				pWnd->DestroyWindow();
				delete pWnd;
			}
		}
	}

	//不处理，返回FALSE
	BOOL SRealWndHandler_Scintilla::OnRealWndSize( IWindow *pRealWnd )
	{
		return FALSE;
	}

	//不处理，返回FALSE
	BOOL SRealWndHandler_Scintilla::OnRealWndInit( IWindow *pRealWnd )
	{
		return FALSE;
	}

	BOOL SRealWndHandler_Scintilla::OnRealWndPosition(IWindow *pWnd, const RECT *rcWnd)
	{
		SAutoRefPtr<IRealWnd> pRealWnd;
		pWnd->QueryInterface(__uuidof(IRealWnd),(IObjRef**)&pRealWnd);
		HWND hRealWnd = pRealWnd->GetRealHwnd(TRUE);
		if(IsWindow(hRealWnd))
		{
			CRect rc(rcWnd);
			::SetWindowPos(hRealWnd, 0, rc.left,rc.top, rc.Width(), rc.Height(), SWP_NOZORDER);
			::UpdateWindow(hRealWnd);
		}
		return TRUE;
	}

}
