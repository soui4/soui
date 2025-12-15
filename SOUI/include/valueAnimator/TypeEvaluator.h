/**
 * @file TypeEvaluator.h
 * @brief Interface for use with the ValueAnimator::setEvaluator(TypeEvaluator) function.
 * @details Evaluators allow developers to create animations on arbitrary property types,
 * by allowing them to supply custom evaluators for types that are not automatically
 * understood and used by the animation system.
 *
 * @see ValueAnimator::setEvaluator(TypeEvaluator)
 */

#ifndef __TYPEEVALUATOR__H__
#define __TYPEEVALUATOR__H__

#include <math.h>
#include <helper/obj-ref-impl.hpp>
#include <helper/SColor.h>

SNSBEGIN

/**
 * @brief Template function for linearly interpolating values.
 * @tparam T The type of the values to interpolate.
 * @param fraction The fraction from the starting to the ending values.
 * @param start The starting value.
 * @param end The ending value.
 * @return The linearly interpolated value.
 */
template <typename T>
inline T Evaluate(float fraction, const T &start, const T &end)
{
    return (T)(start + fraction * (end - start));
}

/**
 * @class TypeEvaluator
 * @brief Template class for evaluating interpolated values between a start and end value.
 * @tparam T The type of the values to evaluate.
 */
template <typename T>
class TypeEvaluator : public TObjRefImpl<IObjRef> {
  public:
    /**
     * @brief Starting value for the interpolation.
     */
    T mStart;

    /**
     * @brief Ending value for the interpolation.
     */
    T mEnd;

  public:
    /**
     * @brief Default constructor.
     */
    TypeEvaluator()
    {
        memset(&mStart,0, sizeof(T));
        memset(&mEnd, 0, sizeof(T));
    }

    /**
     * @brief Constructor initializing the start and end values.
     * @param start The starting value.
     * @param end The ending value.
     */
    TypeEvaluator(T start, T end)
        : mStart(start)
        , mEnd(end)
    {
    }

    /**
     * @brief Sets the range for the interpolation.
     * @param start The starting value.
     * @param end The ending value.
     */
    void setRange(T start, T end)
    {
        mStart = start;
        mEnd = end;
    }

    /**
     * @brief Copies the properties of another TypeEvaluator.
     * @param src The source TypeEvaluator to copy from.
     */
    void copy(const TypeEvaluator<T> *src)
    {
        mStart = src->mStart;
        mEnd = src->mEnd;
    }

    /**
     * @brief Evaluates the interpolated value between the start and end values.
     * @param fraction The fraction from the starting to the ending values.
     * @return The linearly interpolated value.
     */
    T evaluate(float fraction) const
    {
        return Evaluate(fraction, mStart, mEnd);
    }
};

/**
 * @class TypeEvaluator<COLORREF>
 * @brief Specialized class for evaluating interpolated values between two COLORREF values.
 */
template <>
class TypeEvaluator<COLORREF> {
  public:
    /**
     * @brief Starting alpha value.
     */
    float startA;

    /**
     * @brief Starting red value.
     */
    float startR;

    /**
     * @brief Starting green value.
     */
    float startG;

    /**
     * @brief Starting blue value.
     */
    float startB;

    /**
     * @brief Ending alpha value.
     */
    float endA;

    /**
     * @brief Ending red value.
     */
    float endR;

    /**
     * @brief Ending green value.
     */
    float endG;

    /**
     * @brief Ending blue value.
     */
    float endB;

  public:
    /**
     * @brief Constructor initializing the start and end COLORREF values.
     * @param start The starting COLORREF value.
     * @param end The ending COLORREF value.
     */
    TypeEvaluator(COLORREF start, COLORREF end)
    {
        setStart(start);
        setEnd(end);
    }

    /**
     * @brief Sets the starting COLORREF value.
     * @param start The starting COLORREF value.
     */
    void setStart(COLORREF start)
    {
        startA = GetAValue(start) / 255.0f;
        startR = GetRValue(start) / 255.0f;
        startG = GetGValue(start) / 255.0f;
        startB = GetBValue(start) / 255.0f;
        // Convert from sRGB to linear
        startR = (float)pow(startR, 2.2f);
        startG = (float)pow(startG, 2.2f);
        startB = (float)pow(startB, 2.2f);
    }

    /**
     * @brief Sets the ending COLORREF value.
     * @param end The ending COLORREF value.
     */
    void setEnd(COLORREF end)
    {
        endA = GetAValue(end) / 255.0f;
        endR = GetRValue(end) / 255.0f;
        endG = GetGValue(end) / 255.0f;
        endB = GetBValue(end) / 255.0f;
        // Convert from sRGB to linear
        endR = (float)pow(endR, 2.2f);
        endG = (float)pow(endG, 2.2f);
        endB = (float)pow(endB, 2.2f);
    }

    /**
     * @brief Rounds a float value to the nearest integer.
     * @param v The float value to round.
     * @return The rounded integer value.
     */
    int round(float v) const
    {
        return (int)floor(v + 0.5f);
    }

    /**
     * @brief Evaluates the interpolated COLORREF value between the start and end values.
     * @param fraction The fraction from the starting to the ending values.
     * @return The interpolated COLORREF value.
     */
    COLORREF evaluate(float fraction) const
    {
        // Compute the interpolated color in linear space
        float a = startA + fraction * (endA - startA);
        float r = startR + fraction * (endR - startR);
        float g = startG + fraction * (endG - startG);
        float b = startB + fraction * (endB - startB);

        // Convert back to sRGB in the [0..255] range
        a = a * 255.0f;
        r = (float)pow(r, 1.0f / 2.2f) * 255.0f;
        g = (float)pow(g, 1.0f / 2.2f) * 255.0f;
        b = (float)pow(b, 1.0f / 2.2f) * 255.0f;

        return RGBA(round(r), round(g), round(b), round(a));
    }

    /**
     * @brief Copies the properties of another TypeEvaluator<COLORREF>.
     * @param src The source TypeEvaluator<COLORREF> to copy from.
     */
    void copy(const TypeEvaluator<COLORREF> *src)
    {
        startA = src->startA;
        startR = src->startR;
        startG = src->startG;
        startB = src->startB;

        endA = src->endA;
        endR = src->endR;
        endG = src->endG;
        endB = src->endB;
    }

    /**
     * @brief Sets the range for the interpolation.
     * @param from The starting COLORREF value.
     * @param to The ending COLORREF value.
     */
    void setRange(COLORREF from, COLORREF to)
    {
        setStart(from);
        setEnd(to);
    }
};

/**
 * @brief Template specialization for linearly interpolating RECT values.
 * @param fraction The fraction from the starting to the ending values.
 * @param mStart The starting RECT value.
 * @param mEnd The ending RECT value.
 * @return The linearly interpolated RECT value.
 */
template <>
inline RECT Evaluate(float fraction, const RECT &mStart, const RECT &mEnd)
{
    RECT ret;
    ret.left = Evaluate(fraction, mStart.left, mEnd.left);
    ret.top = Evaluate(fraction, mStart.top, mEnd.top);
    ret.right = Evaluate(fraction, mStart.right, mEnd.right);
    ret.bottom = Evaluate(fraction, mStart.bottom, mEnd.bottom);
    return ret;
}

/**
 * @brief Template specialization for linearly interpolating POINT values.
 * @param fraction The fraction from the starting to the ending values.
 * @param mStart The starting POINT value.
 * @param mEnd The ending POINT value.
 * @return The linearly interpolated POINT value.
 */
template <>
inline POINT Evaluate(float fraction, const POINT &mStart, const POINT &mEnd)
{
    POINT ret;
    ret.x = Evaluate(fraction, mStart.x, mEnd.x);
    ret.y = Evaluate(fraction, mStart.y, mEnd.y);
    return ret;
}

/**
 * @brief Template specialization for linearly interpolating SIZE values.
 * @param fraction The fraction from the starting to the ending values.
 * @param mStart The starting SIZE value.
 * @param mEnd The ending SIZE value.
 * @return The linearly interpolated SIZE value.
 */
template <>
inline SIZE Evaluate(float fraction, const SIZE &mStart, const SIZE &mEnd)
{
    SIZE ret;
    ret.cx = Evaluate(fraction, mStart.cx, mEnd.cx);
    ret.cy = Evaluate(fraction, mStart.cy, mEnd.cy);
    return ret;
}

SNSEND

#endif // __TYPEEVALUATOR__H__