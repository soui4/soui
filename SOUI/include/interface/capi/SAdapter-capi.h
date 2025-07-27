#ifndef __SADAPTER_CAPI_H__
#define __SADAPTER_CAPI_H__

#include "../SAdapter-i.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
 * C API Helper Macros for SAdapter Interfaces
 * These macros provide C-style function call syntax for C++ interface methods
 */

/* ILvDataSetObserver C API Macros */
#define ILvDataSetObserver_AddRef(This) \
    ((This)->lpVtbl->AddRef(This))

#define ILvDataSetObserver_Release(This) \
    ((This)->lpVtbl->Release(This))

#define ILvDataSetObserver_OnFinalRelease(This) \
    ((This)->lpVtbl->OnFinalRelease(This))

#define ILvDataSetObserver_onChanged(This) \
    ((This)->lpVtbl->onChanged(This))

#define ILvDataSetObserver_onInvalidated(This) \
    ((This)->lpVtbl->onInvalidated(This))

#define ILvDataSetObserver_OnItemChanged(This, iItem) \
    ((This)->lpVtbl->OnItemChanged(This, iItem))

/* ILvAdapter C API Macros */
#define ILvAdapter_AddRef(This) \
    ((This)->lpVtbl->AddRef(This))

#define ILvAdapter_Release(This) \
    ((This)->lpVtbl->Release(This))

#define ILvAdapter_OnFinalRelease(This) \
    ((This)->lpVtbl->OnFinalRelease(This))

#define ILvAdapter_registerDataSetObserver(This, observer) \
    ((This)->lpVtbl->registerDataSetObserver(This, observer))

#define ILvAdapter_unregisterDataSetObserver(This, observer) \
    ((This)->lpVtbl->unregisterDataSetObserver(This, observer))

#define ILvAdapter_getCount(This) \
    ((This)->lpVtbl->getCount(This))

#define ILvAdapter_getView(This, position, pItem, xmlTemplate) \
    ((This)->lpVtbl->getView(This, position, pItem, xmlTemplate))

#define ILvAdapter_getItemViewType(This, position, dwState) \
    ((This)->lpVtbl->getItemViewType(This, position, dwState))

#define ILvAdapter_getViewTypeCount(This) \
    ((This)->lpVtbl->getViewTypeCount(This))

#define ILvAdapter_getViewDesiredSize(This, ret, position, pItem, wid, hei) \
    ((This)->lpVtbl->getViewDesiredSize(This, ret, position, pItem, wid, hei))

#define ILvAdapter_isEmpty(This) \
    ((This)->lpVtbl->isEmpty(This))

#define ILvAdapter_getItemDesc(This, position, pDesc) \
    ((This)->lpVtbl->getItemDesc(This, position, pDesc))

#define ILvAdapter_InitByTemplate(This, xmlTemplate) \
    ((This)->lpVtbl->InitByTemplate(This, xmlTemplate))

#define ILvAdapter_QueryInterface(This, id, ppObj) \
    ((This)->lpVtbl->QueryInterface(This, id, ppObj))

/* IMcAdapter C API Macros (inherits from ILvAdapter) */
#define IMcAdapter_AddRef(This) \
    ((This)->lpVtbl->AddRef(This))

#define IMcAdapter_Release(This) \
    ((This)->lpVtbl->Release(This))

#define IMcAdapter_OnFinalRelease(This) \
    ((This)->lpVtbl->OnFinalRelease(This))

#define IMcAdapter_registerDataSetObserver(This, observer) \
    ((This)->lpVtbl->registerDataSetObserver(This, observer))

#define IMcAdapter_unregisterDataSetObserver(This, observer) \
    ((This)->lpVtbl->unregisterDataSetObserver(This, observer))

#define IMcAdapter_getCount(This) \
    ((This)->lpVtbl->getCount(This))

#define IMcAdapter_getView(This, position, pItem, xmlTemplate) \
    ((This)->lpVtbl->getView(This, position, pItem, xmlTemplate))

#define IMcAdapter_getItemViewType(This, position, dwState) \
    ((This)->lpVtbl->getItemViewType(This, position, dwState))

#define IMcAdapter_getViewTypeCount(This) \
    ((This)->lpVtbl->getViewTypeCount(This))

#define IMcAdapter_getViewDesiredSize(This, ret, position, pItem, wid, hei) \
    ((This)->lpVtbl->getViewDesiredSize(This, ret, position, pItem, wid, hei))

#define IMcAdapter_isEmpty(This) \
    ((This)->lpVtbl->isEmpty(This))

#define IMcAdapter_getItemDesc(This, position, pDesc) \
    ((This)->lpVtbl->getItemDesc(This, position, pDesc))

#define IMcAdapter_InitByTemplate(This, xmlTemplate) \
    ((This)->lpVtbl->InitByTemplate(This, xmlTemplate))

#define IMcAdapter_QueryInterface(This, id, ppObj) \
    ((This)->lpVtbl->QueryInterface(This, id, ppObj))

#define IMcAdapter_getColumnCount(This) \
    ((This)->lpVtbl->getColumnCount(This))

#define IMcAdapter_getColumnName(This, iCol, pName) \
    ((This)->lpVtbl->getColumnName(This, iCol, pName))

#define IMcAdapter_getColumnWidth(This, iCol) \
    ((This)->lpVtbl->getColumnWidth(This, iCol))

#define IMcAdapter_isColumnVisible(This, iCol) \
    ((This)->lpVtbl->isColumnVisible(This, iCol))

/*
 * C API Helper Functions (Optional - for more C-like usage)
 */

/* ILvDataSetObserver Helper Functions */
static inline long ILvDataSetObserver_AddRef_C(ILvDataSetObserver* pThis)
{
    return ILvDataSetObserver_AddRef(pThis);
}

static inline long ILvDataSetObserver_Release_C(ILvDataSetObserver* pThis)
{
    return ILvDataSetObserver_Release(pThis);
}

static inline void ILvDataSetObserver_onChanged_C(ILvDataSetObserver* pThis)
{
    ILvDataSetObserver_onChanged(pThis);
}

static inline void ILvDataSetObserver_onInvalidated_C(ILvDataSetObserver* pThis)
{
    ILvDataSetObserver_onInvalidated(pThis);
}

static inline void ILvDataSetObserver_OnItemChanged_C(ILvDataSetObserver* pThis, int iItem)
{
    ILvDataSetObserver_OnItemChanged(pThis, iItem);
}

/* ILvAdapter Helper Functions */
static inline long ILvAdapter_AddRef_C(ILvAdapter* pThis)
{
    return ILvAdapter_AddRef(pThis);
}

static inline long ILvAdapter_Release_C(ILvAdapter* pThis)
{
    return ILvAdapter_Release(pThis);
}

static inline void ILvAdapter_registerDataSetObserver_C(ILvAdapter* pThis, ILvDataSetObserver* observer)
{
    ILvAdapter_registerDataSetObserver(pThis, observer);
}

static inline void ILvAdapter_unregisterDataSetObserver_C(ILvAdapter* pThis, ILvDataSetObserver* observer)
{
    ILvAdapter_unregisterDataSetObserver(pThis, observer);
}

static inline int ILvAdapter_getCount_C(ILvAdapter* pThis)
{
    return ILvAdapter_getCount(pThis);
}

static inline void ILvAdapter_getView_C(ILvAdapter* pThis, int position, IWindow* pItem, IXmlNode* xmlTemplate)
{
    ILvAdapter_getView(pThis, position, pItem, xmlTemplate);
}

static inline int ILvAdapter_getItemViewType_C(ILvAdapter* pThis, int position, DWORD dwState)
{
    return ILvAdapter_getItemViewType(pThis, position, dwState);
}

static inline int ILvAdapter_getViewTypeCount_C(ILvAdapter* pThis)
{
    return ILvAdapter_getViewTypeCount(pThis);
}

static inline void ILvAdapter_getViewDesiredSize_C(ILvAdapter* pThis, SIZE* ret, int position, IWindow* pItem, int wid, int hei)
{
    ILvAdapter_getViewDesiredSize(pThis, ret, position, pItem, wid, hei);
}

static inline BOOL ILvAdapter_isEmpty_C(ILvAdapter* pThis)
{
    return ILvAdapter_isEmpty(pThis);
}

static inline void ILvAdapter_getItemDesc_C(ILvAdapter* pThis, int position, IStringW* pDesc)
{
    ILvAdapter_getItemDesc(pThis, position, pDesc);
}

static inline void ILvAdapter_InitByTemplate_C(ILvAdapter* pThis, IXmlNode* xmlTemplate)
{
    ILvAdapter_InitByTemplate(pThis, xmlTemplate);
}

static inline HRESULT ILvAdapter_QueryInterface_C(ILvAdapter* pThis, REFGUID id, IObjRef** ppObj)
{
    return ILvAdapter_QueryInterface(pThis, id, ppObj);
}

/*
 * Convenience macros for common adapter operations
 */
#define ILvAdapter_GetItemCount(This) \
    ILvAdapter_getCount(This)

#define ILvAdapter_IsEmpty(This) \
    ILvAdapter_isEmpty(This)

#define ILvAdapter_RegisterObserver(This, observer) \
    ILvAdapter_registerDataSetObserver(This, observer)

#define ILvAdapter_UnregisterObserver(This, observer) \
    ILvAdapter_unregisterDataSetObserver(This, observer)

#ifdef __cplusplus
}
#endif

#endif /* __SADAPTER_CAPI_H__ */
