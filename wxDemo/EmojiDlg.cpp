#include "StdAfx.h"
#include "EmojiDlg.h"

CEmojiDlg::CEmojiDlg(IListener* pListner)
: SHostWnd(UIRES.LAYOUT.XML_DLG_EMOJI)
, m_pListner(pListner)
{
}

CEmojiDlg::~CEmojiDlg(void)
{
}

BOOL CEmojiDlg::OnInitDialog(HWND wndFocus, LPARAM lInitParam)
{
	SetMsgHandled(FALSE);
	STileView* pTileView = FindChildByName2<STileView>(L"emoji_titleview");
	SASSERT(pTileView);
	CEmotionTileViewAdapter* pAdapter = new CEmotionTileViewAdapter(this);
	pTileView->SetAdapter(pAdapter);
	std::map<std::string, std::string>::iterator iter = CGlobalUnits::instance()->m_mapEmojisIndex.begin();
	for (; iter != CGlobalUnits::instance()->m_mapEmojisIndex.end(); iter++)
	{
		pAdapter->AddItem(iter->first.c_str());
	}
	pAdapter->Release();
	pTileView->SetSel(-1);

	return FALSE;
}

void CEmojiDlg::OnCommand(UINT uNotifyCode, int nID, HWND wndCtl)
{
	//
}

void CEmojiDlg::OnSize(UINT nType, SOUI::CSize size)
{
	SetMsgHandled(FALSE);
}

void CEmojiDlg::OnLButtonDown(UINT nFlags, SOUI::CPoint pt)
{
	SetMsgHandled(FALSE);
}

void CEmojiDlg::OnTimer(UINT_PTR idEvent)
{
	SetMsgHandled(FALSE);
}

void CEmojiDlg::OnSysCommand(UINT nID, SOUI::CPoint pt)
{
	switch (nID)
	{
	case SC_CLOSE:
		{
			ShowWindow(SW_HIDE);
			return;
		}
		break;
	default:
		break;
	}
	SetMsgHandled(FALSE);
}

void CEmojiDlg::SetNoSel()
{
	STileView* pTileView = FindChildByName2<STileView>(L"emoji_titleview");
	assert(pTileView);
	pTileView->SetSel(-1);
}

void CEmojiDlg::OnEmotionItemClick(const std::string& strID)
{
	m_pListner->EmotionTileViewItemClick(strID);
}