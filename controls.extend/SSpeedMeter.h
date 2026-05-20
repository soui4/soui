#ifndef _SPEED_METER_H_
#define _SPEED_METER_H_
#include <valueAnimator/SValueAnimator.h>

SNSBEGIN
class SSpeedMeter : public SWindow, protected IAnimatorUpdateListener
{
	DEF_SOBJECT(SWindow, L"SpeedMeter")
public:
	SSpeedMeter();
	~SSpeedMeter();

	void SetValue(float value);
	float GetValue() const;
protected:
	void OnPaint(IRenderTarget* pRT);

	STDMETHOD_(void, onAnimationUpdate)(THIS_ IValueAnimator* pAnimator) override;
protected:
	SOUI_MSG_MAP_BEGIN()
		MSG_WM_PAINT_EX(OnPaint)
	SOUI_MSG_MAP_END()

	SOUI_ATTRS_BEGIN()
		ATTR_FLOAT(L"value", m_fValue, FALSE)
		ATTR_FLOAT(L"minValue", m_fMinValue, FALSE)
		ATTR_FLOAT(L"maxValue", m_fMaxValue, FALSE)
		ATTR_FLOAT(L"RingWidth", m_nRingWidth, FALSE)
		ATTR_CHAIN(*m_ani,0)
	SOUI_ATTRS_END()

private:
	float m_fValue;			// 当前速度
	float m_fMinValue;
	float m_fMaxValue;		// MAX速度
	int m_nRingWidth;		// 环的宽度

	SAutoRefPtr<SFloatAnimator> m_ani;
private:
	// 内部绘制辅助函数
	void DrawColorArcs(IRenderTarget* pRT, const CRect& rcArc);
	void DrawTicksAndNumbers(IRenderTarget* pRT, const CPoint& center, int radius, int arcWid);
	void DrawPointer(IRenderTarget* pRT, const CPoint& center, int radius, int arcWid);
};

SNSEND

#endif//_SPEED_METER_H_