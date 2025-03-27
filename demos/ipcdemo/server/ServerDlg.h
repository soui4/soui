// MainDlg.h : interface of the CServerDlg class
//
/////////////////////////////////////////////////////////////////////////////
#pragma once

#include <interface/sipcobj-i.h>
#include <helper/obj-ref-impl.hpp>
#include <helper/SIpcParamHelper.hpp>
#include "protocol.h"


class CSvrConnect
    : public IServer
    , public SOUI::TObjRefImpl<SOUI::IIpcConnection> {
  public:
    CSvrConnect(SOUI::IIpcHandle *pHandle);
    ~CSvrConnect();

    protected:
    virtual int GetBufSize() const
    {
        return 1024 * 2;
    }
    virtual int GetStackSize() const
    {
        return 20;
    }

  public:
    // 通过 TObjRefImpl 继承
    virtual SOUI::IIpcHandle *GetIpcHandle() override;
    virtual void BuildShareBufferName(ULONG_PTR idLocal, ULONG_PTR idRemote, TCHAR szBuf[MAX_PATH]) const override;

    void OnAddInt(Param_AddInt &param);
    void OnAddStr(Param_AddString &param);
    void OnSum(Param_Sum &param);
    FUN_BEGIN
    FUN_HANDLER(Param_AddInt, OnAddInt)
    FUN_HANDLER(Param_AddString, OnAddStr)
    FUN_HANDLER(Param_Sum, OnSum)
    FUN_END
  private:
    SOUI::SAutoRefPtr<SOUI::IIpcHandle> m_ipcHandle;

    // 通过 IServer 继承
    virtual void SayHello(const tstring &name) override;
};

class CServerDlg
    : public SHostWnd
    , IIpcSvrCallback {
    SComMgr2 m_comMgr;
    SRichEdit *m_reLog;
    SOUI::SAutoRefPtr<SOUI::IIpcServer> m_ipcSvr;
  public:
	CServerDlg();
	~CServerDlg();

    void AppendLog(const SStringT &log);
  public:
    // 通过 IIpcSvrCallback 继承
    ULONG_PTR OnNewConnection(IIpcHandle *pIpcHandle, IIpcConnection **ppConn) override;
    void *GetSecurityAttr() const override;
    void ReleaseSecurityAttr(void *psa) const override;
    void OnConnected(SOUI::IIpcConnection *pConn) override;
    void OnDisconnected(SOUI::IIpcConnection *pConn) override;

    void SendHello(SAutoRefPtr<IIpcConnection> conn);
 protected:

	void OnClose();
	void OnMaximize();
	void OnRestore();
	void OnMinimize();
	void OnSize(UINT nType, CSize size);
    void OnInit(IEvtArgs *e);
  protected:
	//soui消息
    static BOOL OnEnumClient(IIpcConnection *pConn, ULONG_PTR data);
    void OnBreakAllConn();
    EVENT_MAP_BEGIN()
        EVENT_NAME_COMMAND(L"btn_close", OnClose)
        EVENT_NAME_COMMAND(L"btn_min", OnMinimize)
        EVENT_NAME_COMMAND(L"btn_max", OnMaximize)
        EVENT_NAME_COMMAND(L"btn_restore", OnRestore)
        EVENT_HANDLER(EventInit::EventID, OnInit)
        EVENT_ID_COMMAND(R.id.btn_break, OnBreakAllConn)
    EVENT_MAP_END2(SHostWnd)

    LRESULT OnIpcMessage(UINT uMsg, WPARAM wParam, LPARAM lParam);

	//HostWnd真实窗口消息处理
	BEGIN_MSG_MAP_EX(CServerDlg)
		MSG_WM_CLOSE(OnClose)
		MSG_WM_SIZE(OnSize)
        MESSAGE_HANDLER_EX(UM_CALL_FUN, OnIpcMessage)
		CHAIN_MSG_MAP(SHostWnd)
		REFLECT_NOTIFICATIONS_EX()
	END_MSG_MAP()
};
