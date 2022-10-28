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