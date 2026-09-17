#include "stdafx.h"
#include "WsClientListener.h"

WsClientListener::WsClientListener():m_bMsgBlock(FALSE){

}

WsClientListener::~WsClientListener(){

}

void WsClientListener::PumpMessage() {
    while (!m_bMsgBlock && !m_lstPendingMsg.empty()) {
        auto msg = m_lstPendingMsg.front();
        m_lstPendingMsg.pop_front();
        _OnMessage(msg.first, msg.second);
    }
}

BOOL WsClientListener::OnMessage(DWORD dwType, std::shared_ptr<std::vector<BYTE> > data)
{
    m_lstPendingMsg.push_back(std::make_pair(dwType, data));
    PumpMessage();
    return TRUE;
}

void WsClientListener::BLockReceive(BOOL bBlock){
    m_bMsgBlock=bBlock;
    if(!bBlock){
        PumpMessage();
    }
}