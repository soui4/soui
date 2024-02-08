#include <SNativeWnd.h>
#include "UiState.h"
#include <string.h>

SNSBEGIN

SNativeWnd::SNativeWnd():m_hWnd(0),mConnection(nullptr),mScreen(nullptr),m_gc(0){

}

HWND  SNativeWnd::CreateNative
(LPCTSTR lpWindowName, DWORD dwStyle, DWORD dwExStyle, int x, int y, int nWidth, int nHeight, HWND hWndParent, int nID, LPVOID lpParam ){
    m_id = nID;

    SThreadUiState *state = SUiState::instance()->getThreadUiState();
    mConnection = state->connection;
    mScreen = state->screen;

    m_hWnd = xcb_generate_id(mConnection);

    uint32_t mask = XCB_CW_BACK_PIXEL | XCB_CW_EVENT_MASK;
    uint32_t value_list[2] = {
        mScreen->black_pixel,
        XCB_EVENT_MASK_EXPOSURE | XCB_EVENT_MASK_BUTTON_PRESS |
            XCB_EVENT_MASK_BUTTON_RELEASE | XCB_EVENT_MASK_POINTER_MOTION |
            XCB_EVENT_MASK_ENTER_WINDOW | XCB_EVENT_MASK_LEAVE_WINDOW |
            XCB_EVENT_MASK_KEY_PRESS | XCB_EVENT_MASK_KEY_RELEASE | 
            XCB_EVENT_MASK_RESIZE_REDIRECT|XCB_EVENT_MASK_FOCUS_CHANGE|
            XCB_EVENT_MASK_VISIBILITY_CHANGE|XCB_EVENT_MASK_STRUCTURE_NOTIFY};

    xcb_create_window(mConnection, XCB_COPY_FROM_PARENT, m_hWnd,
                      mScreen->root, x, y, nWidth, nHeight, 0,
                      XCB_WINDOW_CLASS_INPUT_OUTPUT, mScreen->root_visual, mask,
                      value_list);

    m_gc = xcb_generate_id(mConnection);
// 设置绘图上下文属性
    uint32_t value_mask = XCB_GC_FOREGROUND | XCB_GC_GRAPHICS_EXPOSURES;
    value_list[0] = mScreen->black_pixel;
    value_list[1] = 0;

    xcb_create_gc(mConnection, m_gc, m_hWnd, value_mask, value_list);

    xcb_change_property(mConnection, XCB_PROP_MODE_REPLACE, m_hWnd,
        XCB_ATOM_WM_NAME, XCB_ATOM_STRING, 8, strlen(lpWindowName), lpWindowName);

    xcb_change_property(mConnection, XCB_PROP_MODE_REPLACE, m_hWnd, state->wm_protocols_atom, XCB_ATOM_ATOM, 32, 1, &state->wm_delete_window_atom);

    xcb_map_window(mConnection, m_hWnd);

    const unsigned coords[] = {static_cast<unsigned>(x),
                               static_cast<unsigned>(y)};
    xcb_configure_window(mConnection, m_hWnd,
                         XCB_CONFIG_WINDOW_X | XCB_CONFIG_WINDOW_Y, coords);

    state->onWndCreate(m_hWnd,this);
    xcb_flush(mConnection);
    
    return m_hWnd;
}

HWND  SNativeWnd::GetHwnd(){
    return m_hWnd;
}

BOOL SNativeWnd::DestroyWindow(){
    if(!m_hWnd)
        return FALSE;
    xcb_destroy_window(mConnection,m_hWnd);
    SThreadUiState *state = SUiState::instance()->getThreadUiState();
    state->onWndDestroy(m_hWnd);
    m_hWnd = 0;
    return TRUE;
}

SNSEND