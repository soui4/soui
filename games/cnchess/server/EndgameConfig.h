#ifndef __ENDGAMECONFIG_H__
#define __ENDGAMECONFIG_H__

#include <vector>
#include <string>

// 单个残局配置
struct EndgameItem
{
    int nId;                    // 残局ID
    int nLevel;                 // 难度 1初级/2中级/3高级
    int nPlayer;                // 首步行棋方: 0=红先 1=黑先
    char szTitle[64];           // 标题 utf8
    char szIntro[256];          // 说明 utf8
    int layout[10][9];          // 布局, 编码同CHESSMAN: 255空/正红(0-6)/负黑(7-13)
};

// 残局配置管理器(服务器加载 config/endgames.json)
class EndgameConfig
{
public:
    static EndgameConfig * GetInstance();
    // 从 json 文件加载残局数据
    bool Load(LPCTSTR pszJsonPath);
    // 获得残局数量
    int GetCount() const { return (int)m_items.size(); }
    // 每个残局开设的游戏桌数(配置项), 配置缺省时为 2
    int GetTablesPerEndgame() const { return m_nTablesPerEndgame; }
    // 按下标获取残局条目
    const EndgameItem * GetByIndex(int nIndex) const;
    // 按ID查找残局条目, 不存在返回NULL
    const EndgameItem * FindById(int nId) const;
private:
    EndgameConfig() {}
    ~EndgameConfig() {}
    void Clear();
    std::vector<EndgameItem> m_items;
    int m_nTablesPerEndgame = 2;   // 每个残局的游戏桌数
};

#endif //__ENDGAMECONFIG_H__