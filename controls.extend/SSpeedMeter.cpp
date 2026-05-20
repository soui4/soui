#include "stdafx.h"
#include "SSpeedMeter.h"
#include <math.h>
#include <vector>
#define PI 3.1415926f

SNSBEGIN

SSpeedMeter::SSpeedMeter() {
	m_fValue = 0.0f;
	m_fMinValue = 0.0f;
	m_nRingWidth = 20;
	m_fMaxValue = 200.0f;
	m_ani.Attach(new SFloatAnimator);
	m_ani->addUpdateListener(this);
}
SSpeedMeter::~SSpeedMeter()
{
}

void SSpeedMeter::SetValue(float value)
{
	if (value > m_fMaxValue)
		value = m_fMaxValue;
	if (value < m_fMinValue)
		value = m_fMinValue;
	if (value == m_fValue)
		return;
	float start = m_fValue;
	if (m_ani->isRunning()) {
		start = m_ani->getValue();
		m_ani->end();
	}
	m_ani->setRange(start, value);
	m_ani->start(GetContainer());
	m_fValue = value;
	Invalidate();
}

float SSpeedMeter::GetValue() const {
	return m_fValue;
}
void SSpeedMeter::OnPaint(IRenderTarget* pRT) {
	CRect rcClient = GetClientRect();
	int nAdj = (rcClient.Width() - rcClient.Height()) / 2;
	if (nAdj > 0) rcClient.DeflateRect(nAdj, 0);
	else rcClient.DeflateRect(0, -nAdj);
	int arcWid = m_nRingWidth;

	CRect rcArc = rcClient;
	rcArc.DeflateRect(arcWid / 2, arcWid / 2); // 考虑线宽缩进

	CPoint center = rcArc.CenterPoint();
	int radius = rcArc.Width() / 2;

	DrawColorArcs(pRT, rcArc);// 绘制背景及彩色圆弧
	DrawTicksAndNumbers(pRT, center, radius, arcWid);// 绘制刻度与数字

	// 绘制指针与扇形阴影 (图层顺序重要)
	DrawPointer(pRT, center, radius, arcWid);

	int nProWid = 3;
	int nDef = (m_nRingWidth - nProWid) / 2;
	CRect rcPro(rcArc);
	rcPro.DeflateRect(nDef, nDef);
	if(0){
		SAutoRefPtr<IPenS> pPen;
		pRT->CreatePen(PS_SOLID | PS_GEOMETRIC | PS_ENDCAP_ROUND, GETCOLOR(L"#00000030"), nProWid, &pPen);
		pRT->SelectObject(pPen, NULL);
		pRT->DrawArc(&rcPro, 150.f, 240, false); // 覆盖一层白色或亮色的当前进度
	}

	float fSweep = ((m_fValue-m_fMinValue) / (m_fMaxValue - m_fMinValue)) * 240.f;
	if (0) {
		SAutoRefPtr<IPenS> pPen;
		pRT->CreatePen(PS_SOLID | PS_GEOMETRIC | PS_ENDCAP_ROUND, GETCOLOR(L"#0000FF70"), nProWid, &pPen);
		pRT->SelectObject(pPen, NULL);
		pRT->DrawArc(&rcPro, 150.f, fSweep, false); // 覆盖一层白色或亮色的当前进度
	}
}

void SSpeedMeter::DrawColorArcs(IRenderTarget* pRT, const CRect& rcArc) {

	SAutoRefPtr<IPenS> pPenGreen, pPenOrange, pPenRed, oldPen;
	SAutoRefPtr<IPathS> arcPath;
	GETRENDERFACTORY->CreatePath(&arcPath);
	arcPath->addArc(&rcArc, 150, 240);
	// 绿色段
	pRT->CreatePen(PS_SOLID | PS_GEOMETRIC | PS_ENDCAP_ROUND, RGBA(255,0,0,255), m_nRingWidth, &pPenGreen);
	pRT->SelectObject(pPenGreen, (IRenderObj**)&oldPen);
	GradientItem grads[6];
	grads[0].pos = 0.0f;
	grads[0].cr = RGBA(0, 0, 255, 255);
	grads[1].pos = 1.0f/6;
	grads[1].cr = RGBA(0, 240, 120,255);
	grads[2].pos = 1.0f*80/360;
	grads[2].cr = RGBA(0, 255, 127, 255);
	grads[3].pos = 1.0f*160/360;
	grads[3].cr = RGBA(255, 165, 0, 255);
	grads[4].pos = 1.0f * 240 / 360;
	grads[4].cr = RGBA(255, 69, 0, 255);
	grads[5].pos = 1.0f;
	grads[5].cr = RGBA(255, 69, 0, 255);
	GradientInfo info;
	info.type = sweep;
	info.sweep.startAngle = 90.0f;
	info.sweep.centerX = 0.5f;
	info.sweep.centerY = 0.5f;
	pRT->DrawGradientPath(arcPath, grads, 6, &info,255,&rcArc);
	pRT->SelectObject(oldPen);
}

void SSpeedMeter::DrawTicksAndNumbers(IRenderTarget* pRT, const CPoint& center, int radius, int arcWid) {
	pRT->SetTextColor(GETCOLOR(L"#222222"));

	int textOffset = arcWid + m_nRingWidth;
	// 绘制数字与大刻度
	for (int i = 0; i <= 4; ++i) {
		float val = m_fMinValue + i * ((m_fMaxValue - m_fMinValue) / 4.0f);
		float angle = 150.f + (i * 60.f);
		float rad = angle * PI / 180.f;

		int x = center.x + (int)((radius - textOffset) * cos(rad));
		int y = center.y + (int)((radius - textOffset) * sin(rad));

		SStringT strText = SStringT().Format(_T("%.0f"), val);
		CRect rcText(x - 20, y - 10, x + 20, y + 10);
		pRT->DrawText(strText, strText.GetLength(), rcText, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
	}

	// 绘制刻度线
	SAutoRefPtr<IPenS> pTickPen, oldPen;
	pRT->CreatePen(PS_SOLID, GETCOLOR(L"#00000080"), 1, &pTickPen);
	pRT->SelectObject(pTickPen, (IRenderObj**)&oldPen);

	int tickR = radius - m_nRingWidth / 2 + 2;
	for (int deg = 150; deg <= 390; deg += 6) {
		float rad = deg * PI / 180.f;
		int len = ((deg - 150) % 60 == 0) ? m_nRingWidth : m_nRingWidth / 2;

		std::vector<CPoint> vecPts;
		vecPts.push_back(CPoint(center.x + (int)(tickR * cos(rad)), center.y + (int)(tickR * sin(rad))));
		vecPts.push_back(CPoint(center.x + (int)((tickR - len) * cos(rad)), center.y + (int)((tickR - len) * sin(rad))));
		pRT->DrawLines(&vecPts[0], vecPts.size());
	}
	pRT->SelectObject(oldPen);
}

void SSpeedMeter::DrawPointer(IRenderTarget* pRT, const CPoint& center, int radius, int arcWid) {
	float fValue = m_ani->isRunning()?m_ani->getValue():m_fValue;
	float fSweep = ((fValue-m_fMinValue) /(m_fMaxValue - m_fMinValue)) * 240.f;
	float fAngle = 150.f + fSweep;
	float fRad = fAngle * PI / 180.f;

	//绘制半透明扇形阴影 ---
	SAutoRefPtr<IBrushS> pBrush;
	pRT->CreateSolidColorBrush(GETCOLOR(L"rgba(255,165,0,15)"), &pBrush);
	pRT->SelectObject(pBrush, NULL);
	int shadowR = radius - arcWid;
	CRect rcPie(center.x - shadowR, center.y - shadowR, center.x + shadowR, center.y + shadowR);
	pRT->FillArc(rcPie, 150.f, fSweep);

	int nPointerLen = radius - arcWid; // 箭头长度
	int nArrowWidth = 8;                   // 箭头底部的半宽

	// 计算三角形的三个顶点
	CPoint pts[3];
	// 顶点 A: 针尖
	pts[0].x = center.x + (int)(nPointerLen * cos(fRad));
	pts[0].y = center.y + (int)(nPointerLen * sin(fRad));

	// 顶点 B & C: 底部两个角（垂直于指针方向偏移）
	// 垂直方向的角度为 fRad + PI/2 和 fRad - PI/2
	float fRadLeft = fRad + (PI * 0.5f);
	float fRadRight = fRad - (PI * 0.5f);

	pts[1].x = center.x + (int)(nArrowWidth * cos(fRadLeft));
	pts[1].y = center.y + (int)(nArrowWidth * sin(fRadLeft));

	pts[2].x = center.x + (int)(nArrowWidth * cos(fRadRight));
	pts[2].y = center.y + (int)(nArrowWidth * sin(fRadRight));

	// 创建画刷填充箭头颜色
	SAutoRefPtr<IBrushS> pPtrBrush;
	pRT->CreateSolidColorBrush(GETCOLOR(L"#FF4500"), &pPtrBrush);
	pRT->SelectObject(pPtrBrush, NULL);

	// 绘制填充多边形（三角形）
	pRT->FillPolygon(pts, 3);

	// (可选) 给箭头画一个深色细边框，使其更清晰
	SAutoRefPtr<IPenS> pPtrPen;
	pRT->CreatePen(PS_SOLID, GETCOLOR(L"#D35400"), 1, &pPtrPen);
	pRT->SelectObject(pPtrPen, NULL);
	pRT->DrawPolygon(pts, 3); // TRUE 表示闭合路径

	// 绘制中心圆盖 (Hub) ---
	SAutoRefPtr<IBrushS> pBrush1;
	pRT->CreateSolidColorBrush(GETCOLOR(L"#DCDCDC"), &pBrush1);
	pRT->SelectObject(pBrush1, NULL);
	CRect rcHub(center.x - 10, center.y - 10, center.x + 10, center.y + 10);
	pRT->FillEllipse(rcHub);

	// 绘制下方速度数值文字
	pRT->SetTextColor(GETCOLOR(L"#333333"));
	CRect rcVal(center.x - 50, center.y + 20, center.x + 50, center.y + 60);
	pRT->DrawText(SStringT().Format(_T("%.0f"), fValue), -1, rcVal, DT_CENTER | DT_VCENTER | DT_SINGLELINE);

	CRect rcUnit(center.x - 40, center.y + 60, center.x + 40, center.y + 80);
	pRT->SetTextColor(GETCOLOR(L"#999999"));
	pRT->DrawText(_T("m/min"), -1, rcUnit, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
}

void SSpeedMeter::onAnimationUpdate(IValueAnimator* pAnimator)
{
	SASSERT(pAnimator == m_ani);
	Invalidate();
}

SNSEND