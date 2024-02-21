/**
* Copyright (C) 2014-2050 
* All rights reserved.
* 
* @file       SCriticalSection.h
* @brief      
* @version    v1.0      
* @author     SOUI group   
* @date       2014/09/27
* 
* Describe    CriticalSection 包装
*/

#ifndef __SCRITICALSECTION__H__
#define __SCRITICALSECTION__H__
#include <platform_exp.h>
#include <sdef.h>

SNSBEGIN

    class SCriticalSectionImpl;
    class PLATFORM_API SCriticalSection
    {
    public:
		SCriticalSection();
		virtual ~SCriticalSection();

		void Enter();
		void Leave();
    protected:

        SCriticalSectionImpl*    m_cs;
    };

    class PLATFORM_API SAutoLock
    {
    public:
        SAutoLock(SCriticalSection & cs):m_cs(cs)
        {
            m_cs.Enter();
        }

        ~SAutoLock()
        {
            m_cs.Leave();
        }
    protected:
        SCriticalSection & m_cs;
    };
SNSEND

#endif // __SCRITICALSECTION__H__