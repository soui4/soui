#include "stdafx.h"
#include "SVerificationCodeEdit.h"

SNSBEGIN

SVerificationCodeEdit::SVerificationCodeEdit()
    : m_crBox(RGBA(200, 200, 200, 255))
    , m_crActiveBox(RGBA(100, 100, 100, 255))
    , m_crText(RGBA(0, 0, 0, 255))
    , m_nBoxSpacing(10)
    , m_nBoxWidth(40)
    , m_nBoxHeight(40)
{
    m_cchTextMost = 6;
    m_fDisableCaret = 1;
}

SVerificationCodeEdit::~SVerificationCodeEdit()
{
}

SIZE SVerificationCodeEdit::MeasureContent(int nParentWid, int nParentHei)
{
    SIZE ret;
    ret.cx = m_nBoxWidth * m_cchTextMost + m_nBoxSpacing * (m_cchTextMost - 1);
    ret.cy = m_nBoxHeight;
    return ret;
}

void SVerificationCodeEdit::OnPaint(IRenderTarget *pRT)
{
    // 绘制字符格子
    DrawCharBoxes(pRT);
}

void SVerificationCodeEdit::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
    // 调用父类的OnKeyDown方法
    if(nChar == VK_LEFT || nChar == VK_RIGHT || nChar == VK_UP || nChar == VK_DOWN)
        return;
    __baseCls::OnKeyDown(nChar, nRepCnt, nFlags);
    if(GetWindowTextLength() == m_cchTextMost){
        EventKeyEnter evt(this);
        FireEvent(evt);
    }
    Invalidate();
}

void SVerificationCodeEdit::OnLButtonDown(UINT flags, CPoint pt)
{
    // 调用父类的OnLButtonDown方法
    __baseCls::OnLButtonDown(flags, pt);
    
    // 更新光标位置到最后
    UpdateCursorPosition();
}

void SVerificationCodeEdit::OnLButtonUp(UINT flags, CPoint pt)
{
    // 调用父类的OnLButtonUp方法
    __baseCls::OnLButtonUp(flags, pt);
    
    // 更新光标位置到最后
    UpdateCursorPosition();
}

void SVerificationCodeEdit::OnMouseMove(UINT flags, CPoint pt)
{
    // 调用父类的OnMouseMove方法
    __baseCls::OnMouseMove(flags, pt);
    
    // 更新光标位置到最后
    UpdateCursorPosition();
}

void SVerificationCodeEdit::OnSetFocus(SWND wndOld)
{
    // 调用父类的OnSetFocus方法
    __baseCls::OnSetFocus(wndOld);
    
    // 更新光标位置到最后
    UpdateCursorPosition();
    
    // 重绘控件以显示激活状态
    Invalidate();
}

void SVerificationCodeEdit::OnKillFocus(SWND wndFocus)
{
    // 调用父类的OnKillFocus方法
    __baseCls::OnKillFocus(wndFocus);
    
    // 重绘控件以移除激活状态
    Invalidate();
}

void SVerificationCodeEdit::DrawCharBoxes(IRenderTarget *pRT)
{
    // 获取编辑框的矩形区域
    CRect rcClient;
    GetClientRect(&rcClient);
    
    // 获取文本内容
    SStringT strText=GetWindowText();
    
    // 获取maxBuffer属性，确定需要绘制的格子数量
    int nMaxLength = GetLimitText();
    
    // 计算总宽度
    int nTotalWidth = nMaxLength * m_nBoxWidth + (nMaxLength - 1) * m_nBoxSpacing;
    
    // 计算起始X坐标，使格子居中显示
    int nStartX = (rcClient.Width() - nTotalWidth) / 2;
    int nStartY = (rcClient.Height() - m_nBoxHeight) / 2;
    
    // 检查控件是否获得焦点
    BOOL bHasFocus = GetContainer()->GetFocus() == GetSwnd();
    
    
    // 绘制每个格子
    for (int i = 0; i < nMaxLength; i++)
    {
        // 计算当前格子的矩形
        CRect rcBox(nStartX + i * (m_nBoxWidth + m_nBoxSpacing), nStartY, 
                    nStartX + i * (m_nBoxWidth + m_nBoxSpacing) + m_nBoxWidth, 
                    nStartY + m_nBoxHeight);
        rcBox.OffsetRect(rcClient.TopLeft());
        // 创建画笔和画刷
        SAutoRefPtr<IPenS> hPen, oldPen;
        
        // 确定使用哪种颜色
        if (bHasFocus && i == strText.GetLength())
        {
            // 当前格子是活动格子，且控件有焦点
            pRT->CreatePen(PS_SOLID,  m_crActiveBox, 2 , &hPen);
        }
        else
        {
            // 当前格子是普通格子，或者控件没有焦点
            pRT->CreatePen(PS_SOLID,  m_crBox, 1 , &hPen);
        }
        // 选择画笔和画刷
        pRT->SelectObject(hPen,(IRenderObj**)&oldPen);
        
        // 绘制格子
        pRT->DrawRectangle(rcBox);
        
        // 如果当前格子有字符，绘制字符
        if (i < strText.GetLength())
        {
            // 设置文本颜色
            COLORREF crOldText = pRT->SetTextColor(m_crText);
            pRT->DrawText(strText.c_str()+i, 1, &rcBox, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
            // 恢复旧的文本颜色和格式
            pRT->SetTextColor(crOldText);
        }
        
        // 恢复旧的画笔和画刷
        pRT->SelectObject(oldPen);
    }
}

void SVerificationCodeEdit::UpdateCursorPosition()
{
    // 获取文本长度
    int nTextLength = GetWindowTextLength();
    
    // 设置光标位置到最后
    SetSel(nTextLength, nTextLength, FALSE);
}

SNSEND
