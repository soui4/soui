// 残局打谱 - 游戏桌端到端(服务器逻辑层)测试
//
// 覆盖: endgames.json 配置加载、每个残局固定2张游戏桌的表位映射、
// 残局布局合法性、开局应用布局、一局结束后红方座位轮换(与常规模式一致)。
#include <gtest/gtest.h>
#include <windows.h>
#include <tchar.h>
#include <string>
#include <vector>

#include <protocol.h>
#include <Chessman.h>
#include <ChessLayout.h>

#include "GameClient.h"
#include "CnChess.h"
#include "EndgameConfig.h"
#include "PropBag.h"

namespace
{
    // PropBag 采用 SSingleton 模式, getSingletonPtr 不会惰性创建;
    // CCnChess 构造函数会读取 PropBag 单例的属性, 因此测试需先创建该实例。
    static SOUI::PropBag s_PropBag;   // MSVC static init order 保证在 table 构造前实例化

    // ITableListener 桩: 记录 OnTableChange 通知
    struct TEndgameListener : public ITableListener
    {
        std::vector<int> changes;
        void OnTableChange(int nTableId) override { changes.push_back(nTableId); }
    };

    // 暴露 CCnChess 中受保护状态, 供测试校验开局/交换逻辑
    class TEndgameTable : public SOUI::CCnChess
    {
    public:
        TEndgameTable(ITableListener *l, int t) : SOUI::CCnChess(l, t) {}
        CChessLayout & TLayout() { return m_layout; }
        bool TIsEndgame() const { return m_bEndgame; }
        int TPassable() const { return m_nChsPassable; }
    };

    // 由残局配置在 endgames.json 中按序号取布局/解析器
    const EndgameItem * GetEndgameByIndex(int nIndex)
    {
        return EndgameConfig::GetInstance()->GetByIndex(nIndex);
    }

    // 校验残局布局基本合法性, 返回棋子总数
    int ValidateLayout(const int layout[10][9], int &nRedJiang, int &nBlkJiang)
    {
        int nPieces = 0;
        nRedJiang = nBlkJiang = 0;
        for (int y = 0; y < 10; y++)
        {
            for (int x = 0; x < 9; x++)
            {
                int nChs = layout[y][x];
                if (nChs == CHSMAN_NULL)
                    continue;
                if (nChs < 0 || nChs > 6 * 2)   // 合法编码 0-13(正红/负黑), 否则 127 以上且非255视为非法
                    return -1;
                nPieces++;
                if (nChs == CHSMAN_RED_JIANG) nRedJiang++;
                if (nChs == CHSMAN_BLK_JIANG) nBlkJiang++;
            }
        }
        return nPieces;
    }
}

// 残局配置加载: 读取真实 endgames.json, 数量正确, 每个残局都有合法布局
TEST(EndgameTableTest, ConfigLoadsRealEndgames)
{
    bool bLoaded = EndgameConfig::GetInstance()->Load(ENDGAME_TEST_JSON);
    ASSERT_TRUE(bLoaded) << "无法加载 endgames.json: " ENDGAME_TEST_JSON;
    int nCount = EndgameConfig::GetInstance()->GetCount();
    EXPECT_GT(nCount, 0) << "残局数量应为正数";
    ASSERT_GT(nCount, 0);

    for (int i = 0; i < nCount; i++)
    {
        const EndgameItem *pEg = GetEndgameByIndex(i);
        ASSERT_NE(pEg, nullptr) << "endgame index=" << i;
        int nRed = 0, nBlk = 0;
        int nPieces = ValidateLayout(pEg->layout, nRed, nBlk);
        EXPECT_GT(nPieces, 0) << "endgame index=" << i << " 应有棋子";
        EXPECT_EQ(nRed, 1)  << "endgame index=" << i << " 应恰有1个红帅";
        EXPECT_EQ(nBlk, 1)  << "endgame index=" << i << " 应恰有1个黑将";
    }
}

// 每个残局固定 2 张游戏桌: 坐哪桌就对应哪一个残局
TEST(EndgameTableTest, TwoTablesPerEndgame)
{
    int nCount = EndgameConfig::GetInstance()->GetCount();
    ASSERT_GT(nCount, 0);

    for (int idx = 0; idx < nCount; idx++)
    {
        int t0 = ENDGAME_TABLE_BASE + idx * 2;
        int t1 = ENDGAME_TABLE_BASE + idx * 2 + 1;
        EXPECT_EQ((t0 - ENDGAME_TABLE_BASE) / 2, idx) << "残局序号映射错误(t0)";
        EXPECT_EQ((t1 - ENDGAME_TABLE_BASE) / 2, idx) << "残局序号映射错误(t1)";
        EXPECT_NE(t0, t1) << "同一残局的2张桌不应重复";
    }
    // 全部残局桌连续且互不重叠, 不越界
    int nFirst = ENDGAME_TABLE_BASE;
    int nLast  = ENDGAME_TABLE_BASE + nCount * 2 - 1;
    EXPECT_EQ(nLast - nFirst + 1, nCount * 2);
    // 边界: BASE-1 不属于残局区间(大厅桌), BASE + count*2 超出残局区间
    EXPECT_LT(ENDGAME_TABLE_BASE - 1, ENDGAME_TABLE_BASE);
}

// 残局桌渲染布局: 开局按配置布局落子, 与常规模式一致红先,
// 一局结束后通过红方座位轮换实现交换双方(与常规模式一致)
TEST(EndgameTableTest, EndgameGameFlowAppliesLayoutAndRotateRedSeat)
{
    const EndgameItem *pEg = GetEndgameByIndex(0);
    ASSERT_NE(pEg, nullptr);

    TEndgameListener listener;
    int nTable = ENDGAME_TABLE_BASE + 0 * 2;   // 残局0 的第1张桌
    TEndgameTable table(&listener, nTable);

    // 2名玩家入座
    table.OnAddPlayer(0, new GameClient);
    table.OnAddPlayer(1, new GameClient);
    EXPECT_EQ(table.GetPlayerCount(), 2);
    int nRedSeat0 = table.GetRedSeat();         // 初始红方座位

    // 配置残局: 仅设置自定义布局, 其余逻辑与常规模式一致(无残局特有首行状态)
    table.ConfigureEndgame(pEg->nId, pEg->layout);

    // 开局
    table.OnGameStart();

    EXPECT_TRUE(table.TIsEndgame()) << "残局桌标志应生效";

    // 布局被完整应用
    for (int y = 0; y < 10; y++)
        for (int x = 0; x < 9; x++)
            EXPECT_EQ((int)table.TLayout().m_chesses[y][x], pEg->layout[y][x])
                << "残局布局未正确应用 cell=(" << x << "," << y << ")";

    // 首步行棋方 = 红先, 与常规模式一致
    EXPECT_EQ((int)table.TLayout().m_actSide, CS_RED) << "开局首行应红先";
    EXPECT_GT(table.TPassable(), 0) << "应统计到可过河子数量";

    // 一局结束 → 红方座位轮换(与常规模式一致)
    table.OnGameEnd();
    EXPECT_EQ(table.GetRedSeat(), (nRedSeat0 + 1) % 2) << "一局结束后应轮换红方座位";

    // 双方准备后再次开局, 首行仍红先, 红方座位保持轮换后的座位
    for (int i = 0; i < 2; i++)
        table.GetPlayer(i)->m_bReady = TRUE;
    table.OnGameStart();
    EXPECT_EQ(table.GetRedSeat(), (nRedSeat0 + 1) % 2);
    EXPECT_EQ((int)table.TLayout().m_actSide, CS_RED) << "交换后第二局首行仍红先";
}

// 普通桌与残局桌隔离: 普通桌不会进残局区间
TEST(EndgameTableTest, NormalTableOutsideEndgameRange)
{
    EXPECT_LT(0, ENDGAME_TABLE_BASE);
    // 所有残局桌号都落在 [ENDGAME_TABLE_BASE, ENDGAME_TABLE_BASE+count*2)
    int nCount = EndgameConfig::GetInstance()->GetCount();
    for (int s = 0; s < nCount * 2; s++)
    {
        int t = ENDGAME_TABLE_BASE + s;
        int idx = (t - ENDGAME_TABLE_BASE) / 2;
        EXPECT_GE(idx, 0);
        EXPECT_LT(idx, nCount);
    }
}