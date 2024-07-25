#include "SConnection.h"
#include <helper/SCriticalSection.h>
#include <assert.h>
#include <functional>
#include <xcb/xcb_icccm.h>
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

//todo: ��ȡ���˫��ʱ������Ŀǰ��ȡ������Чֵ
static uint32_t GetDoubleClickSpan(xcb_connection_t* connection,xcb_screen_t * screen) {
    uint32_t ret = 200;
    xcb_window_t root_window = screen->root;

    // ��ȡDoubleClickTime��Դ��ԭ��ID
    xcb_atom_t atom = SConnMgr::internAtom(connection, 0, "_NET_DOUBLE_CLICK_TIME");
    // ���ͻ�ȡ���Ե�����
    xcb_get_property_cookie_t cookie = xcb_get_property(connection, 0, root_window, atom, XCB_ATOM_CARDINAL, 0, 1024);
    xcb_get_property_reply_t *reply = xcb_get_property_reply(connection, cookie, NULL);
    if (reply == NULL) {
        fprintf(stderr, "Failed to get property value\n");
        return ret;
    }
    
    // ����ȡ��ֵ�Ƿ���Ч
    if (reply->value_len == 1) {
        ret = *((uint32_t*)xcb_get_property_value(reply));
    }
    // �ͷ���Դ
    free(reply);
    return ret;
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

    m_tsDoubleSpan = GetDoubleClickSpan(connection, screen);

    WM_PROTOCOLS_ATOM = SConnMgr::internAtom(connection, 1, "WM_PROTOCOLS");
    WM_DELETE_WINDOW_ATOM = SConnMgr::internAtom(connection, 1, "WM_DELETE_WINDOW");
    WM_STATE_ATOM = SConnMgr::internAtom(connection, 1, "WM_STATE");
    WM_CLASS_ATOM = SConnMgr::internAtom(connection, 1, "WM_CLASS");
    WM_NAME_ATOM = SConnMgr::internAtom(connection, 1, "WM_NAME");
    WM_CHANGE_STATE_ATOM = SConnMgr::internAtom(connection, 1, "WM_CHANGE_STATE");
    _NET_WM_STATE_ATOM = SConnMgr::internAtom(connection, 1, "_NET_WM_STATE");
    _NET_WM_STATE_HIDDEN_ATOM = SConnMgr::internAtom(connection,1,"_NET_WM_STATE_HIDDEN");
    _NET_WM_STATE_DEMANDS_ATTENTION_ATOM = SConnMgr::internAtom(connection,1,"_NET_WM_STATE_DEMANDS_ATTENTION");

    _NET_WM_STATE_MAXIMIZED_HORZ_ATOM = SConnMgr::internAtom(connection,1,"_NET_WM_STATE_MAXIMIZED_HORZ");
    _NET_WM_STATE_MAXIMIZED_VERT_ATOM = SConnMgr::internAtom(connection,1,"_NET_WM_STATE_MAXIMIZED_VERT");

    m_bQuit=false;
    m_msgPeek = nullptr;
    m_bMsgNeedFree = false;
    m_trdEvtReader = std::move(std::thread(std::bind(&readProc, this)));

    m_deskDC = new _SDC(screen->root);
    m_deskBmp = CreateCompatibleBitmap(m_deskDC, 10, 10);
    SelectObject(m_deskDC, m_deskBmp);
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
    if(m_msgPeek && m_bMsgNeedFree){
        delete m_msgPeek;
        m_msgPeek = nullptr;
        m_bMsgNeedFree = false;
    }
    delete m_deskDC;
    DeleteObject(m_deskBmp);
}


bool SConnection::update(){
    UINT timeOut = -1;
    {
        std::unique_lock<std::recursive_mutex> lock(m_mutex4Msg);
        for (auto& it : m_lstTimer) {
            timeOut = std::min(timeOut, it.fireRemain);
        }
    }
    std::unique_lock<std::mutex> lock(m_mutex4Evt);
    uint64_t ts1 = GetTickCount64();
    m_varCondition.wait_for(lock, std::chrono::milliseconds(timeOut), [&]{return !m_evtQueue.empty(); });

    for(auto it:m_evtQueue){
        pushEvent(it);
        free(it);
    }
    bool bRet = !m_evtQueue.empty();
    m_evtQueue.clear();
    uint64_t ts2 = GetTickCount64();
    {
        UINT elapse = ts2 - ts1;
        std::unique_lock<std::recursive_mutex> lock(m_mutex4Msg);
        for (auto& it : m_lstTimer) {
            if (it.fireRemain > elapse) {
                Msg* pMsg = new Msg;
                pMsg->hwnd = it.hWnd;
                pMsg->message = WM_TIMER;
                pMsg->wParam = it.id;
                pMsg->lParam = (LPARAM)it.proc;
                pMsg->time = ts2;
                m_msgQueue.push_back(pMsg);
                it.fireRemain = it.elapse;
                bRet = true;
            }
            else {
                it.fireRemain = it.elapse;
            }
        }
    }
    return bRet;
}

BOOL SConnection::peekMsg(THIS_ LPMSG pMsg, HWND  hWnd, UINT wMsgFilterMin, UINT wMsgFilterMax, UINT  wRemoveMsg) {
	std::unique_lock<std::recursive_mutex> lock(m_mutex4Msg);
	auto it = m_msgQueue.begin();
	for (; it != m_msgQueue.end(); it++) {
		BOOL bMatch = TRUE;
		Msg* msg = (*it);
		do {
			if (msg->message == WM_QUIT)
				break;
			if (msg->message == WM_TIMER && msg->lParam == 0)
				break;
			if (msg->hwnd != hWnd && hWnd != 0)
			{
				bMatch = FALSE;
				break;
			}
			if (wMsgFilterMin == 0 && wMsgFilterMax == 0)
				break;
			if (wMsgFilterMin <= msg->message && wMsgFilterMax >= msg->message)
				break;
			bMatch = FALSE;
		} while (false);
        if (bMatch) break;
	}
	if (it != m_msgQueue.end()) {
		Msg* msg = (*it);
		if (m_msgPeek && m_bMsgNeedFree) {
			delete m_msgPeek;
			m_msgPeek = nullptr;
			m_bMsgNeedFree=false;
		}
		if (msg->message == WM_TIMER && msg->lParam) {
			TIMERPROC proc = (TIMERPROC)msg->lParam;
			proc(msg->hwnd, WM_TIMER, msg->wParam, msg->time);
			m_msgQueue.erase(it);
			return PeekMessage(pMsg, hWnd, wMsgFilterMin, wMsgFilterMax, wRemoveMsg);
		}
		m_msgPeek = msg;
		if (wRemoveMsg & PM_NOREMOVE)
			m_bMsgNeedFree = false;
		else
		{
			m_msgQueue.erase(it);
			m_bMsgNeedFree = true;
		}
		memcpy(pMsg, (MSG*)m_msgPeek, sizeof(MSG));

		return TRUE;
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

UINT_PTR SConnection::SetTimer(HWND hWnd, UINT_PTR id, UINT uElapse,
    TIMERPROC proc)
{
    std::unique_lock<std::recursive_mutex> lock(m_mutex4Msg);
    if (hWnd) {
        //find exist timer.
        for (auto& it : m_lstTimer) {
            if (it.hWnd != hWnd)
                continue;
            if (it.id == id) {
                it.fireRemain = uElapse;
                it.proc = proc;
                return id;
            }
        }
        TimerInfo timer;
        timer.id = id;
        timer.fireRemain = uElapse;
        timer.hWnd = hWnd;
        timer.proc = proc;
        m_lstTimer.push_back(timer);
        return id;
    }
    else {
        UINT_PTR newId = 0;
        for (auto& it : m_lstTimer) {
            if (it.hWnd)
                continue;
            newId = std::max(it.id, newId);
        }
        TimerInfo timer;
        timer.id = newId+1;
        timer.fireRemain = uElapse;
        timer.hWnd = hWnd;
        timer.proc = proc;
        m_lstTimer.push_back(timer);
        return newId;
    }
}

BOOL SConnection::KillTimer(HWND hWnd,
    UINT_PTR id)
{
    std::unique_lock<std::recursive_mutex> lock(m_mutex4Msg);
    for (auto it = m_lstTimer.begin(); it != m_lstTimer.end(); it++) {
        if (it->hWnd == hWnd && it->id == id) {
            m_lstTimer.erase(it);
            return TRUE;
        }
    }
    return FALSE;
}

HDC SConnection::GetDC()
{
    return m_deskDC;
}

BOOL SConnection::ReleaseDC(HDC hdc)
{
    //todo:hjx
    return TRUE;
}

static uint32_t TsSpan(uint32_t t1, uint32_t t2) {
    if (t1 > t2) {
        return t1 - t2;
    }
    else {
        return t1 + (0xffffffff - t2);
    }
}

static WPARAM ButtonState2Mask(uint16_t state) {
    WPARAM wp = 0;
    if (state & XCB_KEY_BUT_MASK_SHIFT)
        wp |= MK_SHIFT;
    if (state & XCB_KEY_BUT_MASK_CONTROL)
        wp |= MK_CONTROL;
    if (state & XCB_BUTTON_MASK_1)
        wp |= MK_LBUTTON;
    if (state & XCB_BUTTON_MASK_2)
        wp |= MK_RBUTTON;
    if (state & XCB_BUTTON_MASK_3)
        wp |= MK_RBUTTON;
    return wp;
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
    case XCB_PROPERTY_NOTIFY:
    {
        xcb_property_notify_event_t* e2 = (xcb_property_notify_event_t*)event;
        if (e2->atom == _NET_WM_STATE_ATOM || e2->atom == WM_STATE_ATOM) {
            uint32_t newState = SIZE_RESTORED;
            if (e2->atom == WM_STATE_ATOM) {
                const xcb_get_property_cookie_t get_cookie = xcb_get_property(connection, 0, e2->window, WM_STATE_ATOM, XCB_ATOM_ANY, 0, 1024);
                xcb_get_property_reply_t * reply = xcb_get_property_reply(connection, get_cookie, nullptr);
                if (reply && reply->format == 32 && reply->type == WM_STATE_ATOM && reply->length!=0) {
                    const uint32_t* data = (const uint32_t*)xcb_get_property_value(reply);
                    if (data[0] == XCB_ICCCM_WM_STATE_ICONIC/* || data[0]==XCB_ICCCM_WM_STATE_WITHDRAWN*/) {
                        newState = SIZE_MINIMIZED;                       
                    }
                }
                free(reply);
            }
            if (newState != SIZE_MINIMIZED) {

            }
        }
        break;
    }
    case XCB_CONFIGURE_NOTIFY:
    {
        xcb_configure_notify_event_t *e2 = (xcb_configure_notify_event_t*)event;
        std::unique_lock<std::recursive_mutex> lock(m_mutex4Msg);
        for(auto it = m_msgQueue.begin();it!=m_msgQueue.end();it++){
                if((*it)->message == WM_MOVE && (*it)->hwnd == e2->window)
                {
                    m_msgQueue.erase(it);
                    break;
                }
        }
        for(auto it = m_msgQueue.begin();it!=m_msgQueue.end();it++){
                if((*it)->message == WM_SIZE && (*it)->hwnd == e2->window)
                {
                    m_msgQueue.erase(it);
                    break;
                }
        }
        pMsg = new Msg;
        pMsg->hwnd = e2->window;
        pMsg->message = WM_MOVE;
        pMsg->wParam = 0;
        pMsg->lParam = MAKELPARAM(e2->x,e2->y);
        m_msgQueue.push_back(pMsg);
        pMsg = new Msg;
        pMsg->hwnd = e2->window;
        pMsg->message = WM_SIZE;
        pMsg->wParam = 0;
        pMsg->lParam = MAKELPARAM(e2->width,e2->height);
        m_msgQueue.push_back(pMsg);
        pMsg = nullptr;
        break;
    }
    case XCB_CLIENT_MESSAGE:
    {
        xcb_client_message_event_t *client_message_event = (xcb_client_message_event_t *) event;
        if(client_message_event->data.data32[0] == WM_DELETE_WINDOW_ATOM){
            pMsg = new Msg;
            pMsg->message = WM_CLOSE;
            pMsg->hwnd = client_message_event->window;
            pMsg->wParam = pMsg->lParam = 0;
        }else if(client_message_event->type == _NET_WM_STATE_HIDDEN_ATOM){
            pMsg = new Msg;
            pMsg->message = WM_SHOWWINDOW;
            pMsg->hwnd =client_message_event->window;
            pMsg->wParam = client_message_event->data.data32[0];
            pMsg->lParam = 0;
        }
        else if(client_message_event->type == _NET_WM_STATE_DEMANDS_ATTENTION_ATOM){
            pMsg = new Msg;
            pMsg->message = WM_ENABLE;
            pMsg->hwnd =client_message_event->window;
            pMsg->wParam = client_message_event->data.data32[0];
            pMsg->lParam = 0;
        }
        break;
    }
    break;
    case XCB_BUTTON_PRESS:
    {
        xcb_button_press_event_t* e2 = (xcb_button_press_event_t*)event;
        if (e2->detail >= XCB_BUTTON_INDEX_1 && e2->detail <= XCB_BUTTON_INDEX_3) {
            pMsg = new Msg;
            pMsg->hwnd = e2->event;
            pMsg->pt.x = e2->event_x;
            pMsg->pt.y = e2->event_y;
            pMsg->lParam = MAKELPARAM(pMsg->pt.x, pMsg->pt.y);
            switch (e2->detail) {
            case XCB_BUTTON_INDEX_1://left button
                pMsg->message = TsSpan(e2->time, m_tsPrevPress) > m_tsDoubleSpan ? WM_LBUTTONDOWN : WM_LBUTTONDBLCLK;
                break;
            case XCB_BUTTON_INDEX_2:
                pMsg->message = TsSpan(e2->time, m_tsPrevPress) < m_tsDoubleSpan ? WM_RBUTTONDOWN : WM_RBUTTONDBLCLK;
                break;
            case XCB_BUTTON_INDEX_3:
                pMsg->message = TsSpan(e2->time, m_tsPrevPress) < m_tsDoubleSpan ? WM_MBUTTONDOWN : WM_MBUTTONDBLCLK;
                break;
            }
            pMsg->wParam = ButtonState2Mask(e2->state);
            m_tsPrevPress = e2->time;
//            printf("on button down, msg=%d,pt=(%d,%d)\n", pMsg->message, pMsg->pt.x, pMsg->pt.y);
        }
        break;
    }
    case XCB_BUTTON_RELEASE:
    {
        xcb_button_release_event_t* e2 = (xcb_button_release_event_t*)event;
        if (e2->detail >= XCB_BUTTON_INDEX_1 && e2->detail <= XCB_BUTTON_INDEX_3) {
            pMsg = new Msg;
            pMsg->hwnd = e2->event;
            pMsg->pt.x = e2->event_x;
            pMsg->pt.y = e2->event_y;
            pMsg->lParam = MAKELPARAM(pMsg->pt.x, pMsg->pt.y);
            switch (e2->detail) {
            case XCB_BUTTON_INDEX_1://left button
                pMsg->message = WM_LBUTTONUP;
                break;
            case XCB_BUTTON_INDEX_2:
                pMsg->message = WM_RBUTTONUP;
                break;
            case XCB_BUTTON_INDEX_3:
                pMsg->message = WM_MBUTTONUP;
                break;
            }
            pMsg->wParam = ButtonState2Mask(e2->state);
//            printf("on button up!!, msg=%d,pt=(%d,%d)\n", pMsg->message, pMsg->pt.x, pMsg->pt.y);
        }
        break;
    }
    case XCB_MOTION_NOTIFY:
    {
        xcb_motion_notify_event_t* e2 = (xcb_motion_notify_event_t*)event;
        pMsg = new Msg;
        pMsg->hwnd = e2->event;
        pMsg->message = WM_MOUSEMOVE;
        pMsg->pt.x = e2->event_x;
        pMsg->pt.y = e2->event_y;
        pMsg->lParam = MAKELPARAM(pMsg->pt.x, pMsg->pt.y);
        pMsg->wParam = ButtonState2Mask(e2->state);
        pMsg->time = e2->time;
//        printf("XCB_MOTION_NOTIFY!!, msg=%d,pt=(%d,%d)\n", pMsg->message, pMsg->pt.x, pMsg->pt.y);
        break;
    }
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
