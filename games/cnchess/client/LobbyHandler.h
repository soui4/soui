#ifndef LOBBYHANDLER_H
#define LOBBYHANDLER_H

#include <sobject/Sobject.hpp>
#include <helper/obj-ref-impl.hpp>
#include "WebSocketClient.h"
#include "ConnListener-i.h"
class CTableAdapter;

/**
 * @brief 大厅处理器类
 * 
 * 该类负责处理游戏大厅相关的逻辑，包括房间信息、桌子信息、座位信息等的管理和处理。
 * 同时负责与服务器通过WebSocket进行通信，并处理各种网络消息。
 */
class LobbyHandler : public TObjRefImpl<SObject>, public WebSocketClient::IListener, public IConnListener2
{
    DEF_SOBJECT(SObject, L"lobbyhandler")
    friend class CTableAdapter;
    
public:
    /**
     * @brief 构造函数
     */
    LobbyHandler();
    
    /**
     * @brief 析构函数
     */
    virtual ~LobbyHandler();

    /**
     * @brief 初始化大厅处理器
     * @param pRoot 根窗口指针
     * @param pWs WebSocket客户端指针
     */
    void Init(SWindow *pRoot, WebSocketClient *pWs);

public:
    /**
     * @brief 事件映射表
     */
    EVENT_MAP_BEGIN()
        EVENT_CHECK_SENDER_ROOT(m_pRoot)
    EVENT_MAP_BREAK()
    
public:
    /**
     * @brief WebSocket连接建立回调
     */
    void OnConnected() override;
    
    /**
     * @brief WebSocket连接断开回调
     */
    void OnDisconnected() override;
    
    /**
     * @brief 处理收到的网络消息
     * @param dwType 消息类型
     * @param data 消息数据
     * @return 是否处理成功
     */
    BOOL OnMessage(DWORD dwType, std::shared_ptr<std::vector<BYTE> > data) override;
    
private:
    /**
     * @brief 处理房间信息消息
     * @param lpData 消息数据指针
     * @param nSize 消息数据大小
     * @return 是否处理成功
     */
    BOOL OnRoomInfo(const void *lpData, int nSize);
    
    /**
     * @brief 处理桌子信息消息
     * @param lpData 消息数据指针
     * @param nSize 消息数据大小
     * @return 是否处理成功
     */
    BOOL OnTableInfo(const void *lpData, int nSize);
    
    /**
     * @brief 处理坐下确认消息
     * @param lpData 消息数据指针
     * @param nSize 消息数据大小
     * @return 是否处理成功
     */
    BOOL OnSeatDownAck(const void *lpData, int nSize);
    
    /**
     * @brief 处理登录确认消息
     * @param lpData 消息数据指针
     * @param nSize 消息数据大小
     * @return 是否处理成功
     */
    BOOL OnLoginAck(const void *lpData, int nSize);
    
    /**
     * @brief 请求坐下
     * @param iTable 桌号
     * @param iSeat 座位号
     */
    void ReqSeatDown(int iTable, int iSeat);
    
  private:
    SWindow *m_pRoot;              ///< 根窗口指针
    WebSocketClient *m_ws;         ///< WebSocket客户端指针
    CTableAdapter *m_pAdapter;     ///< 桌子适配器指针
};

#endif//LOBBYHANDLER_H