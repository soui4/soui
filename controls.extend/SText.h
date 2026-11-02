
#pragma once
#include "core/SWnd.h"

SNSBEGIN

class SText : public SStatic {
    DEF_SOBJECT(SStatic, L"textex")
  public:
    virtual void DrawText(IRenderTarget *pRT,
                          LPCTSTR pszBuf,
                          int cchText,
                          LPRECT pRect,
                          UINT uFormat);
};

SNSEND
