#ifndef __DPIHELPER_H_
#define __DPIHELPER_H_

#include <windows.h>
#include <helper/SDpiScale.h>
#include <wtl.mini/msgcrack.h>
#include <wtl.mini/souimisc.h>
#include <sdef.h>

SNSBEGIN

/**
 * @class SDpiHandler
 * @brief Template class handling DPI changes
 * @tparam T derived class type
 */
template <class T>
class SDpiHandler {
  protected:
    /**
     * @brief Check whether DPI awareness is supported
     * @return Returns TRUE if DPI awareness is supported, FALSE otherwise
     */
    virtual bool IsDpiAware() const
    {
        return true;
    }

    /**
     * @brief Handle the DPI change event
     * @param dpi new DPI value
     * @param desRect destination rectangle
     */
    void OnDpiChanged(WORD dpi, const RECT *desRect)
    {
        if (!IsDpiAware())
            return;
        int nScale = dpi * 100 / 96;
        nScale = SDpiScale::NormalizeScale(nScale);
        HandleScaleChange(nScale, desRect);
    }

    /**
     * @brief Handle the scale change
     * @param nScale new scale factor
     * @param desRect destination rectangle
     */
    virtual void HandleScaleChange(WORD nScale, const RECT *desRect)
    {
        T *pT = static_cast<T *>(this);
        if (nScale != pT->GetRoot()->GetScale()) // As ShostWnd::GetScale is under protect, here, we using SWindow::GetScale.
        {
            pT->SetScale(nScale, desRect);
        }
    }

    /**
     * @brief Scale the window according to DPI
     * @param hWnd window handle
     */
    void ScaleHost(HWND hWnd)
    {
        int nScale = GetWindowScale(hWnd);
        if (!IsDpiAware())
            nScale = 100;
        nScale = SDpiScale::NormalizeScale(nScale);
        CRect rc;
        ::GetWindowRect(hWnd, (RECT *)&rc);
        CSize sz = rc.Size();
        CPoint center = rc.CenterPoint();
        sz.cx = sz.cx * nScale / 100;
        sz.cy = sz.cy * nScale / 100;
        rc = CRect(center, sz);
        rc.OffsetRect(-sz.cx / 2, -sz.cy / 2);
        HandleScaleChange(nScale, &rc);
    }

  public:
    /**
     * @brief Handle the window message
     * @param hWnd window handle
     * @param uMsg message ID
     * @param wParam additional parameter 1
     * @param lParam additional parameter 2
     * @param lResult message processing result
     * @param dwMsgMapID message map ID
     * @return whether the message was handled
     */
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

#endif /**< __DPIHELPER_H_ */