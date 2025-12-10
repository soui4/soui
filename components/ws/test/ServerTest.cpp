#include <iostream>
#include <chrono>
#include <map>
#include <interface/ws-i.h>
#include <helper/obj-ref-impl.hpp>
#include <tchar.h>
#include <com-loader.hpp>

using namespace SOUI;

typedef std::map<int, IConnection*> Group;

class SvrListener : public TObjRefImpl<ISvrListener> {
    std::map<int, Group> m_conns;
  public:
      Group* id2group(int id) {
          auto it = m_conns.find(id);
          if (it != m_conns.end())
              return &it->second;
          else
              return nullptr;
      }

      void boardcast(Group * g, const void* buf, int len, bool bBinary) {
          if (g) {
              for (auto it : (*g)) {
                  if (bBinary)
                      it.second->sendBinary(buf, len);
                  else
                      it.second->sendText((const char*)buf, len);
              }
          }
      }

      void boardcast(int groupId, const void* buf, int len, bool bBinary) {
          if (groupId == -1) {
              for (auto it : m_conns) {
                  boardcast(&it.second, buf, len, bBinary);
              }
          }
          else {
              Group* g = id2group(groupId);
              boardcast(g, buf, len, bBinary);
          }
      }

    STDMETHODIMP_(BOOL) onConnected(ISvrConnection *pConn, const char *uriPath, const char *uriArgs) override
    {
        int groupId = 0, userId = 0;
        sscanf(uriArgs, "roomid=%d&userid=%d", &groupId, &userId);
        auto it = m_conns.find(groupId);
        if (it == m_conns.end()) {
            auto ret = m_conns.insert(std::make_pair(groupId, std::map<int, IConnection*>()));
            it = ret.first;
        }
        if (it->second.find(userId) == it->second.end()) {
            it->second.insert(std::make_pair(userId, pConn));
            pConn->AddRef();
            pConn->setId(userId);
            pConn->setGroupId(groupId);

            std::cout << "New Connection!" << std::endl;

            char msg[20];
            sprintf(msg, "group size, %d", (int)m_conns.size());
            boardcast(groupId,msg, -1, false);
            return true;
        }
        else {
            printf("err,groupId=%d,userId=%d\n",groupId,userId);
            return false;
        }
    }
    STDMETHODIMP_(void) onConnError(ISvrConnection *pConn, const char *errStr) override
    {
    }
    STDMETHODIMP_(void) onDisconnect(ISvrConnection *pConn) override
    {
        int groupId = pConn->getGroupId();
        int userId = pConn->getId();
        auto it = m_conns.find(groupId);
        if (it != m_conns.end()) {
            auto itUser = it->second.find(userId);
            if (itUser != it->second.end()) {
                it->second.erase(userId);
                pConn->Release();
                if (it->second.empty()) {
                    m_conns.erase(it);
                }
                char msg[100];
                sprintf(msg,"conn leave,groupId=%d,userId=%d", groupId, userId);
                boardcast(groupId, msg, -1, false);
            }
        }
    }

    STDMETHODIMP_(void) onDataSent(ISvrConnection *pConn, int nMsgId) override
    {
    }

    STDMETHODIMP_(void) onDataRecv(ISvrConnection *pConn, const void *data, int len, BOOL bBinary) override
    {
        std::cout << "Received: " << std::string((const char*)data, len).c_str() << std::endl;
        pConn->sendText("Echo4567890123");
    }
};

int main()
{
    SComLoader comLoader;
    {
        SvrListener svrListener;
        IWebsocket *pWs = NULL;
        IWsServer *pSvr = NULL;
        comLoader.CreateInstance(_T("ws"), (IObjRef **)&pWs);
        pSvr = pWs->CreateWsServer(&svrListener);
        int nPort = 3310;
        std::cout << "Started on port " << nPort << std::endl;
        SvrOption optoin = { FALSE, NULL, NULL };
        pSvr->start(nPort, "upgrade", optoin);
        bool bFinish = pSvr->wait(-1);
        std::cout << "server wait ret:" << bFinish << std::endl;
        pSvr->Release();
        pWs->Release();
    }
    std::cout << "server quit" << std::endl;

    system("pause");
}
