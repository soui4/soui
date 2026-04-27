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
protected:
	virtual void DrawText(IRenderTarget* pRT, LPCTSTR pszBuf, int cchText, LPRECT pRect, UINT uFormat);
	virtual void OnContentChanged();

	void ClearColorizeInfo();
	void AddColorizeInfo(int iBegin, int iEnd, COLORREF cr);
	void UpdateTextColors();
    HRESULT OnAttrKey(const SStringW &strValue, BOOL bLoading);

  public:
	SOUI_ATTRS_BEGIN()
		ATTR_COLOR(L"colorKey", m_crKey, TRUE)
		ATTR_BOOL(L"caseSensitive", m_bCaseSensitive,TRUE)
        ATTR_CUSTOM(L"key", OnAttrKey)
	SOUI_ATTRS_END()

private:
	COLORREF m_crKey;
	STrText m_strKey;
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