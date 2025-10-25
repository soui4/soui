#pragma once

#include <core/SNativeWnd.h>

class CScintillaModule
{
public:
	CScintillaModule();
	~CScintillaModule();

	BOOL operator !() const
	{
		return m_hModule==NULL;
	}
protected:
	HINSTANCE m_hModule;
};

// CScintillaWnd
class CScintillaWnd : public SNativeWnd
{
public:
	struct IListener
	{
		virtual void onScintillaSave(CScintillaWnd *pSci,LPCTSTR pszFileName) {}
		virtual void onScintillaAutoComplete(CScintillaWnd *pSci,char c) {}
	};

public:
	CScintillaWnd();
	virtual ~CScintillaWnd();
	BOOL Create (LPCTSTR lpszWindowName, DWORD dwStyle, const RECT& rect, HWND hParentWnd, UINT nID,HINSTANCE hInst);
	BOOL OpenFile(LPCTSTR lpFileName);
	void DoSave();

	void ResetRedo();

	void SetDirty(bool bDirty);
	bool isDirty() const{return m_bDirty;}
	BOOL SaveFile(LPCTSTR lpFileName);

	LPCTSTR GetOpenedFileName(){return m_strFileName;}
	void SetOpenedFileName(LPCTSTR pszFileName){m_strFileName=pszFileName;}

	LRESULT SendEditor(UINT Msg, WPARAM wParam=0, LPARAM lParam=0);
	void GotoFoundLine();

	void GotoPos(int nPos);

	void InsertText(int pos, LPCSTR text);
	
	SStringA GetWindowText();

	SStringA GetHtmlTagName(int &tagStartPos);
	
	void SetSel(int nBegin,int nEnd);

	void ReplaseSel(LPCSTR text);

	void SetListener(IListener *pListener);

	void GetRange(int start, int end, char* text);

	SStringA GetRange(int start, int end);

	SStringA GetNotePart(int curPos);
protected:
	// 显示行号
	void UpdateLineNumberWidth(void);
	void InitScintillaWnd(void);
	void SetAStyle(int style, COLORREF fore, COLORREF back = RGB(0xff,0xff,0xff), int size = 0, const char* face = NULL);
	void SetFold();
	// 设置XML的语法规则
	void SetXmlLexer(COLORREF bkColor);

	void findMatchingBracePos(int & braceAtCaret, int & braceOpposite);
	bool doMatch(); //匹配括号并加亮缩进向导;


	void ShowAutoComplete(char ch);
	

	void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);


	void displaySectionCentered(int posStart, int posEnd, bool isDownwards = true);

	LRESULT OnNotify(int idCtrl, LPNMHDR pnmh);

	BEGIN_MSG_MAP_EX(CScintillaWnd)
		MSG_OCM_NOTIFY(OnNotify)
		MSG_WM_KEYDOWN(OnKeyDown)
		CHAIN_MSG_MAP(SNativeWnd)
	END_MSG_MAP()

protected:
	bool m_bDirty;		//指示文档是否已修改

	SStringT m_strFileName;
	IListener	*m_pListener;
};
