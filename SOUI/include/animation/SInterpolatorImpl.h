#ifndef __SINTERPOLATORIMPL__H__
#define __SINTERPOLATORIMPL__H__

#include <interface/sinterpolator-i.h>
#include <helper/obj-ref-impl.hpp>

SNSBEGIN

typedef TObjRefImpl<SObjectImpl<IInterpolator>> SInterpolatorBase;

class SOUI_EXP SLinearInterpolator : public SInterpolatorBase {
    DEF_SOBJECT(SInterpolatorBase, L"Linear")
  public:
    STDMETHOD_(float, getInterpolation)(THIS_ float input) SCONST OVERRIDE;
};

class SOUI_EXP SAccelerateInterpolator : public SInterpolatorBase {
    DEF_SOBJECT(SInterpolatorBase, L"Accelerate")

  private:
    float mFactor;
    double mDoubleFactor;

    /**
     * Constructor
     *
     * @param factor Degree to which the animation should be eased. Seting
     *        factor to 1.0f produces a y=x^2 parabola. Increasing factor above
     *        1.0f  exaggerates the ease-in effect (i.e., it starts even
     *        slower and ends evens faster)
     */
  public:
    SAccelerateInterpolator(float factor = 1.0f);

    STDMETHOD_(float, getInterpolation)(THIS_ float input) SCONST OVERRIDE;

    SOUI_ATTRS_BEGIN()
        ATTR_FLOAT(L"factor", mFactor, FALSE)
    SOUI_ATTRS_END()
};

class SOUI_EXP SDecelerateInterpolator : public SInterpolatorBase {
    DEF_SOBJECT(SInterpolatorBase, L"Decelerate")

  private:
    float mFactor;

    /**
     * Constructor
     *
     * @param factor Degree to which the animation should be eased. Setting factor to 1.0f produces
     *        an upside-down y=x^2 parabola. Increasing factor above 1.0f makes exaggerates the
     *        ease-out effect (i.e., it starts even faster and ends evens slower)
     */
  public:
    SDecelerateInterpolator(float factor = 1.0f);

    STDMETHOD_(float, getInterpolation)(THIS_ float input) SCONST OVERRIDE;

    SOUI_ATTRS_BEGIN()
        ATTR_FLOAT(L"factor", mFactor, FALSE)
    SOUI_ATTRS_END()
};

class SOUI_EXP SAccelerateDecelerateInterpolator : public SInterpolatorBase {
    DEF_SOBJECT(SInterpolatorBase, L"AccelerateDecelerate")
  public:
    STDMETHOD_(float, getInterpolation)(THIS_ float input) SCONST OVERRIDE;
};

class SOUI_EXP SAnticipateInterpolator : public SInterpolatorBase {
    DEF_SOBJECT(SInterpolatorBase, L"Anticipate")

  private:
    float mTension;

  public:
    SAnticipateInterpolator();

    /**
     * @param tension Amount of anticipation. When tension equals 0.0f, there is
     *                no anticipation and the interpolator becomes a simple
     *                acceleration interpolator.
     */
    SAnticipateInterpolator(float tension);

    STDMETHOD_(float, getInterpolation)(THIS_ float input) SCONST OVERRIDE;

    SOUI_ATTRS_BEGIN()
        ATTR_FLOAT(L"tension", mTension, FALSE)
    SOUI_ATTRS_END()
};

class SOUI_EXP SAnticipateOvershootInterpolator : public SInterpolatorBase {
    DEF_SOBJECT(SInterpolatorBase, L"AnticipateOvershoot")
  private:
    float mTension;
    float mExtraTension;

    /**
     * @param tension Amount of anticipation/overshoot. When tension equals 0.0f,
     *                there is no anticipation/overshoot and the interpolator becomes
     *                a simple acceleration/deceleration interpolator.
     * @param extraTension Amount by which to multiply the tension. For instance,
     *                     to get the same overshoot as an OvershootInterpolator with
     *                     a tension of 2.0f, you would use an extraTension of 1.5f.
     */
  public:
    SAnticipateOvershootInterpolator(float tension = 2.0f, float extraTension = 1.5f);

  private:
    static float a(float t, float s);
    static float o(float t, float s);
    float getTensionMulti() const
    {
        return mTension * mExtraTension;
    }

  public:
    STDMETHOD_(float, getInterpolation)(THIS_ float input) SCONST OVERRIDE;

    SOUI_ATTRS_BEGIN()
        ATTR_FLOAT(L"tension", mTension, FALSE)
        ATTR_FLOAT(L"extraTension", mExtraTension, FALSE)
    SOUI_ATTRS_END()
};

class SOUI_EXP SBounceInterpolator : public SInterpolatorBase {
    DEF_SOBJECT(SInterpolatorBase, L"Bounce")

  private:
    static float bounce(float t);

  public:
    STDMETHOD_(float, getInterpolation)(THIS_ float input) SCONST OVERRIDE;
};

class SOUI_EXP SCycleInterpolator : public SInterpolatorBase {
    DEF_SOBJECT(SInterpolatorBase, L"Cycle")

  private:
    float mCycles;

  public:
    SCycleInterpolator(float cycles = 1.0f);

    STDMETHOD_(float, getInterpolation)(THIS_ float input) SCONST OVERRIDE;

    SOUI_ATTRS_BEGIN()
        ATTR_FLOAT(L"cycles", mCycles, FALSE)
    SOUI_ATTRS_END()
};

class SOUI_EXP SOvershootInterpolator : public SInterpolatorBase {
    DEF_SOBJECT(SInterpolatorBase, L"Overshoot")

  private:
    float mTension;

    /**
     * @param tension Amount of overshoot. When tension equals 0.0f, there is
     *                no overshoot and the interpolator becomes a simple
     *                deceleration interpolator.
     */
  public:
    SOvershootInterpolator(float tension = 2.0f);

    STDMETHOD_(float, getInterpolation)(THIS_ float input) SCONST OVERRIDE;

    SOUI_ATTRS_BEGIN()
        ATTR_FLOAT(L"tension", mTension, FALSE)
    SOUI_ATTRS_END()
};

SNSEND
#endif // __SINTERPOLATORIMPL__H__