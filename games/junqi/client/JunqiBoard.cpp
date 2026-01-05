#include "stdafx.h"
#include "JunqiBoard.h"
#include <layout/SAnchorLayout.h>
#include <JunqiLayout.h>
SNSBEGIN

//--------------------------------------------------------------------------------
CJunqiBoardGrid::CJunqiBoardGrid(void)
{
    m_bMsgTransparent = FALSE;
    AddEvent(EVENTID(EvtGridClick));
}

CJunqiBoardGrid::~CJunqiBoardGrid(void)
{
}

void CJunqiBoardGrid::OnLButtonDown(UINT nFlags, CPoint pt)
{
    __baseCls::OnLButtonDown(nFlags, pt);
    EvtGridClick evt(this);
    CRect rcClient;
    GetClientRect(&rcClient);
    int gridWid = rcClient.Width() / CJunQiLayout::MAT_WIDTH;
    int gridHei = rcClient.Height() / CJunQiLayout::MAT_WIDTH;
    pt -= rcClient.TopLeft();
    pt.x = pt.x / gridWid;
    pt.y = pt.y / gridHei;
    pt.y = 16-pt.y;
    if((pt.x<6 && pt.y<6)
      ||(pt.x<6 && pt.y>10)
      ||(pt.x>10 && pt.y>10)
      ||(pt.x>10 && pt.y<6))
    {
        return;
    }
    evt.pt = pt; 
    FireEvent(evt);
}
//--------------------------------------------------------------------------------
CJunqiBoard::CJunqiBoard(void):m_nChsScale(100)
{
}

CJunqiBoard::~CJunqiBoard(void)
{
}

void CJunqiBoard::SDispatchMessage(UINT message, WPARAM wParam, LPARAM lParam)
{
    if (message == UM_SETSCALE)
    {
        if(lParam!=0)
        {
            m_nChsScale = (int)wParam;
        }else{
            SSendMessage(message, wParam, lParam);
        }
        // the used defined scale should only used for pieces.
        SWindow *pChild = GetWindow(GSW_FIRSTCHILD);
        while (pChild)
        {
            SAnchorLayoutParam *pParam = (SAnchorLayoutParam *)pChild->GetLayoutParam();
            SAnchorLayoutParamStruct *pParamStruct = (SAnchorLayoutParamStruct *)pParam->GetRawData();
            BOOL bChessAnchor = pParamStruct->pos.type >= 10;
            if (bChessAnchor && lParam != 0)
            {
                pChild->SDispatchMessage(message, wParam, lParam);
            }
            else if (!bChessAnchor && lParam == 0)
            {
                pChild->SDispatchMessage(message, wParam, lParam);
            }
            pChild = pChild->GetWindow(GSW_NEXTSIBLING);
        }
    }
    else
    {
        __baseCls::SDispatchMessage(message, wParam, lParam);
    }
}

SNSEND