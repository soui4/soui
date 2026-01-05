#include "stdafx.h"
#include "utils.h"
#include <valueAnimator/SPropertyAnimator.h>
SNSBEGIN
//------------------------------------------------------------------------
class CMovingCardMgr : public IAnimatorListener {
    typedef SMap<IWindow *, SAutoRefPtr<IValueAnimator> > MovingCardMap;

  public:
    void StartAnimator(IValueAnimator *ani, IWindow *pTarget)
    {
        EndTargetAnimator(pTarget);
        ani->addListener(this);
        m_mapMovingCards[pTarget] = ani;
        auto slot = Subscriber(&CMovingCardMgr::OnTargetDestroy, this);
        pTarget->SubscribeEvent(EventSwndDestroy::EventID, &slot);
        ani->start(pTarget->GetContainer());
    }

    BOOL EndTargetAnimator(IWindow *pTarget)
    {
        MovingCardMap::CPair *pPair = m_mapMovingCards.Lookup(pTarget);
        if (!pPair)
            return FALSE;
        pPair->m_value->end();
        return TRUE;
    }

 protected:
    BOOL OnTargetDestroy(IEvtArgs *e)
    {
        IWindow *pTarget = (IWindow*)e->Sender();
        m_mapMovingCards.RemoveKey(pTarget);
        return TRUE;
    }
  protected:
    STDMETHOD_(void, onAnimationStart)(THIS_ IValueAnimator *pAnimator) override
    {
    }
    STDMETHOD_(void, onAnimationRepeat)(THIS_ IValueAnimator *pAnimator) override
    {
    }
    STDMETHOD_(void, onAnimationEnd)(THIS_ IValueAnimator *pAnimator) override
    {
        IPropertyAnimator *pPropAnimator = sobj_cast<IPropertyAnimator>(pAnimator);
        SASSERT(pPropAnimator);
        IWindow *pTarget = pPropAnimator->GetTarget();
        m_mapMovingCards.RemoveKey(pTarget);
        auto slot = Subscriber(&CMovingCardMgr::OnTargetDestroy, this);
        pTarget->UnsubscribeEvent(EventSwndDestroy::EventID, &slot);
    }

    MovingCardMap m_mapMovingCards;
};

static CMovingCardMgr s_movingCardMgr;
//------------------------------------------------------------------------
SAutoRefPtr<IValueAnimator> Util::OffsetSprite(IWindow *pSprite, float dx, float dy, int nSpeed)
{
    ILayoutParam *pParam = (ILayoutParam*)pSprite->GetLayoutParam();
    SASSERT(pParam->IsClass(SAnchorLayoutParam::GetClassName()));
    SAnchorLayoutParamStruct * pParamStruct = (SAnchorLayoutParamStruct*)pParam->GetRawData();
    AnchorPos fromPos = pParamStruct->pos;
    AnchorPos toPos = fromPos;
    toPos.x.fSize += dx;
    toPos.y.fSize += dy;
    return MoveSprite(pSprite, fromPos, toPos, nSpeed);
}

void Util::StartAnimator(IValueAnimator *pAnimator,IWindow *pSprite)
{
    s_movingCardMgr.StartAnimator(pAnimator,pSprite);
}

void Util::EndAnimator(IWindow *pSprite)
{
    s_movingCardMgr.EndTargetAnimator(pSprite);
}

SAutoRefPtr<IValueAnimator> Util::MoveSprite(IWindow *pSprite, AnchorPos fromPos, AnchorPos toPos, int nSpeed)
{
    AnchorPos pos[] = {
		fromPos,
		toPos
	};
    SAutoRefPtr<IValueAnimator> pAnimator(SPropertyAnimator::ofPosition(pSprite, LayoutProperty::POSITION, pos, ARRAYSIZE(pos), sizeof(AnchorPos)), FALSE);
    pAnimator->setDuration(nSpeed);
    s_movingCardMgr.StartAnimator(pAnimator,pSprite);
    return pAnimator;
}

SAutoRefPtr<IValueAnimator> Util::MoveSpriteTo(IWindow *pSprite, AnchorPos toPos, int nSpeed)
{
    ILayoutParam *pParam = (ILayoutParam*)pSprite->GetLayoutParam();
    SAnchorLayoutParamStruct * pParamStruct = (SAnchorLayoutParamStruct*)pParam->GetRawData();
    AnchorPos fromPos = pParamStruct->pos;
    return MoveSprite(pSprite, fromPos, toPos, nSpeed);
}

SAutoRefPtr<IValueAnimator> Util::MoveAndHideSprite(IWindow *pSprite, AnchorPos toPos, int nSpeed)
{
    ILayoutParam *pParam = (ILayoutParam*)pSprite->GetLayoutParam();
    SAnchorLayoutParamStruct * pParamStruct = (SAnchorLayoutParamStruct*)pParam->GetRawData();
    AnchorPos fromPos = pParamStruct->pos;
    AnchorPos pos[] = {
		fromPos,
		toPos
	};
    SAutoRefPtr<IPropertyValuesHolder> pPosHolder(SPropertyValuesHolder::ofPosition(LayoutProperty::POSITION, pos, ARRAYSIZE(pos), sizeof(AnchorPos)),FALSE);
    BYTE alpha[] = { 255, 0 };
    SAutoRefPtr<IPropertyValuesHolder> pAlphaHolder(SPropertyValuesHolder::ofByte(WindowProperty::ALPHA, alpha, 2),FALSE);
    IPropertyValuesHolder *holders[] = { pPosHolder, pAlphaHolder };
    SAutoRefPtr<IValueAnimator> pAnimator(SPropertyAnimator::ofPropertyValuesHolder(pSprite, holders, ARRAYSIZE(holders)), FALSE);
    pAnimator->setDuration(nSpeed);
    s_movingCardMgr.StartAnimator(pAnimator, pSprite);
    return pAnimator;
}

SAutoRefPtr<IValueAnimator> Util::ScaleSprite(IWindow *pSprite, float from, float to, int nSpeed){
    float valuex[] = {
		from,
		to
	};
    SAutoRefPtr<IValueAnimator> pAnimator(SPropertyAnimator::ofFloat(pSprite, WindowProperty::SCALE, valuex, ARRAYSIZE(valuex)), FALSE);
    pAnimator->setDuration(nSpeed);
    s_movingCardMgr.StartAnimator(pAnimator, pSprite);
    return pAnimator;
}

SAutoRefPtr<IValueAnimator> Util::ChangeSpriteHeight(IWindow *pSprite, float from, float to, int nSpeed){
    float valuex[] = {
		from,
		to
	};
    SAutoRefPtr<IValueAnimator> pAnimator(SPropertyAnimator::ofFloat(pSprite, LayoutProperty::HEIGHT, valuex, ARRAYSIZE(valuex)), FALSE);
    pAnimator->setDuration(nSpeed);
    s_movingCardMgr.StartAnimator(pAnimator, pSprite);
    return pAnimator;
}

SNSEND