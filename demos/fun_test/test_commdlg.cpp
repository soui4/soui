/**
 * SwinxPickFileAsyncA / SwinxFilterToAcceptA：swinx 私有的异步文件选择（声明在
 * include/commdlg.h，Win32 没有同名 API）。
 *
 * 这里只覆盖"不需要真对话框"的那部分契约：
 *   - 筛选串 → 浏览器 accept 的纯换算（无副作用，各平台同一份代码，见 src/cmmmdlg.cpp）；
 *   - 参数非法与多选请求必须在把请求交给平台钩子之前就被拒掉，且拒绝时一次回调都不给。
 * 会真弹对话框的路径（返回 1 之后由平台回调）在无头环境不可测：Linux 会去连 DBus portal，
 * macOS 会弹 NSOpenPanel，wasm 会要页面宿主钩子 —— 那部分由 demos/wasm-demo/smoke.js 用假
 * 钩子覆盖（受理/非阻塞/消毒后的落地路径/过期 id 丢弃），浏览器真机按
 * doc/soui-wasm-port-plan.md 的清单人工过。
 *
 * 整个文件被 #ifndef _WIN32 包住：这两个符号只存在于 swinx 的 commdlg.h，Windows 上
 * <commdlg.h> 是系统 SDK 的，没有它们（CTest 在 Windows 仍会注册条目但二进制里没有，
 * 通过证据只能来自 Linux/macOS —— 同 AGENTS.md 对平台专属用例的说明）。
 */
#ifndef _WIN32

#include <gtest/gtest.h>
#include <windows.h>
#include <commdlg.h>

#include <string.h>
#include <string>

namespace
{
    /* 把 Win32 的筛选表拼成"描述\0模式\0…\0"再加一个结尾 0。 */
    void build_filter(std::string &strOut, const char *const *parts, int nParts)
    {
        strOut.clear();
        for (int i = 0; i < nParts; i++)
        {
            strOut += parts[i];
            strOut += '\0';
        }
        strOut += '\0';
    }

    struct AcceptCase
    {
        const char *const *parts;
        int nParts;
        const char *pszWant;
    };

    int g_nPickCalls = 0;
    int g_nPickLastResult = -12345;
    bool g_bPickLastPathIsNull = true;
    char g_szPickLastPath[260];

    void CALLBACK OnPick(int nResult, LPCSTR lpszUtf8Path, void *pCtx)
    {
        (void)pCtx;
        g_nPickCalls++;
        g_nPickLastResult = nResult;
        g_bPickLastPathIsNull = (lpszUtf8Path == NULL);
        g_szPickLastPath[0] = 0;
        if (lpszUtf8Path != NULL)
        {
            strncpy(g_szPickLastPath, lpszUtf8Path, sizeof(g_szPickLastPath) - 1);
            g_szPickLastPath[sizeof(g_szPickLastPath) - 1] = 0;
        }
    }

    void reset_pick_probe()
    {
        g_nPickCalls = 0;
        g_nPickLastResult = -12345;
        g_bPickLastPathIsNull = true;
        g_szPickLastPath[0] = 0;
    }

    /* 只填"必然在钩子之前被拒"的字段，所以这份 OPENFILENAMEA 不触发任何对话框。 */
    OPENFILENAMEA make_of(LPSTR pszFile, DWORD nMaxFile, DWORD flags)
    {
        OPENFILENAMEA of;
        memset(&of, 0, sizeof(of));
        of.lStructSize = sizeof(of);
        of.lpstrFile = pszFile;
        of.nMaxFile = nMaxFile;
        of.lpstrFilter = "文本\0*.txt\0\0";
        of.Flags = flags;
        return of;
    }

    static const char *const kGroupTxtPng[] = {"文本与图片", "*.txt;*.png"};
    static const char *const kGroupAllDots[] = {"所有文件", "*.*"};
    static const char *const kGroupStar[] = {"任意", "*"};
    static const char *const kGroupMime[] = {"图片", "image/png"};
    static const char *const kGroupDup[] = {"A", "*.png; *.PNG", "B", "*.png"};
    static const char *const kGroupEmptyPattern[] = {"全部", ""};
    static const char *const kGroupMixed[] = {"文本", "*.txt", "所有文件", "*.*"};
}

/* accept 换算：描述段丢弃、';' 切分、"*.x"→".x"、MIME 原样、去重、trim 空白；
 * "*" / "*.*" / 空模式段都表示"这组接受全部"，而 accept 表达不了"其中一组接受全部"，
 * 所以整份筛选退化成空 accept（= 不做限制）。 */
TEST(swinx_commdlg, filter_to_accept_table)
{
    static const AcceptCase cases[] = {
        {kGroupTxtPng, 2, ".txt,.png"},
        {kGroupAllDots, 2, ""},
        {kGroupStar, 2, ""},
        {kGroupEmptyPattern, 2, ""},
        {kGroupMime, 2, "image/png"},
        {kGroupDup, 4, ".png,.PNG"},
        {kGroupMixed, 4, ""},
    };
    const int n = (int)(sizeof(cases) / sizeof(cases[0]));
    ASSERT_GT(n, 0);

    char szOut[256];
    for (int i = 0; i < n; i++)
    {
        std::string strFilter;
        build_filter(strFilter, cases[i].parts, cases[i].nParts);
        memset(szOut, 0, sizeof(szOut));
        const int ret = SwinxFilterToAcceptA(strFilter.c_str(), szOut, (int)sizeof(szOut));
        const std::string strWant(cases[i].pszWant);
        EXPECT_EQ(ret, (int)strWant.length()) << "case " << i;
        EXPECT_EQ(std::string(szOut), strWant) << "case " << i;
        /* 返回值必须就是写进去的字符数（不含 0）：调用方靠它判断有没有截断。 */
        if (ret >= 0)
            EXPECT_EQ((size_t)ret, strlen(szOut)) << "case " << i;
    }

    /* 没有筛选表（NULL）时不做限制。 */
    szOut[0] = 'X';
    EXPECT_EQ(SwinxFilterToAcceptA(NULL, szOut, (int)sizeof(szOut)), 0);
    EXPECT_EQ(szOut[0], '\0');
}

/* 探长度与"缓冲不够必须显式失败"：静默截断会把半个 accept 交出去，比报错更糟。 */
TEST(swinx_commdlg, filter_to_accept_probe_and_short_buffer)
{
    std::string strFilter;
    build_filter(strFilter, kGroupTxtPng, 2);
    const std::string strWant(".txt,.png");
    const int cchWant = (int)strWant.length();

    char buf[64];
    memset(buf, 0x5A, sizeof(buf));
    /* cchOut == 0 是探长度形态：返回含结尾 0 的所需字节数，且一个字节都不写。 */
    EXPECT_EQ(SwinxFilterToAcceptA(strFilter.c_str(), buf, 0), cchWant + 1);
    EXPECT_EQ(buf[0], (char)0x5A);
    EXPECT_EQ(buf[cchWant], (char)0x5A);

    /* 刚好差一个 0 的缓冲：必须 -1，不能写出没有结尾 0 的串。 */
    memset(buf, 0, sizeof(buf));
    EXPECT_EQ(SwinxFilterToAcceptA(strFilter.c_str(), buf, cchWant), -1);
    EXPECT_EQ(GetLastError(), (DWORD)ERROR_INVALID_PARAMETER);
    EXPECT_EQ(buf[0], '\0');

    /* 多一个字节就写得下：与上一条一起钉住"需要 length + 1"这个边界。 */
    EXPECT_EQ(SwinxFilterToAcceptA(strFilter.c_str(), buf, cchWant + 1), cchWant);
    EXPECT_EQ(std::string(buf), strWant);

    /* 参数非法的两种形态同样显式失败。 */
    EXPECT_EQ(SwinxFilterToAcceptA(strFilter.c_str(), buf, -1), -1);
    EXPECT_EQ(GetLastError(), (DWORD)ERROR_INVALID_PARAMETER);
    EXPECT_EQ(SwinxFilterToAcceptA(strFilter.c_str(), NULL, 16), -1);
    EXPECT_EQ(GetLastError(), (DWORD)ERROR_INVALID_PARAMETER);

    /* 空 accept 也要 1 个字节放结尾 0 —— 探长度对"没有内容"同样成立。 */
    std::string strAll;
    build_filter(strAll, kGroupAllDots, 2);
    EXPECT_EQ(SwinxFilterToAcceptA(strAll.c_str(), buf, 0), 1);
    EXPECT_EQ(SwinxFilterToAcceptA(strAll.c_str(), buf, 1), 0);
}

/* 非法参数在把请求挂出去之前就拒掉：返回 0，一次回调都不给（契约里 pfnDone 不会被调用）。 */
TEST(swinx_commdlg, pick_invalid_args_rejected_without_callback)
{
    reset_pick_probe();
    char szFile[260];
    szFile[0] = 0;

    EXPECT_EQ(SwinxPickFileAsyncA(NULL, OPEN, OnPick, NULL), 0);
    EXPECT_EQ(GetLastError(), (DWORD)ERROR_INVALID_PARAMETER);

    OPENFILENAMEA of = make_of(szFile, (DWORD)sizeof(szFile), 0);
    EXPECT_EQ(SwinxPickFileAsyncA(&of, OPEN, NULL, NULL), 0);
    EXPECT_EQ(GetLastError(), (DWORD)ERROR_INVALID_PARAMETER);

    of.lpstrFile = NULL;
    EXPECT_EQ(SwinxPickFileAsyncA(&of, OPEN, OnPick, NULL), 0);
    EXPECT_EQ(GetLastError(), (DWORD)ERROR_INVALID_PARAMETER);

    of = make_of(szFile, 0, 0);
    EXPECT_EQ(SwinxPickFileAsyncA(&of, OPEN, OnPick, NULL), 0);
    EXPECT_EQ(GetLastError(), (DWORD)ERROR_INVALID_PARAMETER);

    EXPECT_EQ(g_nPickCalls, 0);
}

/* 多选显式拒绝，而不是悄悄降级成单选：异步契约一次只回一条路径。
 * 这条同样必须发生在平台钩子之前，所以无头环境不会弹窗。 */
TEST(swinx_commdlg, pick_multiselect_rejected_explicitly)
{
    reset_pick_probe();
    char szFile[260];
    szFile[0] = 0;

    OPENFILENAMEA of = make_of(szFile, (DWORD)sizeof(szFile), OFN_ALLOWMULTISELECT);
    EXPECT_EQ(SwinxPickFileAsyncA(&of, OPEN, OnPick, NULL), 0);
    EXPECT_EQ(GetLastError(), (DWORD)ERROR_CALL_NOT_IMPLEMENTED);
    EXPECT_EQ(g_nPickCalls, 0);
    /* 拒绝不改调用方的缓冲。 */
    EXPECT_EQ(szFile[0], '\0');
}

#endif  // !_WIN32
