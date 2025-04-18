// MainDlg.h : interface of the CMainDlg class
//
/////////////////////////////////////////////////////////////////////////////
#pragma once
#include "../uieditor/Global.h"

struct IListener{
	virtual void OnEditorExit() = 0;
	virtual void OnResize() = 0;
	virtual void OnRePos(const POINT *pt) = 0;
};

class CPreviewHost : public SHostWnd
{
public:
	CPreviewHost(IListener *pListener,LPCTSTR pszLayoutId, HWND hEditor = 0);
	~CPreviewHost();

	HWND GetEditor() const { return m_hOwner; }
protected:
	virtual BOOL OnLoadLayoutFromResourceID(SXmlDoc& xmlDoc) override;

protected:
	void GetSwndIndex(SWindow *pWnd,SList<int> &lstIndex);
	void SelectCtrlByOrder(int *pOrder,int nLen);
	void OnSwndInitFinish(IEvtArgs *e);
	void Reload();

	SWindow * m_pHover;
	SWindow * m_pSel;
	BOOL	  m_bSOUIWnd;
	BOOL	  m_bVirtualRoot;
	HWND	  m_hOwner;
	
protected:
	//soui消息
	EVENT_MAP_BEGIN()
		EVENT_HANDLER(EventSwndInitFinish::EventID, OnSwndInitFinish)
	EVENT_MAP_END2(SHostWnd)

	LRESULT OnMouseEvent(UINT uMsg, WPARAM wParam, LPARAM lParam);
	LRESULT OnNcMouseEvent(UINT uMsg, WPARAM wParam, LPARAM lParam);

	BOOL OnCopyData(HWND wnd, PCOPYDATASTRUCT pCopyDataStruct);
	int OnCreate(LPCREATESTRUCT lpCreateStruct);
	void OnDestroy();
	void OnSize(UINT nType, CSize size);
	void OnExitSizeMove();
	LRESULT OnHeart(UINT uMsg, WPARAM wp, LPARAM lp);
	
	//HostWnd真实窗口消息处理
	BEGIN_MSG_MAP_EX(CPreviewHost)
		MSG_WM_COPYDATA(OnCopyData)
		MSG_WM_CREATE(OnCreate)
		MSG_WM_DESTROY(OnDestroy)
		MESSAGE_HANDLER_EX(kmsg_heart, OnHeart)
		MSG_WM_EXITSIZEMOVE(OnExitSizeMove)
		MESSAGE_RANGE_HANDLER_EX(WM_MOUSEFIRST, WM_MOUSELAST, OnMouseEvent)
		MESSAGE_RANGE_HANDLER_EX(WM_NCMOUSEMOVE,WM_NCMBUTTONDBLCLK,OnNcMouseEvent)
		CHAIN_MSG_MAP(SHostWnd)
	END_MSG_MAP()

	SStringA	m_utf8Buffer;
	int			m_iRootIndex;//root element index in soui 
	IListener * m_pListener;
};
