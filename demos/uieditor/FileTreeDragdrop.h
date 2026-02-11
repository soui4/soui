#ifndef __FILETREE_DRAGDROP_H__
#define __FILETREE_DRAGDROP_H__
#include <shlobj.h>
#include <helper/SUnkImpl.h>
#include <helper/SDragWnd.h>
#include "FileTreeAdapter.h"
#include <map>
#include <helper/slog.h>

class FileTreeDropTarget :public SUnkImpl<IDropTarget>
{
	SAutoRefPtr<STreeView>  m_pTreeView;
public:
	FileTreeDropTarget(STreeView *pTreeView)
	{
		m_pTreeView = pTreeView;
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
		/* [out][in] */  DWORD *pdwEffect) OVERRIDE
	{
		// 检查数据对象是否包含 CF_HDROP 格式的数据
		FORMATETC format;
		format.cfFormat = CF_HDROP;
		format.ptd = NULL;
		format.dwAspect = DVASPECT_CONTENT;
		format.lindex = -1;
		format.tymed = TYMED_HGLOBAL;

		if(pDataObj->QueryGetData(&format) == S_OK)
		{
			// 检查目标是否是有效的移动目标
			POINT pt2={pt.x,pt.y};
			ScreenToClient(m_pTreeView->GetContainer()->GetHostHwnd(),&pt2);
			IItemPanel * pItem = m_pTreeView->HitTest(&pt2);
			if(pItem)
			{
				HSTREEITEM hItem = (HSTREEITEM)pItem->GetItemIndex();
				// 检查目标是否是目录
				ITvAdapter *pAdapter = m_pTreeView->GetAdapter();
				if(pAdapter)
				{
					// 查询CFileTreeAdapter接口
					CFileTreeAdapter *pFileAdapter = (CFileTreeAdapter *)pAdapter;
					{
						const FileItemData& itemData = pFileAdapter->GetItemData(hItem);
						// 只有当目标是目录时才允许移动
						if(itemData.bIsDir)
						{
							*pdwEffect = DROPEFFECT_MOVE;
						}
						else
						{
							*pdwEffect = DROPEFFECT_NONE;
						}
					}
				}
			}
			else
			{
				*pdwEffect = DROPEFFECT_NONE;
			}
		}
		else
		{
			*pdwEffect = DROPEFFECT_NONE;
		}
		return S_OK;
	}

	STDMETHOD(DragOver)(
		/* [in] */ DWORD grfKeyState,
		/* [in] */ POINTL pt,
		/* [out][in] */  DWORD *pdwEffect) OVERRIDE
	{
		// 检查目标是否是有效的移动目标
		POINT pt2={pt.x,pt.y};
		ScreenToClient(m_pTreeView->GetContainer()->GetHostHwnd(),&pt2);
		IItemPanel * pItem = m_pTreeView->HitTest(&pt2);
		if(pItem)
		{
			HSTREEITEM hItem = (HSTREEITEM)pItem->GetItemIndex();
			// 检查目标是否是目录
			ITvAdapter *pAdapter = m_pTreeView->GetAdapter();
			if(pAdapter)
			{
				// 查询CFileTreeAdapter接口
				CFileTreeAdapter *pFileAdapter = (CFileTreeAdapter *)pAdapter;
				{
					const FileItemData& itemData = pFileAdapter->GetItemData(hItem);
					// 只有当目标是目录时才允许移动
					if(itemData.bIsDir)
					{
						*pdwEffect = DROPEFFECT_MOVE;
					}
					else
					{
						*pdwEffect = DROPEFFECT_NONE;
					}
				}
			}
		}
		else
		{
			*pdwEffect = DROPEFFECT_NONE;
		}
		return S_OK;
	}

	STDMETHOD(DragLeave)() OVERRIDE
	{
		return S_OK;
	}

	STDMETHOD(Drop)(
		/* [unique][in] */  IDataObject *pDataObj,
		/* [in] */ DWORD grfKeyState,
		/* [in] */ POINTL pt,
		/* [out][in] */  DWORD *pdwEffect)
	{
		POINT pt2={pt.x,pt.y};
		ScreenToClient(m_pTreeView->GetContainer()->GetHostHwnd(),&pt2);
		IItemPanel * pItem = m_pTreeView->HitTest(&pt2);
		if(!pItem)
		{
			*pdwEffect = DROPEFFECT_NONE;
			return S_OK;
		}
		HSTREEITEM hDestParentItem = (HSTREEITEM)pItem->GetItemIndex();
		
		// 检查目标是否是目录
		ITvAdapter *pAdapter = m_pTreeView->GetAdapter();
		// 查询CFileTreeAdapter接口
		CFileTreeAdapter *pFileAdapter = (CFileTreeAdapter *)pAdapter;
		
		// 检查目标是否是目录
		const FileItemData& destItemData = pFileAdapter->GetItemData(hDestParentItem);
		if(!destItemData.bIsDir)
		{
			*pdwEffect = DROPEFFECT_NONE;
			return S_OK;
		}
		
		// 检查数据对象是否包含 CF_HDROP 格式的数据
		FORMATETC format;
		format.cfFormat = CF_HDROP;
		format.ptd = NULL;
		format.dwAspect = DVASPECT_CONTENT;
		format.lindex = -1;
		format.tymed = TYMED_HGLOBAL;

		STGMEDIUM medium = { 0 };
		if(pDataObj->GetData(&format, &medium) == S_OK)
		{
			// 获取 HDROP 数据
			HDROP hdrop = (HDROP)GlobalLock(medium.hGlobal);
			if(hdrop)
			{
				// 获取文件数量
				UINT fileCount = DragQueryFile(hdrop, 0xFFFFFFFF, NULL, 0);
				for(UINT i = 0; i < fileCount; i++)
				{
					// 获取文件路径
					TCHAR filePath[MAX_PATH];
					if(DragQueryFile(hdrop, i, filePath, MAX_PATH))
					{
						// 查找源文件对应的树项
						HSTREEITEM hSrcItem = pFileAdapter->FindItemByPath(filePath);
						if(hSrcItem)
						{
							// 执行文件移动操作
							pFileAdapter->MoveItem(hSrcItem, hDestParentItem);
						}
					}
				}
				GlobalUnlock(medium.hGlobal);
			}
			ReleaseStgMedium(&medium);
			*pdwEffect = DROPEFFECT_MOVE;
		}
		else
		{
			*pdwEffect = DROPEFFECT_NONE;
		}
		return S_OK;
	}
};

class FileTreeFmtEnum: public SUnkImpl<IEnumFORMATETC>
{
	int m_nIndex;
	std::vector<FORMATETC> m_format;
public:
	FileTreeFmtEnum(const FORMATETC * fmts, int nCount):m_nIndex(0)
	{
		m_format.assign(fmts, fmts + nCount);
	}

	virtual ~FileTreeFmtEnum()
	{
	}

	IUNKNOWN_BEGIN(IEnumFORMATETC)
	IUNKNOWN_END()

	// IEnumFORMATETC methods
	STDMETHOD(Next)(ULONG celt, FORMATETC *rgelt, ULONG *pceltFetched) OVERRIDE{
		if(m_nIndex + celt > m_format.size())
		{
			return E_UNEXPECTED;
		}
		if(pceltFetched)
		{
			*pceltFetched = m_nIndex;
		}
		for(int i = 0; i < celt; i++)
		{
			rgelt[i] = m_format[m_nIndex + i];
		}
		m_nIndex += celt;
		return S_OK;
	}

	STDMETHOD(Skip)(ULONG celt) OVERRIDE
	{
		m_nIndex += celt;
		return S_OK;
	}

	STDMETHOD(Reset)() OVERRIDE
	{
		m_nIndex = 0;
		return S_OK;
	}

	STDMETHOD(Clone)(IEnumFORMATETC **ppEnumFormatEtc) OVERRIDE
	{
		*ppEnumFormatEtc = new FileTreeFmtEnum(m_format.data(), m_format.size());
		return S_OK;
	}
};

class FileTreeDataSource : public SUnkImpl<IDataObject>
{
private:
	HGLOBAL m_hGlobal; // 存储拖动数据的全局内存
	BOOL m_bOwnsMemory; // 是否拥有内存的所有权

public:
	FileTreeDataSource(HGLOBAL hGlobal, BOOL bOwnsMemory)
		: m_hGlobal(hGlobal), m_bOwnsMemory(bOwnsMemory)
	{
	}

	virtual ~FileTreeDataSource()
	{
		// 只在拥有内存所有权时才释放
		if (m_bOwnsMemory && m_hGlobal)
		{
			GlobalFree(m_hGlobal);
			m_hGlobal = NULL;
		}
	}

	IUNKNOWN_BEGIN(IDataObject)
	IUNKNOWN_END()

	// IDataObject methods
	STDMETHOD(GetData)(FORMATETC *pFormatEtc, STGMEDIUM *pMedium) OVERRIDE
	{
		if (!pFormatEtc || !pMedium)
			return E_INVALIDARG;

		// 检查是否支持 CF_HDROP 格式
		if (pFormatEtc->cfFormat == CF_HDROP && pFormatEtc->tymed == TYMED_HGLOBAL)
		{
			if (!m_hGlobal)
				return E_FAIL;

			// 填充 STGMEDIUM 结构
			pMedium->tymed = TYMED_HGLOBAL;
			size_t size = GlobalSize(m_hGlobal);
			pMedium->hGlobal = GlobalAlloc(GMEM_MOVEABLE, size);
			pMedium->pUnkForRelease = NULL;
			void *dst = GlobalLock(pMedium->hGlobal);
			memcpy(dst, GlobalLock(m_hGlobal), size);
			GlobalUnlock(m_hGlobal);
			GlobalUnlock(pMedium->hGlobal);

			return S_OK;
		}

		return DV_E_FORMATETC;
	}

	STDMETHOD(GetDataHere)(FORMATETC *pFormatEtc, STGMEDIUM *pMedium) OVERRIDE
	{
		if (!pFormatEtc || !pMedium)
			return E_INVALIDARG;

		// 检查是否支持 CF_HDROP 格式
		if (pFormatEtc->cfFormat == CF_HDROP && pFormatEtc->tymed == TYMED_HGLOBAL)
		{
			if (!m_hGlobal)
				return E_FAIL;
			// 填充 STGMEDIUM 结构
			pMedium->tymed = TYMED_HGLOBAL;
			size_t size = GlobalSize(m_hGlobal);
			pMedium->hGlobal = m_hGlobal;
			return S_OK;
		}

		return DV_E_FORMATETC;
	}

	STDMETHOD(QueryGetData)(FORMATETC *pFormatEtc) OVERRIDE
	{
		if (!pFormatEtc)
			return E_INVALIDARG;
		// 检查是否支持 CF_HDROP 格式
		if (pFormatEtc->cfFormat == CF_HDROP && pFormatEtc->tymed == TYMED_HGLOBAL)
			return S_OK;

		return DV_E_FORMATETC;
	}

	STDMETHOD(GetCanonicalFormatEtc)(FORMATETC *pFormatEtcIn, FORMATETC *pFormatEtcOut) OVERRIDE
	{
		if (!pFormatEtcIn || !pFormatEtcOut)
			return E_INVALIDARG;

		// 复制格式信息
		*pFormatEtcOut = *pFormatEtcIn;
		pFormatEtcOut->ptd = NULL;

		return S_OK;
	}

	STDMETHOD(SetData)(FORMATETC *pFormatEtc, STGMEDIUM *pMedium, BOOL fRelease) OVERRIDE
	{
		return S_OK;
	}

	STDMETHOD(EnumFormatEtc)(DWORD dwDirection, IEnumFORMATETC **ppEnumFormatEtc) OVERRIDE
	{
		if (dwDirection == DATADIR_GET)
		{ 
			FORMATETC fmts[] = {
				{ CF_HDROP, NULL, DVASPECT_CONTENT, -1, TYMED_HGLOBAL },
			};
			*ppEnumFormatEtc = new FileTreeFmtEnum(fmts, ARRAYSIZE(fmts));
			return S_OK;
		}
		return E_NOTIMPL;
	}

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
    static HBITMAP CreateDragBitmap(const std::vector<SStringW> &filePaths)
    {
        if (filePaths.empty())
            return NULL;

        // 计算显示的文件数量
        int displayCount = smin((int)filePaths.size(), 3);

        // 计算位图大小
        int bitmapWidth = 220;
        int lineHeight = 18;
        int bitmapHeight = 10 + displayCount * lineHeight + 10 + (filePaths.size() > 3 ? lineHeight : 0);

        // 获取 SOUI 的渲染工厂
        IRenderFactory *pRenderFactory = GETRENDERFACTORY;
        if (!pRenderFactory)
            return NULL;

        SAutoRefPtr<IRenderTarget> pRenderTarget;
        // 为位图创建渲染目标
        HRESULT hr = pRenderFactory->CreateRenderTarget(&pRenderTarget, bitmapWidth, bitmapHeight);
        if (FAILED(hr))
            return NULL;
        pRenderTarget->BeginDraw();
        CRect rc(0, 0, bitmapWidth, bitmapHeight);
        // 绘制背景（半透明白色）
        // SOUI 使用 RGBA 格式：(Alpha << 24) | (B << 16) | (G << 8) | R
        // 使用预乘Alpha的颜色
        COLORREF crBgColor = RGBA(200, 255, 255, 200); // Alpha=200, RGB=(255,255,200)
        pRenderTarget->ClearRect(rc, crBgColor);

        // 创建边框颜色
        COLORREF crBorderColor = RGBA(180, 0, 0, 0); // Alpha=180, Black

        // 绘制边框
        SAutoRefPtr<IPenS> pBorderPen;
        pRenderTarget->CreatePen(PS_SOLID, crBorderColor, 1, &pBorderPen);
        if (pBorderPen)
        {
            CRect rcFrame;
            rcFrame.SetRect(0, 0, bitmapWidth - 1, bitmapHeight - 1);
            IPenS *pOldPen = NULL;
            pRenderTarget->SelectObject(pBorderPen, (IRenderObj **)&pOldPen);
            pRenderTarget->DrawRectangle(&rcFrame);
            pRenderTarget->SelectObject(pOldPen);
        }

        // 创建文字颜色和字体
        COLORREF crTextColor = RGBA(0, 0, 0, 255); // 黑色文字

        // 获取字体
        SAutoRefPtr<IFontS> pFont;
        LOGFONT lf = { 0 };
        lf.lfHeight = -10; // 10pt
        _tcscpy_s(lf.lfFaceName, LF_FACESIZE, _T("宋体"));
        pRenderFactory->CreateFont(&pFont, &lf);

        // 绘制文件名列表
        if (pFont)
        {
            IFontS *pOldFont = NULL;
            pRenderTarget->SelectObject(pFont, (IRenderObj **)&pOldFont);
            COLORREF crOld = pRenderTarget->SetTextColor(crTextColor);
            for (int i = 0; i < displayCount; i++)
            {
                // 获取文件名（只显示最后一部分）
                const SStringW &fullPath = filePaths[i];
                int pos = fullPath.ReverseFind(L'/');
                SStringW fileName = (pos >= 0) ? fullPath.Mid(pos + 1) : fullPath;

                // 截断过长的文件名
                if (fileName.GetLength() > 25)
                {
                    fileName = fileName.Left(22) + L"...";
                }

                // 计算文本位置
                int y = 10 + i * lineHeight;
                CRect rcText(15, y, bitmapWidth - 10, y + lineHeight);
				SStringT fileNameT = S_CW2T(fileName);
                // 绘制文字
                pRenderTarget->DrawText(fileNameT, -1, &rcText, DT_LEFT | DT_VCENTER);
            }
            // 如果有更多文件，显示省略号
            if (filePaths.size() > 3)
            {
                int y = 10 + displayCount * lineHeight;
                CRect rcText(15, y, bitmapWidth - 10, y + lineHeight);
                pRenderTarget->DrawText(_T("+ 更多文件..."), -1, &rcText, DT_LEFT | DT_VCENTER);
            }
            pRenderTarget->SelectObject(pOldFont);
            pRenderTarget->SetTextColor(crOld);
        }
        pRenderTarget->EndDraw();

		HBITMAP hBmp = CreateBitmap(bitmapWidth, bitmapHeight, 1, 32, NULL);
		HDC hdc = GetDC(0);
		HDC hMemDC = CreateCompatibleDC(hdc);
		::SelectObject(hMemDC, hBmp);
		HDC hdcSrc = pRenderTarget->GetDC(0);
		::BitBlt(hMemDC, 0, 0, bitmapWidth, bitmapHeight, hdcSrc, 0, 0, SRCCOPY);
		pRenderTarget->ReleaseDC(hdcSrc, NULL);
		DeleteDC(hMemDC);
		ReleaseDC(0, hdc);
		return hBmp;
    }

	void SetDragImage(HBITMAP hBitmap, POINT ptHot)
	{
		m_hDragBitmap = hBitmap;
		m_ptHot = ptHot;
	}

	IUNKNOWN_BEGIN(IDropSource)
	IUNKNOWN_END()

	void StopDrag()
    {
        if (m_bDragStarted)
        {
            SDragWnd::EndDrag();
            m_bDragStarted = false;
        }
    }
    // IDropSource methods
    STDMETHOD(QueryContinueDrag)(BOOL fEscapePressed, DWORD grfKeyState) OVERRIDE
	{
        if (!m_bDragStarted)
            return S_OK;
		//SLOGI()<<"QueryContinueDrag, fEscapePressed="<<fEscapePressed<<" grfKeyState="<<grfKeyState;
		if(fEscapePressed)
		{
			// 结束拖动窗口
            StopDrag();
			return DRAGDROP_S_CANCEL;
		}
		if(!(grfKeyState & MK_LBUTTON))
		{
			// 结束拖动窗口
            StopDrag();
			return DRAGDROP_S_DROP;
		}

		// 更新拖动窗口位置
		if (m_bDragStarted)
		{
			POINT pt;
			GetCursorPos(&pt);
			SDragWnd::DragMove(pt);
		}

		return S_OK;
	}

    STDMETHOD(GiveFeedback)(DWORD dwEffect) OVERRIDE
	{
		//SLOGI()<<"GiveFeedback, dwEffect"<<dwEffect;

		// 第一次调用时启动拖动窗口
		if (!m_bDragStarted && m_hDragBitmap)
		{
			if (SDragWnd::BeginDrag(m_hDragBitmap, m_ptHot, CLR_NONE, 200, LWA_ALPHA))
			{
				m_bDragStarted = true;
				POINT pt;
				GetCursorPos(&pt);
				SDragWnd::DragMove(pt);
			}
		}
		return DRAGDROP_S_USEDEFAULTCURSORS;
	}
};

#endif//__FILETREE_DRAGDROP_H__