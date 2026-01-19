#ifndef SWAVEPROGRESS_H
#define SWAVEPROGRESS_H

#include <control/SCmnCtrl.h>
#include <interface/STimelineHandler-i.h>
#include <matrix/SMatrix.h>

SNSBEGIN

class SWaveProgress : public SProgress , public ITimelineHandler{
    DEF_SOBJECT(SProgress, L"waveProgress")
  public:
    SWaveProgress(void);
    ~SWaveProgress(void);

    SOUI_ATTRS_BEGIN()
      ATTR_COLOR(L"colorBack", m_crBackground, TRUE)
      ATTR_COLOR(L"colorFore", m_crForeground, TRUE)
      ATTR_COLOR(L"colorWave", m_crWave, TRUE)
      ATTR_COLOR(L"colorBorder", m_crBorder, TRUE)
      ATTR_FLOAT(L"waveSpeed", m_fWaveSpeed, TRUE)
      ATTR_FLOAT(L"waveHeight", m_fWaveHeight, TRUE)
      ATTR_INT(L"waveCount", m_nWaveCount, TRUE)
    SOUI_ATTRS_END()
 public:
    STDMETHOD_(void, OnNextFrame)(THIS_) OVERRIDE;
  protected:
    void OnPaint(IRenderTarget *pRT);
    void OnDestroy();
    void OnShowWindow(BOOL bShow, UINT nStatus);
    void OnSize(UINT nType, CSize size);
    SOUI_MSG_MAP_BEGIN()
      MSG_WM_PAINT_EX(OnPaint)
      MSG_WM_DESTROY(OnDestroy)
      MSG_WM_SHOWWINDOW(OnShowWindow)
      MSG_WM_SIZE(OnSize)
    SOUI_MSG_MAP_END()

  private:
    void StartAnimation();
    void StopAnimation();

    COLORREF m_crBorder;
    COLORREF m_crBackground;
    COLORREF m_crForeground;
    COLORREF m_crWave;
    float m_fWaveSpeed;
    float m_fWaveHeight;
    int m_nWaveCount;
    
    int m_nOffset;
    SAutoRefPtr<IPathS> m_path;
};

SNSEND
#endif//SWAVEPROGRESS_H