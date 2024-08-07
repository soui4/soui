#ifndef _DEBUG_H_
#define _DEBUG_H_

#define GET_NTH_ARG(_1, _2, _3, _4, _5, N, ...) N
#define CHECK_ARGS(...) GET_NTH_ARG(__VA_ARGS__, 5, 4, 3, 2, 1, 0)

#define DBG_LOG(fmt,...) \
   do{ printf( fmt, __VA_ARGS__); } while(0)

#define DBG_LOG0(msg) \
	do { printf(msg); } while (0)

#define WARN0 DBG_LOG0
#define WARN DBG_LOG

#endif//_DEBUG_H_