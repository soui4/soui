#ifndef __SSINGLETONMAP__H__
#define __SSINGLETONMAP__H__
#include "SSingleton2.h"
#include "SCmnMap.h"
SNSBEGIN

template <class TClass, class TObj, class TKey = SStringA>
class SSingletonMap
    : public SSingleton2<TClass>
    , public SCmnMap<TObj, TKey> {
};

SNSEND
#endif // __SSINGLETONMAP__H__