#ifndef _PREVIEWHOST_H_
#define _PREVIEWHOST_H_
#include "SizingFrame.h"

struct IListener{
	virtual void OnResize() = 0;
	virtual void OnRePos(const POINT *pt) = 0;
	virtual void OnSelectedCtrl(const int * pOrder, int nLen) = 0;
	virtual void OnUpdateSize(int nWidth, int nHeight) = 0;
	virtual void OnUpdatePos(SStringW strPos) = 0;
};

class CPreviewHost : public SHostWnd
{
public:
	CPreviewHost(IListener *pListener,LPCTSTR pszLayoutId, HWND hEditor = 0);
	~CPreviewHost();

	HWND GetEditor() const { return m_hOwner; }
	void UpdateLayoutBuf(SStringA strUtf8);
	void SelectCtrlByOrder(const int *pOrder,int nLen);
protected:
	virtual BOOL OnLoadLayoutFromResourceID(SXmlDoc& xmlDoc) override;

protected:
	void GetSwndIndex(SWindow *pWnd,SList<int> &lstIndex);
	void Reload();

	SWindow * m_pHover;
	SSizingFrame * m_pSel;
	BOOL	  m_bSOUIWnd;
	BOOL	  m_bVirtualRoot;
	HWND	  m_hOwner;
	
protected:
	void OnSwndInitFinish(IEvtArgs *e);
	void OnFrameMoved(IEvtArgs *e);
	//soui消息
	EVENT_MAP_BEGIN()
		EVENT_HANDLER(EventSwndInitFinish::EventID, OnSwndInitFinish)
		EVENT_HANDLER(EventFrameMoved::EventID, OnFrameMoved)
	EVENT_MAP_END2(SHostWnd)

	LRESULT OnMouseEvent(UINT uMsg, WPARAM wParam, LPARAM lParam);
	LRESULT OnNcMouseEvent(UINT uMsg, WPARAM wParam, LPARAM lParam);

	int OnCreate(LPCREATESTRUCT lpCreateStruct);
	void OnSize(UINT nType, CSize size);
	void OnExitSizeMove();
	
	//HostWnd真实窗口消息处理
	BEGIN_MSG_MAP_EX(CPreviewHost)
		MSG_WM_CREATE(OnCreate)
		MSG_WM_SIZE(OnSize)
		MSG_WM_EXITSIZEMOVE(OnExitSizeMove)
		MESSAGE_RANGE_HANDLER_EX(WM_MOUSEFIRST, WM_MOUSELAST, OnMouseEvent)
		MESSAGE_RANGE_HANDLER_EX(WM_NCMOUSEMOVE,WM_NCMBUTTONDBLCLK,OnNcMouseEvent)
		CHAIN_MSG_MAP(SHostWnd)
	END_MSG_MAP()

	SStringA	m_utf8Buffer;
	int			m_iRootIndex;//root element index in soui 
	IListener * m_pListener;
};

#endif
