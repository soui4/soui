/**
 * @file       SEvents.h
 * @brief      SOUI系统中使用的事件系统
 * @version    v1.0
 * @author     SOUI group
 * @date       2014/08/02
 *
 * @details    定义了SOUI系统中使用的各种事件及其处理机制。
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
 * @brief 定义了SOUI系统中的事件ID
 */
typedef enum _SOUI_EVENTS
{
    EVT_INIT = 8000, ///< 初始化事件
    EVT_EXIT,        ///< 退出事件
    EVT_TIMER,       ///< 定时器事件
    EVT_GET_CARET,   ///< 获取光标事件

    // 基本窗口事件
    EVT_SETFOCUS = 8100,  ///< 设置焦点事件
    EVT_KILLFOCUS,        ///< 杀死焦点事件
    EVT_CREATE,           ///< 创建事件
    EVT_INIT_FINISH,      ///< 初始化完成事件
    EVT_DESTROY,          ///< 销毁事件
    EVT_SIZE,             ///< 大小改变事件
    EVT_VISIBLECHANGED,   ///< 可见性改变事件
    EVT_STATECHANGED,     ///< 状态改变事件
    EVT_CAPTURECHANGED,   ///< 捕获改变事件
    EVT_MOUSE_CLICK,      ///< 鼠标点击事件
    EVT_UPDATE_TOOLTIP,   ///< 更新工具提示事件
    EVT_ANIMATION_START,  ///< 动画开始事件
    EVT_ANIMATION_STOP,   ///< 动画停止事件
    EVT_ANIMATION_REPEAT, ///< 动画重复事件
    EVT_POS,              ///< 位置改变事件
    EVT_ANIMATOR_FRACTOR, ///< 数值动画事件

    EVT_KEYDOWN = 8200, ///< 键盘按下事件

    // 两个窗口鼠标状态事件
    EVT_MOUSE_HOVER = 9000, ///< 鼠标悬停事件
    EVT_MOUSE_LEAVE,        ///< 鼠标离开事件

    EVT_CMD = 10000, ///< 命令事件
    EVT_MENU_CMD,    ///< 菜单命令事件
    EVT_CTXMENU,     ///< 上下文菜单事件

    // 增加两个滚动VIEW的事件
    EVT_SCROLLVIEW_ORIGINCHANGED, ///< 滚动视图原点改变事件
    EVT_SCROLLVIEW_SIZECHANGED,   ///< 滚动视图大小改变事件

    EVT_SCROLL,  ///< 滚动事件
    EVT_OFEVENT, ///< 消息二次包装事件
    EVT_OFPANEL, ///< 面板中的消息二次包装事件

    // 增加4个itempanel相关的事件,经过EVT_OFPANEL到达应用层
    EVT_ITEMPANEL_CLICK,     ///< 面板点击事件
    EVT_ITEMPANEL_RCLICK,    ///< 面板右键点击事件
    EVT_ITEMPANEL_CLICK_UP,  ///< 面板点击释放事件
    EVT_ITEMPANEL_RCLICK_UP, ///< 面板右键点击释放事件
    EVT_ITEMPANEL_DBCLICK,   ///< 面板双击事件
    EVT_ITEMPANEL_HOVER,     ///< 面板悬停事件
    EVT_ITEMPANEL_LEAVE,     ///< 面板离开事件

    EVT_RADIOGROUP_CHECK_CHANGED = 10100, ///< 单选按钮组选中状态改变事件

    EVT_TAB_SELCHANGING = 11000, ///< 选项卡选择改变中事件
    EVT_TAB_SELCHANGED,          ///< 选项卡选择改变事件
    EVT_TAB_ITEMHOVER,           ///< 选项卡项悬停事件
    EVT_TAB_ITEMLEAVE,           ///< 选项卡项离开事件

    EVT_LB_SELCHANGING = 12000, ///< 列表框选择改变中事件
    EVT_LB_SELCHANGED,          ///< 列表框选择改变事件
    EVT_LB_DBCLICK,             ///< 列表框双击事件

    EVT_LC_SELCHANGING = 13000, ///< 列表控件选择改变中事件
    EVT_LC_SELCHANGED,          ///< 列表控件选择改变事件
    EVT_LC_ITEMDELETED,         ///< 列表控件项删除事件
    EVT_LC_DBCLICK,             ///< 列表控件双击事件

    EVT_TV_SELCHANGING = 14000, ///< 树控件选择改变中事件
    EVT_TV_SELCHANGED,          ///< 树控件选择改变事件

    EVT_TC_SELCHANGING = 15000, ///< 树形控件选择改变中事件
    EVT_TC_SELCHANGED,          ///< 树形控件选择改变事件
    EVT_TC_EXPAND,              ///< 树形控件展开事件
    EVT_TC_CHECKSTATE,          ///< 树形控件检查状态改变事件
    EVT_TC_DBCLICK,             ///< 树形控件双击事件
    EVT_CB_BEFORE_CLOSEUP,      ///< 组合框关闭前事件

    EVT_LV_SELCHANGING = 15100, ///< 列表视图选择改变中事件
    EVT_LV_SELCHANGED,          ///< 列表视图选择改变事件
    EVT_LV_ITEMCLICK,           ///< 列表视图项点击事件

    EVT_RE_NOTIFY = 16000, ///< 丰富编辑控件通知事件
    EVT_RE_MENU,           ///< 丰富编辑控件菜单事件

    EVT_SLIDER_POS = 17000, ///< 滑块位置改变事件
    EVT_SLIDER_VALUETIP,    ///< 滑块值提示事件

    EVT_HEADER_CLICK = 18000, ///< 表头点击事件
    EVT_HEADER_ITEMCHANGING,  ///< 表头项改变中事件
    EVT_HEADER_ITEMCHANGED,   ///< 表头项改变事件
    EVT_HEADER_ITEMSWAP,      ///< 表头项交换事件
    EVT_HEADER_RELAYOUT,      ///< 表头重新布局事件

    EVT_CB_SELCHANGE = 19000, ///< 组合框选择改变事件
    EVT_CB_DROPDOWN,          ///< 组合框下拉事件

    EVT_CALENDAR_SELDAY = 20000, ///< 日历选择日期事件
    EVT_CALENDAR_SETDATE,        ///< 日历设置日期事件
    EVT_CALENDAREX_CHANGED,      ///< 扩展日历改变事件
    EVT_DATETIME_CHANGED,        ///< 日期时间改变事件

    EVT_SPIN_VALUE2STRING = 21000, ///< 旋转框值转字符串事件

    EVT_SPLIT_PANE_MOVED = 22000, ///< 分割窗格移动事件

    EVT_HOT_KEY_SET = 23000, ///< 热键设置事件

    // 图片动画开始，结束事件
    EVT_IMAGE_ANI_START = 22100, ///< 图片动画开始事件
    EVT_IMAGE_ANI_STOP,          ///< 图片动画停止事件
    EVT_IMAGE_ANI_REPEAT,        ///< 图片动画重复事件

    EVT_SELECTMENU = 22150, ///< 菜单项选择事件
    EVT_POPMENU,            ///< 弹出菜单事件

    EVT_REALWND_CREATE = 22160, ///< 真实窗口创建事件
    EVT_REALWND_DESTROY,        ///< 真实窗口销毁事件
    EVT_REALWND_INIT,           ///< 真实窗口初始化事件
    EVT_REALWND_POSITION,       ///< 真实窗口位置改变事件

    EVT_DROPDOWN_LIST_FILL = 22170,
    EVT_DROPDOWN_LIST_SELECTED,
    EVT_DROPDOWN_LIST_GET_BUDDY_RECT,

    EVT_EXTERNAL_BEGIN = 10000000, ///< 外部事件开始ID
} SOUI_EVENTS;

/**
 * @enum _MouseClickId
 * @brief 定义了鼠标点击类型
 */
typedef enum _MouseClickId
{
    MOUSE_LBTN_DOWN = 0, ///< 左键按下
    MOUSE_LBTN_UP,       ///< 左键释放
    MOUSE_LBTN_DBCLICK,  ///< 左键双击
    MOUSE_RBTN_DOWN,     ///< 右键按下
    MOUSE_RBTN_UP,       ///< 右键释放
    MOUSE_RBTN_DBCLICK,  ///< 右键双击
    MOUSE_MBTN_DOWN,     ///< 中键按下
    MOUSE_MBTN_UP,       ///< 中键释放
    MOUSE_MBTN_DBCLICK,  ///< 中键双击
} MouseClickId;

#ifdef __cplusplus
/**
 * @class SEvtArgs
 * @brief 事件参数基类
 */
class SOUI_EXP SEvtArgs : public TObjRefImpl<SObjectImpl<IEvtArgs>> {
    DEF_SOBJECT(SObjectImpl<IEvtArgs>, L"event")

  public:
    UINT handled;     ///< 处理计数
    BOOL bubbleUp;    ///< 是否冒泡
    int idFrom;       ///< 发送者ID
    LPCWSTR nameFrom; ///< 发送者名称
    IObject *sender;  ///< 发送者对象

    /**
     * @brief 获取发送者对象
     * @return 发送者对象指针
     */
    STDMETHOD_(IObject *, Sender)(THIS)
    {
        return sender;
    }

    /**
     * @brief 获取发送者ID
     * @return 发送者ID
     */
    STDMETHOD_(int, IdFrom)(THIS) SCONST
    {
        return idFrom;
    }

    /**
     * @brief 设置发送者ID
     * @param id 发送者ID
     */
    STDMETHOD_(void, SetIdFrom)(THIS_ int id)
    {
        idFrom = id;
    }

    /**
     * @brief 获取发送者名称
     * @return 发送者名称
     */
    STDMETHOD_(LPCWSTR, NameFrom)(THIS) SCONST
    {
        return nameFrom;
    }

    /**
     * @brief 设置发送者名称
     * @param name 发送者名称
     */
    STDMETHOD_(void, SetNameFrom)(THIS_ LPCWSTR name)
    {
        nameFrom = name;
    }

    /**
     * @brief 检查事件是否冒泡
     * @return TRUE if 冒泡，FALSE otherwise
     */
    STDMETHOD_(BOOL, IsBubbleUp)(THIS) SCONST
    {
        return bubbleUp;
    }

    /**
     * @brief 设置事件是否冒泡
     * @param bSet 是否冒泡
     */
    STDMETHOD_(void, SetBubbleUp)(THIS_ BOOL bSet)
    {
        bubbleUp = bSet;
    }

    /**
     * @brief 获取处理计数
     * @return 处理计数
     */
    STDMETHOD_(UINT, HandleCount)(THIS) SCONST
    {
        return handled;
    }

    /**
     * @brief 增加处理计数
     */
    STDMETHOD_(void, IncreaseHandleCount)(THIS)
    {
        handled++;
    }

    /**
     * @brief 获取事件数据
     * @return 事件数据指针
     */
    STDMETHOD_(LPVOID, Data)(THIS)
    {
        return NULL;
    }

  public:
    /**
     * @brief 构造函数
     * @param pSender 发送者对象
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
     * @brief 析构函数
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

// 定义一组事件定义的宏，简化事件的定义。
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

#endif // __cplusplus

#define DEF_EVT_STRUCT(n, x) typedef struct n x n;

#ifdef __cplusplus
#define DEF_EVENT(evt, id, name, x, api) \
    DEF_EVT_STRUCT(St##evt, x)           \
    DEF_EVT_CLASS(evt, id, name, St##evt, api)
#else
#define DEF_EVENT(evt, id, name, x, api) DEF_EVT_STRUCT(St##evt, x)
#endif // __cplusplus

#define DEF_EVT(evt, id, name, x) DEF_EVENT(evt, id, name, x, SOUI_EXP)
#define DEF_EVT_EXT(evt, id, x)   DEF_EVENT(evt, id, on_##evt, x, )

DEF_EVT(EventInit, EVT_INIT, on_init, { int fake; })

DEF_EVT(EventExit, EVT_EXIT, on_exit, { int fake; })

DEF_EVT(EventTimer, EVT_TIMER, on_timer, {
    UINT uID;     ///< 定时器ID
    LPARAM uData; ///< 定时器数据
})

DEF_EVT(EventGetCaret, EVT_GET_CARET, on_get_caret, { IStringW *strCaret; })

DEF_EVT(EventScroll, EVT_SCROLL, on_scroll, {
    int nSbCode;    ///< 滚动条代码
    int nPos;       ///< 滚动条位置
    BOOL bVertical; ///< 是否垂直滚动条
})

DEF_EVT(EventSpinValue2String, EVT_SPIN_VALUE2STRING, on_spin_valuetostring, {
    BOOL bInit;         ///< 是否初始化
    int nValue;         ///< 旋转框值
    IStringT *strValue; ///< 字符串值
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
    DWORD dwOldState; ///< 旧状态
    DWORD dwNewState; ///< 新状态
})

#ifdef __cplusplus
/**
 * @brief 检查状态是否改变
 * @param pEvt 状态改变事件指针
 * @param dwState 状态
 * @return TRUE if 状态改变，FALSE otherwise
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
    UINT nChar;   ///< 字符码
    UINT nFlags;  ///< 标志
    BOOL bCancel; ///< 是否取消
})

DEF_EVT(EventSwndMouseLeave, EVT_MOUSE_LEAVE, on_mouse_leave, { int fake; })

DEF_EVT(EventSwndMouseHover, EVT_MOUSE_HOVER, on_mouse_hover, { int fake; })

DEF_EVT(EventSwndUpdateTooltip, EVT_UPDATE_TOOLTIP, on_update_tooltip, {
    IStringT *strToolTip; ///< 工具提示字符串
    BOOL bUpdated;        ///< 是否更新
})

DEF_EVT(EventRadioGroupCheckChanged, EVT_RADIOGROUP_CHECK_CHANGED, on_radiogroup_check_changed, { IWindow *pChecked; })

DEF_EVT(EventItemPanelRclick, EVT_ITEMPANEL_RCLICK, on_itemparem_rclick, {
    WPARAM wParam; ///< WPARAM参数
    LPARAM lParam; ///< LPARAM参数
})

DEF_EVT(EventItemPanelDbclick, EVT_ITEMPANEL_DBCLICK, on_itempanel_dbclick, {
    WPARAM wParam; ///< WPARAM参数
    LPARAM lParam; ///< LPARAM参数
})

DEF_EVT(EventItemPanelClick, EVT_ITEMPANEL_CLICK, on_itempanel_click, {
    WPARAM wParam; ///< WPARAM参数
    LPARAM lParam; ///< LPARAM参数
})

DEF_EVT(EventItemPanelRclickUp, EVT_ITEMPANEL_RCLICK_UP, on_itemparem_rclick_up, {
    WPARAM wParam; ///< WPARAM参数
    LPARAM lParam; ///< LPARAM参数
})

DEF_EVT(EventItemPanelClickUp, EVT_ITEMPANEL_CLICK_UP, on_itempanel_click_up, {
    WPARAM wParam; ///< WPARAM参数
    LPARAM lParam; ///< LPARAM参数
})

DEF_EVT(EventItemPanelHover, EVT_ITEMPANEL_HOVER, on_itempanel_hover, {
    WPARAM wParam; ///< WPARAM参数
    LPARAM lParam; ///< LPARAM参数
})

// 注：在EventItemPanelLeave中从IItemPanel中通过GetItemIndex获取表项索引时需要检查索引有效性。
DEF_EVT(EventItemPanelLeave, EVT_ITEMPANEL_LEAVE, on_itempanel_leave, { int fake; })

DEF_EVT(EventMouseClick, EVT_MOUSE_CLICK, on_mouse_click, {
    POINT pt;             ///< 鼠标点击位置
    UINT uFlags;          ///< 鼠标点击标志
    MouseClickId clickId; ///< 鼠标点击类型
    BOOL bHover;          ///< 是否悬停
})

DEF_EVT(EventCmd, EVT_CMD, on_command, { int fake; })

DEF_EVT(EventMenuCmd, EVT_MENU_CMD, on_menu_command, { int menuId; })

DEF_EVT(EventCtxMenu, EVT_CTXMENU, on_conext_menu, {
    POINT pt;     ///< 上下文菜单位置
    BOOL bCancel; ///< 是否取消
})

DEF_EVT(EventSetFocus, EVT_SETFOCUS, on_set_focus, { SWND wndOld; })

DEF_EVT(EventKillFocus, EVT_KILLFOCUS, on_kill_focus, { SWND wndFocus; })

DEF_EVT(EventScrollViewOriginChanged, EVT_SCROLLVIEW_ORIGINCHANGED, on_scrollview_origin_changed, {
    POINT ptOldOrigin; ///< 旧原点位置
    POINT ptNewOrigin; ///< 新原点位置
})

DEF_EVT(EventScrollViewSizeChanged, EVT_SCROLLVIEW_SIZECHANGED, on_scrollview_size_changed, {
    SIZE szOldViewSize; ///< 旧视图大小
    SIZE szNewViewSize; ///< 新视图大小
})

DEF_EVT(EventOfEvent, EVT_OFEVENT, on_event_of_event, { IEvtArgs *pOrgEvt; })

DEF_EVT(EventOfPanel, EVT_OFPANEL, on_event_of_panel, {
    IWindow *pPanel;   ///< 面板对象
    IEvtArgs *pOrgEvt; ///< 原始事件参数
})

DEF_EVT(EventTabSelChanging, EVT_TAB_SELCHANGING, on_tab_sel_changing, {
    UINT uOldSel; ///< 旧选中项
    UINT uNewSel; ///< 新选中项
    BOOL bCancel; ///< 是否取消
})

DEF_EVT(EventTabSelChanged, EVT_TAB_SELCHANGED, on_tab_sel_changed, {
    UINT uOldSel; ///< 旧选中项
    UINT uNewSel; ///< 新选中项
})

DEF_EVT(EventTabItemHover, EVT_TAB_ITEMHOVER, on_tab_item_hover, { int iHover; })

DEF_EVT(EventTabItemLeave, EVT_TAB_ITEMLEAVE, on_tab_item_leave, { int iLeave; })

DEF_EVT(EventLBSelChanging, EVT_LB_SELCHANGING, on_listbox_sel_changing, {
    int nNewSel;  ///< 新选中项
    int nOldSel;  ///< 旧选中项
    BOOL bCancel; ///< 是否取消
})

DEF_EVT(EventLBSelChanged, EVT_LB_SELCHANGED, on_listbox_sel_changed, {
    int nNewSel; ///< 新选中项
    int nOldSel; ///< 旧选中项
})

DEF_EVT(EventLBDbClick, EVT_LB_DBCLICK, on_listbox_item_dbclick, { int nCurSel; })

DEF_EVT(EventLVSelChanged, EVT_LV_SELCHANGED, on_listview_select_changed, {
    int iOldSel; ///< 原选中项
    int iNewSel; ///< 新选中项
})

DEF_EVT(EventLVSelChanging, EVT_LV_SELCHANGING, on_listview_select_changing, {
    int iOldSel;  ///< 原选中项
    int iNewSel;  ///< 新选中项
    BOOL bCancel; ///< 是否取消
})

DEF_EVT(EventTVSelChanged, EVT_TV_SELCHANGED, on_treeview_select_changed, {
    ULONG_PTR hOldSel; ///< 原选中项
    ULONG_PTR hNewSel; ///< 新选中项
})

DEF_EVT(EventTVSelChanging, EVT_TV_SELCHANGING, on_treeview_select_changing, {
    ULONG_PTR hOldSel; ///< 原选中项
    ULONG_PTR hNewSel; ///< 新选中项
    BOOL bCancel;      ///< 是否取消
})

DEF_EVT(EventRENotify, EVT_RE_NOTIFY, on_richedit_notify, {
    DWORD iNotify; ///< 通知码
    LPVOID pv;     ///< 通知数据
    HRESULT hr;    ///< 返回结果
})

DEF_EVT(EventREMenu, EVT_RE_MENU, on_richedit_menu, { UINT uCmd; })

typedef enum _SliderBarAction
{
    SBA_MOUSE_DOWN = 0,
    SBA_MOUSE_MOVING,
    SBA_MOUSE_UP
} SliderBarAction;

DEF_EVT(EventSliderPos, EVT_SLIDER_POS, on_slider_pos, {
    int nPos;               ///< 滑块位置
    SliderBarAction action; ///< 滑块动作
})

DEF_EVT(EventSliderValueTip, EVT_SLIDER_VALUETIP, on_slider_valuetip, {
    int nPos;      ///< 滑块位置
    IStringT *buf; ///< 值提示字符串
})

// 点击表头
DEF_EVT(EventHeaderClick, EVT_HEADER_CLICK, on_header_click, { int iItem; })

// 表头宽度改变中
DEF_EVT(EventHeaderItemChanging, EVT_HEADER_ITEMCHANGING, on_header_item_changing, {
    int iItem;  ///< 表头项索引
    int nWidth; ///< 新宽度
})

// 表头宽度改变
DEF_EVT(EventHeaderItemChanged, EVT_HEADER_ITEMCHANGED, on_header_item_changed, {
    int iItem;  ///< 表头项索引
    int nWidth; ///< 新宽度
})

// 拖动表项调整位置
DEF_EVT(EventHeaderItemSwap, EVT_HEADER_ITEMSWAP, on_header_item_swap, {
    int iOldIndex; ///< 旧索引
    int iNewIndex; ///< 新索引
})

DEF_EVT(EventHeaderRelayout, EVT_HEADER_RELAYOUT, on_header_relayout, { int fake; })

DEF_EVT(EventCBSelChange, EVT_CB_SELCHANGE, on_combobox_sel_change, { int nCurSel; })

DEF_EVT(EventCBDropdown, EVT_CB_DROPDOWN, on_combobox_dropdown, {
    void *pDropDown;          ///< 下拉对象
    const IStringT *strInput; ///< 输入字符串
})

DEF_EVT(EventCBBeforeCloseUp, EVT_CB_BEFORE_CLOSEUP, on_combobox_before_closeup, { BOOL bCloseBlock; })

DEF_EVT(EventLCSelChanging, EVT_LC_SELCHANGING, on_listctrl_sel_changing, {
    int nNewSel;  ///< 新选中项
    int nOldSel;  ///< 旧选中项
    BOOL bCancel; ///< 是否取消
})

DEF_EVT(EventLCSelChanged, EVT_LC_SELCHANGED, on_listctrl_sel_changed, {
    int nNewSel; ///< 新选中项
    int nOldSel; ///< 旧选中项
})

DEF_EVT(EventLCDbClick, EVT_LC_DBCLICK, on_listctrl_item_dbclick, { int nCurSel; })

DEF_EVT(EventLCItemDeleted, EVT_LC_ITEMDELETED, on_listctrl_del_item, {
    int nItem;     ///< 项索引
    LPARAM dwData; ///< 项数据
})

DEF_EVT(EventCalendarSelDay, EVT_CALENDAR_SELDAY, on_calendar_sel_day, {
    WORD wOldDay; ///< 旧选中日
    WORD wNewDay; ///< 新选中日
})

DEF_EVT(EventCalendarSetDate, EVT_CALENDAR_SETDATE, on_calendar_set_date, { int fake; })

DEF_EVT(EventCalendarExChanged, EVT_CALENDAREX_CHANGED, on_calendarex_changed, {
    WORD iNewDay;   ///< 新日
    WORD iNewMonth; ///< 新月
    WORD iNewYear;  ///< 新年
    int nBtnType;   ///< 按钮类型
})

DEF_EVT(EventDateTimeChanged, EVT_DATETIME_CHANGED, on_datetime_changed, { SYSTEMTIME newTime; })

DEF_EVT(EventTCSelChanging, EVT_TC_SELCHANGING, on_treectrl_sel_changing, {
    HSTREEITEM hOldSel; ///< 旧选中项
    HSTREEITEM hNewSel; ///< 新选中项
    BOOL bCancel;       ///< 是否取消
})

DEF_EVT(EventTCSelChanged, EVT_TC_SELCHANGED, on_treectrl_sel_changed, {
    HSTREEITEM hOldSel; ///< 旧选中项
    HSTREEITEM hNewSel; ///< 新选中项
})

DEF_EVT(EventTCCheckState, EVT_TC_CHECKSTATE, on_treectrl_item_check, {
    HSTREEITEM hItem; ///< 项句柄
    UINT uCheckState; ///< 检查状态
})

DEF_EVT(EventTCExpand, EVT_TC_EXPAND, on_treectrl_item_expand, {
    HSTREEITEM hItem; ///< 项句柄
    BOOL bCollapsed;  ///< 是否折叠
})

// 双击treectrl的叶子节点 add by zhaosheng
DEF_EVT(EventTCDbClick, EVT_TC_DBCLICK, on_treectrl_item_dbclick, {
    HSTREEITEM hItem; ///< 双击选中的节点
    BOOL bCancel;     ///< 是否取消
})

DEF_EVT(EventSplitPaneMoved, EVT_SPLIT_PANE_MOVED, on_split_pane_moved, { RECT rcPane; })

DEF_EVT(EventImageAnimateStart, EVT_IMAGE_ANI_START, on_image_animate_start, { int fake; })

DEF_EVT(EventImageAnimateStop, EVT_IMAGE_ANI_STOP, on_image_animate_stop, { int fake; })

DEF_EVT(EventImageAnimateRepeat, EVT_IMAGE_ANI_REPEAT, on_image_animate_repeat, { int fake; })

DEF_EVT(EventSelectMenu, EVT_SELECTMENU, on_select_menu, {
    UINT nMenuId; ///< 菜单项ID
    void *pMenu;  ///< 菜单对象
})

DEF_EVT(EventPopMenu, EVT_POPMENU, on_pop_menu, {
    UINT nMenuIndex; ///< 菜单索引
    void *pMenu;     ///< 菜单对象
})

DEF_EVT(EventSetHotKey, EVT_HOT_KEY_SET, on_hot_key_set_event, {
    WORD vKey;       ///< 虚拟键码
    WORD wModifiers; ///< 修饰键
})

DEF_EVT(EventRealWndCreate, EVT_REALWND_CREATE, on_real_wnd_create, { HWND hRet; })

DEF_EVT(EventRealWndDestroy, EVT_REALWND_DESTROY, on_real_wnd_destroy, { int fake; })

DEF_EVT(EventRealWndInit, EVT_REALWND_INIT, on_real_wnd_init, { BOOL bRet; })

DEF_EVT(EventRealWndPosition, EVT_REALWND_POSITION, on_real_wnd_position, {
    RECT rc;   ///< 窗口位置矩形
    BOOL bRet; ///< 返回值
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

SNSEND

#endif // __SEVENTS__H__
