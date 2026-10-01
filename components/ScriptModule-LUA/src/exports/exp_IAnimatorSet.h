#ifndef __EXP_IANIMATORSET_H__
#define __EXP_IANIMATORSET_H__

#include <interface/SAnimatorSet-i.h>
#include <interface/SValueAnimator-i.h>
#include <valueAnimator/SAnimatorSet.h>
#include <valueAnimator/SValueAnimator.h>
#include "toobj.h"

// ---------------------------------------------------------------------------
// IAnimatorSet (inherits IValueAnimator)
// ---------------------------------------------------------------------------
static void ExpLua_IAnimatorSet_Inner(lua_State *L)
{
    lua_tinker::class_add<IAnimatorSet>(L, "IAnimatorSet");
    lua_tinker::class_inh<IAnimatorSet, IValueAnimator>(L);

    lua_tinker::class_def<IAnimatorSet>(L, "AddAnimator", &IAnimatorSet::AddAnimator);
    lua_tinker::class_def<IAnimatorSet>(L, "AddAnimatorAfter", &IAnimatorSet::AddAnimatorAfter);
    lua_tinker::class_def<IAnimatorSet>(L, "AddAnimatorWith", &IAnimatorSet::AddAnimatorWith);
    lua_tinker::class_def<IAnimatorSet>(L, "RemoveAnimator", &IAnimatorSet::RemoveAnimator);
    lua_tinker::class_def<IAnimatorSet>(L, "RemoveAllAnimators", &IAnimatorSet::RemoveAllAnimators);
    lua_tinker::class_def<IAnimatorSet>(L, "GetAnimatorCount", &IAnimatorSet::GetAnimatorCount);
    lua_tinker::class_def<IAnimatorSet>(L, "GetAnimatorAt", &IAnimatorSet::GetAnimatorAt);
    lua_tinker::class_def<IAnimatorSet>(L, "SetPlayMode", &IAnimatorSet::SetPlayMode);
    lua_tinker::class_def<IAnimatorSet>(L, "GetPlayMode", &IAnimatorSet::GetPlayMode);

    DEF_CAST_OBJREF(L, IAnimatorSet);
}

// ---------------------------------------------------------------------------
// IAnimatorGroup (inherits IObjRef)
// ---------------------------------------------------------------------------
static void ExpLua_IAnimatorGroup_Inner(lua_State *L)
{
    lua_tinker::class_add<IAnimatorGroup>(L, "IAnimatorGroup");
    lua_tinker::class_inh<IAnimatorGroup, IObjRef>(L);

    lua_tinker::class_def<IAnimatorGroup>(L, "AddAnimator", &IAnimatorGroup::AddAnimator);
    lua_tinker::class_def<IAnimatorGroup>(L, "RemoveAnimator", &IAnimatorGroup::RemoveAnimator);

    DEF_CAST_OBJREF(L, IAnimatorGroup);
}

// factories
static IAnimatorSet *CreateAnimatorSet()
{
    return new SAnimatorSet();
}

static IAnimatorGroup *CreateAnimatorGroup(int nID = 0)
{
    return new SAnimatorGroup(nID);
}

// load a value animator from resource (defended in exp_global registration)
static IValueAnimator *Lua_LoadValueAnimator(const char *resId)
{
    SStringT strId = S_CA2T(resId, CP_UTF8);
    return SApplication::getSingleton().LoadValueAnimator(strId);
}

#endif // __EXP_IANIMATORSET_H__
