#ifndef __SANIMATIONSET__H__
#define __SANIMATIONSET__H__

#include <animation/SAnimation.h>
#include <souicoll.h>

/**
 * @brief Represents a group of Animations that should be played together.
 *
 * The transformation of each individual animation are composed together into a single transform.
 * If AnimationSet sets any properties that its children also set (for example, duration or fillBefore),
 * the values of AnimationSet override the child values.
 *
 * <p>The way that AnimationSet inherits behavior from Animation is important to understand.
 * Some of the Animation attributes applied to AnimationSet affect the AnimationSet itself,
 * some are pushed down to the children, and some are ignored, as follows:
 * <ul>
 *     <li>duration, repeatMode, fillBefore, fillAfter: These properties, when set on an AnimationSet object,
 *     will be pushed down to all child animations.</li>
 *     <li>repeatCount, fillEnabled: These properties are ignored for AnimationSet.</li>
 *     <li>startOffset, shareInterpolator: These properties apply to the AnimationSet itself.</li>
 * </ul>
 * Starting with {@link android.os.Build.VERSION_CODES#ICE_CREAM_SANDWICH}, the behavior of these properties
 * is the same in XML resources and at runtime (prior to that release, the values set in XML were ignored
 * for AnimationSet). That is, calling <code>setDuration(500)</code> on an AnimationSet has the same effect
 * as declaring <code>android:duration="500"</code> in an XML resource for an AnimationSet object.</p>
 */

SNSBEGIN

class SOUI_EXP SAnimationSet : public SAnimation {
    DEF_SOBJECT(SAnimation, L"set")

    enum
    {
        PROPERTY_FILL_AFTER_MASK = 0x1,
        PROPERTY_FILL_BEFORE_MASK = 0x2,
        PROPERTY_DURATION_MASK = 0x04,
        PROPERTY_SHARE_INTERPOLATOR_MASK = 0x08,
    };

  private:
    int mFlags;                                  ///< Flags to manage properties like fillAfter, fillBefore, duration, and shareInterpolator.
    bool mDirty;                                 ///< Indicates if the animation set needs to be reinitialized.
    bool mHasAlpha;                              ///< Indicates if any child animation affects the alpha property.
    SArray<SAutoRefPtr<IAnimation>> mAnimations; ///< Array of child animations.
    bool mChildStarted;                          ///< Indicates if any child animation has started.
    long mLastEnd;                               ///< The end time of the last child animation.

  public:
    /**
     * @brief Constructor to use when building an AnimationSet from code.
     * @param shareInterpolator Pass true if all of the animations in this set should use the interpolator
     *                          associated with this AnimationSet. Pass false if each animation should use its own interpolator.
     */
    SAnimationSet(bool shareInterpolator = true);

    /**
     * @brief Add a child animation to this animation set.
     * The transforms of the child animations are applied in the order that they were added.
     * @param a Pointer to the animation to add.
     */
    void addAnimation(IAnimation *a);

    /**
     * @brief Checks if any child animation affects the alpha property.
     * @return TRUE if any child animation affects the alpha property, FALSE otherwise.
     */
    bool hasAlpha();

  private:
    /**
     * @brief Sets a specific flag.
     * @param mask The flag mask to set.
     * @param value The value to set for the flag.
     */
    void setFlag(int mask, bool value);

    /**
     * @brief Initializes the animation set.
     */
    void init();

  public:
    /**
     * @brief Sets whether the animation transformation should be applied after the animation ends.
     * @param bFill Boolean indicating whether to fill after.
     */
    STDMETHOD_(void, setFillAfter)(THIS_ BOOL bFill) OVERRIDE;

    /**
     * @brief Sets whether the animation transformation should be applied before the animation starts.
     * @param bFill Boolean indicating whether to fill before.
     */
    STDMETHOD_(void, setFillBefore)(THIS_ BOOL bFill) OVERRIDE;

    /**
     * @brief Sets the duration of every child animation.
     * @param durationMillis The duration of the animation, in milliseconds, for every child in this set.
     */
    STDMETHOD_(void, setDuration)(THIS_ long durationMillis) OVERRIDE;

    /**
     * @brief Gets the duration of the AnimationSet.
     * The duration of an AnimationSet is defined to be the duration of the longest child animation.
     * @return The duration in milliseconds.
     */
    STDMETHOD_(long, getDuration)(THIS) SCONST OVERRIDE;

    /**
     * @brief Gets the transformation at a specific time.
     * The transformation of an animation set is the concatenation of all of its component animations.
     * @param currentTime Current time in milliseconds.
     * @param outTransformation Pointer to the transformation to apply.
     * @return TRUE if the transformation is valid, FALSE otherwise.
     */
    STDMETHOD_(BOOL, getTransformation)
    (THIS_ uint64_t currentTime, ITransformation *outTransformation) OVERRIDE;

    /**
     * @brief Scales the current duration of the animation.
     * @param scale Scale factor.
     */
    STDMETHOD_(void, scaleCurrentDuration)(THIS_ float scale) OVERRIDE;

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
     * @brief Copies the properties of another animation to this animation.
     * @param src Pointer to the source animation to copy from.
     */
    STDMETHOD_(void, copy)(THIS_ const IAnimation *src) OVERRIDE;

    /**
     * @brief Pauses the animation.
     */
    STDMETHOD_(void, pause)(THIS) OVERRIDE;

    /**
     * @brief Resumes the animation.
     */
    STDMETHOD_(void, resume)(THIS) OVERRIDE;

  protected:
    /**
     * @brief Initializes the animation set from an XML node.
     * @param pNode Pointer to the XML node.
     * @return HRESULT indicating success or failure.
     */
    STDMETHOD_(BOOL, InitFromXml)(THIS_ IXmlNode *pNode) OVERRIDE;

    /**
     * @brief Custom attribute handler for duration.
     * @param value The duration value as a string.
     * @param bLoading Boolean indicating if the attribute is being loaded.
     * @return HRESULT indicating success or failure.
     */
    HRESULT OnAttrDuration(const SStringW &value, BOOL bLoading);

    /**
     * @brief Custom attribute handler for fillBefore.
     * @param value The fillBefore value as a string.
     * @param bLoading Boolean indicating if the attribute is being loaded.
     * @return HRESULT indicating success or failure.
     */
    HRESULT OnAttrFillBefore(const SStringW &value, BOOL bLoading);

    /**
     * @brief Custom attribute handler for fillAfter.
     * @param value The fillAfter value as a string.
     * @param bLoading Boolean indicating if the attribute is being loaded.
     * @return HRESULT indicating success or failure.
     */
    HRESULT OnAttrFillAfter(const SStringW &value, BOOL bLoading);

    /**
     * @brief Custom attribute handler for startOffset.
     * @param value The startOffset value as a string.
     * @param bLoading Boolean indicating if the attribute is being loaded.
     * @return HRESULT indicating success or failure.
     */
    HRESULT OnAttrStartOffset(const SStringW &value, BOOL bLoading);

    /**
     * @brief Attributes for SAnimationSet
     */
    SOUI_ATTRS_BEGIN()
        ATTR_BIT(L"shareInterpolator", mFlags, PROPERTY_SHARE_INTERPOLATOR_MASK, FALSE) ///< Whether to share the interpolator with child animations.
        ATTR_CUSTOM(L"duration", OnAttrDuration)                                        ///< Duration of the animation in milliseconds.
        ATTR_CUSTOM(L"fillBefore", OnAttrFillBefore)                                    ///< Whether the animation transformation should be applied before the animation starts.
        ATTR_CUSTOM(L"fillAfter", OnAttrFillAfter)                                      ///< Whether the animation transformation should be applied after the animation ends.
        ATTR_CUSTOM(L"startOffset", OnAttrStartOffset)                                  ///< Delay before the animation starts in milliseconds.
    SOUI_ATTRS_END()
};

SNSEND

#endif // __SANIMATIONSET__H__