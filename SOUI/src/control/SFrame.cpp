#include <souistd.h>
#include <control/SFrame.h>

SNSBEGIN

SFrame::SFrame(void)
{
}

SFrame::~SFrame(void)
{
}

SWND SFrame::SwndFromPoint(CPoint &pt, BOOL bIncludeMsgTransparent) const
{
    SWND ret = __baseCls::SwndFromPoint(pt, bIncludeMsgTransparent);
    if (ret == m_swnd)
        ret = 0;
    return ret;
}

BOOL SFrame::IsFocusable(THIS) const
{
    return FALSE;
}

SNSEND