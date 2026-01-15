#include "stdafx.h"
#include "SPropertyItem-Color.h"
#include "../SPropertyEmbedWndHelper.hpp"
#include "../SPropertyGrid.h"
#include <commdlg.h>
#include <res.mgr/SNamedValue.h>

const int KTransGridSize    =5;
SNSBEGIN

SPropertyItemColor::SPropertyItemColor(SPropertyGrid *pOwner) :SPropertyItemText(pOwner),m_crValue(CR_INVALID)
	{
		m_strFormat = _T("#%02x%02x%02x%02x");
	}

    void SPropertyItemColor::DrawItem( IRenderTarget *pRT,CRect rc )
    {
        CRect rcColor = rc;
        rcColor.right = rcColor.left + rc.Height();
        rcColor.DeflateRect(2,2);
        
        //画一个代表透明的网格背景
        pRT->PushClipRect(&rcColor,RGN_AND);
        bool bDrawY=true;
        for(int y=rcColor.top;y<rcColor.bottom;y+=KTransGridSize)
        {
            bool bDraw= bDrawY;
            for(int x=rcColor.left;x<rcColor.right;x+=KTransGridSize)
            {
                if(bDraw) pRT->FillSolidRect(CRect(CPoint(x,y),CSize(KTransGridSize,KTransGridSize)),RGBA(0xcc,0xcc,0xcc,0xff));
                bDraw=!bDraw;
            }
            bDrawY=!bDrawY;
        }
        pRT->PopClip();
        
		if(m_crValue!=CR_INVALID)
		{
			pRT->FillSolidRect(&rcColor,m_crValue);
			pRT->DrawRectangle(&rcColor);
			CRect rcValue = rc;
			rcValue.left += rc.Height();
			SStringT strValue = GetValue();
			pRT->DrawText(strValue,strValue.GetLength(),&rcValue,DT_SINGLELINE|DT_VCENTER);
		}else
		{
			SAutoRefPtr<IPenS> pen,oldPen;
			pRT->CreatePen(PS_SOLID,RGBA(255,0,0,255),1,&pen);
			pRT->SelectObject(pen,(IRenderObj**)&oldPen);
			{
				CPoint pts[2]={rcColor.TopLeft(),rcColor.BottomRight()};
				pRT->DrawLines(pts,2);
			}
			{
				CPoint pts[2]={CPoint(rcColor.left,rcColor.bottom),CPoint(rcColor.right,rcColor.top)};
				pRT->DrawLines(pts,2);
			}
			pRT->SelectObject(oldPen,NULL);
		}
    }
    
    void SPropertyItemColor::OnInplaceActive(BOOL bActive)
    {
        SPropertyItemText::OnInplaceActive(bActive);

    }
    void SPropertyItemColor::SetValue( const SStringT & strValue )
    {
		COLORREF crTmp = m_pOwner->ConvertColor(strValue,CR_INVALID);
		if (m_crValue != crTmp)
		{
			m_crValue = crTmp;
            SStringT value = SStringT().Format(m_strFormat.c_str(), GetRValue(m_crValue), GetGValue(m_crValue), GetBValue(m_crValue), GetAValue(m_crValue));
            __baseCls::SetValue(value);
		}
    }

	LRESULT SPropertyItemColor::OnAttrColor(const SStringW &value, BOOL bLoading)
    {
        SetValue(value);
        return bLoading ? S_OK : S_FALSE;
	}

    BOOL SPropertyItemColor::OnButtonClick()
    {
		CColourPopup *pCrPopup = new CColourPopup(GetOwner()->GetContainer()->GetHostHwnd(),this);
		CPoint pt;
		GetCursorPos(&pt);
		pt.x += 10;
		pCrPopup->Create(pt,m_crValue,_T("默认"),_T("更多"));
		return TRUE;
    }

    void SPropertyItemColor::AdjustInplaceActiveWndRect( CRect & rc )
    {
        __baseCls::AdjustInplaceActiveWndRect(rc);
        rc.left += rc.Height();
    }

	void SPropertyItemColor::OnColorChanged( COLORREF cr )
	{
	}

	void SPropertyItemColor::OnColorEnd( BOOL bCancel,COLORREF cr )
	{
		if(!bCancel)
		{
            BYTE a = GetAValue(cr);
            if (a == 0)
                a = 0xff;
            SetValue(SStringT().Format(m_strFormat.c_str(),GetRValue(cr), GetGValue(cr), GetBValue(cr),a));
		}
	}

	IMessageLoop * SPropertyItemColor::GetMsgLoop()
	{
		return GetOwner()->GetContainer()->GetMsgLoop();
	}

	SStringT SPropertyItemColor::GetValue() const
	{
		if (m_crValue == CR_INVALID)
		{
			return _T("");
		}
        return __baseCls::GetValue();
	}

	BOOL SPropertyItemColor::HasValue() const
	{
		return m_crValue != CR_INVALID;
	}

	void SPropertyItemColor::ClearValue()
	{
		m_crValue = CR_INVALID;
		OnValueChanged();
	}

SNSEND