#pragma once

// IBitmapS export (minimal, mirrors soui4js exp_IImage.h: Width/Height/Clone).
// Drawing-related methods need IRenderTarget/IBrushS which are not exported.

#include <interface/SRender-i.h>
#include "toobj.h"

// IBitmapS:Clone() -> IBitmapS or nil
static int IBitmapS_Clone(lua_State *L)
{
    IBitmapS *_this = lua_tinker_toobj<IBitmapS>(L, 1);
    if (!_this)
        return 0;
    IBitmapS *pClone = NULL;
    if (SUCCEEDED(_this->Clone(&pClone)) && pClone)
    {
        lua_tinker::push<IBitmapS *>(L, pClone);
        return 1;
    }
    return 0;
}

BOOL ExpLua_IBitmapS(lua_State *L)
{
    try{
        lua_tinker::class_add<IBitmapS>(L, "IBitmapS");
        lua_tinker::class_inh<IBitmapS, IObjRef>(L);
        lua_tinker::class_def<IBitmapS>(L, "AddRef", &IBitmapS::AddRef);
        lua_tinker::class_def<IBitmapS>(L, "Release", &IBitmapS::Release);
        lua_tinker::class_def<IBitmapS>(L, "Width", &IBitmapS::Width);
        lua_tinker::class_def<IBitmapS>(L, "Height", &IBitmapS::Height);
        lua_tinker::class_def<IBitmapS>(L, "Size", &IBitmapS::Size);
        lua_tinker::class_def<IBitmapS>(L, "LoadFromFile", &IBitmapS::LoadFromFile);
        class_set_cfun<IBitmapS>(L, "Clone", IBitmapS_Clone);

        return TRUE;
    }catch(...)
    {
        return FALSE;
    }
}
