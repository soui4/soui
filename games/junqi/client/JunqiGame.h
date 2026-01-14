#pragma once

#include <junqiProtocol.h>
#include <gif/SGifPlayer.h>
#include <protocol.h>
#include "events.h"
#include "SGameTheme.h"
#include "WebSocketClient.h"
#include "ConnListener-i.h"
#include <sobject/Sobject.hpp>
#include <helper/obj-ref-impl.hpp>
#include <JunqiLayout.h>
#include "myprofile.h"
#include <list>
#include "JunqiBoard.h"

class CMainDlg;

/**
 * @brief 四国军棋游戏核心逻辑类
 * 
 * 该类负责处理四国军棋游戏的核心逻辑，包括：
 * 1. 游戏状态管理
 * 2. 玩家交互处理
 * 3. 游戏数据管理
 * 4. 与UI的交互
 */
class CJunqiGame : public TObjRefImpl<SObject>, public WebSocketClient::IListener, public IConnListener2, public IAnimatorListener, IAnimatorUpdateListener
{
    DEF_SOBJECT(SObject, L"junqigame")     // 定义SObject的类名和类别名
    
    /**
     * @brief 游戏阶段枚举
     */
    enum STAGE{
        STAGE_INIT=0,       ///< 初始化阶段
        STAGE_CONNECTING,   ///< 正在连接服务器
        STAGE_CONTINUE,     ///< 游戏尚未开始
        STAGE_WAIT_START,   ///< 等待开始
        STAGE_PLAYING,      ///< 游戏中
        STAGE_GAMEOVER,   ///< 游戏结束
    };
    
    /**
     * @brief 座位索引枚举
     */
    enum SEAT_INDEX{
        SEAT_INDEX_ME = 0,      ///< 自己的位置
        SEAT_INDEX_BOTTOM = 0,  ///< 界面底部座位，等于自己
        SEAT_INDEX_LEFT = 1,    ///< 界面左侧座位
        SEAT_INDEX_TOP = 2,     ///< 界面顶部座位
        SEAT_INDEX_RIGHT = 3,   ///< 界面右侧座位
    };

    /**
     * @brief 常量定义枚举
     */
    enum{
        ID_AVATAR_BASE = 2000,        ///< 头像ID基准值
        ID_ALARM_CLOCK = 4000,        ///< 闹钟ID
        ID_DIR_BASE = 7000,             ///< 行棋方向标志ID基准值
        ID_CHESS_BASE = 6000,          ///< 棋子ID基准值
    };

public:
    /**
     * @brief 构造函数
     * @param pMainDlg 主窗口指针
     * @param pTheme 游戏主题指针
     */
    CJunqiGame(CMainDlg* pMainDlg, SGameTheme* pTheme);
    
    /**
     * @brief 析构函数
     */
    virtual ~CJunqiGame();

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
    int Index2Seat(int nIndex);

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

    void DeselectPiece();

    void ClearMovePath();

    void SelectPiece(int nId);

    BOOL DoMovePiece(POINT ptFrom, POINT ptTo,BOOL bSapper, int nResult);
    BOOL MovePiece(int nId, POINT pt);

    int GetActivePlayerIndex() const;
    int GetScale() const ;
    POINT PtNet2Local(POINT ptNet) const;
    POINT PtLocal2Net(POINT ptLocal) const;

    int CalcMoveDir(POINT ptFrom, POINT ptTo);

  public:
    STDMETHOD_(void, onAnimationStart)(THIS_ IValueAnimator *pAnimator) override;
    STDMETHOD_(void, onAnimationEnd)(THIS_ IValueAnimator * pAnimator) override;
    STDMETHOD_(void, onAnimationRepeat)(THIS_ IValueAnimator *pAnimator) override
    {
    }
    STDMETHOD_(void, onAnimationUpdate)(THIS_ IValueAnimator * pAnimator) override;
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

    void OnSeatDownAck(const void *lpData, int nSize);

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

    void OnBtnReqPass();

    void OnBtnNewRound();
    void OnBtnReplay();
    void OnBtnSave();
    void OnBtnLoad();
    void OnBtnPrev();
    void OnBtnNext();

    void MoveClock(int nSeat, int value);
    void HideClock();
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

    void OnChessPieceClick(IEvtArgs *e);
    void OnBoardClick(IEvtArgs *e);
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
        EVENT_NAME_COMMAND(Widgets::kbtn_req_pass,OnBtnReqPass)
        EVENT_NAME_COMMAND(Widgets::kbtn_new_round,OnBtnNewRound)
        EVENT_NAME_COMMAND(Widgets::kbtn_replay,OnBtnReplay)
        EVENT_NAME_COMMAND(Widgets::kbtn_save,OnBtnSave)
        EVENT_NAME_COMMAND(Widgets::kbtn_load,OnBtnLoad)
        EVENT_NAME_COMMAND(Widgets::kbtn_prev,OnBtnPrev)
        EVENT_NAME_COMMAND(Widgets::kbtn_next,OnBtnNext)
        EVENT_NAME_HANDLER(Sprites::sprite_vectory, EventGifPlayOver::EventID, OnGifVectoryOver)
        EVENT_NAME_HANDLER(Sprites::board_main, EvtGridClick::EventID, OnBoardClick)
        EVENT_ID_RANGE_HANDLER(ID_CHESS_BASE, ID_CHESS_BASE + 400, EVT_CMD, OnChessPieceClick)
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
    BEGIN_MSG_MAP_EX(CJunqiGame)
        MSG_WM_TIMER(OnTimer)
    END_MSG_MAP()
protected:
    void InitLayout(BYTE byLayout[13], int idx);
    void OnStartFight(const void *pData, int nSize);

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
     * @brief 处理投降消息
     * @param pData 消息数据指针
     * @param nSize 消息数据大小
     */
    void OnReqSurrender(const void *pData, int nSize);
    void OnActivePlayer(const void *pData, int nSize);
    void OnPlayerLose(const void *pData, int nSize);
    void OnFlagPos(const void *pData, int nSize);
    
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

    BOOL CheckMove(POINT ptFrom,POINT ptTo, BOOL bSilent=FALSE);
private:
    CMainDlg* m_pMainDlg;               ///< 主窗口指针
    CJunqiBoard * m_pGameBoard;         ///< 游戏面板
    STAGE m_stage;                      ///< 当前游戏阶段
    SAutoRefPtr<SGameTheme> m_pTheme;   ///< 游戏主题

    int m_iSelfIndex;                   ///< 自己的座位号
    BOOL m_bReady[PLAYER_COUNT];        ///< 玩家准备状态
    enum RoundResult{                   ///< 本局结果，用于显示动画，不参与结算
        RESULT_DRAW = 0,                ///< 平局结果
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

    int  m_nSelectedChessID;
    int  m_iActiveIndex; //当前活动座位号
    BOOL m_bMoving;     //是否正在移动
    CJunQiLayout m_layout;  //布局
    typedef std::list<std::pair<DWORD, std::shared_ptr<std::vector<BYTE> > > > HistoryList; //历史记录列表类型
    HistoryList m_lstHistory; //历史记录列表实例
    HistoryList::iterator m_itCurrentHistory;  //当前复盘位置
    BYTE m_byInitLayouts[PLAYER_COUNT][13];
    GS_USERINFO m_userInfo[PLAYER_COUNT];
    
    // 复盘相关成员变量
    int m_nHistoryIndex;        //当前历史步数（从0开始）
    BOOL m_bReplay;             //是否处于复盘模式
    BOOL m_bEnableAnimation;    //是否启用动画（复盘时禁用以提高速度）
    
    // 复盘相关函数
    void InitReplay();          //初始化复盘
    void ProcessHistoryMessage(DWORD dwType, const void *pData, int nSize); //处理历史消息（复盘用）
    void StepForward();         //前进一步
    void StepBackward();        //后退一步
    void FastReplayToStep(int nTargetStep);  //快速重放到指定步数（无动画）
};