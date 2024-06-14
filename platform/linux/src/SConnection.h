#ifndef _UISTATE_H_
#define _UISTATE_H_

#include <platform.h>
#include <sdef.h>
#include <helper/SNoCopyable.hpp>
#include <helper/SRwLock.h>
#include <map>
#include <list>
#include <pthread.h>
#include <xcb/xcb.h>


#define STR_ATOM(atom_name,onlyExist) SOUI::SConnMgr::instance()->atom(atom_name,onlyExist)
#define ID_ATOM(id,onlyExist) STR_ATOM(#id,onlyExist)

SNSBEGIN
struct UiMsg;
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
    xcb_atom_t wm_window;
public:
    bool update();
    BOOL peekMsg(THIS_ LPMSG pMsg, HWND  hWnd, UINT wMsgFilterMin, UINT wMsgFilterMax, UINT  wRemoveMsg);

private:
    void pushEvent(xcb_generic_event_t *e);
private:
    std::list<UiMsg *> m_msgQueue;
};


class SConnMgr : SNoCopyable{
    SRwLock m_rwLock;
    std::map<pthread_t,SConnection*> m_trdStates;
    friend class SConnection;
public:
    static SConnMgr * instance();
    static void free();

    static xcb_atom_t internAtom(xcb_connection_t *connection, uint8_t onlyIfExist, const char *atomName);
public:
    xcb_atom_t atom(const char *name,bool onlyIfExist=false);
    SConnection * getConnection(int tid=0,int screenNum=0);
private:
    void clearThreadUiState(SConnection *pObj);

    SConnMgr(){}
    ~SConnMgr();
};

SNSEND

#endif//_UISTATE_H_