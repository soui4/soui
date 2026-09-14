#include "stdafx.h"
#include "EndgameConfig.h"
#include <json/json.h>
#include <fstream>
#include <helper/slog.h>
#include <string/strcpcvt.h>
#define kLogTag "EndgameConfig"

EndgameConfig * EndgameConfig::GetInstance()
{
    static EndgameConfig instance;
    return &instance;
}

void EndgameConfig::Clear()
{
    m_items.clear();
    m_nTablesPerEndgame = 2;
}

const EndgameItem * EndgameConfig::GetByIndex(int nIndex) const
{
    if (nIndex < 0 || nIndex >= (int)m_items.size())
        return NULL;
    return &m_items[nIndex];
}

const EndgameItem * EndgameConfig::FindById(int nId) const
{
    for (size_t i = 0; i < m_items.size(); i++)
    {
        if (m_items[i].nId == nId)
            return &m_items[i];
    }
    return NULL;
}

bool EndgameConfig::Load(const char * pszJsonPath)
{
    Clear();
    if (!pszJsonPath)
    {
        SLOGW() << "endgame json path is null";
        return false;
    }
    Json::Value root;
    Json::CharReaderBuilder builder;
    std::ifstream ifs(pszJsonPath, std::ios::binary);
    if (!ifs.is_open())
    {
        SLOGW() << "can not open endgame json: " << pszJsonPath;
        return false;
    }
    std::string errs;
    if (!Json::parseFromStream(builder, ifs, &root, &errs))
    {
        SLOGW() << "parse endgame json error: " << errs.c_str();
        return false;
    }
    Json::Value & arr = root["endgames"];
    if (!arr.isArray())
    {
        SLOGW() << "endgame json has no endgames array";
        return false;
    }
    // 每残局桌数配置项(缺省 2), 供残局桌区间/映射使用
    m_nTablesPerEndgame = root.get("tablesPerEndgame", 2).asInt();
    if (m_nTablesPerEndgame < 1)
        m_nTablesPerEndgame = 2;
    for (Json::Value::ArrayIndex i = 0; i < arr.size(); i++)
    {
        Json::Value & item = arr[i];
        EndgameItem eg;
        memset(&eg, 0, sizeof(eg));
        eg.nId = item.get("id", 0).asInt();
        eg.nLevel = item.get("difficulty", 1).asInt();
        eg.nPlayer = item.get("player", 0).asInt();

        std::string strTitle = item.get("title", "").asString();
        strncpy(eg.szTitle, strTitle.c_str(), sizeof(eg.szTitle) - 1);
        std::string strIntro = item.get("intro", "").asString();
        strncpy(eg.szIntro, strIntro.c_str(), sizeof(eg.szIntro) - 1);

        Json::Value & layout = item["layout"];
        if (!layout.isArray() || layout.size() != 10)
        {
            SLOGW() << "endgame id=" << eg.nId << " invalid layout rows";
            continue;
        }
        bool bValid = true;
        for (int y = 0; y < 10 && bValid; y++)
        {
            if (!layout[y].isArray() || layout[y].size() != 9)
            {
                bValid = false;
                break;
            }
            for (int x = 0; x < 9; x++)
                eg.layout[y][x] = layout[y][x].asInt();
        }
        if (!bValid)
        {
            SLOGW() << "endgame id=" << eg.nId << " invalid layout";
            continue;
        }
        m_items.push_back(eg);
    }
    SLOGI() << "endgame config loaded, count=" << m_items.size();
    return !m_items.empty();
}