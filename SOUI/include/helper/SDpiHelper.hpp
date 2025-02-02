#ifndef __DPIHELPER_H_
#define __DPIHELPER_H_
#include <windows.h>
#include <helper/SDpiScale.h>
#include <wtl.mini/msgcrack.h>
#include <wtl.mini/souimisc.h>
#include <sdef.h>

SNSBEGIN

template <class T>
class SDpiHandler {
  protected:
    virtual bool IsDpiAware() const
    {
        return true;
    }
    void OnDpiChanged(WORD dpi, const RECT *desRect)
    {
        if (!IsDpiAware())
            return;
        int nScale = dpi * 100 / 96;
        nScale = SDpiScale::NormalizeScale(nScale);
        HandleScaleChange(nScale, desRect);
    }
    //能过重写些函数来屏蔽一些不处理的DPI。。比如只支持 100 150 200。。。所有都支持基本不现实
    virtual void HandleScaleChange(WORD nScale, const RECT *desRect)
    {
        T *pT = static_cast<T *>(this);
        if (nScale != pT->GetRoot()->GetScale()) // As ShostWnd::GetScale is under protect, here, we using SWindow::GetScale.
        {
            pT->SetScale(nScale, desRect);
        }
    }

    void ScaleHost(HWND hWnd)
    {
        int nScale = GetWindowScale(hWnd);
        if (!IsDpiAware())
            nScale = 100;
        nScale = SDpiScale::NormalizeScale(nScale);
        CRect rc;
        ::GetWindowRect(hWnd, (RECT *)&rc);
        CSize sz = rc.Size();
        sz.cx = sz.cx * nScale / 100;
        sz.cy = sz.cy * nScale / 100;
        CPoint ntl = rc.CenterPoint();
        ntl.Offset((-sz.cx + rc.Size().cx) / 2, (-sz.cy + rc.Size().cy) / 2);
        rc = CRect(ntl, sz);
        HandleScaleChange(nScale, &rc);
    }

  public:
    BOOL ProcessWindowMessage(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, LRESULT &lResult, DWORD dwMsgMapID = 0)
    {
        switch (dwMsgMapID)
        {
        case 0:
            if (uMsg == WM_INITDIALOG)
            {
                ScaleHost(hWnd);
                lResult = 0;
            }
            if (uMsg == WM_DPICHANGED)
            {
                OnDpiChanged((WORD)HIWORD(wParam), (RECT *const)lParam);
                lResult = 0;
            }
            break;
        }
        return FALSE;
    }
};

SNSEND

#endif //__DPIHELPER_H_