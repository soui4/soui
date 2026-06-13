#ifndef __SHSCROLLBAR_H__
#define __SHSCROLLBAR_H__
SNSBEGIN

//实现一个只有横向滚动条皮肤
/*img format: 1-normal, 2-pushdown, -pushdown, 4-disable
1 2 3 4 //for thumb
1 2 3 4 //for rail
*/
class SSkinHScrollbar : public SSkinScrollbar {
    DEF_SOBJECT(SSkinScrollbar, L"hscrollbar")

  public:
    SSkinHScrollbar()
    {
        m_nStates = 4;
    }

    ~SSkinHScrollbar()
    {
    }
    //不支持显示上下箭头
    virtual BOOL HasArrow() const override
    {
        return FALSE;
    }
    virtual int GetIdealSize() const override
    {
        SIZE sz = GetImageSize();
        return sz.cy / m_nStates;
    }

  protected:

    //返回源指定部分在原位图上的位置。
    virtual CRect GetPartRect(int nSbCode, int nState, BOOL bVertical) const override
    {
        CRect rc;
        if (bVertical || nSbCode == SB_LINEDOWN || nSbCode == SB_LINEUP || nSbCode == SB_CORNOR
            || nSbCode == SB_THUMBGRIPPER)
            return rc;

        if (nState == SBST_INACTIVE && !m_bHasInactive)
        {
            nState = SBST_NORMAL;
        }
        SIZE sz = GetImageSize(TRUE);
        rc.right = sz.cx/2;
        rc.bottom = sz.cy / m_nStates;

        rc.OffsetRect(0,rc.Height() * nState);
        if (nSbCode == SB_PAGEUP || nSbCode == SB_PAGEDOWN)
        {
            rc.OffsetRect(rc.Width(), 0);
        }
        return rc;
    }
};

SNSEND

#endif//__SHSCROLLBAR_H__
