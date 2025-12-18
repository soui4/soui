#ifndef __SWNDFINDER__H__
#define __SWNDFINDER__H__

#include <interface/SWndFinder-i.h>
#include <helper/obj-ref-impl.hpp>

SNSBEGIN

/**
 * @struct SFindInfo
 * @brief 查找信息结构体
 */
struct SFindInfo
{
  public:
    /**
     * @brief 构造函数，通过名称查找
     * @param pParent 父窗口指针
     * @param strName 窗口名称
     * @param nDeep 查找深度
     */
    SFindInfo(IWindow *pParent, const SStringW &strName, int nDeep);

    /**
     * @brief 构造函数，通过ID查找
     * @param pParent 父窗口指针
     * @param nID 窗口ID
     * @param nDeep 查找深度
     */
    SFindInfo(IWindow *pParent, int nID, int nDeep);

    SWND hParent;     // 父窗口句柄
    bool findByName;  // 是否通过名称查找
    SStringW strName; // 窗口名称
    int nID;          // 窗口ID
    int nDeep;        // 查找深度
};

/**
 * @class CElementTraits<SFindInfo>
 * @brief SFindInfo的特性类，用于定义哈希和比较操作
 */
template <>
class CElementTraits<SFindInfo> : public CElementTraitsBase<SFindInfo> {
  public:
    /**
     * @brief 计算SFindInfo对象的哈希值
     * @param fi SFindInfo对象
     * @return 哈希值
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
     * @brief 比较两个SFindInfo对象是否相等
     * @param element1 第一个SFindInfo对象
     * @param element2 第二个SFindInfo对象
     * @return 如果相等返回true，否则返回false
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
     * @brief 比较两个SFindInfo对象的顺序
     * @param element1 第一个SFindInfo对象
     * @param element2 第二个SFindInfo对象
     * @return 如果element1小于element2返回负数，如果相等返回0，如果element1大于element2返回正数
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
 * @brief 窗口查找器类
 */
class SWindowFinder : public TObjRefImpl<ISwndFinder> {
    friend class SWindow;

  protected:
    /**
     * @brief 通过名称查找子窗口
     * @param pParent 父窗口指针
     * @param strName 窗口名称
     * @param nDeep 查找深度
     * @return 找到的窗口指针，未找到返回NULL
     */
    IWindow *WINAPI FindChildByName(IWindow *pParent, LPCWSTR strName, int nDeep) override;

    /**
     * @brief 通过ID查找子窗口
     * @param pParent 父窗口指针
     * @param nID 窗口ID
     * @param nDeep 查找深度
     * @return 找到的窗口指针，未找到返回NULL
     */
    IWindow *WINAPI FindChildByID(IWindow *pParent, int nID, int nDeep) override;

    /**
     * @brief 缓存通过名称查找的结果
     * @param pParent 父窗口指针
     * @param strName 窗口名称
     * @param nDeep 查找深度
     * @param pResult 找到的窗口指针
     */
    void WINAPI CacheResultForName(IWindow *pParent, LPCWSTR strName, int nDeep, IWindow *pResult) override;

    /**
     * @brief 缓存通过ID查找的结果
     * @param pParent 父窗口指针
     * @param nID 窗口ID
     * @param nDeep 查找深度
     * @param pResult 找到的窗口指针
     */
    void WINAPI CacheResultForID(IWindow *pParent, int nID, int nDeep, IWindow *pResult) override;

    /**
     * @brief 删除缓存
     * @param pParent 父窗口指针
     * @param nID 窗口ID
     * @param nDeep 查找深度
     * @return 删除成功返回true，否则返回false
     */
    BOOL WINAPI EraseCacheForID(IWindow *pParent, int nID, int nDeep) override;

    /**
     * @brief 删除缓存
     * @param pParent 父窗口指针
     * @param strName 窗口名称
     * @param nDeep 查找深度
     * @return 删除成功返回true，否则返回false
     */
    BOOL WINAPI EraseCacheForName(IWindow *pParent, LPCWSTR strName, int nDeep) override;

  protected:
    /**
     * @brief 通过查找信息查找子窗口
     * @param pParent 父窗口指针
     * @param fi 查找信息对象
     * @return 找到的窗口指针，未找到返回NULL
     */
    IWindow *FindChildByKey(IWindow *pParent, const SFindInfo &fi);

    typedef SMap<SFindInfo, SWND> FINDCACHE; // 查找缓存类型
    FINDCACHE m_findCache;                   // 查找缓存
};

SNSEND

#endif // __SWNDFINDER__H__