/*
 * Copyright (C) 2010 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/**
 * Interface for use with the {@link ValueAnimator#setEvaluator(TypeEvaluator)} function. Evaluators
 * allow developers to create animations on arbitrary property types, by allowing them to supply
 * custom evaluators for types that are not automatically understood and used by the animation
 * system.
 *
 * @see ValueAnimator#setEvaluator(TypeEvaluator)
 */

#ifndef __TYPEEVALUATOR__H__
#define __TYPEEVALUATOR__H__
#include <math.h>
#include <helper/obj-ref-impl.hpp>

SNSBEGIN

template <typename T>
inline T Evalulate(float fraction, const T &start, const T &end)
{
    return (T)(start + fraction * (end - start));
}

template <typename T>
class TypeEvaluator : public TObjRefImpl<IObjRef> {
  public:
    T mStart;
    T mEnd;

  public:
    TypeEvaluator()
    {
    }
    TypeEvaluator(T start, T end)
    {
        mStart = start;
        mEnd = end;
    }

    void setRange(T start, T end)
    {
        mStart = start;
        mEnd = end;
    }

    void copy(const TypeEvaluator<T> *src)
    {
        mStart = src->mStart;
        mEnd = src->mEnd;
    }

    /**
     * This function returns the result of linearly interpolating the start and end values, with
     * <code>fraction</code> representing the proportion between the start and end values. The
     * calculation is a simple parametric calculation: <code>result = x0 + t * (x1 - x0)</code>,
     * where <code>x0</code> is <code>startValue</code>, <code>x1</code> is <code>endValue</code>,
     * and <code>t</code> is <code>fraction</code>.
     *
     * @param fraction   The fraction from the starting to the ending values
     * @return A linear interpolation between the start and end values, given the
     *         <code>fraction</code> parameter.
     */
    T evaluate(float fraction) const
    {
        return Evalulate(fraction, mStart, mEnd);
    }
};

template <>
class TypeEvaluator<COLORREF> {
  public:
    float startA;
    float startR;
    float startG;
    float startB;

    float endA;
    float endR;
    float endG;
    float endB;

    /**
     * This function returns the calculated in-between value for a color
     * given integers that represent the start and end values in the four
     * bytes of the 32-bit int. Each channel is separately linearly interpolated
     * and the resulting calculated values are recombined into the return value.
     *
     * @param fraction The fraction from the starting to the ending values
     * @param startValue A 32-bit int value representing colors in the
     * separate bytes of the parameter
     * @param endValue A 32-bit int value representing colors in the
     * separate bytes of the parameter
     * @return A value that is calculated to be the linearly interpolated
     * result, derived by separating the start and end values into separate
     * color channels and interpolating each one separately, recombining the
     * resulting values in the same way.
     */
  public:
    TypeEvaluator(COLORREF start, COLORREF end)
    {
        setStart(start);
        setEnd(end);
    }
    void setStart(COLORREF start)
    {
        startA = GetAValue(start) / 255.0f;
        startR = GetRValue(start) / 255.0f;
        startG = GetGValue(start) / 255.0f;
        startB = GetBValue(start) / 255.0f;
        // convert from sRGB to linear
        startR = (float)pow(startR, 2.2f);
        startG = (float)pow(startG, 2.2f);
        startB = (float)pow(startB, 2.2f);
    }

    void setEnd(COLORREF end)
    {
        endA = GetAValue(end) / 255.0f;
        endR = GetRValue(end) / 255.0f;
        endG = GetGValue(end) / 255.0f;
        endB = GetBValue(end) / 255.0f;
        // convert from sRGB to linear
        endR = (float)pow(endR, 2.2f);
        endG = (float)pow(endG, 2.2f);
        endB = (float)pow(endB, 2.2f);
    }

    int round(float v) const
    {
        return (int)floor(v + 0.5f);
    }

    COLORREF evaluate(float fraction) const
    {
        // compute the interpolated color in linear space
        float a = startA + fraction * (endA - startA);
        float r = startR + fraction * (endR - startR);
        float g = startG + fraction * (endG - startG);
        float b = startB + fraction * (endB - startB);

        // convert back to sRGB in the [0..255] range
        a = a * 255.0f;
        r = (float)pow(r, 1.0f / 2.2f) * 255.0f;
        g = (float)pow(g, 1.0f / 2.2f) * 255.0f;
        b = (float)pow(b, 1.0f / 2.2f) * 255.0f;

        return RGBA(round(r), round(g), round(b), round(a));
    }

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

    void setRange(COLORREF from, COLORREF to)
    {
        setStart(from);
        setEnd(to);
    }
};

template <>
inline RECT Evalulate(float fraction, const RECT &mStart, const RECT &mEnd)
{
    RECT ret;
    ret.left = Evalulate(fraction, mStart.left, mEnd.left);
    ret.top = Evalulate(fraction, mStart.top, mEnd.top);
    ret.right = Evalulate(fraction, mStart.right, mEnd.right);
    ret.bottom = Evalulate(fraction, mStart.bottom, mEnd.bottom);
    return ret;
}

template <>
inline POINT Evalulate(float fraction, const POINT &mStart, const POINT &mEnd)
{
    POINT ret;
    ret.x = Evalulate(fraction, mStart.x, mEnd.x);
    ret.y = Evalulate(fraction, mStart.y, mEnd.y);
    return ret;
}

template <>
inline SIZE Evalulate(float fraction, const SIZE &mStart, const SIZE &mEnd)
{
    SIZE ret;
    ret.cx = Evalulate(fraction, mStart.cx, mEnd.cx);
    ret.cy = Evalulate(fraction, mStart.cy, mEnd.cy);
    return ret;
}

SNSEND
#endif // __TYPEEVALUATOR__H__