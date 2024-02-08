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
    assert(m_trdStates.IsEmpty());
    SPOSITION pos = m_trdStates.GetStartPosition();
    while (pos)
    {
        SThreadUiState *pstate = m_trdStates.GetNextValue(pos);
        delete pstate;
    }
    m_trdStates.RemoveAll();
}

bool SUiState::init(int screenNum)
{
    SAutoWriteLock autoLock(&m_rwLock);
    pthread_t tid = pthread_self();
    if (m_trdStates.Lookup(tid) != NULL)
    {
        return true;
    }
    SThreadUiState *state = new SThreadUiState(screenNum);
    if (!state->connection)
    {
        delete state;
        return false;
    }
    m_trdStates[tid] = state;
    return true;
}

void SUiState::uninit()
{
    SAutoWriteLock autoLock(&m_rwLock);
    pthread_t tid = pthread_self();
    auto it = m_trdStates.Lookup(tid);
    if (it == NULL)
    {
        return;
    }
    delete it->m_value;
    m_trdStates.RemoveAtPos(it);
}

SThreadUiState *SUiState::getThreadUiState()
{
    pthread_t tid = pthread_self();
    {
        SAutoReadLock autoLock(&m_rwLock);
        auto it = m_trdStates.Lookup(tid);
        if (it != NULL)
        {
            return it->m_value;
        }
    }
    {
        //not found
        SAutoWriteLock autoLock(&m_rwLock);
        SThreadUiState * state = new SThreadUiState(0);        
        m_trdStates[tid]=state;
        return state;
    }
}

//---------------------------------------------------------------------
xcb_atom_t SThreadUiState::internAtom(xcb_connection_t *connection, uint8_t onlyIfExist, const char *atomName)
{
    xcb_intern_atom_cookie_t cookie = xcb_intern_atom(connection, onlyIfExist, strlen(atomName), atomName);
    xcb_intern_atom_reply_t *reply = xcb_intern_atom_reply(connection, cookie, NULL);
    xcb_atom_t atom = XCB_ATOM_NONE;

    if (reply)
    {
        atom = reply->atom;
        free(reply);
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

    wm_protocols_atom = internAtom(connection, 1, "WM_PROTOCOLS");
    wm_delete_window_atom = internAtom(connection, 0, "WM_DELETE_WINDOW");
    wm_stat_atom = internAtom(connection, 0, "_NET_WM_STATE");
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
    m_mapWnd.RemoveKey(hwnd);
}

SNativeWnd *SThreadUiState::GetNativeWndFromHwnd(HWND hwnd)
{
    auto it = m_mapWnd.Lookup(hwnd);
    if(!it)
    {
        assert(false);
        return nullptr;
    }
    return it->m_value;
}

SNSEND
