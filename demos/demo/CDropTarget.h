﻿#pragma once
class CTestDropTarget :public IDropTarget
{
public:
	CTestDropTarget()
	{
		nRef = 0;
	}

	virtual ~CTestDropTarget() {}

	//////////////////////////////////////////////////////////////////////////
	// IUnknown
	virtual HRESULT STDMETHODCALLTYPE QueryInterface(
		/* [in] */ REFIID riid,
		/* [iid_is][out] */  void  * *ppvObject)
	{
		HRESULT hr = S_FALSE;
		if (riid == __uuidof(IUnknown))
			*ppvObject = (IUnknown*)this, hr = S_OK;
		else if (riid == __uuidof(IDropTarget))
			*ppvObject = (IDropTarget*)this, hr = S_OK;
		if (SUCCEEDED(hr)) AddRef();
		return hr;

	}

	virtual ULONG STDMETHODCALLTYPE AddRef(void) { return ++nRef; }

	virtual ULONG STDMETHODCALLTYPE Release(void) {
		ULONG uRet = --nRef;
		if (uRet == 0) delete this;
		return uRet;
	}

	//////////////////////////////////////////////////////////////////////////
	// IDropTarget

	virtual HRESULT STDMETHODCALLTYPE DragEnter(
		/* [unique][in] */  IDataObject *pDataObj,
		/* [in] */ DWORD grfKeyState,
		/* [in] */ POINTL pt,
		/* [out][in] */  DWORD *pdwEffect)
	{
		*pdwEffect = DROPEFFECT_LINK;
		return S_OK;
	}

	virtual HRESULT STDMETHODCALLTYPE DragOver(
		/* [in] */ DWORD grfKeyState,
		/* [in] */ POINTL pt,
		/* [out][in] */  DWORD *pdwEffect)
	{
		*pdwEffect = DROPEFFECT_LINK;
		return S_OK;
	}

	virtual HRESULT STDMETHODCALLTYPE DragLeave(void)
	{
		return S_OK;
	}


protected:
	int nRef;
};

class CTestDropTarget1 : public CTestDropTarget
{
protected:
	SEdit *m_pEdit;
public:
	CTestDropTarget1(SEdit *pEdit) :m_pEdit(pEdit)
	{
		if (m_pEdit) m_pEdit->AddRef();
	}
	~CTestDropTarget1()
	{
		if (m_pEdit) m_pEdit->Release();
	}
public:
	virtual HRESULT STDMETHODCALLTYPE Drop(
		/* [unique][in] */  IDataObject *pDataObj,
		/* [in] */ DWORD grfKeyState,
		/* [in] */ POINTL pt,
		/* [out][in] */  DWORD *pdwEffect)
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
		for(int i=0;i<nFiles;i++){
			bool success = false;
			TCHAR filename[MAX_PATH];
			int bufLen = DragQueryFile(hdrop, i, filename, MAX_PATH);
			if (bufLen>0 && m_pEdit)
			{
				_tcscpy(filename+bufLen,_T("\n"));
				m_pEdit->SetSel(-1,-1,false);
				m_pEdit->ReplaceSel(filename,false);
			}
		}
		DragFinish(hdrop);
		GlobalUnlock(medium.hGlobal);
		m_pEdit->Update();

		*pdwEffect = DROPEFFECT_LINK;
		return S_OK;
	}
};
