#ifndef __SINTERPOLATORIMPL__H__
#define __SINTERPOLATORIMPL__H__

#include <interface/sinterpolator-i.h>
#include <helper/obj-ref-impl.hpp>
#include <sobject/Sobject.hpp>

SNSBEGIN

typedef TObjRefImpl<SObjectImpl<IInterpolator>> SInterpolatorBase;

/**
 * @class SLinearInterpolator
 * @brief An interpolator where the rate of change is constant.
 */
class SOUI_EXP SLinearInterpolator : public SInterpolatorBase {
    DEF_SOBJECT(SInterpolatorBase, L"Linear")

  public:
    /**
     * @brief Gets the interpolated value for the given input.
     * @param input The input value between 0 and 1.
     * @return The interpolated value.
     */
    STDMETHOD_(float, getInterpolation)(THIS_ float input) SCONST OVERRIDE;
};

/**
 * @class SAccelerateInterpolator
 * @brief An interpolator where the rate of change starts slow and accelerates.
 */
class SOUI_EXP SAccelerateInterpolator : public SInterpolatorBase {
    DEF_SOBJECT(SInterpolatorBase, L"Accelerate")

  private:
    float mFactor;        ///< Degree to which the animation should be eased.
    double mDoubleFactor; ///< Double value of the factor for calculations.

  public:
    /**
     * @brief Constructor for SAccelerateInterpolator.
     * @param factor Degree to which the animation should be eased. Setting factor to 1.0f produces a y=x^2 parabola.
     *               Increasing factor above 1.0f exaggerates the ease-in effect (i.e., it starts even slower and ends even faster).
     */
    SAccelerateInterpolator(float factor = 1.0f);

    /**
     * @brief Gets the interpolated value for the given input.
     * @param input The input value between 0 and 1.
     * @return The interpolated value.
     */
    STDMETHOD_(float, getInterpolation)(THIS_ float input) SCONST OVERRIDE;

    /**
     * @brief Attributes for SAccelerateInterpolator
     */
    SOUI_ATTRS_BEGIN()
        ATTR_FLOAT(L"factor", mFactor, FALSE) ///< Degree to which the animation should be eased.
    SOUI_ATTRS_END()
};

/**
 * @class SDecelerateInterpolator
 * @brief An interpolator where the rate of change starts fast and decelerates.
 */
class SOUI_EXP SDecelerateInterpolator : public SInterpolatorBase {
    DEF_SOBJECT(SInterpolatorBase, L"Decelerate")

  private:
    float mFactor; ///< Degree to which the animation should be eased.

  public:
    /**
     * @brief Constructor for SDecelerateInterpolator.
     * @param factor Degree to which the animation should be eased. Setting factor to 1.0f produces an upside-down y=x^2 parabola.
     *               Increasing factor above 1.0f exaggerates the ease-out effect (i.e., it starts even faster and ends even slower).
     */
    SDecelerateInterpolator(float factor = 1.0f);

    /**
     * @brief Gets the interpolated value for the given input.
     * @param input The input value between 0 and 1.
     * @return The interpolated value.
     */
    STDMETHOD_(float, getInterpolation)(THIS_ float input) SCONST OVERRIDE;

    /**
     * @brief Attributes for SDecelerateInterpolator
     */
    SOUI_ATTRS_BEGIN()
        ATTR_FLOAT(L"factor", mFactor, FALSE) ///< Degree to which the animation should be eased.
    SOUI_ATTRS_END()
};

/**
 * @class SAccelerateDecelerateInterpolator
 * @brief An interpolator where the rate of change starts and ends slowly, accelerating in the middle.
 */
class SOUI_EXP SAccelerateDecelerateInterpolator : public SInterpolatorBase {
    DEF_SOBJECT(SInterpolatorBase, L"AccelerateDecelerate")

  public:
    /**
     * @brief Gets the interpolated value for the given input.
     * @param input The input value between 0 and 1.
     * @return The interpolated value.
     */
    STDMETHOD_(float, getInterpolation)(THIS_ float input) SCONST OVERRIDE;
};

/**
 * @class SAnticipateInterpolator
 * @brief An interpolator where the change starts backward before moving forward.
 */
class SOUI_EXP SAnticipateInterpolator : public SInterpolatorBase {
    DEF_SOBJECT(SInterpolatorBase, L"Anticipate")

  private:
    float mTension; ///< Amount of anticipation.

  public:
    /**
     * @brief Default constructor for SAnticipateInterpolator.
     */
    SAnticipateInterpolator();

    /**
     * @brief Constructor for SAnticipateInterpolator.
     * @param tension Amount of anticipation. When tension equals 0.0f, there is no anticipation and the interpolator becomes a simple acceleration interpolator.
     */
    SAnticipateInterpolator(float tension);

    /**
     * @brief Gets the interpolated value for the given input.
     * @param input The input value between 0 and 1.
     * @return The interpolated value.
     */
    STDMETHOD_(float, getInterpolation)(THIS_ float input) SCONST OVERRIDE;

    /**
     * @brief Attributes for SAnticipateInterpolator
     */
    SOUI_ATTRS_BEGIN()
        ATTR_FLOAT(L"tension", mTension, FALSE) ///< Amount of anticipation.
    SOUI_ATTRS_END()
};

/**
 * @class SAnticipateOvershootInterpolator
 * @brief An interpolator where the change starts backward before moving forward and overshoots the target.
 */
class SOUI_EXP SAnticipateOvershootInterpolator : public SInterpolatorBase {
    DEF_SOBJECT(SInterpolatorBase, L"AnticipateOvershoot")

  private:
    float mTension;      ///< Amount of anticipation/overshoot.
    float mExtraTension; ///< Extra amount by which to multiply the tension.

    /**
     * @brief Helper function for anticipation.
     * @param t Input value.
     * @param s Tension value.
     * @return Calculated anticipation value.
     */
    static float a(float t, float s);

    /**
     * @brief Helper function for overshoot.
     * @param t Input value.
     * @param s Tension value.
     * @return Calculated overshoot value.
     */
    static float o(float t, float s);

    /**
     * @brief Gets the tension multiplier.
     * @return The tension multiplier.
     */
    float getTensionMulti() const
    {
        return mTension * mExtraTension;
    }

  public:
    /**
     * @brief Constructor for SAnticipateOvershootInterpolator.
     * @param tension Amount of anticipation/overshoot. When tension equals 0.0f, there is no anticipation/overshoot and the interpolator becomes a simple acceleration/deceleration interpolator.
     * @param extraTension Extra amount by which to multiply the tension. For instance, to get the same overshoot as an OvershootInterpolator with a tension of 2.0f, you would use an extraTension of 1.5f.
     */
    SAnticipateOvershootInterpolator(float tension = 2.0f, float extraTension = 1.5f);

    /**
     * @brief Gets the interpolated value for the given input.
     * @param input The input value between 0 and 1.
     * @return The interpolated value.
     */
    STDMETHOD_(float, getInterpolation)(THIS_ float input) SCONST OVERRIDE;

    /**
     * @brief Attributes for SAnticipateOvershootInterpolator
     */
    SOUI_ATTRS_BEGIN()
        ATTR_FLOAT(L"tension", mTension, FALSE)           ///< Amount of anticipation/overshoot.
        ATTR_FLOAT(L"extraTension", mExtraTension, FALSE) ///< Extra amount by which to multiply the tension.
    SOUI_ATTRS_END()
};

/**
 * @class SBounceInterpolator
 * @brief An interpolator where the change bounces at the end.
 */
class SOUI_EXP SBounceInterpolator : public SInterpolatorBase {
    DEF_SOBJECT(SInterpolatorBase, L"Bounce")

  private:
    /**
     * @brief Helper function for bounce effect.
     * @param t Input value.
     * @return Calculated bounce value.
     */
    static float bounce(float t);

  public:
    /**
     * @brief Gets the interpolated value for the given input.
     * @param input The input value between 0 and 1.
     * @return The interpolated value.
     */
    STDMETHOD_(float, getInterpolation)(THIS_ float input) SCONST OVERRIDE;
};

/**
 * @class SCycleInterpolator
 * @brief An interpolator where the change repeats a specified number of cycles.
 */
class SOUI_EXP SCycleInterpolator : public SInterpolatorBase {
    DEF_SOBJECT(SInterpolatorBase, L"Cycle")

  private:
    float mCycles; ///< Number of cycles to repeat.

  public:
    /**
     * @brief Constructor for SCycleInterpolator.
     * @param cycles Number of cycles to repeat.
     */
    SCycleInterpolator(float cycles = 1.0f);

    /**
     * @brief Gets the interpolated value for the given input.
     * @param input The input value between 0 and 1.
     * @return The interpolated value.
     */
    STDMETHOD_(float, getInterpolation)(THIS_ float input) SCONST OVERRIDE;

    /**
     * @brief Attributes for SCycleInterpolator
     */
    SOUI_ATTRS_BEGIN()
        ATTR_FLOAT(L"cycles", mCycles, FALSE) ///< Number of cycles to repeat.
    SOUI_ATTRS_END()
};

/**
 * @class SOvershootInterpolator
 * @brief An interpolator where the change overshoots the target and then comes back.
 */
class SOUI_EXP SOvershootInterpolator : public SInterpolatorBase {
    DEF_SOBJECT(SInterpolatorBase, L"Overshoot")

  private:
    float mTension; ///< Amount of overshoot.

  public:
    /**
     * @brief Constructor for SOvershootInterpolator.
     * @param tension Amount of overshoot. When tension equals 0.0f, there is no overshoot and the interpolator becomes a simple deceleration interpolator.
     */
    SOvershootInterpolator(float tension = 2.0f);

    /**
     * @brief Gets the interpolated value for the given input.
     * @param input The input value between 0 and 1.
     * @return The interpolated value.
     */
    STDMETHOD_(float, getInterpolation)(THIS_ float input) SCONST OVERRIDE;

    /**
     * @brief Attributes for SOvershootInterpolator
     */
    SOUI_ATTRS_BEGIN()
        ATTR_FLOAT(L"tension", mTension, FALSE) ///< Amount of overshoot.
    SOUI_ATTRS_END()
};

SNSEND
#endif // __SINTERPOLATORIMPL__H__