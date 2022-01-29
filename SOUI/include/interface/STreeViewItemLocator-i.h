#pragma once

#include <interface/SAdapter-i.h>

SNSBEGIN

#undef INTERFACE
#define INTERFACE ITreeViewItemLocator
DECLARE_INTERFACE_(ITreeViewItemLocator, IObjRef)
{
    STDMETHOD_(void, SetAdapter)(THIS_ ITvAdapter * pAdapter) PURE;
    STDMETHOD_(void, OnBranchChanged)(THIS_ HSTREEITEM hItem) PURE;
    STDMETHOD_(void, OnBranchExpandedChanged)
    (THIS_ HSTREEITEM hItem, BOOL bExpandedOld, BOOL bExpandedNew) PURE;

    STDMETHOD_(int, GetTotalHeight)(THIS) SCONST PURE;
    STDMETHOD_(int, GetTotalWidth)(THIS) SCONST PURE;
    STDMETHOD_(int, Item2Position)(THIS_ HSTREEITEM hItem) SCONST PURE;
    STDMETHOD_(HSTREEITEM, Position2Item)(THIS_ int position) SCONST PURE;
    STDMETHOD_(int, GetScrollLineSize)(THIS) SCONST PURE;

    STDMETHOD_(void, SetItemWidth)(THIS_ HSTREEITEM hItem, int nWidth) PURE;
    STDMETHOD_(int, GetItemWidth)(THIS_ HSTREEITEM hItem) SCONST PURE;
    STDMETHOD_(void, SetItemHeight)(THIS_ HSTREEITEM hItem, int nHeight) PURE;
    STDMETHOD_(int, GetItemHeight)(THIS_ HSTREEITEM hItem) SCONST PURE;
    STDMETHOD_(int, GetItemIndent)(THIS_ HSTREEITEM hItem) SCONST PURE;
};

SNSEND