#include "stdafx.h"
#include "SIpcObject.h"

SNSBEGIN

	SIpcHandle::SIpcHandle() 
		:m_pConn(NULL), m_hLocalId(0),m_hRemoteId(0)
		,m_uCallSeq(0),m_nCallStack(0),m_bSameThread(false)
	{
	}

	SIpcHandle::~SIpcHandle() {
	}


	BOOL SIpcHandle::InitShareBuf(ULONG_PTR idLocal, ULONG_PTR idRemote, UINT uBufSize, void* pSa)
	{
		assert(m_hRemoteId == 0);
		assert(m_hLocalId == 0);


		TCHAR szName[MAX_PATH];
		
		GetIpcConnection()->BuildShareBufferName(idLocal, idRemote, szName);

		if (!m_sendBuf.OpenMemFile(szName, uBufSize, pSa))
		{
			return FALSE;
		}
		GetIpcConnection()->BuildShareBufferName(idRemote, idLocal, szName);

		if (!m_recvBuf.OpenMemFile(szName, uBufSize, pSa))
		{
			m_sendBuf.Close();
			return FALSE;
		}

		m_hLocalId = (HWND)idLocal;
		m_hRemoteId = (HWND)idRemote;
		
		DWORD dwProcLocal=0,dwProcRemote=0;
		DWORD dwTrdLocal = GetWindowThreadProcessId(m_hLocalId,&dwProcLocal);
		DWORD dwTrdRemote = GetWindowThreadProcessId(m_hRemoteId,&dwProcRemote);
		m_bSameThread = (dwProcLocal == dwProcRemote) && (dwTrdLocal == dwTrdRemote);
		return TRUE;
	}

	LRESULT SIpcHandle::OnMessage(ULONG_PTR idLocal, UINT uMsg, WPARAM wp, LPARAM lp, BOOL &bHandled)
	{
		bHandled = FALSE;
        if ((HWND) idLocal != m_hLocalId)
			return 0;
		if (UM_CALL_FUN != uMsg)
			return 0;
		bHandled = TRUE;
        if (wp == FUN_ID_DISCONNECT)
        {
            m_hRemoteId = 0;
            m_recvBuf.Close();
            m_hLocalId = 0;
            m_sendBuf.Close();
            return 0;
        }

		IShareBuffer *pBuf = GetRecvBuffer();
		int iStack = (int)wp;//get call stack.
		pBuf->Seek(IShareBuffer::seek_set, iStack * m_pConn->GetBufSize());//seek to parameter header.

		//read Seq
		int nCallSeq = 0;
		pBuf->Read(&nCallSeq,4);
		//read func id
		UINT uFunId = 0;
		pBuf->Read(&uFunId,4);
		SParamStream ps(pBuf);
		//SLOG_INFO("handle call, this:"<<this<<" seq="<<nCallSeq<<" fun id="<<uFunId<<" wp="<<wp);
		bool bReqHandled = m_pConn->HandleFun(uFunId, ps);

		return  bReqHandled?1:0;
	}

	HRESULT SIpcHandle::ConnectTo(ULONG_PTR idLocal, ULONG_PTR idSvr)
	{
		HWND hSvr = (HWND)idSvr;
		if (!IsWindow((HWND)idLocal))
			return E_INVALIDARG;
		if (!IsWindow(hSvr))
			return E_INVALIDARG;
		DWORD_PTR dwResult = 0;
		LRESULT lRet = ::SendMessageTimeout(hSvr, UM_CALL_FUN, FUN_ID_CONNECT, (LPARAM)idLocal, SMTO_ABORTIFHUNG,100,&dwResult);
		if (lRet == 0)
		{
			return E_FAIL;
		}
		if (dwResult == 0)
		{
			return E_FAIL;
		}
		InitShareBuf(idLocal, dwResult, 0, NULL);
		return S_OK;
	}

	HRESULT SIpcHandle::Disconnect()
	{
		if (m_hLocalId == 0)
			return E_UNEXPECTED;
		if (m_hRemoteId == 0)
			return E_UNEXPECTED;
        ::PostMessage((HWND)m_hRemoteId, UM_CALL_FUN, FUN_ID_DISCONNECT, (LPARAM)m_hLocalId);
		m_hRemoteId = 0;
		m_recvBuf.Close();
		m_hLocalId = 0;
		m_sendBuf.Close();
		return S_OK;
	}

	struct MsgRet{
		bool bRet;
		LRESULT ret;
	};
	void SIpcHandle::OnSendMessageResult(HWND hwnd, UINT msg, ULONG_PTR data, LRESULT res)
	{
		MsgRet *msgRet = (MsgRet*)data;
		msgRet->ret = res;
		msgRet->bRet = true;
	}

	bool SIpcHandle::CallFun(IFunParams * pParam) const
	{
		if (m_hRemoteId == 0)
			return false;

		//make sure msg queue is empty.
		MSG msg;
		while(::PeekMessage(&msg, m_hLocalId, UM_CALL_FUN, UM_CALL_FUN, PM_REMOVE))
		{
			if(msg.message == WM_QUIT)
			{
				PostQuitMessage((int)msg.wParam);
				return false;
			}
			DispatchMessage(&msg);
		}
		IShareBuffer *pBuf = &m_sendBuf;
		assert(m_nCallStack >= 0);
		if (m_nCallStack + 1 > m_pConn->GetStackSize())
			return false;
		pBuf->Seek(IShareBuffer::seek_set, m_nCallStack * m_pConn->GetBufSize());
		m_nCallStack++;

		int nCallSeq = m_uCallSeq ++;
		if(m_uCallSeq>=0xFFFF) m_uCallSeq=0;
		//SLOG_WARN("call function, this:"<<this<<" seq="<<nCallSeq<<" id="<<pParam->GetID());
		pBuf->Write(&nCallSeq,4);//write call seq first.
		UINT uFunId = pParam->GetID();
		pBuf->Write(&uFunId,4);
		ToStream4Input(pParam, pBuf);
		DWORD dwPos = pBuf->Tell();

		MsgRet msgRet={false,0};
		if(m_bSameThread)
		{
			msgRet.ret = SendMessage(m_hRemoteId,UM_CALL_FUN, (WPARAM)m_nCallStack - 1, (LPARAM)m_hLocalId);
		}else{
			SendMessageCallback(m_hRemoteId, UM_CALL_FUN, (WPARAM)m_nCallStack - 1, (LPARAM)m_hLocalId,OnSendMessageResult,(ULONG_PTR)&msgRet);
			while(!msgRet.bRet)
			{
				if(::PeekMessage(&msg, m_hLocalId, UM_CALL_FUN, UM_CALL_FUN, PM_REMOVE))
				{
					if(msg.message == WM_QUIT)
					{
						PostQuitMessage((int)msg.wParam);
						return false;
					}
					DispatchMessage(&msg);
				}
				if(!IsWindow(m_hRemoteId))
					break;
			}
		}

		if (msgRet.ret!=0)
		{
			pBuf->Seek(IShareBuffer::seek_set,dwPos);//output param must be follow input params.
			BOOL bRet = FromStream4Output(pParam,&m_sendBuf);
			assert(bRet);
		}
		//clear params.
		m_nCallStack--;

		return msgRet.ret!=0;
	}

	BOOL SIpcHandle::IsConnected() const
    {
        return m_hRemoteId!=0;
    }

    void SIpcHandle::SetIpcConnection(IIpcConnection *pConn)
	{
		m_pConn = pConn;
	}

	IIpcConnection * SIpcHandle::GetIpcConnection() const
	{
		return m_pConn;
	}

	ULONG_PTR SIpcHandle::GetLocalId() const
	{
		return (ULONG_PTR)m_hRemoteId;
	}

	ULONG_PTR SIpcHandle::GetRemoteId() const
	{
		return (ULONG_PTR)m_hRemoteId;
	}

	IShareBuffer * SIpcHandle::GetSendBuffer()
	{
		return &m_sendBuf;
	}

	IShareBuffer * SIpcHandle::GetRecvBuffer()
	{
		return &m_recvBuf;
	}

	static const BYTE KInputFlag= 0xf0;
	static const BYTE KOutputFlag= 0xf1;

	BOOL SIpcHandle::ToStream4Input(IFunParams * pParams,IShareBuffer * pBuf) const
	{
		UINT uId = pParams->GetID();
		pBuf->Write(&uId,sizeof(UINT));
		pBuf->Write(&KInputFlag,1);
		SParamStream ps(pBuf);
		pParams->ToStream4Input(ps);
		return TRUE;
	}

	BOOL SIpcHandle::FromStream4Input(IFunParams * pParams,IShareBuffer * pBuf) const
	{
		UINT uFunId=0;
		pBuf->Read(&uFunId,sizeof(UINT));
		assert(uFunId == pParams->GetID());
		BYTE flag=0;
		pBuf->Read(&flag,1);
		assert(flag == KInputFlag);
		SParamStream ps(pBuf);
		pParams->FromStream4Input(ps);
		return TRUE;
	}

	BOOL SIpcHandle::ToStream4Output(IFunParams * pParams,IShareBuffer * pBuf) const
	{
		UINT uId = pParams->GetID();
		pBuf->Write(&uId,sizeof(UINT));
		pBuf->Write(&KOutputFlag,1);
		SParamStream ps(pBuf);
		pParams->ToStream4Output(ps);
		return TRUE;
	}

	BOOL SIpcHandle::FromStream4Output(IFunParams * pParams,IShareBuffer * pBuf) const
	{
		UINT uFunId=0;
		pBuf->Read(&uFunId,sizeof(UINT));
		assert(uFunId == pParams->GetID());
		BYTE flag=0;
		pBuf->Read(&flag,1);
		assert(flag == KOutputFlag);
		SParamStream ps(pBuf);
		pParams->FromStream4Output(ps);
		return TRUE;
	}


	///////////////////////////////////////////////////////////////////////
	SIpcServer::SIpcServer() 
		:m_pCallback(NULL) 
		, m_hSvr(0)
	{}

	LRESULT SIpcServer::OnMessage(ULONG_PTR idLocal, UINT uMsg, WPARAM wp, LPARAM lp,BOOL &bHandled)
	{
		bHandled = FALSE;
		if ((HWND)idLocal != m_hSvr)
			return 0;
		if (UM_CALL_FUN != uMsg)
			return 0;
		bHandled = TRUE;
		if (wp == FUN_ID_CONNECT)
			return OnConnect((HWND)lp);
		else if (wp == FUN_ID_DISCONNECT)
			return OnDisconnect((HWND)lp);
        else
        {
            HWND hClient = (HWND)lp;
            std::map<HWND, IIpcConnection *>::iterator it = m_mapClients.find(hClient);
            if (it == m_mapClients.end())
                return 0;
            BOOL bHandled = FALSE;
            return it->second->GetIpcHandle()->OnMessage((ULONG_PTR)m_hSvr, uMsg, wp, lp, bHandled);
        }
	}

	void SIpcServer::EnumClient(FunEnumConnection funEnum, ULONG_PTR data)
	{
		std::map<HWND, IIpcConnection *> map2 = m_mapClients;
		std::map<HWND, IIpcConnection *>::iterator it = map2.begin();
		while (it != map2.end())
		{
			it->second->AddRef();
			it++;
		}

		it = map2.begin();
		while (it != map2.end())
		{
            if (!funEnum(it->second, data))
                break;
			it++;
		}
        it = map2.begin();
        while (it != map2.end())
        {
            it->second->Release();
            it++;
        }
	}

	LRESULT SIpcServer::OnConnect(HWND hClient)
	{
		if (m_mapClients.find(hClient) != m_mapClients.end()) 
			return 0; // existed.
		if (hClient == m_hSvr)
			return 0; // must be different hwnd.
		SAutoRefPtr<IIpcHandle> pIpcHandle;
		pIpcHandle.Attach(new SIpcHandle);

		IIpcConnection *pConn = NULL;
		ULONG_PTR connId = m_pCallback->OnNewConnection(pIpcHandle, &pConn);
		assert(pConn);
		pIpcHandle->SetIpcConnection(pConn);

		void * pSa = m_pCallback->GetSecurityAttr();
		if (!pIpcHandle->InitShareBuf(connId, (ULONG_PTR)hClient, pConn->GetBufSize()* pConn->GetStackSize(), pSa))
		{
			m_pCallback->ReleaseSecurityAttr(pSa);
			pConn->Release();
			return 0;
		}
		m_pCallback->ReleaseSecurityAttr(pSa);

		m_mapClients[hClient] = pConn;
		m_pCallback->OnConnected(pConn);
		return connId;
	}

	LRESULT SIpcServer::OnDisconnect(HWND hClient)
	{
		if (m_mapClients.find(hClient) == m_mapClients.end())
			return 0;

		IIpcConnection *pClient = m_mapClients[hClient];
		m_pCallback->OnDisconnected(pClient);

		pClient->Release();
		m_mapClients.erase(hClient);
		return 1;
	}

	HRESULT SIpcServer::Init(ULONG_PTR idSvr, IIpcSvrCallback * pCallback)
	{
		HWND hSvr = (HWND)idSvr;
		if (!IsWindow(hSvr))
			return E_INVALIDARG;
		if (m_hSvr != 0)
			return E_UNEXPECTED;

		m_hSvr = hSvr;
		m_pCallback = pCallback;
		return S_OK;
	}

	void SIpcServer::CheckConnectivity()
	{
		std::map<HWND, IIpcConnection *>::iterator it = m_mapClients.begin();
		while (it != m_mapClients.end())
		{
			if (!::IsWindow(it->first))
			{
				m_pCallback->OnDisconnected(it->second);
				it->second->Release();
				it = m_mapClients.erase(it);
			}
			else
			{
				it++;
			}
		}
	}

	BOOL SIpcServer::FindConnection(ULONG_PTR idConn)
	{
		return m_mapClients.find((HWND)idConn)!=m_mapClients.end();
    }

    BOOL SIpcServer::Disconnect(ULONG_PTR idConn)
    {
        CLIENTMAP::iterator it = m_mapClients.find((HWND)idConn);
        if (it == m_mapClients.end())
            return FALSE;
        it->second->GetIpcHandle()->Disconnect();
        it->second->Release();
        m_mapClients.erase(it);
        return TRUE;
    }

	////////////////////////////////////////////////////////////////////////
	HRESULT SIpcFactory::CreateIpcServer(IIpcServer ** ppServer)
	{
		*ppServer = new SIpcServer;
		return S_OK;
	}
	HRESULT SIpcFactory::CreateIpcHandle(IIpcHandle ** ppHandle)
	{
		*ppHandle = new SIpcHandle;
		return S_OK;
	}

	namespace IPC
	{
		SOUI_COM_C BOOL SOUI_COM_API  SCreateInstance(IObjRef ** ppIpcFactory)
		{
			*ppIpcFactory = new SIpcFactory();
			return TRUE;
		}
	} // namespace IPC
SNSEND

EXTERN_C BOOL Ipc_SCreateInstance(IObjRef **ppIpcFactory)
{
	return SNS::IPC::SCreateInstance(ppIpcFactory);
}


 
