#ifndef __SVALUEANIMATORPROXY__H__
#define __SVALUEANIMATORPROXY__H__
#include <valueAnimator/SValueAnimator.h>
SNSBEGIN

template <class T>
class TValueAnimatorProxy
	: public T
	, public SValueAnimator {
public://proxy for IObjRef
	STDMETHOD_(long, AddRef)(THIS) OVERRIDE
	{
		return SValueAnimator::AddRef();
	}
	STDMETHOD_(long, Release)(THIS) OVERRIDE
	{
		return SValueAnimator::Release();
	}
	STDMETHOD_(void, OnFinalRelease)(THIS) OVERRIDE
	{
		return SValueAnimator::OnFinalRelease();
	}

public://proxy for IObject
    STDMETHOD_(int, GetObjectType)(CTHIS) SCONST OVERRIDE{
        return SValueAnimator::GetObjectType();
    }
    STDMETHOD_(BOOL, InitFromXml)(THIS_ IXmlNode * pXmlNode) OVERRIDE {
        return SValueAnimator::InitFromXml(pXmlNode);
    }
    
    STDMETHOD_(LPCWSTR, GetName)(THIS) SCONST OVERRIDE {
        return SValueAnimator::GetName();
    }

    STDMETHOD_(LPCSTR, GetNameA)(THIS) SCONST OVERRIDE {
        return SValueAnimator::GetNameA();
    }

    STDMETHOD_(void, SetName)(THIS_ LPCWSTR pszName) OVERRIDE {
        SValueAnimator::SetName(pszName);
    }

    STDMETHOD_(int, GetID)(THIS) SCONST OVERRIDE {
        return SValueAnimator::GetID();
    }

    STDMETHOD_(void, SetID)(THIS_ int nID) OVERRIDE {
        SValueAnimator::SetID(nID);
    }

    STDMETHOD_(HRESULT, AfterAttribute)(THIS_ LPCWSTR strAttribName, LPCWSTR strValue, BOOL bLoading, HRESULT hr) OVERRIDE {
        return SValueAnimator::AfterAttribute(strAttribName, strValue, bLoading, hr);
    }

    STDMETHOD_(HRESULT, SetAttributeA)(THIS_ LPCSTR pszAttr, LPCSTR pszValue, BOOL bLoading) OVERRIDE {
        return SValueAnimator::SetAttributeA(pszAttr, pszValue, bLoading);
    }

    STDMETHOD_(HRESULT, SetAttribute)(THIS_ LPCWSTR pszAttr, LPCWSTR pszValue, BOOL bLoading) OVERRIDE {
        return SValueAnimator::SetAttribute(pszAttr, pszValue, bLoading);
    }

    STDMETHOD_(HRESULT, ISetAttribute)(THIS_ const IStringW *strAttr, const IStringW *strValue, BOOL bLoading) OVERRIDE {
        return SValueAnimator::ISetAttribute(strAttr, strValue, bLoading);
    }
    STDMETHOD_(BOOL, GetAttribute)(THIS_ LPCWSTR strAttr, IStringW *pValue) SCONST OVERRIDE {
        return SValueAnimator::GetAttribute(strAttr, pValue);
    }

    STDMETHOD_(void, OnInitFinished)(THIS_ IXmlNode *xmlNode) OVERRIDE {
        SValueAnimator::OnInitFinished(xmlNode);
    }

    STDMETHOD_(void, SetAttrHandler)(THIS_ FunAttrHandler attrHandler) OVERRIDE {
        SValueAnimator::SetAttrHandler(attrHandler);
    }

public://proxy for IValueAnimator
    /**
     * @brief Sets the length of the animation.
     * @param duration The length of the animation, in milliseconds. This value cannot be negative.
     * @return SValueAnimator The object called with setDuration(). This return value makes it easier to compose statements together that construct
     * and then set the duration, as in SValueAnimator.ofInt(0, 10).setDuration(500).start().
     */
    STDMETHOD_(void, setDuration)(THIS_ long duration) OVERRIDE;

    /**
     * @brief Gets the length of the animation.
     * @return The length of the animation, in milliseconds.
     */
    STDMETHOD_(long, getDuration)(THIS) SCONST OVERRIDE;

    /**
     * @brief Gets the total duration of the animation, including any repetitions.
     * @return The total duration of the animation, in milliseconds.
     */
    STDMETHOD_(long, getTotalDuration)(THIS) SCONST OVERRIDE;

    /**
     * @brief Sets the position of the animation to the specified point in time.
     * @param playTime The time, in milliseconds, to which the animation is advanced or rewound.
     */
    STDMETHOD_(void, setCurrentPlayTime)(THIS_ long playTime) OVERRIDE;

    /**
     * @brief Sets the position of the animation to the specified fraction.
     * @param fraction The fraction to which the animation is advanced or rewound. Values outside the range of 0 to the maximum fraction for the animator
     * will be clamped to the correct range.
     */
    STDMETHOD_(void, setCurrentFraction)(THIS_ float fraction) OVERRIDE;

      public:
    /**
     * @brief Gets the current position of the animation in time.
     * @return The current position in time of the animation.
     */
    STDMETHOD_(long, getCurrentPlayTime)(THIS) OVERRIDE;

    /**
     * @brief Gets the amount of time, in milliseconds, to delay starting the animation after start() is called.
     * @return The number of milliseconds to delay running the animation.
     */
    STDMETHOD_(long, getStartDelay)(THIS) SCONST OVERRIDE;

    /**
     * @brief Sets the amount of time, in milliseconds, to delay starting the animation after start() is called.
     * @param startDelay The amount of the delay, in milliseconds.
     */
    STDMETHOD_(void, setStartDelay)(THIS_ long startDelay) OVERRIDE;

    /**
     * @brief Sets how many times the animation should be repeated.
     * @param value The number of times the animation should be repeated. If the repeat count is 0, the animation is never repeated.
     * If the repeat count is greater than 0 or INFINITE, the repeat mode will be taken into account. The repeat count is 0 by default.
     */
    STDMETHOD_(void, setRepeatCount)(THIS_ int value) OVERRIDE;

    /**
     * @brief Defines how many times the animation should repeat.
     * @return The number of times the animation should repeat, or INFINITE.
     */
    STDMETHOD_(int, getRepeatCount)(THIS) SCONST OVERRIDE;

    /**
     * @brief Defines what this animation should do when it reaches the end.
     * @param value Either RESTART or REVERSE. Defaults to RESTART.
     */
    STDMETHOD_(void, setRepeatMode)(THIS_ RepeatMode value) OVERRIDE;

    /**
     * @brief Defines what this animation should do when it reaches the end.
     * @return Either REVERSE or RESTART.
     */
    STDMETHOD_(RepeatMode, getRepeatMode)(THIS) SCONST OVERRIDE;

    /**
     * @brief Adds a listener to the set of listeners that are sent update events through the life of an animation.
     * @param listener The listener to be added to the current set of listeners for this animation.
     */
    STDMETHOD_(void, addUpdateListener)(THIS_ IAnimatorUpdateListener *listener) OVERRIDE;

    /**
     * @brief Removes all listeners from the set listening to frame updates for this animation.
     */
    STDMETHOD_(void, removeAllUpdateListeners)(THIS) OVERRIDE;

    /**
     * @brief Removes a listener from the set listening to frame updates for this animation.
     * @param listener The listener to be removed from the current set of update listeners for this animation.
     */
    STDMETHOD_(void, removeUpdateListener)(THIS_ IAnimatorUpdateListener *listener) OVERRIDE;

    /**
     * @brief Sets the time interpolator used in calculating the elapsed fraction of this animation.
     * @param value The interpolator to be used by this animation. A value of nullptr will result in linear interpolation.
     */
    STDMETHOD_(void, setInterpolator)(THIS_ IInterpolator *value) OVERRIDE;

    /**
     * @brief Returns the timing interpolator that this SValueAnimator uses.
     * @return The timing interpolator for this SValueAnimator.
     */
    STDMETHOD_(IInterpolator *, getInterpolator)(THIS) SCONST OVERRIDE;

    /**
     * @brief Adds an animator listener.
     * @param p The listener to be added.
     */
    STDMETHOD_(void, addListener)(THIS_ IAnimatorListener *p) OVERRIDE;

    /**
     * @brief Removes an animator listener.
     * @param p The listener to be removed.
     */
    STDMETHOD_(void, removeListener)(THIS_ IAnimatorListener *p) OVERRIDE;

    /**
     * @brief 获取用户数据
     * @return LPVOID - 用户数据指针
     */
    STDMETHOD_(LPVOID, GetUserData)(CTHIS) SCONST OVERRIDE;

    /**
     * @brief 设置用户数据
     * @param pUserData - 用户数据指针
     * @return void
     */
    STDMETHOD_(void, SetUserData)(THIS_ LPVOID pUserData) OVERRIDE;

     public:
    /**
     * @brief Starts the animation playing.
     * @param pContainer The container managing the timeline handlers.
     */
    STDMETHOD_(void, start)(THIS_ ITimelineHandlersMgr *pContainer) OVERRIDE;

    /**
     * @brief Ends the animation.
     */
    STDMETHOD_(void, end)(THIS) OVERRIDE;

    /**
     * @brief Checks if the animation is running.
     * @return TRUE if the animation is running, FALSE otherwise.
     */
    STDMETHOD_(BOOL, isRunning)(THIS) SCONST OVERRIDE;

    /**
     * @brief Checks if the animation has been started.
     * @return TRUE if the animation has been started, FALSE otherwise.
     */
    STDMETHOD_(BOOL, isStarted)(THIS) SCONST OVERRIDE;

    /**
     * @brief Plays the SValueAnimator in reverse.
     * @details If the animation is already running, it will stop itself and play backwards from the point reached when reverse was called.
     * If the animation is not currently running, then it will start from the end and play backwards. This behavior is only set for the current
     * animation; future playing of the animation will use the default behavior of playing forward.
     */
    STDMETHOD_(void, reverse)(THIS) OVERRIDE;

    /**
     * @brief Creates a clone of the current animation.
     * @return A clone of the current animation.
     */
    STDMETHOD_(IValueAnimator *, clone)(THIS) SCONST OVERRIDE;

    /**
     * @brief Copies the properties of another animation to this animation.
     * @param pSrc The source animation to copy properties from.
     */
    STDMETHOD_(void, copy)(THIS_ const IValueAnimator *pSrc) OVERRIDE;

public:
    /**
     * @brief Commits a frame of animation.
     * @param frameTime The current frame time.
     * @return TRUE if the animation is finished, FALSE otherwise.
     */
    STDMETHOD_(BOOL, commitAnimationFrame)(THIS_ uint64_t frameTime) OVERRIDE;  public:
    /**
     * @brief Returns the current animation fraction.
     * @details This is the elapsed/interpolated fraction used in the most recent frame update on the animation.
     * @return The elapsed/interpolated fraction of the animation.
     */
    STDMETHOD_(float, getAnimatedFraction)(THIS) SCONST OVERRIDE;

    /**
     * @brief Handles the next frame of the animation.
     */
    STDMETHOD_(void, OnNextFrame)(THIS_) OVERRIDE;

    STDMETHOD_(ITimelineHandler *, GetTimelineHandler)(CTHIS) SCONST OVERRIDE;
};

template <class T>
STDMETHODIMP_(void) TValueAnimatorProxy<T>::setDuration(long duration)
{
    SValueAnimator::setDuration(duration);
}

template <class T>
STDMETHODIMP_(long) TValueAnimatorProxy<T>::getDuration() SCONST
{
    return SValueAnimator::getDuration();
}

template <class T>
STDMETHODIMP_(long) TValueAnimatorProxy<T>::getTotalDuration() SCONST
{
    return SValueAnimator::getTotalDuration();
}

template <class T>
STDMETHODIMP_(void) TValueAnimatorProxy<T>::setCurrentPlayTime(long playTime)
{
    SValueAnimator::setCurrentPlayTime(playTime);
}

template <class T>
STDMETHODIMP_(void) TValueAnimatorProxy<T>::setCurrentFraction(float fraction)
{
    SValueAnimator::setCurrentFraction(fraction);
}

template <class T>
STDMETHODIMP_(long) TValueAnimatorProxy<T>::getCurrentPlayTime()
{
    return SValueAnimator::getCurrentPlayTime();
}

template <class T>
STDMETHODIMP_(long) TValueAnimatorProxy<T>::getStartDelay() SCONST
{
    return SValueAnimator::getStartDelay();
}

template <class T>
STDMETHODIMP_(void) TValueAnimatorProxy<T>::setStartDelay(long startDelay)
{
    SValueAnimator::setStartDelay(startDelay);
}

template <class T>
STDMETHODIMP_(void) TValueAnimatorProxy<T>::setRepeatCount(int value)
{
    SValueAnimator::setRepeatCount(value);
}

template <class T>
STDMETHODIMP_(int) TValueAnimatorProxy<T>::getRepeatCount() SCONST
{
    return SValueAnimator::getRepeatCount();
}

template <class T>
STDMETHODIMP_(void) TValueAnimatorProxy<T>::setRepeatMode(RepeatMode value)
{
    SValueAnimator::setRepeatMode(value);
}

template <class T>
STDMETHODIMP_(RepeatMode) TValueAnimatorProxy<T>::getRepeatMode() SCONST
{
    return SValueAnimator::getRepeatMode();
}

template <class T>
STDMETHODIMP_(void) TValueAnimatorProxy<T>::addUpdateListener(IAnimatorUpdateListener *listener)
{
    SValueAnimator::addUpdateListener(listener);
}

template <class T>
STDMETHODIMP_(void) TValueAnimatorProxy<T>::removeAllUpdateListeners()
{
    SValueAnimator::removeAllUpdateListeners();
}

template <class T>
STDMETHODIMP_(void) TValueAnimatorProxy<T>::removeUpdateListener(IAnimatorUpdateListener *listener)
{
    SValueAnimator::removeUpdateListener(listener);
}

template <class T>
STDMETHODIMP_(void) TValueAnimatorProxy<T>::setInterpolator(IInterpolator *value)
{
    SValueAnimator::setInterpolator(value);
}

template <class T>
STDMETHODIMP_(IInterpolator *) TValueAnimatorProxy<T>::getInterpolator() SCONST
{
    return SValueAnimator::getInterpolator();
}

template <class T>
STDMETHODIMP_(void) TValueAnimatorProxy<T>::addListener(IAnimatorListener *p)
{
    SValueAnimator::addListener(p);
}

template <class T>
STDMETHODIMP_(void) TValueAnimatorProxy<T>::removeListener(IAnimatorListener *p)
{
    SValueAnimator::removeListener(p);
}

template <class T>
STDMETHODIMP_(LPVOID) TValueAnimatorProxy<T>::GetUserData() SCONST
{
    return SValueAnimator::GetUserData();
}

template <class T>
STDMETHODIMP_(void) TValueAnimatorProxy<T>::SetUserData(LPVOID pUserData)
{
    SValueAnimator::SetUserData(pUserData);
}

template <class T>
STDMETHODIMP_(void) TValueAnimatorProxy<T>::start(ITimelineHandlersMgr *pContainer)
{
    SValueAnimator::start(pContainer);
}

template <class T>
STDMETHODIMP_(void) TValueAnimatorProxy<T>::end()
{
    SValueAnimator::end();
}

template <class T>
STDMETHODIMP_(BOOL) TValueAnimatorProxy<T>::isRunning() SCONST
{
    return SValueAnimator::isRunning();
}

template <class T>
STDMETHODIMP_(BOOL) TValueAnimatorProxy<T>::isStarted() SCONST
{
    return SValueAnimator::isStarted();
}

template <class T>
STDMETHODIMP_(void) TValueAnimatorProxy<T>::reverse()
{
    SValueAnimator::reverse();
}

template <class T>
STDMETHODIMP_(IValueAnimator *) TValueAnimatorProxy<T>::clone() SCONST
{
    return SValueAnimator::clone();
}

template <class T>
STDMETHODIMP_(void) TValueAnimatorProxy<T>::copy(const IValueAnimator *pSrc)
{
    SValueAnimator::copy(pSrc);
}

template <class T>
STDMETHODIMP_(BOOL) TValueAnimatorProxy<T>::commitAnimationFrame(uint64_t frameTime)
{
    return SValueAnimator::commitAnimationFrame(frameTime);
}

template <class T>
STDMETHODIMP_(float) TValueAnimatorProxy<T>::getAnimatedFraction() SCONST
{
    return SValueAnimator::getAnimatedFraction();
}

template <class T>
STDMETHODIMP_(void) TValueAnimatorProxy<T>::OnNextFrame()
{
    SValueAnimator::OnNextFrame();
}

template <class T>
STDMETHODIMP_(ITimelineHandler *) TValueAnimatorProxy<T>::GetTimelineHandler() const
{
    return SValueAnimator::GetTimelineHandler();
}

SNSEND
#endif//__SVALUEANIMATORPROXY__H__