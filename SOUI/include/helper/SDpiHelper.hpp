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
 * @brief 处理DPI变化的模板类
 * @tparam T 派生类类型
 */
template <class T>
class SDpiHandler {
  protected:
    /**
     * @brief 检查是否支持DPI感知
     * @return 支持DPI感知返回TRUE，否则返回FALSE
     */
    virtual bool IsDpiAware() const
    {
        return true;
    }

    /**
     * @brief 处理DPI变化事件
     * @param dpi 新的DPI值
     * @param desRect 目标矩形区域
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
     * @brief 处理缩放变化
     * @param nScale 新的缩放比例
     * @param desRect 目标矩形区域
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
     * @brief 根据DPI缩放窗口
     * @param hWnd 窗口句柄
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
        sz.cx = sz.cx * nScale / 100;
        sz.cy = sz.cy * nScale / 100;
        CPoint ntl = rc.CenterPoint();
        ntl.Offset((-sz.cx + rc.Size().cx) / 2, (-sz.cy + rc.Size().cy) / 2);
        rc = CRect(ntl, sz);
        HandleScaleChange(nScale, &rc);
    }

  public:
    /**
     * @brief 处理窗口消息
     * @param hWnd 窗口句柄
     * @param uMsg 消息ID
     * @param wParam 附加参数1
     * @param lParam 附加参数2
     * @param lResult 消息处理结果
     * @param dwMsgMapID 消息映射ID
     * @return 消息是否被处理
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
                return TRUE;
            }
            if (uMsg == WM_DPICHANGED)
            {
                OnDpiChanged((WORD)HIWORD(wParam), (RECT *const)lParam);
                lResult = 0;
                return TRUE;
            }
            break;
        }
        return FALSE;
    }
};

SNSEND

#endif //__DPIHELPER_H_