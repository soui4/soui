#pragma once

class CMainDlg;

class CImageViewer
{
public:
	CImageViewer(CMainDlg *pMainDlg);
	~CImageViewer();

	void Init(SWindow *pRoot);
	void LoadImage(const SStringT &strImagePath);
	void SetZoom(int nZoom);
	int GetZoom() const;

protected:
	void OnZoomChanged(IEvtArgs *pEvt);

    EVENT_MAP_BEGIN()
		EVENT_ID_HANDLER(R.id.slider_zoom,EVT_SLIDER_POS, OnZoomChanged)
	EVENT_MAP_BREAK()

private:
	CMainDlg *m_pMainDlg;
	SWindow *m_pRoot;
	SImageWnd *m_pImage;
	SSliderBar *m_pSliderZoom;
	int m_nZoom;
	SStringT m_strImagePath;
};
