﻿#pragma once
#include <interface/sipcobj-i.h>
#include <helper/obj-ref-impl.hpp>
#include <map>
#include "ShareMemBuffer.h"


SNSBEGIN

	class SIpcHandle : public TObjRefImpl<IIpcHandle>
	{
	public:
		SIpcHandle();
		virtual ~SIpcHandle();

	public:
        virtual BOOL IsConnected() const override;
		virtual void SetIpcConnection(IIpcConnection *pConn);

		virtual IIpcConnection * GetIpcConnection() const;

		virtual LRESULT OnMessage(ULONG_PTR idLocal, UINT uMsg, WPARAM wp, LPARAM lp, BOOL &bHandled);

		virtual HRESULT ConnectTo(ULONG_PTR idLocal, ULONG_PTR idSvr);

		virtual HRESULT Disconnect();

		virtual bool CallFun(IFunParams * pParam) const;

		virtual ULONG_PTR GetLocalId() const;

		virtual ULONG_PTR GetRemoteId() const;

		virtual IShareBuffer * GetSendBuffer() ;

		virtual IShareBuffer * GetRecvBuffer() ;

		virtual BOOL InitShareBuf(ULONG_PTR idLocal, ULONG_PTR idRemote, UINT nBufSize, void* pSa);

		virtual BOOL ToStream4Input(IFunParams * pParams,IShareBuffer * pBuf) const;

		virtual BOOL FromStream4Input(IFunParams * pParams,IShareBuffer * pBuf) const;

		virtual BOOL ToStream4Output(IFunParams * pParams,IShareBuffer * pBuf) const;

		virtual BOOL FromStream4Output(IFunParams * pParams,IShareBuffer * pBuf) const;
	protected:
		static void CALLBACK OnSendMessageResult(HWND hwnd, UINT msg, ULONG_PTR data, LRESULT res);

		HWND	m_hLocalId;
		mutable CShareMemBuffer	m_sendBuf;
		HWND	m_hRemoteId;
		CShareMemBuffer	m_recvBuf;
		IIpcConnection * m_pConn;
		mutable UINT	m_uCallSeq;
		mutable int     m_nCallStack;
		bool	m_bSameThread;
	};


	class SIpcServer : public TObjRefImpl<IIpcServer>
	{
	public:
		SIpcServer();


	public:
		// 通过 TObjRefImpl 继承
		virtual HRESULT Init(ULONG_PTR idSvr, IIpcSvrCallback * pCallback) override;
		virtual void CheckConnectivity() override;
		virtual LRESULT OnMessage(ULONG_PTR idLocal, UINT uMsg, WPARAM wp, LPARAM lp,BOOL &bHandled) override;
		virtual void EnumClient(FunEnumConnection funEnum, ULONG_PTR data) override;
		virtual BOOL FindConnection(ULONG_PTR idConn) override;
        virtual BOOL Disconnect(ULONG_PTR idConn) override;

	private:
		LRESULT OnConnect(HWND hClient);
		LRESULT OnDisconnect(HWND hClient);
	private:
		WNDPROC			  m_prevWndProc;
		IIpcSvrCallback * m_pCallback;
		HWND			  m_hSvr;
        typedef std::map<HWND, IIpcConnection *> CLIENTMAP;
        CLIENTMAP m_mapClients;
	};

	class SIpcFactory : public TObjRefImpl<IIpcFactory>
	{
	public:
		// 通过 TObjRefImpl 继承
		virtual HRESULT CreateIpcServer(IIpcServer ** ppServer) override;
		virtual HRESULT CreateIpcHandle(IIpcHandle ** ppHandle) override;
	};
	namespace IPC
	{
		SOUI_COM_C BOOL SOUI_COM_API SCreateInstance(IObjRef **ppIpcFactory);
	}

SNSEND

EXTERN_C BOOL SOUI_COM_API Ipc_SCreateInstance(IObjRef **ppIpcFactory);

