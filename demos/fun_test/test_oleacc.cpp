/**
 * MSAA (oleacc) tests — 双平台同一套用例：
 *  - Windows: 真实 oleacc.dll / user32（语义基准）；
 *  - 非 Windows: swinx 兼容实现（src/oleacc.cpp）。
 *
 * 用例统一原则：Windows 与 Linux/macOS 跑同一份测试代码、同一组断言。
 * 如果同一功能在两个平台需要不同断言，说明 swinx 的实现没有对齐 Windows
 * API 语义——此时应修 swinx，而不是给测试加平台分支。
 *
 * 平台消息路径说明：本套用例所有 WM_GETOBJECT / WinEvent 派发都发生在
 * 测试线程自己创建的窗口上——swinx 的 SendMessage 对本线程窗口是直接
 * WndProc 调用（wnd.cpp _SendMessageTimeout 的 same-thread 分支），不经过
 * HWND IPC（PostIpcMessage 只对 WndMgr 里查不到的跨进程窗口触发）。因此
 * macOS 未实现 HWND IPC（SendClientMessage 为空桩）不影响本套用例，
 * Linux/macOS/Windows 走的是同一条进程内路径。
 *
 * 仅有的两个 #ifndef _WIN32 用例断言的是 swinx 内部不变量（在 Win32 API 面
 * 上观察不到、但属于 swinx 胶水架构的设计约束）：
 *  - iid_self_resolution：swinx 的 DECLARE_CLASS_SIID 机制（COM 接口声明
 *    辅助，Windows SDK 头文件无此概念）。缺它时 QI 自身 IID 会静默失败。
 *  - weak_table_no_refcount：句柄表弱登记（swinx 全链路不持有 IAccessible，
 *    否则控件销毁后留下悬垂的活尸体）。Win32 的 oleacc 由内部存根持引用、
 *    计数不可观察，故此断言只对 swinx 有意义。
 *
 * Regression context（Windows 基线行为均经本机 oleacc 探针实测确认）：
 *  - LresultFromObject：每次调用产生新句柄；失败以 HRESULT 作 LRESULT 返回
 *    （null 对象 → E_INVALIDARG、不支持请求接口 → E_NOINTERFACE，并非文档
 *    描述的 0）。wParam 必须是 WM_GETOBJECT 原样收到的值，ObjectFromLresult
 *    按 (句柄, wParam) 匹配，不匹配 → E_FAIL（SHostWnd::OnGetObject 已按此
 *    实现，服务端自写 WndProc 时极易漏传）。
 *  - AccessibleChildren：child id 从 iChildStart+1 开始枚举，永不产生
 *    CHILDID_SELF；cChildren=0 → S_OK/obtained=0、cChildren<0 → S_OK 且不写
 *    出参、空参 → E_INVALIDARG。
 *  - GetRoleText/GetStateText：字符串本地化（真实 Windows 随 UI 语言），只做
 *    结构性断言（长度自洽、NULL 缓冲区返回所需长度、未知键返回 0 且清空）。
 *  - NotifyWinEvent：OUTOFCONTEXT 钩子回调经消息循环异步投递（两平台一致，
 *    swinx 曾是同步转发，已按 Windows 语义改为消息泵派发）；NotifyWinEvent
 *    返回时回调尚未执行。SKIPOWNPROCESS/SKIPOWNTHREAD 钩子注册成功但收不到
 *    本进程/本线程事件。
 *  - AccessibleObjectFromEvent：经 AccessibleObjectFromWindow（WM_GETOBJECT）
 *    解析对象，pvarChild 返回事件携带的 child id（两平台一致）。
 */
#include <gtest/gtest.h>
#include <windows.h>
#include <oleacc.h>
#include <string.h>
#include <wchar.h>
#include <stdio.h>
#include <stdint.h>

// C++11 feature detection: lambdas used by the message-pump helpers below are
// not available on compilers such as VS2008; the dependent tests are excluded
// there so the file still compiles.
#if defined(_MSC_VER) && _MSC_VER < 1700
#define SWINX_TEST_HAS_CPP11 0
#else
#define SWINX_TEST_HAS_CPP11 1
#endif

namespace
{

    /* COM 依赖的用例（LresultFromObject/ObjectFromLresult 内部走 COM 封送，
     * 线程必须先初始化 COM）。测试进程一次性初始化即可，不再 CoUninitialize
     * （进程退出时统一回收）。非 Windows 上 swinx 的 CoInitializeEx 是协议
     * 占位实现，调用无副作用。 */
    struct EnsureCom
    {
        EnsureCom() { CoInitializeEx(NULL, COINIT_APARTMENTTHREADED); }
        ~EnsureCom() { CoUninitialize(); }
    };
    static EnsureCom s_comInit;

    /* 最小 IAccessible 实现：只实现被测试用到的成员，其余返回 E_NOTIMPL。
     * 能编译通过本身就验证了 IAccessible/IDispatch 声明完整。 */
    class FakeAcc : public IAccessible
    {
      public:
        FakeAcc()
            : m_cRef(1)
            , m_name(SysAllocString(L"fake"))
            , m_childCount(2)
        {
        }

        // ---------- IUnknown ----------
        STDMETHODIMP_(ULONG) AddRef()
        {
            return ++m_cRef;
        }
        STDMETHODIMP_(ULONG) Release()
        {
            ULONG r = --m_cRef;
            if (r == 0)
                delete this;
            return r;
        }
        STDMETHODIMP QueryInterface(REFIID riid, void **ppv)
        {
            if (!ppv)
                return E_INVALIDARG;
            *ppv = NULL;
            if (IsEqualIID(riid, IID_IUnknown) || IsEqualIID(riid, IID_IDispatch) ||
                IsEqualIID(riid, IID_IAccessible))
            {
                *ppv = static_cast<IAccessible *>(this);
                AddRef();
                return S_OK;
            }
            return E_NOINTERFACE;
        }

        // ---------- IDispatch ----------
        STDMETHODIMP GetTypeInfoCount(UINT *pctinfo)
        {
            if (pctinfo)
                *pctinfo = 0;
            return S_OK;
        }
        STDMETHODIMP GetTypeInfo(UINT, LCID, ITypeInfo **)
        {
            return E_NOTIMPL;
        }
        STDMETHODIMP GetIDsOfNames(REFIID, LPOLESTR *, UINT, LCID, DISPID *)
        {
            return E_NOTIMPL;
        }
        STDMETHODIMP Invoke(DISPID, REFIID, LCID, WORD, DISPPARAMS *, VARIANT *, EXCEPINFO *, UINT *)
        {
            return E_NOTIMPL;
        }

        // ---------- IAccessible ----------
        STDMETHODIMP get_accParent(IDispatch **)
        {
            return E_NOTIMPL;
        }
        STDMETHODIMP get_accChildCount(long *pcountChildren)
        {
            if (!pcountChildren)
                return E_POINTER;
            *pcountChildren = m_childCount;
            return S_OK;
        }
        STDMETHODIMP get_accChild(VARIANT, IDispatch **)
        {
            return E_NOTIMPL; // 让 AccessibleChildren 回退为 VT_I4 子索引
        }
        STDMETHODIMP get_accName(VARIANT varChild, BSTR *pszName)
        {
            if (!pszName)
                return E_POINTER;
            *pszName = NULL;
            if (varChild.vt != VT_I4 || varChild.lVal != CHILDID_SELF)
                return E_INVALIDARG;
            *pszName = SysAllocString(m_name);
            return *pszName ? S_OK : E_OUTOFMEMORY;
        }
        STDMETHODIMP get_accValue(VARIANT, BSTR *)
        {
            return E_NOTIMPL;
        }
        STDMETHODIMP get_accDescription(VARIANT, BSTR *)
        {
            return E_NOTIMPL;
        }
        STDMETHODIMP get_accRole(VARIANT, VARIANT *)
        {
            return E_NOTIMPL;
        }
        STDMETHODIMP get_accState(VARIANT, VARIANT *)
        {
            return E_NOTIMPL;
        }
        STDMETHODIMP get_accHelp(VARIANT, BSTR *)
        {
            return E_NOTIMPL;
        }
        STDMETHODIMP get_accHelpTopic(BSTR *, VARIANT, long *)
        {
            return E_NOTIMPL;
        }
        STDMETHODIMP get_accKeyboardShortcut(VARIANT, BSTR *)
        {
            return E_NOTIMPL;
        }
        STDMETHODIMP get_accFocus(VARIANT *)
        {
            return E_NOTIMPL;
        }
        STDMETHODIMP get_accSelection(VARIANT *)
        {
            return E_NOTIMPL;
        }
        STDMETHODIMP get_accDefaultAction(VARIANT, BSTR *)
        {
            return E_NOTIMPL;
        }
        STDMETHODIMP accSelect(long, VARIANT)
        {
            return E_NOTIMPL;
        }
        STDMETHODIMP accLocation(long *, long *, long *, long *, VARIANT)
        {
            return E_NOTIMPL;
        }
        STDMETHODIMP accNavigate(long, VARIANT, VARIANT *)
        {
            return E_NOTIMPL;
        }
        STDMETHODIMP accHitTest(long, long, VARIANT *)
        {
            return E_NOTIMPL;
        }
        STDMETHODIMP accDoDefaultAction(VARIANT)
        {
            return E_NOTIMPL;
        }
        STDMETHODIMP put_accName(VARIANT, BSTR)
        {
            return E_NOTIMPL;
        }
        STDMETHODIMP put_accValue(VARIANT, BSTR)
        {
            return E_NOTIMPL;
        }

        long m_cRef;
        BSTR m_name;
        long m_childCount;

      protected:
        ~FakeAcc()
        {
            SysFreeString(m_name);
        }
    };

    /* 只支持 IUnknown 的对象：用于验证 riid 不被支持时返回 0 */
    class PlainUnk : public IUnknown
    {
      public:
        PlainUnk()
            : m_cRef(1)
        {
        }
        STDMETHODIMP_(ULONG) AddRef()
        {
            return ++m_cRef;
        }
        STDMETHODIMP_(ULONG) Release()
        {
            ULONG r = --m_cRef;
            if (r == 0)
                delete this;
            return r;
        }
        STDMETHODIMP QueryInterface(REFIID riid, void **ppv)
        {
            if (!ppv)
                return E_INVALIDARG;
            *ppv = NULL;
            if (IsEqualIID(riid, IID_IUnknown))
            {
                *ppv = this;
                AddRef();
                return S_OK;
            }
            return E_NOINTERFACE;
        }
        long m_cRef;

      protected:
        ~PlainUnk()
        {
        }
    };

    /* 同步钩子记录（未过滤来源，只用于无窗口参与的断言） */
    struct HookRecord
    {
        DWORD event;
        HWND hwnd;
        LONG idObject;
        LONG idChild;
        int calls;
    };
    static HookRecord g_hook;

    /* WinEvent 计数钩子：不做任何过滤（事件原样计数）。调用方必须把
     * idProcess 限定为本进程（GetCurrentProcessId）——真实 user32 上
     * idProcess=0 表示接收全桌面所有进程的事件，外部进程（explorer、
     * IME、任意应用）创建窗口触发的 EVENT_OBJECT_CREATE 会污染计数，
     * 使断言随机失败；swinx 是进程内实现天然无此噪声，两平台语义在
     * 限定本进程后一致（swinx 端同样支持 idProcess 过滤）。 */
    void CALLBACK TestEventHook(HWINEVENTHOOK hHook, DWORD event, HWND hwnd, LONG idObject,
                                LONG idChild, DWORD idEventThread, DWORD dwmsEventTime)
    {
        (void)hHook;
        (void)idEventThread;
        (void)dwmsEventTime;
        g_hook.event = event;
        g_hook.hwnd = hwnd;
        g_hook.idObject = idObject;
        g_hook.idChild = idChild;
        g_hook.calls++;
    }

    /* ---------------- 测试窗口（两平台共用） ----------------
     * 服务端模式与 SHostWnd::OnGetObject 相同：WM_GETOBJECT 应答
     * LresultFromObject(IID_IAccessible, wParam, pAcc)——wParam 必须
     * 原样回传（真实 oleacc 校验 (句柄, wParam) 匹配，传 0 会导致客户端
     * ObjectFromLresult 失败 E_FAIL，实测确认）。 */

    FakeAcc *g_e2eAcc = NULL;
    LONG g_lastGetObjectObjId = 0;

    LRESULT CALLBACK AccTestWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
    {
        if (msg == WM_GETOBJECT && g_e2eAcc)
        {
            g_lastGetObjectObjId = (LONG)lParam;
            if ((LONG)lParam == OBJID_CLIENT || (LONG)lParam == OBJID_WINDOW)
            {
                LRESULT lres = LresultFromObject(IID_IAccessible, wParam, g_e2eAcc);
                if (lres != 0)
                    return lres;
            }
        }
        return DefWindowProc(hwnd, msg, wParam, lParam);
    }

    HWND CreateAccTestWindow()
    {
        static const wchar_t kCls[] = L"swinx_acc_test_wnd";
        WNDCLASSEXW wc = { sizeof(wc) };
        HINSTANCE hInst = GetModuleHandleW(NULL);
        if (!GetClassInfoExW(hInst, kCls, &wc))
        {
            memset(&wc, 0, sizeof(wc));
            wc.cbSize = sizeof(wc);
            wc.lpfnWndProc = AccTestWndProc;
            wc.hInstance = hInst;
            wc.lpszClassName = kCls;
            if (!RegisterClassExW(&wc))
                return NULL;
        }
        return CreateWindowExW(0, kCls, L"acc-test", WS_POPUP, 0, 0, 100, 100, NULL, NULL, hInst,
                               NULL);
    }

    /* 异步事件钩子：只统计目标窗口的事件，避免同进程其它窗口干扰 */
    struct AsyncHookRecord
    {
        DWORD event;
        LONG idObject;
        LONG idChild;
        int calls;
    };
    static AsyncHookRecord g_asyncHook;
    static HWND g_asyncExpectHwnd = NULL;

    void CALLBACK AsyncEventHook(HWINEVENTHOOK hHook, DWORD event, HWND hwnd, LONG idObject,
                                 LONG idChild, DWORD idEventThread, DWORD dwmsEventTime)
    {
        (void)hHook;
        (void)idEventThread;
        (void)dwmsEventTime;
        if (g_asyncExpectHwnd && hwnd != g_asyncExpectHwnd)
            return;
        g_asyncHook.event = event;
        g_asyncHook.idObject = idObject;
        g_asyncHook.idChild = idChild;
        g_asyncHook.calls++;
    }

    /* 泵消息直到条件成立或超时。OUTOFCONTEXT 钩子的回调经消息循环投递
     * （两平台一致：真实 user32 与 swinx 都在消息泵里派发 WinEvent）。 */
    template <typename Pred>
    bool PumpUntil(DWORD timeoutMs, Pred pred)
    {
        ULONGLONG deadline = GetTickCount64() + timeoutMs;
        for (;;)
        {
            if (pred())
                return true;
            if (GetTickCount64() >= deadline)
                return false;
            MSG msg;
            while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
            {
                TranslateMessage(&msg);
                DispatchMessage(&msg);
                if (pred())
                    return true;
            }
            Sleep(10);
        }
    }

    /* 查询 (hwnd, OBJID_CLIENT) 的对象并做 FakeAcc 功能断言 */
    void ExpectClientAccIsFake(HWND hwnd)
    {
        IAccessible *acc = NULL;
        ASSERT_EQ(AccessibleObjectFromWindow(hwnd, OBJID_CLIENT, IID_IAccessible, (void **)&acc),
                  S_OK);
        ASSERT_TRUE(acc != NULL);

        VARIANT v;
        VariantInit(&v);
        v.vt = VT_I4;
        v.lVal = CHILDID_SELF;
        BSTR name = NULL;
        EXPECT_EQ(acc->get_accName(v, &name), S_OK);
        EXPECT_STREQ(name, L"fake");
        SysFreeString(name);
        VariantClear(&v);

        long cnt = -1;
        EXPECT_EQ(acc->get_accChildCount(&cnt), S_OK);
        EXPECT_EQ(cnt, 2);
        acc->Release();
    }

} // namespace

// ------------------------------------------------------------------------
// [swinx 内部不变量] 接口 IID 解析（DECLARE_CLASS_SIID 回归）
// ------------------------------------------------------------------------

#ifndef _WIN32
TEST(swinx_oleacc, iid_self_resolution)
{
    // 缺少 DECLARE_CLASS_SIID 时，GetIID() 会解析到基类（IUnknown/IDispatch），
    // QI 自身 IID 将永远失败。
    EXPECT_TRUE(IsEqualIID(IAccessible::GetIID(), IID_IAccessible));
    EXPECT_TRUE(IsEqualIID(IEnumVARIANT::GetIID(), IID_IEnumVARIANT));
    EXPECT_FALSE(IsEqualIID(IAccessible::GetIID(), IID_IUnknown));
}
#endif //_WIN32

// ------------------------------------------------------------------------
// LresultFromObject / ObjectFromLresult（两平台同一语义）
// ------------------------------------------------------------------------

TEST(swinx_oleacc, lresult_roundtrip_iaccessible)
{
    FakeAcc *pAcc = new FakeAcc();
    LRESULT h = LresultFromObject(IID_IAccessible, 0, pAcc);
    ASSERT_NE(h, 0);

    // 往返解析：拿回的接口功能等价（不假设指针同一性——Windows 经 COM
    // 解析，同一 STA 通常即原指针，但这是实现细节不是 API 契约）
    void *pv = NULL;
    ASSERT_EQ(ObjectFromLresult(h, IID_IAccessible, 0, &pv), S_OK);
    ASSERT_TRUE(pv != NULL);
    IAccessible *acc = static_cast<IAccessible *>(pv);

    VARIANT v;
    VariantInit(&v);
    v.vt = VT_I4;
    v.lVal = CHILDID_SELF;
    BSTR name = NULL;
    EXPECT_EQ(acc->get_accName(v, &name), S_OK);
    EXPECT_STREQ(name, L"fake");
    SysFreeString(name);
    VariantClear(&v);
    acc->Release();
    pAcc->Release();
}

TEST(swinx_oleacc, lresult_new_handle_per_call)
{
    // 与 Win32 一致：同一对象每次 LresultFromObject 产生独立句柄，且每个
    // 句柄都能独立解析（Windows oleacc 内部按调用生成存根句柄；swinx 曾
    // 错误地按对象复用句柄，已对齐）。
    FakeAcc *pAcc = new FakeAcc();
    LRESULT h1 = LresultFromObject(IID_IAccessible, 0, pAcc);
    LRESULT h2 = LresultFromObject(IID_IAccessible, 0, pAcc);
    EXPECT_NE(h1, 0);
    EXPECT_NE(h2, 0);
    EXPECT_NE(h1, h2);

    void *pv1 = NULL, *pv2 = NULL;
    EXPECT_EQ(ObjectFromLresult(h1, IID_IAccessible, 0, &pv1), S_OK);
    EXPECT_EQ(ObjectFromLresult(h2, IID_IAccessible, 0, &pv2), S_OK);
    if (pv1)
        static_cast<IUnknown *>(pv1)->Release();
    if (pv2)
        static_cast<IUnknown *>(pv2)->Release();
    pAcc->Release();
}

TEST(swinx_oleacc, lresult_null_object_returns_einvalidarg)
{
    // 实测真实 oleacc：null 对象返回 E_INVALIDARG（作为 LRESULT），
    // 而非文档描述的 0。
    EXPECT_EQ(LresultFromObject(IID_IAccessible, 0, NULL), (LRESULT)E_INVALIDARG);
}

TEST(swinx_oleacc, lresult_unsupported_iid_returns_enointerface)
{
    PlainUnk *pUnk = new PlainUnk();
    // 实测真实 oleacc：对象不支持请求的接口时返回 E_NOINTERFACE（作为
    // LRESULT），而非文档描述的 0。
    EXPECT_EQ(LresultFromObject(IID_IAccessible, 0, pUnk), (LRESULT)E_NOINTERFACE);
    // 但 IUnknown 可以
    LRESULT h = LresultFromObject(IID_IUnknown, 0, pUnk);
    EXPECT_NE(h, 0);
    pUnk->Release(); // swinx 弱登记不续命（见 weak_table_no_refcount）
}

TEST(swinx_oleacc, object_from_lresult_invalid_args)
{
    // 两平台一致的失败语义：非法 lparam / 未知句柄 / 空出参 / wParam 不匹配
    // 都失败。（精确错误码除实测确认的 E_FAIL 外不假设。）
    void *pv = (void *)1;
    EXPECT_NE(ObjectFromLresult(0, IID_IAccessible, 0, &pv), S_OK);
    EXPECT_NE(ObjectFromLresult(0x7ffffff0, IID_IAccessible, 0, &pv), S_OK);
    EXPECT_NE(ObjectFromLresult(1, IID_IAccessible, 0, NULL), S_OK);

    // wParam 必须与 LresultFromObject 登记时一致（服务端原样回传 WM_GETOBJECT
    // 的 wParam；实测真实 oleacc 不匹配时 ObjectFromLresult 返回 E_FAIL）。
    // 注：wp=0 的句柄在真实 oleacc 上可脱离 WM_GETOBJECT 往返独立解析（持久），
    // wp!=0 的句柄则与应答往返绑定——两平台统一按 wp=0 组合断言。
    FakeAcc *pAcc = new FakeAcc();
    LRESULT h = LresultFromObject(IID_IAccessible, 0, pAcc);
    ASSERT_NE(h, 0);
    EXPECT_EQ(ObjectFromLresult(h, IID_IAccessible, 0, &pv), S_OK);
    if (pv)
        static_cast<IUnknown *>(pv)->Release();
    pv = (void *)1;
    EXPECT_EQ(ObjectFromLresult(h, IID_IAccessible, 4321, &pv), E_FAIL);
    pAcc->Release();
}

// ------------------------------------------------------------------------
// AccessibleObjectFromWindow / FromEvent（两平台同一语义）
// ------------------------------------------------------------------------

TEST(swinx_oleacc, accessible_object_from_window_invalid_args)
{
    void *pv = (void *)1;
    EXPECT_NE(AccessibleObjectFromWindow(NULL, OBJID_CLIENT, IID_IAccessible, &pv), S_OK);
    EXPECT_NE(AccessibleObjectFromWindow(NULL, OBJID_CLIENT, IID_IAccessible, NULL), S_OK);
}

/* 端到端：MSAA 客户端/服务端闭环（两平台同一套）。
 * 服务端：测试窗口在 WM_GETOBJECT 里返回 LresultFromObject(...)（与
 * SHostWnd::OnGetObject 相同的模式）；客户端：AccessibleObjectFromWindow
 * 经 SendMessage(WM_GETOBJECT) + ObjectFromLresult 拿回接口。 */
TEST(swinx_oleacc, accessible_object_from_window_end_to_end)
{
    HWND hwnd = CreateAccTestWindow();
    if (!hwnd)
    {
        SUCCEED() << "window creation unavailable (headless environment)";
        return;
    }

    g_e2eAcc = new FakeAcc(); // ref = 1
    g_lastGetObjectObjId = 0;
    // OBJID_CLIENT：WM_GETOBJECT 确实到达窗口，OBJID 经 lParam 传递。
    // 回归：OBJID_* 常量在 LP64 上必须保持 Windows 的 32 位有符号值
    // （服务端 "(LONG)lParam == OBJID_CLIENT" 依赖这一点）——此前
    // 0xFFFFFFFCL 在 Linux 上是 64 位正数，分支永不命中、WM_GETOBJECT
    // 应答 0，本用例的 OBJID_CLIENT 查询随即 E_FAIL。
    ExpectClientAccIsFake(hwnd);
    EXPECT_EQ(g_lastGetObjectObjId, (LONG)OBJID_CLIENT);

    // OBJID_WINDOW 同样可达
    IAccessible *accWin = NULL;
    g_lastGetObjectObjId = 0;
    EXPECT_EQ(AccessibleObjectFromWindow(hwnd, OBJID_WINDOW, IID_IAccessible, (void **)&accWin),
              S_OK);
    EXPECT_EQ(g_lastGetObjectObjId, (LONG)OBJID_WINDOW);
    if (accWin)
        accWin->Release();

    g_e2eAcc->Release(); // oleacc 内部存根可能仍持引用，不断言归零
    g_e2eAcc = NULL;
    DestroyWindow(hwnd);
}

TEST(swinx_oleacc, accessible_object_from_event)
{
    // 与 Win32 同路径：经 AccessibleObjectFromWindow 解析 (hwnd, idObject)
    // 处的对象，pvarChild 返回事件携带的 child id。
    IAccessible *pAcc = NULL;
    VARIANT var;
    VariantInit(&var);

    // hwnd 为 NULL：失败（真实 oleacc 返回 E_INVALIDARG）
    EXPECT_NE(AccessibleObjectFromEvent(NULL, OBJID_CLIENT, CHILDID_SELF, &pAcc, &var), S_OK);
    VariantClear(&var);

    HWND hwnd = CreateAccTestWindow();
    if (!hwnd)
    {
        SUCCEED() << "window creation unavailable (headless environment)";
        return;
    }

    g_e2eAcc = new FakeAcc();

    // CHILDID_SELF：对象即目标，pvarChild 回带 CHILDID_SELF
    pAcc = NULL;
    VariantInit(&var);
    EXPECT_EQ(AccessibleObjectFromEvent(hwnd, OBJID_CLIENT, CHILDID_SELF, &pAcc, &var), S_OK);
    ASSERT_TRUE(pAcc != NULL);
    EXPECT_EQ(var.vt, VT_I4);
    EXPECT_EQ(var.lVal, (LONG)CHILDID_SELF);
    pAcc->Release();

    // 非 SELF 的 child id：对象为父级，pvarChild 回带原 child id
    pAcc = NULL;
    VariantInit(&var);
    EXPECT_EQ(AccessibleObjectFromEvent(hwnd, OBJID_CLIENT, 5, &pAcc, &var), S_OK);
    ASSERT_TRUE(pAcc != NULL);
    EXPECT_EQ(var.vt, VT_I4);
    EXPECT_EQ(var.lVal, 5);
    pAcc->Release();

    VariantClear(&var);
    g_e2eAcc->Release();
    g_e2eAcc = NULL;
    DestroyWindow(hwnd);
}

// ------------------------------------------------------------------------
// AccessibleChildren（两平台同一语义）
// ------------------------------------------------------------------------

TEST(swinx_oleacc, accessible_children_enumerates)
{
    FakeAcc *pAcc = new FakeAcc(); // get_accChildCount = 2

    VARIANT vars[5];
    for (int i = 0; i < 5; i++)
        VariantInit(&vars[i]);
    long obtained = -1;

    // Win32 语义：child id 从 iChildStart+1 开始，永不产生 CHILDID_SELF
    HRESULT hr = AccessibleChildren(pAcc, 0, 2, vars, &obtained);
    EXPECT_EQ(hr, S_OK);
    EXPECT_EQ(obtained, 2);
    EXPECT_EQ(vars[0].vt, VT_I4);
    EXPECT_EQ(vars[0].lVal, 1);
    EXPECT_EQ(vars[1].vt, VT_I4);
    EXPECT_EQ(vars[1].lVal, 2);
    for (int i = 0; i < 2; i++)
        VariantClear(&vars[i]);

    // 请求数超过子对象数：部分返回 -> S_FALSE
    obtained = -1;
    hr = AccessibleChildren(pAcc, 0, 3, vars, &obtained);
    EXPECT_EQ(hr, S_FALSE);
    EXPECT_EQ(obtained, 2);
    for (int i = 0; i < 2; i++)
        VariantClear(&vars[i]);

    // 起始偏移：从 child id 2 开始只剩 1 个
    obtained = -1;
    hr = AccessibleChildren(pAcc, 1, 2, vars, &obtained);
    EXPECT_EQ(hr, S_FALSE);
    EXPECT_EQ(obtained, 1);
    EXPECT_EQ(vars[0].vt, VT_I4);
    EXPECT_EQ(vars[0].lVal, 2);
    VariantClear(&vars[0]);

    // 起始偏移越界：0 个
    obtained = -1;
    hr = AccessibleChildren(pAcc, 2, 1, vars, &obtained);
    EXPECT_EQ(hr, S_FALSE);
    EXPECT_EQ(obtained, 0);

    // 参数校验（实测真实 oleacc 语义）
    // cChildren == 0：S_OK 且 obtained = 0（不是 E_INVALIDARG）
    obtained = -1;
    EXPECT_EQ(AccessibleChildren(pAcc, 0, 0, vars, &obtained), S_OK);
    EXPECT_EQ(obtained, 0);
    // cChildren < 0：S_OK 且不写出参（真实 oleacc 保持调用方原值）
    obtained = -1;
    EXPECT_EQ(AccessibleChildren(pAcc, 0, -1, vars, &obtained), S_OK);
    EXPECT_EQ(obtained, -1);
    // 空容器指针：E_INVALIDARG
    EXPECT_EQ(AccessibleChildren(NULL, 0, 1, vars, &obtained), E_INVALIDARG);

    pAcc->Release();
}

// ------------------------------------------------------------------------
// GetRoleText / GetStateText（两平台同一语义；字符串本地化不做内容断言）
// ------------------------------------------------------------------------

TEST(swinx_oleacc, get_role_text)
{
    // 真实 Windows 按用户 UI 语言返回本地化文本（中文系统返回"按下按钮"），
    // swinx 固定英文——两者都是"该角色的本地化名称"，故只断言结构性质：
    // 返回长度与缓冲区内容一致、NULL 缓冲区返回所需长度、未知角色返回 0。
    WCHAR buf[64] = { 0 };
    UINT n = GetRoleTextW(ROLE_SYSTEM_PUSHBUTTON, buf, 64);
    EXPECT_GT(n, 0u);
    EXPECT_EQ(n, (UINT)wcslen(buf));

    // 缓冲区为 NULL 时返回所需长度（不含结尾 NUL），与带缓冲区调用一致
    EXPECT_EQ(GetRoleTextW(ROLE_SYSTEM_PUSHBUTTON, NULL, 0), n);

    // 未知角色：返回 0 且缓冲区置空
    wcscpy(buf, L"dirty");
    EXPECT_EQ(GetRoleTextW(0x9999, buf, 64), 0u);
    EXPECT_STREQ(buf, L"");

    char abuf[64] = { 0 };
    UINT na = GetRoleTextA(ROLE_SYSTEM_CHECKBUTTON, abuf, 64);
    EXPECT_GT(na, 0u);
    EXPECT_EQ(na, (UINT)strlen(abuf));
}

TEST(swinx_oleacc, get_state_text)
{
    WCHAR buf[64] = { 0 };
    UINT n = GetStateTextW(STATE_SYSTEM_FOCUSED, buf, 64);
    EXPECT_GT(n, 0u);
    EXPECT_EQ(n, (UINT)wcslen(buf));
    EXPECT_EQ(GetStateTextW(STATE_SYSTEM_FOCUSED, NULL, 0), n);

    n = GetStateTextW(STATE_SYSTEM_CHECKED, buf, 64);
    EXPECT_GT(n, 0u);
    EXPECT_EQ(n, (UINT)wcslen(buf));

    EXPECT_EQ(GetStateTextW(0x80000000, buf, 64), 0u); // 非法状态位
    EXPECT_STREQ(buf, L"");
}

// ------------------------------------------------------------------------
// NotifyWinEvent + SetWinEventHook（两平台同一语义）
// ------------------------------------------------------------------------

// 这些用例依赖消息泵/谓词 lambda（C++11），非 C++11 编译器上屏蔽。
#if SWINX_TEST_HAS_CPP11

TEST(swinx_oleacc, notify_win_event_delivered_via_message_loop)
{
    // OUTOFCONTEXT 钩子的回调经消息循环异步投递（user32 语义）：
    // NotifyWinEvent 返回时回调尚未执行。
    HWND hwnd = CreateAccTestWindow();
    if (!hwnd)
    {
        SUCCEED() << "window creation unavailable (headless environment)";
        return;
    }

    memset(&g_asyncHook, 0, sizeof(g_asyncHook));
    g_asyncExpectHwnd = hwnd;

    HWINEVENTHOOK hHook = SetWinEventHook(EVENT_OBJECT_CREATE, EVENT_OBJECT_CREATE, NULL,
                                          AsyncEventHook, 0, 0, WINEVENT_OUTOFCONTEXT);
    ASSERT_NE(hHook, (HWINEVENTHOOK)NULL);

    NotifyWinEvent(EVENT_OBJECT_CREATE, hwnd, OBJID_CLIENT, CHILDID_SELF);
    EXPECT_EQ(g_asyncHook.calls, 0); // 当前线程忙于本调用，回调尚未投递

    ASSERT_TRUE(PumpUntil(2000, []() { return g_asyncHook.calls > 0; }));
    EXPECT_EQ(g_asyncHook.event, (DWORD)EVENT_OBJECT_CREATE);
    EXPECT_EQ(g_asyncHook.idObject, (LONG)OBJID_CLIENT);
    EXPECT_EQ(g_asyncHook.idChild, (LONG)CHILDID_SELF);

    // 区间外的事件不投递
    NotifyWinEvent(EVENT_OBJECT_DESTROY, hwnd, OBJID_CLIENT, CHILDID_SELF);
    PumpUntil(300, []() { return false; }); // 纯泵 300ms
    EXPECT_EQ(g_asyncHook.calls, 1);

    EXPECT_TRUE(UnhookWinEvent(hHook));

    // 注销后新事件不再投递（已入队的事件不受影响，这里发的是新事件）
    int calls = g_asyncHook.calls;
    NotifyWinEvent(EVENT_OBJECT_CREATE, hwnd, OBJID_CLIENT, CHILDID_SELF);
    PumpUntil(300, []() { return false; }); // 纯泵 300ms
    EXPECT_EQ(g_asyncHook.calls, calls);

    DestroyWindow(hwnd);
    g_asyncExpectHwnd = NULL;
}

TEST(swinx_oleacc, notify_win_event_unhooked_is_safe)
{
    // 未注册钩子时 NotifyWinEvent 是安全的空操作
    memset(&g_hook, 0, sizeof(g_hook));
    NotifyWinEvent(EVENT_OBJECT_CREATE, NULL, OBJID_CLIENT, CHILDID_SELF);
    EXPECT_EQ(g_hook.calls, 0);

    // 与 user32 一致：钩子注册之前发出的事件不能滞留到之后——注册钩子并
    // 泵消息，上面那条事件不得再投递过来（swinx 曾把它滞留队列，导致事件
    // 泄漏进下一个注册者的计数）。
    HWND hwnd = CreateAccTestWindow();
    if (hwnd)
    {
        HWINEVENTHOOK hHook = SetWinEventHook(EVENT_OBJECT_CREATE, EVENT_OBJECT_CREATE, NULL,
                                              TestEventHook, GetCurrentProcessId(), 0,
                                              WINEVENT_OUTOFCONTEXT);
        ASSERT_NE(hHook, (HWINEVENTHOOK)NULL);
        PumpUntil(300, []() { return false; }); // 纯泵，给残留事件投递机会
        EXPECT_EQ(g_hook.calls, 0);
        EXPECT_TRUE(UnhookWinEvent(hHook));
        DestroyWindow(hwnd);
    }
}

TEST(swinx_oleacc, notify_win_event_multiple_hooks_and_ranges)
{
    // 多个并发钩子各自按 [eventMin, eventMax] 收到匹配事件
    HWND hwnd = CreateAccTestWindow();
    if (!hwnd)
    {
        SUCCEED() << "window creation unavailable (headless environment)";
        return;
    }

    memset(&g_hook, 0, sizeof(g_hook));
    g_hook.hwnd = hwnd;

    HWINEVENTHOOK h1 = SetWinEventHook(EVENT_OBJECT_CREATE, EVENT_OBJECT_CREATE, NULL,
                                       TestEventHook, GetCurrentProcessId(), 0,
                                       WINEVENT_OUTOFCONTEXT);
    HWINEVENTHOOK h2 = SetWinEventHook(EVENT_OBJECT_SHOW, EVENT_OBJECT_HIDE, NULL, TestEventHook,
                                       GetCurrentProcessId(), 0, WINEVENT_OUTOFCONTEXT);
    ASSERT_NE(h1, (HWINEVENTHOOK)NULL);
    ASSERT_NE(h2, (HWINEVENTHOOK)NULL);

    NotifyWinEvent(EVENT_OBJECT_CREATE, hwnd, OBJID_CLIENT, CHILDID_SELF);
    NotifyWinEvent(EVENT_OBJECT_SHOW, hwnd, OBJID_CLIENT, CHILDID_SELF);
    NotifyWinEvent(EVENT_OBJECT_DESTROY, hwnd, OBJID_CLIENT, CHILDID_SELF); // 都不匹配
    ASSERT_TRUE(PumpUntil(2000, []() { return g_hook.calls >= 2; }));
    EXPECT_EQ(g_hook.calls, 2); // DESTROY 落在两个钩子的区间之外

    EXPECT_TRUE(UnhookWinEvent(h1));
    EXPECT_TRUE(UnhookWinEvent(h2));
    DestroyWindow(hwnd);
}

TEST(swinx_oleacc, skipownprocess_and_skipownthread_hooks)
{
    // 与 Win32 一致：SKIP 标志是过滤条件而非注册错误——钩子注册成功
    // （返回有效句柄），只是收不到本进程（SKIPOWNPROCESS）/注册线程
    // （SKIPOWNTHREAD）的事件。swinx 的事件全部来自本进程，SKIP 钩子
    // 永远收不到回调。（swinx 曾错误地注册即拒绝返回 NULL，已对齐。）
    HWND hwnd = CreateAccTestWindow();
    if (!hwnd)
    {
        SUCCEED() << "window creation unavailable (headless environment)";
        return;
    }

    memset(&g_asyncHook, 0, sizeof(g_asyncHook));
    g_asyncExpectHwnd = hwnd;

    HWINEVENTHOOK hSkipProc = SetWinEventHook(EVENT_MIN, EVENT_MAX, NULL, AsyncEventHook, 0, 0,
                                              WINEVENT_SKIPOWNPROCESS);
    EXPECT_NE(hSkipProc, (HWINEVENTHOOK)NULL);

    HWINEVENTHOOK hSkipThread = SetWinEventHook(EVENT_MIN, EVENT_MAX, NULL, AsyncEventHook, 0, 0,
                                                WINEVENT_SKIPOWNTHREAD);
    EXPECT_NE(hSkipThread, (HWINEVENTHOOK)NULL);

    NotifyWinEvent(EVENT_OBJECT_CREATE, hwnd, OBJID_CLIENT, CHILDID_SELF);
    PumpUntil(300, []() { return false; }); // 纯泵 300ms
    EXPECT_EQ(g_asyncHook.calls, 0);        // 两个 SKIP 钩子都收不到本线程事件

    if (hSkipProc)
        EXPECT_TRUE(UnhookWinEvent(hSkipProc));
    if (hSkipThread)
        EXPECT_TRUE(UnhookWinEvent(hSkipThread));
    DestroyWindow(hwnd);
    g_asyncExpectHwnd = NULL;
}

#endif // SWINX_TEST_HAS_CPP11

TEST(swinx_oleacc, set_winevent_hook_invalid_args)
{
    // 与 Win32 一致：非法参数返回 NULL
    EXPECT_EQ(SetWinEventHook(EVENT_MIN, EVENT_MAX, NULL, NULL, 0, 0, WINEVENT_OUTOFCONTEXT),
              (HWINEVENTHOOK)NULL);
    EXPECT_EQ(SetWinEventHook(EVENT_OBJECT_SHOW, EVENT_OBJECT_CREATE, NULL, TestEventHook, 0, 0,
                              WINEVENT_OUTOFCONTEXT),
              (HWINEVENTHOOK)NULL); // eventMin > eventMax
}

TEST(swinx_oleacc, unhook_win_event_twice_returns_false)
{
    HWINEVENTHOOK hHook = SetWinEventHook(EVENT_OBJECT_CREATE, EVENT_OBJECT_CREATE, NULL,
                                          TestEventHook, 0, 0, WINEVENT_OUTOFCONTEXT);
    ASSERT_NE(hHook, (HWINEVENTHOOK)NULL);
    EXPECT_TRUE(UnhookWinEvent(hHook));
    EXPECT_FALSE(UnhookWinEvent(hHook)); // 重复注销返回 FALSE
    EXPECT_FALSE(UnhookWinEvent(NULL));  // 空句柄返回 FALSE
}

// ------------------------------------------------------------------------
// [swinx 内部不变量] 弱登记句柄表：swinx 全链路不持有 IAccessible
// ------------------------------------------------------------------------

#ifndef _WIN32
TEST(swinx_oleacc, weak_table_no_refcount)
{
    // 胶水架构约束：swinx 的句柄表只做弱登记（不 AddRef）。若 swinx 持有
    // 对象引用，最后一次 Release 会返回 1（表续命），控件销毁后将留下
    // 内部指针已悬垂的活尸体——这是回归。
    FakeAcc *pAcc = new FakeAcc();
    LRESULT h = LresultFromObject(IID_IAccessible, 0, pAcc);
    EXPECT_NE(h, 0);

    void *pv = NULL;
    EXPECT_EQ(ObjectFromLresult(h, IID_IAccessible, 0, &pv), S_OK);
    if (pv)
        static_cast<IUnknown *>(pv)->Release();

    // 弱登记：对象由最后一次 Release 销毁（返回 0）
    EXPECT_EQ(pAcc->Release(), 0);
}
#endif //_WIN32
