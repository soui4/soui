#ifndef __SFRAME__H__
#define __SFRAME__H__
#include <core/SWnd.h>

SNSBEGIN

class SOUI_EXP SFrame : public SWindow {
    DEF_SOBJECT(SWindow, L"frame")
  public:
    SFrame(void);
    ~SFrame(void);

  public:
    STDMETHOD_(BOOL, IsFocusable)(THIS) SCONST OVERRIDE;

  protected:
    SWND SwndFromPoint(CPoint &pt, BOOL bIncludeMsgTransparent) const override;
};

SNSEND
#endif // __SFRAME__H__