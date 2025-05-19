#ifndef __SRotateAnimation__H__
#define __SRotateAnimation__H__

#include <animation/SAnimation.h>

SNSBEGIN

/**
 * @class SRotateAnimation
 * @brief An animation that controls the rotation of an object. This rotation takes place in the X-Y plane.
 *
 * You can specify the point to use for the center of the rotation, where (0,0) is the top left point.
 * If not specified, (0,0) is the default rotation point.
 */
class SOUI_EXP SRotateAnimation : public SAnimation {
    DEF_SOBJECT(SAnimation, L"rotate")

  private:
    float mFromDegrees; ///< Rotation offset to apply at the start of the animation.
    float mToDegrees;   ///< Rotation offset to apply at the end of the animation.

    SValueDescription mPivotXDesc; ///< Description of the pivot X value.
    SValueDescription mPivotYDesc; ///< Description of the pivot Y value.

    float mPivotX; ///< The X coordinate of the pivot point.
    float mPivotY; ///< The Y coordinate of the pivot point.

  public:
    /**
     * @brief Default constructor for SRotateAnimation.
     * Default pivotX/pivotY point is (0,0).
     */
    SRotateAnimation();

    /**
     * @brief Constructor to use when building a RotateAnimation from code.
     *
     * @param fromDegrees Rotation offset to apply at the start of the animation.
     * @param toDegrees Rotation offset to apply at the end of the animation.
     * @param pivotXType Specifies how pivotXValue should be interpreted. One of Animation.ABSOLUTE, Animation.RELATIVE_TO_SELF, or Animation.RELATIVE_TO_PARENT.
     * @param pivotXValue The X coordinate of the point about which the object is being rotated, specified as an absolute number where 0 is the left edge.
     *                    This value can either be an absolute number if pivotXType is ABSOLUTE, or a percentage (where 1.0 is 100%) otherwise.
     * @param pivotYType Specifies how pivotYValue should be interpreted. One of Animation.ABSOLUTE, Animation.RELATIVE_TO_SELF, or Animation.RELATIVE_TO_PARENT.
     * @param pivotYValue The Y coordinate of the point about which the object is being rotated, specified as an absolute number where 0 is the top edge.
     *                    This value can either be an absolute number if pivotYType is ABSOLUTE, or a percentage (where 1.0 is 100%) otherwise.
     */
    void init(float fromDegrees, float toDegrees, const SValueDescription &pivotX, const SValueDescription &pivotY);

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
     * @brief Attributes for SRotateAnimation
     */
    SOUI_ATTRS_BEGIN()
        ATTR_FLOAT(L"fromDegrees", mFromDegrees, FALSE) ///< Rotation offset to apply at the start of the animation.
        ATTR_FLOAT(L"toDegrees", mToDegrees, FALSE)     ///< Rotation offset to apply at the end of the animation.
        ATTR_VALUE_DESC(L"pivotX", mPivotXDesc)         ///< Description of the pivot X value.
        ATTR_VALUE_DESC(L"pivotY", mPivotYDesc)         ///< Description of the pivot Y value.
    SOUI_ATTRS_END()
};

SNSEND

#endif // __SRotateAnimation__H__