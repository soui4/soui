#pragma once

#include <souicoll.h>

SNSBEGIN

class SScrollText
    : public SStatic
    , public ITimelineHandler {
    DEF_SOBJECT(SStatic, L"scrolltext")
  public:
    SScrollText(void);
    ~SScrollText(void);

    void SetWindowText(const SStringT &strText);

  protected:
    STDMETHOD_(void, OnNextFrame)(THIS_) OVERRIDE;
    virtual void OnContainerChanged(ISwndContainer *pOldContainer, ISwndContainer *pNewContainer);

  protected:
    void OnPaint(IRenderTarget *pRT);
    void OnSize(UINT nType, CSize size);
    void OnShowWindow(BOOL bShow, UINT nStatus);
    void OnDestroy();

    void UpdateScrollInfo(CSize size);
    bool IsScrolling() const;

  private:
    void OnNextFrameHoz();
    void OnNextFrameMulti();
    void StepHozReveal();

    SOUI_MSG_MAP_BEGIN()
      MSG_WM_PAINT_EX(OnPaint)
      MSG_WM_SIZE(OnSize)
      MSG_WM_DESTROY(OnDestroy)
      MSG_WM_SHOWWINDOW(OnShowWindow)
    SOUI_MSG_MAP_END()

    SOUI_ATTRS_BEGIN()
    ATTR_INT(L"speed", m_nSpeed, FALSE)
    ATTR_INT(L"rolltype", m_nRollType, FALSE)
    ATTR_INT(L"staytime", m_nStayTime, FALSE)
    SOUI_ATTRS_END()

    int m_nSpeed;
    int m_nOffset;
    int m_nScrollWidth;
    int m_nNextInterval;
    int m_nRollType;      // 0首位衔接 1 非衔接
    int m_nStayTime;      // 多行模式每行停留时间(ms)
    int m_nLineHeight;    // 单行像素高度
    int m_nLineCount;     // 行数
    int m_nCurLine;       // 多行模式当前显示行下标
    int m_nVPos;          // 多行模式垂直过渡位移
    int m_nHOff;          // 多行模式当前行水平展示位移
    int m_nHTick;         // 多行模式水平滚动步进计时
    BOOL m_bMultiline;    // 是否多行(垂直滚动)
    BOOL m_bVDwell;       // 多行模式是否处于停留阶段
    SArray<SStringT> m_vLines; // 按换行符拆分后的各行文本
    SArray<int> m_awLineWidth; // 各行文本宽度
};

SNSEND
