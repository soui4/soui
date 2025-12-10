#ifndef __SWNDFINDER_I__H__
#define __SWNDFINDER_I__H__

#include <interface/obj-ref-i.h>

SNSBEGIN

typedef struct IWindow IWindow;

#undef INTERFACE
#define INTERFACE ISwndFinder
DECLARE_INTERFACE_(ISwndFinder, IObjRef)
{
    /**
     * @brief Increments the reference count.
     * @return The new reference count.
     */
    STDMETHOD_(long, AddRef)(THIS) PURE;

    /**
     * @brief Decrements the reference count.
     * @return The new reference count.
     */
    STDMETHOD_(long, Release)(THIS) PURE;

    /**
     * @brief Finalizes the release of the object.
     * @return void
     */
    STDMETHOD_(void, OnFinalRelease)(THIS) PURE;

    //---------------------------------------------------------------------
    /**
     * @brief 通过名称查找子窗口
     * @param pParent 父窗口指针
     * @param strName 窗口名称
     * @param nDeep 查找深度
     * @return 找到的窗口指针，未找到返回NULL
     */
    STDMETHOD_(IWindow *, FindChildByName)(THIS_ IWindow * pParent, LPCWSTR strName, int nDeep) PURE;

    /**
     * @brief 通过ID查找子窗口
     * @param pParent 父窗口指针
     * @param nID 窗口ID
     * @param nDeep 查找深度
     * @return 找到的窗口指针，未找到返回NULL
     */
    STDMETHOD_(IWindow *, FindChildByID)(THIS_ IWindow * pParent, int nID, int nDeep) PURE;

    /**
     * @brief 缓存通过名称查找的结果
     * @param pParent 父窗口指针
     * @param strName 窗口名称
     * @param nDeep 查找深度
     * @param pResult 找到的窗口指针
     */
    STDMETHOD_(void, CacheResultForName)(THIS_ IWindow * pParent, LPCWSTR strName, int nDeep, IWindow *pResult) PURE;

    /**
     * @brief 缓存通过ID查找的结果
     * @param pParent 父窗口指针
     * @param nID 窗口ID
     * @param nDeep 查找深度
     * @param pResult 找到的窗口指针
     */
    STDMETHOD_(void, CacheResultForID)(THIS_ IWindow * pParent, int nID, int nDeep, IWindow *pResult) PURE;

    /**
     * @brief 删除缓存结果
     * @param pParent 父窗口指针
     * @param strName 窗口名称
     * @param nDeep 缓存深度
     * @return 删除成功返回TRUE，否则返回FALSE
     */
    STDMETHOD_(BOOL, EraseCacheForName)(THIS_ IWindow * pParent, LPCWSTR strName, int nDeep) PURE;

    /**
     * @brief 删除缓存结果
     * @param pParent 父窗口指针
     * @param nID 窗口ID
     * @param nDeep 缓存深度
     * @return 删除成功返回TRUE，否则返回FALSE
     */
    STDMETHOD_(BOOL, EraseCacheForID)(THIS_ IWindow * pParent, int nID, int nDeep) PURE;
};

SNSEND
#endif //__SWNDFINDER_I__H__