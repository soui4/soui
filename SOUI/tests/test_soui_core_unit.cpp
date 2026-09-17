#include <souistd.h>
#include <core/SLayoutSize.h>
#include <core/SWnd.h>
#include <matrix/SMatrix.h>
#include <commgr2.h>
#include <gtest/gtest.h>

using namespace SOUI;

TEST(soui_layout_size, special_values_survive_parsing_and_serialization)
{
    SLayoutSize wrap = SLayoutSize::fromString(L"wrapContent");
    EXPECT_TRUE(wrap.isWrapContent());
    EXPECT_EQ(wrap.toPixelSize(150), SIZE_WRAP_CONTENT);
    EXPECT_EQ(wrap.toString(), L"-1");

    SLayoutSize match = SLayoutSize::fromString(L"full");
    EXPECT_TRUE(match.isMatchParent());
    EXPECT_EQ(match.toPixelSize(150), SIZE_MATCH_PARENT);
    EXPECT_EQ(match.toString(), L"-2");
}

TEST(soui_layout_size, device_independent_size_scales_but_pixels_do_not)
{
    SLayoutSize dpSize = SLayoutSize::fromString(L"15dp");
    EXPECT_TRUE(dpSize.isSpecifiedSize());
    EXPECT_EQ(dpSize.toPixelSize(150), 23); // 22.5 rounds up

    SLayoutSize pxSize = SLayoutSize::fromString(L"15px");
    EXPECT_EQ(pxSize.toPixelSize(150), 15);
}

TEST(soui_layout_size, invalid_size_does_not_produce_pixels)
{
    SLayoutSize size(10, dp);
    size.setInvalid();
    EXPECT_FALSE(size.isValid());
    EXPECT_EQ(size.toPixelSize(200), 0);
}

TEST(soui_matrix, inverse_restores_points_after_pivot_scale)
{
    SMatrix matrix;
    matrix.setScale2(2, 3, 10, 20);
    SPoint point = SPoint::Make(12, 24);
    matrix.mapPoints(&point, 1);
    EXPECT_NEAR(point.x(), 14, 1e-5);
    EXPECT_NEAR(point.y(), 32, 1e-5);

    SMatrix inverse;
    ASSERT_TRUE(matrix.invert(&inverse));
    inverse.mapPoints(&point, 1);
    EXPECT_NEAR(point.x(), 12, 1e-5);
    EXPECT_NEAR(point.y(), 24, 1e-5);
}

TEST(soui_matrix, zero_scale_cannot_be_inverted)
{
    SMatrix matrix;
    matrix.setScale(0, 1);
    SMatrix inverse;
    EXPECT_FALSE(matrix.invert(&inverse));
}

TEST(soui_window, named_child_lookup_respects_depth)
{
    SComMgr2 comMgr;
    SAutoRefPtr<IRenderFactory> renderFactory;
    ASSERT_TRUE(comMgr.CreateRender_GDI((IObjRef **)&renderFactory));
    ASSERT_TRUE(renderFactory);
    SApplication app(renderFactory, NULL);

    SWindow root;
    ASSERT_TRUE(root.CreateChildrenFromXml(
        L"<text name='direct' text='Ready'/>"
        L"<window name='branch'><text name='nested' text='Inner'/></window>"));
    ASSERT_EQ(root.GetChildrenCount(), 2u);
    EXPECT_TRUE(root.FindChildByName("direct", 1) != NULL);
    EXPECT_TRUE(root.FindChildByName("nested", 1) == NULL);
    SWindow *nested = root.FindChildByName("nested", 2);
    ASSERT_TRUE(nested);
    EXPECT_EQ(nested->GetWindowText(), _T("Inner"));
}
