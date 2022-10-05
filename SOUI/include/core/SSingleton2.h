/**
 * Copyright (C) 2014-2050
 * All rights reserved.
 *
 * @file       SSingleton.h
 * @brief
 * @version    v1.0
 * @author     SOUI group
 * @date       2014/08/02
 *
 * Describe    SOUI系统中使用的单件模块
 */

#pragma once

#include <assert.h>
#include <SApp.h>
SNSBEGIN

#define SINGLETON2_TYPE(x)     \
  public:                      \
    friend class SApplication; \
    static SingletonType GetType()       \
    {                          \
        return x;              \
    }

/**
 * @class      SSingleton2
 * @brief      伪单件模板，通过真当件SApplication来获取指针。
 *
 * Describe
 */
template <typename T>
class SSingleton2 {
  public:
    SSingleton2(void)
    {
    }
    virtual ~SSingleton2(void)
    {
    }
    static T &getSingleton(void)
    {
        assert(getObjPtr());
        return (*getObjPtr());
    }
    static T *getSingletonPtr(void)
    {
        assert(getObjPtr());
        return getObjPtr();
    }

  private:
    static T *getObjPtr()
    {
        return (T *)SApplication::getSingletonPtr()->GetInnerSingleton(T::GetType());
    }

    SSingleton2 &operator=(const SSingleton2 &)
    {
        return *this;
    }
    SSingleton2(const SSingleton2 &)
    {
    }
};

SNSEND