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
#include "sdc.h"
#define STR_ATOM(atom_name,onlyExist) SOUI::SConnMgr::instance()->atom(atom_name,onlyExist)
#define ID_ATOM(id,onlyExist) STR_ATOM(#id,onlyExist)

SNSBEGIN
struct Msg;

struct TimerInfo {
    UINT_PTR id;
    HWND hWnd;
    UINT elapse;
    UINT fireRemain;
    TIMERPROC proc;
};

class SConnection : SNoCopyable{
public:
   enum NetWmState {
        NetWmStateAbove = 0x1,
        NetWmStateBelow = 0x2,
        NetWmStateFullScreen = 0x4,
        NetWmStateMaximizedHorz = 0x8,
        NetWmStateMaximizedVert = 0x10,
        NetWmStateModal = 0x20,
        NetWmStateStaysOnTop = 0x40,
        NetWmStateDemandsAttention = 0x80
    };

  SConnection(int screenNum);
  ~SConnection();
public:
    xcb_connection_t* connection;
    xcb_screen_t* screen;
    xcb_atom_t WM_DELETE_WINDOW_ATOM;
    xcb_atom_t WM_PROTOCOLS_ATOM;
    xcb_atom_t _NET_WM_STATE_ATOM;
    xcb_atom_t _NET_WM_STATE_HIDDEN_ATOM;
    xcb_atom_t _NET_WM_STATE_MAXIMIZED_HORZ_ATOM;
    xcb_atom_t _NET_WM_STATE_MAXIMIZED_VERT_ATOM;
    
    xcb_atom_t _NET_WM_STATE_ABOVE_ATOM;
    xcb_atom_t _NET_WM_STATE_BELOW_ATOM;
    xcb_atom_t _NET_WM_STATE_FULLSCREEN_ATOM;
    xcb_atom_t _NET_WM_STATE_STAYS_ON_TOP_ATOM;
    xcb_atom_t _NET_WM_STATE_DEMANDS_ATTENTION_ATOM;

    xcb_atom_t _NET_WM_ICON;
    xcb_atom_t _NET_WM_ICON_NAME;
    //ICCCM window state
    xcb_atom_t WM_STATE_ATOM;
    xcb_atom_t WM_CLASS_ATOM;
    xcb_atom_t WM_NAME_ATOM;
    xcb_atom_t WM_CHANGE_STATE_ATOM;

public:
    bool update();
    BOOL peekMsg(LPMSG pMsg, HWND  hWnd, UINT wMsgFilterMin, UINT wMsgFilterMax, UINT  wRemoveMsg);
    void postMsg(HWND hWnd,UINT message,WPARAM wp,LPARAM lp);
    UINT_PTR SetTimer(HWND hWnd, UINT_PTR id, UINT uElapse,
        TIMERPROC proc);
    BOOL KillTimer(HWND hWnd,
        UINT_PTR id);
    HDC GetDC();
    BOOL ReleaseDC(HDC hdc);

    HWND SetCapture(HWND hCapture);
    BOOL ReleaseCapture();
    HWND GetCapture() const;

    HCURSOR SetCursor(HCURSOR cursor);
    HCURSOR LoadCursor(LPCSTR pszName);
    BOOL DestroyCursor(HCURSOR cursor);

    void SetTimerBlock(bool bBlock){
        m_bBlockTimer = bBlock;
    }

    HWND GetActiveWnd() const {
        return m_hWndActive;
    }

    HWND SetActiveWindow(HWND hWnd);

    HWND GetParentWnd(HWND hWnd) const;

    HWND GetWindow(HWND hWnd, int code) const;

    HWND WindowFromPoint(POINT pt, HWND hWnd) const;


    BOOL GetCursorPos(LPPOINT ppt) const;

private:
  static LPCSTR getStdCursorName(WORD wId);
  xcb_cursor_t createXcbCursor(HCURSOR cursor);
  uint32_t netWmStates(HWND hWnd);
  bool pushEvent(xcb_generic_event_t *e);

  static void *readProc(void *p);
  void _readProc();
private:
    std::condition_variable m_varCondition;
    std::mutex m_mutex4Evt;
    std::list<xcb_generic_event_t*> m_evtQueue;

    std::recursive_mutex m_mutex4Msg;
    std::list<Msg *> m_msgQueue;
    xcb_timestamp_t  m_tsPrevPress;
    xcb_timestamp_t  m_tsDoubleSpan;

    Msg * m_msgPeek;
    bool  m_bMsgNeedFree;
    std::thread      m_trdEvtReader;
    std::atomic<bool> m_bQuit;

    std::list<TimerInfo> m_lstTimer;
    bool m_bBlockTimer;

    HDC m_deskDC;
    HBITMAP m_deskBmp;
    HWND m_hWndCapture;
    HCURSOR m_hCursor;

    HWND m_hWndActive;
    std::map<HCURSOR,xcb_cursor_t> m_sysCursor;
    std::map<WORD,HCURSOR> m_stdCursor;
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