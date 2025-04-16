/**
 * Copyright (C) 2014-2050
 * All rights reserved.
 *
 * @file       SSingleton2.h
 * @brief      Pseudo-Singleton Template
 * @version    v1.0
 * @author     SOUI group
 * @date       2014/08/02
 *
 * Description: Provides a pseudo-singleton template in SOUI, which uses the true singleton SApplication to manage instances.
 */

 #ifndef __SSINGLETON2__H__
 #define __SSINGLETON2__H__
 
 #include <assert.h>
 
 SNSBEGIN
 
 /**
  * @def SINGLETON2_TYPE(x)
  * @brief Macro to define the singleton type.
  * @param x Type identifier for the singleton.
  */
 #define SINGLETON2_TYPE(x)         \
   public:                          \
     friend class SApplication;     \
     static SingletonType GetType() \
     {                              \
         return x;                  \
     }
 
 /**
  * @class      SSingleton2
  * @brief      Pseudo-Singleton Template
  *
  * Description: Implements a pseudo-singleton pattern where the actual singleton management is handled by the SApplication class.
  *              This allows for centralized management of singleton instances.
  * @tparam     T Type of the class that will be made a pseudo-singleton.
  */
 template <typename T>
 class SSingleton2 {
 public:
     /**
      * @brief Constructor for SSingleton2.
      */
     SSingleton2(void) {}
 
     /**
      * @brief Destructor for SSingleton2.
      */
     virtual ~SSingleton2(void) {}
 
     /**
      * @brief Gets the singleton instance.
      * @return Reference to the singleton instance.
      * @note Asserts that the singleton instance exists.
      */
     static T &getSingleton(void) {
         assert(getObjPtr());
         return (*getObjPtr());
     }
 
     /**
      * @brief Gets the pointer to the singleton instance.
      * @return Pointer to the singleton instance.
      * @note Asserts that the singleton instance exists.
      */
     static T *getSingletonPtr(void) {
         assert(getObjPtr());
         return getObjPtr();
     }
 
 private:
     /**
      * @brief Gets the pointer to the singleton object managed by SApplication.
      * @return Pointer to the singleton object.
      */
     static T *getObjPtr() {
         return (T *)SApplication::getSingletonPtr()->GetInnerSingleton(T::GetType());
     }
 
     /**
      * @brief Private copy assignment operator to prevent copying.
      * @param rhs Right-hand side object.
      * @return Reference to the current object.
      */
     SSingleton2 &operator=(const SSingleton2 &) {
         return *this;
     }
 
     /**
      * @brief Private copy constructor to prevent copying.
      * @param rhs Right-hand side object.
      */
     SSingleton2(const SSingleton2 &) {}
 };
 
 SNSEND
 
 #endif // __SSINGLETON2__H__