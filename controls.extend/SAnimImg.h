/**
* Copyright (C) 2014-2050 SOUI团队
* All rights reserved.
* 
* @file       SAnimImg.h
* @brief      
* @version    v1.0      
* @author     水木之     
* @date       2014-07-06
* 
* Describe    动作图片
*/
#pragma once
#include <core/SWnd.h>

namespace SOUI
{
 
    class SAnimImg : public SWindow , protected ITimelineHandler
    {
        DEF_SOBJECT(SWindow, L"anmimg")
	public:
		SAnimImg(void);
		~SAnimImg(void);
    protected:
        virtual void WINAPI OnNextFrame() override;
        
		virtual SIZE MeasureContent(int nParentWid, int nParentHei) override;
        
        void NextImage();
        
        void OnDestroy();
		int  OnCreate(void*);
        void OnPaint(IRenderTarget *pRT);
        void OnMouseHover(WPARAM wParam, CPoint ptPos);
        void OnMouseLeave();
        
        SOUI_MSG_MAP_BEGIN()
			MSG_WM_CREATE(OnCreate)
            MSG_WM_DESTROY(OnDestroy)
			MSG_WM_PAINT_EX(OnPaint)   
			MSG_WM_MOUSEHOVER(OnMouseHover)
			MSG_WM_MOUSELEAVE(OnMouseLeave)
         SOUI_MSG_MAP_END()
 
        SOUI_ATTRS_BEGIN()
			ATTR_INT(L"interval",m_iInterval,FALSE)
			ATTR_INT(L"autorun",m_iAutorun,FALSE)
			ATTR_SKIN(L"ani_in",m_pAniIn,TRUE)
			ATTR_SKIN(L"ani_out",m_pAniOut,TRUE)
		SOUI_ATTRS_END()
	public:
		int m_iInterval;//动画间隔 单位毫秒
		int m_iAutorun;//自动运行模式
		//-1:默认，不自动运行
		//0:连续动画（使用ani_in）
		//大于0：间隔动画的暂停时间，单位秒
		int m_iAutorun1000;
		bool m_bWaitrun;
		int m_iHitTest;//记录鼠标位置（大于0表示鼠标还在里面）
		enum ANI_STATUS{
		STATUS_NOR=0,
		STATUS_IN,
		STATUS_OUT,
		} m_aniStatus;
		
		int          m_iFrame;
		
		int          m_nElapseTime;
		int m_nAutoElapseTime;
		ISkinObj    *m_pAniIn;
		ISkinObj    *m_pAniOut;
    };

}//namespace SOUI