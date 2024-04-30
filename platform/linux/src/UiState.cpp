#include "UiState.h"
#include <helper/SCriticalSection.h>
#include <assert.h>
SNSBEGIN

static SUiState *s_uiState = NULL;
static SCriticalSection s_cs;

SUiState *SUiState::instance()
{
    if (!s_uiState)
    {
        SAutoLock lock(s_cs);
        if (!s_uiState)
        {
            s_uiState = new SUiState();
        }
    }
    return s_uiState;
}

void SUiState::free()
{
    SAutoLock lock(s_cs);
    if (s_uiState)
    {
        delete s_uiState;
        s_uiState = NULL;
    }
}

//----------------------------------------------------------
SUiState::~SUiState()
{
    SAutoWriteLock autoLock(&m_rwLock);
    assert(m_trdStates.empty());
    auto it = m_trdStates.begin();
    while(it!=m_trdStates.end()){
        delete it->second;
        it++;
    }
    m_trdStates.clear();
}

void SUiState::clearThreadUiState(SThreadUiState *pObj)
{
    SAutoWriteLock autoLock(&m_rwLock);
    pthread_t tid = pthread_self();
    auto it = m_trdStates.find(tid);
    if (it == m_trdStates.end())
    {
        return;
    }
    assert(it->second == pObj);
    delete it->second;
    m_trdStates.erase(it);
}

SThreadUiState * SUiState::getThreadUiState2(int tid_,int screenNum){
   pthread_t tid = pthread_t(tid_);
    {
        SAutoReadLock autoLock(&m_rwLock);
        auto it = m_trdStates.find(tid);
        if (it != m_trdStates.end())
        {
            return it->second;
        }
    }
    {
        //not found
        SAutoWriteLock autoLock(&m_rwLock);
        SThreadUiState * state = new SThreadUiState(screenNum);        
        m_trdStates[tid]=state;
        return state;
    }
}

SThreadUiState *SUiState::getThreadUiState(int screenNum)
{
    pthread_t tid = pthread_self();
    return getThreadUiState2(tid,screenNum);
}

xcb_atom_t SUiState::atom(const char *name,bool onlyIfExist){
    SThreadUiState *trdUiState = getThreadUiState();
    if(!trdUiState)
        return 0;
    return internAtom(trdUiState->connection,onlyIfExist?1:0,name);
}

//---------------------------------------------------------------------
xcb_atom_t SUiState::internAtom(xcb_connection_t *connection, uint8_t onlyIfExist, const char *atomName)
{
    xcb_intern_atom_cookie_t cookie = xcb_intern_atom(connection, onlyIfExist, strlen(atomName), atomName);
    xcb_intern_atom_reply_t *reply = xcb_intern_atom_reply(connection, cookie, NULL);
    xcb_atom_t atom = XCB_ATOM_NONE;

    if (reply)
    {
        atom = reply->atom;
        ::free(reply);
    }

    return atom;
}

SThreadUiState::SThreadUiState(int screenNum)
{
    connection = xcb_connect(nullptr, &screenNum);
    if (xcb_connection_has_error(connection) > 0)
    {
        connection = NULL;
        return;
    }

    /* Get the screen whose number is screenNum */

    const xcb_setup_t *setup = xcb_get_setup(connection);
    xcb_screen_iterator_t iter = xcb_setup_roots_iterator(setup);

    // we want the screen at index screenNum of the iterator
    for (int i = 0; i < screenNum; ++i)
    {
        xcb_screen_next(&iter);
    }
    if (!iter.data)
    {
        // get the first screen
        iter = xcb_setup_roots_iterator(setup);
    }
    screen = iter.data;

    wm_protocols_atom = SUiState::internAtom(connection, 1, "WM_PROTOCOLS");
    wm_delete_window_atom = SUiState::internAtom(connection, 0, "WM_DELETE_WINDOW");
    wm_stat_atom = SUiState::internAtom(connection, 0, "_NET_WM_STATE");
}

SThreadUiState::~SThreadUiState()
{
    if (!connection)
    {
        return;
    }
    xcb_disconnect(connection);
}

void SThreadUiState::onWndCreate(HWND hwnd, SNativeWnd *pWnd)
{
    m_mapWnd[hwnd] = pWnd;
}

void SThreadUiState::onWndDestroy(HWND hwnd)
{
    m_mapWnd.erase(hwnd);
    if(m_mapWnd.empty()){
        SUiState::instance()->clearThreadUiState(this);
    }
}

SNativeWnd *SThreadUiState::GetNativeWndFromHwnd(HWND hwnd)
{
    auto it = m_mapWnd.find(hwnd);
    if(it == m_mapWnd.end())
    {
        assert(false);
        return nullptr;
    }
    return it->second;
}

SNSEND
