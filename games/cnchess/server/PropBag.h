#ifndef __PROPBAG_H__
#define __PROPBAG_H__

#include <core/SSingleton.h>
#include <cnchessProtocol.h>
#include <windows.h>
SNSBEGIN
class PropBag : public SSingleton<PropBag>
{
    WORD m_wPort;
    int m_nRobotPool;
public:
    enum{
        DEF_PORT = 3311,
    };
    DWORD m_dwProps[MAX_PROP];

    DWORD GetProp(DWORD dwProp) const;
    
    void Init(LPCTSTR pszPropXml);

    WORD GetPort() const;

    // 机器人AI线程池线程数
    int GetRobotAIPool() const { return m_nRobotPool; }
public:
    PropBag();
    ~PropBag();

};

SNSEND

#endif//__PROPBAG_H__
