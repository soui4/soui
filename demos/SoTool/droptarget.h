#ifndef _DROP_TARGET_H_
#define _DROP_TARGET_H_

#include <shellapi.h>

#include <atl.mini/SComHelper.h>

struct IFileDropHandler{
    virtual void OnFileDropdown(HDROP hDrop) = 0;
};

class CDropTarget:public SOUI::SUnknownImpl<IDropTarget>
{
protected:
    IFileDropHandler *m_pFileDropHandler;
public:
    CDropTarget(IFileDropHandler *pDropHandler):m_pFileDropHandler(pDropHandler)
    {
        SASSERT(m_pFileDropHandler);
    }

    virtual ~CDropTarget(){}

    COM_INTERFACE_BEGIN()
        COM_INTERFACE(IDropTarget)
    COM_INTERFACE_END()

        //////////////////////////////////////////////////////////////////////////
        // IDropTarget

        virtual HRESULT STDMETHODCALLTYPE DragEnter( 
        /* [unique][in] */ __RPC__in_opt IDataObject *pDataObj,
        /* [in] */ DWORD grfKeyState,
        /* [in] */ POINTL pt,
        /* [out][in] */ __RPC__inout DWORD *pdwEffect)
    {
        *pdwEffect=DROPEFFECT_LINK;
        return S_OK;
    }

    virtual HRESULT STDMETHODCALLTYPE DragOver( 
        /* [in] */ DWORD grfKeyState,
        /* [in] */ POINTL pt,
        /* [out][in] */ __RPC__inout DWORD *pdwEffect)
    {
        *pdwEffect=DROPEFFECT_LINK;
        return S_OK;
    }

    virtual HRESULT STDMETHODCALLTYPE DragLeave( void)
    {
        return S_OK;
    }

    virtual HRESULT STDMETHODCALLTYPE Drop( 
        /* [unique][in] */ __RPC__in_opt IDataObject *pDataObj,
        /* [in] */ DWORD grfKeyState,
        /* [in] */ POINTL pt,
        /* [out][in] */ __RPC__inout DWORD *pdwEffect)
    {
        FORMATETC format =
        {
            CF_HDROP, 0, DVASPECT_CONTENT, -1, TYMED_HGLOBAL
        };
        STGMEDIUM medium;
        if(FAILED(pDataObj->GetData(&format, &medium)))
        {
            return S_FALSE;
        }

        HDROP hdrop = static_cast<HDROP>(GlobalLock(medium.hGlobal));

        if(!hdrop)
        {
            return S_FALSE;
        }
        
        if(m_pFileDropHandler)
        {
            m_pFileDropHandler->OnFileDropdown(hdrop);
        }
        DragFinish(hdrop);
        GlobalUnlock(medium.hGlobal);


        *pdwEffect=DROPEFFECT_LINK;
        return S_OK;
    }

};


#endif//_DROP_TARGET_H_