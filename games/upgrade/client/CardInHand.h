// CCardInHand.h: interface for the CCardInHand class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_CARDINHAND_H__20CA1CE8_4873_4538_97FA_0A2583F8B542__INCLUDED_)
#define AFX_CARDINHAND_H__20CA1CE8_4873_4538_97FA_0A2583F8B542__INCLUDED_

#include <layout/SAnchorLayout.h>
#include "SSpriteCard.h"
#include "upgcard.h"

class CCardInHand;
class UpgradeGame;
// 管理一轮牌的数据
class CPutCard : public IAnimatorListener {
  public:
    STDMETHOD_(void, onAnimationStart)(THIS_ IValueAnimator *pAnimator) override
    {
    }
    STDMETHOD_(void, onAnimationEnd)(THIS_ IValueAnimator *pAnimator) override;
    STDMETHOD_(void, onAnimationRepeat)(THIS_ IValueAnimator *pAnimator) override
    {
    }

  public:
    /**
     * @brief 构造函数
     * @param pGame 指向游戏主类的指针
     */
    CPutCard(UpgradeGame *pGame);
    
    /**
     * @brief 析构函数
     */
    virtual ~CPutCard();

    /**
     * @brief 设置得分牌位置
     * @param posScore 得分牌位置锚点
     */
    void SetScoreCardPos(AnchorPos posScore);

    /**
     * @brief 设置出牌区域基准位置
     * @param posBottom 底部位置锚点
     * @param posLeft 左侧位置锚点
     * @param posTop 顶部位置锚点
     * @param posRight 右侧位置锚点
     */
    void SetBase(AnchorPos posBottom, AnchorPos posLeft, AnchorPos posTop, AnchorPos posRight);

    /**
     * @brief 设置头像位置
     * @param posBottom 底部位置锚点
     * @param posLeft 左侧位置锚点
     * @param posTop 顶部位置锚点
     * @param posRight 右侧位置锚点
     */
    void SetAvatarPos(AnchorPos posBottom, AnchorPos posLeft, AnchorPos posTop, AnchorPos posRight);

    /**
     * @brief 设置玩家出的牌
     * @param[in] iSeat 玩家座位号
	 * @param[in] lstCard 牌对象列表
     */
    void SetPutCard(int iSeat, SList<SAutoRefPtr<SSpriteCard> > &lstCard);

    /**
     * @brief 获取指定玩家的出牌
     * @param[in] iSeat 玩家座位号
     * @param[out] lstCard 返回牌值数组
     * @return 牌数量
     */
    int GetCard(int iSeat, SList<SAutoRefPtr<SSpriteCard> > &lstCard);

    /**
     * @brief 清除本轮出牌数据
     */
    void Clear();

    /**
     * @brief 收牌处理
     * @param[in] nPlayer 玩家索引
     * @return 得分数
     */
    int TakeinCard(int nWinner);

    /**
     * @brief 显示上一轮的牌
     * @param[in] bShow 是否显示
     */
    void ShowPrevRound(BOOL bShow);

    /**
     * @brief 清除得分牌
     */
    void ClearScoreCards();

    /**
     * @brief 获取首张牌
     * @param nSeat 座位索引
     * @return 首张牌的牌值
     */
    int GetFirstCard(int nSeat);

    /**
     * @brief 获取指定座位的所有牌
     * @param nSeat 座位索引
     * @param cards 牌值数组
     * @return 牌的数量
     */
    int GetCards(int nSeat, int *cards);

  private:
    int m_nFirstPlayer;   ///< 第一个出牌人的坐位
    int m_nFirstColor;    ///< 首家牌花色
    int m_nFirstCardType; ///< 首家牌型

    SList<SAutoRefPtr<SSpriteCard> > m_lstCard[PLAYER_COUNT]; ///< 各玩家当前出牌列表
    SList<SAutoRefPtr<SSpriteCard> > m_lstScore;              ///< 得分牌列表
    SList<SAutoRefPtr<SSpriteCard> > m_lstPrevCard[PLAYER_COUNT]; ///< 上一轮出牌列表
    BOOL m_bShowPrevRound;                                    ///< 是否显示上一轮出牌

    AnchorPos m_posPutBase[PLAYER_COUNT]; ///< 出牌基准位置
    AnchorPos m_posScore;                 ///< 得分牌位置
    AnchorPos m_posAvatar[PLAYER_COUNT];  ///< 头像位置
	  CSize m_szCard;                       ///< 牌尺寸

    UpgradeGame *m_pGame;                 ///< 游戏主类指针
};

/**
 * @brief 手牌管理类
 * 
 * 该类负责管理玩家手中的牌，包括牌的显示、选择、出牌等操作
 */
class CCardInHand : public IAnimatorListener{
  public:
    /**
     * @brief 卡牌相关常量定义
     */
    enum
    {
        kCardInterval_Horz = 20,  ///< 水平方向卡牌间隔
        kCardInterval_Vert = 4,   ///< 垂直方向卡牌间隔
        kCardLayer_Base = 300,    ///< 卡牌基础层级
    };

  public:
    /**
     * @brief 构造函数
     */
    CCardInHand()
    {
        ISkinObj *pCard = GETSKIN(L"skin_card", 100);
        m_szCard = pCard->GetSkinSize();
		m_mainColor = -1; // 主花色，-1表示没有主花色，0-3表示主花色
    };
    
    /**
     * @brief 析构函数
     */
    ~CCardInHand() {};

    /**
     * @brief 初始化手牌管理器
     * @param pGame 游戏主类指针
     * @param bHorz 是否水平排列
     */
	void Init(UpgradeGame *pGame, BOOL bHorz){
        m_pGame = pGame;
        m_bHorz = bHorz;
	}
	
    /**
     * @brief 清除手牌数据
     */
    void ClearData()
    {
        m_lstCard.RemoveAll();
		m_mainColor = -1;
    }

    /**
     * @brief 设置基准位置
     * @param posBase 基准位置锚点
     */
    void SetBase(AnchorPos posBase)
    {
        m_posBase = posBase;
    }

    /**
     * @brief 接收一张牌到指定位置
     * @param pCard 牌对象
     */
    void ReceiveCard(SSpriteCard *pCard);

    /**
     * @brief 接收多张牌
     * @param lstCards 牌对象列表
     */
    void ReceiveCards(SList<SAutoRefPtr<SSpriteCard> > &lstCards);

    /**
     * @brief 根据矩形区域选择牌
     * @param rc 选择区域
     */
    void SelectCardForRect(const CRect &rc);

    /**
     * @brief 将选中的牌出牌
     * @param lstSelected 选中的牌
     * @return 是否成功
     */
    BOOL PutCard(SList<SAutoRefPtr<SSpriteCard> > &lstSelected);

    /**
     * @brief 将指定牌出牌
     * @param pCards 牌数组
     * @param nCount 数组大小
     * @param lstCard 返回的牌对象列表
     * @return 是否成功
     * @remark 用来给另外三方出牌接口。
     */
    BOOL PutCard(int *pCards, int nCount, SList<SAutoRefPtr<SSpriteCard> > &lstCard);

    /**
     * @brief 将指定牌出牌（另一种实现）
     * @param pCards 牌数组
     * @param nCount 数组大小
     * @param lstCard 返回的牌对象列表
     * @return 是否成功
     */
    BOOL PutCard2(int *pCards, int nCount, SList<SAutoRefPtr<SSpriteCard> > &lstCard);

    /**
     * @brief 设置是否可选择
     * @param bSelectable 是否可选择
     */
    void SetSelectable(BOOL bSelectable);

    /**
     * @brief 获取卡牌尺寸
     * @return 卡牌尺寸
     */
    SIZE GetCardSize() const;

    /**
     * @brief 测试能否报主
     * @param nColor 花色
     * @return 可报主的牌数
     */
    int TestReport(int nColor);

    /**
     * @brief 报主操作
     * @param nCard 报主的牌
     * @param bCouple 是否是对子
     * @param lstCards 返回的牌对象列表
     * @return 是否成功
     * @remark 亮主,返回亮主的牌对象,如果是对子,返回两个对象,如果是单张,返回一个对象,如果失败,返回NULL
     */
    BOOL ReportColor(int nCard, BOOL bCouple, SList<SAutoRefPtr<SSpriteCard> > &lstCards); // 亮主,返回亮主的牌对象,如果是对子,返回两个对象,如果是单张,返回一个对象,如果失败,返回NULL

    /**
     * @brief 清除选择状态
     */
    void ClearSelect();
    
    /**
     * @brief 获取选中的牌
     * @param lstCards 返回的选中牌列表
     * @remark 获取选中的牌
     */
    void GetSelectedCards(SList<SAutoRefPtr<SSpriteCard> > &lstCards); //获取选中的牌

    /**
     * @brief 设置主花色
     * @param mainColor 主花色
     * @param nAniSpeed 动画速度
     */
    void SetMainColor(int mainColor, int nAniSpeed);

    /**
     * @brief 打印卡牌ID（调试用）
     * @return 卡牌ID字符串
     */
    SStringT PrintCardID();

    /**
     * @brief 调整卡牌位置
     */
    void AdjustCardsPosition();
    
    /**
     * @brief 判断手牌是否为空
     * @return 是否为空
     */
    BOOL IsEmpty() const { return m_lstCard.IsEmpty(); } //是否手牌为空
    
    /**
     * @brief 清空手牌
     */
    void Clear();
    
    /**
     * @brief 随机选择牌
     * @param nCards 选择的牌数
     * @param bClear 是否清空之前的选择
     * @param nColor 花色
     * @param b2ConstMain 是否2为常主
     */
    void SelectCardRandom(int nCards, BOOL bClear,int nColor, BOOL b2ConstMain);
    
    /**
     * @brief 自动选择牌
     * @param pnCardFirst 首牌数组
     * @param nFirstCards 首牌数量
     * @return 选择的牌数
     */
    int AutoSelectCard(const int * pnCardFirst, int nFirstCards);
    
    /**
     * @brief 判断选中的牌是否同花色
     * @param nColor 花色
     * @param pCard 牌数组
     * @param nCards 牌数量
     * @return 是否同花色
     */
    BOOL IsSelectedCardSameColor(int nColor,int *pCard,int nCards) const;
    
    /**
     * @brief 判断某花色的牌是否全部被选中
     * @param nColor 花色
     * @return 是否全部选中
     */
    BOOL IsColorCardAllSelected(int nColor) const;
    
    /**
     * @brief 选中所有牌
     */
    void SelectAllCard();

    /**
     * @brief 获取卡牌数量
     * @return 卡牌数量
     */
    int GetCount() const { return m_lstCard.GetCount(); } //获取卡牌数量
  protected:
    /**
     * @brief 移动卡牌
     * @param lstToUp 需要向上移动的卡牌位置列表
     * @param lstToLeft 需要向左移动的卡牌位置列表
     * @param nSpeed 移动速度
     */
  	void MoveCards(SList<SPOSITION> &lstToUp, SList<SPOSITION> &lstToLeft, int nSpeed);
  	
    /**
     * @brief 动画开始回调
     * @param pAnimator 动画对象指针
     */
    STDMETHOD_(void, onAnimationStart)(THIS_ IValueAnimator * pAnimator) override {}
    
    /**
     * @brief 动画结束回调
     * @param pAnimator 动画对象指针
     */
    STDMETHOD_(void, onAnimationEnd)(THIS_ IValueAnimator *pAnimator) override;
    
    /**
     * @brief 动画重复回调
     * @param pAnimator 动画对象指针
     */
    STDMETHOD_(void, onAnimationRepeat)(THIS_ IValueAnimator *pAnimator) override{}

  protected:
    /**
     * @brief 插入卡牌
     * @param pCard 卡牌对象指针
     * @return 插入位置
     */
    int InsertCard(SSpriteCard *pCard);
    
    /**
     * @brief 获取指定花色的卡牌
     * @param nColor 花色
     * @param pPosCard 返回的卡牌位置数组
     * @return 卡牌数量
     */
    int GetColorCard(int nColor,SPOSITION pPosCard[33]);

    SList<SAutoRefPtr<SSpriteCard> > m_lstCard; ///< 手牌列表
    BOOL m_bHorz;                               ///< 是否水平排列
    CSize m_szCard;                             ///< 卡牌尺寸
    AnchorPos m_posBase;                        ///< 基准位置
    UpgradeGame *m_pGame;                       ///< 游戏主类指针
    int m_mainColor;                            ///< 主花色
};

#endif // !defined(AFX_CARDINHAND_H__20CA1CE8_4873_4538_97FA_0A2583F8B542__INCLUDED_)