#ifndef __SWNDFINDER__H__
#define __SWNDFINDER__H__

#include <interface/SWndFinder-i.h>
#include <helper/obj-ref-impl.hpp>

SNSBEGIN

/**
 * @struct SFindInfo
 * @brief Find information structure
 */
struct SFindInfo
{
  public:
    /**
     * @brief Constructor, find by name
     * @param pParent Pointer to parent window
     * @param strName Window name
     * @param nDeep Search depth
     */
    SFindInfo(IWindow *pParent, const SStringW &strName, int nDeep);

    /**
     * @brief Constructor, find by ID
     * @param pParent Pointer to parent window
     * @param nID Window ID
     * @param nDeep Search depth
     */
    SFindInfo(IWindow *pParent, int nID, int nDeep);

    SWND hParent;     /**< Parent window handle */
    bool findByName;  /**< Whether to find by name */
    SStringW strName; /**< Window name */
    int nID;          /**< Window ID */
    int nDeep;        /**< Search depth */
};

/**
 * @class CElementTraits<SFindInfo>
 * @brief Traits class for SFindInfo, defining hash and comparison operations
 */
template <>
class CElementTraits<SFindInfo> : public CElementTraitsBase<SFindInfo> {
  public:
    /**
     * @brief Compute the hash value of an SFindInfo object
     * @param fi SFindInfo object
     * @return Hash value
     */
    static ULONG Hash(INARGTYPE fi)
    {
        ULONG lRet = 0;
        if (fi.findByName)
            lRet = SStringElementTraits<SStringW>::Hash(fi.strName);
        else
            lRet = fi.nID << 16;

        union KEY {
            LONG key;
            struct long_sep
            {
                LONG hParent : 24;
                LONG nDeep : 7;
                LONG findByName : 1;
            } sep;
        } key;
        key.sep.hParent = fi.hParent;
        key.sep.nDeep = fi.nDeep;
        key.sep.findByName = fi.findByName;

        lRet += key.key;
        return lRet;
    }

    /**
     * @brief Compare whether two SFindInfo objects are equal
     * @param element1 First SFindInfo object
     * @param element2 Second SFindInfo object
     * @return Returns true if equal, otherwise false
     */
    static bool CompareElements(INARGTYPE element1, INARGTYPE element2)
    {
        bool bRet = element1.findByName == element2.findByName;
        if (bRet)
            bRet = element1.hParent == element2.hParent;
        if (bRet)
        {
            if (element1.findByName)
                bRet = element1.strName == element2.strName;
            else
                bRet = element1.nID == element2.nID;
        }
        return bRet;
    }

    /**
     * @brief Compare the order of two SFindInfo objects
     * @param element1 First SFindInfo object
     * @param element2 Second SFindInfo object
     * @return Returns a negative value if element1 is less than element2, 0 if equal, and a positive value if element1 is greater than element2
     */
    static int CompareElementsOrdered(INARGTYPE element1, INARGTYPE element2)
    {
        int nRet = element1.findByName - element2.findByName;
        if (nRet == 0)
            nRet = element1.hParent - element2.hParent;
        if (nRet == 0)
        {
            if (element1.findByName)
                nRet = element1.strName.Compare(element2.strName);
            else
                nRet = element1.nID - element2.nID;
        }
        return nRet;
    }
};

/**
 * @class SWindowFinder
 * @brief Window finder class
 */
class SWindowFinder : public TObjRefImpl<ISwndFinder> {
    friend class SWindow;

  protected:
    /**
     * @brief Find a child window by name
     * @param pParent Pointer to parent window
     * @param strName Window name
     * @param nDeep Search depth
     * @return Pointer to the found window, or NULL if not found
     */
    IWindow *WINAPI FindChildByName(IWindow *pParent, LPCWSTR strName, int nDeep) override;

    /**
     * @brief Find a child window by ID
     * @param pParent Pointer to parent window
     * @param nID Window ID
     * @param nDeep Search depth
     * @return Pointer to the found window, or NULL if not found
     */
    IWindow *WINAPI FindChildByID(IWindow *pParent, int nID, int nDeep) override;

    /**
     * @brief Cache the result of finding by name
     * @param pParent Pointer to parent window
     * @param strName Window name
     * @param nDeep Search depth
     * @param pResult Pointer to the found window
     */
    void WINAPI CacheResultForName(IWindow *pParent, LPCWSTR strName, int nDeep, IWindow *pResult) override;

    /**
     * @brief Cache the result of finding by ID
     * @param pParent Pointer to parent window
     * @param nID Window ID
     * @param nDeep Search depth
     * @param pResult Pointer to the found window
     */
    void WINAPI CacheResultForID(IWindow *pParent, int nID, int nDeep, IWindow *pResult) override;

    /**
     * @brief Remove cache
     * @param pParent Pointer to parent window
     * @param nID Window ID
     * @param nDeep Search depth
     * @return Returns true if removed successfully, otherwise false
     */
    BOOL WINAPI EraseCacheForID(IWindow *pParent, int nID, int nDeep) override;

    /**
     * @brief Remove cache
     * @param pParent Pointer to parent window
     * @param strName Window name
     * @param nDeep Search depth
     * @return Returns true if removed successfully, otherwise false
     */
    BOOL WINAPI EraseCacheForName(IWindow *pParent, LPCWSTR strName, int nDeep) override;

  protected:
    /**
     * @brief Find a child window by find information
     * @param pParent Pointer to parent window
     * @param fi Find information object
     * @return Pointer to the found window, or NULL if not found
     */
    IWindow *FindChildByKey(IWindow *pParent, const SFindInfo &fi);

    typedef SMap<SFindInfo, SWND> FINDCACHE; /**< Find cache type */
    FINDCACHE m_findCache;                   /**< Find cache */
};

SNSEND

#endif /**< __SWNDFINDER__H__ */