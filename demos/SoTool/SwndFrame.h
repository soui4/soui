#pragma once

#include <core/SNativeWnd.h>

namespace SOUI
{
    class SwndFrame : public SNativeWnd
    {
    public:
        SwndFrame(void);
        ~SwndFrame(void);
        
        BOOL Create(HWND hOwner=0);
		void Hide();
        void Show(RECT rc,HWND hWnd=HWND_TOPMOST);
    protected:
        void OnPaint(HDC hdc);
        void OnTimer(UINT_PTR nIDEvent);
        
        BEGIN_MSG_MAP_EX(SwndFrame)
            MSG_WM_PAINT(OnPaint)
            MSG_WM_TIMER(OnTimer)
            REFLECT_NOTIFICATIONS_EX()
       END_MSG_MAP()
    };

}
