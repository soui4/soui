#include "stdafx.h"
#include "SGifPlayer.h"
#include "SSkinAni.h"

namespace SOUI
{

SGifPlayer::SGifPlayer() 
	: m_iCurFrame(0)
	, m_nNextInterval(0)
	, m_bEnableScale(TRUE)
	, m_nScale(100)
	, m_bLoop(TRUE)
	, m_bTile(TRUE)
{

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
	if(!bShow)
	{
        GetContainer()->UnregisterTimelineHandler(this);
	}else if(m_aniSkin && m_aniSkin->GetStates()>1)
	{
        GetContainer()->RegisterTimelineHandler(this);
        if(m_aniSkin->GetFrameDelay()==0)
            m_nNextInterval = 90;
        else
            m_nNextInterval = m_aniSkin->GetFrameDelay();
	}
}

void SGifPlayer::OnNextFrame()
{
    if (!IsVisible(TRUE))
        return;
    m_nNextInterval -= 10;
    if(m_nNextInterval <= 0 && m_aniSkin)
    {
        int nStates=m_aniSkin->GetStates();
		if(nStates==0)
			return;
        m_iCurFrame++;
		if (!m_bLoop && m_iCurFrame >= nStates)
    	{
			GetContainer()->UnregisterTimelineHandler(this);
			EventGifPlayOver evt(this);
			FireEvent(evt);
			return;
    	}
        m_iCurFrame%=nStates;
        Invalidate();
        if(m_aniSkin->GetFrameDelay()==0)
            m_nNextInterval = 90;
        else
            m_nNextInterval =m_aniSkin->GetFrameDelay();	
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

SIZE SGifPlayer::MeasureContent(int wid,int hei )
{
	CSize sz;
	if(m_aniSkin) sz=m_aniSkin->GetSkinSize();
	return sz;
}

BOOL SGifPlayer::PlayGifFile( LPCTSTR pszFileName )
{
    return _PlayFile(pszFileName,TRUE);
}

BOOL SGifPlayer::PlayAPNGFile( LPCTSTR pszFileName )
{
    return _PlayFile(pszFileName,FALSE);
}

BOOL SGifPlayer::_PlayFile( LPCTSTR pszFileName, BOOL bGif )
{
    SSkinAni *pAniSkin = (SSkinAni *)SApplication::getSingleton().CreateSkinByName(SSkinAni::GetClassName());
	if(!pAniSkin) return FALSE;
	if(0==pAniSkin->LoadFromFile(pszFileName))
	{
		pAniSkin->Release();
		return FALSE;
	}

	GetContainer()->UnregisterTimelineHandler(this);

	m_aniSkin = pAniSkin;
	pAniSkin->Release();

	m_iCurFrame = 0;
    SStringW str;
    str.Format(L"%d", m_bEnableScale);
    m_aniSkin->SetAttribute(L"enableScale", str);
    str.Format(L"%d", m_nScale);
    m_aniSkin->SetAttribute(L"scale", str);
    str.Format(L"%d", m_bTile);
    m_aniSkin->SetAttribute(L"tile", str);
	
	if(GetLayoutParam()->IsWrapContent(Any))
	{
		RequestRelayout();
	}
	if(IsVisible(TRUE))
	{
		GetContainer()->RegisterTimelineHandler(this);
	}
	return TRUE;
}
	
void SGifPlayer::OnDestroy()
{
    GetContainer()->UnregisterTimelineHandler(this);
    __baseCls::OnDestroy();
}

void SGifPlayer::OnContainerChanged(ISwndContainer *pOldContainer,ISwndContainer *pNewContainer)
{
	if(pOldContainer)
		pOldContainer->UnregisterTimelineHandler(this);
	if(pNewContainer)
		pNewContainer->RegisterTimelineHandler(this);
	SWindow::OnContainerChanged(pOldContainer,pNewContainer);
}

}
