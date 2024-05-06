#ifndef _DEBUG_H_
#define _DEBUG_H_

#define DBG_LOG(fmt,...) \
   do{ printf( fmt, __VA_ARGS__); } while(0)

#define WARN DBG_LOG

#endif//_DEBUG_H_