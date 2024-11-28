// Windows Template Library - WTL version 8.1
// Copyright (C) Microsoft Corporation. All rights reserved.
//
// This file is a part of the Windows Template Library.
// The use and distribution terms for this software are covered by the
// Common Public License 1.0 (http://opensource.org/licenses/cpl1.0.php)
// which can be found in the file CPL.TXT at the root of this distribution.
// By using this software in any fashion, you are agreeing to be bound by
// the terms of this license. You must not remove this notice, or
// any other, from this software.

#ifndef __SOUIMISC_H__
#define __SOUIMISC_H__




///////////////////////////////////////////////////////////////////////////////
// Classes in this file:
//
// CSize
// CPoint
// CRect


namespace SOUI
{

// forward declarations
class CSize;
class CPoint;
class CRect;

///////////////////////////////////////////////////////////////////////////////
// CSize - Wrapper for Windows SIZE structure.

class CSize : public SIZE
{
public:
// Constructors
    CSize()
    {
        cx = 0;
        cy = 0;
    }

    CSize(int initCX, int initCY)
    {
        cx = initCX;
        cy = initCY;
    }

    CSize(SIZE initSize)
    {
        *(SIZE*)this = initSize;
    }

    CSize(POINT initPt)
    {
        *(POINT*)this = initPt;
    }

    CSize(DWORD dwSize)
    {
        cx = (short)LOWORD(dwSize);
        cy = (short)HIWORD(dwSize);
    }

// Operations
    BOOL operator ==(SIZE size) const
    {
        return (cx == size.cx && cy == size.cy);
    }

    BOOL operator !=(SIZE size) const
    {
        return (cx != size.cx || cy != size.cy);
    }

    void operator +=(SIZE size)
    {
        cx += size.cx;
        cy += size.cy;
    }

    void operator -=(SIZE size)
    {
        cx -= size.cx;
        cy -= size.cy;
    }

    void SetSize(int CX, int CY)
    {
        cx = CX;
        cy = CY;
    }

// Operators returning CSize values
    CSize operator +(SIZE size) const
    {
        return CSize(cx + size.cx, cy + size.cy);
    }

    CSize operator -(SIZE size) const
    {
        return CSize(cx - size.cx, cy - size.cy);
    }

    CSize operator -() const
    {
        return CSize(-cx, -cy);
    }

// Operators returning CPoint values
    CPoint operator +(POINT point) const;
    CPoint operator -(POINT point) const;

// Operators returning CRect values
    CRect operator +(const RECT* lpRect) const;
    CRect operator -(const RECT* lpRect) const;
};


///////////////////////////////////////////////////////////////////////////////
// CPoint - Wrapper for Windows POINT structure.

class CPoint : public POINT
{
public:
// Constructors
    CPoint()
    {
        x = 0;
        y = 0;
    }

    CPoint(int initX, int initY)
    {
        x = initX;
        y = initY;
    }

    CPoint(POINT initPt)
    {
        *(POINT*)this = initPt;
    }

    CPoint(SIZE initSize)
    {
        *(SIZE*)this = initSize;
    }

    CPoint(DWORD dwPoint)
    {
        x = (short)LOWORD(dwPoint);
        y = (short)HIWORD(dwPoint);
    }

// Operations
    void Offset(int xOffset, int yOffset)
    {
        x += xOffset;
        y += yOffset;
    }

    void Offset(POINT point)
    {
        x += point.x;
        y += point.y;
    }

    void Offset(SIZE size)
    {
        x += size.cx;
        y += size.cy;
    }

    BOOL operator ==(POINT point) const
    {
        return (x == point.x && y == point.y);
    }

    BOOL operator !=(POINT point) const
    {
        return (x != point.x || y != point.y);
    }

    void operator +=(SIZE size)
    {
        x += size.cx;
        y += size.cy;
    }

    void operator -=(SIZE size)
    {
        x -= size.cx;
        y -= size.cy;
    }

    void operator +=(POINT point)
    {
        x += point.x;
        y += point.y;
    }

    void operator -=(POINT point)
    {
        x -= point.x;
        y -= point.y;
    }

    void SetPoint(int X, int Y)
    {
        x = X;
        y = Y;
    }

// Operators returning CPoint values
    CPoint operator +(SIZE size) const
    {
        return CPoint(x + size.cx, y + size.cy);
    }

    CPoint operator -(SIZE size) const
    {
        return CPoint(x - size.cx, y - size.cy);
    }

    CPoint operator -() const
    {
        return CPoint(-x, -y);
    }

    CPoint operator +(POINT point) const
    {
        return CPoint(x + point.x, y + point.y);
    }

// Operators returning CSize values
    CSize operator -(POINT point) const
    {
        return CSize(x - point.x, y - point.y);
    }

// Operators returning CRect values
    CRect operator +(const RECT* lpRect) const;
    CRect operator -(const RECT* lpRect) const;
};


///////////////////////////////////////////////////////////////////////////////
// CRect - Wrapper for Windows RECT structure.

class CRect : public RECT
{
public:
// Constructors
    CRect()
    {
        left = 0;
        top = 0;
        right = 0;
        bottom = 0;
    }

    CRect(int l, int t, int r, int b)
    {
        left = l;
        top = t;
        right = r;
        bottom = b;
    }

    CRect(const RECT& srcRect)
    {
        CopyRect(&srcRect);
    }

    CRect(LPCRECT lpSrcRect)
    {
        CopyRect(lpSrcRect);
    }

    CRect(POINT point, SIZE size)
    {
        right = (left = point.x) + size.cx;
        bottom = (top = point.y) + size.cy;
    }

    CRect(POINT topLeft, POINT bottomRight)
    {
        left = topLeft.x;
        top = topLeft.y;
        right = bottomRight.x;
        bottom = bottomRight.y;
    }

// Attributes (in addition to RECT members)
    int Width() const
    {
        return right - left;
    }

    int Height() const
    {
        return bottom - top;
    }

    CSize Size() const
    {
        return CSize(right - left, bottom - top);
    }

    CPoint& TopLeft()
    {
        return *((CPoint*)this);
    }

    CPoint& BottomRight()
    {
        return *((CPoint*)this + 1);
    }

    const CPoint& TopLeft() const
    {
        return *((CPoint*)this);
    }

    const CPoint& BottomRight() const
    {
        return *((CPoint*)this + 1);
    }

    CPoint CenterPoint() const
    {
        return CPoint((left + right) / 2, (top + bottom) / 2);
    }

    // convert between CRect and LPRECT/LPCRECT (no need for &)
    operator LPRECT()
    {
        return this;
    }

    operator LPCRECT() const
    {
        return this;
    }


    BOOL IsRectEmpty() const
    {
		return ::IsRectEmpty(this);
    }

    BOOL IsRectNull() const
    {
        return (left == 0 && right == 0 && top == 0 && bottom == 0);
    }

    BOOL PtInRect(POINT point) const
    {
        return point.x>=left && point.x<right
                && point.y>=top && point.y<bottom;
    }

// Operations
    void SetRect(int x1, int y1, int x2, int y2)
    {
        left = x1, right=x2;
        top = y1, bottom = y2;
    }

    void SetRect(POINT topLeft, POINT bottomRight)
    {
        SetRect(topLeft.x, topLeft.y, bottomRight.x, bottomRight.y);
    }

    void SetRectEmpty()
    {
        right = left;
        bottom = top;
    }

    void CopyRect(LPCRECT lpSrcRect)
    {
        left = lpSrcRect->left;
        top = lpSrcRect->top;
        right = lpSrcRect->right;
        bottom = lpSrcRect->bottom;
    }

    BOOL EqualRect(LPCRECT lpRect) const
    {
        return left == lpRect->left && right == lpRect->right
               && top == lpRect->top && bottom == lpRect->bottom;
    }

    void InflateRect(int x, int y)
    {
        left -= x, right+=x;
        top -=y ,bottom += y;
    }

    void InflateRect(SIZE size)
    {
        InflateRect(size.cx, size.cy);
    }

    void InflateRect(LPCRECT lpRect)
    {
        left -= lpRect->left;
        top -= lpRect->top;
        right += lpRect->right;
        bottom += lpRect->bottom;
    }

    void InflateRect(int l, int t, int r, int b)
    {
        left -= l;
        top -= t;
        right += r;
        bottom += b;
    }

    void DeflateRect(int x, int y)
    {
        InflateRect(-x, -y);
    }

    void DeflateRect(SIZE size)
    {
        InflateRect(-size.cx, -size.cy);
    }

    void DeflateRect(LPCRECT lpRect)
    {
        left += lpRect->left;
        top += lpRect->top;
        right -= lpRect->right;
        bottom -= lpRect->bottom;
    }

    void DeflateRect(int l, int t, int r, int b)
    {
        left += l;
        top += t;
        right -= r;
        bottom -= b;
    }

    void OffsetRect(int x, int y)
    {
        left +=x;
        right +=x;
        top += y;
        bottom += y;
    }
    void OffsetRect(SIZE size)
    {
        OffsetRect(size.cx, size.cy);
    }

    void OffsetRect(POINT point)
    {
        OffsetRect(point.x, point.y);
    }

    void NormalizeRect()
    {
        int nTemp;
        if (left > right)
        {
            nTemp = left;
            left = right;
            right = nTemp;
        }
        if (top > bottom)
        {
            nTemp = top;
            top = bottom;
            bottom = nTemp;
        }
    }

    // absolute position of rectangle
    void MoveToY(int y)
    {
        bottom = Height() + y;
        top = y;
    }

    void MoveToX(int x)
    {
        right = Width() + x;
        left = x;
    }

    void MoveToXY(int x, int y)
    {
        MoveToX(x);
        MoveToY(y);
    }

    void MoveToXY(POINT pt)
    {
        MoveToX(pt.x);
        MoveToY(pt.y);
    }

    
    // operations that fill '*this' with result
    BOOL IntersectRect(LPCRECT src1, LPCRECT src2)
    {
        if (!src1 || !src2) return FALSE;
        CRect rc1(src1),rc2(src2);
        if (rc1.IsRectEmpty() || rc2.IsRectEmpty() ||
            (src1->left >= src2->right) || (src2->left >= src1->right) ||
            (src1->top >= src2->bottom) || (src2->top >= src1->bottom))
        {
            SetRectEmpty();
            return FALSE;
        }
        left   = smax( src1->left, src2->left );
        right  = smin( src1->right, src2->right );
        top    = smax( src1->top, src2->top );
        bottom = smin( src1->bottom, src2->bottom );
        return TRUE;
    }

    BOOL UnionRect(LPCRECT lpRect1, LPCRECT lpRect2)
    {
        CRect src1(lpRect1),src2(lpRect2);
        if (src1.IsRectEmpty())
        {
            if (src2.IsRectEmpty())
            {
                SetRectEmpty();
                return FALSE;
            }
            else *this = src2;
        }
        else
        {
            if (src2.IsRectEmpty()) *this = src1;
            else
            {
                left   = smin( src1.left, src2.left );
                right  = smax( src1.right, src2.right );
                top    = smin( src1.top, src2.top );
                bottom = smax( src1.bottom, src2.bottom );
            }
        }
        return TRUE;
    }

    BOOL SubtractRect(const RECT *src1, const RECT *src2 )
    {
        CRect tmp;
		if (::IsRectEmpty( src1 ))
        {
            SetRectEmpty();
            return FALSE;
        }
        if (tmp.IntersectRect(src1, src2 ))
        {
            if (tmp.EqualRect(src1 ))
            {
                SetRectEmpty( );
                return FALSE;
            }
            *this = *src1;
            if ((tmp.top == top) && (tmp.bottom == bottom))
            {
                if (tmp.left == left) left = tmp.right;
                else if (tmp.right == right) right = tmp.left;
            }
            else if ((tmp.left == left) && (tmp.right == right))
            {
                if (tmp.top == top) top = tmp.bottom;
                else if (tmp.bottom == bottom) bottom = tmp.top;
            }
        }
        else
        {
            *this = *src1;
        }
        return TRUE;
    }

// Additional Operations
    void operator =(const RECT& srcRect)
    {
        CopyRect(&srcRect);
    }

    BOOL operator ==(const RECT& rect) const
    {
        return EqualRect(&rect);
    }

    BOOL operator !=(const RECT& rect) const
    {
        return EqualRect(&rect)==0;
    }

    void operator +=(POINT point)
    {
        OffsetRect(point.x, point.y);
    }

    void operator +=(SIZE size)
    {
        OffsetRect(size.cx, size.cy);
    }

    void operator +=(LPCRECT lpRect)
    {
        InflateRect(lpRect);
    }

    void operator -=(POINT point)
    {
        OffsetRect(-point.x, -point.y);
    }

    void operator -=(SIZE size)
    {
        OffsetRect(-size.cx, -size.cy);
    }

    void operator -=(LPCRECT lpRect)
    {
        DeflateRect(lpRect);
    }

    void operator &=(const RECT& rect)
    {
        IntersectRect(this, &rect);
    }

    void operator |=(const RECT& rect)
    {
        UnionRect(this, &rect);
    }

// Operators returning CRect values
    CRect operator +(POINT pt) const
    {
        CRect rect(*this);
        rect.OffsetRect(pt.x, pt.y);
        return rect;
    }

    CRect operator -(POINT pt) const
    {
        CRect rect(*this);
        rect.OffsetRect(-pt.x, -pt.y);
        return rect;
    }

    CRect operator +(LPCRECT lpRect) const
    {
        CRect rect(this);
        rect.InflateRect(lpRect);
        return rect;
    }

    CRect operator +(SIZE size) const
    {
        CRect rect(*this);
        rect.OffsetRect(size.cx, size.cy);
        return rect;
    }

    CRect operator -(SIZE size) const
    {
        CRect rect(*this);
        rect.OffsetRect(-size.cx, -size.cy);
        return rect;
    }

    CRect operator -(LPCRECT lpRect) const
    {
        CRect rect(this);
        rect.DeflateRect(lpRect);
        return rect;
    }

    CRect operator &(const RECT& rect2) const
    {
        CRect rect;
        rect.IntersectRect(this, &rect2);
        return rect;
    }

    CRect operator |(const RECT& rect2) const
    {
        CRect rect;
        rect.UnionRect(this, &rect2);
        return rect;
    }

    CRect MulDiv(int nMultiplier, int nDivisor) const
    {
        return CRect(
                   ::MulDiv(left, nMultiplier, nDivisor),
                   ::MulDiv(top, nMultiplier, nDivisor),
                   ::MulDiv(right, nMultiplier, nDivisor),
                   ::MulDiv(bottom, nMultiplier, nDivisor));
    }
};


// CSize implementation

inline CPoint CSize::operator +(POINT point) const
{
    return CPoint(cx + point.x, cy + point.y);
}

inline CPoint CSize::operator -(POINT point) const
{
    return CPoint(cx - point.x, cy - point.y);
}

inline CRect CSize::operator +(const RECT* lpRect) const
{
    return CRect(lpRect) + *this;
}

inline CRect CSize::operator -(const RECT* lpRect) const
{
    return CRect(lpRect) - *this;
}


// CPoint implementation

inline CRect CPoint::operator +(const RECT* lpRect) const
{
    return CRect(lpRect) + *this;
}

inline CRect CPoint::operator -(const RECT* lpRect) const
{
    return CRect(lpRect) - *this;
}


template <class Num>
inline CSize operator *(SIZE s, Num n)
{
    return CSize((int)(s.cx * n), (int)(s.cy * n));
};

template <class Num>
inline void operator *=(SIZE & s, Num n)
{
    s = s * n;
};

template <class Num>
inline CSize operator /(SIZE s, Num n)
{
    return CSize((int)(s.cx / n), (int)(s.cy / n));
};

template <class Num>
inline void operator /=(SIZE & s, Num n)
{
    s = s / n;
};


}; // namespace SOUI

#endif // __SOUIMISC_H__
