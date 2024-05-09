#include "UiState.h"
#include <helper/SCriticalSection.h>
#include <assert.h>
#include "uimsg.h"

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
    wm_window = SUiState::internAtom(connection,0,"WM_WINDOWS");
}

SThreadUiState::~SThreadUiState()
{
    if (!connection)
    {
        return;
    }

    for(auto it:m_msgQueue){
        free(it);
    }
    m_msgQueue.clear();

    xcb_disconnect(connection);
}


bool SThreadUiState::update(){
    int evtCnt=0;
    xcb_generic_event_t* e;
    while (xcb_generic_event_t* e = xcb_poll_for_event(connection))
    {        
        pushEvent(e);
        free(e);
        evtCnt++;
    }
    return evtCnt>0;
}

void SThreadUiState::pushEvent(xcb_generic_event_t *event){
    uint8_t event_code = event->response_type & 0x7f;
    UiMsg *pMsg = nullptr;
    switch (event_code)
    {
    case XCB_CLIENT_MESSAGE:
    {
        xcb_client_message_event_t *client_message_event = (xcb_client_message_event_t *) event;
        if(client_message_event->type == wm_window)
        {
            pMsg = new UiMsg;
            pMsg->hwnd=client_message_event->window;
            pMsg->message = client_message_event->data.data32[0];
            pMsg->wParam = MAKEWPARAM(client_message_event->data.data32[1],client_message_event->data.data32[2]);
            pMsg->lParam = MAKELPARAM(client_message_event->data.data32[3],client_message_event->data.data32[4]);
        }
        else if(client_message_event->data.data32[0] == wm_delete_window_atom){
            pMsg = new UiMsg;
            pMsg->message = WM_CLOSE;
            pMsg->hwnd = client_message_event->window;
            pMsg->wParam = pMsg->lParam = 0;
        }
        break;
    }
    case XCB_CONFIGURE_NOTIFY:
    {}
    break;
    default:
        break;
    }
    if(pMsg){
        m_msgQueue.push_back(pMsg);
    }
}


SNSEND
