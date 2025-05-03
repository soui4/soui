#ifndef _SWINDOW_EX_H_
#define _SWINDOW_EX_H_

#include "core/SWnd.h"

SNSBEGIN
class SWindowEx : public SWindow {
  public:
    DEF_SOBJECT(SWindow, L"windowex")

  protected:
    SOUI_MSG_MAP_BEGIN()
    MSG_WM_LBUTTONDBLCLK(OnLButtonDown) //将双击消息处理为单击
    SOUI_MSG_MAP_END()
};

SNSEND

#endif//_SWINDOW_EX_H_