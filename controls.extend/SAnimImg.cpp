#include "stdafx.h"
#include "souistd.h"
#include "SAnimImg.h"
 
namespace SOUI
{
	SAnimImg::SAnimImg(void):
		m_pAniIn(NULL),
		m_pAniOut(NULL),
		m_iInterval(50),
		m_aniStatus(STATUS_NOR),
		m_iFrame(0),
		m_nElapseTime(0),
		m_iAutorun(-1),
		m_bWaitrun(FALSE),
		m_iHitTest(0),
		m_nAutoElapseTime(0)
	{
		m_style.m_bTrackMouseEvent = true;//
	}

	SAnimImg::~SAnimImg(void)
	{
	}
 
	void SAnimImg::OnPaint(IRenderTarget *pRT)
	{
	    CRect rcSkin = GetClientRect();
	    if(m_aniStatus == STATUS_IN && m_pAniIn)
	    {
	        m_pAniIn->DrawByIndex(pRT,rcSkin,m_iFrame);
	    }else if(m_aniStatus == STATUS_OUT && m_pAniOut)
	    {
	        m_pAniOut->DrawByIndex(pRT,rcSkin,m_iFrame);
	    }
		else if (m_pAniIn)
		{//默认显示in动画的第一幅
			m_pAniIn->DrawByIndex(pRT,rcSkin,0);
		}
	}
 
 
	void SAnimImg::OnMouseHover(WPARAM wParam, CPoint ptPos)
	{
		m_iHitTest++;
	    m_aniStatus = STATUS_IN;
	    m_iFrame = 0;
	    m_nElapseTime = 0;
		m_nAutoElapseTime = 0;
	    Invalidate();
	    
		__baseCls::OnMouseHover(wParam,ptPos);
	}

	void SAnimImg::OnMouseLeave()
	{
		m_iHitTest--;//记录鼠标位置，为0表示已经移出
		__baseCls::OnMouseLeave();
	}

	int SAnimImg::OnCreate(void*)
	{
	    __baseCls::OnCreate(NULL);
        GetContainer()->RegisterTimelineHandler(this);		
		if (m_iAutorun >= 0)
		{
			m_bWaitrun = true;
			m_iAutorun1000 = m_iAutorun * 1000;//避免反复*1000
		}
		
		return 0;
	}


    void SAnimImg::OnDestroy()
    {
        GetContainer()->UnregisterTimelineHandler(this);
        __baseCls::OnDestroy();
    }


    void SAnimImg::OnNextFrame()
    {
		if (m_aniStatus > 0)
		{//in和out状态才计数
			m_nElapseTime+=10;
			if(m_nElapseTime>=m_iInterval)
			{
				NextImage();
				m_nElapseTime = 0;    
			}
		}
		if (m_bWaitrun)
		{//自动播放模式
			m_nAutoElapseTime+=10;
			if (m_nAutoElapseTime >= m_iAutorun1000)
			{
				m_nAutoElapseTime = 0;
				m_iFrame = 0;
				m_bWaitrun = false;
				m_aniStatus = STATUS_IN;
			}
		}
    }

    void SAnimImg::NextImage()
    {
		switch (m_aniStatus)
		{
		case STATUS_IN:
			{
				if (!m_pAniIn)
					break;
				if(++m_iFrame >= m_pAniIn->GetStates()-1)
				{
					switch (m_iAutorun)
					{
					case -1:
						m_iFrame = m_pAniIn->GetStates() - 1;
						if (m_iHitTest <= 0)//应该不会小于0吧
						{//非自动播放模式，考虑鼠标已经移出，播放移出动画
							if (m_pAniOut)
							{//设置了out皮肤才起作用
								m_aniStatus = STATUS_OUT;
								m_iFrame = 0;
								m_nElapseTime = 0;
								Invalidate();
							}
						}
						break;
					case 0:
						m_iFrame = 0;//自动播放模式，直接跳到0帧
					default:
						m_aniStatus = STATUS_NOR;//自动间隔播放模式，切换到停止状态
						m_bWaitrun = true;
					}
				}
				Invalidate();
 			}
			break;
		case STATUS_OUT:
			{
				if (m_pAniOut)//防止没有设置ani_out而出错
				{
					if(m_iFrame< m_pAniOut->GetStates()-1)
					{
						m_iFrame++;
						Invalidate();
					}
					else
					{//鼠标已经移出，还原到正常状态
						m_aniStatus = STATUS_NOR;
					}
				}
			}
			break;
		default:
			break;
		}
     }

    SIZE SAnimImg::MeasureContent(int nWid,int nHei)
    {
		CSize szRet;
		if (m_pAniIn)
		{
			szRet = m_pAniIn->GetSkinSize();;
		}
		return szRet;
    }

}
 