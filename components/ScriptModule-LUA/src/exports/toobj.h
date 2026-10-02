#pragma once

#ifndef DEF_TOOBJ
#define DEF_TOOBJ(luaStat,x) 		lua_tinker::def(luaStat,"to"#x,&sobj_cast<x>)
#endif

template<typename T> 
T* cast_pvoid(void* p){
	return (T*)p;
}

#ifndef DEF_CAST_PVOID
#define DEF_CAST_PVOID(luaState,x) lua_tinker::def(luaState,"to"#x,&cast_pvoid<x>)
#endif

template<typename T> 
T* cast_pobjref(IObjRef* p){
	return (T*)p;
}

#ifndef DEF_CAST_OBJREF
#define DEF_CAST_OBJREF(luaState,x) lua_tinker::def(luaState,"to"#x,&cast_pobjref<x>)
#endif

// ---------------------------------------------------------------------------
// helpers for binding functions that need raw lua_State access (tables, arrays)
// ---------------------------------------------------------------------------
#include <string>
#include <vector>

// convert a lua_tinker userdata (class object) on stack to a C++ pointer
template<typename T>
T* lua_tinker_toobj(lua_State *L, int idx)
{
    if (!lua_isuserdata(L, idx))
        return NULL;
    lua_tinker::user *u = lua_tinker::user2type<lua_tinker::user*>::invoke(L, idx);
    if (!u)
        return NULL;
    return (T*)u->m_p;
}

// set a raw lua_CFunction into a registered class table, so it can be called
// as obj:method(...) with the object at stack index 1.
// must be called after lua_tinker::class_add<T>(L, name).
template<typename T>
void class_set_cfun(lua_State *L, const char *name, lua_CFunction f)
{
    lua_tinker::push_meta(L, lua_tinker::class_name<T>::name());
    if (lua_istable(L, -1))
    {
        lua_pushstring(L, name);
        lua_pushcclosure(L, f, 0);
        lua_rawset(L, -3);
    }
    lua_pop(L, 1);
}

// read a lua table at idx into std::vector<int>
inline std::vector<int> lua_table_to_ints(lua_State *L, int idx)
{
    std::vector<int> vals;
    if (!lua_istable(L, idx))
        return vals;
    int n = (int)lua_rawlen(L, idx);
    vals.reserve(n);
    for (int i = 1; i <= n; i++)
    {
        lua_rawgeti(L, idx, i);
        vals.push_back((int)lua_tointeger(L, -1));
        lua_pop(L, 1);
    }
    return vals;
}

// read a lua table at idx into std::vector<float>
inline std::vector<float> lua_table_to_floats(lua_State *L, int idx)
{
    std::vector<float> vals;
    if (!lua_istable(L, idx))
        return vals;
    int n = (int)lua_rawlen(L, idx);
    vals.reserve(n);
    for (int i = 1; i <= n; i++)
    {
        lua_rawgeti(L, idx, i);
        vals.push_back((float)lua_tonumber(L, -1));
        lua_pop(L, 1);
    }
    return vals;
}

// push a std::vector<int> as a new lua table (1-based)
inline void lua_push_int_table(lua_State *L, const std::vector<int> &vals)
{
    lua_newtable(L);
    for (size_t i = 0; i < vals.size(); i++)
    {
        lua_pushinteger(L, vals[i]);
        lua_rawseti(L, -2, (int)(i + 1));
    }
}

// push a std::vector<float> as a new lua table (1-based)
inline void lua_push_float_table(lua_State *L, const std::vector<float> &vals)
{
    lua_newtable(L);
    for (size_t i = 0; i < vals.size(); i++)
    {
        lua_pushnumber(L, vals[i]);
        lua_rawseti(L, -2, (int)(i + 1));
    }
}