#include "stdafx.h"
#include "SSpriteCard.h"
#include <layout/SAnchorLayout.h>
#include <valueAnimator/SPropertyAnimator.h>

SNSBEGIN


SCard::SCard()
{
    m_nStates = 54+1;//54张牌+背面， 13×4+2+1
}

SCard::~SCard()
{
}

SIZE SCard::GetSkinSize() const
{
    SIZE szRet = m_szOrig;
    szRet.cx /= 13;
    szRet.cy /= 5;
    szRet.cx = szRet.cx * GetScale()/100;
    szRet.cy = szRet.cy * GetScale()/100;
    return szRet;
}

void SCard::_DrawByIndex(IRenderTarget *pRT, LPCRECT prcDraw, int iState, BYTE byAlpha) const
{
    if (iState >= m_nStates || iState < 0)
        return;
    int iLine = iState / 13;
    int iCol = iState % 13;
    float wid = m_szOrig.cx;
    float hei = m_szOrig.cy;
    wid = wid * GetScale()/100/13;
    hei = hei * GetScale()/100/5;
    float left = iCol * wid;
    float top = iLine * hei;
    float right = left + wid;
    float bottom = top + hei;
    CRect rcSrc(sround(left),sround(top),sround(right),sround(bottom));
    pRT->DrawBitmapEx(prcDraw, GetImage(), &rcSrc, GetExpandMode(), byAlpha);
}

void SCard::_Scale(ISkinObj *skinObj, int nScale)
{
    SSkinObjBase::_Scale(skinObj, nScale);
    SCard *pRet = sobj_cast<SCard>(skinObj);
    pRet->m_nStates = m_nStates;
    pRet->m_bTile = m_bTile;
    pRet->m_bVertical = m_bVertical;
    pRet->m_filterLevel = m_filterLevel;
    pRet->m_bAutoFit = m_bAutoFit;
    pRet->m_state2Index = m_state2Index;
    pRet->m_bLazyLoad = FALSE;

    IBitmapS *pImg = GetImage();
    CSize szSkin=pImg->Size();
    szSkin.cx = MulDiv(szSkin.cx, nScale, GetScale());
    szSkin.cy = MulDiv(szSkin.cy, nScale, GetScale());
    pImg->Scale2(&pRet->m_pImg, szSkin.cx, szSkin.cy, kHigh_FilterLevel);
    pRet->m_szOrig = m_szOrig;
    //pImg->Save2(L"d:/test_scaled_card.png", Img_PNG);
}

HRESULT SCard::OnAttrSrc(const SStringW &value, BOOL bLoading)
{
    m_pImg.Attach(LOADIMAGE2(value));
    m_szOrig = m_pImg->Size();
    return S_OK;
}
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
        SPropertyAnimator *pPropAnimator = sobj_cast<SPropertyAnimator>(pAnimator);
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
SSpriteCard::SSpriteCard():m_bSelected(FALSE),m_bDraging(FALSE),m_bSelectable(FALSE)
{
    m_pLayoutParam.Attach(new SAnchorLayoutParam());
    AddEvent(EVENTID(EventSpriteCardDragStart));
    AddEvent(EVENTID(EventSpriteCardDrag));
    AddEvent(EVENTID(EventSpriteCardDragEnd));
}

SSpriteCard::~SSpriteCard()
{
}

void SSpriteCard::SetCard(int nCard)
{
    SetUserData(nCard);
    if(nCard>=0 && nCard<54)
        SetIcon(nCard);
    else
        SetIcon(54);
}

int SSpriteCard::GetCard() const
{
    return GetUserData();
}
void SSpriteCard::InvertSelect()
{
    SetSelected(!m_bSelected);
}

BOOL SSpriteCard::IsSelected() const
{
    return m_bSelected;
}

void SSpriteCard::ClearSelect()
{
    m_bSelected = FALSE;
    Invalidate();
}
void SSpriteCard::SetSelected(BOOL bSelected)
{
    if (!m_bSelectable)
        return;
    if (m_bSelected == bSelected)
        return;
    s_movingCardMgr.EndTargetAnimator(this);
    m_bSelected = bSelected;
    Util::OffsetSprite(this, 0, bSelected?-kSelectOffset:kSelectOffset, SPEED_POSTCARD/2);
}

void SSpriteCard::SetMaskSkin(ISkinObj *pSkin)
{
    m_skinSel = pSkin;
    Invalidate();
}

void SSpriteCard::SetSelectable(BOOL bSelectable)
{
    m_bSelectable = bSelectable;
    m_bMsgTransparent = !bSelectable;
    Invalidate();
}

void SSpriteCard::OnFinalRelease()
{
    delete this;
}

void SSpriteCard::OnPaint(IRenderTarget *pRT)
{
    __baseCls::OnPaint(pRT);
    if(m_skinSel && m_bSelected)
    {
        CRect rc = GetClientRect();
        m_skinSel->DrawByIndex(pRT,rc,0);
    }
}

void SSpriteCard::OnLButtonDown(UINT nFlags, CPoint pt)
{
    if(GetUserData()!=-1 && m_bSelectable){
        m_bDraging = TRUE;
        m_ptClick = pt;
        SetCapture();
        EventSpriteCardDragStart evt(this);
        evt.rcDraging = CRect(m_ptClick, pt);
        FireEvent(evt);
    }
}

void SSpriteCard::OnLButtonUp(UINT nFlags, CPoint pt)
{
    if(GetUserData()!=-1 && m_bSelectable){
        m_bDraging = FALSE;
        ReleaseCapture();
        CRect rc = CRect(m_ptClick, pt);
        EventSpriteCardDragEnd evt(this);
        rc.NormalizeRect();
        evt.rcDraging = rc;
        FireEvent(evt);
        if(rc.Width() < kMinDragDistance){//拖动的距离超过2像素，则认为是拖动，否则认为是点击。
            InvertSelect();          
        }    
    }
}

void SSpriteCard::OnMouseMove(UINT nFlags, CPoint pt)
{
    if(m_bDraging){
        EventSpriteCardDrag evt(this);
        evt.rcDraging = CRect(m_ptClick, pt);
        evt.rcDraging.NormalizeRect();
        FireEvent(evt);
    }
}

//------------------------------------------------------------------------
SAutoRefPtr<IValueAnimator> Util::OffsetSprite(IWindow *pCard, int dx, int dy, int nSpeed)
{
    ILayoutParam *pParam = (ILayoutParam*)pCard->GetLayoutParam();
    SASSERT(pParam->IsClass(SAnchorLayoutParam::GetClassName()));
    SAnchorLayoutParamStruct * pParamStruct = (SAnchorLayoutParamStruct*)pParam->GetRawData();
    AnchorPos fromPos = pParamStruct->pos;
    AnchorPos toPos = fromPos;
    toPos.x.fSize += dx;
    toPos.y.fSize += dy;
    return MoveSprite(pCard, fromPos, toPos, nSpeed);
}

void Util::StartAnimator(IValueAnimator *pAnimator,IWindow *pCard)
{
    s_movingCardMgr.StartAnimator(pAnimator,pCard);
}

void Util::EndAnimator(IWindow *pCard)
{
    s_movingCardMgr.EndTargetAnimator(pCard);
}

SAutoRefPtr<IValueAnimator> Util::MoveSprite(IWindow *pCard, AnchorPos fromPos, AnchorPos toPos, int nSpeed)
{
    AnchorPos pos[] = {
		fromPos,
		toPos
	};
    SAutoRefPtr<IValueAnimator> pAnimator(SPropertyAnimator::ofPosition(pCard, LayoutProperty::POSITION, pos, ARRAYSIZE(pos), sizeof(AnchorPos)), FALSE);
    pAnimator->setDuration(nSpeed);
    s_movingCardMgr.StartAnimator(pAnimator,pCard);
    return pAnimator;
}

SAutoRefPtr<IValueAnimator> Util::MoveSpriteTo(IWindow *pCard, AnchorPos toPos, int nSpeed)
{
    ILayoutParam *pParam = (ILayoutParam*)pCard->GetLayoutParam();
    SAnchorLayoutParamStruct * pParamStruct = (SAnchorLayoutParamStruct*)pParam->GetRawData();
    AnchorPos fromPos = pParamStruct->pos;
    return MoveSprite(pCard, fromPos, toPos, nSpeed);
}

SAutoRefPtr<IValueAnimator> Util::MoveAndHideSprite(IWindow *pCard, AnchorPos toPos, int nSpeed)
{
    ILayoutParam *pParam = (ILayoutParam*)pCard->GetLayoutParam();
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
    SAutoRefPtr<IValueAnimator> pAnimator(SPropertyAnimator::ofPropertyValuesHolder(pCard, holders, ARRAYSIZE(holders)), FALSE);
    pAnimator->setDuration(nSpeed);
    s_movingCardMgr.StartAnimator(pAnimator, pCard);
    return pAnimator;
}
SNSEND
