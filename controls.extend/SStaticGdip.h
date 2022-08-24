#pragma once

SNSBEGIN
class SStaticGdip : public SStatic
{
	DEF_SOBJECT(SStatic,L"text2")
public:
	SStaticGdip(void);
	~SStaticGdip(void);

public:
	static BOOL initGdip();
	static void uninitGdip();
protected:
	void OnDrawLine(IRenderTarget * pRT, LPCTSTR pszBuf, int iBegin, int cchText, LPRECT pRect, UINT uFormat) override;
	SIZE OnMeasureText(IRenderTarget *pRT,LPCTSTR pszText,int cchLen) override;
private:
	static ULONG_PTR s_gdiPlusToken;
};

SNSEND
