/**
 * @file       souitest.h
 * @brief      SouiTest —— SOUI4 UI 自动化测试支撑库（参考 Qt Test / QAccessible 范式）
 *
 * 设计依据见 doc/SOUI4-UI自动化测试框架设计.md。本文件只调用 SOUI4 既有公共 API，
 * 对 soui4 / utilities4 零侵入：
 *   - 对象定位  : FindChildByName2 / FindChildByNamePath / GetWindow / IsClass
 *   - 框架级注入: SWindow::SSendMessage（无需原生窗口，可无头运行）
 *   - 行为验证  : SubscribeEvent / FireCommand / FireEvent
 *   - 事件循环  : IMessageLoop（Run/PostTask/PeekMsg/HandleMsg）
 *
 * 全部为头文件实现，作为 fun_test 的 soui-uitest 层与未来外部 E2E（Squish 式）共享。
 */
#ifndef __SOUITEST_H__
#define __SOUITEST_H__

#include <souistd.h>
#include <core/SWnd.h>
#include <core/SWindowMgr.h>
#include <event/SEvents.h>
#include <gtest/gtest.h>

#include <vector>

namespace SouiTest
{
using namespace SOUI;

// ===========================================================================
// 1. 几何辅助：点击坐标按宿主坐标系计算（与 GetWindowRect 同一坐标系）
// ===========================================================================
inline CPoint centerOf(SWindow *w)
{
    CRect rc = w->GetWindowRect();
    return CPoint((rc.left + rc.right) / 2, (rc.top + rc.bottom) / 2);
}

// ===========================================================================
// 2. 输入注入层（对齐 QTest::mouseClick / keyClick）
// ===========================================================================
// Tier A —— 框架级（无头，主推）：直接把 WM_* 送进窗口处理链，不依赖原生窗口/显示。
// 顺序与 SWindow::OnLButtonUp 的判定保持一致：DOWN 置 PushDown+SetCapture，
// UP 在落点位于 GetWindowRect() 内时 FireCommand。
inline void mouseClick(SWindow *wnd, CPoint pt)
{
    wnd->SSendMessage(WM_LBUTTONDOWN, MK_LBUTTON, MAKELPARAM(pt.x, pt.y));
    wnd->SSendMessage(WM_LBUTTONUP, 0, MAKELPARAM(pt.x, pt.y));
}

inline void mouseClick(SWindow *wnd)
{
    mouseClick(wnd, centerOf(wnd));
}

inline void mousePress(SWindow *wnd, CPoint pt)
{
    wnd->SSendMessage(WM_LBUTTONDOWN, MK_LBUTTON, MAKELPARAM(pt.x, pt.y));
}

inline void mouseRelease(SWindow *wnd, CPoint pt)
{
    wnd->SSendMessage(WM_LBUTTONUP, 0, MAKELPARAM(pt.x, pt.y));
}

inline void mouseMove(SWindow *wnd, CPoint pt)
{
    wnd->SSendMessage(WM_MOUSEMOVE, 0, MAKELPARAM(pt.x, pt.y));
}

// 键盘：DOWN/UP 触发按钮的 PushDown/Command；CHAR 供编辑框输入路径。
inline void keyClick(SWindow *wnd, UINT vk)
{
    wnd->SSendMessage(WM_KEYDOWN, vk, 0);
    wnd->SSendMessage(WM_KEYUP, vk, 0);
    wnd->SSendMessage(WM_CHAR, vk, 0);
}

// Tier B —— 原生级（需显示，保真）：向宿主 hwnd 发原生消息。仅用于对照/保真用例。

inline void nativeMouseClick(HWND hwnd, CPoint clientPt)
{
    LPARAM lp = MAKELPARAM(clientPt.x, clientPt.y);
    ::SendMessage(hwnd, WM_MOUSEMOVE, 0, lp);
    ::SendMessage(hwnd, WM_LBUTTONDOWN, MK_LBUTTON, lp);
    ::SendMessage(hwnd, WM_LBUTTONUP, 0, lp);
}

// ===========================================================================
// 3. 对象定位层（对齐 QObject::findChild）
// ===========================================================================
// 按 name 属性查找（强类型）。name 即 XML 的 name="..."，是定位键（见 SobjectApi.h:77）。
template <class T>
T *findByName(SWindow *root, LPCWSTR name, int deep = -1)
{
    return root->FindChildByName2<T>(name, deep);
}

template <class T>
T *findByName(SWindow *root, LPCSTR name, int deep = -1)
{
    return root->FindChildByName2<T>(name, deep);
}

// 按 "page1/btn_ok" 路径查找（复用 FindChildByNamePath）。
inline SWindow *findByPath(SWindow *root, LPCWSTR path)
{
    return root->FindChildByNamePath(path);
}
inline SWindow *findByPath(SWindow *root, LPCSTR path)
{
    return root->FindChildByNamePath(path);
}

namespace internal
{
// 深度优先遍历控件树（与 SWnd.h 内部 FindChildByName2 同源的 GSW_FIRSTCHILD/NEXTSIBLING 遍历）。
template <typename Pred>
SWindow *dfsFirst(SWindow *w, Pred pred)
{
    if (!w)
        return NULL;
    if (pred(w))
        return w;
    SWindow *child = w->GetWindow(GSW_FIRSTCHILD);
    while (child)
    {
        SWindow *found = dfsFirst(child, pred);
        if (found)
            return found;
        child = child->GetWindow(GSW_NEXTSIBLING);
    }
    return NULL;
}

// 按类名匹配的谓词：C++03 下没有 lambda，用一个轻量函数对象代替。
struct ClassNamePred
{
    LPCWSTR m_clsName;

    explicit ClassNamePred(LPCWSTR clsName)
        : m_clsName(clsName)
    {
    }

    bool operator()(SWindow *w) const { return w->IsClass(m_clsName); }
};

/**
 * @brief 判断 SWindow 指针是否仍指向一个存活窗口。
 *
 * 场景：Spy 常常比宿主控件活得更久。例如
 *     SignalSpy spy(btn, EventCmd::EventID);
 *     root.DestroyAllChildren();   // btn 在这里被 delete
 * 随后 spy 析构时若直接 btn->UnsubscribeEvent(...) 就是对已释放对象解引用。
 *
 * 判定依据：每个 SWindow 在构造时由 SWindowMgr::NewWindow 分配一个单调递增、
 * **永不复用**的 SWND 句柄（SWindowMgr.cpp:NewWindow 里 ++m_hNextWnd），析构时在
 * ~SWindow 里 SWindowMgr::DestroyWindow(m_swnd) 注销。因此"该句柄仍映射到同一个
 * SWindow 指针"即可证明对象未被销毁，也能防住"新窗口恰好分配到同一地址"。
 */
inline bool isAlive(SWindow *wnd, SWND swnd)
{
    return wnd != NULL && SWindowMgr::GetWindow(swnd) == wnd;
}
} // namespace internal

// 按类名查找（遍历 GetWindow 兄弟/子树，用 IsClass 匹配）。
inline SWindow *findByClass(SWindow *root, LPCWSTR clsName)
{
    return internal::dfsFirst(root, internal::ClassNamePred(clsName));
}

// 按谓词查找（可见、可用、文本匹配等）。
template <typename Pred>
inline SWindow *findIf(SWindow *root, Pred pred)
{
    return internal::dfsFirst(root, pred);
}

// ===========================================================================
// 4. 行为验证层（对齐 QSignalSpy）
// ===========================================================================
// 基于 SubscribeEvent + 计数槽，订阅任意事件 ID；析构时自动取消订阅。
class SignalSpy
{
  public:
    SignalSpy(SWindow *wnd, DWORD eventId)
        : m_wnd(wnd)
        , m_swnd(wnd->GetSwnd())
        , m_id(eventId)
        , m_count(0)
        , m_last(NULL)
        , m_slot(Subscriber(&SignalSpy::onEvent, this))
    {
        m_wnd->SubscribeEvent(m_id, &m_slot);
    }

    ~SignalSpy()
    {
        // 宿主可能先于 Spy 被销毁（DestroyAllChildren / DestroyWindow 等），
        // 此时 m_wnd 是悬垂指针，必须用 SWND 判活后再触碰它。
        if (internal::isAlive(m_wnd, m_swnd))
            m_wnd->UnsubscribeEvent(m_id, &m_slot);
    }

    int count() const { return m_count; }

    /**
     * @brief 最近一次事件的参数对象指针。
     *
     * @warning 事件对象（EventCmd 等）是 FireEvent 调用方栈上的临时对象，FireEvent
     *          返回后即析构，因此 lastArgs() **只能在回调当次使用**；在 FireEvent
     *          返回后再解引用是悬垂访问。需要在派发结束后断言负载/发送者时，请改用
     *          下面的 DataSpy（按值保存负载）或 ArgsSpy（快照发送者与处理状态）。
     */
    const IEvtArgs *lastArgs() const { return m_last; }

  private:
    BOOL onEvent(IEvtArgs *e)
    {
        ++m_count;
        m_last = e;
        return TRUE;
    }

    SWindow *m_wnd;
    SWND m_swnd; ///< 宿主句柄快照，用于析构时判活（见 internal::isAlive）
    DWORD m_id;
    int m_count;
    const IEvtArgs *m_last;
    MemberFunctionSlot<SignalSpy, IEvtArgs> m_slot;
};

// ---------------------------------------------------------------------------
// 4b. 事件负载 / 发送者快照（补 SignalSpy 无法覆盖的两类断言）
// ---------------------------------------------------------------------------
// DataSpy<StEventXxx>：把每次事件的 Data() 负载按值存进 vector。
//   DEF_EVT 展开出的负载结构（StEventMouseClick / StEventSwndSize / StEventKeyDown ...）
//   都是 POD，可安全拷贝，因此能断言"事件序列"与"负载内容"。
//   用法：SouiTest::DataSpy<StEventMouseClick> spy(btn, EventMouseClick::EventID);
template <class Payload>
class DataSpy
{
  public:
    DataSpy(SWindow *wnd, DWORD eventId)
        : m_wnd(wnd)
        , m_swnd(wnd->GetSwnd())
        , m_id(eventId)
        , m_slot(Subscriber(&DataSpy::onEvent, this))
    {
        m_wnd->SubscribeEvent(m_id, &m_slot);
    }

    ~DataSpy()
    {
        if (internal::isAlive(m_wnd, m_swnd))
            m_wnd->UnsubscribeEvent(m_id, &m_slot);
    }

    int count() const { return (int)m_events.size(); }
    const std::vector<Payload> &events() const { return m_events; }
    const Payload &at(int i) const { return m_events[(size_t)i]; }
    void clear() { m_events.clear(); }

  private:
    BOOL onEvent(IEvtArgs *e)
    {
        if (e->Data())
            m_events.push_back(*(const Payload *)e->Data());
        return TRUE;
    }

    SWindow *m_wnd;
    SWND m_swnd; ///< 宿主句柄快照，用于析构时判活
    DWORD m_id;
    std::vector<Payload> m_events;
    MemberFunctionSlot<DataSpy, IEvtArgs> m_slot;
};

// 事件参数快照：在回调内取值，避免持有栈上事件对象。
struct EvtArgsSnapshot
{
    IObject *sender;
    int idFrom;
    SStringW nameFrom;
    UINT handled;
    BOOL bubbleUp;

    EvtArgsSnapshot()
        : sender(NULL)
        , idFrom(0)
        , handled(0)
        , bubbleUp(TRUE)
    {
    }
};

// ArgsSpy：记录 sender / idFrom / nameFrom / 处理计数 / 冒泡标志的快照。
// 用于断言"谁发出的、被处理了几次、此刻是否还会继续冒泡"。
class ArgsSpy
{
  public:
    ArgsSpy(SWindow *wnd, DWORD eventId)
        : m_wnd(wnd)
        , m_swnd(wnd->GetSwnd())
        , m_id(eventId)
        , m_count(0)
        , m_slot(Subscriber(&ArgsSpy::onEvent, this))
    {
        m_wnd->SubscribeEvent(m_id, &m_slot);
    }

    ~ArgsSpy()
    {
        if (internal::isAlive(m_wnd, m_swnd))
            m_wnd->UnsubscribeEvent(m_id, &m_slot);
    }

    int count() const { return m_count; }
    const EvtArgsSnapshot &last() const { return m_last; }

  private:
    BOOL onEvent(IEvtArgs *e)
    {
        ++m_count;
        m_last.sender = e->Sender();
        m_last.idFrom = e->IdFrom();
        m_last.nameFrom = e->NameFrom() ? e->NameFrom() : L"";
        m_last.handled = e->HandleCount();
        m_last.bubbleUp = e->IsBubbleUp();
        return TRUE;
    }

    SWindow *m_wnd;
    SWND m_swnd; ///< 宿主句柄快照，用于析构时判活
    DWORD m_id;
    int m_count;
    EvtArgsSnapshot m_last;
    MemberFunctionSlot<ArgsSpy, IEvtArgs> m_slot;
};

// ===========================================================================
// 5. 断言助手（对齐 QVERIFY / QCOMPARE + 属性检查）
// ===========================================================================
inline bool isVisible(SWindow *w) { return w->IsVisible(TRUE) != FALSE; }   // SWnd.h:529
inline bool isEnabled(SWindow *w) { return w->IsDisabled(TRUE) == FALSE; }  // SWnd.h:515
inline SStringT textOf(SWindow *w) { return w->GetWindowText(); }           // SWnd.h:1387
inline CRect rectOf(SWindow *w)
{
    CRect r;
    w->GetWindowRect(&r);
    return r;
}
inline bool isChecked(SWindow *w) { return (w->GetState() & WndState_Check) != 0; } // SWnd.h:137

// 宏糖：直接复用 gtest 的 EXPECT_*，命名上对齐 QVERIFY。
#define SOUITEST_EXPECT_VISIBLE(w) EXPECT_TRUE(SouiTest::isVisible(w))
#define SOUITEST_EXPECT_ENABLED(w)  EXPECT_TRUE(SouiTest::isEnabled(w))
#define SOUITEST_EXPECT_TEXT(w, t)  EXPECT_EQ(SouiTest::textOf(w), t)
#define SOUITEST_EXPECT_CHECKED(w)  EXPECT_TRUE(SouiTest::isChecked(w))

// ===========================================================================
// 6. 录制 / 回放（对齐 QTestEventList）—— Phase 1 数据驱动的基础
// ===========================================================================
struct InputEvent
{
    enum Type
    {
        Move,
        Down,
        Up,
        Key
    } t;
    CPoint pt;
    UINT vk;

    InputEvent(Type type, CPoint p, UINT v = 0)
        : t(type)
        , pt(p)
        , vk(v)
    {
    }
};

class EventList
{
  public:
    void addMouseClick(CPoint pt)
    {
        m_evts.push_back(InputEvent(InputEvent::Move, pt, 0));
        m_evts.push_back(InputEvent(InputEvent::Down, pt, 0));
        m_evts.push_back(InputEvent(InputEvent::Up, pt, 0));
    }
    void addMouseClick(SWindow *w) { addMouseClick(centerOf(w)); }

    void addKeyClick(UINT vk) { m_evts.push_back(InputEvent(InputEvent::Key, CPoint(0, 0), vk)); }

    // 回放复用 Input 层的 SSendMessage 分发，天然支持数据驱动。
    void replay(SWindow *target) const
    {
        for (size_t i = 0; i < m_evts.size(); ++i)
        {
            const InputEvent &e = m_evts[i];
            switch (e.t)
            {
            case InputEvent::Move:
                target->SSendMessage(WM_MOUSEMOVE, 0, MAKELPARAM(e.pt.x, e.pt.y));
                break;
            case InputEvent::Down:
                target->SSendMessage(WM_LBUTTONDOWN, MK_LBUTTON, MAKELPARAM(e.pt.x, e.pt.y));
                break;
            case InputEvent::Up:
                target->SSendMessage(WM_LBUTTONUP, 0, MAKELPARAM(e.pt.x, e.pt.y));
                break;
            case InputEvent::Key:
                target->SSendMessage(WM_KEYDOWN, e.vk, 0);
                target->SSendMessage(WM_KEYUP, e.vk, 0);
                target->SSendMessage(WM_CHAR, e.vk, 0);
                break;
            }
        }
    }

    size_t size() const { return m_evts.size(); }

  private:
    std::vector<InputEvent> m_evts;
};

} // namespace SouiTest

#endif // __SOUITEST_H__
