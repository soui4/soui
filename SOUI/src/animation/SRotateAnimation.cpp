
#include <souistd.h>
#include <animation/SRotateAnimation.h>

SNSBEGIN
SRotateAnimation::SRotateAnimation()
{
    mFromDegrees = 0.0f;
    mToDegrees = 0.0f;
    mPivotX = 0.0f;
    mPivotY = 0.0f;
}

void SRotateAnimation::initialize(int width, int height, int parentWidth, int parentHeight, int nScale)
{
    mPivotX = (float)resolveSize(mPivotXDesc, width, parentWidth, nScale);
    mPivotY = (float)resolveSize(mPivotYDesc, height, parentHeight, nScale);
}

void SRotateAnimation::applyTransformation(float interpolatedTime, ITransformation *t)
{
    float degrees = mFromDegrees + ((mToDegrees - mFromDegrees) * interpolatedTime);
    float scale = getScaleFactor();

    if (mPivotX == 0.0f && mPivotY == 0.0f)
    {
        t->GetMatrix()->setRotate(degrees);
    }
    else
    {
        t->GetMatrix()->setRotate2(degrees, mPivotX * scale, mPivotY * scale);
    }
    t->SetTransformationType(TYPE_MATRIX);
}

void SRotateAnimation::copy(const IAnimation *src)
{
    const SRotateAnimation *src2 = sobj_cast<const SRotateAnimation>(src);
    if (!src2)
        return;
    SAnimation::copy(src);
    mFromDegrees = src2->mFromDegrees;
    mToDegrees = src2->mToDegrees;
    mPivotXDesc = src2->mPivotXDesc;
    mPivotYDesc = src2->mPivotYDesc;
}

void SRotateAnimation::init(float fromDegrees, float toDegrees, const SValueDescription &pivotX, const SValueDescription &pivotY)
{
    mFromDegrees = fromDegrees;
    mToDegrees = toDegrees;

    mPivotXDesc = pivotX;
    mPivotYDesc = pivotY;
}

SNSEND