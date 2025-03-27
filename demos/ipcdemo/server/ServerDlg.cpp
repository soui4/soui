// MainDlg.cpp : implementation of the CServerDlg class
//
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "ServerDlg.h"	
#include <helper/SFunctor.hpp>

#define kLogTag "server"

CSvrConnect::CSvrConnect(IIpcHandle *pHandle)
    : m_ipcHandle(pHandle)
{
}

CSvrConnect::~CSvrConnect()
{
    m_ipcHandle = NULL;
}

IIpcHandle *CSvrConnect::GetIpcHandle()
{
    return m_ipcHandle;
}

void CSvrConnect::BuildShareBufferName(ULONG_PTR idLocal, ULONG_PTR idRemote, TCHAR szBuf[MAX_PATH]) const
{
    _stprintf(szBuf, DEMO_SHARE_BUF_NAME_FMT, (DWORD)(((LPARAM)idLocal) & 0xffffffff), (DWORD)(((LPARAM)idRemote) & 0xffffffff));
}

void CSvrConnect::OnAddInt(Param_AddInt &param)
{
    param.ret = param.a + param.b;
}
void CSvrConnect::OnAddStr(Param_AddString &param)
{
    param.ret = param.a + param.b;
}

void CSvrConnect::SayHello(const tstring &name)
{
    Param_Hello param;
    param.text = name;
    m_ipcHandle->CallFun(&param);
}

void CSvrConnect::OnSum(Param_Sum &param)
{
    // recursive call to calc sum (1,n);
    SASSERT(param.n >= 1);
    if (param.n == 1)
    {
        param.nRet = 1;
    }
    else
    {
        Param_Sum p2;
        p2.n = param.n - 1;
        m_ipcHandle->CallFun(&p2);
        param.nRet = param.n + p2.nRet;
    }
}

//----------------------------------------------------
CServerDlg::CServerDlg()
    : SHostWnd(_T("LAYOUT:dlg_server"))
    , m_reLog(NULL)
{
    SOUI::SAutoRefPtr<IIpcFactory> ipcFac;
    m_comMgr.CreateIpcObject((IObjRef **)&ipcFac);
    ipcFac->CreateIpcServer(&m_ipcSvr);
}

CServerDlg::~CServerDlg()
{
}

void CServerDlg::AppendLog(const SStringT &log)
{
    m_reLog->SSendMessage(EM_SETSEL, -1, 0);
    m_reLog->ReplaceSel(log.c_str());
}

void CServerDlg::OnInit(IEvtArgs *e)
{
    SetWindowText(SVR_NAME);
    m_ipcSvr->Init((ULONG_PTR)m_hWnd, this);
    m_reLog = FindChildByName2<SRichEdit>(_T("re_log"));
}

BOOL CServerDlg::OnEnumClient(IIpcConnection *pConn, ULONG_PTR data)
{
    CServerDlg *pThis = (CServerDlg *)data;
    pThis->OnDisconnected(pConn);
    pThis->m_ipcSvr->Disconnect(pConn->GetIpcHandle()->GetRemoteId());
    return TRUE;
}

void CServerDlg::OnBreakAllConn()
{
    m_ipcSvr->EnumClient(OnEnumClient,(ULONG_PTR)this);
}

//TODO:消息映射
void CServerDlg::OnClose()
{
	SNativeWnd::DestroyWindow();
}

void CServerDlg::OnMaximize()
{
	SendMessage(WM_SYSCOMMAND, SC_MAXIMIZE);
}
void CServerDlg::OnRestore()
{
	SendMessage(WM_SYSCOMMAND, SC_RESTORE);
}
void CServerDlg::OnMinimize()
{
	SendMessage(WM_SYSCOMMAND, SC_MINIMIZE);
}

void CServerDlg::OnSize(UINT nType, CSize size)
{
	SetMsgHandled(FALSE);	
	SWindow *pBtnMax = FindChildByName(L"btn_max");
	SWindow *pBtnRestore = FindChildByName(L"btn_restore");
	if(!pBtnMax || !pBtnRestore) return;
	
	if (nType == SIZE_MAXIMIZED)
	{
		pBtnRestore->SetVisible(TRUE);
		pBtnMax->SetVisible(FALSE);
	}
	else if (nType == SIZE_RESTORED)
	{
		pBtnRestore->SetVisible(FALSE);
		pBtnMax->SetVisible(TRUE);
	}
}


ULONG_PTR CServerDlg::OnNewConnection(IIpcHandle *pIpcHandle, IIpcConnection **ppConn)
{
    CSvrConnect *pConn = new CSvrConnect(pIpcHandle);
    *ppConn = pConn;
    return ULONG_PTR(m_hWnd);
}

void *CServerDlg::GetSecurityAttr() const
{
    return NULL;
}

void CServerDlg::ReleaseSecurityAttr(void *psa) const
{
}

void CServerDlg::SendHello(SAutoRefPtr<IIpcConnection> pConn)
{
    SLOGI() << "async say hello to new connection";
    Param_Hello param;
    TCHAR szBuf[100];
    _stprintf(szBuf, _T("hello, %08x"), (UINT)pConn->GetIpcHandle()->GetRemoteId());
    param.text = szBuf;
    pConn->GetIpcHandle()->CallFun(&param);
}

void CServerDlg::OnConnected(IIpcConnection *pConn)
{
    ULONG_PTR remoteId = pConn->GetIpcHandle()->GetRemoteId();
    SLOGI()<<"OnConnected, remote:"<<remoteId<<" local:"<<m_hWnd;
    TCHAR szLog[100];
    _stprintf(szLog, _T("client %08x connected +++\n"), (UINT)pConn->GetIpcHandle()->GetRemoteId());
    AppendLog(szLog);
    //delay say hello
    STaskHelper::post(GetMsgLoop(), this, &CServerDlg::SendHello, pConn);
}

void CServerDlg::OnDisconnected(IIpcConnection *pConn)
{
    TCHAR szLog[100];
    _stprintf(szLog, _T("client %08x disconnected ---\n"), (UINT)pConn->GetIpcHandle()->GetRemoteId());
    AppendLog(szLog);
}


LRESULT CServerDlg::OnIpcMessage(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    SLOGI() << "CServerDlg::OnIpcMessage uMsg:" << uMsg << " wParam:" << wParam << " lParam:" << lParam;    
    BOOL bHandled = FALSE;
    return m_ipcSvr->OnMessage((ULONG_PTR)m_hWnd, uMsg, wParam, lParam, bHandled);
}
