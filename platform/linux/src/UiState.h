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


#define STR_ATOM(atom_name,onlyExist) SOUI::SUiState::instance()->atom(atom_name,onlyExist)
#define ID_ATOM(id,onlyExist) STR_ATOM(#id,onlyExist)

SNSBEGIN
struct UiMsg;
class SThreadUiState : SNoCopyable{
public:
  SThreadUiState(int screenNum);
  ~SThreadUiState();
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


class SUiState : SNoCopyable{
    SRwLock m_rwLock;
    std::map<pthread_t,SThreadUiState*> m_trdStates;
    friend class SThreadUiState;
public:
    static SUiState * instance();
    static void free();

    static xcb_atom_t internAtom(xcb_connection_t *connection, uint8_t onlyIfExist, const char *atomName);
public:
    xcb_atom_t atom(const char *name,bool onlyIfExist=false);
    SThreadUiState * getThreadUiState(int screenNum=0);
    SThreadUiState * getThreadUiState2(int tid,int screenNum=0);
private:
    void clearThreadUiState(SThreadUiState *pObj);

    SUiState(){}
    ~SUiState();
};

SNSEND

#endif//_UISTATE_H_