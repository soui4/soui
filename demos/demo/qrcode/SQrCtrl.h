﻿#ifndef _SQRCTRL_H_
#define _SQRCTRL_H_

#include "QR_Encode.h"
#include "control/SCmnCtrl.h"

SNSBEGIN

class SQrCtrl :public SImageWnd
{
	DEF_SOBJECT(SImageWnd, L"qrctrl")
public:
	SQrCtrl();
	~SQrCtrl();

	STDMETHOD_(void,SetWindowText)(THIS_ LPCTSTR lpszText) OVERRIDE;
protected:
	void CreateQrImg(SStringT strContent);
	virtual BOOL OnRelayout(const CRect & rcWnd) override;

	void MakeCacheApha(ISkinObj *pSkin, IBitmapS *_pBitCache, IBitmapS *_pBitMask);

	SOUI_ATTRS_BEGIN()
		ATTR_RECT(L"qrrect", m_QrRect, FALSE)
	SOUI_ATTRS_END()
private:
	CRect m_QrRect;
};

SNSEND
#endif//_SQRCTRL_H_