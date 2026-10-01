#ifndef __EXP_IPROPERTYANIMATOR_H__
#define __EXP_IPROPERTYANIMATOR_H__

#include <interface/SPropertyAnimator-i.h>
#include <valueAnimator/SPropertyAnimator.h>
#include "toobj.h"

// ---------------------------------------------------------------------------
// IPropertyValuesHolder
// ---------------------------------------------------------------------------
static IPropertyValuesHolder *pvh_check(lua_State *L, int idx)
{
    IPropertyValuesHolder *p = lua_tinker_toobj<IPropertyValuesHolder>(L, idx);
    if (!p)
        luaL_error(L, "expect IPropertyValuesHolder at arg %d", idx);
    return p;
}

static int L_PVH_SetPropertyName(lua_State *L)
{
    IPropertyValuesHolder *_this = pvh_check(L, 1);
    const char *name = luaL_checkstring(L, 2);
    SStringW strName = S_CA2W(name, CP_UTF8);
    _this->SetPropertyName(strName.c_str());
    return 0;
}

static int L_PVH_GetPropertyName(lua_State *L)
{
    IPropertyValuesHolder *_this = pvh_check(L, 1);
    LPCWSTR name = _this->GetPropertyName();
    SStringA strName = name ? S_CW2A(name, CP_UTF8) : "";
    lua_pushstring(L, strName.c_str());
    return 1;
}

static int L_PVH_SetIntValues(lua_State *L)
{
    IPropertyValuesHolder *_this = pvh_check(L, 1);
    std::vector<int> vals = lua_table_to_ints(L, 2);
    _this->SetIntValues(vals.empty() ? NULL : &vals[0], (int)vals.size());
    return 0;
}

static int L_PVH_SetFloatValues(lua_State *L)
{
    IPropertyValuesHolder *_this = pvh_check(L, 1);
    std::vector<float> vals = lua_table_to_floats(L, 2);
    _this->SetFloatValues(vals.empty() ? NULL : &vals[0], (int)vals.size());
    return 0;
}

static int L_PVH_SetByteValues(lua_State *L)
{
    IPropertyValuesHolder *_this = pvh_check(L, 1);
    std::vector<int> vals = lua_table_to_ints(L, 2);
    std::vector<BYTE> bytes;
    bytes.reserve(vals.size());
    for (size_t i = 0; i < vals.size(); i++)
        bytes.push_back((BYTE)vals[i]);
    _this->SetByteValues(bytes.empty() ? NULL : &bytes[0], (int)bytes.size());
    return 0;
}

static int L_PVH_SetShortValues(lua_State *L)
{
    IPropertyValuesHolder *_this = pvh_check(L, 1);
    std::vector<int> vals = lua_table_to_ints(L, 2);
    std::vector<short> shorts;
    shorts.reserve(vals.size());
    for (size_t i = 0; i < vals.size(); i++)
        shorts.push_back((short)vals[i]);
    _this->SetShortValues(shorts.empty() ? NULL : &shorts[0], (int)shorts.size());
    return 0;
}

static int L_PVH_SetColorRefValues(lua_State *L)
{
    IPropertyValuesHolder *_this = pvh_check(L, 1);
    std::vector<int> vals = lua_table_to_ints(L, 2);
    std::vector<COLORREF> colors;
    colors.reserve(vals.size());
    for (size_t i = 0; i < vals.size(); i++)
        colors.push_back((COLORREF)vals[i]);
    _this->SetColorRefValues(colors.empty() ? NULL : &colors[0], (int)colors.size());
    return 0;
}

static int L_PVH_SetKeyFrameWeights(lua_State *L)
{
    IPropertyValuesHolder *_this = pvh_check(L, 1);
    std::vector<float> vals = lua_table_to_floats(L, 2);
    BOOL bRet = _this->SetKeyFrameWeights(vals.empty() ? NULL : &vals[0], (int)vals.size());
    lua_pushboolean(L, bRet);
    return 1;
}

static int L_PVH_GetKeyFrameWeights(lua_State *L)
{
    IPropertyValuesHolder *_this = pvh_check(L, 1);
    int count = _this->GetKeyframeCount();
    std::vector<float> weights(count);
    BOOL bRet = FALSE;
    if (count > 0)
        bRet = _this->GetKeyFrameWeights(&weights[0], count);
    if (!bRet)
    {
        lua_pushnil(L);
        return 1;
    }
    lua_push_float_table(L, weights);
    return 1;
}

static int L_PVH_GetAnimatedValue(lua_State *L)
{
    IPropertyValuesHolder *_this = pvh_check(L, 1);
    float fraction = (float)luaL_checknumber(L, 2);
    PROP_TYPE type = _this->GetValueType();
    if (type == PROP_TYPE_FLOAT)
    {
        float v = 0.f;
        if (_this->GetAnimatedValue(fraction, &v))
        {
            lua_pushnumber(L, v);
            return 1;
        }
    }
    else if (type == PROP_TYPE_INT || type == PROP_TYPE_COLORREF || type == PROP_TYPE_BYTE
             || type == PROP_TYPE_SHORT)
    {
        int v = 0;
        if (_this->GetAnimatedValue(fraction, &v))
        {
            lua_pushinteger(L, v);
            return 1;
        }
    }
    else
    {
        // PROP_TYPE_LAYOUT_SIZE / VARIANT: read raw 4 bytes as int as a fallback
        int v = 0;
        if (_this->GetAnimatedValue(fraction, &v))
        {
            lua_pushinteger(L, v);
            return 1;
        }
    }
    lua_pushnil(L);
    return 1;
}

static void ExpLua_IPropertyValuesHolder_Inner(lua_State *L)
{
    lua_tinker::class_add<IPropertyValuesHolder>(L, "IPropertyValuesHolder");
    lua_tinker::class_inh<IPropertyValuesHolder, IObjRef>(L);

    lua_tinker::class_def<IPropertyValuesHolder>(L, "GetValueType", &IPropertyValuesHolder::GetValueType);
    lua_tinker::class_def<IPropertyValuesHolder>(L, "GetValueSize", &IPropertyValuesHolder::GetValueSize);
    lua_tinker::class_def<IPropertyValuesHolder>(L, "GetKeyframeCount", &IPropertyValuesHolder::GetKeyframeCount);

    class_set_cfun<IPropertyValuesHolder>(L, "SetPropertyName", L_PVH_SetPropertyName);
    class_set_cfun<IPropertyValuesHolder>(L, "GetPropertyName", L_PVH_GetPropertyName);
    class_set_cfun<IPropertyValuesHolder>(L, "SetIntValues", L_PVH_SetIntValues);
    class_set_cfun<IPropertyValuesHolder>(L, "SetFloatValues", L_PVH_SetFloatValues);
    class_set_cfun<IPropertyValuesHolder>(L, "SetByteValues", L_PVH_SetByteValues);
    class_set_cfun<IPropertyValuesHolder>(L, "SetShortValues", L_PVH_SetShortValues);
    class_set_cfun<IPropertyValuesHolder>(L, "SetColorRefValues", L_PVH_SetColorRefValues);
    class_set_cfun<IPropertyValuesHolder>(L, "SetKeyFrameWeights", L_PVH_SetKeyFrameWeights);
    class_set_cfun<IPropertyValuesHolder>(L, "GetKeyFrameWeights", L_PVH_GetKeyFrameWeights);
    class_set_cfun<IPropertyValuesHolder>(L, "GetAnimatedValue", L_PVH_GetAnimatedValue);

    DEF_CAST_OBJREF(L, IPropertyValuesHolder);
}

// ---------------------------------------------------------------------------
// IPropertyAnimator
// ---------------------------------------------------------------------------
static IPropertyAnimator *pa_check(lua_State *L, int idx)
{
    IPropertyAnimator *p = lua_tinker_toobj<IPropertyAnimator>(L, idx);
    if (!p)
        luaL_error(L, "expect IPropertyAnimator at arg %d", idx);
    return p;
}

static int L_PA_SetPropertyValuesHolders(lua_State *L)
{
    IPropertyAnimator *_this = pa_check(L, 1);
    if (!lua_istable(L, 2))
        return 0;
    int n = (int)lua_rawlen(L, 2);
    std::vector<IPropertyValuesHolder *> holders;
    holders.reserve(n);
    for (int i = 1; i <= n; i++)
    {
        lua_rawgeti(L, 2, i);
        IPropertyValuesHolder *h = lua_tinker_toobj<IPropertyValuesHolder>(L, -1);
        if (h)
            holders.push_back(h);
        lua_pop(L, 1);
    }
    _this->SetPropertyValuesHolders(holders.empty() ? NULL : &holders[0], (int)holders.size());
    return 0;
}

static int L_PA_GetPropertyValuesHolderByName(lua_State *L)
{
    IPropertyAnimator *_this = pa_check(L, 1);
    const char *name = luaL_checkstring(L, 2);
    SStringW strName = S_CA2W(name, CP_UTF8);
    IPropertyValuesHolder *h = _this->GetPropertyValuesHolderByName(strName.c_str());
    if (!h)
    {
        lua_pushnil(L);
        return 1;
    }
    lua_tinker::push<IPropertyValuesHolder *>(L, h);
    return 1;
}

static int L_PA_GetPropertyValuesHolderByIndex(lua_State *L)
{
    IPropertyAnimator *_this = pa_check(L, 1);
    int index = (int)luaL_checkinteger(L, 2);
    IPropertyValuesHolder *h = _this->GetPropertyValuesHolderByIndex(index);
    if (!h)
    {
        lua_pushnil(L);
        return 1;
    }
    lua_tinker::push<IPropertyValuesHolder *>(L, h);
    return 1;
}

static void ExpLua_IPropertyAnimator_Inner(lua_State *L)
{
    lua_tinker::class_add<IPropertyAnimator>(L, "IPropertyAnimator");
    lua_tinker::class_inh<IPropertyAnimator, IValueAnimator>(L);

    lua_tinker::class_def<IPropertyAnimator>(L, "SetTarget", &IPropertyAnimator::SetTarget);
    lua_tinker::class_def<IPropertyAnimator>(L, "GetTarget", &IPropertyAnimator::GetTarget);
    lua_tinker::class_def<IPropertyAnimator>(L, "SetPropertyValuesHolder", &IPropertyAnimator::SetPropertyValuesHolder);
    lua_tinker::class_def<IPropertyAnimator>(L, "GetPropertyValuesHolderCount", &IPropertyAnimator::GetPropertyValuesHolderCount);

    class_set_cfun<IPropertyAnimator>(L, "SetPropertyValuesHolders", L_PA_SetPropertyValuesHolders);
    class_set_cfun<IPropertyAnimator>(L, "GetPropertyValuesHolderByName", L_PA_GetPropertyValuesHolderByName);
    class_set_cfun<IPropertyAnimator>(L, "GetPropertyValuesHolderByIndex", L_PA_GetPropertyValuesHolderByIndex);

    DEF_CAST_OBJREF(L, IPropertyAnimator);
}

// factories
static IPropertyAnimator *CreatePropertyAnimator()
{
    return new SPropertyAnimator(NULL);
}

static IPropertyValuesHolder *CreatePropertyValuesHolder()
{
    return new SPropertyValuesHolder();
}

#endif // __EXP_IPROPERTYANIMATOR_H__
