#pragma once
namespace SOUI
{
	#define EVT_TB_ITEMCLICK (EVT_EXTERNAL_BEGIN+500)

	DEF_EVT_EXT(EventToolBarCmd,EVT_TB_ITEMCLICK,{
		int iItem;
		int nCmdId;
		SStringT strText;
		LPARAM lParam;
	});


	class SToolBar : public SWindow
	{
		DEF_SOBJECT(SWindow,L"toolbar")
	public:
		SToolBar(void);
		~SToolBar(void);

	public:
		struct ToolBarItem
		{
			int nId;
			int iIcon;
			LPARAM lParam;
			SStringT strText;
			SStringT strTip;
			int dwState;
			SAutoRefPtr<IBitmapS> icon;
		};

		void InsertItem(const ToolBarItem & item,int nPos=-1);
		void SetIconsSkin(SAutoRefPtr<ISkinObj> skinIcons);
		BOOL SetItemCheck(int nItemId,BOOL bCheck);
	public:
		SOUI_ATTRS_BEGIN()
			ATTR_SKIN(L"iconSkin",m_skinIcons,TRUE)
			ATTR_SKIN(L"sepSkin",m_skinSep,TRUE)
			ATTR_SKIN(L"stateSkin",m_skinState,TRUE)
			ATTR_SKIN(L"dropArrowSkin",m_skinDropArrow,TRUE)
			ATTR_BOOL(L"showText",m_bShowText,TRUE)
			ATTR_INT(L"itemSize",m_nItemSize,TRUE)
			ATTR_INT(L"dropDownSize",m_nDropDownSize,TRUE)
			ATTR_BOOL(L"vertical",m_bVert,TRUE)
		SOUI_ATTRS_END()

	protected:
		virtual void DrawItem(IRenderTarget *pRT,const CRect &rc,const ToolBarItem * pItem);
		virtual void DrawDropButton(IRenderTarget *pRT,const CRect &rc,DWORD dwState);
		virtual BOOL CreateChildren(pugi::xml_node xmlNode);
		virtual SIZE MeasureContent(int nParentWid,int nParentHei);
		virtual BOOL OnUpdateToolTip(CPoint pt, SwndToolTipInfo &tipInfo);
	protected:
		void OnPaint(IRenderTarget *pRT);
		void OnLButtonDown(UINT nFlags,CPoint pt);
		void OnLButtonUp(UINT nFlags,CPoint pt);
		void OnMouseMove(UINT nFlags,CPoint pt);
		void OnMouseLeave();
		void OnSize(UINT nType, CSize size);

		SOUI_MSG_MAP_BEGIN()
			MSG_WM_LBUTTONDOWN(OnLButtonDown)
			MSG_WM_LBUTTONUP(OnLButtonUp)
			MSG_WM_MOUSEMOVE(OnMouseMove)
			MSG_WM_MOUSELEAVE(OnMouseLeave)
			MSG_WM_PAINT_EX(OnPaint)
			MSG_WM_SIZE(OnSize)
		SOUI_MSG_MAP_END()

	protected:
		CSize GetItemSize() const;
		int   GetSepWid() const;
		CRect GetItemRect(int iItem) const;
		int   HitTest(CPoint pt) const;
		BOOL  IsSeparator(const ToolBarItem * pItem) const;
		void  UpdateVisibleItemCount();
		BOOL HasDropDownButton() const;
		CRect GetDropdownButtonRect() const;
		void ShowDropDownMenu(const CRect &rc);
		int  GetItemByID(int nID) const;
	protected:
		SAutoRefPtr<ISkinObj> m_skinState;
		SAutoRefPtr<ISkinObj> m_skinIcons;
		SAutoRefPtr<ISkinObj> m_skinSep;
		SAutoRefPtr<ISkinObj> m_skinDropArrow;
		BOOL				  m_bShowText;
		int					  m_nItemSize;
		SXmlDoc	  m_menuStyle;
		SArray<ToolBarItem> m_arrItems;

		int					m_iClickItem;
		int					m_iHoverItem;
		BOOL				m_bVert;
		int					m_nVisibleItems;
		int					m_nDropDownSize;
		DWORD				m_dwDropBtnState;
	};

}

