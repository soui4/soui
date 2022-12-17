/*
 * Copyright (C) 2006 The Android Open Source Project
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

#include <animation/SAnimation.h>

SNSBEGIN
/**
 * An animation that controls the position of an object. See the
 * {@link android.view.animation full package} description for details and
 * sample code.
 *
 */
class SOUI_EXP STranslateAnimation : public SAnimation {
    DEF_SOBJECT(SAnimation, L"translate")

  protected:
    SValueDescription mFromXValue;
    SValueDescription mToXValue;
    SValueDescription mFromYValue;
    SValueDescription mToYValue;

    float mFromXDelta;
    float mToXDelta;
    float mFromYDelta;
    float mToYDelta;
  public:
    STranslateAnimation();


    /**
     * Constructor to use when building a TranslateAnimation from code
     *
     * @param fromXType Specifies how fromXValue should be interpreted. One of
     *        Animation.ABSOLUTE, Animation.RELATIVE_TO_SELF, or
     *        Animation.RELATIVE_TO_PARENT.
     * @param fromXValue Change in X coordinate to apply at the start of the
     *        animation. This value can either be an absolute number if fromXType
     *        is ABSOLUTE, or a percentage (where 1.0 is 100%) otherwise.
     * @param toXType Specifies how toXValue should be interpreted. One of
     *        Animation.ABSOLUTE, Animation.RELATIVE_TO_SELF, or
     *        Animation.RELATIVE_TO_PARENT.
     * @param toXValue Change in X coordinate to apply at the end of the
     *        animation. This value can either be an absolute number if toXType
     *        is ABSOLUTE, or a percentage (where 1.0 is 100%) otherwise.
     * @param fromYType Specifies how fromYValue should be interpreted. One of
     *        Animation.ABSOLUTE, Animation.RELATIVE_TO_SELF, or
     *        Animation.RELATIVE_TO_PARENT.
     * @param fromYValue Change in Y coordinate to apply at the start of the
     *        animation. This value can either be an absolute number if fromYType
     *        is ABSOLUTE, or a percentage (where 1.0 is 100%) otherwise.
     * @param toYType Specifies how toYValue should be interpreted. One of
     *        Animation.ABSOLUTE, Animation.RELATIVE_TO_SELF, or
     *        Animation.RELATIVE_TO_PARENT.
     * @param toYValue Change in Y coordinate to apply at the end of the
     *        animation. This value can either be an absolute number if toYType
     *        is ABSOLUTE, or a percentage (where 1.0 is 100%) otherwise.
     */
    void init(const SValueDescription & fromX, const SValueDescription & toX, const SValueDescription & fromY,  const SValueDescription & toY);

  protected:
    STDMETHOD_(void, applyTransformation)
    (THIS_ float interpolatedTime, ITransformation *t) OVERRIDE;

  public:
    STDMETHOD_(void, initialize)
    (THIS_ int width, int height, int parentWidth, int parentHeight,int nScale) OVERRIDE;

    SOUI_ATTRS_BEGIN()
        ATTR_VALUE_DESC(L"fromXDelta",  mFromXValue)
        ATTR_VALUE_DESC(L"toXDelta", mToXValue)
        ATTR_VALUE_DESC(L"fromYDelta", mFromYValue)
        ATTR_VALUE_DESC(L"toYDelta",  mToYValue)
    SOUI_ATTRS_END()
};

SNSEND