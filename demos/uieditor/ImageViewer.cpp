#include "stdafx.h"
#include "MainDlg.h"
#include "ImageViewer.h"

CImageViewer::CImageViewer(CMainDlg *pMainDlg)
	: m_pMainDlg(pMainDlg)
	, m_pRoot(NULL)
	, m_pImage(NULL)
	, m_pSliderZoom(NULL)
	, m_nZoom(100)
{}

CImageViewer::~CImageViewer()
{}

void CImageViewer::Init(SWindow *pRoot)
{
	m_pRoot = pRoot;
	
	// 获取图片控件和滑块控件
	m_pImage = sobj_cast<SImageWnd>(pRoot->FindChildByName(L"img_viewer"));
	m_pSliderZoom = sobj_cast<SSliderBar>(pRoot->FindChildByName(L"slider_zoom"));
	
	// 初始化滑块值
	if(m_pSliderZoom)
	{
		m_pSliderZoom->SetValue(m_nZoom);
	}
}

void CImageViewer::LoadImage(const SStringT &strImagePath)
{
	m_strImagePath = strImagePath;
	
    IBitmapS *pBitmap = SResLoadFromFile::LoadImage(strImagePath);
    if(pBitmap)
    {
        m_pImage->SetImage(pBitmap);
        pBitmap->Release();
        SetZoom(100);
    }
}

void CImageViewer::SetZoom(int nZoom)
{
	m_nZoom = nZoom;
	
	// 限制缩放范围
	if(m_nZoom < 20) m_nZoom = 20;
	if(m_nZoom > 500) m_nZoom = 500;
	
	// 更新滑块
	if(m_pSliderZoom)
	{
		m_pSliderZoom->SetValue(m_nZoom);
	}
    // 更新图片
    if(m_pImage)
    {
        IBitmapS *pBitmap = m_pImage->GetImage();
        if(pBitmap){
            CSize sz = pBitmap->Size();
            sz.cx *= m_nZoom*1.0f/100;
            sz.cy *= m_nZoom*1.0f/100;
            m_pImage->SetAttribute(L"size",SStringW().Format(L"%dpx,%dpx",sz.cx,sz.cy));
        }
    }
}

int CImageViewer::GetZoom() const
{
	return m_nZoom;
}

void CImageViewer::OnZoomChanged(IEvtArgs *pEvt)
{
	if(m_pSliderZoom)
	{
		int nValue = m_pSliderZoom->GetValue();
		SetZoom(nValue);
	}
}

void CImageViewer::Clear()
{
	m_pImage->SetImage(NULL);
}