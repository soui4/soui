#ifndef _SIMAGE_KNOB_H_
#define _SIMAGE_KNOB_H_

SNSBEGIN

DEF_EVT_EXT(EventKnobValue, EVT_EXTERNAL_BEGIN + 400, { float value; })

class SImageKnob : public SWindow
{
	struct MouseEditingState;
	DEF_SOBJECT(SWindow,L"imgknob")
public:
	enum KnobMode
	{
		kCircularMode = 0,
		kRelativCircularMode,
		kLinearMode
	};

	SImageKnob();
	~SImageKnob();

	float getValue(bool bRaw=false) const;
	void setValue(float v);

	float getRange(bool bRaw = false) const;
	void setRange(float vMin, float vMax);

	void setMin(float v);
	float getMin(bool bRaw = false) const;
	void setMax(float v);
	float getMax(bool bRaw = false) const;


protected:
	MouseEditingState& getMouseEditingState();

	void bounceValue();

	void OnPaint(IRenderTarget* pRT);

	BOOL OnEraseBkgnd(IRenderTarget* pRT);

	void OnLButtonDown(UINT nFlags, CPoint pt);

	float valueFromPoint(CPoint& point) const;

	void OnLButtonUp(UINT nFlags, CPoint pt);

	void OnMouseMove(UINT nFlags, CPoint pt);

	SOUI_MSG_MAP_BEGIN()
		MSG_WM_PAINT_EX(OnPaint)
		MSG_WM_LBUTTONDOWN(OnLButtonDown)
		MSG_WM_LBUTTONUP(OnLButtonUp)
		MSG_WM_MOUSEMOVE(OnMouseMove)
		MSG_WM_ERASEBKGND_EX(OnEraseBkgnd)
	SOUI_MSG_MAP_END()
public:
	HRESULT OnAttrStartAngle(const SStringW& value, BOOL bLoading);
	HRESULT OnAttrRangeAngle(const SStringW& value, BOOL bLoading);

	SOUI_ATTRS_BEGIN()
		ATTR_FLOAT(L"min", m_min, TRUE)
		ATTR_FLOAT(L"max", m_max, TRUE)
		ATTR_FLOAT(L"value", m_value, TRUE)
		ATTR_CUSTOM(L"startAngle", OnAttrStartAngle)
		ATTR_CUSTOM(L"rangeAngle", OnAttrRangeAngle)
		ATTR_COLOR(L"colorRulerBack",m_crRulerBack,TRUE)
		ATTR_COLOR(L"colorRulerForeEnable", m_crRulerForeEnable, TRUE)
		ATTR_COLOR(L"colorRulerForeDisable", m_crRulerForeDisable, TRUE)
		ATTR_BOOL(L"logTransform",m_bLogTransform,TRUE)
		ATTR_BOOL(L"startFromCenter", m_startFromCenter,TRUE)
		ATTR_BOOL(L"startFromRight", m_startFromRight, TRUE)
	SOUI_ATTRS_END()

protected:
	float m_startAngle, m_rangeAngle;
	float m_rulerWidth;
	COLORREF m_crRulerBack;
	COLORREF m_crRulerForeEnable;
	COLORREF m_crRulerForeDisable;

	bool  m_bLogTransform;
	bool m_startFromCenter;
	bool m_startFromRight;
	float m_min, m_max, m_value;

	MouseEditingState* m_state;
};

SNSEND
#endif//_SIMAGE_KNOB_H_

