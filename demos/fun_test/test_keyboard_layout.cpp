/**
 * Keyboard layout API tests: GetKeyboardLayout / GetKeyboardLayoutList /
 * ActivateKeyboardLayout.
 *
 * 以 Windows 平台的真实 Win32 行为为基线（baseline），不针对任何具体平台写
 * 分支：同样的断言既要在 Windows 上针对原生 user32 通过，也要在其它平台（swinx
 * 的 SConnection 实现）上通过。swinx 把键盘布局抽象成 HKL：
 *   - Windows  : 真实 HKL 句柄，ActivateKeyboardLayout 真正切换输入法；
 *   - Linux    : XKB group 索引（经 SKeyboard 真正切换）；
 *   - macOS    : TIS 输入源（经 TISSelectInputSource 真正切换）；
 *   - iOS/OHOS : 应用层无法切换系统布局，退化为“保存 + 循环”（OS 限制）。
 * 因此断言只描述平台无关的契约（列表可枚举、激活后 GetKeyboardLayout 反映所激活
 * 的布局、HKL_NEXT/HKL_PREV 可调用且返回上一布局），不断言“必须成功切换”。
 *
 * 注意：Windows 上 ActivateKeyboardLayout 会真正改变系统输入语言，测试夹具在
 * TearDown 中把布局还原为初始值，避免遗留副作用。
 */
#include <gtest/gtest.h>
#include <windows.h>
#include <vector>

// HKL_NEXT / HKL_PREV 是 Win32 标准魔法值（winuser.h 定义为 1 / 2）；swinx 的
// windows.h 未提供，这里在无定义时补齐，避免在非 Windows 平台编译失败。这不是
// 平台分支，只是补齐标准常量。
// 注意：Win32 下 HKL 是指针类型，直接把整数传给 ActivateKeyboardLayout 会触发
// int->指针 的无效转换，因此调用处统一用 (HKL)(size_t) 显式转换；该写法对
// “指针型 HKL”（Win32）与“整数型 HKL”（swinx，HKL==DWORD）都安全。
#ifndef HKL_NEXT
#define HKL_NEXT 1
#endif
#ifndef HKL_PREV
#define HKL_PREV 2
#endif

class KeyboardLayoutTest : public testing::Test
{
protected:
    HKL m_saved = NULL;

    void SetUp() override
    {
        // 记录当前线程布局，供 TearDown 还原（Windows 真实切换的副作用收敛）。
        m_saved = GetKeyboardLayout(0);
    }

    void TearDown() override
    {
        if (m_saved != NULL)
            ActivateKeyboardLayout(m_saved, 0);
    }
};

// ---------------------------------------------------------------------------
// 1) 列表可枚举：返回的布局数 >= 1，且 nBuff=0 查询与完整查询一致。
//    Windows 恒 >=1；swinx Linux（XKB）>=1；iOS/OHOS 恒为 1（仅默认布局）。
// ---------------------------------------------------------------------------
TEST_F(KeyboardLayoutTest, ListReturnsAtLeastOne)
{
    UINT n = GetKeyboardLayoutList(0, nullptr);
    EXPECT_GE(n, 1u);
}

TEST_F(KeyboardLayoutTest, ListCountIsStable)
{
    UINT n = GetKeyboardLayoutList(0, nullptr);
    ASSERT_GE(n, 1u);

    // nBuff=0 时 lpList 可为 NULL，返回的是总布局数，不应写入。
    std::vector<HKL> buf(n);
    UINT got = GetKeyboardLayoutList((int)n, buf.data());
    EXPECT_EQ(got, n);
}

// ---------------------------------------------------------------------------
// 2) 激活↔查询闭环：列表里每一个 HKL，激活后 GetKeyboardLayout(0) 必须等于它。
//    该契约在 Windows 与各 swinx 平台都成立（SConnection::GetKeyboardLayout
//    返回的是刚被 ActivateKeyboardLayout 设置的抽象 HKL / XKB group）。
// ---------------------------------------------------------------------------
TEST_F(KeyboardLayoutTest, ActivateThenGetRoundTrip)
{
    UINT n = GetKeyboardLayoutList(0, nullptr);
    ASSERT_GE(n, 1u);

    std::vector<HKL> layouts(n);
    ASSERT_EQ(GetKeyboardLayoutList((int)n, layouts.data()), n);

    for (UINT i = 0; i < n; ++i)
    {
        HKL prev = ActivateKeyboardLayout(layouts[i], 0);
        // 激活后当前布局应当等于刚刚激活的那个。
        EXPECT_EQ(GetKeyboardLayout(0), layouts[i]) << "layout index " << i;
        (void)prev;
    }
}

// ---------------------------------------------------------------------------
// 3) 重复激活同一布局是幂等的：第二次激活返回的上一个布局就是它自己。
// ---------------------------------------------------------------------------
TEST_F(KeyboardLayoutTest, ReactivateIsIdempotent)
{
    UINT n = GetKeyboardLayoutList(0, nullptr);
    ASSERT_GE(n, 1u);

    std::vector<HKL> layouts(n);
    ASSERT_EQ(GetKeyboardLayoutList((int)n, layouts.data()), n);

    HKL L = layouts[0];
    ActivateKeyboardLayout(L, 0);
    HKL prev = ActivateKeyboardLayout(L, 0);
    EXPECT_EQ(prev, L);
    EXPECT_EQ(GetKeyboardLayout(0), L);
}

// ---------------------------------------------------------------------------
// 4) HKL_NEXT / HKL_PREV 可调用且返回“上一个布局”（平台无关的烟雾测试）。
//    不断言具体切换结果：单布局系统上 NEXT/PREV 可能原地不动；iOS/OHOS 退化为
//    保存语义。只要不崩溃、返回合法 HKL 即可。
// ---------------------------------------------------------------------------
TEST_F(KeyboardLayoutTest, NextPrevAreCallable)
{
    HKL prevNext = ActivateKeyboardLayout((HKL)(size_t)HKL_NEXT, 0);
    HKL prevPrev = ActivateKeyboardLayout((HKL)(size_t)HKL_PREV, 0);
    (void)prevNext;
    (void)prevPrev;
    // 调用后仍可查询当前布局（返回合法 HKL，含 0 表示默认布局）。
    HKL cur = GetKeyboardLayout(0);
    (void)cur;
}

// 在有 >=2 个布局的系统上，NEXT 再 PREV 应回到 NEXT 之前的布局（真实循环）。
// 单布局系统上两者都原地不动，同样回到起点——故该断言在所有平台上都成立，
// 仅在布局数 <2 时不强制（由运行时分支判断，不是平台分支）。
TEST_F(KeyboardLayoutTest, NextPrevCycleRestores)
{
    UINT n = GetKeyboardLayoutList(0, nullptr);
    HKL before = GetKeyboardLayout(0);

    ActivateKeyboardLayout((HKL)(size_t)HKL_NEXT, 0);
    HKL after = ActivateKeyboardLayout((HKL)(size_t)HKL_PREV, 0);

    if (n >= 2)
    {
        // 多布局：PREV 返回的是 NEXT 之后的布局，再切回 before。
        EXPECT_EQ(GetKeyboardLayout(0), before);
    }
    else
    {
        // 单布局：NEXT/PREV 均原地不动，after 即 before。
        EXPECT_EQ(after, before);
    }
}
