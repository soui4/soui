#include "stdafx.h"
#include "SCaptureButton.h"

namespace SOUI
{
    SCaptureButton::SCaptureButton(void)
    {
        m_evtSet.addEvent(EVENTID(EventCapture));
        m_evtSet.addEvent(EVENTID(EventCaptureFinish));
    }

    SCaptureButton::~SCaptureButton(void)
    {
    }

    void SCaptureButton::OnLButtonDown( UINT nFlags, CPoint point )
    {
        SWindow::OnLButtonDown(nFlags,point);
//        ReleaseCapture();
        HCURSOR hCursor=GETRESPROVIDER->LoadCursor(_T("target"));
        ::SetCursor(hCursor);
    }

    void SCaptureButton::OnLButtonUp( UINT nFlags, CPoint point )
    {
        SWindow::OnLButtonUp(nFlags,point);
        EventCaptureFinish evt(this);
		evt.pt_ = point;
        FireEvent(evt);
    }

    void SCaptureButton::OnMouseMove( UINT nFlags, CPoint point )
    {
        if(IsChecked())
        {
            EventCapture evt(this);
			evt.pt_ = point;
            FireEvent(evt);
        }
    }

    void SCaptureButton::OnPaint( IRenderTarget *pRT )
    {
        if(!m_pBgSkin) return;
        m_pBgSkin->DrawByIndex(pRT,GetWindowRect(),IsChecked()?1:0);
    }

    void SCaptureButton::GetDesiredSize(SIZE* psz, int nParentWid, int nParentHei)
    {
        if (!m_pBgSkin) {
			psz->cx = 0;
			psz->cy = 0;
			return;
        }
        *psz =  m_pBgSkin->GetSkinSize();
    }

    BOOL SCaptureButton::IsChecked()
    {
        return m_dwState & WndState_PushDown;
    }

    void SCaptureButton::OnMouseLeave()
    {
        
    }

}
