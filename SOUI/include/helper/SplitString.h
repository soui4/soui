#ifndef __SPLITSTRING__H__
#define __SPLITSTRING__H__

SNSBEGIN

/**
 * @brief 拆分字符串模板函数
 * @tparam T 字符串类型（例如，SStringA, SStringW）
 * @tparam TC 分隔符类型（例如，char, wchar_t）
 * @param str 要拆分的字符串
 * @param cSep 分隔符
 * @param strLst 存储拆分结果的数组
 * @return 拆分后的字符串数量
 */
template <class T, class TC>
size_t SplitString(const T &str, TC cSep, SArray<T> &strLst)
{
    int nBegin = 0;
    int nEnd = 0;
    while (nEnd != str.GetLength())
    {
        if (str[nEnd] == cSep)
        {
            if (nEnd > nBegin)
            {
                strLst.Add(str.Mid(nBegin, nEnd - nBegin));
            }
            nBegin = nEnd + 1;
        }
        nEnd++;
    }
    if (nEnd > nBegin)
    {
        strLst.Add(str.Mid(nBegin, nEnd - nBegin));
    }
    return strLst.GetCount();
}

/**
 * @typedef SStringAList
 * @brief SStringA 类型的数组
 */
typedef SArray<SStringA> SStringAList;

/**
 * @typedef SStringWList
 * @brief SStringW 类型的数组
 */
typedef SArray<SStringW> SStringWList;

// 显式模板实例化
template size_t SplitString<SStringA, char>(const SStringA &str, char cSep, SStringAList &strLst);
template size_t SplitString<SStringW, wchar_t>(const SStringW &str, wchar_t cSep, SStringWList &strLst);

#ifdef _UNICODE
/**
 * @typedef SStringTList
 * @brief 根据编译环境定义的字符串数组类型（Unicode环境为SStringWList，非Unicode环境为SStringAList）
 */
#define SStringTList SStringWList
#else
#define SStringTList SStringAList
#endif // _UNICODE

/**
 * @brief 解析资源ID
 * @param str 包含资源ID的字符串
 * @param strLst 存储解析结果的数组
 * @return 解析后的字符串数量
 */
inline int ParseResID(const SStringT &str, SStringTList &strLst)
{
    int nPos = str.FindChar(_T(':'));
    if (nPos == -1)
    {
        strLst.Add(str);
    }
    else
    {
        strLst.Add(str.Left(nPos));
        strLst.Add(str.Right(str.GetLength() - nPos - 1));
    }
    return (int)strLst.GetCount();
}

SNSEND
#endif // __SPLITSTRING__H__