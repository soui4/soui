/**
* Copyright (C) 2014-2050 SOUI团队
* All rights reserved.
*
* @file       SMCListViewEx.h
* @brief
* @version    v2.0
* @author     soui
* @date       2020-04-03
*
* Describe    MCListViewEx控件
*/
#pragma once
#include "core/SPanel.h"
#include "core/SItemPanel.h"
#include "control/SHeaderCtrl.h"
#include "helper/SAdapterBase.h"


#if _MSC_VER <1600
#define override
#endif
namespace SOUI
{
	interface IMcAdapterEx : public IMcAdapter
	{
		//按下ctrl时，可以是选中也可能是取消选中
		virtual void ControlSel(long idx) = 0;
		virtual void ShiftSel(long idx) = 0;
		virtual void ClearSel() = 0;
		//清除比索引大的选中项
		virtual void ClearSel(long idx) = 0;
		//清除指定项
		virtual void DelSel(long idx) = 0;
		virtual void SelAll() = 0;
		virtual void ClickSel(long idx) = 0;
		virtual bool IsSel(long idx) = 0;
		virtual bool IsOnlySel(long idx) = 0;
		//virtual bool CanSwap(long idx) = 0;
		virtual void SelectionStart() = 0;
		virtual void SelectionReady(long idx) = 0;
		virtual void UpataSel(long idx) = 0;
		virtual void SelectionEnd() = 0;
		virtual void SwapEnd(long idx) = 0;
		//因为不可预知实际数据存储方式应该在实际的适配器上实现
		virtual bool SwapItem(long pos, SList<long>&) = 0;
	public:
		virtual SList<long>& GetSelList() = 0;
	};

#undef INTERFACE
#define INTERFACE IMcListViewEx
DEFINE_GUID(IID_IMcListViewEx,
    0x15398C65, 0x1A58, 0x4074, 0xA5, 0x83, 0xF8, 0x26, 0xF8, 0x21, 0xA8, 0x2F);
DECLARE_INTERFACE_(IMcListViewEx, IObjRef)
{
    DECLARE_CLASS_SIID(IID_IMcListViewEx)
	private:

		STDMETHOD_(BOOL, SetAdapter)(THIS_ IMcAdapter * adapter) { return FALSE; }

		STDMETHOD_(IMcAdapter*, GetAdapter)(THIS) SCONST { return NULL; }

	public:

		STDMETHOD_(BOOL, SetAdapter)(THIS_ IMcAdapterEx * adapter) PURE;

		STDMETHOD_(IMcAdapterEx*, GetAdapterEx)(THIS) SCONST PURE;
	};


	class SMcAdapterBaseEx : public TObjRefImpl<LvAdatperImpl<IMcAdapterEx>>
	{
	protected:
		SList<long> m_selList;
		long m_selFirstIdx;
		//上次的位置
		long m_selLastIdx;
	public:
		SMcAdapterBaseEx()
		{
			//m_selList.AddHead(-1);
		}

		~SMcAdapterBaseEx()
		{
		}

		virtual SList<long>& GetSelList()override
		{
			return m_selList;
		}

		virtual void ClearSel(long idx)override
		{
			SPOSITION pos = m_selList.GetHeadPosition();
			while (pos)
			{
				const long& position = m_selList.GetAt(pos);
				SPOSITION nextpos = m_selList.Next(pos);
				if (position > idx)
					m_selList.RemoveAt(pos);
				pos = nextpos;
			}
		}

		virtual void DelSel(long idx)override
		{
			SPOSITION pos = m_selList.Find(idx);
			if (pos)
			{
				m_selList.RemoveAt(pos);
				pos = m_selList.GetHeadPosition();
				//比它索引大的都减1
				while (pos)
				{
					long& position = m_selList.GetNext(pos);
					if (position > idx)
					{
						--position;
					}
				}
			}
		}

		virtual bool SwapItem(long pos, SList<long>&)override { return false; }

		virtual void SelectionReady(long idx)override
		{
			m_selLastIdx = m_selFirstIdx = idx;
		}

		virtual void SelectionStart()override
		{
			m_selList.RemoveAll();
			m_selList.AddHead(m_selLastIdx);
			notifyDataSetInvalidated();
		}

		virtual void UpataSel(long idx)override
		{
			if (idx == -1)
				return;
			if (m_selList.GetHead() == -1)
			{
				if (idx < getCount() - 1)
				{
					m_selList.SetAt(m_selList.GetHeadPosition(), getCount() - 1);
					goto UPDATASEL;
				}
				else m_selList.SetAt(m_selList.GetHeadPosition(), idx);
			}
			else
			{
			UPDATASEL:
				//第一个选中项
				long relFirstIdx = m_selFirstIdx != -1 ? m_selFirstIdx : m_selList.GetHead();
				m_selList.RemoveAll();
				if (relFirstIdx == idx)
				{
					m_selList.AddTail(idx);
				}
				else if (relFirstIdx > idx)
				{
					//从下往上选
					for (long i = relFirstIdx; i >= idx; i--)
					{
						m_selList.AddTail(i);
					}
				}
				else
				{
					//从上往下选				
					for (long i = relFirstIdx; i <= idx; i++)
					{
						m_selList.AddTail(i);
					}
				}
			}
			notifyDataSetInvalidated();
		}

		virtual void SelectionEnd()override
		{}

		virtual void SwapEnd(long idx)override
		{
			if (idx == -1)return;
			if (IsSel(idx))return;
			if (SwapItem(idx, m_selList))
				notifyDataSetInvalidated();
		}

		virtual void ClickSel(long idx)override
		{
			m_selList.RemoveAll();
			m_selList.AddTail(idx);
			this->notifyDataSetInvalidated();
		}

		virtual bool IsSel(long idx)override
		{
			SPOSITION pos = m_selList.Find(idx);
			return (pos != NULL);
		}
		virtual bool IsOnlySel(long idx)
		{
			return (m_selList.GetCount() == 1) && IsSel(idx);
		}
		virtual void ControlSel(long idx)override
		{
			SPOSITION pos = m_selList.Find(idx);
			if (pos != NULL)
			{
				m_selList.RemoveAt(pos);
			}
			else
			{
				m_selList.AddTail(idx);
			}
			notifyItemDataChanged(idx);
		}

		virtual void ShiftSel(long idx)override
		{
			if (m_selList.IsEmpty())
				return;
			long lastidx = m_selList.GetTail();
			m_selList.RemoveAll();
			if (lastidx > idx)
			{
				//从下往上选
				for (long i = lastidx; i >= idx; i--)
				{
					m_selList.AddTail(i);
				}
			}
			else
			{
				//从上往下选				
				for (long i = lastidx; i <= idx; i++)
				{
					m_selList.AddTail(i);
				}
			}
			this->notifyDataSetInvalidated();
		}

		virtual void ClearSel()override
		{
			m_selList.RemoveAll();
		}

		virtual void SelAll()override {}

	public:

		STDMETHOD_(BOOL, OnSort)(int iCol, UINT* pFmts, int nCols) override { return FALSE; }

		STDMETHOD_(SStringW, GetColumnName)(int iCol) SCONST PURE;

		STDMETHOD_(void, GetColumnName)(THIS_ int iCol, IStringW* pName) SCONST
		{
			SStringW strName = GetColumnName(iCol);
			pName->Copy(&strName);
		}

		STDMETHOD_(BOOL, IsColumnVisible)(THIS_ int iCol) SCONST OVERRIDE
		{
			(iCol);
			return TRUE;
		}
		STDMETHOD_(void,SetColumnsWidth)(THIS_ int *pColWid,int nCols) OVERRIDE{
			(pColWid);
			(nCols);
		}
	};
	//////////////////////////////////////////////////////////////////////////
	//  SMCListViewEx
	class SMCListViewEx : public TPanelProxy<IMcListViewEx>
		, protected SHostProxy
		, protected IItemContainer {

		DEF_SOBJECT(SPanel, L"mclistviewex")

		friend class SMCListViewDataSetObserverEx;

	public:

		SMCListViewEx();

		virtual ~SMCListViewEx();

	public:

		STDMETHOD_(BOOL, SetAdapter)(THIS_ IMcAdapterEx* adapter) OVERRIDE;

		STDMETHOD_(IMcAdapterEx*, GetAdapterEx)(THIS) SCONST OVERRIDE;

		STDMETHOD_(IListViewItemLocator*, GetItemLocator)(THIS) SCONST ;

		STDMETHOD_(void, SetItemLocator)(THIS_ IListViewItemLocator* pItemLocator) ;

		STDMETHOD_(void, EnsureVisible)(THIS_ int iItem) ;

		STDMETHOD_(void, SetSel)(THIS_ int iItem, BOOL bNotify = FALSE) ;


		STDMETHOD_(IItemPanel*, HitTest)(THIS_ const POINT* pt) SCONST ;

		STDMETHOD_(IHeaderCtrl*, GetIHeaderCtrl)(THIS) SCONST ;

		STDMETHOD_(int, InsertColumn)
			(THIS_ int nIndex,
				LPCTSTR pszText,
				int nWidth,
				UINT fmt,
				LPARAM lParam = 0,
				BOOL bDpiAware = TRUE,
				float fWeight = 0.0f) ;

		STDMETHOD_(void, DeleteColumn)(THIS_ int iCol) ;

		STDMETHOD_(int, GetColumnCount)(THIS) SCONST ;

	protected:
		//禁止调用
		STDMETHOD_(int, GetSel)(THIS) SCONST  { return -1; }

	public:

		SItemPanel* HitTest(CPoint& pt) const;

		SHeaderCtrl* GetHeaderCtrl() const;

		void UpdateVisibleItems();

		void UpdateVisibleItem(int iItem);

	protected:

		virtual void OnItemSetCapture(SOsrPanel* pItem, BOOL bCapture);

		virtual BOOL OnItemGetRect(const SOsrPanel* pItem, CRect& rcItem)const;

		virtual BOOL IsItemRedrawDelay()const;

		virtual void OnItemRequestRelayout(SItemPanel* pItem);

	protected:

		void onDataSetChanged();

		void onDataSetInvalidated();

		void onItemDataChanged(int iItem);

	protected:

		BOOL OnItemClick(IEvtArgs* pEvt);

	protected:
		virtual BOOL OnScroll(BOOL bVertical, UINT uCode, int nPos)override;

		virtual int  GetScrollLineSize(BOOL bVertical)override;

		virtual BOOL CreateChildren(SXmlNode xmlNode)override;

		virtual BOOL UpdateToolTip(CPoint pt, SwndToolTipInfo& tipInfo)override;

		virtual UINT WINAPI OnGetDlgCode();

		virtual BOOL OnSetCursor(const CPoint& pt)override;

		virtual void OnColorize(COLORREF cr)override;

		virtual void OnScaleChanged(int nScale)override;

		virtual HRESULT OnLanguageChanged()override;

		void OnShowWindow(BOOL bShow, UINT nStatus);

		virtual void OnRebuildFont();
	protected:
		void DispatchMessage2Items(UINT uMsg, WPARAM wParam, LPARAM lParam);

		void UpdateScrollBar();

		void RedrawItem(SOsrPanel* pItem);

		SItemPanel* GetItemPanel(int iItem);

		void OnPaint(IRenderTarget* pRT);

		void OnSize(UINT nType, CSize size);

		void OnDestroy();

		LRESULT OnMouseEvent(UINT uMsg, WPARAM wParam, LPARAM lParam);

		LRESULT OnKeyEvent(UINT uMsg, WPARAM wParam, LPARAM lParam);

		void OnKeyDown(TCHAR nChar, UINT nRepCnt, UINT nFlags);

		void OnMouseLeave();

		BOOL OnMouseWheel(UINT nFlags, short zDelta, CPoint pt);

		void OnKillFocus(SWND wndFocus);

		void OnSetFocus(SWND wndOld);

		void OnTimer(char cTimerID);

		SOUI_MSG_MAP_BEGIN()
			MSG_WM_PAINT_EX(OnPaint)
			MSG_WM_SIZE(OnSize)
			MSG_WM_DESTROY(OnDestroy)
			MSG_WM_MOUSEWHEEL(OnMouseWheel)
			MSG_WM_MOUSELEAVE(OnMouseLeave)
			MSG_WM_KEYDOWN(OnKeyDown)
			MSG_WM_SETFOCUS_EX(OnSetFocus)
			MSG_WM_TIMER_EX(OnTimer)
			MSG_WM_KILLFOCUS_EX(OnKillFocus)
			MSG_WM_SHOWWINDOW(OnShowWindow)
			MESSAGE_RANGE_HANDLER_EX(WM_MOUSEFIRST, WM_MOUSELAST, OnMouseEvent)
			MESSAGE_RANGE_HANDLER_EX(WM_KEYFIRST, WM_KEYLAST, OnKeyEvent)
			MESSAGE_RANGE_HANDLER_EX(WM_IME_STARTCOMPOSITION, WM_IME_KEYLAST, OnKeyEvent)
			MESSAGE_HANDLER_EX(WM_IME_CHAR, OnKeyEvent)
			MESSAGE_HANDLER_EX(WM_IME_REQUEST, OnKeyEvent)
			SOUI_MSG_MAP_END()

			SOUI_ATTRS_BEGIN()
			ATTR_LAYOUTSIZE(L"headerHeight", m_nHeaderHeight, FALSE)
			ATTR_INT(L"hotTrack", m_bHotTrack, FALSE)
			ATTR_SKIN(L"dividerSkin", m_pSkinDivider, TRUE)
			ATTR_LAYOUTSIZE(L"dividerSize", m_nDividerSize, FALSE)
			ATTR_INT(L"wantTab", m_bWantTab, FALSE)
			ATTR_COLOR(L"colorGrid", m_crGrid, TRUE)
			ATTR_INT(L"canSwapItem", m_bCanSwapItem, FALSE)
			ATTR_COLOR(L"colorSwapLine", m_colorSwapLine, FALSE)
			ATTR_COLOR(L"colorDropBk", m_colorDropBk, FALSE)
			ATTR_INT(L"dropBkAlpha", m_DropBkAlpha, FALSE)
			SOUI_ATTRS_END()

	protected:
		SAutoRefPtr<IMcAdapterEx>			m_adapter;
		SAutoRefPtr<ILvDataSetObserver>		m_observer;
		SAutoRefPtr<IListViewItemLocator>	m_lvItemLocator;//列表项定位接口
		struct ItemInfo
		{
			SItemPanel* pItem;
			int nType;
		};
		bool							m_bPendingUpdate;//response for data set changed in OnShowWindow.
		int								m_iPendingUpdateItem; //-1 for all. -2 for nothing
		int								m_iSelItem;
		int                             m_iFirstVisible;//第一个显示项索引
		SList<ItemInfo>                 m_lstItems; //当前正在显示的项
		SOsrPanel* m_itemCapture;//The item panel that has been set capture.		
		SOsrPanel* m_pHoverItem;
		CPoint							m_dropStartPos, m_dropEndPos;
		bool							m_bDrop, m_bDroping, m_bSwap, m_bSwaped, m_bSwapToTop;
		LPARAM							oldLoc;
		COLORREF						m_colorSwapLine, m_colorDropBk;
		bool							m_bOutLeft, m_bOutRight, m_bOutTop, m_bOutBottom;

		SArray<SList<SItemPanel*>*>		m_itemRecycle;//item回收站,每一种样式在回收站中保持一个列表，以便重复利用

		SXmlDoc							m_xmlTemplate;
		SAutoRefPtr<ISkinObj>			m_pSkinDivider;
		SLayoutSize						m_nDividerSize;
		BOOL                            m_bWantTab;
		BOOL                            m_bDatasetInvalidated;
		BOOL							m_bCanSwapItem;
		int								m_DropBkAlpha;
		COLORREF						m_crGrid;
	protected:

		/**
		* SMCListViewEx::OnHeaderClick
		* @brief    列表头单击事件 --
		* @param    EventArgs *pEvt
		*
		* Describe  列表头单击事件
		*/
		BOOL OnHeaderClick(IEvtArgs* pEvt);
		/**
		* SMCListViewEx::OnHeaderSizeChanging
		* @brief    列表头大小改变
		* @param    EventArgs *pEvt --
		*
		* Describe  列表头大小改变
		*/
		BOOL OnHeaderSizeChanging(IEvtArgs* pEvt);
		/**
		* SMCListViewEx::OnHeaderSwap
		* @brief    列表头交换
		* @param    EventArgs *pEvt --
		*
		* Describe  列表头交换
		*/
		BOOL OnHeaderSwap(IEvtArgs* pEvt);

		/**
		* SMCListViewEx::GetListRect
		* @brief    获取list位置
		* @return   返回CRect
		*
		* Describe  获取list位置
		*/
		CRect GetListRect();

		/**
		* SMCListViewEx::UpdateHeaderCtrl
		* @brief    更新列表头控件
		*
		* Describe  更新列表头控件
		*/
		void UpdateHeaderCtrl();

		STDMETHOD_(void, UpdateChildrenPosition)();

		int GetHeaderHeight() const;

		CRect _OnItemGetRect(int iPosition)const;

	protected:

		SHeaderCtrl* m_pHeader;
		SLayoutSize		m_nHeaderHeight;  /**< 列表头高度 */
		BOOL            m_bHotTrack;  /**<  */
	};

}//end of namespace
