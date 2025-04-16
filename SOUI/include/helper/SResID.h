#ifndef __SRESID__H__
#define __SRESID__H__

#include <tchar.h>

#define MAX_RES_TYPE 30  // 资源类型预定义，最大长度不超过MAX_RES_TYPE
#define MAX_RES_NAME 100 // 注意：给资源名称时，最大长度不要超过MAX_RES_NAME

SNSBEGIN

/**
 * @class SResID
 * @brief 资源标识符类
 */
class SResID {
  public:
    /**
     * @brief 构造函数
     * @param pszType 资源类型字符串
     * @param pszName 资源名称字符串
     */
    SResID(LPCTSTR pszType, LPCTSTR pszName)
    {
        memset(this, 0, sizeof(SResID));
        if (pszType)
            _tcscpy_s(szType, MAX_RES_TYPE, pszType);
        if (pszName)
            _tcscpy_s(szName, MAX_RES_NAME, pszName);
        _tcslwr(szType);
        _tcslwr(szName);
    }

    /**
     * @brief 资源类型字符串
     */
    TCHAR szType[MAX_RES_TYPE + 1];

    /**
     * @brief 资源名称字符串
     */
    TCHAR szName[MAX_RES_NAME + 1];
};

/**
 * @class CElementTraits<SResID>
 * @brief 特性类，用于定义SResID的哈希和比较操作
 */
template <>
class CElementTraits<SResID> : public CElementTraitsBase<SResID> {
  public:
    /**
     * @brief 计算SResID对象的哈希值
     * @param resid SResID对象
     * @return 哈希值
     */
    static ULONG Hash(INARGTYPE resid)
    {
        ULONG nHash = 0;

        const TCHAR *pch = resid.szType;
        while (*pch != 0)
        {
            nHash = (nHash << 5) + nHash + (*pch);
            pch++;
        }

        pch = resid.szName;
        while (*pch != 0)
        {
            nHash = (nHash << 5) + nHash + (*pch);
            pch++;
        }
        return (nHash);
    }

    /**
     * @brief 比较两个SResID对象是否相等
     * @param element1 第一个SResID对象
     * @param element2 第二个SResID对象
     * @return 如果相等返回true，否则返回false
     */
    static bool CompareElements(INARGTYPE element1, INARGTYPE element2)
    {
        return _tcscmp(element1.szType, element2.szType) == 0 && _tcscmp(element1.szName, element2.szName) == 0;
    }

    /**
     * @brief 比较两个SResID对象的顺序
     * @param element1 第一个SResID对象
     * @param element2 第二个SResID对象
     * @return 如果element1小于element2返回负数，如果相等返回0，如果element1大于element2返回正数
     */
    static int CompareElementsOrdered(INARGTYPE element1, INARGTYPE element2)
    {
        int nRet = _tcscmp(element1.szType, element2.szType);
        if (nRet == 0)
            nRet = _tcscmp(element1.szName, element2.szName);
        return nRet;
    }
};

SNSEND
#endif // __SRESID__H__