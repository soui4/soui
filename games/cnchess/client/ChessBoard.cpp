#include "stdafx.h"
#include "ChessBoard.h"
#include <layout/SAnchorLayout.h>

SNSBEGIN

CChessBoard::CChessBoard(void):m_nChsScale(100)
{
}

CChessBoard::~CChessBoard(void)
{
}

void CChessBoard::SDispatchMessage(UINT message, WPARAM wParam, LPARAM lParam)
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