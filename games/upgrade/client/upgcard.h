#ifndef __UPGCARD_H__
#define __UPGCARD_H__

#define ONESETCOUNT			54			//定义一副牌的张数



/**
 * 升级游戏牌操作类
 * 提供牌的比较、牌型判断、花色计算等核心功能
 */
class CUpgCard
{
public:
	/**
	 * 获取指定牌型的牌
	 * @param[in] pCard 牌数组
	 * @param[in] nCards 牌数量
	 * @param[in] nType 牌型(对子、拖拉机等)
	 * @param[out] pRet 返回牌索引数组
	 * @param[in] nMainColor 主花色
	 * @param[in] nLevelCard 级牌
	 * @param[in] b2ConstMain 是否常主
	 * @return 符合牌型的牌数量
	 */
	static int ClientGetTypeCard(const int *pCard,int nCards, int nType,int *pRet,int nMainColor,int nLevelCard,BOOL b2ConstMain);
	
	/**
	 * 获取指定牌型的数量
	 * @param[in] pCard 牌数组
	 * @param[in] nCards 牌数量
	 * @param[in] nType 牌型(对子、拖拉机等)
	 * @param[in] nMainColor 主花色
	 * @param[in] nLevelCard 级牌
	 * @param[in] b2ConstMain 是否常主
	 * @return 指定牌型的数量
	 */
	static int ClientGetCardType(const int *pCard,int nCards,int nType,int nMainColor,int nLevelCard,int b2ConstMain);
	
	/**
	 * 判断两张牌是否相邻
	 * @param[in] nCard1 第一张牌
	 * @param[in] nCard2 第二张牌
	 * @param[in] nMainColor 主花色
	 * @param[in] nLevelCard 级牌
	 * @param[in] b2ConstMain 是否常主
	 * @return 是否相邻
	 */
	static BOOL ClientIsAdjcent(int nCard1,int nCard2,int nMainColor,int nLevelCard,BOOL b2ConstMain);
	
	/**
	 * 获取主牌的值
	 * @param[in] nCard 牌值
	 * @param[in] nMainColor 主花色
	 * @param[in] nLevelCard 级牌
	 * @param[in] b2ConstMain 是否常主
	 * @return 主牌值
	 */
	static int ClientGetMainValue(int nCard,int nMainColor,int nLevelCard,BOOL b2ConstMain);
	
	/**
	 * 计算牌的花色(正主、副主是一个花色)
	 * @param[in] nCard 牌值
	 * @param[in] nMainColor 主花色
	 * @param[in] nLevelCard 级牌
	 * @param[in] b2ConstMain 是否常主
	 * @return 牌的花色
	 */
	static int ClientGetColor2(int nCard,int nMainColor,int nLevelCard,BOOL b2ConstMain);
	
	/**
	 * 获取牌的花色
	 * @param[in] nCard 牌值
	 * @return 牌的花色
	 */
	static int ClientGetColor(int nCard);
	
	/**
	 * 根据花色和值构造牌
	 * @param[in] nColor 花色
	 * @param[in] nValue 牌值
	 * @return 牌值
	 */
	static int ClientGetCard(int nColor,int nValue);
	
	/**
	 * 获取牌值
	 * @param[in] nCard 牌值
	 * @return 牌值
	 */
	static int ClientGetValue(int nCard);
	
	/**
	 * 比较两张牌的大小
	 * @param[in] nCard1 第一张牌
	 * @param[in] nCard2 第二张牌
	 * @param[in] nLevel 级别
	 * @param[in] nMainColor 主花色
	 * @param[in] b2ConstMain 2是否常主
	 * @return 比较结果(0-相等,负数-第一张小,正数-第一张大)
	 */
	static int ClientCompare(int nCard1, int nCard2,int nLevel,int nMainColor,BOOL b2ConstMain);

};
#endif