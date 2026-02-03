#ifndef _DESIGNWND_H_
#define _DESIGNWND_H_

#include "PreviewHost.h"

SNSBEGIN
class CXmlEditor;
class CDesignWnd : public SHostWnd,IListener
{
	CPreviewHost	*m_previewHost;
	CXmlEditor *m_pXmlEditor;
	int m_nDelta;
public:
	CDesignWnd(LPCTSTR pszLayoutId);
	~CDesignWnd(void);
	void SetXmlEditor(CXmlEditor *pXmlEditor){
		m_pXmlEditor = pXmlEditor;
	}
	BOOL Init(LPCTSTR pszLayoutId,HWND hWndParent);
	void UpdateLayoutBuf(SStringA strUtf8);
	void SelectCtrlByOrder(const int *pOrder,int nLen);
	void Close();
    XmlParseResult GetXmlParseResult() const
    {
        return m_previewHost->GetXmlParseResult();
    }

  protected:
	void OnResize() override;
	void OnRePos(const POINT *pt) override;
	void OnSelectedCtrl(const int * pOrder, int nLen) override;
	void OnUpdateSize(int nWidth, int nHeight) override;
	void OnUpdatePos(SStringW strPos,BOOL bRemoveSize) override;
protected:
	CPoint GetViewPos() const;
	void UpdateViewPos();

protected:
    STDMETHOD_(BOOL,IsDesignerMode)(THIS) SCONST override{
		return TRUE;
    }
protected:
	void SetScrollMax(int hmax, int vmax) const;
	int OnCreate(LPCREATESTRUCT lpCreateStruct);
	void OnScroll(int nBar,int nSBCode, int nPos, HWND hScrollBar);
	void OnHScroll(int nSBCode, int nPos, HWND hScrollBar);
	void OnVScroll(int nSBCode, int nPos, HWND hScrollBar);
	void OnSize(UINT nType, CSize size);
	BOOL OnMouseWheel(UINT nFlags, short zDelta, CPoint pt);

	BEGIN_MSG_MAP_EX(CDesignWnd)
		MSG_WM_CREATE(OnCreate)
		MSG_WM_HSCROLL(OnHScroll)
		MSG_WM_VSCROLL(OnVScroll)
		MSG_WM_MOUSEWHEEL(OnMouseWheel)
		MSG_WM_SIZE(OnSize)
		CHAIN_MSG_MAP(SHostWnd)
	END_MSG_MAP()
};

SNSEND
#endif//_DESIGNWND_H_