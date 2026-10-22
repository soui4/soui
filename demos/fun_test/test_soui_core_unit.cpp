#include <souistd.h>
#include <core/SLayoutSize.h>
#include <core/SWnd.h>
#include <matrix/SMatrix.h>
#include <valueAnimator/SValueAnimator.h>
#include <xml/SXml.h>
#include <commgr2.h>
#include <gtest/gtest.h>
#include <vector>

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

TEST(soui_xml, interface_objects_are_unique_and_stable_per_handle)
{
    SXmlDoc doc;
    ASSERT_TRUE(doc.LoadString(L"<root><item id=\"1\"/><item id=\"2\"/></root>", xml_parse_default));

    IXmlNode *pRoot = doc.Root()->Child(L"root", TRUE);
    ASSERT_TRUE(pRoot != NULL);

    // 同一个句柄多次转换必须得到同一个接口对象。
    IXmlNode *pFirst = pRoot->Child(L"item", TRUE);
    ASSERT_TRUE(pFirst != NULL);
    EXPECT_EQ(pFirst, pRoot->Child(L"item", TRUE));

    // 不同句柄必须对应不同的接口对象。
    IXmlNode *pSecond = pFirst->NextSibling();
    ASSERT_TRUE(pSecond != NULL);
    EXPECT_NE(pFirst, pSecond);

    IXmlAttr *pAttr = pFirst->Attribute(L"id", TRUE);
    ASSERT_TRUE(pAttr != NULL);
    EXPECT_EQ(pAttr, pFirst->Attribute(L"id", TRUE));
    EXPECT_STREQ(L"1", pAttr->Value());
}

TEST(soui_xml, reload_rebinds_interface_objects_to_new_document_content)
{
    SXmlDoc doc;
    ASSERT_TRUE(doc.LoadString(L"<root><item/></root>", xml_parse_default));
    IXmlNode *pRoot = doc.Root()->Child(L"root", TRUE);
    ASSERT_TRUE(pRoot != NULL);
    ASSERT_TRUE(pRoot->Child(L"item", TRUE) != NULL);

    ASSERT_TRUE(doc.LoadString(L"<root><other/></root>", xml_parse_default));
    pRoot = doc.Root()->Child(L"root", TRUE);
    ASSERT_TRUE(pRoot != NULL);
    EXPECT_TRUE(pRoot->Child(L"other", TRUE) != NULL);
    EXPECT_TRUE(pRoot->Child(L"item", TRUE) == NULL);
}

TEST(soui_xml, value_and_interface_apis_share_the_same_handle)
{
    SXmlDoc doc;
    ASSERT_TRUE(doc.LoadString(L"<root><item id=\"1\"/></root>", xml_parse_default));

    SXmlNode valueRoot = doc.root().child(L"root");
    ASSERT_FALSE(valueRoot.empty());

    IXmlNode *pRoot = doc.Root()->Child(L"root", TRUE);
    ASSERT_TRUE(pRoot != NULL);
    EXPECT_EQ(valueRoot.GetPrivPtr(), pRoot->GetPrivPtr());

    SStringW text;
    pRoot->ToString(&text);
    EXPECT_FALSE(text.IsEmpty());
}

TEST(soui_xml, traversal_walks_reuse_the_cached_wrappers)
{
    SXmlDoc doc;
    ASSERT_TRUE(doc.LoadString(L"<root a=\"1\" b=\"2\"><item id=\"1\"><sub/></item><item id=\"2\"/></root>",
                               xml_parse_default));

    IXmlNode *pRoot = doc.Root()->Child(L"root", TRUE);
    ASSERT_TRUE(pRoot != NULL);

    // 属性链与按名查找必须指向同一个缓存对象。
    IXmlAttr *pA = pRoot->FirstAttribute();
    ASSERT_TRUE(pA != NULL);
    EXPECT_STREQ(L"a", pA->Name());
    EXPECT_EQ(pA, pRoot->Attribute(L"a", TRUE));

    IXmlAttr *pB = pA->Next();
    ASSERT_TRUE(pB != NULL);
    EXPECT_STREQ(L"b", pB->Name());
    EXPECT_EQ(pB, pRoot->Attribute(L"b", TRUE));
    EXPECT_EQ(pB, pRoot->LastAttribute());
    EXPECT_EQ(pA, pB->Prev());

    // 子节点链与按名/按序查找也必须指向同一个缓存对象。
    IXmlNode *pFirst = pRoot->FirstChild();
    ASSERT_TRUE(pFirst != NULL);
    EXPECT_EQ(pFirst, pRoot->Child(L"item", TRUE));

    IXmlNode *pLast = pRoot->LastChild();
    ASSERT_TRUE(pLast != NULL);
    EXPECT_EQ(pLast, pFirst->NextSibling());
    EXPECT_EQ(pLast, pFirst->NextSibling2(L"item", TRUE));
    EXPECT_EQ(pFirst, pLast->PrevSibling());
    EXPECT_EQ(pFirst, pLast->PrevSibling2(L"item", TRUE));
    EXPECT_EQ(1, pFirst->Attribute(L"id", TRUE)->AsInt());
    EXPECT_EQ(2, pLast->Attribute(L"id", TRUE)->AsInt());

    IXmlNode *pSub = pFirst->Child(L"sub", TRUE);
    ASSERT_TRUE(pSub != NULL);
    EXPECT_EQ(pSub, pFirst->FirstChild());
    EXPECT_EQ(pSub, pFirst->LastChild());
    EXPECT_EQ(pSub, pLast->PrevSibling()->FirstChild());
}

TEST(soui_xml, missing_lookups_and_empty_nodes_return_null)
{
    SXmlDoc doc;
    ASSERT_TRUE(doc.LoadString(L"<root><leaf/></root>", xml_parse_default));

    IXmlNode *pRoot = doc.Root()->Child(L"root", TRUE);
    ASSERT_TRUE(pRoot != NULL);
    EXPECT_TRUE(pRoot->Child(L"missing", TRUE) == NULL);
    EXPECT_TRUE(pRoot->Child(L"leaf", FALSE) != NULL);
    EXPECT_TRUE(pRoot->Attribute(L"missing", TRUE) == NULL);

    IXmlNode *pLeaf = pRoot->Child(L"leaf", TRUE);
    ASSERT_TRUE(pLeaf != NULL);
    EXPECT_TRUE(pLeaf->FirstChild() == NULL);
    EXPECT_TRUE(pLeaf->LastChild() == NULL);
    EXPECT_TRUE(pLeaf->FirstAttribute() == NULL);
    EXPECT_TRUE(pLeaf->LastAttribute() == NULL);
    EXPECT_TRUE(pLeaf->NextSibling() == NULL);
    EXPECT_TRUE(pLeaf->PrevSibling() == NULL);
    EXPECT_TRUE(pLeaf->NextSibling2(L"leaf", TRUE) == NULL);
    EXPECT_TRUE(pLeaf->PrevSibling2(L"leaf", TRUE) == NULL);
    EXPECT_TRUE(pLeaf->Child(L"missing", TRUE) == NULL);
}

TEST(soui_xml, buffer_loads_rebind_cached_objects)
{
    SXmlDoc doc;

    const char utf8[] = "<root><alpha/></root>";
    ASSERT_TRUE(doc.LoadBuffer(utf8, sizeof(utf8) - 1, xml_parse_default, enc_utf8));
    IXmlNode *pRoot = doc.Root()->Child(L"root", TRUE);
    ASSERT_TRUE(pRoot != NULL);
    EXPECT_TRUE(pRoot->Child(L"alpha", TRUE) != NULL);

    // 再次加载不同内容后，旧内容不可见，缓存对象必须绑定到新句柄。
    const char reload[] = "<root><beta id=\"2\"/></root>";
    ASSERT_TRUE(doc.LoadBuffer(reload, sizeof(reload) - 1, xml_parse_default, enc_utf8));
    pRoot = doc.Root()->Child(L"root", TRUE);
    ASSERT_TRUE(pRoot != NULL);
    IXmlNode *pBeta = pRoot->Child(L"beta", TRUE);
    ASSERT_TRUE(pBeta != NULL);
    EXPECT_TRUE(pRoot->Child(L"alpha", TRUE) == NULL);
    EXPECT_EQ(2, pBeta->Attribute(L"id", TRUE)->AsInt());

    // 原地解析的缓冲区在文档生命周期内保持有效即可。
    const wchar_t wide[] = L"<root><gamma/></root>";
    std::vector<wchar_t> buffer(wide, wide + sizeof(wide) / sizeof(wide[0]));
    ASSERT_TRUE(doc.LoadBufferInplace(buffer.data(), (buffer.size() - 1) * sizeof(wchar_t),
                                      xml_parse_default, enc_wchar));
    pRoot = doc.Root()->Child(L"root", TRUE);
    ASSERT_TRUE(pRoot != NULL);
    EXPECT_TRUE(pRoot->Child(L"gamma", TRUE) != NULL);
    EXPECT_TRUE(pRoot->Child(L"beta", TRUE) == NULL);
}

TEST(soui_xml, copy_and_reset_discard_cached_objects)
{
    SXmlDoc src;
    ASSERT_TRUE(src.LoadString(L"<src><item id=\"9\"/></src>", xml_parse_default));

    SXmlDoc dst;
    ASSERT_TRUE(dst.LoadString(L"<dst/>", xml_parse_default));
    ASSERT_TRUE(dst.Root()->Child(L"dst", TRUE) != NULL);

    dst.Copy(&src);
    IXmlNode *pSrc = dst.Root()->Child(L"src", TRUE);
    ASSERT_TRUE(pSrc != NULL);
    EXPECT_TRUE(dst.Root()->Child(L"dst", TRUE) == NULL);

    IXmlNode *pItem = pSrc->Child(L"item", TRUE);
    ASSERT_TRUE(pItem != NULL);
    EXPECT_EQ(pItem, pSrc->FirstChild());
    IXmlAttr *pId = pItem->Attribute(L"id", TRUE);
    ASSERT_TRUE(pId != NULL);
    EXPECT_EQ(9, pId->AsInt());

    dst.Reset();
    EXPECT_TRUE(dst.Root()->FirstChild() == NULL);
    EXPECT_TRUE(dst.Root()->Child(L"src", TRUE) == NULL);
}

TEST(soui_xml, file_roundtrip_works_through_ansi_and_unicode_overloads)
{
    SXmlDoc doc;
    ASSERT_TRUE(doc.LoadString(L"<root ver=\"2\"><item id=\"1\"/></root>", xml_parse_default));

    const wchar_t *wPath = L"sxml_unit_test_w.xml";
    ASSERT_TRUE(doc.SaveFileW(wPath, L"\t", pugi::format_default, enc_utf8));
    SXmlDoc loadedW;
    ASSERT_TRUE(loadedW.LoadFileW(wPath, xml_parse_default, enc_utf8));
    IXmlNode *pRoot = loadedW.Root()->Child(L"root", TRUE);
    ASSERT_TRUE(pRoot != NULL);
    EXPECT_EQ(2, pRoot->Attribute(L"ver", TRUE)->AsInt());
    EXPECT_STREQ(L"1", pRoot->Child(L"item", TRUE)->Attribute(L"id", TRUE)->Value());
    EXPECT_EQ(0, ::remove("sxml_unit_test_w.xml"));

    const char *aPath = "sxml_unit_test_a.xml";
    ASSERT_TRUE(doc.SaveFileA(aPath, L"\t", pugi::format_default, enc_utf8));
    SXmlDoc loadedA;
    ASSERT_TRUE(loadedA.LoadFileA(aPath, xml_parse_default, enc_utf8));
    pRoot = loadedA.Root()->Child(L"root", TRUE);
    ASSERT_TRUE(pRoot != NULL);
    EXPECT_STREQ(L"root", pRoot->Name());
    EXPECT_EQ(1, pRoot->Child(L"item", TRUE)->Attribute(L"id", TRUE)->AsInt());
    EXPECT_EQ(0, ::remove(aPath));
}

TEST(soui_xml, parse_result_reports_last_operation)
{
    SXmlDoc doc;

    const char broken[] = "<root><item></root>";
    EXPECT_FALSE(doc.LoadBuffer(broken, sizeof(broken) - 1, xml_parse_default, enc_utf8));

    XmlParseResult result = {};
    doc.GetParseResult(&result);
    EXPECT_NE(xml_ok, result.status);
    EXPECT_GT(result.offset, 0);

    ASSERT_TRUE(doc.LoadString(L"<root/>", xml_parse_default));
    doc.GetParseResult(&result);
    EXPECT_EQ(xml_ok, result.status);
    EXPECT_EQ(0, result.offset);
}

TEST(soui_xml, mutation_methods_keep_cache_in_sync)
{
    SXmlDoc doc;
    ASSERT_TRUE(doc.LoadString(L"<root a=\"1\"/>", xml_parse_default));

    IXmlNode *pRoot = doc.Root()->Child(L"root", TRUE);
    ASSERT_TRUE(pRoot != NULL);

    IXmlAttr *pAttr = pRoot->FirstAttribute();
    ASSERT_TRUE(pAttr != NULL);
    EXPECT_EQ(pAttr, pRoot->Attribute(L"a", TRUE));
    EXPECT_TRUE(pAttr->set_userdata(11));
    EXPECT_EQ(11, pAttr->get_userdata());

    IXmlNode *pFirst = pRoot->PrependChild(L"first");
    ASSERT_TRUE(pFirst != NULL);
    EXPECT_EQ(pFirst, pRoot->FirstChild());

    IXmlNode *pChild = pRoot->AppendChild(L"child");
    ASSERT_TRUE(pChild != NULL);
    EXPECT_EQ(pChild, pRoot->Child(L"child", TRUE));
    EXPECT_EQ(pChild, pRoot->LastChild());

    EXPECT_TRUE(pChild->SetText(L"hello"));
    EXPECT_STREQ(L"hello", pChild->Text());
    EXPECT_TRUE(pChild->set_userdata(7));
    EXPECT_EQ(7, pChild->get_userdata());

    IXmlAttr *pId = pChild->AppendAttribute(L"id");
    ASSERT_TRUE(pId != NULL);
    EXPECT_EQ(pId, pChild->Attribute(L"id", TRUE));
    SXmlAttr(pId).set_value(L"42");
    EXPECT_EQ(42, pId->AsInt());

    IXmlAttr *pCopyAttr = pChild->AppendCopyAttribute(pAttr);
    ASSERT_TRUE(pCopyAttr != NULL);
    EXPECT_STREQ(L"a", pCopyAttr->Name());
    EXPECT_STREQ(L"1", pCopyAttr->Value());

    // 复制出来的节点是新句柄，因此必须是新的缓存对象。
    IXmlNode *pCopy = pRoot->AppendCopyNode(pChild);
    ASSERT_TRUE(pCopy != NULL);
    EXPECT_NE(pCopy, pChild);
    EXPECT_EQ(pCopy, pRoot->LastChild());
    EXPECT_STREQ(L"child", pCopy->Name());
    EXPECT_EQ(42, pCopy->Attribute(L"id", TRUE)->AsInt());
    EXPECT_NE(pCopy->Attribute(L"id", TRUE), pId);

    EXPECT_TRUE(pRoot->RemoveAttribute(L"a"));
    EXPECT_TRUE(pRoot->Attribute(L"a", TRUE) == NULL);

    EXPECT_TRUE(pRoot->RemoveChild(L"first"));
    EXPECT_TRUE(pRoot->Child(L"first", TRUE) == NULL);

    EXPECT_TRUE(pRoot->RemoveAllChilden());
    EXPECT_TRUE(pRoot->FirstChild() == NULL);
}
