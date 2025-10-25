#ifndef _SIZINGFRAME_H_
#define _SIZINGFRAME_H_

#include <core/SWnd.h>

SNSBEGIN

#ifndef SF_EVT_BEGIN
#define SF_EVT_BEGIN (EVT_EXTERNAL_BEGIN + 2000)
#endif//SF_EVT_BEGIN

enum{
	EVT_SF_FRAME_MOVED = SF_EVT_BEGIN,
};

typedef enum _LayoutType{
	UnknownLayout=0,
	AnchorLayout,
	GridLayout,
	LinearLayout,
}LayoutType;

DEF_EVT_EXT(EventFrameMoved, EVT_SF_FRAME_MOVED, {
	SWND hTarget;
	LayoutType layoutType;
	CRect rcOld;
	CRect rcNew;
	SStringW strPos;
})

// 用于在设计器中提供调整窗口大小功能的框架控件
class  SSizingFrame : public SWindow
{
	DEF_SOBJECT(SWindow, L"sizingframe")
	
public:
	SSizingFrame();
	~SSizingFrame();

	void SetTarget(SWND hTarget);
protected:
    STDMETHOD_(BOOL, IsContainPoint)(THIS_ POINT pt, BOOL bClientOnly) SCONST OVERRIDE;

	void OnPaint(IRenderTarget *pRT);
	
	// 鼠标事件处理
	void OnLButtonDown(UINT nFlags, CPoint point);
	void OnLButtonUp(UINT nFlags, CPoint point);
	void OnMouseMove(UINT nFlags, CPoint point);
	void OnMouseLeave();
	
	// 确定鼠标位置对应的热点区域
	int HitTest(CPoint pt);
	
	// 获取框架各部分的矩形区域
	CRect GetBorderRect(int nBorder) const;
	CRect GetCornerRect(int nCorner) const;
	CRect GetCenterRect() const;
	
	BOOL OnSetCursor(const CPoint &pt) override;
	
	// 消息映射
	SOUI_MSG_MAP_BEGIN()
		MSG_WM_PAINT_EX(OnPaint)
		MSG_WM_LBUTTONDOWN(OnLButtonDown)
		MSG_WM_LBUTTONUP(OnLButtonUp)
		MSG_WM_MOUSEMOVE(OnMouseMove)
		MSG_WM_MOUSELEAVE(OnMouseLeave)
	SOUI_MSG_MAP_END()
	
	// 属性
	SOUI_ATTRS_BEGIN()
		ATTR_INT(L"borderSize", m_nBorderSize, FALSE)     // 边框大小
		ATTR_INT(L"cornerSize", m_nCornerSize, FALSE)     // 角落大小
		ATTR_COLOR(L"frameColor", m_crFrame, FALSE)       // 框架颜色
	SOUI_ATTRS_END()
	
protected:
	BOOL		m_bDragging;     // 是否正在拖拽
	int			m_nHitTest;      // 当前鼠标热点位置
	CPoint		m_ptDragStart;   // 拖拽起始点
	CRect		m_rcDragStart;   // 拖拽起始矩形
	
	// 属性值
	int			m_nBorderSize;   // 边框大小
	int			m_nCornerSize;   // 角落大小
	COLORREF	m_crFrame;       // 框架颜色
	
	// 热点区域常量
	enum {
		HT_UNKNOWN = 0,
		HT_LEFT = 1,
		HT_TOP = 2,
		HT_RIGHT = 3,
		HT_BOTTOM = 4,
		HT_TOPLEFT = 5,
		HT_TOPRIGHT = 6,
		HT_BOTTOMRIGHT = 7,
		HT_BOTTOMLEFT = 8,
		HT_CENTER = 9
	};

	SWND m_hTarget;
};

SNSEND

#endif // _SIZINGFRAME_H_