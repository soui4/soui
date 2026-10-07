#include <souistd.h>
#include <core/SLayoutSize.h>
#include <core/SWnd.h>
#include <matrix/SMatrix.h>
#include <valueAnimator/SValueAnimator.h>
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
    root.DestroyAllChildren();
}

namespace
{
// SWindow::~SWindow() does not destroy the child window tree, so a stack root has to
// release its children explicitly. The guard keeps that true even when an ASSERT_*
// returns early, which would otherwise leak a whole control tree.
struct WindowTreeGuard
{
    SWindow *root;
    explicit WindowTreeGuard(SWindow *pRoot)
        : root(pRoot)
    {
    }
    ~WindowTreeGuard()
    {
        root->DestroyAllChildren();
    }
};
} // namespace

// An XML attribute may name a chained member explicitly with a namespace prefix, e.g.
// layout:width="-1". The chain macro strips the prefix before forwarding the attribute
// to the member, and a name without the prefix still reaches the same member, so both
// spellings must leave identical layout parameters on the window.
TEST(soui_window, chained_member_prefix_is_stripped)
{
    SComMgr2 comMgr;
    SAutoRefPtr<IRenderFactory> renderFactory;
    ASSERT_TRUE(comMgr.CreateRender_GDI((IObjRef **)&renderFactory));
    ASSERT_TRUE(renderFactory);
    SApplication app(renderFactory, NULL);

    SWindow root;
    WindowTreeGuard guard(&root);
    ASSERT_TRUE(root.CreateChildrenFromXml(L"<window name='plain' width='120'/>"
                                           L"<window name='prefixed' layout:width='120'/>"
                                           L"<window name='upper' LAYOUT:width='120'/>"
                                           L"<window name='wrap' layout:width='-1'/>"
                                           L"<window name='unknown' nosuch:width='120'/>"));

    SWindow *plain = root.FindChildByName(L"plain", 1);
    SWindow *prefixed = root.FindChildByName(L"prefixed", 1);
    SWindow *upper = root.FindChildByName(L"upper", 1);
    SWindow *wrap = root.FindChildByName(L"wrap", 1);
    SWindow *unknown = root.FindChildByName(L"unknown", 1);
    ASSERT_TRUE(plain != NULL && plain->GetLayoutParam() != NULL);
    ASSERT_TRUE(prefixed != NULL && prefixed->GetLayoutParam() != NULL);
    ASSERT_TRUE(upper != NULL && upper->GetLayoutParam() != NULL);
    ASSERT_TRUE(wrap != NULL && wrap->GetLayoutParam() != NULL);
    ASSERT_TRUE(unknown != NULL && unknown->GetLayoutParam() != NULL);

    // Both spellings must reach the layout parameter object of the window. The default is
    // wrapContent, so IsSpecifiedSize() is only true when the attribute really arrived.
    ASSERT_TRUE(plain->GetLayoutParam()->IsSpecifiedSize(Horz));
    ASSERT_TRUE(prefixed->GetLayoutParam()->IsSpecifiedSize(Horz));
    ASSERT_TRUE(upper->GetLayoutParam()->IsSpecifiedSize(Horz));

    LAYOUTSIZE plainSize = { 0.0f, SOUI::px };
    LAYOUTSIZE prefixedSize = { 0.0f, SOUI::px };
    LAYOUTSIZE upperSize = { 0.0f, SOUI::px };
    ASSERT_TRUE(plain->GetLayoutParam()->GetSpecifiedSize(Horz, &plainSize));
    ASSERT_TRUE(prefixed->GetLayoutParam()->GetSpecifiedSize(Horz, &prefixedSize));
    ASSERT_TRUE(upper->GetLayoutParam()->GetSpecifiedSize(Horz, &upperSize));
    EXPECT_EQ(plainSize.unit, prefixedSize.unit);
    EXPECT_EQ(plainSize.fSize, prefixedSize.fSize);
    EXPECT_EQ(120.0f, prefixedSize.fSize);
    // The prefix itself is matched without case sensitivity, like every other attribute name.
    EXPECT_EQ(prefixedSize.fSize, upperSize.fSize);

    // layout:width="-1" means wrapContent, exactly like width="-1".
    EXPECT_TRUE(wrap->GetLayoutParam()->IsWrapContent(Horz));

    // A prefix nobody declared must not be accepted as if it were the bare name.
    EXPECT_FALSE(unknown->GetLayoutParam()->IsSpecifiedSize(Horz));
}

// The window style is a chained value member, so the prefix has to go through the value
// (non-pointer) chain macro as well, and it has to name that member only: style: is the
// style's prefix, layout: belongs to the two layout members.
TEST(soui_window, chained_style_member_prefix_is_stripped)
{
    SComMgr2 comMgr;
    SAutoRefPtr<IRenderFactory> renderFactory;
    ASSERT_TRUE(comMgr.CreateRender_GDI((IObjRef **)&renderFactory));
    ASSERT_TRUE(renderFactory);
    SApplication app(renderFactory, NULL);

    SWindow root;
    WindowTreeGuard guard(&root);
    ASSERT_TRUE(root.CreateChildrenFromXml(L"<window name='plain' colorBkgnd='#102030'/>"
                                           L"<window name='styled' style:colorBkgnd='#102030'/>"
                                           L"<window name='upper' STYLE:colorBkgnd='#102030'/>"
                                           L"<window name='layoutPrefixed' layout:colorBkgnd='#102030'/>"));

    SWindow *plain = root.FindChildByName(L"plain", 1);
    SWindow *styled = root.FindChildByName(L"styled", 1);
    SWindow *upper = root.FindChildByName(L"upper", 1);
    SWindow *layoutPrefixed = root.FindChildByName(L"layoutPrefixed", 1);
    ASSERT_TRUE(plain != NULL);
    ASSERT_TRUE(styled != NULL);
    ASSERT_TRUE(upper != NULL);
    ASSERT_TRUE(layoutPrefixed != NULL);
    // The attribute really was applied: the style default is CR_INVALID.
    EXPECT_NE(CR_INVALID, plain->GetStyle().m_crBg);
    // ... and the prefixed spelling produced the very same style field.
    EXPECT_EQ(plain->GetStyle().m_crBg, styled->GetStyle().m_crBg);
    // ... matched without case sensitivity.
    EXPECT_EQ(plain->GetStyle().m_crBg, upper->GetStyle().m_crBg);
    // style: and layout: name different members, so no layout attribute may reach the style.
    EXPECT_EQ(CR_INVALID, layoutPrefixed->GetStyle().m_crBg);
}

namespace
{
// The prefixes exist to tell members apart. A member reached through ATTR_CHAIN shares the
// XML attribute namespace of the object owning it, so once two members accept the same name
// the bare spelling can only ever reach the first one in chain order. The SWindow chains
// above cannot show that (their members own disjoint attribute names), so this host mirrors
// the arrangement with two animators that both take "duration".
class SChainedMemberHost : public SWindow {
    DEF_SOBJECT(SWindow, L"chainedMemberHost")

  public:
    SOUI_ATTRS_BEGIN()
        ATTR_CHAIN_PREFIX(m_animator, 0, SAttrChainPrefix::ANIMATOR)
        ATTR_CHAIN_PREFIX(m_slider, 0, SAttrChainPrefix::SLIDER)
    SOUI_ATTRS_END()

    SFloatAnimator m_animator;
    SFloatAnimator m_slider;
};
} // namespace

TEST(soui_window, chained_members_sharing_an_attribute_are_told_apart)
{
    SComMgr2 comMgr;
    SAutoRefPtr<IRenderFactory> renderFactory;
    ASSERT_TRUE(comMgr.CreateRender_GDI((IObjRef **)&renderFactory));
    ASSERT_TRUE(renderFactory);
    SApplication app(renderFactory, NULL);

    SChainedMemberHost host;
    const long sliderDuration = host.m_slider.getDuration();

    // A prefixed name reaches exactly the member owning that prefix, and only it.
    ASSERT_TRUE(SUCCEEDED(host.SetAttribute(L"animator:duration", L"500", TRUE)));
    EXPECT_EQ(500, host.m_animator.getDuration());
    EXPECT_EQ(sliderDuration, host.m_slider.getDuration());

    ASSERT_TRUE(SUCCEEDED(host.SetAttribute(L"slider:duration", L"700", TRUE)));
    EXPECT_EQ(500, host.m_animator.getDuration());
    EXPECT_EQ(700, host.m_slider.getDuration());

    // The bare name still goes to the first member in chain order -- the behaviour the
    // prefixes let XML opt out of.
    ASSERT_TRUE(SUCCEEDED(host.SetAttribute(L"duration", L"900", TRUE)));
    EXPECT_EQ(900, host.m_animator.getDuration());
    EXPECT_EQ(700, host.m_slider.getDuration());

    // A prefix belonging to neither member is not silently treated as the bare name.
    EXPECT_TRUE(FAILED(host.SetAttribute(L"interpolator:duration", L"100", TRUE)));
    EXPECT_EQ(900, host.m_animator.getDuration());
    EXPECT_EQ(700, host.m_slider.getDuration());
}

namespace
{
// A prefix routes, it does not merely decorate: a member must not even be asked about an
// attribute addressing somebody else. SetAttribute below accepts whatever it is handed, so
// the call counter is positive proof of whether the chain offered it the attribute at all.
// Were the prefix only stripped and the raw name forwarded to every member, this member
// would swallow the "animator:" attributes and the animator behind it would never be
// configured while the call still reported success.
class SLaxMember : public SFloatAnimator {
  public:
    SLaxMember()
        : m_nAsked(0)
    {
    }

    virtual HRESULT SetAttribute(const SStringW &strAttr, const SStringW &strValue, BOOL bLoading)
    {
        (void)strValue;
        (void)bLoading;
        ++m_nAsked;
        m_strLastAsked = strAttr;
        return S_OK;
    }

    int m_nAsked;
    SStringW m_strLastAsked;
};

// The lax member is deliberately the first in chain order: whatever it is asked about, it
// takes. Only the routing rule keeps another member's attributes away from it.
class SRoutingHost : public SWindow {
    DEF_SOBJECT(SWindow, L"routingHost")

  public:
    SOUI_ATTRS_BEGIN()
        ATTR_CHAIN_PREFIX(m_lax, 0, SAttrChainPrefix::SLIDER)
        ATTR_CHAIN_PREFIX(m_animator, 0, SAttrChainPrefix::ANIMATOR)
    SOUI_ATTRS_END()

    SLaxMember m_lax;
    SFloatAnimator m_animator;
};
} // namespace

TEST(soui_window, chained_member_is_not_asked_about_another_members_prefix)
{
    SComMgr2 comMgr;
    SAutoRefPtr<IRenderFactory> renderFactory;
    ASSERT_TRUE(comMgr.CreateRender_GDI((IObjRef **)&renderFactory));
    ASSERT_TRUE(renderFactory);
    SApplication app(renderFactory, NULL);

    SRoutingHost host;

    // animator: addresses the animator, so the lax slider member is never handed the name,
    // not even in its still prefixed form.
    ASSERT_TRUE(SUCCEEDED(host.SetAttribute(L"animator:duration", L"500", TRUE)));
    EXPECT_EQ(0, host.m_lax.m_nAsked);
    EXPECT_EQ(500, host.m_animator.getDuration());

    // slider: addresses the lax member, which is asked with the prefix already stripped.
    ASSERT_TRUE(SUCCEEDED(host.SetAttribute(L"slider:duration", L"700", TRUE)));
    EXPECT_EQ(1, host.m_lax.m_nAsked);
    EXPECT_EQ(0, host.m_lax.m_strLastAsked.CompareNoCase(L"duration"));
    EXPECT_EQ(500, host.m_animator.getDuration());

    // A prefix no member declares is dropped instead of being retried as the bare name:
    // forwarded, it would have been taken by the lax member above.
    EXPECT_TRUE(FAILED(host.SetAttribute(L"interpolator:duration", L"100", TRUE)));
    EXPECT_EQ(1, host.m_lax.m_nAsked);
    EXPECT_EQ(500, host.m_animator.getDuration());

    // A plain name is still offered to every member in chain order, so the lax member takes
    // it first and the animator is left alone.
    ASSERT_TRUE(SUCCEEDED(host.SetAttribute(L"duration", L"900", TRUE)));
    EXPECT_EQ(2, host.m_lax.m_nAsked);
    EXPECT_EQ(500, host.m_animator.getDuration());
}

namespace
{
// SOUI_ATTRS_BEGIN splits the name once, and the object's own attribute table keeps comparing
// the full name while the chained members read the bare one. Were the split applied to the
// table itself, "duration" would be taken by the object below even when spelled
// "animator:duration" -- the member the prefix names would silently keep its default.
class SOwnNameHost : public SWindow {
    DEF_SOBJECT(SWindow, L"ownNameHost")

  public:
    SOwnNameHost()
        : m_nOwnDuration(0)
    {
    }

    SOUI_ATTRS_BEGIN()
        ATTR_INT(L"duration", m_nOwnDuration, FALSE)
        ATTR_CHAIN_PREFIX(m_animator, 0, SAttrChainPrefix::ANIMATOR)
    SOUI_ATTRS_END()

    int m_nOwnDuration;
    SFloatAnimator m_animator;
};
} // namespace

TEST(soui_window, own_attribute_table_never_sees_a_prefixed_name)
{
    SComMgr2 comMgr;
    SAutoRefPtr<IRenderFactory> renderFactory;
    ASSERT_TRUE(comMgr.CreateRender_GDI((IObjRef **)&renderFactory));
    ASSERT_TRUE(renderFactory);
    SApplication app(renderFactory, NULL);

    SOwnNameHost host;
    const long animatorDefault = host.m_animator.getDuration();

    // The plain name belongs to the object, so the chain behind it is not reached.
    ASSERT_TRUE(SUCCEEDED(host.SetAttribute(L"duration", L"123", TRUE)));
    EXPECT_EQ(123, host.m_nOwnDuration);
    EXPECT_EQ(animatorDefault, host.m_animator.getDuration());

    // The prefixed name addresses the member: the object's own field must stay put even though
    // "duration" is in its own table, and the member reads the name with the prefix stripped.
    ASSERT_TRUE(SUCCEEDED(host.SetAttribute(L"animator:duration", L"500", TRUE)));
    EXPECT_EQ(123, host.m_nOwnDuration);
    EXPECT_EQ(500, host.m_animator.getDuration());

    // A prefix nobody in the chain owns is dropped, and it may not fall back to the own table.
    EXPECT_TRUE(FAILED(host.SetAttribute(L"slider:duration", L"700", TRUE)));
    EXPECT_EQ(123, host.m_nOwnDuration);
    EXPECT_EQ(500, host.m_animator.getDuration());
}
