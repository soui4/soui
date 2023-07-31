#ifndef __SSTACKVIEW__H__
#define __SSTACKVIEW__H__
#include <valueAnimator/SValueAnimator.h>

SNSBEGIN

class SStackView;
class SOUI_EXP SViewSwitchAnimator : public TValueAnimator<float> {
    DEF_SOBJECT(TValueAnimator<float>, L"ViewSwitchAnimator")
  public:
    SViewSwitchAnimator(SStackView *pOwner)
        : TValueAnimator<float>(0.f, 1.f)
        , m_pOwner(pOwner)
        , m_iFrom(-1)
        , m_iTo(-1)
        , m_isOwnerResize(FALSE)
    {
    }

    STDMETHOD_(IValueAnimator *, clone)(THIS) SCONST OVERRIDE
    {
        return NULL;
    }

    STDMETHOD_(void, onEvaluateValue)(THIS_ float fraction) OVERRIDE;

    STDMETHOD_(void, start)(THIS_ ITimelineHandlersMgr *pTimerlineMgr) OVERRIDE;

    void SetFrom(int iChild)
    {
        m_iFrom = iChild;
    }

    int GetFrom() const
    {
        return m_iFrom;
    }

    void SetTo(int iChild)
    {
        m_iTo = iChild;
    }

    int GetTo() const
    {
        return m_iTo;
    }

    void OnOwnerResize();

    void SetOwnerSizeRange(SIZE szFrom, SIZE szTo);

    BOOL IsOwnerResize() const
    {
        return m_isOwnerResize;
    }
    void DisableOwnerResize()
    {
        m_isOwnerResize = FALSE;
    }

    SIZE GetOwnerSize() const;

    StackViewAniStyle GetAniStyle() const
    {
        return m_aniStyle;
    }

  protected:
    void initEvaluator();

    SStackView *m_pOwner;
    int m_iFrom, m_iTo;
    BOOL m_bVertAni;
    StackViewAniStyle m_aniStyle;

    SAutoRefPtr<IWindow> m_pFrom;
    SAutoRefPtr<IWindow> m_pTo;

    TypeEvaluator<RECT> m_evalRcFrom, m_evalRcTo;
    TypeEvaluator<BYTE> m_evalAlphaFrom, m_evalAlphaTo;
    TypeEvaluator<SIZE> m_evalOwnerSize;
    BOOL m_isOwnerResize;
};

class SOUI_EXP SStackPage : public SWindow {
    DEF_SOBJECT(SWindow, L"stackPage")
  public:
    SStackPage()
        : m_bVertAni(FALSE)
        , m_aniStyle(kAniNone)
    {
    }

  public:
    StackViewAniStyle GetAnimateStyle() const
    {
        return m_aniStyle;
    }

    BOOL IsVertAnimate() const
    {
        return m_bVertAni;
    }

  public:
    SOUI_ATTRS_BEGIN()
        ATTR_BOOL(L"vertical", m_bVertAni, FALSE)
        ATTR_ENUM_BEGIN(L"aniType", StackViewAniStyle, FALSE)
            ATTR_ENUM_VALUE(L"none", kAniNone)
            ATTR_ENUM_VALUE(L"fade", kFadeInOut)
            ATTR_ENUM_VALUE(L"move", kMoveInOut)
            ATTR_ENUM_VALUE(L"push", kPushInOut)
        ATTR_ENUM_END(m_aniStyle)
    SOUI_ATTRS_END()
  protected:
    BOOL m_bVertAni;
    StackViewAniStyle m_aniStyle;
};

class SOUI_EXP SStackView
    : public TWindowProxy<IStackView>
    , IAnimatorListener {
    friend class SViewSwitchAnimator;
    DEF_SOBJECT(SWindow, L"stack")
  public:
    SStackView(void);
    ~SStackView(void);

  public:
    STDMETHOD_(BOOL, SelectPage)(THIS_ int iView, BOOL enableAnimate DEF_VAL(TRUE)) OVERRIDE;
    STDMETHOD_(void, SetAniStyle)(THIS_ StackViewAniStyle aniStyle) OVERRIDE;
    STDMETHOD_(void, SetAniDir)(THIS_ BOOL bVert) OVERRIDE;
    STDMETHOD_(IWindow *, GetSelPage)(CTHIS) SCONST OVERRIDE;
    STDMETHOD_(IWindow *, GetPage)(CTHIS_ int iPage) SCONST OVERRIDE;

  public:
    STDMETHOD_(IWindow *, GetIChild)(THIS_ int iChild) SCONST OVERRIDE;
    STDMETHOD_(void, UpdateChildrenPosition)(THIS) OVERRIDE;

  protected:
    STDMETHOD_(void, onAnimationStart)(THIS_ IValueAnimator *pAnimator) OVERRIDE
    {
    }
    STDMETHOD_(void, onAnimationRepeat)(THIS_ IValueAnimator *pAnimator) OVERRIDE
    {
    }
    STDMETHOD_(void, onAnimationEnd)(THIS_ IValueAnimator *pAnimator) OVERRIDE;

  public:
    SOUI_ATTRS_BEGIN()
        ATTR_INT(L"curSel", m_iSel, FALSE)
        ATTR_BOOL(L"vertical", m_bVertAni, FALSE)
        ATTR_ENUM_BEGIN(L"aniType", StackViewAniStyle, FALSE)
            ATTR_ENUM_VALUE(L"none", kAniNone)
            ATTR_ENUM_VALUE(L"fade", kFadeInOut)
            ATTR_ENUM_VALUE(L"move", kMoveInOut)
            ATTR_ENUM_VALUE(L"push", kPushInOut)
        ATTR_ENUM_END(m_aniStyle)
        ATTR_BOOL(L"samePageSize", m_isSamePageSize, FALSE)
        ATTR_CHAIN_CLASS(__baseCls)
        ATTR_CHAIN(m_animator, 0) // chain attributes to animator
    SOUI_ATTRS_BREAK()

  protected:
    void OnDestroy();
    void OnSize(UINT nType, CSize size);
    SOUI_MSG_MAP_BEGIN()
        MSG_WM_DESTROY(OnDestroy)
        MSG_WM_SIZE(OnSize)
    SOUI_MSG_MAP_END()
  protected:
    BOOL CreateChildren(SXmlNode xmlNode) override;
    SIZE MeasureChildren(int nParentWid, int nParentHei) override;

    void OnContainerChanged(ISwndContainer *pOldContainer, ISwndContainer *pNewContainer) override;

    void OnAfterInsertChild(SWindow *pChild) override;
    void OnAfterRemoveChild(SWindow *pChild) override;

  protected:
    void BuildChildsArray(BOOL updateSel);
    StackViewAniStyle GetChildAnimateStyle(int iChild) const;
    BOOL IsVertChildAnimate(int iChild) const;
    CSize GetChildSize(IWindow *pPage) const;

  protected:
    int m_iSel;
    SViewSwitchAnimator m_animator;
    SArray<SWindow *> m_childs;

    BOOL m_bVertAni;
    StackViewAniStyle m_aniStyle;
    BOOL m_isSamePageSize;
};

SNSEND

#endif // __SSTACKVIEW__H__