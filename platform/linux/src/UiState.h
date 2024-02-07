#include <platform.h>
#include <sdef.h>
#include <helper/SNoCopyable.hpp>
#include <helper/SRwLock.h>
#include <souicoll.h>
#include <pthread.h>
#include <xcb/xcb.h>

using namespace std;

SNSBEGIN

class SThreadUiState : SNoCopyable{
public:
    SThreadUiState(int screenNum);
    ~SThreadUiState();
public:
    xcb_connection_t* connection;
    xcb_screen_t* screen;
    xcb_atom_t wm_delete_window_atom;
    xcb_atom_t wm_protocols_atom;
};

class SUiState : SNoCopyable{
    SRwLock m_rwLock;
    SMap<pthread_t,SThreadUiState*> m_thdStates;
public:
    static SUiState * instance();
    static void free();

public:
    bool init(int screenNum=0);
    void uninit();
    SThreadUiState * getThreadUiState();

private:
    SUiState(){}
    ~SUiState();
};

SNSEND