#ifndef _SCAPTURE_BUTTON_H_
#define _SCAPTURE_BUTTON_H_

#include <control/SCmnCtrl.h>

SNSBEGIN


	DEF_EVT_EXT(EventCapture,EVT_EXTERNAL_BEGIN+100,{CPoint pt_;});

	DEF_EVT_EXT(EventCaptureFinish,EVT_EXTERNAL_BEGIN+101,{CPoint pt_;});

	class SCaptureButton : public SWindow
    {
        DEF_SOBJECT(SWindow,L"captureButton")
    public:        
        SCaptureButton(void);
        ~SCaptureButton(void);

    protected:
        virtual CSize GetDesiredSize(LPCRECT pRcContainer);  
        
        BOOL IsChecked();      
    protected:
        void OnLButtonDown(UINT nFlags, CPoint point);
        void OnLButtonUp(UINT nFlags, CPoint point);
        void OnMouseMove(UINT nFlags, CPoint point);
        void OnMouseLeave();
        
        void OnPaint(IRenderTarget *pRT);
        
        SOUI_MSG_MAP_BEGIN()
            MSG_WM_LBUTTONDOWN(OnLButtonDown)
            MSG_WM_LBUTTONUP(OnLButtonUp)
            MSG_WM_MOUSEMOVE(OnMouseMove)
            MSG_WM_MOUSELEAVE(OnMouseLeave)
            MSG_WM_PAINT_EX(OnPaint)
        SOUI_MSG_MAP_END()
    };
SNSEND
#endif//_SCAPTURE_BUTTON_H_