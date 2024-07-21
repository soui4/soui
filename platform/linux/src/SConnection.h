#ifndef _SCONN_H_
#define _SCONN_H_

#include <platform.h>
#include <sdef.h>
#include <helper/SNoCopyable.hpp>
#include <helper/SRwLock.h>
#include <map>
#include <list>
#include <pthread.h>
#include <xcb/xcb.h>
#include <mutex>
#include <thread>
#include <atomic>
#include <condition_variable>

#define STR_ATOM(atom_name,onlyExist) SOUI::SConnMgr::instance()->atom(atom_name,onlyExist)
#define ID_ATOM(id,onlyExist) STR_ATOM(#id,onlyExist)

SNSBEGIN
struct Msg;
class SConnection : SNoCopyable{
public:
  SConnection(int screenNum);
  ~SConnection();
public:
    xcb_connection_t* connection;
    xcb_screen_t* screen;
    xcb_atom_t wm_delete_window_atom;
    xcb_atom_t wm_protocols_atom;
    xcb_atom_t wm_stat_atom;
    xcb_atom_t wm_stat_hidden_atom;
    xcb_atom_t wm_stat_enable_atom;
public:
    bool update();
    BOOL peekMsg(LPMSG pMsg, HWND  hWnd, UINT wMsgFilterMin, UINT wMsgFilterMax, UINT  wRemoveMsg);
    void postMsg(HWND hWnd,UINT message,WPARAM wp,LPARAM lp);
private:
    bool pushEvent(xcb_generic_event_t *e);

    static void* readProc(void *p);
    void _readProc();

private:
    std::condition_variable m_varCondition;
    std::mutex m_mutex4Evt;
    std::list<xcb_generic_event_t*> m_evtQueue;

    std::recursive_mutex m_mutex4Msg;
    std::list<Msg *> m_msgQueue;
    Msg * m_msgPeek;
    
    std::thread      m_trdEvtReader;
    std::atomic<bool> m_bQuit;
};


class SConnMgr : SNoCopyable{
    SRwLock m_rwLock;
    std::map<pthread_t,SConnection*> m_conns;
    friend class SConnection;
public:
    static SConnMgr * instance();
    static void free();

    static xcb_atom_t internAtom(xcb_connection_t *connection, uint8_t onlyIfExist, const char *atomName);
public:
    xcb_atom_t atom(const char *name,bool onlyIfExist=false);
    SConnection * getConnection(pthread_t tid=0,int screenNum=0);
private:
    void removeConn(SConnection *pObj);

    SConnMgr(){}
    ~SConnMgr();
};

SNSEND

#endif//_SCONN_H_