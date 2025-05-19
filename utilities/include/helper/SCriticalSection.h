/**
 * @file SCriticalSection.h
 * @brief Header file for SCriticalSection and SAutoLock classes.
 * @version v1.0
 * @author SOUI group
 * @date 2014/09/27
 *
 * @details This file provides a wrapper for critical sections and an auto-lock mechanism.
 */

 #ifndef __SCRITICALSECTION__H__
 #define __SCRITICALSECTION__H__
 
 #include <utilities-def.h>
 
 SNSBEGIN
 
 /**
  * @class SCriticalSectionImpl
  * @brief Implementation class for critical section.
  * @note This class is intended for internal use.
  */
 class SCriticalSectionImpl;
 
 /**
  * @class SCriticalSection
  * @brief Wrapper class for a critical section.
  * @details This class provides methods to enter and leave a critical section, ensuring thread safety.
  */
 class UTILITIES_API SCriticalSection
 {
 public:
     /**
      * @brief Constructor.
      * Initializes a new critical section.
      */
     SCriticalSection();
 
     /**
      * @brief Destructor.
      * Destroys the critical section.
      */
     virtual ~SCriticalSection();
 
     /**
      * @brief Enters the critical section.
      * @details This method blocks the calling thread until the critical section is available.
      */
     void Enter();
 
     /**
      * @brief Leaves the critical section.
      * @details This method releases the critical section, allowing other threads to enter.
      */
     void Leave();
 
 protected:
     SCriticalSectionImpl* m_cs; /**< Pointer to the critical section implementation. */
 };
 
 /**
  * @class SAutoLock
  * @brief Auto-lock class for managing critical sections.
  * @details This class automatically enters the critical section upon construction and leaves it upon destruction.
  */
 class UTILITIES_API SAutoLock
 {
 public:
     /**
      * @brief Constructor.
      * @param cs Reference to the SCriticalSection object to manage.
      * @details Enters the critical section.
      */
     SAutoLock(SCriticalSection & cs) : m_cs(cs)
     {
         m_cs.Enter();
     }
 
     /**
      * @brief Destructor.
      * @details Leaves the critical section.
      */
     ~SAutoLock()
     {
         m_cs.Leave();
     }
 
 protected:
     SCriticalSection & m_cs; /**< Reference to the managed SCriticalSection object. */
 };
 
 SNSEND
 
 #endif // __SCRITICALSECTION__H__