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
#include "myprofile.h"
#include <list>
#include "ChessBoard.h"

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
class CChessGame : public TObjRefImpl<SObject>, public WebSocketClient::IListener, public IConnListener2, public IAnimatorListener
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

    /**
     * @brief 常量定义枚举
     */
    enum{
        ID_AVATAR_BASE = 2000,        ///< 头像ID基准值
        ID_ALARM_CLOCK_ME = 4000,        ///< 闹钟ID
        ID_ALARM_CLOCK_ENEMY = 4001,        ///< 闹钟ID
        ID_FLAG_BASE = 5000,
        ID_SHADOW_BASE = 6000,
        ID_CHESS_BASE = 7000,    
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

    BOOL OnChessPieceClick(IEvtArgs *e);

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
    int GetEnemyIndex() const { return (m_iSelfIndex + 1) % PLAYER_COUNT; }
    void ShowPosFlags(POINT pt, BOOL bShow);

    void DeselectPiece(int nId);

    void SelectPiece(int nId);

    void MovePiece(int nId, POINT pt);

    void SelectAndMovePiece(int nId, POINT pt);
    BOOL IsRedSide() const { return m_iRedIndex == m_iSelfIndex; }

    int GetActivePlayerIndex() const;
    int GetScale() const ;
    void OnTimeout();
    void ShowClock(int iSeat,BOOL bShow);
    POINT PtNet2Local(POINT ptNet) const;
    POINT PtLocal2Net(POINT ptLocal) const;



  public:
    STDMETHOD_(void, onAnimationStart)(THIS_ IValueAnimator * pAnimator) override{}
    STDMETHOD_(void, onAnimationEnd)(THIS_ IValueAnimator * pAnimator) override;
    STDMETHOD_(void, onAnimationRepeat)(THIS_ IValueAnimator * pAnimator) override{}
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
    void OnBtnTest();
    /**
     * @brief 开始游戏按钮点击事件处理
     */
    void OnBtnStart();

    void OnBtnReqPeace();

    void OnBtnReqSurrender();

    void OnBtnReqRegret();

    void OnSetActivePlayerIndex(int nActiveSeat);
    void OnGifVectoryOver(IEvtArgs *e);
    /**
     * @brief 处理表格信息消息
     * @param e 事件参数
     */
    void OnTableInfo(IEvtArgs *e);

    /**
     * @brief 游戏阶段改变处理
     * @param stage 新的游戏阶段
     */
    void OnStageChanged(STAGE stage);

    /**
     * @brief 事件映射表
     */
    EVENT_MAP_BEGIN()
        EVENT_CHECK_SENDER_ROOT(m_pGameBoard)
        EVENT_NAME_HANDLER(m_pGameBoard->GetName(), EventSwndSize::EventID, OnGameBoardSizeChanged)
        EVENT_NAME_COMMAND(L"btn_test", OnBtnTest)
        EVENT_NAME_COMMAND(Widgets::kbtn_start, OnBtnStart)
        EVENT_NAME_COMMAND(Widgets::kbtn_req_peace, OnBtnReqPeace)
        EVENT_NAME_COMMAND(Widgets::kbtn_req_surrender,OnBtnReqSurrender)
        EVENT_NAME_COMMAND(Widgets::kbtn_req_regret,OnBtnReqRegret)
        EVENT_NAME_HANDLER(Sprites::sprite_vectory, EventGifPlayOver::EventID, OnGifVectoryOver)
        EVENT_HANDLER(EVT_TABLE_INFO, OnTableInfo)
    EVENT_MAP_BREAK()
public:
    /**
     * @brief 定时器事件处理
     * @param uIDEvent 定时器ID
     */
  void OnTimer(UINT_PTR uIDEvent);
    
    /**
     * @brief 消息映射表
     */
    BEGIN_MSG_MAP_EX(CChessGame)
        MSG_WM_TIMER(OnTimer)
    END_MSG_MAP()
protected:
    /**
     * @brief 显示等待动画
     * @param pszTip 等待提示文本
     */
    void PlayWattingAnimation(LPCTSTR pszTip);

    /**
     * @brief 停止等待动画
     */
    void StopWatingAnimation();

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
    void OnAckRegret(const void *pData, int nSize);

    /**
     * @brief 处理求和请求
     * @param pData 消息数据指针
     * @param nSize 消息数据大小
     */
    void OnAckPeace(const void *pData, int nSize);

    /**
     * @brief 处理求和请求
     * @param pData 消息数据指针
     * @param nSize 消息数据大小
     */
    void OnReqPeace(const void *pData, int nSize);

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
     * @brief 游戏消息处理函数
     * @param pData 消息数据指针
     * @param nSize 消息数据大小
     */
    void OnGameMsg(const void *pData, int nSize);
    
    /**
     * @brief 更新时钟显示
     * @param nSecond 剩余秒数
     * @param iSeat 座位号
     */
    void UpdateClock(int nSecond, int iSeat);
    
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
    
    void UndoLastMove();
    void OnChessMove(const MOVESTEP & mstep,BOOL bCheckResult);
    UINT GetFarthestRepeat(CHESSMAN & chsEnemy); //计算当前步最远的一次重复走棋，在计算长捉时使用
    BOOL CheckMove(POINT ptFrom,POINT ptTo, BOOL bSilent=FALSE);
private:
    CMainDlg* m_pMainDlg;               ///< 主窗口指针
    CChessBoard * m_pGameBoard;         ///< 游戏面板
    STAGE m_stage;                      ///< 当前游戏阶段
    SAutoRefPtr<SGameTheme> m_pTheme;   ///< 游戏主题

    int m_iSelfIndex;                   ///< 自己的座位号
    BOOL m_bReady[PLAYER_COUNT];        ///< 玩家准备状态
    enum RoundResult{                   ///< 本局结果，用于显示动画，不参与结算
        RESULT_NEUTRAL = 0,             ///< 平局结果
        RESULT_WIN = 1,                 ///< 获胜
        RESULT_LOSE = 2,                ///< 失败
    };
    RoundResult m_roundResult;          ///< 回合结果
    // 网络通信
    WebSocketClient *m_webSocketClient;                         ///< WebSocket客户端
    std::shared_ptr<GS_USERINFO> m_pUserInfo[PLAYER_COUNT];     ///< 用户信息
    std::shared_ptr<std::vector<BYTE> > m_userAvatar[PLAYER_COUNT]; ///< 用户头像数据
    DWORD m_dwProps[MAX_PROP];          ///< 游戏属性

    SAutoRefPtr<SImageWnd> m_pChessBoard;
    float m_cellWidth;
    float m_cellHeight;
    CPoint  m_ptBoardOrigin;    ///棋盘左下角坐标

    CChessLayout    m_layout;
    CChsLytState    m_LytState;
    int  m_nSelectedChessID;
    int  m_iRedIndex; //红方座位号

    int m_nUsedRegretCount;//已使用的悔棋次数
    std::list<MOVESTEP> m_history;
    int  m_nUndoLeft;   //正在执行悔棋过程时，记录剩余需要继续undo的步数
    BOOL m_bStuding;    //是否处于研究模式
};