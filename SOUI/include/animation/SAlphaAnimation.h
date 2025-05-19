#ifndef __SALPHAANIMATION__H__
#define __SALPHAANIMATION__H__

#include <animation/SAnimation.h>

SNSBEGIN

/**
 * @class SAlphaAnimation
 * @brief An animation that controls the alpha level of an object.
 *
 * Useful for fading things in and out. This animation ends up changing the alpha property of a {@link STransformation}.
 */
class SOUI_EXP SAlphaAnimation : public SAnimation {
    DEF_SOBJECT(SAnimation, L"alpha")

  private:
    float mFromAlpha; ///< Starting alpha value for the animation, where 1.0 means fully opaque and 0.0 means fully transparent.
    float mToAlpha;   ///< Ending alpha value for the animation.

  public:
    /**
     * @brief Attributes for SAlphaAnimation
     */
    SOUI_ATTRS_BEGIN()
        ATTR_FLOAT(L"fromAlpha", mFromAlpha, FALSE) ///< Starting alpha value attribute.
        ATTR_FLOAT(L"toAlpha", mToAlpha, FALSE)     ///< Ending alpha value attribute.
    SOUI_ATTRS_END()

    /**
     * @brief Default constructor
     */
    SAlphaAnimation();

    /**
     * @brief Initializes the alpha animation with specified start and end alpha values
     * @param fromAlpha Starting alpha value for the animation, where 1.0 means fully opaque and 0.0 means fully transparent.
     * @param toAlpha Ending alpha value for the animation.
     */
    void init(float fromAlpha = 0.0f, float toAlpha = 0.0f);

  protected:
    /**
     * @brief Changes the alpha property of the supplied {@link STransformation}
     * @param interpolatedTime The elapsed fraction of the animation's duration.
     * @param t Pointer to the transformation to apply the animation to.
     */
    STDMETHOD_(void, applyTransformation)
    (THIS_ float interpolatedTime, ITransformation *t) OVERRIDE;

    /**
     * @brief Copies the properties of another animation to this animation
     * @param src Pointer to the source animation to copy from.
     */
    STDMETHOD_(void, copy)(THIS_ const IAnimation *src) OVERRIDE;

  public:
    /**
     * @brief Checks if the animation affects the alpha property
     * @return TRUE if the animation affects the alpha property, FALSE otherwise.
     * @hide
     */
    STDMETHOD_(BOOL, hasAlpha)(THIS) SCONST OVERRIDE;
};

SNSEND

#endif // __SALPHAANIMATION__H__