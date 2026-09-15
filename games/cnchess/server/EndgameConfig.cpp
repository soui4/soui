#include "stdafx.h"
#include "EndgameConfig.h"
#include "Chessman.h"
#include <json/json.h>
#include <fstream>
#include <helper/slog.h>
#include <string/strcpcvt.h>
#define kLogTag "EndgameConfig"

// 汉字棋谱布局: 把一行9个汉字(UTF-8)翻译为CHESSMAN数字编码.
// 红方简体: 帅车马炮仕相兵; 黑方繁体: 将車馬砲士象卒; 空位 '.'
static int EndgameCharToMan(const std::string & ch)
{
    if (ch == "\xe5\xb8\x85") return CHSMAN_RED_JIANG;  // 帅
    if (ch == "\xe8\xbd\xa6") return CHSMAN_RED_JU;     // 车
    if (ch == "\xe9\xa9\xac") return CHSMAN_RED_MA;     // 马
    if (ch == "\xe7\x82\xae") return CHSMAN_RED_PAO;    // 炮
    if (ch == "\xe4\xbb\x95") return CHSMAN_RED_SHI;    // 仕
    if (ch == "\xe7\x9b\xb8") return CHSMAN_RED_XIANG;  // 相
    if (ch == "\xe5\x85\xb5") return CHSMAN_RED_BING;   // 兵
    if (ch == "\xe5\xb0\x86") return CHSMAN_BLK_JIANG;  // 将
    if (ch == "\xe8\xbb\x8a") return CHSMAN_BLK_JU;     // 車
    if (ch == "\xe9\xa6\xac") return CHSMAN_BLK_MA;     // 馬
    if (ch == "\xe7\xa0\xb2") return CHSMAN_BLK_PAO;    // 砲
    if (ch == "\xe5\xa3\xab") return CHSMAN_BLK_SHI;    // 士
    if (ch == "\xe8\xb1\xa1") return CHSMAN_BLK_XIANG;  // 象
    if (ch == "\xe5\x8d\x92") return CHSMAN_BLK_BING;   // 卒
    return CHSMAN_NULL; // '.' 或不认识视为空
}

static bool EndgameTranslateRow(const std::string & s, int out[9])
{
    size_t len = s.size();
    size_t i = 0;
    int idx = 0;
    while (i < len && idx < 9)
    {
        unsigned char c = (unsigned char)s[i];
        if (c == '.')
        {
            out[idx++] = CHSMAN_NULL;
            i += 1;
        }
        else if (c >= 0x80)
        {
            if (i + 3 > len)
                break;
            int man = EndgameCharToMan(s.substr(i, 3));
            if (man == CHSMAN_NULL && c != '.')
                return false; // 无法识别的汉字
            out[idx++] = man;
            i += 3;
        }
        else
        {
            return false; // 意外ASCII字符
        }
    }
    while (idx < 9)
        out[idx++] = CHSMAN_NULL;
    return true;
}

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
            int row[9];
            if (layout[y].isString())
            {
                // 汉字布局: 每行一个9字字符串, 翻译为数字编码
                std::string s = layout[y].asString();
                if (!EndgameTranslateRow(s, row))
                    bValid = false;
            }
            else if (layout[y].isArray() && layout[y].size() == 9)
            {
                // 兼容数字数组形式
                for (int x = 0; x < 9; x++)
                    row[x] = layout[y][x].asInt();
            }
            else
            {
                bValid = false;
                break;
            }
            for (int x = 0; x < 9; x++)
                eg.layout[y][x] = row[x];
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