#include "stdafx.h"
#include "SGifPlayer.h"
#include "SSkinAni.h"

SNSBEGIN

SGifPlayer::SGifPlayer() 
	: m_iCurFrame(0)
	, m_nNextInterval(0)
	, m_bEnableScale(TRUE)
	, m_nScale(100)
	, m_nLoop(-1)
	, m_iLoop(0)
	, m_bTile(TRUE)
	, m_bAutoPlay(TRUE)
	, m_bPlaying(FALSE)
{
	AddEvent(EVENTID(EventGifLoopOver));
	AddEvent(EVENTID(EventGifPlayOver));
	AddEvent(EVENTID(EventGifPlayFrame));
}

SGifPlayer::~SGifPlayer()
{
}


void SGifPlayer::OnPaint( IRenderTarget *pRT )
{	
	__baseCls::OnPaint(pRT);
	if (m_aniSkin)
	{		
		m_aniSkin->DrawByIndex(pRT, GetWindowRect(),m_iCurFrame);
	}
}

void SGifPlayer::OnShowWindow( BOOL bShow, UINT nStatus )
{
	__baseCls::OnShowWindow(bShow,nStatus);
	if(!IsVisible(TRUE))
	{
		Stop();
	}else if(m_aniSkin && m_aniSkin->GetStates()>1)
	{
		if(m_bAutoPlay)
			Start();
	}
}

void SGifPlayer::OnNextFrame()
{
    m_nNextInterval -= ITimelineHandler::kPulseInterval;
    if(m_nNextInterval <= 0 && m_aniSkin)
    {
        int nStates=m_aniSkin->GetStates();
		if(nStates==0)
			return;
        m_iCurFrame++;
		m_iCurFrame%=nStates;

		EventGifPlayFrame evt(this);
		evt.iFrame = m_iCurFrame;
		FireEvent(evt);
		
		Invalidate();
        if(m_aniSkin->GetFrameDelay(m_iCurFrame)==0)
            m_nNextInterval = 90;
        else
            m_nNextInterval =m_aniSkin->GetFrameDelay(m_iCurFrame);	
		if(m_iCurFrame == 0){
			m_iLoop++;
			EventGifLoopOver evt(this);
			evt.iLoop = m_iLoop;
			evt.nLoop = m_nLoop;
			FireEvent(evt);
			if(m_nLoop>=0 && m_iLoop>=m_nLoop){
				Stop();
				EventGifPlayOver evt(this);
				FireEvent(evt);
			}
		}
    }
}

HRESULT SGifPlayer::OnAttrSkin( const SStringW & strValue, BOOL bLoading )
{
	ISkinObj *pSkin = GETSKIN(strValue,GetScale());
	if(!pSkin) return E_FAIL;
	if(!pSkin->IsClass(SSkinAni::GetClassName())) return S_FALSE;
	m_aniSkin=static_cast<SSkinAni*>(pSkin);
    if(!bLoading)
    {
        m_iCurFrame = 0;
        if(m_aniSkin->GetFrameDelay()==0)
            m_nNextInterval = 90;
        else
            m_nNextInterval =m_aniSkin->GetFrameDelay()*10;	
    }
	return bLoading?S_OK:S_FALSE;
}

void SGifPlayer::SetLoop(int  nLoop)
{
	m_nLoop = nLoop;
	m_iLoop = 0;
}

int SGifPlayer::GetFrameCount() const
{
    return m_aniSkin ? m_aniSkin->GetStates() : 0;
}

void SGifPlayer::SetCurFrame(int iFrame)
{
	if(iFrame>=0 && iFrame<m_aniSkin->GetStates())
	{
		m_iCurFrame = iFrame;
		Invalidate();
	}	

}

int SGifPlayer::GetCurFrame() const
{
    return m_iCurFrame;
}

SIZE SGifPlayer::MeasureContent(int wid, int hei)
{
	CSize sz;
	if(m_aniSkin) sz=m_aniSkin->GetSkinSize();
	return sz;
}

BOOL SGifPlayer::PlayGifFile( LPCTSTR pszFileName)
{
    return _PlayFile(pszFileName);
}

BOOL SGifPlayer::PlayAPNGFile( LPCTSTR pszFileName)
{
    return _PlayFile(pszFileName);
}

BOOL SGifPlayer::LoadFromMemory(LPVOID pBits, size_t szData)
{
    SSkinAni *pAniSkin = (SSkinAni *)SApplication::getSingleton().CreateSkinByName(SSkinAni::GetClassName());
	if(!pAniSkin) return FALSE;
	if(0==pAniSkin->LoadFromMemory(pBits,szData))
	{
		pAniSkin->Release();
		return FALSE;
	}
	SetAniSkin(pAniSkin);
	pAniSkin->Release();
	return TRUE;
}

void SGifPlayer::Start(int iFrame)
{
	if(iFrame>=0 && iFrame<m_aniSkin->GetStates())
		m_iCurFrame = iFrame;	//设置当前帧
	GetContainer()->RegisterTimelineHandler(this);
    if (m_aniSkin->GetFrameDelay(m_iCurFrame) == 0)
        m_nNextInterval = 90;
    else
        m_nNextInterval = m_aniSkin->GetFrameDelay(m_iCurFrame);
	m_iLoop = 0;
    m_bPlaying = TRUE;	//设置播放状态
}

void SGifPlayer::Stop()
{
	GetContainer()->UnregisterTimelineHandler(this);
	m_bPlaying = FALSE;
}

BOOL SGifPlayer::_PlayFile( LPCTSTR pszFileName)
{
    SSkinAni *pAniSkin = (SSkinAni *)SApplication::getSingleton().CreateSkinByName(SSkinAni::GetClassName());
	if(!pAniSkin) return FALSE;
	if(0==pAniSkin->LoadFromFile(pszFileName))
	{
		pAniSkin->Release();
		return FALSE;
	}
	SetAniSkin(pAniSkin);
	pAniSkin->Release();
	return TRUE;
}

void SGifPlayer::SetAniSkin(SSkinAni *pAniSkin)
{
	if(!pAniSkin)
		return;
	Stop();
	if(pAniSkin->GetScale() != GetScale()){
		m_aniSkin = (SSkinAni *)pAniSkin->Scale(GetScale());
	}else{
		m_aniSkin = pAniSkin;
	}
	m_iCurFrame = 0;
	if(GetLayoutParam()->IsWrapContent(Any))
	{
		RequestRelayout();
	}
	if(IsVisible(TRUE) && m_bAutoPlay)
	{
		Start();
	}
}

void SGifPlayer::OnDestroy()
{
    Stop();
    __baseCls::OnDestroy();
}

void SGifPlayer::OnContainerChanged(ISwndContainer *pOldContainer,ISwndContainer *pNewContainer)
{
	if(pOldContainer && m_bPlaying)
	{
		pOldContainer->UnregisterTimelineHandler(this);
	}
	if(pNewContainer && m_bPlaying){
		pNewContainer->RegisterTimelineHandler(this);
	}
	SWindow::OnContainerChanged(pOldContainer,pNewContainer);
}


void SGifPlayer::OnScaleChanged(int scale)
{
    __baseCls::OnScaleChanged(scale);
	SAutoRefPtr<ISkinObj> pSkin = (ISkinObj *)m_aniSkin;
    GetScaleSkin(pSkin, scale);
	m_aniSkin = (SSkinAni *)(ISkinObj*)pSkin;
}

SNSEND
