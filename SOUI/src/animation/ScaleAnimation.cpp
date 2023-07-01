
#include <souistd.h>
#include <animation/ScaleAnimation.h>

SNSBEGIN

void SScaleAnimation::initialize(int width, int height, int parentWidth, int parentHeight, int nScale)
{
    mPivotX = (float)resolveSize(mPivotXDesc, width, parentWidth, nScale);
    mPivotY = (float)resolveSize(mPivotYDesc, height, parentHeight, nScale);
}

void SScaleAnimation::applyTransformation(float interpolatedTime, ITransformation *t)
{
    float sx = 1.0f;
    float sy = 1.0f;
    float scale = getScaleFactor();

    if (mFromX != 1.0f || mToX != 1.0f)
    {
        sx = mFromX + ((mToX - mFromX) * interpolatedTime);
    }
    if (mFromY != 1.0f || mToY != 1.0f)
    {
        sy = mFromY + ((mToY - mFromY) * interpolatedTime);
    }

    if (mPivotX == 0 && mPivotY == 0)
    {
        t->GetMatrix()->setScale(sx, sy);
    }
    else
    {
        t->GetMatrix()->setScale2(sx, sy, scale * mPivotX, scale * mPivotY);
    }
    t->SetTransformationType(TYPE_MATRIX);
}

void SScaleAnimation::copy(const IAnimation *src)
{
    const SScaleAnimation *src2 = sobj_cast<const SScaleAnimation>(src);
    if (!src2)
        return;
    SAnimation::copy(src);
    mFromX = src2->mFromX;
    mToX = src2->mToX;
    mFromY = src2->mFromY;
    mToY = src2->mToY;

    mPivotXDesc = src2->mPivotXDesc;
    mPivotYDesc = src2->mPivotYDesc;
}

void SScaleAnimation::init(float fromX, float toX, float fromY, float toY, const SValueDescription &pivotX, const SValueDescription &pivotY)
{
    mFromX = fromX;
    mToX = toX;
    mFromY = fromY;
    mToY = toY;

    mPivotXDesc = pivotX;
    mPivotYDesc = pivotY;
}

SScaleAnimation::SScaleAnimation()
{
    init(1.0f, 1.0f, 1.0f, 1.0f, SValueDescription(RELATIVE_TO_SELF, 0.5f), SValueDescription(RELATIVE_TO_SELF, 0.5f));
}

SNSEND