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
#include <ChessLayout.h>

class CMainDlg;

/**
 * @brief 中国象棋游戏核心逻辑类
 * 
 * 该类负责处理中国象棋游戏的核心逻辑，包括：
 * 1. 游戏状态管理
 * 2. 玩家交互处理
 * 3. 游戏数据管理
 * 4. 与UI的交互
 */
class CChessGame : public TObjRefImpl<SObject>, public WebSocketClient::IListener, public IConnListener2, public IAnimatorGroupListerer
{
    DEF_SOBJECT(SObject, L"chessgame")     // 定义SObject的类名和类别名
    
    /**
     * @brief 游戏阶段枚举
     */
    enum STAGE{
        STAGE_INIT=0,       ///< 初始化阶段
        STAGE_CONNECTING,   ///< 正在连接服务器
        STAGE_CONTINUE,     ///< 游戏尚未开始
        STAGE_WAIT_START,   ///< 等待开始
        STAGE_PLAYING,      ///< 游戏中
    };
    
    /**
     * @brief 座位索引枚举
     */
    enum SEAT_INDEX{
        SEAT_INDEX_ME = 0,      ///< 自己的位置
        SEAT_INDEX_BOTTOM = 0,  ///< 界面底部座位，等于自己
        SEAT_INDEX_TOP = 1,     ///< 界面顶部座位
    };

public:
    /**
     * @brief 构造函数
     * @param pMainDlg 主窗口指针
     * @param pTheme 游戏主题指针
     */
    CChessGame(CMainDlg* pMainDlg, SGameTheme* pTheme);
    
    /**
     * @brief 析构函数
     */
    virtual ~CChessGame();
    
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
    
public:
    static POINT CALLBACK ChessAnchor2Pos(const AnchorPos &pos, const CRect &rcParent, const CSize & szChild, int nScale, void * userData);   
    void OnGameBoardSizeChanged(IEvtArgs *e);
    /**
     * @brief 事件映射表
     */
    EVENT_MAP_BEGIN()
        EVENT_CHECK_SENDER_ROOT(m_pGameBoard)
        EVENT_NAME_HANDLER(m_pGameBoard->GetName(), EventSwndSize::EventID, OnGameBoardSizeChanged)
    EVENT_MAP_BREAK()
public:
    /**
     * @brief 定时器事件处理
     * @param uIDEvent 定时器ID
     */
    void OnTimer(UINT_PTR uIDEvent){}
    
    /**
     * @brief 消息映射表
     */
    BEGIN_MSG_MAP_EX(UpgradeGame)
        MSG_WM_TIMER(OnTimer)
    END_MSG_MAP()
protected:
    /**
     * @brief 处理游戏开始消息
     * @param pData 消息数据指针
     * @param nSize 消息数据大小
     */
    void OnGameStart(const void *pData, int nSize);
    
    /**
     * @brief 处理走棋消息
     * @param pData 消息数据指针
     * @param nSize 消息数据大小
     */
    void OnMoveChess(const void *pData, int nSize);
    
    /**
     * @brief 处理悔棋请求消息
     * @param pData 消息数据指针
     * @param nSize 消息数据大小
     */
    void OnUndoRequest(const void *pData, int nSize);
    
    /**
     * @brief 处理悔棋响应消息
     * @param pData 消息数据指针
     * @param nSize 消息数据大小
     */
    void OnUndoResponse(const void *pData, int nSize);
    
    /**
     * @brief 处理游戏结束消息
     * @param pData 消息数据指针
     * @param nSize 消息数据大小
     */
    void OnGameOver(const void *pData, int nSize);
    
    /**
     * @brief 登录应答消息处理函数
     * @param pData 消息数据指针
     * @param nSize 消息数据大小
     */
    void OnLoginAck(const void *pData, int nSize);
    
    /**
     * @brief 头像应答消息处理函数
     * @param pData 消息数据指针
     * @param nSize 消息数据大小
     */
    void OnAvatarAck(const void *pData, int nSize);
    
    /**
     * @brief 更新时钟显示
     * @param nSecond 剩余秒数
     */
    void UpdateClock(int nSecond);
    
    /**
     * @brief 显示提示信息
     * @param strTip 提示信息
     */
    void PlayTip(const SStringT &strTip);
    
    /**
     * @brief 通过WebSocket发送消息
     * @param dwType 消息类型
     * @param lpData 消息数据指针
     * @param dwSize 消息数据大小
     */
    void wsSendMsg(DWORD dwType, LPVOID lpData, DWORD dwSize);
    
    /**
     * @brief 播放特效音效
     * @param pszSound 音效名称
     */
    void PlayEffectSound(LPCWSTR pszSound);
    
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
    SAutoRefPtr<SImageWnd> m_pChessBoard;
    CSize   m_szCell;
    CPoint  m_ptBoardCenter;
};