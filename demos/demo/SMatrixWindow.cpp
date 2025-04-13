﻿#include "stdafx.h"
#include "SMatrixWindow.h"
#include <matrix/SCamera.h>
#pragma warning(disable : 4244)

namespace SOUI
{

	S3dWindow::S3dWindow(void):m_iStep(0),m_nRotate(0),m_rotateDir(RotateY),m_nSpeed(5)
	{
	}

	S3dWindow::~S3dWindow(void)
	{
	}


	void S3dWindow::OnShowWindow(BOOL bShow, UINT nStatus)
	{
		__baseCls::OnShowWindow(bShow,nStatus);
		if(IsVisible(TRUE))
		{
			GetContainer()->RegisterTimelineHandler(this);
		}else
		{
			GetContainer()->UnregisterTimelineHandler(this);
		}
	}


	void S3dWindow::OnPaint(IRenderTarget *pRT)
	{
		SPainter painter;
		BeforePaint(pRT,painter);
		CRect rc = GetClientRect();
		if(!(m_skinBack && m_skinFore))
		{
			pRT->DrawText(_T("没有指定skin对象"),-1,rc,DT_SINGLELINE|DT_VCENTER|DT_VCENTER);
		}else
		{
			SCamera camera;
			float zOffset = 0.0f;
			switch(m_rotateDir)
			{
			case RotateX:
				zOffset = rc.Height()/2*fabs(sin(m_nRotate*3.1415926/180));
				camera.translate(0,0,zOffset);
				camera.rotateX(m_nRotate);
				break;
			case RotateY:
				zOffset = rc.Width()/2*fabs(sin(m_nRotate*3.1415926/180));
				camera.translate(0,0,zOffset);
				camera.rotateY(m_nRotate);
				break;
			case RotateZ:camera.rotateZ(m_nRotate);break;
			}
			SMatrix mat;
			camera.getMatrix(&mat);

			int wid = rc.Width()/2+rc.left;
			int hei = rc.Height()/2+rc.top;
			mat.preTranslate(-wid,-hei);
			mat.postTranslate(wid,hei);

			pRT->SetTransform(mat.fMat,NULL);
			//[0,90) [180,270)=> fore
			int nRotate = (int)m_nRotate;
			if(m_nRotate<180)
			{
				if((m_nRotate%180)>90)
					m_skinBack->DrawByIndex(pRT,rc,0);
				else
					m_skinFore->DrawByIndex(pRT,rc,0);
			}else
			{
				if((m_nRotate%180)>90)
					m_skinFore->DrawByIndex(pRT,rc,0);
				else
					m_skinBack->DrawByIndex(pRT,rc,0);
			}
			
			mat.setIdentity();
			pRT->SetTransform(mat.fMat,NULL);
		}
		AfterPaint(pRT,painter);
	}

	void S3dWindow::OnNextFrame()
	{
		m_iStep ++;
		if(m_iStep==m_nSpeed)
		{
			m_nRotate += 5;
			if(m_nRotate>=360) 
			{
				m_nRotate-=360;
			}
			Invalidate();
			m_iStep=0;
		}
	}

	void S3dWindow::OnDestroy()
	{
		GetContainer()->UnregisterTimelineHandler(this);
		__baseCls::OnDestroy();
	}


	/////////////////////////////////////////////////////////////////////////
SMatrixWindow::SMatrixWindow(void)
:m_fRotate(.0f)
,m_fScaleX(1.0F),m_fScaleY(1.0f)
,m_fSkewX(.0f),m_fSkewY(.0f)
,m_fTransX(.0f),m_fTransY(.0f)
{
}

SMatrixWindow::~SMatrixWindow(void)
{
}

HRESULT SMatrixWindow::OnAttrRotate(const SStringW & strValue,BOOL bLoading)
{
    float fRotate = 0.0f;
    if(1==swscanf(strValue,L"%f",&fRotate))
        m_fRotate = fRotate;
    return bLoading?S_FALSE:S_OK;
}

HRESULT SMatrixWindow::OnAttrScale(const SStringW & strValue,BOOL bLoading)
{
    float fScaleX=0.0f,fScaleY=0.0f;
    if(2!=swscanf(strValue,L"%f,%f",&fScaleX,&fScaleY)) return E_FAIL;
    m_fScaleX = fScaleX;
    m_fScaleY = fScaleY;
    return bLoading?S_FALSE:S_OK;
}

HRESULT SMatrixWindow::OnAttrSkew(const SStringW & strValue,BOOL bLoading)
{
    float fSkewX=0.0f,fSkewY=0.0f;
    if(2!=swscanf(strValue,L"%f,%f",&fSkewX,&fSkewY)) return E_FAIL;
    m_fSkewX = fSkewX;
    m_fSkewY = fSkewY;
    return bLoading?S_FALSE:S_OK;
}

HRESULT SMatrixWindow::OnAttrTranslate(const SStringW & strValue,BOOL bLoading)
{
    float fTransX=0.0f,fTransY=0.0f;
    if(2!=swscanf(strValue,L"%f,%f",&fTransX,&fTransY)) return E_FAIL;
    m_fTransX = fTransX;
    m_fTransY = fTransY;
    return bLoading?S_FALSE:S_OK;
}


void SMatrixWindow::OnPaint(IRenderTarget *pRT)
{
    SPainter painter;
    BeforePaint(pRT,painter);
    CRect rc = GetClientRect();
    if(!m_pBgSkin)
    {
        pRT->DrawText(_T("没有指定skin对象"),-1,rc,DT_SINGLELINE|DT_VCENTER|DT_VCENTER);
    }else
    {
        SMatrix m;
		float m2[9];
         m.rotate(m_fRotate)
         .scale(m_fScaleX,m_fScaleY)
         .shear(m_fSkewX,m_fSkewY)
         .translate(m_fTransX,m_fTransY);
        
        pRT->SetTransform(m.fMat,m2);
        m_pBgSkin->DrawByIndex(pRT,rc,0);
        pRT->SetTransform(m2,NULL);
    }
    AfterPaint(pRT,painter);
}
}