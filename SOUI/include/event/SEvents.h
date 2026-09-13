/**
 * @file       SEvents.h
 * @brief      Event system used in the SOUI system
 * @version    v1.0
 * @author     SOUI group
 * @date       2014/08/02
 *
 * @details    Defines various events used in the SOUI system and their handling mechanisms.
 */

#ifndef __SEVENTS__H__
#define __SEVENTS__H__

#include <interface/SEvtArgs-i.h>
#include <interface/sstring-i.h>

#ifdef __cplusplus
#include <sobject/Sobject.hpp>
#include <helper/obj-ref-impl.hpp>
#endif

SNSBEGIN

typedef struct IWindow IWindow;

/**
 * @enum _SOUI_EVENTS
 * @brief Defines the event IDs in the SOUI system
 */
typedef enum _SOUI_EVENTS
{
    EVT_INIT = 8000, /**<  Initialization event */
    EVT_EXIT,        /**<  Exit event */
    EVT_TIMER,       /**<  Timer event */
    EVT_GET_CARET,   /**<  Get cursor event */

    /** Basic window events */
    EVT_SETFOCUS = 8100,  /**<  Set focus event */
    EVT_KILLFOCUS,        /**<  Kill focus event */
    EVT_CREATE,           /**<  Create event */
    EVT_INIT_FINISH,      /**<  Initialization complete event */
    EVT_DESTROY,          /**<  Destroy event */
    EVT_SIZE,             /**<  Size change event */
    EVT_VISIBLECHANGED,   /**<  Visibility change event */
    EVT_STATECHANGED,     /**<  State change event */
    EVT_CAPTURECHANGED,   /**<  Capture change event */
    EVT_MOUSE_CLICK,      /**<  Mouse click event */
    EVT_UPDATE_TOOLTIP,   /**<  Update tooltip event */
    EVT_ANIMATION_START,  /**<  Animation start event */
    EVT_ANIMATION_STOP,   /**<  Animation stop event */
    EVT_ANIMATION_REPEAT, /**<  Animation repeat event */
    EVT_POS,              /**<  Position change event */
    EVT_ANIMATOR_FRACTOR, /**<  Value animation event */

    EVT_KEYDOWN = 8200, /**<  Key down event */
    EVT_KEYUP,          /**<  Key up event */
    EVT_CHAR,           /**<  Character input event */
    EVT_SYSKEYDOWN,     /**<  System key down event */
    EVT_SYSKEYUP,       /**<  System keyboard release event */

    /** Mouse state events for two windows */
    EVT_MOUSE_HOVER = 9000, /**<  Mouse hover event */
    EVT_MOUSE_LEAVE,        /**<  Mouse leave event */
    EVT_MOUSE_MOVE,         /**<  Mouse move event */

    EVT_CMD = 10000, /**<  Command event */
    EVT_MENU_CMD,    /**<  Menu command event */
    EVT_CTXMENU,     /**<  Context menu event */

    /** Add two scroll VIEW events */
    EVT_SCROLLVIEW_ORIGINCHANGED, /**<  Scroll view origin changed event */
    EVT_SCROLLVIEW_SIZECHANGED,   /**<  Scroll view size changed event */

    EVT_SCROLL,  /**<  Scroll event */
    EVT_OFEVENT, /**<  Message re-packaging event */
    EVT_OFPANEL, /**<  Message re-packaging event in panel */

    /** Add 4 itempanel-related events, reaching the application layer via EVT_OFPANEL */
    EVT_ITEMPANEL_CLICK,     /**<  Panel click event */
    EVT_ITEMPANEL_RCLICK,    /**<  Panel right-click event */
    EVT_ITEMPANEL_CLICK_UP,  /**<  Panel click release event */
    EVT_ITEMPANEL_RCLICK_UP, /**<  Panel right-click release event */
    EVT_ITEMPANEL_DBCLICK,   /**<  Panel double-click event */
    EVT_ITEMPANEL_HOVER,     /**<  Panel hover event */
    EVT_ITEMPANEL_LEAVE,     /**<  Panel leave event */

    EVT_RADIOGROUP_CHECK_CHANGED = 10100, /**<  Radio button group selection state changed event */

    EVT_TAB_SELCHANGING = 11000, /**<  Tab selection changing event */
    EVT_TAB_SELCHANGED,          /**<  Tab selection changed event */
    EVT_TAB_ITEMHOVER,           /**<  Tab item hover event */
    EVT_TAB_ITEMLEAVE,           /**<  Tab item leave event */

    EVT_LB_SELCHANGING = 12000, /**<  List box selection changing event */
    EVT_LB_SELCHANGED,          /**<  List box selection changed event */
    EVT_LB_DBCLICK,             /**<  List box double-click event */
    EVT_LB_RCLICK,              /**<  List box right-click event */

    EVT_LC_SELCHANGING = 13000, /**<  List control selection changing event */
    EVT_LC_SELCHANGED,          /**<  List control selection changed event */
    EVT_LC_ITEMDELETED,         /**<  List control item deleted event */
    EVT_LC_DBCLICK,             /**<  List control double-click event */
    EVT_LC_RCLICK,              /**<  List control right-click event */

    EVT_TV_SELCHANGING = 14000, /**<  Tree control selection changing event */
    EVT_TV_SELCHANGED,          /**<  Tree control selection changed event */

    EVT_TC_SELCHANGING = 15000, /**<  Tree control selection changing event */
    EVT_TC_SELCHANGED,          /**<  Tree control selection changed event */
    EVT_TC_EXPAND,              /**<  Tree control expand event */
    EVT_TC_CHECKSTATE,          /**<  Tree control check state changed event */
    EVT_TC_DBCLICK,             /**<  Tree control double-click event */
    EVT_TC_RCLICK,              /**<  Tree control right-click event */

    EVT_CB_BEFORE_CLOSEUP = 15050, /**<  Combo box before-close event */

    EVT_LV_SELCHANGING = 15100, /**<  List view selection changing event */
    EVT_LV_SELCHANGED,          /**<  List view selection changed event */
    EVT_LV_ITEMCLICK,           /**<  List view item click event */

    EVT_RE_NOTIFY = 16000, /**<  Rich edit control notify event */
    EVT_RE_MENU,           /**<  Rich edit control menu event */

    EVT_SLIDER_POS = 17000, /**<  Slider position changed event */
    EVT_SLIDER_VALUETIP,    /**<  Slider value tip event */

    EVT_HEADER_CLICK = 18000, /**<  Header click event */
    EVT_HEADER_ITEMCHANGING,  /**<  Header item changing event */
    EVT_HEADER_ITEMCHANGED,   /**<  Header item changed event */
    EVT_HEADER_ITEMSWAP,      /**<  Header item swap event */
    EVT_HEADER_RELAYOUT,      /**<  Header relayout event */

    EVT_CB_SELCHANGE = 19000, /**<  Combo box selection changed event */
    EVT_CB_DROPDOWN,          /**<  Combo box dropdown event */

    EVT_CALENDAR_SELDAY = 20000, /**<  Calendar date selected event */
    EVT_CALENDAR_SETDATE,        /**<  Calendar date set event */
    EVT_CALENDAREX_CHANGED,      /**<  Extended calendar changed event */
    EVT_DATETIME_CHANGED,        /**<  DateTime changed event */

    EVT_SPIN_VALUE2STRING = 21000, /**<  Spin box value-to-string event */

    EVT_SPLIT_PANE_MOVED = 22000, /**<  Splitter pane moved event */

    EVT_HOT_KEY_SET = 23000, /**<  Hotkey set event */

    /** Image animation start and end events */
    EVT_IMAGE_ANI_START = 22100, /**<  Image animation start event */
    EVT_IMAGE_ANI_STOP,          /**<  Image animation stop event */
    EVT_IMAGE_ANI_REPEAT,        /**<  Image animation repeat event */

    EVT_SELECTMENU = 22150, /**<  Menu item selected event */
    EVT_POPMENU,            /**<  Popup menu event */

    EVT_REALWND_CREATE = 22160, /**<  Real window created event */
    EVT_REALWND_DESTROY,        /**<  Real window destroyed event */
    EVT_REALWND_INIT,           /**<  Real window initialized event */
    EVT_REALWND_POSITION,       /**<  Real window position changed event */

    EVT_DROPDOWN_LIST_FILL = 22170,
    EVT_DROPDOWN_LIST_SELECTED,
    EVT_DROPDOWN_LIST_GET_BUDDY_RECT,

    EVT_TB_ITEMCLICK = 22180, /**< Toolbar item click event */

    EVT_UPDATE_ITEMTIP = 22190, /**<  Update item tip event */
    EVT_UPDATE_CMDUI = 22191,   /**<  Update command UI event */

    EVT_EXIT_MODALVIEW = 22100,

    EVT_EXTERNAL_BEGIN = 10000000, /**<  External event start ID */
} SOUI_EVENTS;

/**
 * @enum _MouseClickId
 * @brief Defines mouse click types
 */
typedef enum _MouseClickId
{
    MOUSE_LBTN_DOWN = 0, /**<  Left button down */
    MOUSE_LBTN_UP,       /**<  Left button up */
    MOUSE_LBTN_DBCLICK,  /**<  Left button double-click */
    MOUSE_RBTN_DOWN,     /**<  Right button down */
    MOUSE_RBTN_UP,       /**<  Right button up */
    MOUSE_RBTN_DBCLICK,  /**<  Right button double-click */
    MOUSE_MBTN_DOWN,     /**<  Middle button down */
    MOUSE_MBTN_UP,       /**<  Middle button up */
    MOUSE_MBTN_DBCLICK,  /**<  Middle button double-click */
} MouseClickId;

#ifdef __cplusplus
/**
 * @class SEvtArgs
 * @brief Event argument base class
 */
class SOUI_EXP SEvtArgs : public TObjRefImpl<SObjectImpl<IEvtArgs>> {
    DEF_SOBJECT(SObjectImpl<IEvtArgs>, L"event")

  public:
    UINT handled;     /**<  Handling count */
    BOOL bubbleUp;    /**<  Whether to bubble */
    int idFrom;       /**<  Sender ID */
    LPCWSTR nameFrom; /**<  Sender name */
    IObject *sender;  /**<  Sender object */

    /**
     * @brief Get sender object
     * @return Sender object pointer
     */
    STDMETHOD_(IObject *, Sender)(THIS)
    {
        return sender;
    }

    /**
     * @brief Get sender ID
     * @return Sender ID
     */
    STDMETHOD_(int, IdFrom)(THIS) SCONST
    {
        return idFrom;
    }

    /**
     * @brief Set sender ID
     * @param id Sender ID
     */
    STDMETHOD_(void, SetIdFrom)(THIS_ int id)
    {
        idFrom = id;
    }

    /**
     * @brief Get sender name
     * @return Sender name
     */
    STDMETHOD_(LPCWSTR, NameFrom)(THIS) SCONST
    {
        return nameFrom;
    }

    /**
     * @brief Set sender name
     * @param name Sender name
     */
    STDMETHOD_(void, SetNameFrom)(THIS_ LPCWSTR name)
    {
        nameFrom = name;
    }

    /**
     * @brief Check whether the event bubbles
     * @return TRUE if bubbling, FALSE otherwise
     */
    STDMETHOD_(BOOL, IsBubbleUp)(THIS) SCONST
    {
        return bubbleUp;
    }

    /**
     * @brief Set whether the event bubbles
     * @param bSet Whether to bubble
     */
    STDMETHOD_(void, SetBubbleUp)(THIS_ BOOL bSet)
    {
        bubbleUp = bSet;
    }

    /**
     * @brief Get handling count
     * @return Handling count
     */
    STDMETHOD_(UINT, HandleCount)(THIS) SCONST
    {
        return handled;
    }

    /**
     * @brief Increment handling count
     */
    STDMETHOD_(void, IncreaseHandleCount)(THIS)
    {
        handled++;
    }

    /**
     * @brief Get event data
     * @return Event data pointer
     */
    STDMETHOD_(LPVOID, Data)(THIS)
    {
        return NULL;
    }

  public:
    /**
     * @brief Constructor
     * @param pSender Sender object
     */
    SEvtArgs(IObject *pSender)
        : handled(0)
        , sender(pSender)
        , bubbleUp(true)
    {
        if (NULL != sender)
        {
            idFrom = sender->GetID();
            nameFrom = sender->GetName();
            sender->AddRef();
        }
        else
        {
            idFrom = 0;
            nameFrom = NULL;
        }
    }

    /**
     * @brief Destructor
     */
    virtual ~SEvtArgs()
    {
        if (sender)
        {
            sender->Release();
            sender = NULL;
        }
    }
};

/** Define a group of macros for event definitions to simplify event definition. */
#define DEF_EVT_CLASS(evt, id, evt_name, evtData, api) \
    class api evt                                      \
        : public SEvtArgs                              \
        , public evtData {                             \
        DEF_SOBJECT(SEvtArgs, WIDESTR(evt_name))       \
      public:                                          \
        STDMETHOD_(int, GetID)(THIS) const             \
        {                                              \
            return evt::EventID;                       \
        }                                              \
        STDMETHOD_(LPCWSTR, GetName)(THIS) const       \
        {                                              \
            return evt::GetClassName();                \
        }                                              \
        STDMETHOD_(LPVOID, Data)(THIS)                 \
        {                                              \
            return (evtData *)this;                    \
        }                                              \
        enum                                           \
        {                                              \
            EventID = id                               \
        };                                             \
        evt(SNS::IObject *pSender = NULL)              \
            : SEvtArgs(pSender)                        \
        {                                              \
        }                                              \
    };

#define EVENTID(x) x::EventID, x::GetClassName()

#endif /**< __cplusplus */

#define DEF_EVT_STRUCT(n, x) typedef struct n x n;

#ifdef __cplusplus
#define DEF_EVENT(evt, id, name, x, api) \
    DEF_EVT_STRUCT(St##evt, x)           \
    DEF_EVT_CLASS(evt, id, name, St##evt, api)
#else
#define DEF_EVENT(evt, id, name, x, api) DEF_EVT_STRUCT(St##evt, x)
#endif /**< __cplusplus */

#define DEF_EVT(evt, id, name, x) DEF_EVENT(evt, id, name, x, SOUI_EXP)
#define DEF_EVT_EXT(evt, id, x)   DEF_EVENT(evt, id, on_##evt, x, )

DEF_EVT(EventInit, EVT_INIT, on_init, { int fake; })

DEF_EVT(EventExit, EVT_EXIT, on_exit, { int fake; })

DEF_EVT(EventTimer, EVT_TIMER, on_timer, {
    UINT uID;     /**<  Timer ID */
    LPARAM uData; /**<  Timer data */
})

DEF_EVT(EventGetCaret, EVT_GET_CARET, on_get_caret, { IStringW *strCaret; })

DEF_EVT(EventScroll, EVT_SCROLL, on_scroll, {
    int nSbCode;    /**<  Scroll bar code */
    int nPos;       /**<  Scroll bar position */
    BOOL bVertical; /**<  Whether vertical scroll bar */
})

DEF_EVT(EventSpinValue2String, EVT_SPIN_VALUE2STRING, on_spin_valuetostring, {
    BOOL bInit;         /**<  Whether initialized */
    int nValue;         /**<  Spin box value */
    IStringT *strValue; /**<  String value */
})

DEF_EVT(EventSwndCreate, EVT_CREATE, on_create, { int fake; })
DEF_EVT(EventSwndInitFinish, EVT_INIT_FINISH, on_init_finish, { int fake; })
DEF_EVT(EventSwndDestroy, EVT_DESTROY, on_destroy, { int fake; })

DEF_EVT(EventSwndSize, EVT_SIZE, on_size, { SIZE szWnd; })
DEF_EVT(EventSwndPos, EVT_POS, on_pos, { RECT rcWnd; })

typedef struct IPropertyValuesHolder IPropertyValuesHolder;
typedef enum _ANI_STATE ANI_STATE;
DEF_EVT(EventSwndAnimatorFractor, EVT_ANIMATOR_FRACTOR, on_animator_fractor, {
    IPropertyValuesHolder *pHolder;
    float fraction;
    ANI_STATE state;
})

DEF_EVT(EventSwndStateChanged, EVT_STATECHANGED, on_state_changed, {
    DWORD dwOldState; /**<  Old state */
    DWORD dwNewState; /**<  New state */
})

#ifdef __cplusplus
/**
 * @brief Check whether the state changed
 * @param pEvt State change event pointer
 * @param dwState State
 * @return TRUE if state changed, FALSE otherwise
 */
inline BOOL EventSwndStateChanged_CheckState(EventSwndStateChanged *pEvt, DWORD dwState)
{
    return (pEvt->dwOldState & dwState) != (pEvt->dwNewState & dwState);
}
#endif

DEF_EVT(EventSwndCaptureChanged, EVT_CAPTURECHANGED, on_capture_changed, { BOOL bCaptured; })

DEF_EVT(EventSwndVisibleChanged, EVT_VISIBLECHANGED, on_visible_changed, { BOOL bVisible; })

DEF_EVT(EventSwndAnimationStart, EVT_ANIMATION_START, on_animation_start, { IAnimation *pAni; })

DEF_EVT(EventSwndAnimationStop, EVT_ANIMATION_STOP, on_animation_stop, { IAnimation *pAni; })
DEF_EVT(EventSwndAnimationRepeat, EVT_ANIMATION_REPEAT, on_animation_repeat, { IAnimation *pAni; })

DEF_EVT(EventKeyDown, EVT_KEYDOWN, on_key_down, {
    UINT nChar;   /**<  Character code */
    UINT nRepCnt; /**<  Repeat count */
    UINT nFlags;  /**<  Flag */
})

DEF_EVT(EventKeyUp, EVT_KEYUP, on_key_up, {
    UINT nChar;   /**<  Character code */
    UINT nRepCnt; /**<  Repeat count */
    UINT nFlags;  /**<  Flag */
})

DEF_EVT(EventChar, EVT_CHAR, on_char, {
    UINT nChar;   /**<  Character code */
    UINT nRepCnt; /**<  Repeat count */
    UINT nFlags;  /**<  Flag */
})

DEF_EVT(EventSysKeyDown, EVT_SYSKEYDOWN, on_sys_key_down, {
    UINT nChar;   /**<  Character code */
    UINT nRepCnt; /**<  Repeat count */
    UINT nFlags;  /**<  Flag */
})

DEF_EVT(EventSysKeyUp, EVT_SYSKEYUP, on_sys_key_up, {
    UINT nChar;   /**<  Character code */
    UINT nRepCnt; /**<  Repeat count */
    UINT nFlags;  /**<  Flag */
})

DEF_EVT(EventSwndMouseLeave, EVT_MOUSE_LEAVE, on_mouse_leave, { int fake; })

DEF_EVT(EventSwndMouseHover, EVT_MOUSE_HOVER, on_mouse_hover, { int fake; })

DEF_EVT(EventSwndMouseMove, EVT_MOUSE_MOVE, on_mouse_move, {
    int nFlags;
    POINT pt;
})

DEF_EVT(EventSwndUpdateTooltip, EVT_UPDATE_TOOLTIP, on_update_tooltip, {
    IStringT *strToolTip; /**<  Tooltip string */
    BOOL bUpdated;        /**<  Whether to update */
})

DEF_EVT(EventRadioGroupCheckChanged, EVT_RADIOGROUP_CHECK_CHANGED, on_radiogroup_check_changed, { IWindow *pChecked; })

DEF_EVT(EventItemPanelRclick, EVT_ITEMPANEL_RCLICK, on_itemparem_rclick, {
    WPARAM wParam; /**<  WPARAM parameter */
    LPARAM lParam; /**<  LPARAM parameter */
})

DEF_EVT(EventItemPanelDbclick, EVT_ITEMPANEL_DBCLICK, on_itempanel_dbclick, {
    WPARAM wParam; /**<  WPARAM parameter */
    LPARAM lParam; /**<  LPARAM parameter */
})

DEF_EVT(EventItemPanelClick, EVT_ITEMPANEL_CLICK, on_itempanel_click, {
    WPARAM wParam; /**<  WPARAM parameter */
    LPARAM lParam; /**<  LPARAM parameter */
})

DEF_EVT(EventItemPanelRclickUp, EVT_ITEMPANEL_RCLICK_UP, on_itemparem_rclick_up, {
    WPARAM wParam; /**<  WPARAM parameter */
    LPARAM lParam; /**<  LPARAM parameter */
})

DEF_EVT(EventItemPanelClickUp, EVT_ITEMPANEL_CLICK_UP, on_itempanel_click_up, {
    WPARAM wParam; /**<  WPARAM parameter */
    LPARAM lParam; /**<  LPARAM parameter */
})

DEF_EVT(EventItemPanelHover, EVT_ITEMPANEL_HOVER, on_itempanel_hover, {
    WPARAM wParam; /**<  WPARAM parameter */
    LPARAM lParam; /**<  LPARAM parameter */
})

/** Note: When getting the item index via GetItemIndex from IItemPanel in EventItemPanelLeave, the index validity must be checked. */
DEF_EVT(EventItemPanelLeave, EVT_ITEMPANEL_LEAVE, on_itempanel_leave, { int fake; })

DEF_EVT(EventMouseClick, EVT_MOUSE_CLICK, on_mouse_click, {
    POINT pt;             /**<  Mouse click position */
    UINT uFlags;          /**<  Mouse click flag */
    MouseClickId clickId; /**<  Mouse click type */
    BOOL bHover;          /**<  Whether hovering */
})

DEF_EVT(EventCmd, EVT_CMD, on_command, { int fake; })

DEF_EVT(EventMenuCmd, EVT_MENU_CMD, on_menu_command, { int menuId; })

DEF_EVT(EventCtxMenu, EVT_CTXMENU, on_conext_menu, {
    POINT pt;     /**<  Context menu position */
    BOOL bCancel; /**<  Whether canceled */
})

DEF_EVT(EventSetFocus, EVT_SETFOCUS, on_set_focus, { SWND wndOld; })

DEF_EVT(EventKillFocus, EVT_KILLFOCUS, on_kill_focus, { SWND wndFocus; })

DEF_EVT(EventScrollViewOriginChanged, EVT_SCROLLVIEW_ORIGINCHANGED, on_scrollview_origin_changed, {
    POINT ptOldOrigin; /**<  Old origin position */
    POINT ptNewOrigin; /**<  New origin position */
})

DEF_EVT(EventScrollViewSizeChanged, EVT_SCROLLVIEW_SIZECHANGED, on_scrollview_size_changed, {
    SIZE szOldViewSize; /**<  Old view size */
    SIZE szNewViewSize; /**<  New view size */
})

DEF_EVT(EventOfEvent, EVT_OFEVENT, on_event_of_event, { IEvtArgs *pOrgEvt; })

DEF_EVT(EventOfPanel, EVT_OFPANEL, on_event_of_panel, {
    IWindow *pPanel;   /**<  Panel object */
    IEvtArgs *pOrgEvt; /**<  Original event arguments */
})

DEF_EVT(EventTabSelChanging, EVT_TAB_SELCHANGING, on_tab_sel_changing, {
    UINT uOldSel; /**<  Old selected item */
    UINT uNewSel; /**<  New selected item */
    BOOL bCancel; /**<  Whether canceled */
})

DEF_EVT(EventTabSelChanged, EVT_TAB_SELCHANGED, on_tab_sel_changed, {
    UINT uOldSel; /**<  Old selected item */
    UINT uNewSel; /**<  New selected item */
})

DEF_EVT(EventTabItemHover, EVT_TAB_ITEMHOVER, on_tab_item_hover, { int iHover; })

DEF_EVT(EventTabItemLeave, EVT_TAB_ITEMLEAVE, on_tab_item_leave, { int iLeave; })

DEF_EVT(EventLBSelChanging, EVT_LB_SELCHANGING, on_listbox_sel_changing, {
    int nNewSel;  /**<  New selected item */
    int nOldSel;  /**<  Old selected item */
    BOOL bCancel; /**<  Whether canceled */
})

DEF_EVT(EventLBSelChanged, EVT_LB_SELCHANGED, on_listbox_sel_changed, {
    int nNewSel; /**<  New selected item */
    int nOldSel; /**<  Old selected item */
})

DEF_EVT(EventLBDbClick, EVT_LB_DBCLICK, on_listbox_item_dbclick, {
    int nCurSel;
    POINT pt;
})

DEF_EVT(EventLBRClick, EVT_LB_RCLICK, on_listbox_item_rclick, {
    int nCurSel;
    POINT pt;
})

DEF_EVT(EventLVSelChanged, EVT_LV_SELCHANGED, on_listview_select_changed, {
    int iOldSel; /**<  Previously selected item */
    int iNewSel; /**<  New selected item */
})

DEF_EVT(EventLVSelChanging, EVT_LV_SELCHANGING, on_listview_select_changing, {
    int iOldSel;  /**<  Previously selected item */
    int iNewSel;  /**<  New selected item */
    BOOL bCancel; /**<  Whether canceled */
})

DEF_EVT(EventTVSelChanged, EVT_TV_SELCHANGED, on_treeview_select_changed, {
    ULONG_PTR hOldSel; /**<  Previously selected item */
    ULONG_PTR hNewSel; /**<  New selected item */
})

DEF_EVT(EventTVSelChanging, EVT_TV_SELCHANGING, on_treeview_select_changing, {
    ULONG_PTR hOldSel; /**<  Previously selected item */
    ULONG_PTR hNewSel; /**<  New selected item */
    BOOL bCancel;      /**<  Whether canceled */
})

DEF_EVT(EventRENotify, EVT_RE_NOTIFY, on_richedit_notify, {
    DWORD iNotify; /**<  Notification code */
    LPVOID pv;     /**<  Notification data */
    HRESULT hr;    /**<  Return result */
})

DEF_EVT(EventREMenu, EVT_RE_MENU, on_richedit_menu, { UINT uCmd; })

typedef enum _SliderBarAction
{
    SBA_MOUSE_DOWN = 0,
    SBA_MOUSE_MOVING,
    SBA_MOUSE_UP,
} SliderBarAction;

DEF_EVT(EventSliderPos, EVT_SLIDER_POS, on_slider_pos, {
    int nPos;               /**<  Slider position */
    SliderBarAction action; /**<  Slider action */
})

DEF_EVT(EventSliderValueTip, EVT_SLIDER_VALUETIP, on_slider_valuetip, {
    int nPos;      /**<  Slider position */
    IStringT *buf; /**<  Value tip string */
})

/** Click header */
DEF_EVT(EventHeaderClick, EVT_HEADER_CLICK, on_header_click, { int iItem; })

/** Header width changing */
DEF_EVT(EventHeaderItemChanging, EVT_HEADER_ITEMCHANGING, on_header_item_changing, {
    int iItem;  /**<  Header item index */
    int nWidth; /**<  New width */
})

/** Header width changed */
DEF_EVT(EventHeaderItemChanged, EVT_HEADER_ITEMCHANGED, on_header_item_changed, {
    int iItem;  /**<  Header item index */
    int nWidth; /**<  New width */
})

/** Drag item to adjust position */
DEF_EVT(EventHeaderItemSwap, EVT_HEADER_ITEMSWAP, on_header_item_swap, {
    int iOldIndex; /**<  Old index */
    int iNewIndex; /**<  New index */
})

DEF_EVT(EventHeaderRelayout, EVT_HEADER_RELAYOUT, on_header_relayout, { int fake; })

DEF_EVT(EventCBSelChange, EVT_CB_SELCHANGE, on_combobox_sel_change, { int nCurSel; })

DEF_EVT(EventCBDropdown, EVT_CB_DROPDOWN, on_combobox_dropdown, {
    void *pDropDown;          /**<  Dropdown object */
    const IStringT *strInput; /**<  Input string */
})

DEF_EVT(EventCBBeforeCloseUp, EVT_CB_BEFORE_CLOSEUP, on_combobox_before_closeup, { BOOL bCloseBlock; })

DEF_EVT(EventLCSelChanging, EVT_LC_SELCHANGING, on_listctrl_sel_changing, {
    int nNewSel;  /**<  New selected item */
    int nOldSel;  /**<  Old selected item */
    BOOL bCancel; /**<  Whether canceled */
})

DEF_EVT(EventLCSelChanged, EVT_LC_SELCHANGED, on_listctrl_sel_changed, {
    int nNewSel; /**<  New selected item */
    int nOldSel; /**<  Old selected item */
})

DEF_EVT(EventLCDbClick, EVT_LC_DBCLICK, on_listctrl_item_dbclick, {
    int nCurSel;
    POINT pt;
})

DEF_EVT(EventLCRClick, EVT_LC_RCLICK, on_listctrl_item_rclick, {
    int nCurSel;
    POINT pt;
})

DEF_EVT(EventLCItemDeleted, EVT_LC_ITEMDELETED, on_listctrl_del_item, {
    int nItem;     /**<  Item index */
    LPARAM dwData; /**<  Item data */
})

DEF_EVT(EventCalendarSelDay, EVT_CALENDAR_SELDAY, on_calendar_sel_day, {
    WORD wOldDay; /**<  Old selected day */
    WORD wNewDay; /**<  New selected day */
})

DEF_EVT(EventCalendarSetDate, EVT_CALENDAR_SETDATE, on_calendar_set_date, { int fake; })

DEF_EVT(EventCalendarExChanged, EVT_CALENDAREX_CHANGED, on_calendarex_changed, {
    WORD iNewDay;   /**<  New day */
    WORD iNewMonth; /**<  New month */
    WORD iNewYear;  /**<  New year */
    int nBtnType;   /**<  Button type */
})

DEF_EVT(EventDateTimeChanged, EVT_DATETIME_CHANGED, on_datetime_changed, { SYSTEMTIME newTime; })

DEF_EVT(EventTCSelChanging, EVT_TC_SELCHANGING, on_treectrl_sel_changing, {
    HSTREEITEM hOldSel; /**<  Old selected item */
    HSTREEITEM hNewSel; /**<  New selected item */
    BOOL bCancel;       /**<  Whether canceled */
})

DEF_EVT(EventTCSelChanged, EVT_TC_SELCHANGED, on_treectrl_sel_changed, {
    HSTREEITEM hOldSel; /**<  Old selected item */
    HSTREEITEM hNewSel; /**<  New selected item */
})

DEF_EVT(EventTCCheckState, EVT_TC_CHECKSTATE, on_treectrl_item_check, {
    HSTREEITEM hItem; /**<  Item handle */
    UINT uCheckState; /**<  Check state */
})

DEF_EVT(EventTCExpand, EVT_TC_EXPAND, on_treectrl_item_expand, {
    HSTREEITEM hItem; /**<  Item handle */
    BOOL bCollapsed;  /**<  Whether collapsed */
})

/** Double-click treectrl leaf node add by zhaosheng */
DEF_EVT(EventTCDbClick, EVT_TC_DBCLICK, on_treectrl_item_dbclick, {
    HSTREEITEM hItem; /**<  Double-clicked selected node */
    BOOL bCancel;     /**<  Whether canceled */
})

DEF_EVT(EventTCRClick, EVT_TC_RCLICK, on_treectrl_item_rclick, {
    POINT pt;         /**<  Right-click position */
    HSTREEITEM hItem; /**<  Right-clicked selected node */
})

DEF_EVT(EventSplitPaneMoved, EVT_SPLIT_PANE_MOVED, on_split_pane_moved, { RECT rcPane; })

DEF_EVT(EventImageAnimateStart, EVT_IMAGE_ANI_START, on_image_animate_start, { int fake; })

DEF_EVT(EventImageAnimateStop, EVT_IMAGE_ANI_STOP, on_image_animate_stop, { int fake; })

DEF_EVT(EventImageAnimateRepeat, EVT_IMAGE_ANI_REPEAT, on_image_animate_repeat, { int fake; })

DEF_EVT(EventSelectMenu, EVT_SELECTMENU, on_select_menu, {
    UINT nMenuId; /**<  Menu item ID */
    void *pMenu;  /**<  Menu object */
})

DEF_EVT(EventPopMenu, EVT_POPMENU, on_pop_menu, {
    UINT nMenuIndex; /**<  Menu index */
    void *pMenu;     /**<  Menu object */
})

DEF_EVT(EventSetHotKey, EVT_HOT_KEY_SET, on_hot_key_set_event, {
    WORD vKey;       /**<  Virtual key code */
    WORD wModifiers; /**<  Modifier key */
})

DEF_EVT(EventRealWndCreate, EVT_REALWND_CREATE, on_real_wnd_create, { HWND hRet; })

DEF_EVT(EventRealWndDestroy, EVT_REALWND_DESTROY, on_real_wnd_destroy, { int fake; })

DEF_EVT(EventRealWndInit, EVT_REALWND_INIT, on_real_wnd_init, { BOOL bRet; })

DEF_EVT(EventRealWndPosition, EVT_REALWND_POSITION, on_real_wnd_position, {
    RECT rc;   /**<  Window position rect */
    BOOL bRet; /**<  Return value */
})

typedef struct IListView IListView;

DEF_EVT(EventDropdownListGetBuddyRect, EVT_DROPDOWN_LIST_GET_BUDDY_RECT, on_dropdown_list_get_buddy_rect, { RECT rcBuddy; })

DEF_EVT(EventDropdownListFill, EVT_DROPDOWN_LIST_FILL, on_dropdown_list_fill, {
    const IStringT *strKey;
    IListView *pListView;
    BOOL bPopup;
})

DEF_EVT(EventDropdownListSelected, EVT_DROPDOWN_LIST_SELECTED, on_dropdown_list_selected, {
    IListView *pListView;
    int nValue;
})

DEF_EVT(EventToolBarCmd, EVT_TB_ITEMCLICK, on_toolbar_cmd, {
    int iItem;
    int nCmdId;
    LPARAM lParam;
    BOOL bDropDown;
})

DEF_EVT(EventUpdateCmdTip, EVT_UPDATE_ITEMTIP, on_update_cmd_tip, {
    int iIndex;
    int nCmdId;
    const IStringT *strTip;
})

DEF_EVT(EventUpdateCmdUI, EVT_UPDATE_CMDUI, on_update_cmd_ui, {
    int iIndex;
    int nCmdId;
    BOOL bEnable;
    BOOL bChecked;
})

DEF_EVT(EventExitModalView, EVT_EXIT_MODALVIEW, on_exit_modal_view, { int exitCode; })

SNSEND

#endif /**< __SEVENTS__H__ */
