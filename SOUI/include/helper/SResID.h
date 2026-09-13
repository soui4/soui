#ifndef __SRESID__H__
#define __SRESID__H__

#include <tchar.h>

#define MAX_RES_TYPE 30  /**< Resource type predefined, max length not exceeding MAX_RES_TYPE */
#define MAX_RES_NAME 100 /**< Note: when giving a resource name, the max length should not exceed MAX_RES_NAME */

SNSBEGIN

/**
 * @class SResID
 * @brief Resource identifier class
 */
class SResID {
  public:
    /**
     * @brief Constructor
     * @param pszType Resource type string
     * @param pszName Resource name string
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
     * @brief Resource type string
     */
    TCHAR szType[MAX_RES_TYPE + 1];

    /**
     * @brief Resource name string
     */
    TCHAR szName[MAX_RES_NAME + 1];
};

/**
 * @class CElementTraits<SResID>
 * @brief Traits class used to define the hash and comparison operations for SResID
 */
template <>
class CElementTraits<SResID> : public CElementTraitsBase<SResID> {
  public:
    /**
     * @brief Compute the hash value of an SResID object
     * @param resid SResID object
     * @return Hash value
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
     * @brief Compare whether two SResID objects are equal
     * @param element1 First SResID object
     * @param element2 Second SResID object
     * @return Returns true if equal, otherwise returns false
     */
    static bool CompareElements(INARGTYPE element1, INARGTYPE element2)
    {
        return _tcscmp(element1.szType, element2.szType) == 0 && _tcscmp(element1.szName, element2.szName) == 0;
    }

    /**
     * @brief Compare the order of two SResID objects
     * @param element1 First SResID object
     * @param element2 Second SResID object
     * @return Returns a negative number if element1 is less than element2, 0 if equal, a positive number if element1 is greater than element2
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
#endif /**< __SRESID__H__ */