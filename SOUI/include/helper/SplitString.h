#ifndef __SPLITSTRING__H__
#define __SPLITSTRING__H__

SNSBEGIN

/**
 * @brief Split string template function
 * @tparam T String type (e.g., SStringA, SStringW)
 * @tparam TC Separator type (e.g., char, wchar_t)
 * @param str String to split
 * @param cSep Separator
 * @param strLst Array storing the split results
 * @return Number of split strings
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
                T subStr = str.Mid(nBegin, nEnd - nBegin);
                subStr.Trim();
                strLst.Add(subStr);
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
 * @brief Array of SStringA type
 */
typedef SArray<SStringA> SStringAList;

/**
 * @typedef SStringWList
 * @brief Array of SStringW type
 */
typedef SArray<SStringW> SStringWList;

/** Explicit template instantiation */
template size_t SplitString<SStringA, char>(const SStringA &str, char cSep, SStringAList &strLst);
template size_t SplitString<SStringW, wchar_t>(const SStringW &str, wchar_t cSep, SStringWList &strLst);

#ifdef _UNICODE
/**
 * @typedef SStringTList
 * @brief String array type defined according to the compilation environment (SStringWList in Unicode environment, SStringAList in non-Unicode environment)
 */
#define SStringTList SStringWList
#else
#define SStringTList SStringAList
#endif /**< _UNICODE */

/**
 * @brief Parse resource ID
 * @param str String containing the resource ID
 * @param strLst Array storing the parse results
 * @return Number of parsed strings
 */
inline int ParseResID(const SStringT &str, SStringTList &strLst)
{
    int nPos = str.FindChar(_T(':'));
    if (nPos == -1 || nPos == 1)
    {
        // resid with nPos==1 will treat as an obsolute path for windows OS, so we don't split it.
        strLst.Add(str);
    }
    else if (nPos > 1)
    {
        strLst.Add(str.Left(nPos));
        strLst.Add(str.Right(str.GetLength() - nPos - 1));
    }
    return (int)strLst.GetCount();
}

inline SStringTList ParseResID(const SStringT &str)
{
    SStringTList strLst;
    ParseResID(str, strLst);
    return strLst;
}

SNSEND
#endif /**< __SPLITSTRING__H__ */