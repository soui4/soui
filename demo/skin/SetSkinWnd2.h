#pragma once

#define MagicNumber 9527
extern UINT g_dwSkinChangeMessage;
struct SKIN_CONFIG_INF
{
	int id;
	CRect margin;
};
class CSetSkinWnd :
	public SHostDialog
{
	//TODO:消息映射
	void OnClose()
	{
		EndDialog(IDCANCEL);
	}
	HRESULT OnSkinChangeMessage(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL bHandled);
	
	void OnColor(IEvtArgs * e);
protected:
	long NotifUpdataWindow();
	void LoadSkinConfigFormXml();
	CRect GetMargin(int id);	
public:
	CSetSkinWnd();
	~CSetSkinWnd();
	void OnSetSkin(IEvtArgs *e);
	void OnBuiltinSkin();

	EVENT_MAP_BEGIN()
		EVENT_NAME_COMMAND(L"btn_close", OnClose)
		EVENT_ID_RANGE_HANDLER(10, 27, EVT_CMD, OnSetSkin)
		EVENT_ID_RANGE_HANDLER(30,48,EVT_CMD, OnColor)		
		EVENT_ID_COMMAND(51,OnBuiltinSkin)
	EVENT_MAP_END2(SHostDialog)

	BEGIN_MSG_MAP_EX(CSetSkinWnd)
		MESSAGE_HANDLER(g_dwSkinChangeMessage, OnSkinChangeMessage)
		CHAIN_MSG_MAP(SHostDialog)
		REFLECT_NOTIFICATIONS_EX()
	END_MSG_MAP()
private:
	SList<SKIN_CONFIG_INF> m_skinConfigInf;
};

