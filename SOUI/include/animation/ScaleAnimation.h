#ifndef __SCALEANIMATION__H__
#define __SCALEANIMATION__H__

#include <animation/SAnimation.h>

SNSBEGIN

/**
 * @class SScaleAnimation
 * @brief An animation that controls the scale of an object. You can specify the point to use for the center of scaling.
 */
class SOUI_EXP SScaleAnimation : public SAnimation {
    DEF_SOBJECT(SAnimation, L"scale")

  private:
    float mFromX; ///< Horizontal scaling factor to apply at the start of the animation.
    float mToX;   ///< Horizontal scaling factor to apply at the end of the animation.
    float mFromY; ///< Vertical scaling factor to apply at the start of the animation.
    float mToY;   ///< Vertical scaling factor to apply at the end of the animation.

    SValueDescription mPivotXDesc; ///< Description of the pivot X value.
    SValueDescription mPivotYDesc; ///< Description of the pivot Y value.

    float mPivotX; ///< The X coordinate of the pivot point.
    float mPivotY; ///< The Y coordinate of the pivot point.

  public:
    /**
     * @brief Default constructor for SScaleAnimation.
     */
    SScaleAnimation();

    /**
     * @brief Constructor to use when building a ScaleAnimation from code.
     *
     * @param fromX Horizontal scaling factor to apply at the start of the animation.
     * @param toX Horizontal scaling factor to apply at the end of the animation.
     * @param fromY Vertical scaling factor to apply at the start of the animation.
     * @param toY Vertical scaling factor to apply at the end of the animation.
     * @param pivotXType Specifies how pivotXValue should be interpreted. One of Animation.ABSOLUTE, Animation.RELATIVE_TO_SELF, or Animation.RELATIVE_TO_PARENT.
     * @param pivotXValue The X coordinate of the point about which the object is being scaled, specified as an absolute number where 0 is the left edge.
     *                    (This point remains fixed while the object changes size.) This value can either be an absolute number if pivotXType is ABSOLUTE,
     *                    or a percentage (where 1.0 is 100%) otherwise.
     * @param pivotYType Specifies how pivotYValue should be interpreted. One of Animation.ABSOLUTE, Animation.RELATIVE_TO_SELF, or Animation.RELATIVE_TO_PARENT.
     * @param pivotYValue The Y coordinate of the point about which the object is being scaled, specified as an absolute number where 0 is the top edge.
     *                    (This point remains fixed while the object changes size.) This value can either be an absolute number if pivotYType is ABSOLUTE,
     *                    or a percentage (where 1.0 is 100%) otherwise.
     */
    void init(float fromX, float toX, float fromY, float toY, const SValueDescription &pivotX, const SValueDescription &pivotY);

  protected:
    /**
     * @brief Applies the transformation at a specific interpolated time.
     * @param interpolatedTime Interpolated time between 0 and 1.
     * @param t Pointer to the transformation to apply.
     */
    STDMETHOD_(void, applyTransformation)
    (THIS_ float interpolatedTime, ITransformation *t) OVERRIDE;

    /**
     * @brief Copies the properties of another animation to this animation.
     * @param src Pointer to the source animation to copy from.
     */
    STDMETHOD_(void, copy)(THIS_ const IAnimation *src) OVERRIDE;

  public:
    /**
     * @brief Initializes the animation with the dimensions of the object and its parent.
     * @param width Width of the object being animated.
     * @param height Height of the object being animated.
     * @param parentWidth Width of the parent of the object being animated.
     * @param parentHeight Height of the parent of the object being animated.
     * @param nScale Scale factor.
     */
    STDMETHOD_(void, initialize)
    (THIS_ int width, int height, int parentWidth, int parentHeight, int nScale) OVERRIDE;

public:
    /**
     * @brief Attributes for SScaleAnimation
     */
    SOUI_ATTRS_BEGIN()
        ATTR_FLOAT(L"fromScale", mFromX=mFromY, FALSE)
        ATTR_FLOAT(L"toScale", mToX=mToY, FALSE)
        ATTR_FLOAT(L"fromXScale", mFromX, FALSE) ///< Horizontal scaling factor to apply at the start of the animation.
        ATTR_FLOAT(L"toXScale", mToX, FALSE)     ///< Horizontal scaling factor to apply at the end of the animation.
        ATTR_FLOAT(L"fromYScale", mFromY, FALSE) ///< Vertical scaling factor to apply at the start of the animation.
        ATTR_FLOAT(L"toYScale", mToY, FALSE)     ///< Vertical scaling factor to apply at the end of the animation.
        ATTR_VALUE_DESC(L"pivotX", mPivotXDesc)  ///< Description of the pivot X value.
        ATTR_VALUE_DESC(L"pivotY", mPivotYDesc)  ///< Description of the pivot Y value.
    SOUI_ATTRS_END()
};

SNSEND

#endif // __SCALEANIMATION__H__