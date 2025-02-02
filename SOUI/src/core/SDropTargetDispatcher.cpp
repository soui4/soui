#include "souistd.h"
#include "core/SDropTargetDispatcher.h"
#include <core/SWndContainerImpl.h>

SNSBEGIN
SDropTargetDispatcher::SDropTargetDispatcher()
    : m_pDataObj(NULL)
    , m_hHover(0)
    , m_pOwner(NULL)
{
}

void SDropTargetDispatcher::SetOwner(SWindow *pOwner)
{
    m_pOwner = pOwner;
}

SDropTargetDispatcher::~SDropTargetDispatcher(void)
{
    DragLeave();
    SPOSITION pos = m_mapDropTarget.GetStartPosition();
    while (pos)
    {
        DTMAP::CPair *pPair = m_mapDropTarget.GetNext(pos);
        pPair->m_value->Release();
    }
}

BOOL SDropTargetDispatcher::RegisterDragDrop(SWND swnd, IDropTarget *pDropTarget)
{
    if (m_mapDropTarget.Lookup(swnd))
        return FALSE;
    m_mapDropTarget[swnd] = pDropTarget;
    pDropTarget->AddRef();
    return TRUE;
}

BOOL SDropTargetDispatcher::UnregisterDragDrop(SWND swnd)
{
    DTMAP::CPair *pPair = m_mapDropTarget.Lookup(swnd);
    if (!pPair)
        return FALSE;
    pPair->m_value->Release();
    m_mapDropTarget.RemoveKey(swnd);
    return TRUE;
}

#ifdef _WIN32
#define __uuidofsys __uuidof
#else
#define __uuidofsys __suidof
#endif

HRESULT SDropTargetDispatcher::QueryInterface(/* [in] */ REFIID riid,
                                              /* [iid_is][out] */ void **ppvObject)
{
    HRESULT hr = S_FALSE;
    if (riid == __uuidofsys(IUnknown))
        *ppvObject = (IUnknown *)this, hr = S_OK;
    else if (riid == __uuidofsys(IDropTarget))
        *ppvObject = (IDropTarget *)this, hr = S_OK;
    return hr;
}

HRESULT SDropTargetDispatcher::DragEnter(/* [unique][in] */ IDataObject *pDataObj,
                                         /* [in] */ DWORD grfKeyState,
                                         /* [in] */ POINTL pt,
                                         /* [out][in] */ DWORD *pdwEffect)
{
    m_pDataObj = pDataObj;
    m_pDataObj->AddRef();
    return DragOver(grfKeyState, pt, pdwEffect);
}

HRESULT SDropTargetDispatcher::DragOver(/* [in] */ DWORD grfKeyState,
                                        /* [in] */ POINTL pt,
                                        /* [out][in] */ DWORD *pdwEffect)
{
    *pdwEffect = DROPEFFECT_NONE;
    CPoint pt2 = PointL2FrameClient(pt);
    CPoint pt3 = pt2;
    SWND hHover = m_pOwner->SwndFromPoint(pt2);

    if (hHover != m_hHover)
    {
        DTMAP::CPair *pPairOld = m_mapDropTarget.Lookup(m_hHover);
        DTMAP::CPair *pPairNew = m_mapDropTarget.Lookup(hHover);
        if (pPairOld && !pPairNew)
        {
            SWindow *pOldWnd = SWindowMgr::GetWindow(m_hHover);
            if (pOldWnd)
            {
                CRect rcWnd = pOldWnd->GetWindowRect();
                if (rcWnd.PtInRect(pt3))
                {
                    pPairOld->m_value->DragOver(grfKeyState, pt, pdwEffect);
                    return S_OK;
                }
            }
        }
        if (m_hHover && pPairOld)
            pPairOld->m_value->DragLeave();
        m_hHover = hHover;
        if (pPairNew && m_hHover)
            pPairNew->m_value->DragEnter(m_pDataObj, grfKeyState, pt, pdwEffect);
    }
    else
    {
        DTMAP::CPair *pPair = m_mapDropTarget.Lookup(m_hHover);
        if (m_hHover && pPair)
            pPair->m_value->DragOver(grfKeyState, pt, pdwEffect);
    }

    return S_OK;
}

HRESULT STDMETHODCALLTYPE SDropTargetDispatcher::DragLeave(void)
{
    if (m_pDataObj)
    {
        m_pDataObj->Release();
        m_pDataObj = NULL;
    }
    return S_OK;
}

HRESULT STDMETHODCALLTYPE SDropTargetDispatcher::Drop(/* [unique][in] */ IDataObject *pDataObj,
                                                      /* [in] */ DWORD grfKeyState,
                                                      /* [in] */ POINTL pt,
                                                      /* [out][in] */ DWORD *pdwEffect)
{
    DTMAP::CPair *pPair = m_mapDropTarget.Lookup(m_hHover);
    if (m_hHover && pPair)
        pPair->m_value->Drop(pDataObj, grfKeyState, pt, pdwEffect);
    m_hHover = 0;
    m_pDataObj->Release();
    m_pDataObj = NULL;
    return S_OK;
}

POINT SDropTargetDispatcher::PointL2FrameClient(const POINTL &pt)
{
    CPoint pt2(pt.x, pt.y);
    ScreenToClient(m_pOwner->GetContainer()->GetHostHwnd(), &pt2);
    return pt2;
}

SNSEND
