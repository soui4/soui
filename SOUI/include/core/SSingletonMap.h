#pragma once
#include "SSingleton2.h"
#include "SCmnMap.h"
SNSBEGIN

template <class TClass, class TObj, class TKey = SStringA>
class SSingletonMap
    : public SSingleton2<TClass>
    , public SCmnMap<TObj, TKey> {
};

SNSEND