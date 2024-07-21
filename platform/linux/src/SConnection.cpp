#include "SConnection.h"
#include <helper/SCriticalSection.h>
#include <assert.h>
#include <functional>
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
    auto it = m_conns.begin();
    while(it!=m_conns.end()){
        delete it->second;
        it++;
    }
    m_conns.clear();
}

void SConnMgr::removeConn(SConnection *pObj)
{
    SAutoWriteLock autoLock(&m_rwLock);
    pthread_t tid = pthread_self();
    auto it = m_conns.find(tid);
    if (it == m_conns.end())
    {
        return;
    }
    assert(it->second == pObj);
    delete it->second;
    m_conns.erase(it);
}

SConnection * SConnMgr::getConnection(pthread_t tid_,int screenNum){
   pthread_t tid = tid_!=0? tid_:pthread_self();   
    {
        SAutoReadLock autoLock(&m_rwLock);
        auto it = m_conns.find(tid);
        if (it != m_conns.end())
        {
            return it->second;
        }
    }
    {
        //not found
        SAutoWriteLock autoLock(&m_rwLock);
        SConnection * state = new SConnection(screenNum);        
        m_conns[tid]=state;
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
    wm_delete_window_atom = SConnMgr::internAtom(connection, 1, "WM_DELETE_WINDOW");
    wm_stat_atom = SConnMgr::internAtom(connection, 1, "_NET_WM_STATE");
    wm_stat_hidden_atom = SConnMgr::internAtom(connection,1,"_NET_WM_STATE_HIDDEN");
    wm_stat_enable_atom = SConnMgr::internAtom(connection,1,"_NET_WM_STATE_DEMANDS_ATTENTION");

    m_bQuit=false;
    m_msgPeek = nullptr;
    m_trdEvtReader = std::move(std::thread(std::bind(&readProc, this)));
}

SConnection::~SConnection()
{
    if (!connection)
    {
        return;
    }
    m_bQuit = true;
    xcb_disconnect(connection);

    m_trdEvtReader.join();

    for(auto it:m_msgQueue){
        delete it;
    }
    m_msgQueue.clear();
    if(m_msgPeek){
        delete m_msgPeek;
        m_msgPeek = nullptr;
    }
}


bool SConnection::update(){
    std::unique_lock<std::mutex> lock(m_mutex4Evt);
    m_varCondition.wait(lock, [&]{return !m_evtQueue.empty(); });

    for(auto it:m_evtQueue){
        pushEvent(it);
        free(it);
    }
    bool bRet = !m_evtQueue.empty();
    m_evtQueue.clear();
    return bRet;
}

BOOL SConnection::peekMsg(THIS_ LPMSG pMsg, HWND  hWnd, UINT wMsgFilterMin, UINT wMsgFilterMax, UINT  wRemoveMsg){
    std::unique_lock<std::recursive_mutex> lock(m_mutex4Msg);
    for(auto it = m_msgQueue.begin();it!=m_msgQueue.end();it++){
        BOOL bMatch=TRUE;
        Msg *msg = (*it);
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
            if(m_msgPeek){
                delete m_msgPeek;
                m_msgPeek = nullptr;
            }
            if(wRemoveMsg & PM_NOREMOVE)
                m_msgPeek = msg->clone();
            else
            {
                m_msgQueue.erase(it);
                m_msgPeek = msg;
            }
            memcpy(pMsg,m_msgPeek,sizeof(MSG));
            
            return TRUE;
        }
       
    }
    return FALSE;
}

void SConnection::postMsg(HWND hWnd, UINT message, WPARAM wp, LPARAM lp)
{
    std::unique_lock<std::recursive_mutex> lock(m_mutex4Msg);
    Msg *pMsg = new Msg;
    pMsg->hwnd = hWnd;
    pMsg->message = message;
    pMsg->wParam = wp;
    pMsg->lParam = lp;
    m_msgQueue.push_back(pMsg);
}

bool SConnection::pushEvent(xcb_generic_event_t *event){
    uint8_t event_code = event->response_type & 0x7f;
    Msg *pMsg = nullptr;
    bool ret = false;
    switch (event_code)
    {
    case XCB_EXPOSE:
    {
        xcb_expose_event_t* expose = (xcb_expose_event_t*)event;
        RECT rc = {expose->x,expose->y,expose->x+expose->width,expose->y+expose->height};
        HRGN hrgn = CreateRectRgnIndirect(&rc);
        std::unique_lock<std::recursive_mutex> lock(m_mutex4Msg);
        for(auto it = m_msgQueue.begin();it!=m_msgQueue.end();it++){
                if((*it)->message == WM_PAINT && (*it)->hwnd == expose->window)
                {
                    MsgPaint *oldMsg = (MsgPaint*)(*it);
                    RECT rcOld,rcNew;
                    GetRgnBox(oldMsg->rgn, &rcOld);
                    CombineRgn(oldMsg->rgn, oldMsg->rgn, hrgn, RGN_OR);
                    GetRgnBox(oldMsg->rgn, &rcNew);
                    DeleteObject(hrgn);
                    return true;
                }
        }
        MsgPaint *pMsgPaint = new MsgPaint(hrgn);
        pMsgPaint->hwnd = expose->window;
        pMsgPaint->message = WM_PAINT;
        pMsgPaint->wParam = 0;
        pMsgPaint->lParam = (LPARAM)pMsgPaint->rgn;
        pMsg = pMsgPaint;
        break;
    }
    case XCB_CONFIGURE_NOTIFY:
    {
        xcb_configure_notify_event_t *e2 = (xcb_configure_notify_event_t*)event;
        std::unique_lock<std::recursive_mutex> lock(m_mutex4Msg);
        for(auto it = m_msgQueue.begin();it!=m_msgQueue.end();it++){
                if((*it)->message == WM_SIZE && (*it)->hwnd == e2->window)
                {
                    m_msgQueue.erase(it);
                    break;
                }
        }

        MsgWndPosChanged *pMsgPosChanged = new MsgWndPosChanged;
        pMsgPosChanged->hwnd = e2->window;
        pMsgPosChanged->message = WM_WINDOWPOSCHANGED;
        pMsgPosChanged->wParam=0;
        pMsgPosChanged->pos.hwnd = e2->window;
        pMsgPosChanged->pos.hwndInsertAfter = e2->above_sibling;
        pMsgPosChanged->pos.x = e2->x;
        pMsgPosChanged->pos.y = e2->y;
        pMsgPosChanged->pos.cx = e2->width;
        pMsgPosChanged->pos.cy = e2->height;
        pMsgPosChanged->lParam = (LPARAM)&pMsgPosChanged->pos;

        pMsg = pMsgPosChanged;
        break;
    }
    case XCB_CLIENT_MESSAGE:
    {
        xcb_client_message_event_t *client_message_event = (xcb_client_message_event_t *) event;
        if(client_message_event->data.data32[0] == wm_delete_window_atom){
            pMsg = new Msg;
            pMsg->message = WM_CLOSE;
            pMsg->hwnd = client_message_event->window;
            pMsg->wParam = pMsg->lParam = 0;
        }else if(client_message_event->type == wm_stat_hidden_atom){
            pMsg = new Msg;
            pMsg->message = WM_SHOWWINDOW;
            pMsg->hwnd =client_message_event->window;
            pMsg->wParam = client_message_event->data.data32[0];
            pMsg->lParam = 0;
        }
        else if(client_message_event->type == wm_stat_enable_atom){
            pMsg = new Msg;
            pMsg->message = WM_ENABLE;
            pMsg->hwnd =client_message_event->window;
            pMsg->wParam = client_message_event->data.data32[0];
            pMsg->lParam = 0;
        }
        break;
    }
    break;
    default:
        break;
    }
    if(pMsg){
        std::unique_lock<std::recursive_mutex> lock(m_mutex4Msg);
        m_msgQueue.push_back(pMsg);
    }
    return ret;
}

void* SConnection::readProc(void *p)
{
    SConnection * _this = static_cast<SConnection*>(p);
    _this->_readProc();
    return p;
}

void SConnection::_readProc()
{
    while (!m_bQuit){
        xcb_generic_event_t * event = xcb_wait_for_event(connection);
        if(!event){
            m_bQuit = true;
            break;
        }
        {
            std::unique_lock<std::mutex> lock(m_mutex4Evt);
            m_evtQueue.push_back(event);
            m_varCondition.notify_one();
        }
    }

    m_mutex4Evt.lock();
    for (auto it : m_evtQueue)
    {
        free(it);
    }
    m_evtQueue.clear();
    m_mutex4Evt.unlock();
}

SNSEND
