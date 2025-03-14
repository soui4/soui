#include <souistd.h>
#include "SImageKnob.h"
#include <matrix/SMatrix.h>
#pragma warning(disable:4244)

SNSBEGIN

namespace Constants {
	static const float kCKnobRange = 200.f;

	static const float pi = 3.14159265358979323846f;
	static const float double_pi = 6.28318530717958647692f;
	static const float half_pi = 1.57079632679489661923f;
	static const float quarter_pi = 0.78539816339744830962f;
	static const float e = 2.7182818284590452354f;
	static const float ln2 = 0.69314718055994530942f;
	static const float sqrt2 = 1.41421356237309504880f;

	static const float log2 = logf(2.0f);
} // Constants

static inline float mylog2f(float v){
	return logf(v)/Constants::log2;
}

struct SImageKnob::MouseEditingState
{
	CPoint firstPoint;
	CPoint lastPoint;
	float startValue;
	float entryState;
	float range;
	float coef;
	bool modeLinear;
};


SImageKnob::SImageKnob() {
	m_rulerWidth = 10.0f;
	m_crRulerBack = RGBA(0x2F, 0x2F, 0x2F,0xff);
	m_crRulerForeEnable = RGBA(0x00, 0xDE, 0xC1, 0xff);
	m_crRulerForeDisable = RGBA(0x6D, 0x6E, 0x6E, 0xff);

	m_bLogTransform=false;
	m_startFromCenter = false;
	m_startFromRight = false;
	m_min=0.0, m_max=1.0, m_value=0.5;

	m_state=NULL;
	m_startAngle = (float)(3.f * Constants::quarter_pi);
	m_rangeAngle = (float)(3.f * Constants::half_pi);
	GetEventSet()->addEvent(EVENTID(EventKnobValue));
}

SImageKnob::~SImageKnob() {}

float SImageKnob::getValue(bool bRaw) const
{
	if (!m_bLogTransform)
		return m_value;
	if (bRaw)
		return powf(2.0f,m_value);
	else
		return m_value;
}

void SImageKnob::setValue(float v)
{
	if (m_bLogTransform)
		m_value = mylog2f(v);
	else
		m_value = v;
	Invalidate();
}

float SImageKnob::getRange(bool bRaw) const
{
	if (bRaw) {
		return powf(2.0f, m_max) - powf(2.0f, m_min);
	}
	else
	{
		return m_max - m_min;
	}
}

void SImageKnob::setRange(float vMin, float vMax)
{
	setMin(vMin);
	setMax(vMax);
	bounceValue();
	Invalidate();
}

void SImageKnob::setMin(float v)
{
	if (m_bLogTransform)
		m_min = mylog2f(v);
	else
		m_min = v;
	Invalidate();
}

float SImageKnob::getMin(bool bRaw) const
{
	if (!m_bLogTransform)
		return m_min;
	if (bRaw)
		return powf(2.0f, m_min);
	else
		return m_min;
}

void SImageKnob::setMax(float v)
{
	if (m_bLogTransform)
		m_max = mylog2f(v);
	else
		m_max = v;
	Invalidate();
}

float SImageKnob::getMax(bool bRaw) const
{
	if (!m_bLogTransform)
		return m_max;
	if (bRaw)
		return powf(2.0f, m_max);
	else
		return m_max;
}

SImageKnob::MouseEditingState& SImageKnob::getMouseEditingState()
{
	if (!m_state)
	{
		m_state = new MouseEditingState;
	}
	return *m_state;
}

void SImageKnob::bounceValue() {
	if (m_value > getMax())
		m_value = getMax();
	else if (m_value < getMin())
		m_value = getMin();
}

float SImageKnob::valueFromPoint(CPoint& point) const
{
	CRect rc = GetClientRect();
	CPoint c(rc.Width() / 2., rc.Height() / 2.);

	double dx = (point.x - c.x); // xradius;
	double dy = (point.y - c.y); // yradius;

	double alpha = atan2(dy, dx);
	alpha -= Constants::half_pi;//make sure 0 is downward.
	//make sure alpha is in [0,2pi);
	alpha += Constants::double_pi;
	while (alpha >= Constants::double_pi)
		alpha -= Constants::double_pi;

	double v = getMin() + alpha / m_rangeAngle * getRange();
	return v;
}

void SImageKnob::OnLButtonDown(UINT nFlags, CPoint pt)
{
	__baseCls::OnLButtonDown(nFlags, pt);
	MouseEditingState& mouseState = getMouseEditingState();
	mouseState.firstPoint = pt;
	mouseState.lastPoint=CPoint(-1, -1);
	mouseState.startValue = m_value;
	mouseState.modeLinear = false;
	mouseState.entryState = m_value;
	mouseState.range = Constants::kCKnobRange;
	mouseState.coef = (getMax() - getMin()) / mouseState.range;

	int32_t mode = kLinearMode;
	if (GetKeyState(VK_MENU)&0x8000)
	{
		mode = kCircularMode;
	}
	if (mode == kLinearMode)
	{
		mouseState.lastPoint = pt;
		mouseState.modeLinear = true;
		mouseState.coef = (getMax() - getMin()) / mouseState.range;
	}
	else
	{
		CPoint where2(pt);
		CRect rc = GetClientRect();
		where2+=CPoint(-rc.left, -rc.top);
		mouseState.startValue = valueFromPoint(where2);
		mouseState.lastPoint = pt;
	}
}

void SImageKnob::OnLButtonUp(UINT nFlags, CPoint pt)
{
	__baseCls::OnLButtonUp(nFlags, pt);
	if (m_state)
	{
		delete m_state;
		m_state = NULL;
	}

}

void SImageKnob::OnMouseMove(UINT nFlags, CPoint where)
{
	if (m_state)
	{
		MouseEditingState& mouseState = getMouseEditingState();
		float middle = (getMax() - getMin()) * 0.5f;
		CRect rc = GetClientRect();
		if (where != mouseState.lastPoint)
		{
			mouseState.lastPoint = where;
			if (mouseState.modeLinear)
			{
				LONG diff = (mouseState.firstPoint.y - where.y) + (where.x - mouseState.firstPoint.x);
				m_value = (float)(mouseState.entryState + diff * mouseState.coef);
				bounceValue();
			}
			else
			{
				where += -rc.TopLeft();
				m_value = valueFromPoint(where);
				if (mouseState.startValue - m_value > middle)
					m_value = getMax();
				else if (m_value - mouseState.startValue > middle)
					m_value = getMin();
				else
					mouseState.startValue = m_value;
			}
			EventKnobValue evt(this);
			evt.value = m_value;
			FireEvent(evt);
			Invalidate();
		}
	}
}


void SImageKnob::OnPaint(IRenderTarget* pRT) {
	SPainter painter;
	BeforePaint(pRT, painter);

	CRect r = GetClientRect();
	CPoint origin(r.left,r.top+r.Height()/2);
	float alpha = (getValue() - getMin()) / (getMax() - getMin());
	float endAngle = alpha * m_rangeAngle + m_startAngle;
	ISkinObj* img = m_pBgSkin;
	if (img) {
		SMatrix matrix;
		CPoint ptCenter = r.CenterPoint();
		float degree = endAngle * 180 / Constants::pi + 90;
		matrix.translate(-ptCenter.x,-ptCenter.y).rotate(degree).translate(ptCenter.x,ptCenter.y);
		CSize szImg = img->GetSkinSize();
		CRect rc = r;
		rc.DeflateRect((rc.Size() - szImg) / 2);
		SMatrix oldMtx;
		pRT->SetTransform(matrix.fMat, oldMtx.fMat);
		img->DrawByState(pRT, rc, GetState());
		pRT->SetTransform(oldMtx.fMat);
	}
	if(1){
		SAutoRefPtr<IPen> pen, oldPen;
		pRT->CreatePen(PS_SOLID|PS_ENDCAP_FLAT, m_crRulerBack, m_rulerWidth, &pen);
		pRT->SelectObject(pen, (IRenderObj**)&oldPen);

		r.InflateRect(-m_rulerWidth / 2, -m_rulerWidth / 2);
		double startDegree = m_startAngle * 180 / Constants::pi;
		double rangeDegree = m_rangeAngle * 180 / Constants::pi;
		
		pRT->DrawArc(r, startDegree, rangeDegree, false);
		pen->SetColor(GetState() & WndState_Disable ? m_crRulerForeDisable : m_crRulerForeEnable);

		if (!m_startFromCenter && !m_startFromRight)
		{
			pRT->DrawArc(r, startDegree, rangeDegree * alpha, false);
		}
		else
		{
			if (m_startFromCenter)
			{
				startDegree = 270;
				pRT->DrawArc(r, startDegree, rangeDegree * (alpha - 0.5), false);
			}
			if (m_startFromRight)
			{
				startDegree = 45;
				pRT->DrawArc(r, startDegree, rangeDegree * (alpha - 1), false);
			}
		}
		pRT->SelectObject(oldPen);
	}
	AfterPaint(pRT, painter);
}

BOOL SImageKnob::OnEraseBkgnd(IRenderTarget* pRT)
{
	return TRUE;
}


HRESULT SImageKnob::OnAttrStartAngle(const SStringW& value, BOOL bLoading) {
	float v=_wtof(value);
	v = v / 180.f * Constants::pi;
	m_startAngle = v;
	return bLoading?S_FALSE:S_OK;
}

HRESULT SImageKnob::OnAttrRangeAngle(const SStringW& value, BOOL bLoading) {
	float v = _wtof(value);
	v = v / 180.f * Constants::pi;
	m_rangeAngle = v;
	return bLoading ? S_FALSE : S_OK;
}

SNSEND
