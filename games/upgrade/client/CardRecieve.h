// CardRecieve.h: interface for the CCardRecieve class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_CARDRECIEVE_H__AE38D624_8C05_4697_A877_A200A05CE26F__INCLUDED_)
#define AFX_CARDRECIEVE_H__AE38D624_8C05_4697_A877_A200A05CE26F__INCLUDED_

/**
 * @brief 发牌管理类
 * 
 * 该类负责管理发牌过程，包括存储牌数据、确定发牌顺序和获取下一张牌等功能
 */
class CCardRecieve  
{
public:
	/**
	 * @brief 构造函数
	 */
	CCardRecieve();
	
	/**
	 * @brief 析构函数
	 */
	virtual ~CCardRecieve();

public:
	/**
	 * @brief 获取下一张牌
	 * @param[out] nCard 返回的牌值，如果发给自己则返回真实牌值，否则返回-1
	 * @param[out] nToPlayer 返回接收牌的玩家座位号
	 * @return 是否还有牌可以发
	 * @remark 当牌已发完且回到第一个玩家时返回FALSE
	 */
	BOOL FatchCard(int &nCard,int &nToPlayer);
	
	/**
	 * @brief 初始化发牌数据
	 * @param pnCards 牌数据数组
	 * @param nSelfIndex 自己的索引
	 * @param nFirstIndex 第一个发牌的玩家索引
	 */
	void Init(const int *pnCards,int nSelfIndex,int nFirstIndex);
	
	/**
	 * @brief 清空发牌数据
	 */
	void Clear();
	
	/**
	 * @brief 获取剩余牌数
	 * @return 剩余牌数
	 */
	int GetCount() const {return m_nCount;}
	
private:
	int		m_nCards[25];      ///< 牌数据数组，最多25张牌
	int		m_nCount;          ///< 剩余牌数
	int		m_nToSeat;	       ///< 当前发到的玩家号（座位号）
	int		m_nFirstIndex;     ///< 第一个发牌的玩家索引
	int		m_nSelfIndex;      ///< 自己的索引
};

#endif // !defined(AFX_CARDRECIEVE_H__AE38D624_8C05_4697_A877_A200A05CE26F__INCLUDED_)