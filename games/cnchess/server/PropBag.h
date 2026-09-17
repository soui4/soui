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
    int m_nRobotTimeMs[3]; // 按智力等级(初级/中级/高级)的思考时间预算(毫秒, 0=固定深度)
    DWORD m_dwMinVersion; // 服务器支持的最低客户端协议版本(config.xml min_version, 0=不限制)
public:
    enum{
        DEF_PORT = 3311,
    };
    DWORD m_dwProps[MAX_PROP];

    DWORD GetProp(DWORD dwProp) const;

    void Init(LPCTSTR pszPropXml);

    WORD GetPort() const;

    // 服务器支持的最低客户端协议版本(0=不限制), 低于该版本的客户端登录时被拒绝
    DWORD GetMinVersion() const { return m_dwMinVersion; }

    // 机器人AI线程池线程数
    int GetRobotAIPool() const { return m_nRobotPool; }
    // 机器人思考时间预算(毫秒, 0=由固定深度决定)。按智力等级取对应档位, 非法等级回退初级
    int GetRobotAITimeMs(int nLevel = ROBOT_LEVEL_BEGINNER) const
    {
        int idx = (nLevel >= ROBOT_LEVEL_BEGINNER && nLevel <= ROBOT_LEVEL_ADVANCED)
                      ? nLevel - ROBOT_LEVEL_BEGINNER
                      : 0;
        return m_nRobotTimeMs[idx];
    }
public:
    PropBag();
    ~PropBag();

};

SNSEND

#endif//__PROPBAG_H__
