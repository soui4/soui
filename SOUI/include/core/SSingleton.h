/**
 * Copyright (C) 2014-2050
 * All rights reserved.
 *
 * @file       SSingleton.h
 * @brief      Singleton Template
 * @version    v1.0
 * @author     SOUI group
 * @date       2014/08/02
 *
 * Description: Provides a template for implementing the Singleton design pattern in SOUI.
 */

 #ifndef __SSINGLETON__H__
 #define __SSINGLETON__H__
 
 #include <assert.h>
 
 SNSBEGIN
 
 /**
  * @class      SSingleton
  * @brief      Singleton Template
  *
  * Description: Implements the Singleton design pattern, ensuring that a class has only one instance
  *              and providing a global point of access to it.
  * @tparam     T Type of the class that will be made a singleton.
  */
 template <typename T>
 class SSingleton {
 protected:
     /**
      * @brief Static pointer to the singleton instance.
      */
     static T *ms_Singleton;
 
 public:
     /**
      * @brief Constructor for SSingleton.
      * @note Asserts that no instance already exists.
      */
     SSingleton(void) {
         assert(!ms_Singleton);
         ms_Singleton = static_cast<T *>(this);
     }
 
     /**
      * @brief Destructor for SSingleton.
      * @note Asserts that an instance exists and sets the singleton pointer to null.
      */
     virtual ~SSingleton(void) {
         assert(ms_Singleton);
         ms_Singleton = 0;
     }
 
     /**
      * @brief Gets the singleton instance.
      * @return Reference to the singleton instance.
      * @note Asserts that the singleton instance exists.
      */
     static T &getSingleton(void) {
         assert(ms_Singleton);
         return (*ms_Singleton);
     }
 
     /**
      * @brief Gets the pointer to the singleton instance.
      * @return Pointer to the singleton instance.
      */
     static T *getSingletonPtr(void) {
         return (ms_Singleton);
     }
 
 private:
     /**
      * @brief Private copy assignment operator to prevent copying.
      * @param rhs Right-hand side object.
      * @return Reference to the current object.
      */
     SSingleton &operator=(const SSingleton &) {
         return *this;
     }
 
     /**
      * @brief Private copy constructor to prevent copying.
      * @param rhs Right-hand side object.
      */
     SSingleton(const SSingleton &) {}
 };

 
 SNSEND
 
 #endif // __SSINGLETON__H__