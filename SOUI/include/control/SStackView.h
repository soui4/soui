#pragma once
#include <valueAnimator/SValueAnimator.h>

SNSBEGIN


class SViewSwitchAnimator : public TValueAnimator<float> {
	DEF_SOBJECT(TValueAnimator<float>, L"ViewSwitchAnimator")	
public:
	SViewSwitchAnimator(SWindow * pOwner)
		: TValueAnimator<float>(0.f, 1.f),m_pOwner(pOwner),m_aniType(kAniNone),m_iFrom(-1),m_iTo(-1),m_bVertAni(FALSE)
	{
	}

	STDMETHOD_(IValueAnimator *, clone)(THIS) SCONST OVERRIDE
	{
		return NULL;
	}

	STDMETHOD_(void, onEvaluateValue)(THIS_ float fraction) OVERRIDE;

	STDMETHOD_(void, start)(THIS_ ITimelineHandlersMgr *pTimerlineMgr) OVERRIDE;

	void SetFrom(int iChild){
		m_iFrom = iChild;
	}

	int GetFrom() const{return m_iFrom;}

	SWindow * GetFromWindow() const{
		return m_pFrom;
	}

	void SetTo(int iChild){
		m_iTo = iChild;
	}

	int GetTo() const{ return m_iTo;}

	SWindow * GetToWindow() const{
		return m_pTo;
	}

	void SetAniStyle(StackViewAniStyle aniStyle);
	void SetAniDir(BOOL bVert);	

	void OnOwnerResize();
protected:
	void initEvaluator();

	SWindow * m_pOwner;
	int m_iFrom,m_iTo;
	SAutoRefPtr<SWindow> m_pFrom;
	SAutoRefPtr<SWindow> m_pTo;
	StackViewAniStyle m_aniType;

	TypeEvaluator<RECT> m_evalRcFrom,m_evalRcTo;
	TypeEvaluator<BYTE> m_evalAlphaFrom,m_evalAlphaTo;
	BOOL m_bVertAni;

	SOUI_ATTRS_BEGIN()
		ATTR_BOOL(L"vertical",m_bVertAni,FALSE)
		ATTR_ENUM_BEGIN(L"aniType",StackViewAniStyle,FALSE)
			ATTR_ENUM_VALUE(L"none",kAniNone)
			ATTR_ENUM_VALUE(L"fade",kFadeInOut)
			ATTR_ENUM_VALUE(L"move",kMoveInOut)
			ATTR_ENUM_VALUE(L"push",kPushInOut)
		ATTR_ENUM_END(m_aniType)
	SOUI_ATTRS_END()
};

class SOUI_EXP SStackView : public TWindowProxy<IStackView>, IAnimatorListener
{
	DEF_SOBJECT(SWindow,L"stack")
public:
	SStackView(void);
	~SStackView(void);

	STDMETHOD_(BOOL,SelectView)(THIS_ int iView,BOOL enableAnimate DEF_VAL(TRUE)) OVERRIDE;
	STDMETHOD_(void,SetAniStyle)(THIS_ StackViewAniStyle aniStyle) OVERRIDE;
	STDMETHOD_(void,SetAniDir)(THIS_ BOOL bVert) OVERRIDE;	
	STDMETHOD_(IWindow*,GetCurView)(CTHIS) SCONST OVERRIDE;
protected:
	STDMETHOD_(void, onAnimationStart)(THIS_ IValueAnimator * pAnimator) OVERRIDE{}
	STDMETHOD_(void, onAnimationRepeat)(THIS_ IValueAnimator * pAnimator) OVERRIDE{}
	STDMETHOD_(void, onAnimationEnd)(THIS_ IValueAnimator * pAnimator) OVERRIDE;
public:
	SOUI_ATTRS_BEGIN()
		ATTR_INT(L"curSel",m_iSel,FALSE)
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
	STDMETHOD_(void, OnInitFinished)(THIS_ IXmlNode * xmlNode) OVERRIDE;
	STDMETHOD_(void, UpdateChildrenPosition)(THIS) OVERRIDE;

	void OnContainerChanged(ISwndContainer *pOldContainer, ISwndContainer *pNewContainer) override;
protected:
	int m_iSel;
	SViewSwitchAnimator m_animator;
};

SNSEND
