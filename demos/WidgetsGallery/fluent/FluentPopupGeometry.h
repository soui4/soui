#pragma once

// Demo-only arithmetic; no SOUI, HWND, monitor or renderer dependency.
namespace FluentPopupGeometry
{
static const int EnterDurationMs = 150;
static const float EnterDistanceDp = 4.0f;

#if defined(_MSC_VER)
typedef __int64 int64;
#else
typedef long long int64;
#endif

enum Alignment
{
    AlignStart,
    AlignCenter,
    AlignEnd
};

struct Insets
{
    Insets()
        : left(0), top(0), right(0), bottom(0)
    {
    }
    Insets(int l, int t, int r, int b)
        : left(l), top(t), right(r), bottom(b)
    {
    }

    int left, top, right, bottom;
};

struct Rect
{
    Rect()
        : left(0), top(0), right(0), bottom(0)
    {
    }
    Rect(int l, int t, int r, int b)
        : left(l), top(t), right(r), bottom(b)
    {
    }

    int left, top, right, bottom;
    int Width() const
    {
        return right - left;
    }
    int Height() const
    {
        return bottom - top;
    }
};

struct Placement
{
    Placement()
        : opensDown(false), opensRight(false), fromX(0.0f), fromY(0.0f)
    {
    }
    Placement(const Rect &area, bool down, bool right, float x, float y)
        : bounds(area), opensDown(down), opensRight(right), fromX(x), fromY(y)
    {
    }

    Rect bounds;
    bool opensDown, opensRight;
    float fromX, fromY; // dp, NOT already-scaled pixels
};

inline int Min(int a, int b)
{
    return a < b ? a : b;
}
inline int Max(int a, int b)
{
    return a > b ? a : b;
}
inline int Clamp(int value, int low, int high)
{
    return Max(low, Min(value, high));
}

inline int Dp(int value, int scale)
{
    const int64 product = static_cast<int64>(value) * (scale > 0 ? scale : 100);
    return static_cast<int>((product + (product < 0 ? -50 : 50)) / 100);
}

inline Insets Shadow(int scale)
{
    return Insets(Dp(18, scale), Dp(12, scale), Dp(18, scale), Dp(24, scale));
}

inline Insets ContentPadding(const Insets &authored, int gapDp, int scale)
{
    const Insets shadow = Shadow(scale);
    const int gap = Dp(gapDp, scale);
    return Insets(shadow.left + Max(authored.left, gap), shadow.top + Max(authored.top, gap), shadow.right + Max(authored.right, gap), shadow.bottom + Max(authored.bottom, gap));
}

inline bool OpensPositive(int extent, int positiveSpace, int negativeSpace, bool preferPositive)
{
    if (positiveSpace >= extent && (preferPositive || negativeSpace < extent))
        return true;
    if (negativeSpace >= extent)
        return false;
    return positiveSpace >= negativeSpace;
}

inline bool OpensDown(int height, int below, int above)
{
    return OpensPositive(height, below, above, true);
}

inline Rect Fit(const Rect &rect, const Rect &work)
{
    const int width = Min(Max(1, rect.Width()), Max(1, work.Width()));
    const int height = Min(Max(1, rect.Height()), Max(1, work.Height()));
    const int x = Clamp(rect.left, work.left, work.right - width);
    const int y = Clamp(rect.top, work.top, work.bottom - height);
    return Rect(x, y, x + width, y + height);
}

inline Placement PlaceCombo(const Rect &owner, int requestedHeight, const Rect &work, int scale)
{
    const Insets shadow = Shadow(scale);
    const int below = Max(1, work.bottom - owner.bottom + shadow.top);
    const int above = Max(1, owner.top - work.top + shadow.bottom);
    const bool down = OpensDown(requestedHeight, below, above);
    // The framework already counted root padding in requestedHeight. Width is
    // rebuilt from the owner every time, including while filtering a live list.
    const int height = Min(Max(1, requestedHeight), down ? below : above);
    const int width = Max(1, owner.Width()) + shadow.left + shadow.right;
    const int x = owner.left - shadow.left;
    const int y = down ? owner.bottom - shadow.top : owner.top - height + shadow.bottom;
    return Placement(Fit(Rect(x, y, x + width, y + height), work), down, true, 0.0f, down ? -EnterDistanceDp : EnterDistanceDp);
}

// Rebuild the un-clipped framework rectangle from the original TrackPopupMenu
// anchor. This avoids treating an rcMonitor-flipped rectangle as a new anchor.
inline Rect MenuRectAt(int x, int y, int width, int height, Alignment horizontal, Alignment vertical)
{
    const int left = x - (horizontal == AlignEnd ? width : horizontal == AlignCenter ? width / 2 : 0);
    const int top = y - (vertical == AlignEnd ? height : vertical == AlignCenter ? height / 2 : 0);
    return Rect(left, top, left + width, top + height);
}

inline Placement PlaceMenu(const Rect &incoming, const Rect &work, int scale, Alignment horizontal = AlignStart, Alignment vertical = AlignStart)
{
    const Insets shadow = Shadow(scale);
    const int width = Max(1, incoming.Width());
    const int height = Max(1, incoming.Height());
    const int anchorX = incoming.left + (horizontal == AlignEnd ? width : horizontal == AlignCenter ? width / 2 : 0);
    const int anchorY = incoming.top + (vertical == AlignEnd ? height : vertical == AlignCenter ? height / 2 : 0);
    const int rightX = anchorX - shadow.left;
    const int leftX = anchorX - width + shadow.right;
    const bool right = OpensPositive(width, work.right - rightX, anchorX + shadow.right - work.left, horizontal != AlignEnd);
    const int x = horizontal == AlignCenter ? anchorX - width / 2 + (shadow.right - shadow.left) / 2 : right ? rightX : leftX;
    const int downY = anchorY - shadow.top;
    const int upY = anchorY - height + shadow.bottom;
    const bool down = OpensPositive(height, work.bottom - downY, anchorY + shadow.bottom - work.top, vertical != AlignEnd);
    const int y = vertical == AlignCenter ? anchorY - height / 2 + (shadow.bottom - shadow.top) / 2 : down ? downY : upY;
    return Placement(Fit(Rect(x, y, x + width, y + height), work), down, right, 0.0f, down ? -EnterDistanceDp : EnterDistanceDp);
}

inline Placement PlaceSubmenu(const Rect &parent, int triggerTop, int width, int height, const Rect &work, int scale, const Insets &parentPadding, const Insets &childPadding, bool preferRight)
{
    const Insets shadow = Shadow(scale);
    // Adjacent panel edges meet the parent's content edge. The outer shadow
    // rectangles overlap, so moving from the triggering row into either child
    // does not cross an unowned desktop gap (including a left-opening child).
    const int rightX = parent.right - parentPadding.right - shadow.left;
    const int leftEdge = parent.left + parentPadding.left + shadow.right;
    const bool right = OpensPositive(width, work.right - rightX, leftEdge - work.left, preferRight);
    const int x = right ? rightX : leftEdge - width;
    // Align the actual first row, not the panel's top or just the shadow gutter.
    const int y = triggerTop - childPadding.top;
    return Placement(Fit(Rect(x, y, x + width, y + height), work), true, right, right ? -EnterDistanceDp : EnterDistanceDp, 0.0f);
}

inline bool ShouldEnter(bool reduced, bool keyboard, bool liveAnimation, bool authoredEnter, bool systemEnabled)
{
    return !reduced && !keyboard && !liveAnimation && !authoredEnter && systemEnabled;
}

inline float EaseOut(float input)
{
    if (input <= 0.0f)
        return 0.0f;
    if (input >= 1.0f)
        return 1.0f;
    // Invert x(t) for cubic-bezier(0.23, 1, 0.32, 1), then evaluate y(t).
    float lower = 0.0f, upper = 1.0f;
    for (int i = 0; i < 24; ++i)
    {
        const float t = (lower + upper) * 0.5f;
        const float u = 1.0f - t;
        const float x = 3.0f * u * u * t * 0.23f + 3.0f * u * t * t * 0.32f + t * t * t;
        if (x < input)
            lower = t;
        else
            upper = t;
    }
    const float u = 1.0f - (lower + upper) * 0.5f;
    return 1.0f - u * u * u;
}
} // namespace FluentPopupGeometry