#ifndef __STRANSLATEANIMATION__H__
#define __STRANSLATEANIMATION__H__

#include <animation/SAnimation.h>

SNSBEGIN

/**
 * @class STranslateAnimation
 * @brief An animation that controls the position of an object.
 *
 * See the {@link android.view.animation full package} description for details and sample code.
 */
class SOUI_EXP STranslateAnimation : public SAnimation {
    DEF_SOBJECT(SAnimation, L"translate")

  protected:
    SValueDescription mFromXValue; ///< Description of the starting X value.
    SValueDescription mToXValue;   ///< Description of the ending X value.
    SValueDescription mFromYValue; ///< Description of the starting Y value.
    SValueDescription mToYValue;   ///< Description of the ending Y value.

    float mFromXDelta; ///< The starting X delta value.
    float mToXDelta;   ///< The ending X delta value.
    float mFromYDelta; ///< The starting Y delta value.
    float mToYDelta;   ///< The ending Y delta value.

  public:
    /**
     * @brief Default constructor for STranslateAnimation.
     */
    STranslateAnimation();

    /**
     * @brief Constructor to use when building a TranslateAnimation from code.
     *
     * @param fromX Description of the starting X value. This value can either be an absolute number
     *              if fromXType is ABSOLUTE, or a percentage (where 1.0 is 100%) otherwise.
     * @param toX Description of the ending X value. This value can either be an absolute number
     *            if toXType is ABSOLUTE, or a percentage (where 1.0 is 100%) otherwise.
     * @param fromY Description of the starting Y value. This value can either be an absolute number
     *              if fromYType is ABSOLUTE, or a percentage (where 1.0 is 100%) otherwise.
     * @param toY Description of the ending Y value. This value can either be an absolute number
     *            if toYType is ABSOLUTE, or a percentage (where 1.0 is 100%) otherwise.
     */
    void init(const SValueDescription &fromX, const SValueDescription &toX, const SValueDescription &fromY, const SValueDescription &toY);

  protected:
    /**
     * @brief Applies the transformation at a specific interpolated time.
     * @param interpolatedTime Interpolated time between 0 and 1.
     * @param t Pointer to the transformation to apply.
     */
    STDMETHOD_(void, applyTransformation)
    (THIS_ float interpolatedTime, ITransformation *t) OVERRIDE;

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

    /**
     * @brief Attributes for STranslateAnimation
     */
    SOUI_ATTRS_BEGIN()
        ATTR_VALUE_DESC(L"fromXDelta", mFromXValue) ///< Description of the starting X value.
        ATTR_VALUE_DESC(L"toXDelta", mToXValue)     ///< Description of the ending X value.
        ATTR_VALUE_DESC(L"fromYDelta", mFromYValue) ///< Description of the starting Y value.
        ATTR_VALUE_DESC(L"toYDelta", mToYValue)     ///< Description of the ending Y value.
    SOUI_ATTRS_END()
};

SNSEND

#endif // __STRANSLATEANIMATION__H__