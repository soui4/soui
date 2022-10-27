#include <interface/SAnimation-i.h>
#include "toobj.h"

BOOL ExpLua_IAnimation(lua_State *L)
{
	try{
		lua_tinker::class_add<IAnimation>(L,"IAnimation");
		lua_tinker::class_inh<IAnimation,IObject>(L);
		lua_tinker::class_def<IAnimation>(L,"clone",&IAnimation::clone);
		lua_tinker::class_def<IAnimation>(L,"copy",&IAnimation::copy);
		lua_tinker::class_def<IAnimation>(L,"reset",&IAnimation::reset);
		lua_tinker::class_def<IAnimation>(L,"cancel",&IAnimation::cancel);
		lua_tinker::class_def<IAnimation>(L,"setInterpolator",&IAnimation::setInterpolator);
		lua_tinker::class_def<IAnimation>(L,"setDuration",&IAnimation::setDuration);
		lua_tinker::class_def<IAnimation>(L,"scaleCurrentDuration",&IAnimation::scaleCurrentDuration);
		lua_tinker::class_def<IAnimation>(L,"setFillBefore",&IAnimation::setFillBefore);
		lua_tinker::class_def<IAnimation>(L,"setFillAfter",&IAnimation::setFillAfter);
		lua_tinker::class_def<IAnimation>(L,"getFillBefore",&IAnimation::getFillBefore);
		lua_tinker::class_def<IAnimation>(L,"getFillAfter",&IAnimation::getFillAfter);
		lua_tinker::class_def<IAnimation>(L,"setStartOffset",&IAnimation::setStartOffset);
		lua_tinker::class_def<IAnimation>(L,"isFillEnabled",&IAnimation::isFillEnabled);
		lua_tinker::class_def<IAnimation>(L,"setFillEnabled",&IAnimation::setFillEnabled);
		lua_tinker::class_def<IAnimation>(L,"setStartTime",&IAnimation::setStartTime);
		lua_tinker::class_def<IAnimation>(L,"start",&IAnimation::start);
		lua_tinker::class_def<IAnimation>(L,"startNow",&IAnimation::startNow);
		lua_tinker::class_def<IAnimation>(L,"setRepeatMode",&IAnimation::setRepeatMode);
		lua_tinker::class_def<IAnimation>(L,"setRepeatCount",&IAnimation::setRepeatCount);
		lua_tinker::class_def<IAnimation>(L,"setZAdjustment",&IAnimation::setZAdjustment);
		lua_tinker::class_def<IAnimation>(L,"getInterpolator",&IAnimation::getInterpolator);
		lua_tinker::class_def<IAnimation>(L,"getStartTime",&IAnimation::getStartTime);
		lua_tinker::class_def<IAnimation>(L,"getDuration",&IAnimation::getDuration);
		lua_tinker::class_def<IAnimation>(L,"getStartOffset",&IAnimation::getStartOffset);
		lua_tinker::class_def<IAnimation>(L,"getRepeatMode",&IAnimation::getRepeatMode);
		lua_tinker::class_def<IAnimation>(L,"getRepeatCount",&IAnimation::getRepeatCount);
		lua_tinker::class_def<IAnimation>(L,"getZAdjustment",&IAnimation::getZAdjustment);
		lua_tinker::class_def<IAnimation>(L,"setAnimationListener",&IAnimation::setAnimationListener);
		lua_tinker::class_def<IAnimation>(L,"computeDurationHint",&IAnimation::computeDurationHint);
		lua_tinker::class_def<IAnimation>(L,"getTransformation2",&IAnimation::getTransformation2);
		lua_tinker::class_def<IAnimation>(L,"getTransformation",&IAnimation::getTransformation);
		lua_tinker::class_def<IAnimation>(L,"hasStarted",&IAnimation::hasStarted);
		lua_tinker::class_def<IAnimation>(L,"hasEnded",&IAnimation::hasEnded);
		lua_tinker::class_def<IAnimation>(L,"applyTransformation",&IAnimation::applyTransformation);
		lua_tinker::class_def<IAnimation>(L,"hasAlpha",&IAnimation::hasAlpha);
		lua_tinker::class_def<IAnimation>(L,"initialize",&IAnimation::initialize);
		lua_tinker::class_def<IAnimation>(L,"setUserData",&IAnimation::setUserData);
		lua_tinker::class_def<IAnimation>(L,"getUserData",&IAnimation::getUserData);

		DEF_TOOBJ(L,IAnimation);

		return TRUE;
	}catch(...)
	{
		return FALSE;
	}
}