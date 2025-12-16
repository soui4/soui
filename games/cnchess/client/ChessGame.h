#pragma once

#include <cnchessProtocol.h>
#include <gif/SGifPlayer.h>
#include <protocol.h>
#include "events.h"
#include "SGameTheme.h"
#include "WebSocketClient.h"
#include "ConnListener-i.h"
#include <sobject/Sobject.hpp>
#include <helper/obj-ref-impl.hpp>

class CMainDlg;
class CChessGame : public TObjRefImpl<SObject>, public WebSocketClient::IListener, public IConnListener2, public IAnimatorGroupListerer
{
    DEF_SOBJECT(SObject, L"chessgame")     // 定义SObject的类名和类别名
public:
    CChessGame(CMainDlg* pMainDlg, SGameTheme* pTheme);
    virtual ~CChessGame();
    
    /**
     * @brief 游戏阶段枚举
     */
    enum STAGE{
        STAGE_INIT=0,       ///< 初始化阶段
        STAGE_CONNECTING,   ///< 正在连接服务器
        STAGE_CONTINUE,     ///< 游戏尚未开始
        STAGE_PLAYING,      ///< 游戏中
    };
    
   
    /**
     * @brief 初始化游戏
     * @param pGameBoard 游戏面板窗口指针
     * @param pWs WebSocket客户端指针
     */
    void Init(SWindow *pGameBoard, WebSocketClient *pWs);

    /**
     * @brief 将玩家索引转换为座位索引
     * @param nIndex 玩家索引
     * @return 座位索引
     */
    int Index2Seat(int nIndex) { return (nIndex + PLAYER_COUNT - m_iSelfIndex) % PLAYER_COUNT; }

    /**
     * @brief 获取游戏属性
     * @return 游戏属性数组指针
     */
    const DWORD *  GetProps() const { return m_dwProps; }

    /**
     * @brief 获取自己的索引
     * @return 自己的索引
     */
    int GetSelfIndex() const { return m_iSelfIndex; }

    /**
     * @brief 获取庄家索引
     * @return 庄家索引
     */
    int GetBankerIndex() const { return m_iBankerIndex; }
    
public:
    void WINAPI OnAnimatorGroupEnd(IAnimatorGroup * pGroup, int nID) override;

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

protected:
private:
    CMainDlg* m_pMainDlg;               ///< 主窗口指针
    SWindow * m_pGameBoard;             ///< 游戏面板
    STAGE m_stage;                      ///< 当前游戏阶段
    SAutoRefPtr<SGameTheme> m_pTheme;   ///< 游戏主题

    //player is index recveived from server, same as index
    //seat is game view index. bottom is 0, left is 1, right is 2, top is 3
    int m_iSelfIndex;                   ///< 自己的座位号
    int m_iBankerIndex;                 ///< 庄家座位号
    BOOL m_bReady[PLAYER_COUNT];        ///< 玩家准备状态
    enum RoundResult{                   ///< 本局结果，用于显示动画，不参与结算
        RESULT_UNKNOWN = 0,             ///< 未知结果
        RESULT_WIN = 1,                 ///< 获胜
        RESULT_LOSE = 2,                ///< 失败
    };
    RoundResult m_roundResult;          ///< 回合结果
    // 网络通信
    WebSocketClient *m_webSocketClient;                         ///< WebSocket客户端
    std::shared_ptr<GS_USERINFO> m_pUserInfo[PLAYER_COUNT];     ///< 用户信息
    std::shared_ptr<std::vector<BYTE> > m_userAvatar[PLAYER_COUNT]; ///< 用户头像数据
    DWORD m_dwProps[MAX_PROP];          ///< 游戏属性

    SList<SAutoRefPtr<IAnimatorGroup> > m_animatorGroupList;
};
