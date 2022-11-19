#pragma once
SNSBEGIN

struct IScrollBarHost
{
    enum kSbConst
    {
        Timer_Wait = 100,
        Timer_Go = 101,
        kTime_Wait = 200,
        kTime_Go = 100,
    };
    STDMETHOD_(CRect, GetScrollBarRect)(CTHIS_ BOOL bVert) SCONST PURE;
    STDMETHOD_(ISkinObj*,GetScrollBarSkin)(CTHIS_ BOOL bVert) SCONST PURE;
    STDMETHOD_(const SCROLLINFO *,GetScrollBarInfo)(CTHIS_ BOOL bVert) SCONST PURE;
    STDMETHOD_(int,GetScrollBarArrowSize)(CTHIS_ BOOL bVert) SCONST PURE;
    STDMETHOD_(void,OnScrollUpdatePart)(THIS_ BOOL bVert, int iPart) PURE;
    STDMETHOD_(void,OnScrollUpdateThumbTrack)(THIS_ BOOL bVert, int nPos) PURE;
    STDMETHOD_(ISwndContainer*, GetScrollBarContainer)(THIS) PURE;
    STDMETHOD_(BOOL, IsScrollBarEnable)(THIS_ BOOL bVertical) SCONST PURE;
    STDMETHOD_(void,OnScrollCommand)(THIS_ BOOL bVert, int iCmd, int nPos) PURE;
    STDMETHOD_(void,OnScrollSetTimer)(THIS_ BOOL bVert, char id, UINT uElapse) PURE;
    STDMETHOD_(void,OnScrollKillTimer)(THIS_ BOOL bVert, char id) PURE;
    STDMETHOD_(const IInterpolator *,GetScrollInterpolator)(CTHIS) SCONST PURE;
    STDMETHOD_(int,GetScrollFadeFrames)(CTHIS) SCONST PURE;
    STDMETHOD_(BYTE,GetScrollThumbTrackMinAlpha)(CTHIS) SCONST PURE;
};

class SOUI_EXP SScrollBarHandler : ITimelineHandler {
  private:
    SScrollBarHandler(const SScrollBarHandler &);
    const SScrollBarHandler &operator=(const SScrollBarHandler &);

  public:
    enum
    {
        kSbRail = 100,
    };

    SScrollBarHandler(IScrollBarHost *pCB, bool bVert = false);

  public:
    CRect GetPartRect(int iPart) const;

    int GetHitPart() const;

    int GetClickPart() const;

    bool IsVertical() const;

    void SetVertical(bool bVert);

    void OnMouseHover(CPoint pt);

    bool OnMouseDown(CPoint pt);

    void OnMouseUp(CPoint pt);

    void OnMouseMove(CPoint pt);

    void OnMouseLeave();

    void OnDraw(IRenderTarget *pRT, int iPart) const;

    void OnTimer(char id);

    void OnDestroy();

    int HitTest(CPoint pt) const;

    void OnContainerChanged(ISwndContainer *pOldContainer, ISwndContainer *pNewContainer);

  protected:
    ISwndContainer *GetContainer();

    const IInterpolator *GetInterpolator() const;

    BYTE GetAlpha(int iPart) const;

    int GetFadeStep() const;

    DWORD GetPartState(int iPart) const;

  protected:
    STDMETHOD_(void, OnNextFrame)(THIS_) OVERRIDE;

  private:
    enum FADEMODE
    {
        FADEOUT = -1,
        FADE_STOP = 0,
        FADEIN = 1,
    };

    IScrollBarHost *m_pSbHost;
    bool m_bVert;
    int m_iFrame;
    FADEMODE m_fadeMode;
    int m_iHitPart;
    int m_iClickPart;
    CPoint m_ptClick;
    CPoint m_ptCursor;
    int m_nClickPos;
};

SNSEND
