#include "TestLvAdapter.h"
#include <stdio.h>
#include <tchar.h>

#define VTABLE(ob, member) (*((ob)->lpVtbl->member))

#define IOB_VT_OF(x) ((IObjRefVtbl *((x)->lpVtbl))

// from stddef.h
#ifndef offsetof
#define offsetof(s,m)  (size_t)&(((s *)0)->m)
#endif

#define IMPL(class, member, pointer) \
	(&((class *)0)->member == pointer, ((class *) (((LONG_PTR) pointer) - offsetof(class, member))))




long WINAPI TestLvAdapter_AddRef(ILvAdapter *pObj)
{
	TestLvAdapter *_thiz = IMPL (TestLvAdapter, adapter, pObj);
	return ++_thiz->nRefs;
}

void WINAPI TestLvAdapter_OnFinalRelease(ILvAdapter *pObj)
{
	TestLvAdapter *_thiz = IMPL (TestLvAdapter, adapter, pObj);
	if(_thiz->obz)
	{
		_thiz->obz->lpVtbl->Release(_thiz->obz);
		_thiz->obz = NULL;
	}
	free(_thiz);
}

long WINAPI TestLvAdapter_Release(ILvAdapter *pObj)
{
	TestLvAdapter *_thiz = IMPL (TestLvAdapter, adapter, pObj);
	long nRet = --_thiz->nRefs;
	if(nRet == 0)
	{
		TestLvAdapter_OnFinalRelease(pObj);
	}
	return nRet;
}


    /**
     * Register an observer that is called when changes happen to the data used by this adapter.
     *
     * @param observer the object that gets notified when the data set changes.
     */
    void  WINAPI TestLvAdapter_registerDataSetObserver(ILvAdapter *pObj, ILvDataSetObserver * observer)
	{
		TestLvAdapter *_thiz = IMPL (TestLvAdapter, adapter, pObj);
		if(_thiz->obz == NULL)
		{
			_thiz->obz = observer;
			observer->lpVtbl->AddRef(observer);
		}
	}

    /**
     * Unregister an observer that has previously been registered with this
     * adapter via {@link #registerDataSetObserver}.
     *
     * @param observer the object to unregister.
     */
    void WINAPI TestLvAdapter_unregisterDataSetObserver(ILvAdapter *pObj, ILvDataSetObserver * observer)
	{
		TestLvAdapter *_thiz = IMPL (TestLvAdapter, adapter, pObj);
		if(_thiz->obz == observer)
		{
			_thiz->obz = NULL;
			observer->lpVtbl->Release(observer);
		}
	}
    /**
     * How many items are in the data set represented by this Adapter.
     *
     * @return Count of items.
     */
	int WINAPI TestLvAdapter_getCount(ILvAdapter *pObj){
		TestLvAdapter *_thiz = IMPL (TestLvAdapter, adapter, pObj);
		return _thiz->nItems;
	}

    /**
     * Get a View that displays the data at the specified position in the data set. You can either
     * create a View manually or inflate it from an XML layout file. When the View is inflated, the
     * parent View (GridView, ListView...) will apply default layout parameters unless you use
     * {@link android.view.LayoutInflater#inflate(int, android.view.ViewGroup, boolean)}
     * to specify a root view and to prevent attachment to the root.
     *
     * @param position The position of the item within the adapter's data set of the item whose view
     *        we want.
     * @param pItem The old view to reuse, if possible. Note: You should check that this view
     *        is non-null and of an appropriate type before using. If it is not possible to convert
     *        this view to display the correct data, this method can create a new view.
     *        Heterogeneous lists can specify their number of view types, so that this View is
     *        always of the right type (see {@link #getViewTypeCount()} and
     *        {@link #getItemViewType(int,DWORD)}).
     * @param xmlTemplate the xml template provided by its owner
     */
	void WINAPI TestLvAdapter_getView(ILvAdapter *pObj, int position, IWindow *pItem, IXmlNode *xmlTemplate){
		IWindow *pText=NULL;
		if(pItem->lpVtbl->GetChildrenCount(pItem)==0)
		{
			pItem->lpVtbl->InitFromXml(pItem,xmlTemplate);
		}
		pText = pItem->lpVtbl->FindIChildByName(pItem,L"txt_test");
		if(pText)
		{
			TCHAR szBuf[100];
			_stprintf(szBuf,_T("item %d"),position);
			pText->lpVtbl->SetWindowText(pText,szBuf);
		}
	}


    /**
     * Get the type of View that will be created by {@link #getView} for the specified item.
     *
     * @param position The position of the item within the adapter's data set whose view type we
     *        want.
     * @param dwState the state of view.
     * @return An integer representing the type of View. Two views should share the same type if one
     *         can be converted to the other in {@link #getView}. Note: Integers must be in the
     *         range 0 to {@link #getViewTypeCount} - 1. {@link #IGNORE_ITEM_VIEW_TYPE} can
     *         also be returned.
     * @see #IGNORE_ITEM_VIEW_TYPE
     */
    int WINAPI TestLvAdapter_getItemViewType(ILvAdapter *pObj, int position, DWORD dwState)
	{
		return 0;
	}

    /**
     * <p>
     * Returns the number of types of Views that will be created by
     * {@link #getView}. Each type represents a set of views that can be
     * converted in {@link #getView}. If the adapter always returns the same
     * type of View for all items, this method should return 1.
     * </p>
     * <p>
     * This method will only be called when when the adapter is set on the
     * the {@link AdapterView}.
     * </p>
     *
     * @return The number of types of Views that will be created by this adapter
     */
    int WINAPI TestLvAdapter_getViewTypeCount(ILvAdapter *pObj) {
		return 1;
	}

    void WINAPI TestLvAdapter_getViewDesiredSize(ILvAdapter *pObj, SIZE *ret,int position, IWindow *pItem, int wid, int hei) {
		pItem->lpVtbl->GetDesiredSize(pItem,ret,wid,hei);
	}


    /**
     * @return true if this adapter doesn't contain any data.  This is used to determine
     * whether the empty view should be displayed.  A typical implementation will return
     * getCount() == 0 but since getCount() includes the headers and footers, specialized
     * adapters might want a different behavior.
     */
    BOOL WINAPI TestLvAdapter_isEmpty(ILvAdapter *pObj) {
		return FALSE;
	}

    /**
     * @return a string object that associate with the specified item.
     */
    void WINAPI TestLvAdapter_getItemDesc(ILvAdapter *pObj, int positoin, IStringW *pDesc) {
		pDesc->lpVtbl->Assign(pDesc,L"test");
	}

    /**
     * init adapter from the specified template xml data
     */
    void WINAPI TestLvAdapter_InitByTemplate(ILvAdapter *pObj,IXmlNode * xmlTemplate) {}

	HRESULT WINAPI TestLvAdapter_QueryInterface(ILvAdapter *pObj,REFGUID id, IObjRef**ppObj) {
		return E_NOINTERFACE;
	}


ILvAdapterVtbl vtblTestLv ={
	TestLvAdapter_AddRef,
	TestLvAdapter_Release,
	TestLvAdapter_OnFinalRelease,
	TestLvAdapter_registerDataSetObserver,
	TestLvAdapter_unregisterDataSetObserver,
	TestLvAdapter_getCount,
	TestLvAdapter_getView,
	TestLvAdapter_getItemViewType,
	TestLvAdapter_getViewTypeCount,
	TestLvAdapter_getViewDesiredSize,
	TestLvAdapter_isEmpty,
	TestLvAdapter_getItemDesc,
	TestLvAdapter_InitByTemplate,
	TestLvAdapter_QueryInterface
};

BOOL TestLvAdapter_Init(ILvAdapter **pObj)
{
	TestLvAdapter *pAdapter = (TestLvAdapter *)malloc(sizeof(TestLvAdapter));
	pAdapter->adapter.lpVtbl = &vtblTestLv;
	pAdapter->nRefs = 1;
	pAdapter->nItems = 5;
	pAdapter->obz = NULL;
	*pObj = &pAdapter->adapter;
	return TRUE;
}

void TestLvAdapter_notifyChanged(ILvAdapter *pObj)
{
	TestLvAdapter *_thiz = IMPL (TestLvAdapter, adapter, pObj);
	if(_thiz->obz)
	{
		_thiz->obz->lpVtbl->onChanged(_thiz->obz);
	}
}

void TestLvAdapter_notifyInvalidated(ILvAdapter *pObj)
{
	TestLvAdapter *_thiz = IMPL (TestLvAdapter, adapter, pObj);
	if(_thiz->obz)
	{
		_thiz->obz->lpVtbl->onInvalidated(_thiz->obz);
	}
}

void TestLvAdapter_notifyItemChanged(ILvAdapter *pObj,int iItem)
{
	TestLvAdapter *_thiz = IMPL (TestLvAdapter, adapter, pObj);
	if(_thiz->obz)
	{
		_thiz->obz->lpVtbl->OnItemChanged(_thiz->obz,iItem);
	}
}

void TestLvAdapter_Add(ILvAdapter *pObj)
{
	TestLvAdapter *_thiz = IMPL (TestLvAdapter, adapter, pObj);
	_thiz->nItems++;
	TestLvAdapter_notifyChanged(pObj);
}

void TestLvAdapter_Clean(ILvAdapter *pObj)
{
	TestLvAdapter *_thiz = IMPL (TestLvAdapter, adapter, pObj);
	_thiz->nItems=0;
	TestLvAdapter_notifyChanged(pObj);
}