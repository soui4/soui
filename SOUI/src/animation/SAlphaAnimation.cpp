
#include <souistd.h>
#include <animation/SAlphaAnimation.h>

SNSBEGIN

SAlphaAnimation::SAlphaAnimation()
    : mFromAlpha(0.f)
    , mToAlpha(0.0f)
{
}

void SAlphaAnimation::init(float fromAlpha, float toAlpha)
{
    mFromAlpha = fromAlpha;
    mToAlpha = toAlpha;
}

BOOL SAlphaAnimation::hasAlpha() const
{
    return true;
}

void SAlphaAnimation::applyTransformation(float interpolatedTime, ITransformation *t)
{
    float alpha = mFromAlpha;
    t->SetAlpha((BYTE)((alpha + ((mToAlpha - alpha) * interpolatedTime)) * 255));
}

void SAlphaAnimation::copy(const IAnimation *src)
{
    const SAlphaAnimation *src2 = sobj_cast<const SAlphaAnimation>(src);
    if (!src2)
        return;
    SAnimation::copy(src);
    mFromAlpha = src2->mFromAlpha;
    mToAlpha = src2->mToAlpha;
}

SNSEND