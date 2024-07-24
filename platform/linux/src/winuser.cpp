#include <platform.h>
#include <algorithm>

BOOL  SetRect(LPRECT lprc, int xLeft, int yTop, int xRight, int yBottom)
{
    lprc->left = xLeft;
    lprc->top = yTop;
    lprc->right = xRight;
    lprc->bottom = yBottom;
    return TRUE;
}

BOOL  SetRectEmpty(LPRECT lprc)
{
    lprc->left = lprc->right = lprc->top=lprc->bottom=0;
    return TRUE;
}

BOOL  CopyRect(LPRECT lprcDst, const RECT *lprcSrc)
{
    memcpy(lprcDst,lprcSrc,sizeof(RECT));
    return TRUE;
}

BOOL  InflateRect(LPRECT lprc, int dx, int dy)
{
    lprc->left -= dx;
    lprc->right+=dx;
    lprc->top -=dy;
    lprc->bottom+=dy;
    return TRUE;
}

BOOL  IntersectRect(LPRECT lprcDst, const RECT *lprcSrc1, const RECT *lprcSrc2)
{
    lprcDst->left = std::max(lprcSrc1->left,lprcSrc2->left);
    lprcDst->top = std::max(lprcSrc1->top,lprcSrc2->top);
    lprcDst->right = std::min(lprcSrc1->right,lprcSrc2->right);
    lprcDst->bottom = std::min(lprcSrc1->bottom,lprcSrc2->bottom);
    return TRUE;
}

BOOL  UnionRect(LPRECT lprcDst, const RECT *lprcSrc1, const RECT *lprcSrc2)
{
    lprcDst->left = std::min(lprcSrc1->left,lprcSrc2->left);
    lprcDst->top = std::min(lprcSrc1->top,lprcSrc2->top);
    lprcDst->right = std::max(lprcSrc1->right,lprcSrc2->right);
    lprcDst->bottom = std::max(lprcSrc1->bottom,lprcSrc2->bottom);
    return TRUE;
}

BOOL SubtractRect(LPRECT dest, const RECT* src1, const RECT* src2) {
    // 确保源矩形是有效的
    if (!src1 || !src2) {
        return FALSE;
    }

    // 检查src1是否完全在src2内部
    if (src1->left >= src2->left &&
        src1->right <= src2->right &&
        src1->top >= src2->top &&
        src1->bottom <= src2->bottom) {
        return FALSE; // src1完全在src2内部，没有剩余部分
    }

    // 计算src1和src2的交点
    int left = std::max(src1->left, src2->left);
    int top = std::max(src1->top, src2->top);
    int right = std::min(src1->right, src2->right);
    int bottom = std::min(src1->bottom, src2->bottom);

    // 如果没有交点，src1完全在src2外部
    if (left >= right || top >= bottom) {
        *dest = *src1; // 直接复制src1到dest
        return TRUE;
    }

    // 计算减法结果
    dest[0] = { src1->left, src1->top, left, top };
    dest[1] = { left, top, right, bottom };
    dest[2] = { right, src1->top, src1->right, bottom };

    return TRUE;
}

BOOL OffsetRect(LPRECT prc, int dx, int dy)
{
    prc->left += dx;
    prc->right+= dx;
    prc->top += dy;
    prc->bottom += dy;
    return 0;
}

BOOL IsRectEmpty(const RECT *prc)
{
    return prc->left == prc->right && prc->top == prc->bottom;
}

BOOL EqualRect(const RECT *lprc1, const RECT *lprc2)
{
    return lprc1->left == lprc2->left &&
            lprc1->top == lprc2->top &&
            lprc1->right == lprc2->right &&
            lprc1->bottom == lprc2->bottom;
}

BOOL PtInRect(const RECT *lprc, POINT pt)
{
    return pt.x>=lprc->left && pt.x<lprc->right
           && pt.y>=lprc->top && pt.y<lprc->bottom;
}

BOOL TrackMouseEvent(LPTRACKMOUSEEVENT lpEventTrack)
{
    return FALSE;
}