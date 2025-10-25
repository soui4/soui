#include "stdafx.h"
#include "SouiRealWndHandler.h"
#include "ScintillaWnd.h"
#include "designer/DesignWnd.h"

namespace SOUI
{
	CSouiRealWndHandler::CSouiRealWndHandler(void)
	{
	}

	CSouiRealWndHandler::~CSouiRealWndHandler(void)
	{
	}

	HWND CSouiRealWndHandler::OnRealWndCreate(IWindow *pRealWnd1)
	{
		SRealWnd *pRealWnd = sobj_cast<SRealWnd>(pRealWnd1);
		if (pRealWnd->GetRealClassName()->CompareNoCase(_T("scintilla"))==0)
		{
			CScintillaWnd *pWnd = new CScintillaWnd;
			BOOL bOK = pWnd->Create(pRealWnd->GetRealWindowName()->c_str(), WS_CHILD, CRect(), pRealWnd->GetContainer()->GetHostHwnd(), pRealWnd->GetID(), SApplication::getSingleton().GetModule());
			if (!bOK)
			{
				SASSERT(FALSE);
				delete pWnd;
				return 0;
			}
			pRealWnd->SetUserData((ULONG_PTR)pWnd);
			return pWnd->m_hWnd;
		}
		else if (pRealWnd->GetRealClassName()->CompareNoCase(_T("designer_wnd"))==0)
		{
			CDesignWnd *pWnd = new CDesignWnd(_T("layout:designer_wnd"));
			HWND hWnd = pWnd->CreateEx(pRealWnd->GetContainer()->GetHostHwnd(),WS_CHILD|WS_CLIPCHILDREN|WS_HSCROLL|WS_VSCROLL,0,0,0,0,0);
			if (!hWnd)
			{
				SASSERT(FALSE);
				delete pWnd;
				return 0;
			}
			pRealWnd->SetUserData((ULONG_PTR)pWnd);
			return hWnd;
		}else
		{
			return 0;
		}
	}

	void CSouiRealWndHandler::OnRealWndDestroy(IWindow *pRealWnd1)
	{
		SRealWnd *pRealWnd = sobj_cast<SRealWnd>(pRealWnd1);
		if (pRealWnd->GetRealClassName()->CompareNoCase(_T("scintilla"))==0)
		{
			CScintillaWnd *pWnd = (CScintillaWnd *)pRealWnd->GetUserData();
			if (pWnd)
			{
				pWnd->DestroyWindow();
				delete pWnd;
			}
		}
	}

	//不处理，返回FALSE
	BOOL CSouiRealWndHandler::OnRealWndInit(IWindow *pRealWnd)
	{
		return FALSE;
	}

	BOOL CSouiRealWndHandler::OnRealWndPosition(THIS_ IWindow * pRealWnd, const RECT *rcWnd)
	{
		return FALSE;
	}

}
