/**
 * @file       test_e2e_richedit_ime.cpp
 * @brief      Headless E2E tests for WM_IME_CHAR (input-method committed text) in SRichEdit.
 *
 * 为什么要单独立一条用例：swinx 在每个非 Windows 平台都把"键盘打不出来的字符"投成
 * WM_IME_CHAR，一条消息一个码点 —— Linux 是 XIM 的 commit 回调
 * (swinx/src/platform/linux/SConnection.cpp:235)，macOS 是 NSTextInputClient 的
 * insertText:（SNsWindow.mm:1181）加上 TranslateMessage 里 c<127 ? WM_CHAR : WM_IME_CHAR
 * 那条规则（SConnection.mm:601），iOS 同构，wasm 由 soui.html 的 DOM 组合事件经
 * SwinxInjectText 补齐。Windows 不是这样：DefWindowProc 先把 WM_IME_CHAR 转成 WM_CHAR
 * 再交给应用，于是输入法文本顺路走过 SRichEdit::OnChar 的风格过滤（ES_NUMBER 等）。
 * 非 Windows 没有那步转换，过滤就得在真正接 WM_IME_CHAR 的地方重做一遍 —— 也就是
 * SRichEdit::OnImeChar。这条用例锁住的就是这个差别，它不只保护 wasm 演示。
 *
 * 覆盖（无头）：
 *  - 逐码点 WM_IME_CHAR 按顺序进入文档，且码点原样保留（不被 ANSI DBCS 累加器拆坏）
 *  - number="1"（ES_NUMBER）的编辑框丢弃输入法提交的非数字
 *
 * 不覆盖（需要真实桌面输入法 + 人工步骤）：
 *  - 真实候选窗、组合期预编辑文本（marked text / underlined composition）
 *  - 各平台 IME 桥本身投递的消息形态（属于 swinx，不在 SOUI 这层）
 * 人工验证：
 *  1. 打开带 <edit> 的演示，用系统输入法提交一串汉字 -> 文字逐字出现在光标处
 *  2. 在 number="1" 的编辑框里用输入法提交汉字 -> 汉字不进文档，数字照常进
 */

#include <souistd.h>
#include <core/SWnd.h>
#include <control/SRichEdit.h>
#include <vector>
#include <gtest/gtest.h>
#include "common.h"
#include "test_container.h"

using namespace SOUI;

namespace
{
    // 兼容旧版 gtest（无 GTEST_SKIP）：拿不到文本服务时按成功结束，不算失败。
    // 触发条件是 richedit 引擎没进构建图（桌面构建没有 INIT_RICHEDIT 时要 dlopen
    // libmsftedit.so / Msftedit.dll 失败），此时整棵编辑框根本没有文档，断言无从谈起。
#define RICHEDIT_SKIP(reason) \
    do                        \
    {                         \
        SUCCEED() << reason;  \
        return;               \
    } while (0)

    /** 加载布局并给根窗口一个非零矩形：无头测试没有宿主，不 Move 的话客户区为空。 */
    void InitRoot(SWindow &root, TestContainer &container)
    {
        root.SetContainer(&container);
        ASSERT_TRUE(root.InitFromResId(_T("layout:XML_RICHEDIT_E2E")));
        CRect rcRoot(0, 0, 400, 200);
        root.Move(&rcRoot);
    }

    /**
     * 把码点序列变成 SStringW。用例里的码点全在 BMP，Windows 的 UTF-16 wchar_t 也
     * 放得下，不需要代理对 —— 这正是"一条消息一个码点"这个约定在各平台都成立的前提。
     * 用码点而不是源码里的中文串：源文件只留 ASCII，MSVC 不用 /source-charset 也不会
     * 把字面量按系统 ANSI 代码页解错。
     */
    SStringW FromCodePoints(const UINT *pCps, int nCps)
    {
        std::vector<wchar_t> buf(nCps + 1, 0);
        for (int i = 0; i < nCps; ++i)
            buf[i] = static_cast<wchar_t>(pCps[i]);
        return SStringW(&buf[0]);
    }

    SRichEdit *FindEdit(SWindow &root, LPCTSTR pszName)
    {
        return sobj_cast<SRichEdit>(root.FindChildByName(pszName));
    }

    /** 按 swinx 的投递形态送消息：一条一个码点，lParam 为 0（不是按键，没有重复次数）。 */
    void Inject(SRichEdit *pEdit, const UINT *pCps, int nCps)
    {
        for (int i = 0; i < nCps; ++i)
            pEdit->SSendMessage(WM_IME_CHAR, static_cast<WPARAM>(pCps[i]), 0);
    }

    SStringW DocOf(SRichEdit *pEdit)
    {
        return S_CT2W(pEdit->GetWindowText().c_str());
    }
}

// ---------------------------------------------------------------------------
// 逐码点提交进文档。用例串挑「输入法人」：非 Unicode 构建下 SRichEdit::OnChar 里
// 有个 ANSI DBCS 累加器，按 wParam 低字节调 IsDBCSLeadByte，而这四个码点的低字节
// 0x93 / 0x65 / 0xD5 / 0xBA 里前、后两个都落在 GBK 前导区 0x81-0xFE。文本一旦被塞
// 进 WM_CHAR，它们会被两两拼成乱码、最后一个整条吞掉，所以这里按整串逐字比对，
// 长度对上了也不算通过。
// ---------------------------------------------------------------------------
TEST(soui_richedit_ime_e2e, wm_ime_char_inserts_each_code_point)
{
    HeadlessApp app(_T("/e2e-res"));
    TestContainer container;
    SWindow root;
    InitRoot(root, container);

    SRichEdit *pEdit = FindEdit(root, _T("re_plain"));
    ASSERT_TRUE(pEdit != NULL);
    if (!pEdit->GetTextServices())
        RICHEDIT_SKIP("richedit 文本服务不可用：编辑框没有文档，本用例无从判断");

    static const UINT kCps[] = {0x8F93, 0x5165, 0x6CD5, 0x4EBA}; // 输入法人
    Inject(pEdit, kCps, sizeof(kCps) / sizeof(kCps[0]));

    EXPECT_STREQ(DocOf(pEdit).c_str(), FromCodePoints(kCps, 4).c_str());

    root.DestroyAllChildren();
}

// ---------------------------------------------------------------------------
// ES_NUMBER 对输入法提交的字符同样生效。Windows 靠 DefWindowProc 的
// WM_IME_CHAR -> WM_CHAR 转换顺路拿到这个过滤，非 Windows 平台以前整个落空：
// 摘掉 SRichEdit 的 WM_IME_CHAR 处理，消息会被 SwndProc 末尾的兜底转发直接送进
// ITextServices，数字照常收、汉字也照常收。
// ---------------------------------------------------------------------------
TEST(soui_richedit_ime_e2e, number_style_drops_non_digit_ime_chars)
{
    HeadlessApp app(_T("/e2e-res"));
    TestContainer container;
    SWindow root;
    InitRoot(root, container);

    SRichEdit *pNumber = FindEdit(root, _T("re_number"));
    ASSERT_TRUE(pNumber != NULL);
    if (!pNumber->GetTextServices())
        RICHEDIT_SKIP("richedit 文本服务不可用：编辑框没有文档，本用例无从判断");

    static const UINT kCps[] = {'1', '2', 0x4EBA, '0'}; // 12人0
    Inject(pNumber, kCps, sizeof(kCps) / sizeof(kCps[0]));

    // 期望只留数字：人（0x4EBA）必须被控件挡在文档外面。
    static const UINT kExpected[] = {'1', '2', '0'};
    EXPECT_STREQ(DocOf(pNumber).c_str(), FromCodePoints(kExpected, 3).c_str());

    root.DestroyAllChildren();
}
