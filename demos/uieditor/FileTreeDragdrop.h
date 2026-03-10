#ifndef __FILETREE_DRAGDROP_H__
#define __FILETREE_DRAGDROP_H__
#include <shlobj.h>
#include <helper/SUnkImpl.h>
#include <helper/SDragWnd.h>
#include "FileTreeAdapter.h"
#include <map>
class CMainDlg;
class FileTreeDropTarget :public SUnkImpl<IDropTarget>
{
	SAutoRefPtr<STreeView>  m_pTreeView;
    CMainDlg *m_pMainDlg;
	BOOL m_bDragIdx;
    DWORD m_dwEffect;
  public:
	FileTreeDropTarget(CMainDlg * pMainDlg, STreeView *pTreeView)
	{
        m_pMainDlg = pMainDlg;
		m_pTreeView = pTreeView;
		m_bDragIdx = FALSE;
        m_dwEffect = 0;
	}

	virtual ~FileTreeDropTarget() {}

	IUNKNOWN_BEGIN(IDropTarget)
	IUNKNOWN_END()

	//////////////////////////////////////////////////////////////////////////
	// IDropTarget

	STDMETHOD(DragEnter)(
		/* [unique][in] */  IDataObject *pDataObj,
		/* [in] */ DWORD grfKeyState,
		/* [in] */ POINTL pt,
		/* [out][in] */  DWORD *pdwEffect) OVERRIDE;
	STDMETHOD(DragOver)(
		/* [in] */ DWORD grfKeyState,
		/* [in] */ POINTL pt,
		/* [out][in] */  DWORD *pdwEffect) OVERRIDE;

	STDMETHOD(DragLeave)() OVERRIDE;

	STDMETHOD(Drop)(
		/* [unique][in] */  IDataObject *pDataObj,
		/* [in] */ DWORD grfKeyState,
		/* [in] */ POINTL pt,
		/* [out][in] */  DWORD *pdwEffect) OVERRIDE;
};

class FileTreeDataSource : public SUnkImpl<IDataObject>
{
private:
	HGLOBAL m_hGlobal; // 存储拖动数据的全局内存
	BOOL m_bOwnsMemory; // 是否拥有内存的所有权

public:
	FileTreeDataSource(HGLOBAL hGlobal, BOOL bOwnsMemory);

	virtual ~FileTreeDataSource();

	IUNKNOWN_BEGIN(IDataObject)
	IUNKNOWN_END()

	// IDataObject methods
	STDMETHOD(GetData)(FORMATETC *pFormatEtc, STGMEDIUM *pMedium) OVERRIDE;
	STDMETHOD(GetDataHere)(FORMATETC *pFormatEtc, STGMEDIUM *pMedium) OVERRIDE;

	STDMETHOD(QueryGetData)(FORMATETC *pFormatEtc) OVERRIDE;

	STDMETHOD(GetCanonicalFormatEtc)(FORMATETC *pFormatEtcIn, FORMATETC *pFormatEtcOut) OVERRIDE;
	STDMETHOD(SetData)(FORMATETC *pFormatEtc, STGMEDIUM *pMedium, BOOL fRelease) OVERRIDE
	{
		return S_OK;
	}

	STDMETHOD(EnumFormatEtc)(DWORD dwDirection, IEnumFORMATETC **ppEnumFormatEtc) OVERRIDE;
	STDMETHOD(DAdvise)(FORMATETC *pFormatEtc, DWORD advf, IAdviseSink *pAdvSink, DWORD *pdwConnection) OVERRIDE
	{
		return E_NOTIMPL;
	}

	STDMETHOD(DUnadvise)(DWORD dwConnection) OVERRIDE
	{
		return E_NOTIMPL;
	}

	STDMETHOD(EnumDAdvise)(IEnumSTATDATA **ppenumAdvise) OVERRIDE
	{
		return E_NOTIMPL;
	}
};

class FileTreeDropSource : public SUnkImpl<IDropSource>
{
private:
	HBITMAP m_hDragBitmap;
	POINT m_ptHot;
	bool m_bDragStarted;

public:
	FileTreeDropSource()
		: m_hDragBitmap(NULL)
		, m_bDragStarted(false)
	{
		m_ptHot.x = 0;
		m_ptHot.y = 0;
	}

	virtual ~FileTreeDropSource()
	{
        StopDrag();
	}

	// 使用SOUI的IRenderTarget创建拖动位图
    static HBITMAP CreateDragBitmap(const std::vector<SStringW> &filePaths);

	void SetDragImage(HBITMAP hBitmap, POINT ptHot);

	IUNKNOWN_BEGIN(IDropSource)
	IUNKNOWN_END()

	void StopDrag();
    // IDropSource methods
    STDMETHOD(QueryContinueDrag)(BOOL fEscapePressed, DWORD grfKeyState) OVERRIDE;

    STDMETHOD(GiveFeedback)(DWORD dwEffect) OVERRIDE;
};

#endif//__FILETREE_DRAGDROP_H__