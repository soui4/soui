#include "SConnection.h"
#include <helper/SCriticalSection.h>
#include <assert.h>
#include "uimsg.h"

SNSBEGIN

static SConnMgr *s_connMgr = NULL;
static SCriticalSection s_cs;

SConnMgr *SConnMgr::instance()
{
    if (!s_connMgr)
    {
        SAutoLock lock(s_cs);
        if (!s_connMgr)
        {
            s_connMgr = new SConnMgr();
        }
    }
    return s_connMgr;
}

void SConnMgr::free()
{
    SAutoLock lock(s_cs);
    if (s_connMgr)
    {
        delete s_connMgr;
        s_connMgr = NULL;
    }
}

//----------------------------------------------------------
SConnMgr::~SConnMgr()
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

void SConnMgr::clearThreadUiState(SConnection *pObj)
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

SConnection * SConnMgr::getConnection(int tid_,int screenNum){
   pthread_t tid = tid_!=0? pthread_t(tid_):pthread_self();   
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
        SConnection * state = new SConnection(screenNum);        
        m_trdStates[tid]=state;
        return state;
    }
}

xcb_atom_t SConnMgr::atom(const char *name,bool onlyIfExist){
    SConnection *trdUiState = getConnection();
    if(!trdUiState)
        return 0;
    return internAtom(trdUiState->connection,onlyIfExist?1:0,name);
}

//---------------------------------------------------------------------
xcb_atom_t SConnMgr::internAtom(xcb_connection_t *connection, uint8_t onlyIfExist, const char *atomName)
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

SConnection::SConnection(int screenNum)
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

    wm_protocols_atom = SConnMgr::internAtom(connection, 1, "WM_PROTOCOLS");
    wm_delete_window_atom = SConnMgr::internAtom(connection, 0, "WM_DELETE_WINDOW");
    wm_stat_atom = SConnMgr::internAtom(connection, 0, "_NET_WM_STATE");
    wm_window = SConnMgr::internAtom(connection,0,"WM_WINDOWS");
}

SConnection::~SConnection()
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


bool SConnection::update(){
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

BOOL SConnection::peekMsg(THIS_ LPMSG pMsg, HWND  hWnd, UINT wMsgFilterMin, UINT wMsgFilterMax, UINT  wRemoveMsg){
    for(auto it = m_msgQueue.begin();it!=m_msgQueue.end();it++){
        BOOL bMatch=TRUE;
        UiMsg *msg = (*it);
        do{
            if(msg->message==WM_QUIT)
                break;
            if(msg->hwnd != hWnd && hWnd != 0)
            {
                bMatch=FALSE;
                break;
            }
            if(wMsgFilterMin==0 && wMsgFilterMax==0)
                break;
            if(wMsgFilterMin<= msg->message && wMsgFilterMax>=msg->message)
                break;
            bMatch=FALSE;
        }while(false);
        if(bMatch){
            pMsg->hwnd = msg->hwnd;
            pMsg->message = msg->message;
            pMsg->wParam = msg->wParam;
            pMsg->lParam = msg->lParam;
            delete msg;
            m_msgQueue.erase(it);
            return TRUE;
        }
       
    }
    return FALSE;
}

void SConnection::pushEvent(xcb_generic_event_t *event){
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
