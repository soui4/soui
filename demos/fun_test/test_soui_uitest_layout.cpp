/**
 * @file  test_soui_uitest_layout.cpp
 * @brief 基于 SouiTest 的 SOUI 布局系统无头单元测试（Tier A）
 *
 * 覆盖 SouiLayout（绝对定位）与 SLinearLayout（vbox/hbox）、SGridLayout 的度量规则：
 *   - pos 两个值 / 四个值的语义；负值 = 从右/下边锚定
 *   - padding 收缩父窗口的「子窗口布局矩形」（GetChildrenLayoutRect）
 *   - vbox 纵向堆叠、hbox 按 weight 分配剩余空间
 *   - wrap_content 父窗口按子窗口度量自身尺寸
 *   - extend 计入占位后内缩子窗口（线性布局里的"外边距"，净可见尺寸不变）
 *   - grid 按 columnCount/rowCount 把子窗口放进单元格（行优先填充）
 *   - 嵌套容器下的坐标累积（全体窗口共用宿主坐标系）
 *   - OnRelayout 改变视口后子窗口跟随重排
 *
 * 断言依据源码：
 *   - SWindow::GetClientRect       = 窗口矩形按 margin 收缩（SWnd.cpp）
 *   - SWindow::GetChildrenLayoutRect = 客户区再按 padding 收缩（SWnd.cpp）
 *   - SouiLayout::CalcPostion      pos 负值 => nMax - value；match_parent => nMax
 *   - SLinearLayout::LayoutChildren  weight 分配 int(remain*weight/fWeight+0.5)
 *   - SGridLayout::LayoutChildren  按 columnCount/rowCount 均分
 */
#include "test_soui_uitest_common.h"

#include <string>

using namespace SOUI;

namespace
{
/**
 * @brief 断言窗口矩形（宿主坐标系，全部窗口共享同一坐标系）。
 * @note  用自写 helper 而不是 EXPECT_EQ(CRect)，是为了在失败时打印 (l,t,r,b)。
 */
::testing::AssertionResult RectIs(SWindow *w, LONG l, LONG t, LONG r, LONG b)
{
    if (!w)
        return ::testing::AssertionFailure() << "window is null";
    CRect rc = SouiTest::rectOf(w);
    if (rc.left == l && rc.top == t && rc.right == r && rc.bottom == b)
        return ::testing::AssertionSuccess();
    return ::testing::AssertionFailure() << "actual=(" << rc.left << "," << rc.top << "," << rc.right << "," << rc.bottom
                                         << ") expected=(" << l << "," << t << "," << r << "," << b << ")";
}
} // namespace

#define EXPECT_RECT(w, l, t, r, b) EXPECT_TRUE(RectIs((w), (l), (t), (r), (b)))

// ---------------------------------------------------------------------------
// 1. SouiLayout：绝对定位
// ---------------------------------------------------------------------------
// pos='x,y' 只给左上角，尺寸由 width/height 决定。
TEST_F(SouiTestFixture, layout_pos_xy_with_width_height)
{
    SWindow *root = loadUi(
        L"<window>"
        L"  <window name='a' pos='10,20' width='50' height='30'/>"
        L"</window>");
    SWindow *a = SouiTest::findByName<SWindow>(root, L"a");
    ASSERT_TRUE(a != NULL);
    EXPECT_RECT(a, 10, 20, 60, 50);
}

// pos='l,t,r,b' 四个值直接给出矩形（宽高由 r-l / b-t 推出）。
TEST_F(SouiTestFixture, layout_pos_ltrb_defines_rect)
{
    SWindow *root = loadUi(
        L"<window>"
        L"  <window name='a' pos='10,20,110,70'/>"
        L"</window>");
    SWindow *a = SouiTest::findByName<SWindow>(root, L"a");
    ASSERT_TRUE(a != NULL);
    EXPECT_RECT(a, 10, 20, 110, 70);
}

// pos 负值 = 从父窗口右边/下边锚定（PositionItem2Value: nMax - value）。
TEST_F(SouiTestFixture, layout_negative_pos_anchors_right_bottom)
{
    SWindow *root = loadUi(
        L"<window>"
        L"  <window name='p' pos='0,0' width='200' height='100'>"
        L"    <window name='a' pos='-60,-40' width='50' height='30'/>"
        L"  </window>"
        L"</window>");
    SWindow *p = SouiTest::findByName<SWindow>(root, L"p");
    SWindow *a = SouiTest::findByName<SWindow>(root, L"a");
    ASSERT_TRUE(p != NULL);
    ASSERT_TRUE(a != NULL);
    EXPECT_RECT(p, 0, 0, 200, 100);
    // left = 200 - 60 = 140, top = 100 - 40 = 60
    EXPECT_RECT(a, 140, 60, 190, 90);
}

// ---------------------------------------------------------------------------
// 2. padding / extend：两套不同的"内缩"语义
// ---------------------------------------------------------------------------
// padding 收缩父窗口的「子窗口布局矩形」，子窗口的坐标原点随之偏移。
TEST_F(SouiTestFixture, layout_padding_shrinks_children_area)
{
    SWindow *root = loadUi(
        L"<window>"
        L"  <window name='p' pos='0,0' width='200' height='100' padding='10,20,30,40'>"
        L"    <window name='c' pos='0,0' width='-2' height='-2'/>"
        L"  </window>"
        L"</window>");
    SWindow *p = SouiTest::findByName<SWindow>(root, L"p");
    SWindow *c = SouiTest::findByName<SWindow>(root, L"c");
    ASSERT_TRUE(p != NULL);
    ASSERT_TRUE(c != NULL);
    EXPECT_RECT(p, 0, 0, 200, 100);
    // 子布局矩形 = (10,20,170,60)；宽度 -2 表示填满该矩形
    EXPECT_RECT(c, 10, 20, 170, 60);
}

// extend 在【线性布局】里是子窗口的外边距：先并入尺寸再内缩，净可见尺寸不变，
// 但纵向占位变大（后一个兄弟从 extend 之后的边界开始）。
TEST_F(SouiTestFixture, layout_extend_deflates_child_rect_in_vbox)
{
    SWindow *root = loadUi(
        L"<window layout='vbox'>"
        L"  <window name='a' width='-2' height='30' extend='5,6,7,8'/>"
        L"  <window name='b' width='-2' height='20'/>"
        L"</window>");
    SWindow *a = SouiTest::findByName<SWindow>(root, L"a");
    SWindow *b = SouiTest::findByName<SWindow>(root, L"b");
    ASSERT_TRUE(a != NULL);
    ASSERT_TRUE(b != NULL);

    // 尺寸先变成 30+6+8=44，再 DeflateRect(5,6,7,8) => (5,6,793,36)，可见高仍是 30
    EXPECT_RECT(a, 5, 6, 793, 36);
    EXPECT_EQ(SouiTest::rectOf(a).Height(), 30);

    // 下一个兄弟从 44 开始（extend 计入占位），不受内缩影响
    EXPECT_RECT(b, 0, 44, 800, 64);
}

// ---------------------------------------------------------------------------
// 3. SLinearLayout：vbox / hbox
// ---------------------------------------------------------------------------
// vbox 纵向堆叠：后一个的 top 等于前一个的 bottom，不会被父高度拉伸。
TEST_F(SouiTestFixture, layout_vbox_stacks_children_vertically)
{
    SWindow *root = loadUi(
        L"<window layout='vbox'>"
        L"  <window name='a' width='-2' height='30'/>"
        L"  <window name='b' width='-2' height='40'/>"
        L"  <window name='c' width='-2' height='50'/>"
        L"</window>");
    SWindow *a = SouiTest::findByName<SWindow>(root, L"a");
    SWindow *b = SouiTest::findByName<SWindow>(root, L"b");
    SWindow *c = SouiTest::findByName<SWindow>(root, L"c");
    ASSERT_TRUE(a != NULL);
    ASSERT_TRUE(b != NULL);
    ASSERT_TRUE(c != NULL);
    // 视口 800x600；width='-2' 填满父宽
    EXPECT_RECT(a, 0, 0, 800, 30);
    EXPECT_RECT(b, 0, 30, 800, 70);
    EXPECT_RECT(c, 0, 70, 800, 120);
    // 堆叠而非拉伸：最后一个不落到父底边
    EXPECT_LT(SouiTest::rectOf(c).bottom, 600);
}

// hbox 按 weight 分配【剩余】空间：固定宽 + weight 1:2。
TEST_F(SouiTestFixture, layout_hbox_weight_splits_remaining_space)
{
    SWindow *root = loadUi(
        L"<window layout='hbox'>"
        L"  <window name='a' width='100' height='-2'/>"
        L"  <window name='b' width='0' height='-2' weight='1'/>"
        L"  <window name='c' width='0' height='-2' weight='2'/>"
        L"</window>");
    SWindow *a = SouiTest::findByName<SWindow>(root, L"a");
    SWindow *b = SouiTest::findByName<SWindow>(root, L"b");
    SWindow *c = SouiTest::findByName<SWindow>(root, L"c");
    ASSERT_TRUE(a != NULL);
    ASSERT_TRUE(b != NULL);
    ASSERT_TRUE(c != NULL);

    CRect rca = SouiTest::rectOf(a);
    CRect rcb = SouiTest::rectOf(b);
    CRect rcc = SouiTest::rectOf(c);

    EXPECT_EQ(rca.right, 100);         // 固定宽
    EXPECT_EQ(rcb.left, rca.right);    // 紧邻排布
    EXPECT_EQ(rcc.left, rcb.right);    // 紧邻排布
    EXPECT_EQ(rcc.right, 800);         // 剩余空间被分完，右边界贴父窗口
    EXPECT_EQ(rcb.Width() + rcc.Width(), 700);
    // 权重 1:2 —— 允许 ±2px 的取整误差（源码 int(remain*w/fWeight+0.5f) 逐项取整）
    EXPECT_NEAR((double)rcb.Width() / (double)rcc.Width(), 0.5, 0.02);
}

// wrap_content：父窗口按子窗口内容度量自身尺寸（width='-1' height='-1'）。
TEST_F(SouiTestFixture, layout_wrap_content_parent_measures_children)
{
    SWindow *root = loadUi(
        L"<window>"
        L"  <window name='p' layout='vbox' width='-1' height='-1'>"
        L"    <window name='a' width='60' height='30'/>"
        L"    <window name='b' width='90' height='40'/>"
        L"  </window>"
        L"</window>");
    SWindow *p = SouiTest::findByName<SWindow>(root, L"p");
    ASSERT_TRUE(p != NULL);
    // 宽 = max(60,90) = 90，高 = 30+40 = 70
    EXPECT_RECT(p, 0, 0, 90, 70);
}

// ---------------------------------------------------------------------------
// 4. SGridLayout：按 columnCount/rowCount 把子窗口放进单元格
// ---------------------------------------------------------------------------
// 列宽/行高由单元格内子窗口的度量值决定（本用例统一给 400x300，正好填满 800x600）。
TEST_F(SouiTestFixture, layout_grid_places_children_into_cells)
{
    SWindow *root = loadUi(
        L"<window layout='grid' columnCount='2' rowCount='2'>"
        L"  <window name='a' width='400' height='300'/>"
        L"  <window name='b' width='400' height='300'/>"
        L"  <window name='c' width='400' height='300'/>"
        L"  <window name='d' width='400' height='300'/>"
        L"</window>");
    SWindow *a = SouiTest::findByName<SWindow>(root, L"a");
    SWindow *b = SouiTest::findByName<SWindow>(root, L"b");
    SWindow *c = SouiTest::findByName<SWindow>(root, L"c");
    SWindow *d = SouiTest::findByName<SWindow>(root, L"d");
    ASSERT_TRUE(a != NULL);
    ASSERT_TRUE(b != NULL);
    ASSERT_TRUE(c != NULL);
    ASSERT_TRUE(d != NULL);

    // 行优先填充：a b / c d
    EXPECT_RECT(a, 0, 0, 400, 300);
    EXPECT_RECT(b, 400, 0, 800, 300);
    EXPECT_RECT(c, 0, 300, 400, 600);
    EXPECT_RECT(d, 400, 300, 800, 600);
}

// ---------------------------------------------------------------------------
// 5. 坐标系统与重排协议
// ---------------------------------------------------------------------------
// 嵌套容器的坐标在整个窗口树里是同一个宿主坐标系，直接相加而非各自独立。
TEST_F(SouiTestFixture, layout_nested_containers_accumulate_offset)
{
    SWindow *root = loadUi(
        L"<window>"
        L"  <window name='p' pos='100,50' width='200' height='100'>"
        L"    <window name='c' pos='10,10' width='30' height='20'/>"
        L"  </window>"
        L"</window>");
    SWindow *p = SouiTest::findByName<SWindow>(root, L"p");
    SWindow *c = SouiTest::findByName<SWindow>(root, L"c");
    ASSERT_TRUE(p != NULL);
    ASSERT_TRUE(c != NULL);
    EXPECT_RECT(p, 100, 50, 300, 150);
    EXPECT_RECT(c, 110, 60, 140, 80);
}

// OnRelayout 换视口后，填满父窗口的子控件应跟随重排（layout 脏标记 -> 重算）。
TEST_F(SouiTestFixture, layout_relayout_moves_children_after_resize)
{
    SWindow *root = loadUi(
        L"<window layout='vbox'>"
        L"  <window name='a' width='-2' height='-2'/>"
        L"</window>");
    SWindow *a = SouiTest::findByName<SWindow>(root, L"a");
    ASSERT_TRUE(a != NULL);
    EXPECT_RECT(a, 0, 0, 800, 600);

    root->OnRelayout(CRect(0, 0, 400, 300));
    EXPECT_RECT(a, 0, 0, 400, 300);

    root->OnRelayout(CRect(0, 0, 640, 480));
    EXPECT_RECT(a, 0, 0, 640, 480);
}
