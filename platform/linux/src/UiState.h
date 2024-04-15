#include <platform.h>
#include <sdef.h>
#include <helper/SNoCopyable.hpp>
#include <helper/SRwLock.h>
#include <map>
#include <pthread.h>
#include <xcb/xcb.h>

using namespace std;

SNSBEGIN

class SNativeWnd;
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
public:
    void onWndCreate(HWND hwnd, SNativeWnd* pWnd);
    void onWndDestroy(HWND hwnd);
    SNativeWnd * GetNativeWndFromHwnd(HWND hwnd);

    static xcb_atom_t internAtom(xcb_connection_t *connection, uint8_t onlyIfExist, const char *atomName);
private:
    std::map<HWND,SNativeWnd*> m_mapWnd;
};

class SUiState : SNoCopyable{
    SRwLock m_rwLock;
    std::map<pthread_t,SThreadUiState*> m_trdStates;
    friend class SThreadUiState;
public:
    static SUiState * instance();
    static void free();

public:
    SThreadUiState * getThreadUiState(int screenNum=0);
private:
    void clearThreadUiState(SThreadUiState *pObj);

    SUiState(){}
    ~SUiState();
};

SNSEND