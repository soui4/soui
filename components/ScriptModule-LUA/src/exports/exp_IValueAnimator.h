#include <interface/SAnimation-i.h>
#include "toobj.h"

BOOL ExpLua_IValueAnimator(lua_State *L)
{
	try{
		lua_tinker::class_add<IValueAnimator>(L,"IValueAnimator");
		lua_tinker::class_inh<IValueAnimator,IObject>(L);
		lua_tinker::class_def<IValueAnimator>(L,"setDuration",&IValueAnimator::setDuration);
		lua_tinker::class_def<IValueAnimator>(L,"getDuration",&IValueAnimator::getDuration);
		lua_tinker::class_def<IValueAnimator>(L,"getTotalDuration",&IValueAnimator::getTotalDuration);
		lua_tinker::class_def<IValueAnimator>(L,"setCurrentPlayTime",&IValueAnimator::setCurrentPlayTime);
		lua_tinker::class_def<IValueAnimator>(L,"setCurrentFraction",&IValueAnimator::setCurrentFraction);
		lua_tinker::class_def<IValueAnimator>(L,"getCurrentPlayTime",&IValueAnimator::getCurrentPlayTime);
		lua_tinker::class_def<IValueAnimator>(L,"getStartDelay",&IValueAnimator::getStartDelay);
		lua_tinker::class_def<IValueAnimator>(L,"setStartDelay",&IValueAnimator::setStartDelay);
		lua_tinker::class_def<IValueAnimator>(L,"setRepeatCount",&IValueAnimator::setRepeatCount);
		lua_tinker::class_def<IValueAnimator>(L,"getRepeatCount",&IValueAnimator::getRepeatCount);
		lua_tinker::class_def<IValueAnimator>(L,"setRepeatMode",&IValueAnimator::setRepeatMode);
		lua_tinker::class_def<IValueAnimator>(L,"getRepeatMode",&IValueAnimator::getRepeatMode);
		lua_tinker::class_def<IValueAnimator>(L,"addUpdateListener",&IValueAnimator::addUpdateListener);
		lua_tinker::class_def<IValueAnimator>(L,"removeAllUpdateListeners",&IValueAnimator::removeAllUpdateListeners);
		lua_tinker::class_def<IValueAnimator>(L,"removeUpdateListener",&IValueAnimator::removeUpdateListener);
		lua_tinker::class_def<IValueAnimator>(L,"setInterpolator",&IValueAnimator::setInterpolator);
		lua_tinker::class_def<IValueAnimator>(L,"getInterpolator",&IValueAnimator::getInterpolator);
		lua_tinker::class_def<IValueAnimator>(L,"addListener",&IValueAnimator::addListener);
		lua_tinker::class_def<IValueAnimator>(L,"removeListener",&IValueAnimator::removeListener);
		lua_tinker::class_def<IValueAnimator>(L,"start",&IValueAnimator::start);
		lua_tinker::class_def<IValueAnimator>(L,"end",&IValueAnimator::end);
		lua_tinker::class_def<IValueAnimator>(L,"isRunning",&IValueAnimator::isRunning);
		lua_tinker::class_def<IValueAnimator>(L,"isStarted",&IValueAnimator::isStarted);
		lua_tinker::class_def<IValueAnimator>(L,"reverse",&IValueAnimator::reverse);
		lua_tinker::class_def<IValueAnimator>(L,"commitAnimationFrame",&IValueAnimator::commitAnimationFrame);
		lua_tinker::class_def<IValueAnimator>(L,"getAnimatedFraction",&IValueAnimator::getAnimatedFraction);
		lua_tinker::class_def<IValueAnimator>(L,"clone",&IValueAnimator::clone);
		lua_tinker::class_def<IValueAnimator>(L,"onEvaluateValue",&IValueAnimator::onEvaluateValue);

		return TRUE;
	}catch(...)
	{
		return FALSE;
	}
}