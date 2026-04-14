#ifndef __COLORIZETEXT_H__
#define __COLORIZETEXT_H__

SNSBEGIN

class SColorizeText : public SWindow
{
	DEF_SOBJECT(SWindow, L"colorizeText")
public:
	SColorizeText(void);
	~SColorizeText(void);

	void SetKeyText(LPCTSTR pszKey);
	void SetWindowText(LPCTSTR pszText) override;
protected:
	virtual void DrawText(IRenderTarget* pRT, LPCTSTR pszBuf, int cchText, LPRECT pRect, UINT uFormat);

	void ClearColorizeInfo();
	void AddColorizeInfo(int iBegin, int iEnd, COLORREF cr);
	void UpdateTextColors();
public:
	SOUI_ATTRS_BEGIN()
		ATTR_COLOR(L"colorKey", m_crKey, TRUE)
		ATTR_BOOL(L"caseSensitive", m_bCaseSensitive,TRUE)
	SOUI_ATTRS_END()

private:
	COLORREF m_crKey;
	SStringT m_strKey;
	BOOL m_bCaseSensitive;

	struct COLORIZEINFO
	{
		int iBegin;
		int iEnd;
		COLORREF cr;
	};

	SArray<COLORIZEINFO> m_lstColorizeInfo;
};

SNSEND

#endif // __COLORIZETEXT_H__