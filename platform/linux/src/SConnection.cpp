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
        free(it);
    }
    m_msgQueue.clear();
}


bool SConnection::update(){
    std::unique_lock<std::recursive_mutex> lock(m_mutex);
    for(auto it:m_evtQueue){
        pushEvent(it);
        free(it);
    }
    bool bRet = !m_evtQueue.empty();
    m_evtQueue.clear();
    return bRet;
}

BOOL SConnection::peekMsg(THIS_ LPMSG pMsg, HWND  hWnd, UINT wMsgFilterMin, UINT wMsgFilterMax, UINT  wRemoveMsg){
    std::unique_lock<std::recursive_mutex> lock(m_mutex);
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

void triggerRedraw(xcb_connection_t* connection, xcb_window_t window) {
    xcb_expose_event_t exposeEvent;
    exposeEvent.response_type = XCB_EXPOSE;
    exposeEvent.window = window;
    exposeEvent.x = 0;
    exposeEvent.y = 0;
    exposeEvent.width = 0;  // 设置为0表示重绘整个窗口
    exposeEvent.height = 0; // 设置为0表示重绘整个窗口
    exposeEvent.count = 0;

    xcb_send_event(connection, false, window, XCB_EVENT_MASK_EXPOSURE, (const char*)&exposeEvent);
    //xcb_flush(connection);
}

void SConnection::postMsg(HWND hWnd, UINT message, WPARAM wp, LPARAM lp)
{
    std::unique_lock<std::recursive_mutex> lock(m_mutex);
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
    //printf("pushEvent code=%u\n",event_code);
    bool ret = false;
    switch (event_code)
    {
    case XCB_EXPOSE:
    {
        xcb_expose_event_t* expose = (xcb_expose_event_t*)event;
        cairo_rectangle_int_t rc = {expose->x,expose->y,expose->width,expose->height};
        std::unique_lock<std::recursive_mutex> lock(m_mutex);
        for(auto it = m_msgQueue.begin();it!=m_msgQueue.end();it++){
                if((*it)->message == WM_PAINT && (*it)->hwnd == expose->window)
                {
                    MsgPaint *oldMsg = (MsgPaint*)(*it);
                    cairo_region_union_rectangle(oldMsg->rgn,&rc);
                    return true;
                }
        }

        MsgPaint *pMsgPaint = new MsgPaint(cairo_region_create_rectangle(&rc));
        pMsgPaint->hwnd = expose->window;
        pMsgPaint->message = WM_PAINT;
        pMsgPaint->wParam = 0;
        pMsgPaint->lParam = 0;
        pMsg = pMsgPaint;
        break;
    }
    case XCB_CONFIGURE_NOTIFY:
    {
        xcb_configure_notify_event_t *e2 = (xcb_configure_notify_event_t*)event;
        std::unique_lock<std::recursive_mutex> lock(m_mutex);
        for(auto it = m_msgQueue.begin();it!=m_msgQueue.end();it++){
                if((*it)->message == WM_SIZE && (*it)->hwnd == e2->window)
                {
                    m_msgQueue.erase(it);
                    break;
                }
        }

        pMsg = new Msg;
        pMsg->hwnd = e2->window;
        pMsg->message = WM_SIZE;
        pMsg->wParam=SIZE_RESTORED;
        pMsg->lParam = MAKELPARAM(e2->width,e2->height);
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
        std::unique_lock<std::recursive_mutex> lock(m_mutex);
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
        m_mutex.lock();
        m_evtQueue.push_back(event);
        m_mutex.unlock();
    }

    m_mutex.lock();
    for (auto it : m_evtQueue)
    {
        free(it);
    }
    m_evtQueue.clear();
    m_mutex.unlock();
}

SNSEND