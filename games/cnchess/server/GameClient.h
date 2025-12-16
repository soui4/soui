#ifndef __GAMECLIENT_H__
#define __GAMECLIENT_H__

#include <protocol.h>
#include <memory>
#include <vector>
#include <string>
#include <interface/ws-i.h>
#include <helper/obj-ref-impl.hpp>

// WebSocket客户端信息结构
class GameClient
{
  public:
	GameClient()
		: m_nTable(-1)
		, m_nIndex(-1)
		, m_dwState(0)
		, m_dwData(0)
		, m_bReady(FALSE)
	{
		m_userInfo.uid = 0; // 0表示未登录用户，登录后服务器会分配一个uid
	}
	~GameClient()
	{
    }
	GS_USERINFO m_userInfo;
	std::shared_ptr<std::vector<BYTE> > m_avatar;// 用户头像信息
	SOUI::SAutoRefPtr<SOUI::ISvrConnection> m_pConn;    // WebSocket连接
	std::string m_strGameName;  // 游戏名称
	int m_nTable;				// 游戏桌号
	int m_nIndex;                // 游戏座位
	BOOL m_bReady;				// 是否准备
	DWORD m_dwState;            // 游戏状态
	DWORD_PTR m_dwData;         // 扩展数据，由具体游戏使用的数据，一般是指向一个指针
};

typedef GameClient WSCLIENT, *PWSCLIENT;

#endif // !__GAMECLIENT_H__