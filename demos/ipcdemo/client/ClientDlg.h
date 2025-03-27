// MainDlg.h : interface of the CClientDlg class
//
/////////////////////////////////////////////////////////////////////////////
#pragma once

#include <interface/sipcobj-i.h>
#include <helper/obj-ref-impl.hpp>
#include <helper/SIpcParamHelper.hpp>
#include "protocol.h"

class CClientConnect
    : public IClient
    , public SOUI::TObjRefImpl<SOUI::IIpcConnection> {
  public:
    CClientConnect();
    ~CClientConnect();

  public:
    virtual int Add(int a, int b) override;
    virtual tstring Add(const tstring &a, const tstring &b) override;
    virtual int Sum(int n) override;

  public:
    virtual int GetBufSize() const
    {
        return 1024*2;
    }
    virtual int GetStackSize() const
    {
        return 10;
    }
    virtual SOUI::IIpcHandle *GetIpcHandle() override;
    virtual void BuildShareBufferName(ULONG_PTR idLocal, ULONG_PTR idRemote, TCHAR szBuf[MAX_PATH]) const override;

    void OnHello(Param_Hello &param);
    void OnSum(Param_Sum &param);
    FUN_BEGIN
    FUN_HANDLER(Param_Hello, OnHello)
    FUN_HANDLER(Param_Sum, OnSum)
    FUN_END
  private:
    SOUI::SAutoRefPtr<SOUI::IIpcHandle> m_ipcHandle;
    SOUI::SComMgr2 m_comMgr;
};


class CClientDlg : public SHostWnd
{
    HWND m_hSvr;
    CClientConnect m_conn;
  public:
	CClientDlg();
	~CClientDlg();

 public:
    BOOL Disconnect();

    void OnAddInt();
    void OnAddString();
    void OnSum();
 protected:
	void OnClose();
	void OnMaximize();
	void OnRestore();
	void OnMinimize();
    void OnInit(IEvtArgs *e);
    void OnConn();
    void OnConnChange(BOOL bConnected);
    BOOL Connect();

  protected:
	//soui消息
	EVENT_MAP_BEGIN()
		EVENT_NAME_COMMAND(L"btn_close", OnClose)
		EVENT_NAME_COMMAND(L"btn_min", OnMinimize)
		EVENT_NAME_COMMAND(L"btn_max", OnMaximize)
		EVENT_NAME_COMMAND(L"btn_restore", OnRestore)
        EVENT_HANDLER(EventInit::EventID, OnInit)
        EVENT_ID_COMMAND(R.id.btn_sum_math, OnAddInt)
        EVENT_ID_COMMAND(R.id.btn_sum_str, OnAddString)
        EVENT_ID_COMMAND(R.id.btn_sum, OnSum)
        EVENT_ID_COMMAND(R.id.btn_conn, OnConn)
	EVENT_MAP_END2(SHostWnd)
		
    void OnDestroy();
    void OnSize(UINT nType, CSize size);
    LRESULT OnIpcMessage(UINT uMsg, WPARAM wParam, LPARAM lParam);

    // HostWnd真实窗口消息处理
	BEGIN_MSG_MAP_EX(CClientDlg)
    MSG_WM_DESTROY(OnDestroy)
		MSG_WM_CLOSE(OnClose)
		MSG_WM_SIZE(OnSize)
    MESSAGE_HANDLER_EX(UM_CALL_FUN, OnIpcMessage)
		CHAIN_MSG_MAP(SHostWnd)
		REFLECT_NOTIFICATIONS_EX()
	END_MSG_MAP()
};
