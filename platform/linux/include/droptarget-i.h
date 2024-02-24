#ifndef _DROP_TARGET_I_H_
#define _DROP_TARGET_I_H_

#include <unknown-i.h>
#include <dataobject-i.h>

#undef INTERFACE
#define INTERFACE IDropTarget
DECLARE_INTERFACE_(IDropTarget, IUnknown)
{
    DECLARE_CLASS_SIID(0, 0, 0, 0, 0, 8)

    STDMETHOD_(HRESULT, DragEnter)(THIS_
        /* [unique][in] */ __RPC__in_opt IDataObject * pDataObj,
        /* [in] */ DWORD grfKeyState,
        /* [in] */ POINTL pt,
        /* [out][in] */ DWORD * pdwEffect)
        = 0;

    STDMETHOD_(HRESULT, DragOver)(THIS_ 
        /* [in] */ DWORD grfKeyState,
        /* [in] */ POINTL pt,
        /* [out][in] */ DWORD * pdwEffect)
        = 0;

    STDMETHOD_(HRESULT,DragLeave)(THIS) = 0;

    STDMETHOD_(HRESULT,Drop)(THIS_
        /* [unique][in] */ IDataObject * pDataObj,
        /* [in] */ DWORD grfKeyState,
        /* [in] */ POINTL pt,
        /* [out][in] */ DWORD * pdwEffect)
        = 0;
};

#endif //_DROP_TARGET_I_H_