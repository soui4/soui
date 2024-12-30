/**
 * Copyright (C) 2014-2050 SOUI团队
 * All rights reserved.
 *
 * @file       SHeaderCtrlEx.h
 * @brief
 * @version    v1.0
 * @author     008
 * @date       2017-01-07
 *
 * Describe
 */
#pragma once


#include "control/SHeaderCtrl.h"
namespace SOUI
{
	/**
	 * @class     SHeaderCtrlEx
	 * @brief     表头控件
	 *
	 * Describe   表头控件
	 */
	class SHeaderCtrlEx : public SHeaderCtrl
	{
		DEF_SOBJECT(SHeaderCtrl, L"header2")
	public:
		SHeaderCtrlEx(void);
		bool isViewWidthMatchParent() const;
		//第二个参数表明是否拉伸
		void Move(LPCRECT prect, bool _newSize);

		SOUI_ATTRS_BEGIN()
			ATTR_COLOR(L"SplitLineColor", m_crLine, FALSE)
			ATTR_COLOR(L"SplitLineOffset", m_Offset.x= m_Offset.y, FALSE)
			ATTR_COLOR(L"SplitLineOffsetTop", m_Offset.x, FALSE)
			ATTR_COLOR(L"SplitLineOffsetBottom", m_Offset.y, FALSE)
			ATTR_COLOR(L"SplitLineColor", m_crLine, FALSE)
			ATTR_BOOL(L"DrawSplitLine", m_bDrawSplitLine, FALSE)
			ATTR_INT(L"matchParent", m_bMatchParent, FALSE)
		SOUI_ATTRS_END()

	protected:
		//添加一个函数支持同父窗口变化自身大小，些时和width按照它们的值等比例化分
		
		void UpdataWidth();
		void OnSize(UINT nType, CSize size);

		virtual BOOL CreateChildren(SXmlNode xmlNode);
		virtual BOOL OnSetCursor(const CPoint& pt);
		
		void OnPaint(IRenderTarget* pRT);

		SOUI_MSG_MAP_BEGIN()
			MSG_WM_SIZE(OnSize)
			MSG_WM_PAINT_EX(OnPaint)
		SOUI_MSG_MAP_END()

		SArray<int> m_widItems; /**< 列表项集合 */
		BOOL m_bMatchParent;
		bool m_bNewSize;
		COLORREF    m_crLine;
		BOOL		m_bDrawSplitLine;
		CPoint		m_Offset;
	};
}//end of namespace SOUI
