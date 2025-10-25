#ifndef _DROP_TARGET_H_
#define _DROP_TARGET_H_

#include <shellapi.h>
#include <helper/SUnknown.h>


class CDropTarget : public SUnknown, public IDropTarget
{
public:
	struct IDropListener
	{
		virtual BOOL OnDrop(LPCTSTR pszName) =0;
	} * m_pListener;

public:
	CDropTarget(IDropListener *pListener) :m_pListener(pListener)
	{
	}
	~CDropTarget()
	{
	}
public:

	//////////////////////////////////////////////////////////////////////////
	// IDropTarget

	virtual HRESULT STDMETHODCALLTYPE DragEnter(
		/* [unique][in] */ __RPC__in_opt IDataObject *pDataObj,
		/* [in] */ DWORD grfKeyState,
		/* [in] */ POINTL pt,
		/* [out][in] */ __RPC__inout DWORD *pdwEffect)
	{
		*pdwEffect = DROPEFFECT_LINK;
		return S_OK;
	}

	virtual HRESULT STDMETHODCALLTYPE DragOver(
		/* [in] */ DWORD grfKeyState,
		/* [in] */ POINTL pt,
		/* [out][in] */ __RPC__inout DWORD *pdwEffect)
	{
		*pdwEffect = DROPEFFECT_LINK;
		return S_OK;
	}

	virtual HRESULT STDMETHODCALLTYPE DragLeave(void)
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
		if (FAILED(pDataObj->GetData(&format, &medium)))
		{
			return S_FALSE;
		}

		HDROP hdrop = static_cast<HDROP>(GlobalLock(medium.hGlobal));

		if (!hdrop)
		{
			return S_FALSE;
		}

		bool success = false;
		TCHAR filename[MAX_PATH];
		success = !!DragQueryFile(hdrop, 0, filename, MAX_PATH);
		DragFinish(hdrop);
		GlobalUnlock(medium.hGlobal);

		HRESULT hRet = S_OK;
		if (success && m_pListener)
		{
			if(!m_pListener->OnDrop(filename))
				hRet = S_FALSE;
		}

		*pdwEffect = DROPEFFECT_LINK;
		return hRet;
	}

public:
	IUNKNOWN_BEGIN(IDropTarget)
	IUNKNOWN_END()
};

#endif//_DROP_TARGET_H_