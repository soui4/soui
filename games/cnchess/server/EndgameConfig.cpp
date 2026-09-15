#include "stdafx.h"
#include "EndgameConfig.h"
#include "Chessman.h"
#include <json/json.h>
#include <sstream>
#include <helper/slog.h>
#include <string/strcpcvt.h>
#define kLogTag "EndgameConfig"


static size_t UTF8CharLength(unsigned char ch)
{
    if (ch < 0x80)
    {
        return 1;
    }
    else if (ch < 0x80 + 0x40 + 0x20)
    {
        return 2;
    }
    else if (ch < 0x80 + 0x40 + 0x20 + 0x10)
    {
        return 3;
    }
    else
    {
        return 4;
    }
}

typedef std::map<std::string, int> CHESS_NAME_MAP;

static bool EndgameTranslateRow(const std::string & s, int out[9], const CHESS_NAME_MAP & chessMap)
{
    int idx = 0;
    const uint8_t* p = (const uint8_t*)s.c_str();
    while (*p && idx < 9)
    {
        int n = UTF8CharLength(*p);
        std::string name((const char*)p, n);
        p += n;
        CHESS_NAME_MAP::const_iterator it = chessMap.find(name);
        if (it == chessMap.cend())
            out[idx++] = CHSMAN_NULL;
        else
            out[idx++] = it->second;
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

void read_all(FILE* fp,std::stringstream & ss) {
    char buf[8192];
    size_t n;
    uint8_t bom[3];
    fread(bom, 1, 3, fp);
    if (!(bom[0] == 0xef && bom[1] == 0xbb && bom[2] == 0xbf))
        ss << std::string((char*)bom, 3);
    while ((n = fread(buf, 1, sizeof buf, fp)) > 0) {
        std::string str(buf, n);
        ss << str;
    }
}

bool EndgameConfig::Load(LPCTSTR pszJsonPath)
{
    Clear();
    if (!pszJsonPath)
    {
        SLOGW() << "endgame json path is null";
        return false;
    }
    Json::Value root;
    Json::CharReaderBuilder builder;
    FILE* f = _tfopen(pszJsonPath, _T("rb"));
    if (!f) {
        SLOGW() << "can not open endgame json: " << pszJsonPath;
        fclose(f);
        return false;
    }
    std::stringstream ss;
    read_all(f, ss);
    fclose(f);
    std::string errs;
    if (!Json::parseFromStream(builder, ss, &root, &errs))
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
    if (m_nTablesPerEndgame < 2)
        m_nTablesPerEndgame = 2;
    std::string strRed = root.get("red", "").asString();
    std::string strBlack = root.get("black", "").asString();
    CHESS_NAME_MAP chessmap;
    const uint8_t* pRed = (const uint8_t*)strRed.c_str();
    const uint8_t* pBlack = (const uint8_t*)strBlack.c_str();
    for (int i = 0; i < 7; i++) {
        int n1 = UTF8CharLength(*pRed);
        chessmap.insert(std::make_pair(std::string((const char*)pRed, n1), CHSMAN_RED_JIANG+i));
        pRed += n1;
        int n2 = UTF8CharLength(*pBlack);
        chessmap.insert(std::make_pair(std::string((const char*)pBlack, n1), CHSMAN_BLK_JIANG + i));
        pBlack += n2;
        if (pRed[0] == 0 || pBlack[0] == 0)
            break;
    }
    if (chessmap.size() != 14)
        return false;
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
                if (!EndgameTranslateRow(s, row, chessmap))
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