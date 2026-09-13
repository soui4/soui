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
     * @brief Find a child window by name
     * @param pParent Pointer to parent window
     * @param strName Window name
     * @param nDeep Search depth
     * @return Pointer to the found window, or NULL if not found
     */
    STDMETHOD_(IWindow *, FindChildByName)(THIS_ IWindow * pParent, LPCWSTR strName, int nDeep) PURE;

    /**
     * @brief Find a child window by ID
     * @param pParent Pointer to parent window
     * @param nID Window ID
     * @param nDeep Search depth
     * @return Pointer to the found window, or NULL if not found
     */
    STDMETHOD_(IWindow *, FindChildByID)(THIS_ IWindow * pParent, int nID, int nDeep) PURE;

    /**
     * @brief Cache the result of finding by name
     * @param pParent Pointer to parent window
     * @param strName Window name
     * @param nDeep Search depth
     * @param pResult Pointer to the found window
     */
    STDMETHOD_(void, CacheResultForName)(THIS_ IWindow * pParent, LPCWSTR strName, int nDeep, IWindow *pResult) PURE;

    /**
     * @brief Cache the result of finding by ID
     * @param pParent Pointer to parent window
     * @param nID Window ID
     * @param nDeep Search depth
     * @param pResult Pointer to the found window
     */
    STDMETHOD_(void, CacheResultForID)(THIS_ IWindow * pParent, int nID, int nDeep, IWindow *pResult) PURE;

    /**
     * @brief Delete cached result
     * @param pParent parent window pointer
     * @param strName window name
     * @param nDeep cache depth
     * @return Returns TRUE on success, otherwise FALSE
     */
    STDMETHOD_(BOOL, EraseCacheForName)(THIS_ IWindow * pParent, LPCWSTR strName, int nDeep) PURE;

    /**
     * @brief Delete cached result
     * @param pParent parent window pointer
     * @param nID window ID
     * @param nDeep cache depth
     * @return Returns TRUE on success, otherwise FALSE
     */
    STDMETHOD_(BOOL, EraseCacheForID)(THIS_ IWindow * pParent, int nID, int nDeep) PURE;
};

SNSEND
#endif /**< __SWNDFINDER_I__H__ */