
#include <souistd.h>
#include <animation/STranslateAnimation.h>
SNSBEGIN

void STranslateAnimation::initialize(int width, int height, int parentWidth, int parentHeight, int nScale)
{
    mFromXDelta = (float)resolveSize(mFromXValue, width, parentWidth, nScale);
    mToXDelta = (float)resolveSize(mToXValue, width, parentWidth, nScale);
    mFromYDelta = (float)resolveSize(mFromYValue, height, parentHeight, nScale);
    mToYDelta = (float)resolveSize(mToYValue, height, parentHeight, nScale);
}

void STranslateAnimation::applyTransformation(float interpolatedTime, ITransformation *t)
{
    float dx = mFromXDelta;
    float dy = mFromYDelta;
    if (mFromXDelta != mToXDelta)
    {
        dx = mFromXDelta + ((mToXDelta - mFromXDelta) * interpolatedTime);
    }
    if (mFromYDelta != mToYDelta)
    {
        dy = mFromYDelta + ((mToYDelta - mFromYDelta) * interpolatedTime);
    }
    t->GetMatrix()->setTranslate(dx, dy);
    t->SetTransformationType(TYPE_MATRIX);
}

void STranslateAnimation::copy(const IAnimation *src)
{
    const STranslateAnimation *src2 = sobj_cast<const STranslateAnimation>(src);
    if (!src2)
        return;
    SAnimation::copy(src);
    mFromXValue = src2->mFromXValue;
    mToXValue   = src2->mToXValue;
    mFromYValue = src2->mFromYValue;
    mToYValue   = src2->mToYValue;
}

IAnimation *STranslateAnimation::clone() const
{
    STranslateAnimation *pRet = new STranslateAnimation();
    pRet->copy(this);
    return pRet;
}

void STranslateAnimation::init(const SValueDescription &fromX, const SValueDescription &toX, const SValueDescription &fromY, const SValueDescription &toY)
{
    mFromXValue = fromX;
    mToXValue = toX;
    mFromYValue = fromY;
    mToYValue = toY;
}

STranslateAnimation::STranslateAnimation()
{
    mFromXValue = mToXValue = mFromYValue = mToYValue = SValueDescription();
}

SNSEND