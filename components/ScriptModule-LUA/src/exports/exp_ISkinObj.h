#ifndef __EXP_ISKINOBJ_H__
#define __EXP_ISKINOBJ_H__

#include <interface/SSkinobj-i.h>
#include "toobj.h"

static void ExpLua_ISkinObj_Inner(lua_State *L)
{
    lua_tinker::class_add<ISkinObj>(L, "ISkinObj");
    lua_tinker::class_inh<ISkinObj, IObject>(L);

    // DrawByState/DrawByIndex need an IRenderTarget* which is not exported to
    // lua, they are reachable through SWindow painting instead.
    lua_tinker::class_def<ISkinObj>(L, "GetSkinSize", &ISkinObj::GetSkinSize);
    lua_tinker::class_def<ISkinObj>(L, "GetStates", &ISkinObj::GetStates);
    lua_tinker::class_def<ISkinObj>(L, "GetAlpha", &ISkinObj::GetAlpha);
    lua_tinker::class_def<ISkinObj>(L, "SetAlpha", &ISkinObj::SetAlpha);
    lua_tinker::class_def<ISkinObj>(L, "GetScale", &ISkinObj::GetScale);
    lua_tinker::class_def<ISkinObj>(L, "SetScale", &ISkinObj::SetScale);
    lua_tinker::class_def<ISkinObj>(L, "Scale", &ISkinObj::Scale);
    lua_tinker::class_def<ISkinObj>(L, "OnColorize", &ISkinObj::OnColorize);

    DEF_TOOBJ(L, ISkinObj);
}

#endif // __EXP_ISKINOBJ_H__
