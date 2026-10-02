#pragma once

// SMatrix / IMatrix exports
// IWindow::SetMatrix/GetMatrix take IMatrix*, which was never registered before,
// so those methods could not be called from Lua. Registering IMatrix/SMatrix
// fixes that and mirrors soui4js exp_SMatrix.h.

#include <interface/SMatrix-i.h>
#include <matrix/SMatrix.h>
#include "toobj.h"

// IMatrix:Data() -> table of 9 floats
static int IMatrix_Data(lua_State *L)
{
    IMatrix *_this = lua_tinker_toobj<IMatrix>(L, 1);
    if (!_this)
        return 0;
    IxForm *mtx = _this->Data();
    if (!mtx)
        return 0;
    lua_push_float_table(L, std::vector<float>(mtx->fMat, mtx->fMat + 9));
    return 1;
}

// SMatrix:setMatrix(tbl) -- tbl is a table of 9 floats; returns self
static int SMatrix_setMatrix(lua_State *L)
{
    SMatrix *_this = lua_tinker_toobj<SMatrix>(L, 1);
    if (!_this)
        return 0;
    std::vector<float> vals = lua_table_to_floats(L, 2);
    if (vals.size() < 9)
        return 0;
    float data[9];
    for (int i = 0; i < 9; i++)
        data[i] = vals[i];
    _this->setMatrix(data);
    lua_tinker::push<SMatrix *>(L, _this);
    return 1;
}

// chained ops, each returns self
static int SMatrix_rotate(lua_State *L)
{
    SMatrix *_this = lua_tinker_toobj<SMatrix>(L, 1);
    if (!_this)
        return 0;
    _this->rotate((float)lua_tonumber(L, 2));
    lua_tinker::push<SMatrix *>(L, _this);
    return 1;
}

static int SMatrix_translate(lua_State *L)
{
    SMatrix *_this = lua_tinker_toobj<SMatrix>(L, 1);
    if (!_this)
        return 0;
    _this->translate((float)lua_tonumber(L, 2), (float)lua_tonumber(L, 3));
    lua_tinker::push<SMatrix *>(L, _this);
    return 1;
}

static int SMatrix_scale(lua_State *L)
{
    SMatrix *_this = lua_tinker_toobj<SMatrix>(L, 1);
    if (!_this)
        return 0;
    _this->scale((float)lua_tonumber(L, 2), (float)lua_tonumber(L, 3));
    lua_tinker::push<SMatrix *>(L, _this);
    return 1;
}

static int SMatrix_shear(lua_State *L)
{
    SMatrix *_this = lua_tinker_toobj<SMatrix>(L, 1);
    if (!_this)
        return 0;
    _this->shear((float)lua_tonumber(L, 2), (float)lua_tonumber(L, 3));
    lua_tinker::push<SMatrix *>(L, _this);
    return 1;
}

// SMatrix:invert([dest]) -> SMatrix (dest userdata or a new value)
static int SMatrix_invert(lua_State *L)
{
    SMatrix *_this = lua_tinker_toobj<SMatrix>(L, 1);
    if (!_this)
        return 0;
    if (lua_isuserdata(L, 2))
    {
        SMatrix *dst = lua_tinker_toobj<SMatrix>(L, 2);
        if (dst)
        {
            _this->invert(dst);
            lua_tinker::push<SMatrix *>(L, dst);
            return 1;
        }
    }
    SMatrix ret;
    _this->invert(&ret);
    lua_tinker::push<SMatrix>(L, ret);
    return 1;
}

BOOL ExpLua_SMatrix(lua_State *L)
{
    try{
        lua_tinker::class_add<IMatrix>(L, "IMatrix");
        lua_tinker::class_def<IMatrix>(L, "reset", &IMatrix::reset);
        lua_tinker::class_def<IMatrix>(L, "setIdentity", &IMatrix::setIdentity);
        lua_tinker::class_def<IMatrix>(L, "isIdentity", &IMatrix::isIdentity);
        lua_tinker::class_def<IMatrix>(L, "setTranslate", &IMatrix::setTranslate);
        lua_tinker::class_def<IMatrix>(L, "setScale", &IMatrix::setScale);
        lua_tinker::class_def<IMatrix>(L, "setScale2", &IMatrix::setScale2);
        lua_tinker::class_def<IMatrix>(L, "setRotate", &IMatrix::setRotate);
        lua_tinker::class_def<IMatrix>(L, "setRotate2", &IMatrix::setRotate2);
        lua_tinker::class_def<IMatrix>(L, "setSkew", &IMatrix::setSkew);
        lua_tinker::class_def<IMatrix>(L, "setSkew2", &IMatrix::setSkew2);
        class_set_cfun<IMatrix>(L, "Data", IMatrix_Data);

        lua_tinker::class_add<SMatrix>(L, "SMatrix");
        lua_tinker::class_inh<SMatrix, IMatrix>(L);
        lua_tinker::class_con<SMatrix>(L, lua_tinker::constructor<SMatrix>);
        lua_tinker::class_def<SMatrix>(L, "getAt", (float (SMatrix::*)(int) const)&SMatrix::get);
        lua_tinker::class_def<SMatrix>(L, "setAt", (void (SMatrix::*)(int, float))&SMatrix::set);
        class_set_cfun<SMatrix>(L, "setMatrix", SMatrix_setMatrix);
        class_set_cfun<SMatrix>(L, "rotate", SMatrix_rotate);
        class_set_cfun<SMatrix>(L, "translate", SMatrix_translate);
        class_set_cfun<SMatrix>(L, "scale", SMatrix_scale);
        class_set_cfun<SMatrix>(L, "shear", SMatrix_shear);
        class_set_cfun<SMatrix>(L, "invert", SMatrix_invert);

        return TRUE;
    }catch(...)
    {
        return FALSE;
    }
}
