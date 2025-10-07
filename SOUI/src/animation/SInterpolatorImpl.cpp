#include "souistd.h"
#include "animation/SInterpolatorImpl.h"
#include "layout/SLayoutSize.h" //for SLayoutSize::fequal
#pragma warning(push)
#pragma warning(disable : 4985) // disable the warning message during the include
#include <math.h>               // this is where I would normally get the warning message
#pragma warning(pop)

SNSBEGIN

static const float PI = 3.1415926f;

//////////////////////////////////////////////////////////////////////////
float SLinearInterpolator::getInterpolation(float input) const
{
    return input;
}

//////////////////////////////////////////////////////////////////////////
float SAccelerateInterpolator::getInterpolation(float input) const
{
    if (SLayoutSize::fequal(mFactor, 1.0f))
    {
        return input * input;
    }
    else
    {
        return (float)pow((double)input, mDoubleFactor);
    }
}

SAccelerateInterpolator::SAccelerateInterpolator(float factor)
    : mFactor(factor)
    , mDoubleFactor(2 * factor)
{
}

//////////////////////////////////////////////////////////////////////////

float SDecelerateInterpolator::getInterpolation(float input) const
{
    float result;
    if (SLayoutSize::fequal(mFactor, 1.0f))
    {
        result = (float)(1.0f - (1.0f - input) * (1.0f - input));
    }
    else
    {
        result = (float)(1.0f - pow((1.0f - input), 2 * mFactor));
    }
    return result;
}

SDecelerateInterpolator::SDecelerateInterpolator(float factor)
    : mFactor(factor)
{
}

//////////////////////////////////////////////////////////////////////////
float SAccelerateDecelerateInterpolator::getInterpolation(float input) const
{
    return (float)(cos((input + 1) * PI) / 2.0f) + 0.5f;
}

//////////////////////////////////////////////////////////////////////////
float SAnticipateInterpolator::getInterpolation(float t) const
{
    // a(t) = t * t * ((tension + 1) * t - tension)
    return t * t * ((mTension + 1) * t - mTension);
}

SAnticipateInterpolator::SAnticipateInterpolator(float tension)
    : mTension(tension)
{
}

SAnticipateInterpolator::SAnticipateInterpolator()
    : mTension(2.0f)
{
}

//////////////////////////////////////////////////////////////////////////
float SAnticipateOvershootInterpolator::getInterpolation(float t) const
{
    // a(t, s) = t * t * ((s + 1) * t - s)
    // o(t, s) = t * t * ((s + 1) * t + s)
    // f(t) = 0.5 * a(t * 2, tension * extraTension), when t < 0.5
    // f(t) = 0.5 * (o(t * 2 - 2, tension * extraTension) + 2), when t <= 1.0
    if (t < 0.5f)
        return 0.5f * a(t * 2.0f, getTensionMulti());
    else
        return 0.5f * (o(t * 2.0f - 2.0f, getTensionMulti()) + 2.0f);
}

float SAnticipateOvershootInterpolator::o(float t, float s)
{
    return t * t * ((s + 1) * t + s);
}

float SAnticipateOvershootInterpolator::a(float t, float s)
{
    return t * t * ((s + 1) * t - s);
}

SAnticipateOvershootInterpolator::SAnticipateOvershootInterpolator(float tension, float extraTension)
    : mTension(tension)
    , mExtraTension(extraTension)
{
}

//////////////////////////////////////////////////////////////////////////
float SBounceInterpolator::getInterpolation(float t) const
{
    // _b(t) = t * t * 8
    // bs(t) = _b(t) for t < 0.3535
    // bs(t) = _b(t - 0.54719) + 0.7 for t < 0.7408
    // bs(t) = _b(t - 0.8526) + 0.9 for t < 0.9644
    // bs(t) = _b(t - 1.0435) + 0.95 for t <= 1.0
    // b(t) = bs(t * 1.1226)
    t *= 1.1226f;
    if (t < 0.3535f)
        return bounce(t);
    else if (t < 0.7408f)
        return bounce(t - 0.54719f) + 0.7f;
    else if (t < 0.9644f)
        return bounce(t - 0.8526f) + 0.9f;
    else
        return bounce(t - 1.0435f) + 0.95f;
}

float SBounceInterpolator::bounce(float t)
{
    return t * t * 8.0f;
}

//////////////////////////////////////////////////////////////////////////
float SCycleInterpolator::getInterpolation(float input) const
{
    return (float)(sin(2 * mCycles * PI * input));
}

SCycleInterpolator::SCycleInterpolator(float cycles)
    : mCycles(cycles)
{
}

//////////////////////////////////////////////////////////////////////////

float SOvershootInterpolator::getInterpolation(float t) const
{
    // _o(t) = t * t * ((tension + 1) * t + tension)
    // o(t) = _o(t - 1) + 1
    t -= 1.0f;
    return t * t * ((mTension + 1) * t + mTension) + 1.0f;
}

SOvershootInterpolator::SOvershootInterpolator(float tension)
    : mTension(tension)
{
}

//////////////////////////////////////////////////////////////////////////
// Quad interpolators
float SQuadInInterpolator::getInterpolation(float t) const { return t * t; }
float SQuadOutInterpolator::getInterpolation(float t) const { return t * (2.0f - t); }
float SQuadInOutInterpolator::getInterpolation(float t) const {
    if (t < 0.5f) return 2.0f * t * t;
    return -1.0f + (4.0f - 2.0f * t) * t;
}

//////////////////////////////////////////////////////////////////////////
// Cubic interpolators
float SCubicInInterpolator::getInterpolation(float t) const { return t * t * t; }
float SCubicOutInterpolator::getInterpolation(float t) const { float p = t - 1.0f; return p * p * p + 1.0f; }
float SCubicInOutInterpolator::getInterpolation(float t) const {
    if (t < 0.5f) return 4.0f * t * t * t;
    float p = 2.0f * t - 2.0f; return 0.5f * p * p * p + 1.0f;
}

//////////////////////////////////////////////////////////////////////////
// Sine interpolators
float SSineInInterpolator::getInterpolation(float t) const { return (float)(1.0f - cos((t * PI) / 2.0f)); }
float SSineOutInterpolator::getInterpolation(float t) const { return (float)sin((t * PI) / 2.0f); }
float SSineInOutInterpolator::getInterpolation(float t) const { return (float)(-0.5f * (cos(PI * t) - 1.0f)); }

//////////////////////////////////////////////////////////////////////////
// Exponential interpolators
float SExpoInInterpolator::getInterpolation(float t) const { return (t == 0.0f) ? 0.0f : (float)pow(2.0f, 10.0f * (t - 1.0f)); }
float SExpoOutInterpolator::getInterpolation(float t) const { return (t == 1.0f) ? 1.0f : 1.0f - (float)pow(2.0f, -10.0f * t); }
float SExpoInOutInterpolator::getInterpolation(float t) const {
    if (t == 0.0f) return 0.0f;
    if (t == 1.0f) return 1.0f;
    if (t < 0.5f) return 0.5f * (float)pow(2.0f, (20.0f * t) - 10.0f);
    return -0.5f * (float)pow(2.0f, (-20.0f * t) + 10.0f) + 1.0f;
}

//////////////////////////////////////////////////////////////////////////
// Back interpolators
SBackInInterpolator::SBackInInterpolator(float tension)
    : mTension(tension)
{
}

float SBackInInterpolator::getInterpolation(float t) const
{
    return t * t * ((mTension + 1.0f) * t - mTension);
}

SBackOutInterpolator::SBackOutInterpolator(float tension)
    : mTension(tension)
{
}

float SBackOutInterpolator::getInterpolation(float t) const
{
    t = t - 1.0f;
    return t * t * ((mTension + 1.0f) * t + mTension) + 1.0f;
}

SBackInOutInterpolator::SBackInOutInterpolator(float tension)
    : mTension(tension)
{
}

float SBackInOutInterpolator::getInterpolation(float t) const
{
    float s = mTension * 1.525f;
    if (t < 0.5f)
    {
        t *= 2.0f;
        return 0.5f * (t * t * ((s + 1.0f) * t - s));
    }
    else
    {
        t = t * 2.0f - 2.0f;
        return 0.5f * (t * t * ((s + 1.0f) * t + s) + 2.0f);
    }
}

//////////////////////////////////////////////////////////////////////////
// Quint interpolators
float SQuintInInterpolator::getInterpolation(float t) const { return t * t * t * t * t; }
float SQuintOutInterpolator::getInterpolation(float t) const { float p = t - 1.0f; return p * p * p * p * p + 1.0f; }
float SQuintInOutInterpolator::getInterpolation(float t) const {
    if (t < 0.5f) return 16.0f * t * t * t * t * t; // (2t)^5 / 2 = 32 t^5 /2 =16 t^5
    float p = (2.0f * t) - 2.0f; return 0.5f * p * p * p * p * p + 1.0f;
}

SNSEND