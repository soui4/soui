#pragma once

SNSBEGIN

typedef enum _SObjectType
{
    None = -1,
    Undef = 0,
    NativeWnd,
    Window,
    Skin,
    Layout,
    Event,
    Interpolator,
    Animation,
    ValueAnimator,
    UserType,
} SObjectType;

SNSEND