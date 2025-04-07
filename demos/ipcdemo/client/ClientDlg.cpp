// MainDlg.cpp : implementation of the CClientDlg class
//
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "ClientDlg.h"	

#define kLogTag "client"

CClientConnect::CClientConnect()
{
    SAutoRefPtr<IIpcFactory> ipcFac;
    m_comMgr.CreateIpcObject((IObjRef **)&ipcFac);
    ipcFac->CreateIpcHandle(&m_ipcHandle);
    m_ipcHandle->SetIpcConnection(this);
}

CClientConnect::~CClientConnect()
{
    m_ipcHandle = NULL;
}

int CClientConnect::Add(int a, int b)
{
    Param_AddInt params;
    params.a = a;
    params.b = b;
    m_ipcHandle->CallFun(&params);
    return params.ret;
}

tstring CClientConnect::Add(const tstring &a, const tstring &b)
{
    Param_AddString params;
    params.a = a;
    params.b = b;
    m_ipcHandle->CallFun(&params);
    return params.ret;
}

int CClientConnect::Sum(int n)
{
    Param_Sum param;
    param.n = n;
    m_ipcHandle->CallFun(&param);
    return param.nRet;
}

IIpcHandle *CClientConnect::GetIpcHandle()
{
    return m_ipcHandle;
}

void CClientConnect::BuildShareBufferName(ULONG_PTR idLocal, ULONG_PTR idRemote, TCHAR szBuf[MAX_PATH]) const
{
    _stprintf(szBuf, DEMO_SHARE_BUF_NAME_FMT, (DWORD)(((LPARAM)idLocal) & 0xffffffff), (DWORD)(((LPARAM)idRemote) & 0xffffffff));
}

void CClientConnect::OnHello(Param_Hello &param)
{
    SLOGI() << "recv,"<<param.text.c_str();
}

void CClientConnect::OnSum(Param_Sum &param)
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

//---------------------------------------------------------------
CClientDlg::CClientDlg()
    : SHostWnd(_T("LAYOUT:dlg_client"))
    , m_hSvr(0)
{
}

CClientDlg::~CClientDlg()
{
}

void CClientDlg::OnDestroy()
{
    Disconnect();
    SetMsgHandled(FALSE);
}

//TODO:消息映射
void CClientDlg::OnClose()
{
	SNativeWnd::DestroyWindow();
}

void CClientDlg::OnMaximize()
{
	SendMessage(WM_SYSCOMMAND, SC_MAXIMIZE);
}
void CClientDlg::OnRestore()
{
	SendMessage(WM_SYSCOMMAND, SC_RESTORE);
}
void CClientDlg::OnMinimize()
{
	SendMessage(WM_SYSCOMMAND, SC_MINIMIZE);
}

void CClientDlg::OnInit(IEvtArgs *e)
{
    OnConn();
}

inline void CClientDlg::OnConn()
{
    BOOL conn = Connect();
    OnConnChange(conn);
}

void CClientDlg::OnSize(UINT nType, CSize size)
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

LRESULT CClientDlg::OnIpcMessage(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    SLOGI()<<"CClientDlg::OnIpcMessage uMsg:"<<uMsg<<" wParam:"<<wParam<<" lParam:"<<lParam;    
    BOOL bHandled=FALSE;
    LRESULT ret = m_conn.GetIpcHandle()->OnMessage((ULONG_PTR)m_hWnd, uMsg, wParam, lParam, bHandled);
    if (!m_conn.GetIpcHandle()->IsConnected())
    {
        OnConnChange(FALSE);
    }
    return ret;
}

void CClientDlg::OnConnChange(BOOL bConnected)
{
    if (bConnected)
    {
        SStringT str;
        str.Format(_T("Connected to server %s"), SVR_NAME);
        FindChildByID(R.id.txt_status)->SetWindowText(str);
    }
    else
    {
        FindChildByID(R.id.txt_status)->SetWindowText(_T("Disconnected"));
    }
    FindChildByID(R.id.btn_conn)->EnableWindow(!bConnected,TRUE);
}

BOOL CClientDlg::Connect()
{
    HWND hSvr = FindWindow(NULL, SVR_NAME);
    if (!hSvr)
        return FALSE;
    SLOGI()<<"connect to remote:"<<hSvr<<" local:"<<m_hWnd;
    HRESULT hr = m_conn.GetIpcHandle()->ConnectTo((ULONG_PTR)m_hWnd, (ULONG_PTR)hSvr);
    BOOL bRet = hr == S_OK;
    if (bRet)
        m_hSvr = hSvr;
    return bRet;
}

BOOL CClientDlg::Disconnect()
{
    HRESULT hr = m_conn.GetIpcHandle()->Disconnect();
    m_hSvr = 0;
    return (hr == S_OK);
}

void CClientDlg::OnAddInt()
{
    SWindow *pEdit1 = FindChildByID(R.id.edit_param1);
    SWindow *pEdit2 = FindChildByID(R.id.edit_param2);
    int a = pEdit1->GetWindowText().ToInt();
    int b = pEdit2->GetWindowText().ToInt();
    int ret = m_conn.Add(a, b);
    FindChildByID(R.id.edit_result)->SetWindowText(SStringT().Format(_T("%d"), ret));
}

static tstring tostdstr(const SStringT &str)
{
    return tstring(str.c_str(),str.GetLength());
}

void CClientDlg::OnAddString()
{
    SWindow *pEdit1 = FindChildByID(R.id.edit_param1);
    SWindow *pEdit2 = FindChildByID(R.id.edit_param2);
    SStringT a = pEdit1->GetWindowText();
    SStringT b = pEdit2->GetWindowText();
    tstring res = m_conn.Add(tostdstr(a), tostdstr(b));
    FindChildByID(R.id.edit_result)->SetWindowText(res.c_str());
}

void CClientDlg::OnSum()
{
    SWindow *pEdit1 = FindChildByID(R.id.edit_param1);
    int n = pEdit1->GetWindowText().ToInt();
    if (n > 10)
    {
        pEdit1->SetWindowText(_T("10"));
        n = 10;
    }
    int ret = m_conn.Sum(n);
    FindChildByID(R.id.edit_result)->SetWindowText(SStringT().Format(_T("%d"), ret));
}