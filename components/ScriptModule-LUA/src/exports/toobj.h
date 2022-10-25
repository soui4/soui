#ifndef DEF_TOOBJ
#define DEF_TOOBJ(luaStat,x) 		lua_tinker::def(luaStat,"to"#x,&sobj_cast<x>)
#endif