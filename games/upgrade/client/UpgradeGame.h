#ifndef UPGRADEGAME_H
#define UPGRADEGAME_H

#include <gif/SGifPlayer.h>
#include <protocol.h>
#include "events.h"
#include "SGameTheme.h"
#include "CardRecieve.h"
#include "WebSocketClient.h"
#include "SSpriteCard.h"
#include "CardInHand.h"
#include "ConnListener-i.h"
#include <sobject/Sobject.hpp>
#include <helper/obj-ref-impl.hpp>

// 前向声明
class CMainDlg;

/**
 * @brief 升级游戏核心逻辑类
 * 
 * 该类负责处理升级游戏的核心逻辑，包括：
 * 1. 游戏状态管理
 * 2. 玩家交互处理
 * 3. 游戏数据管理
 * 4. 与UI的交互
 */
class UpgradeGame : public TObjRefImpl<SObject>, public WebSocketClient::IListener, public IConnListener2, public IAnimatorGroupListerer
{
    DEF_SOBJECT(SObject, L"upgradegame")     // 定义SObject的类名和类别名
    friend class CCardInHand;
    
    /**
     * @brief 游戏阶段枚举
     */
    enum STAGE{
        STAGE_INIT=0,       ///< 初始化阶段
        STAGE_CONNECTING,   ///< 正在连接服务器
        STAGE_CONTINUE,     ///< 游戏尚未开始
        STAGE_WAIT_START,   ///< 等待开始
        STAGE_WAIT_CARD,    ///< 等待发牌
        STAGE_POSTCARD,     ///< 发牌阶段
        STAGE_SHOW_BOTTOM,  ///< 亮底牌阶段
        STAGE_SETBOTTOM,    ///< 垫底牌阶段
        STAGE_REBELBOTTOM,  ///< 反底牌阶段
        STAGE_PUTCARD,      ///< 出牌阶段
    };
    
    /**
     * @brief 常量定义枚举
     */
    enum{
        ID_STACK_CARD_BASE = 1000,    ///< 牌堆卡片ID基准值
        ID_AVATAR_BASE = 2000,        ///< 头像ID基准值
        ID_AVATAR_FLAGS_BASE = 3000,  ///< 头像标记ID基准值
        ID_ALARM_CLOCK = 4000,        ///< 闹钟ID
        kCardInterval = 2,            ///< 卡牌间隔
        kCardCount = 108,             ///< 卡牌总数
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
     * @brief 标记ID枚举
     */
    enum FLAG_ID{ 
        FLAG_COLOR_1 = 1,   ///< 报主颜色标记1
        FLAG_COLOR_2,       ///< 报主颜色标记2
        FLAG_BANKER,        ///< 庄家标志
    };
    
public:
    /**
     * @brief 构造函数
     * @param pMainDlg 主窗口指针
     * @param pTheme 游戏主题指针
     */
    UpgradeGame(CMainDlg* pMainDlg, SGameTheme* pTheme);
    
    /**
     * @brief 析构函数
     */
    ~UpgradeGame();
    
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
    /**
     * @brief 获取当前等级
     * @return 当前等级
     */
    int GetLevel() const;

    /**
     * @brief 开始游戏按钮点击事件处理
     */
    void OnBtnStart();
    
    /**
     * @brief 报主按钮点击事件处理
     * @param nID 按钮ID
     */
    void OnBtnReportRange(int nID);
 
    /**
     * @brief 放弃按钮点击事件处理
     */
    void OnBtnPass();
    
    /**
     * @brief 出牌按钮点击事件处理
     */
    void OnBtnPutCard();
    
    /**
     * @brief 扣底牌按钮点击事件处理
     */
    void OnBtnSetBottom();
    
    /**
     * @brief 上一轮按钮点击事件处理
     */
    void OnBtnPrevRound();
    
    /**
     * @brief 看底牌按钮点击事件处理
     */
    void OnBtnSeeBottom();
    
    /**
     * @brief 测试按钮点击事件处理
     */
    void OnBtnTest();
    
    /**
     * @brief 洗牌动画循环结束事件处理
     * @param e 事件参数
     */
    void OnShuffleLoopOver(IEvtArgs *e);
    
    /**
     * @brief 洗牌动画帧事件处理
     * @param e 事件参数
     */
    void OnShuffleFrame(IEvtArgs *e);

    /**
     * @brief 高亮动画结束事件处理
     * @param e 事件参数
     */
    void OnGifHighlightOver(IEvtArgs *e);
    
    /**
     * @brief 胜利动画结束事件处理
     * @param e 事件参数
     */
    void OnGifVectoryOver(IEvtArgs *e);
    
    /**
     * @brief 等待动画帧事件处理
     * @param e 事件参数
     */
    void OnGifWaitingFrame(IEvtArgs *e);
    
    /**
     * @brief 处理一轮结束消息
     * @param pMsg 一轮结束消息指针
     */
    void OnTurnOverTimer(GAME_UPGRADE_TURNOVER *pMsg);
    
    /**
     * @brief 处理一局结束消息
     * @param pMsg 一局结束消息指针
     */
    void OnRoundEndTimer(GAME_UPGRADE_ROUNDEND *pMsg);
    
    /**
     * @brief 甩牌失败定时器处理
     * @param pMsg 甩牌失败消息指针
     */
    void OnThrowFailedTimer(GAME_UPGRADE_THROWFAILED *pMsg);
    
    /**
     * @brief 甩牌失败定时器2处理
     * @param pMsg 甩牌失败消息指针
     */
    void OnThrowFailedTimer2(GAME_UPGRADE_THROWFAILED *pMsg);
    
    void OnSetBottomTimer(GAME_UPGRADE_REQPUTBOTTOM *pMsg);

    /**
     * @brief 初始化游戏
     */
    void InitGame();
    
    /**
     * @brief 卡牌开始拖拽事件处理
     * @param e 事件参数
     */
    void OnCardStartDrag(IEvtArgs *e);
    
    /**
     * @brief 卡牌拖拽事件处理
     * @param e 事件参数
     */
    void OnCardDrag(IEvtArgs *e);
    
    /**
     * @brief 卡牌结束拖拽事件处理
     * @param e 事件参数
     */
    void OnCardEndDrag(IEvtArgs *e);
    
    /**
     * @brief 游戏面板点击事件处理
     */
    void OnGameBoardClick();
    
    /**
     * @brief 桌面信息事件处理
     * @param e 事件参数
     */
    void OnTableInfo(IEvtArgs *e);
    
    /**
     * @brief 按钮悬停事件处理
     * @param e 事件参数
     */
    void OnButtonHover(IEvtArgs *e);
    
    /**
     * @brief 事件映射表
     */
    EVENT_MAP_BEGIN()
        EVENT_CHECK_SENDER_ROOT(m_pGameBoard)
        EVENT_NAME_COMMAND(L"btn_test", OnBtnTest)
        EVENT_NAME_COMMAND(Sprites::btn_start, OnBtnStart)
        EVENT_NAME_COMMAND(Sprites::btn_set_bottom, OnBtnSetBottom)
        EVENT_NAME_HANDLER(Sprites::shuffle, EventGifLoopOver::EventID, OnShuffleLoopOver)
        EVENT_NAME_HANDLER(Sprites::shuffle, EventGifPlayFrame::EventID, OnShuffleFrame)
        EVENT_NAME_HANDLER(Sprites::sprite_vectory, EventGifPlayOver::EventID, OnGifVectoryOver)
        EVENT_NAME_HANDLER(Sprites::sprite_waiting,EventGifPlayFrame::EventID, OnGifWaitingFrame)
        EVENT_ID_HANDLER(Sprites::gif_highlight, EventGifPlayOver::EventID, OnGifHighlightOver)
        EVENT_ID_COMMAND_RANGE(Sprites::btn_diamond, Sprites::btn_big_joker, OnBtnReportRange)
        EVENT_ID_COMMAND(Sprites::btn_pass, OnBtnPass)
        EVENT_NAME_COMMAND(Sprites::btn_put_card, OnBtnPutCard)
        EVENT_NAME_COMMAND(Sprites::btn_prev_round, OnBtnPrevRound)
        EVENT_NAME_COMMAND(Sprites::btn_see_bottom, OnBtnSeeBottom)
        EVENT_HANDLER(EventSpriteCardDragStart::EventID, OnCardStartDrag)
        EVENT_HANDLER(EventSpriteCardDrag::EventID, OnCardDrag)
        EVENT_HANDLER(EventSpriteCardDragEnd::EventID, OnCardEndDrag)
        EVENT_NAME_COMMAND(m_pGameBoard->GetName(), OnGameBoardClick)
        EVENT_HANDLER(EVT_TABLE_INFO, OnTableInfo)
        EVENT_HANDLER(EventSwndMouseHover::EventID, OnButtonHover)
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
    BEGIN_MSG_MAP_EX(UpgradeGame)
        MSG_WM_TIMER(OnTimer)
    END_MSG_MAP()
    
private:
    /**
     * @brief 处理开始游戏消息
     * @param pData 消息数据指针
     * @param nSize 消息数据大小
     */
    void OnStartGame(const void *pData, int nSize);

    /**
     * @brief 处理游戏结束消息
     * @param pData 消息数据指针
     * @param nSize 消息数据大小
     */
    void OnGameEnd(const void *pData, int nSize);

    /**
     * @brief 游戏阶段改变处理
     * @param stage 新的游戏阶段
     */
    void OnStageChanged(STAGE stage);
    
    /**
     * @brief 发牌定时器处理函数
     */
    void OnTimerPostCard();	

    /**
     * @brief 执行发牌操作
     */
    void DoPostCard();
    
    /**
     * @brief 发牌完成处理
     */
    void OnPostCardDone();

    // 游戏消息处理函数
    /**
     * @brief 处理发牌消息
     * @param pData 消息数据指针
     * @param nSize 消息数据大小
     */
    void OnPostCard(const void *pData, int nSize);
    
    /**
     * @brief 处理设置花色消息
     * @param pData 消息数据指针
     * @param nSize 消息数据大小
     */
    void OnSetColor(const void *pData, int nSize);
    
    /**
     * @brief 处理展示底牌消息
     * @param pData 消息数据指针
     * @param nSize 消息数据大小
     */
    void OnPostBottom(const void *pData, int nSize);
    
    /**
     * @brief 处理设置底牌消息
     * @param pData 消息数据指针
     * @param nSize 消息数据大小
     */
    void OnSetBottom(const void *pData, int nSize);
    
    /**
     * @brief 处理自动选主消息
     * @param pData 消息数据指针
     * @param nSize 消息数据大小
     */
    void OnAutoColor(const void *pData, int nSize);
    
    /**
     * @brief 处理开始出牌消息
     * @param pData 消息数据指针
     * @param nSize 消息数据大小
     */
    void OnStartPutCard(const void *pData, int nSize);
    
    /**
     * @brief 处理出牌消息
     * @param pData 消息数据指针
     * @param nSize 消息数据大小
     */
    void OnPutCard(const void *pData, int nSize);
    
    /**
     * @brief 处理一轮结束消息
     * @param pData 消息数据指针
     * @param nSize 消息数据大小
     */
    void OnTurnOver(const void *pData, int nSize);
    
    /**
     * @brief 处理出牌错误消息
     * @param pData 消息数据指针
     * @param nSize 消息数据大小
     */
    void OnPutCardError(const void *pData, int nSize);
    
    /**
     * @brief 处理一局结束消息
     * @param pData 消息数据指针
     * @param nSize 消息数据大小
     */
    void OnRoundEnd(const void *pData, int nSize);
    
    /**
     * @brief 处理甩牌失败消息
     * @param pData 消息数据指针
     * @param nSize 消息数据大小
     */
    void OnThrowFailed(const void *pData, int nSize);
    
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
     * @brief 更新分数显示
     */
    void UpdateScore();
    
    /**
     * @brief 更新等级显示
     */
    void UpdateLevel();
    
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
    
    /**
     * @brief 播放过程语音
     * @param pszSound 语音名称
     * @param nSex 性别标识
     */
    void PlayProcessSound(LPCWSTR pszSound,int nSex);
    
    /**
     * @brief 播放等待动画
     * @param pszTip 提示文本
     */
    void PlayWattingAnimation(LPCTSTR pszTip); 
    
    /**
     * @brief 停止等待动画
     */
    void StopWatingAnimation(); 
    
    /**
     * @brief 获取座位标记
     * @param nSeat 座位索引
     * @param flagId 标记ID
     * @return 标记窗口指针
     */
    SImageWnd * GetSeatFlag(int nSeat,int flagId);
    
    /**
     * @brief 超时处理
     */
    void OnTimeout();
    
    /**
     * @brief 移动时钟
     * @param nSeat 座位索引
     * @param value 值
     */
    void MoveClock(int nSeat, int value);
    
    /**
     * @brief 隐藏时钟
     */
    void HideClock();
    
    int GetScale() const;
private:
    void onBottomCardsExtended();
private:
    CMainDlg* m_pMainDlg;               ///< 主窗口指针
    SWindow * m_pGameBoard;             ///< 游戏面板
    STAGE m_stage;                      ///< 当前游戏阶段
    SAutoRefPtr<SGameTheme> m_pTheme;   ///< 游戏主题
    int  m_nCardInStack;                ///< 牌堆剩余牌数

    //player is index recveived from server, same as index
    //seat is game view index. bottom is 0, left is 1, right is 2, top is 3
    int m_iSelfIndex;                   ///< 自己的座位号
    int m_iBankerIndex;                 ///< 庄家座位号
    int m_nLevelA;                      ///< 0,2位当前的级别
    int m_nLevelB;                      ///< 1,3位当前的级别
    GAME_UPGRADE_SHOWMAIN m_curShowMain;///< 当前亮主信息
    BOOL m_bAutoColor;                  ///< 自动抽主标志
    int m_iSetBottomIndex;              ///< 当前正在垫底牌的人
    int m_nMainColor;                   ///< 主花色,在结束垫底牌前该值为-1
    int m_iCurPutCardIndex;             ///< 当前出牌人
    int m_iTurnFirstIndex;              ///< 本轮首先出牌方
    int m_nPutCardCount;                ///< 当前出牌张数
    int m_nCardBottom[8];               ///< 自己垫的8张底牌数据
    int m_iBottomOwnerIndex;            ///< 底牌拥有者
    BOOL m_bBottomColor;                ///< 抽底牌确定花色标志
    BOOL m_bShowBottom;                 ///< 显示底牌标志
    BOOL m_bShowPrevRound;              ///< 显示上一轮出牌标志
    int m_nScore;                       ///< 闲家得分
    int m_nDiscount;                    ///< 闲家折扣

    BOOL m_bReady[4];                   ///< 玩家准备状态
    enum RoundResult{                   ///< 本局结果，用于显示动画，不参与结算
        RESULT_UNKNOWN = 0,             ///< 未知结果
        RESULT_WIN = 1,                 ///< 获胜
        RESULT_LOSE = 2,                ///< 失败
    };
    RoundResult m_roundResult;          ///< 回合结果

    CCardRecieve    m_cardRecieve;      ///< 接牌处理器
    CCardInHand     m_cardInHand[4];    ///< 各座位手牌
    CPutCard        m_putCard;          ///< 出牌管理器
    // 网络通信
    WebSocketClient *m_webSocketClient;                         ///< WebSocket客户端
    std::shared_ptr<GS_USERINFO> m_pUserInfo[PLAYER_COUNT];     ///< 用户信息
    std::shared_ptr<std::vector<BYTE> > m_userAvatar[PLAYER_COUNT]; ///< 用户头像数据
    std::shared_ptr<GAME_UPGRADE_THROWFAILED> m_throwFailMsg;   ///< 甩牌失败消息
    std::shared_ptr<GAME_UPGRADE_ROUNDEND> m_roundEndMsg;       ///< 回合结束消息
    std::shared_ptr<GAME_UPGRADE_TURNOVER> m_turnOverMsg;       ///< 一轮结束消息
    std::shared_ptr<GAME_UPGRADE_REQPUTBOTTOM> m_reqPutBottomMsg; ///< 请求垫底消息
    DWORD m_dwProps[MAX_PROP];          ///< 游戏属性
    BOOL m_bSound;                      ///< 音效开关

    SList<SAutoRefPtr<IAnimatorGroup> > m_animatorGroupList;
};

#endif // UPGRADEGAME_H