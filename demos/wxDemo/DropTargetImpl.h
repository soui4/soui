#ifndef _DROP_TARGET_IMPL_H_
#define _DROP_TARGET_IMPL_H_

#include <helper/SUnkImpl.h>
#include <shellapi.h>

SNSBEGIN

class CDropTarget :public SUnkImpl<IDropTarget>
{
public:
	CDropTarget()
	{
	}

	virtual ~CDropTarget() {}

	//////////////////////////////////////////////////////////////////////////
	// IDropTarget

	virtual HRESULT STDMETHODCALLTYPE DragEnter(
		/* [unique][in] */  IDataObject* pDataObj,
		/* [in] */ DWORD grfKeyState,
		/* [in] */ POINTL pt,
		/* [out][in] */  DWORD* pdwEffect)
	{
		*pdwEffect = DROPEFFECT_LINK;
		return S_OK;
	}

	virtual HRESULT STDMETHODCALLTYPE DragOver(
		/* [in] */ DWORD grfKeyState,
		/* [in] */ POINTL pt,
		/* [out][in] */  DWORD* pdwEffect)
	{
		*pdwEffect = DROPEFFECT_LINK;
		return S_OK;
	}

	virtual HRESULT STDMETHODCALLTYPE DragLeave(void)
	{
		return S_OK;
	}

	IUNKNOWN_BEGIN(IDropTarget)
		IUNKNOWN_END()
};

class CEditDropTarget : public CDropTarget
{
protected:
	SEdit* m_pEdit;
public:
	CEditDropTarget(SEdit* pEdit) :m_pEdit(pEdit)
	{
		if (m_pEdit) m_pEdit->AddRef();
	}
	~CEditDropTarget()
	{
		if (m_pEdit) m_pEdit->Release();
	}
public:
	virtual HRESULT STDMETHODCALLTYPE Drop(
		/* [unique][in] */  IDataObject* pDataObj,
		/* [in] */ DWORD grfKeyState,
		/* [in] */ POINTL pt,
		/* [out][in] */  DWORD* pdwEffect)
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

		int nFiles = DragQueryFile(hdrop, -1, NULL, 0);
		for (int i = 0; i < nFiles; i++) {
			bool success = false;
			TCHAR filename[MAX_PATH];
			int bufLen = DragQueryFile(hdrop, i, filename, MAX_PATH);
			if (bufLen > 0 && m_pEdit)
			{
				_tcscpy(filename + bufLen, _T("\n"));
				m_pEdit->SetSel(-1, -1, false);
				m_pEdit->ReplaceSel(filename, false);
			}
		}
		DragFinish(hdrop);
		GlobalUnlock(medium.hGlobal);
		m_pEdit->Update();

		*pdwEffect = DROPEFFECT_LINK;
		return S_OK;
	}
};

SNSEND
#endif//_DROP_TARGET_IMPL_H_