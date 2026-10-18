/**
 * @file       GLWglTest.cpp
 * @brief      WGL 入口的契约测试：覆盖 swinx WGL 兼容层新增的扩展入口与降级入口。
 *
 * @details    这里验证的不是"某个驱动一定支持某个扩展"，而是"三平台的行为契约一致"：
 *             1. 扩展入口能经 wglGetProcAddress 取到，且扩展串的声明与取址结果自洽
 *                （不能声明了却取不到，也不能取到了却没声明）；
 *             2. 参数合法时入口真的生效（写入后用对应的 Get 入口回读）；
 *             3. 参数非法时必须明确失败，而不是静默成功或悄悄降级（例如请求 9.9 core）；
 *             4. 降级入口在 macOS/Linux 上返回文档写明的失败值，在 Windows 上交给真实实现；
 *             5. 整个测试过程不改变线程级的"当前上下文/当前 DC"。
 *
 *             所以断言对三个平台是同一份：Windows 的真实现与 swinx 的兼容实现都应当通过。
 *             只有"Windows 专有能力的返回值"用 GLWGL_EXPECT_DEGRADED 放宽。
 *
 * @note       测试会临时创建、切换、销毁 GL 上下文，全部在测试内部恢复；结束时把主上下文
 *             重新置为当前并让窗口重画一次。截图自检把这段放在抓图之前，一旦测试把上下文
 *             状态弄坏，后面的截图会立刻失败。
 */

#include "stdafx.h"
#include "GLWnd.h"

#include <stdio.h>
#include <string.h>

///////////////////////////////////////////////////////////////////////////////
// Windows 的系统头只有核心 WGL 声明（没有 wglext.h）；macOS/Linux 的扩展常量与
// PFN 类型由 swinx 的 <GL/wglext.h> 提供（经 <GL/gl.h> -> <wgl.h> 拉入）。
// 这里按 #ifndef 补上 demo 用到的那几个，与 GLProc.h 补 GL 1.5/2.0 常量的做法一致。
///////////////////////////////////////////////////////////////////////////////
#ifndef WGL_CONTEXT_MAJOR_VERSION_ARB
#define WGL_CONTEXT_MAJOR_VERSION_ARB 0x2091
#endif
#ifndef WGL_CONTEXT_MINOR_VERSION_ARB
#define WGL_CONTEXT_MINOR_VERSION_ARB 0x2092
#endif
#ifndef WGL_CONTEXT_PROFILE_MASK_ARB
#define WGL_CONTEXT_PROFILE_MASK_ARB 0x9126
#endif
#ifndef WGL_CONTEXT_CORE_PROFILE_BIT_ARB
#define WGL_CONTEXT_CORE_PROFILE_BIT_ARB 0x00000001
#endif
#ifndef WGL_CONTEXT_COMPATIBILITY_PROFILE_BIT_ARB
#define WGL_CONTEXT_COMPATIBILITY_PROFILE_BIT_ARB 0x00000002
#endif

///////////////////////////////////////////////////////////////////////////////
// 平台差异只有这一处：swinx 是我们自己实现的，严格按规范校验/降级；Windows 上的
// 返回值一律交给驱动（驱动可能不校验参数，也可能真的支持图层平面/GDI 字形）。
///////////////////////////////////////////////////////////////////////////////
#ifdef _WIN32
#define GLWGL_STRICT_SPEC 0
#define GLWGL_DEGRADED_NOTE _T("Windows：真实 GDI 能力，返回值交给驱动，只校验可调用且不改状态")
#else
#define GLWGL_STRICT_SPEC 1
#define GLWGL_DEGRADED_NOTE _T("swinx：无对应能力，要求返回文档写明的失败值")
#endif

/** WGL 扩展入口类型（一律经 wglGetProcAddress 取址，与 Windows 上的写法一致）。 */
typedef const char *(WINAPI *PFN_WGL_GetExtensionsStringARB)(HDC hdc);
typedef const char *(WINAPI *PFN_WGL_GetExtensionsStringEXT)(void);
typedef BOOL(WINAPI *PFN_WGL_SwapIntervalEXT)(int nInterval);
typedef int(WINAPI *PFN_WGL_GetSwapIntervalEXT)(void);
typedef HGLRC(WINAPI *PFN_WGL_CreateContextAttribsARB)(HDC hdc, HGLRC hShareContext,
                                                       const int *attribList);
typedef BOOL(WINAPI *PFN_WGL_MakeContextCurrentARB)(HDC hDrawDC, HDC hReadDC, HGLRC hglrc);
typedef HDC(WINAPI *PFN_WGL_GetCurrentReadDCARB)(void);
typedef PROC(WINAPI *PFN_WGL_GetDefaultProcAddress)(LPCSTR lpszProc);

/** 一次测试用到的全部扩展入口，取不到（NULL）表示该扩展不可用。 */
struct SWGLEntry
{
    PFN_WGL_GetExtensionsStringARB pfnExtARB;
    PFN_WGL_GetExtensionsStringEXT pfnExtEXT;
    PFN_WGL_SwapIntervalEXT pfnSetInterval;
    PFN_WGL_GetSwapIntervalEXT pfnGetInterval;
    PFN_WGL_CreateContextAttribsARB pfnCreateAttribs;
    PFN_WGL_MakeContextCurrentARB pfnMakeCurrentRead;
    PFN_WGL_GetCurrentReadDCARB pfnGetReadDC;
};

/** 报告里的三种状态 */
enum
{
    GLT_PASS = 0,
    GLT_FAIL = 1,
    GLT_SKIP = 2,
};

/**
 * @brief 测试结果收集器：一边拼报告一边计数。
 */
class CTestReport
{
  public:
    explicit CTestReport(SStringT &strOut) : m_strOut(strOut), m_nPass(0), m_nFail(0), m_nSkip(0)
    {
    }

    /** 一个分组标题 */
    void Group(LPCTSTR pszTitle)
    {
        m_strOut.AppendFormat(_T("\n%s\n"), pszTitle);
    }

    /** 一条不计分的说明（原始返回值、扩展串等） */
    void Info(LPCTSTR pszText)
    {
        m_strOut.AppendFormat(_T("       %s\n"), pszText ? pszText : _T(""));
    }

    /** 一条断言；bOK 为 FALSE 记 FAIL */
    void Check(LPCTSTR pszName, BOOL bOK, LPCTSTR pszDetail)
    {
        Add(bOK ? GLT_PASS : GLT_FAIL, pszName, pszDetail);
    }

    /** 一条跳过（平台/驱动没这个能力，不算失败） */
    void Skip(LPCTSTR pszName, LPCTSTR pszDetail)
    {
        Add(GLT_SKIP, pszName, pszDetail);
    }

    int Pass() const
    {
        return m_nPass;
    }
    int Fail() const
    {
        return m_nFail;
    }
    int Skip() const
    {
        return m_nSkip;
    }

  private:
    void Add(int nStatus, LPCTSTR pszName, LPCTSTR pszDetail)
    {
        LPCTSTR pszTag = _T("[OK]  ");
        if (nStatus == GLT_FAIL)
        {
            pszTag = _T("[FAIL]");
            m_nFail++;
        }
        else if (nStatus == GLT_SKIP)
        {
            pszTag = _T("[SKIP]");
            m_nSkip++;
        }
        else
        {
            m_nPass++;
        }
        m_strOut.AppendFormat(_T("  %s %s: %s\n"), pszTag, pszName ? pszName : _T(""),
                             pszDetail ? pszDetail : _T(""));
    }

    SStringT &m_strOut;
    int m_nPass;
    int m_nFail;
    int m_nSkip;
};

/** 把之前遗留的 GL 错误清掉，避免干扰后面的判定。 */
static void ClearGLErrors()
{
    for (int i = 0; i < 16 && glGetError() != GL_NO_ERROR; ++i)
    {
    }
}

/**
 * @brief 取 wglGetDefaultProcAddress 的地址。
 *
 * @details 它由 opengl32.dll 导出，但系统头里没有原型（wingdi.h 只声明 wglGetProcAddress），
 *          所以 Windows 上统一走 GetProcAddress 取址，避免和 SDK/编译器各自的声明打架；
 *          macOS/Linux 由 swinx 的 wgl.h 直接声明，取地址即可。
 */
static PFN_WGL_GetDefaultProcAddress GetDefaultProcAddressFn()
{
#ifdef _WIN32
    static PFN_WGL_GetDefaultProcAddress s_pfn = NULL;
    static BOOL s_bInit = FALSE;
    if (!s_bInit)
    {
        s_bInit = TRUE;
        HMODULE hGL = ::GetModuleHandleA("opengl32.dll");
        if (hGL)
            s_pfn = (PFN_WGL_GetDefaultProcAddress)(void *)::GetProcAddress(hGL,
                                                                           "wglGetDefaultProcAddress");
    }
    return s_pfn;
#else
    return &wglGetDefaultProcAddress;
#endif
}

/** 降级入口的"是否符合预期"：swinx 上必须失败，Windows 上不设限。 */
static BOOL DegradedMatches(BOOL bFailed)
{
    return GLWGL_STRICT_SPEC ? bFailed : TRUE;
}

static void LoadWGLEntries(SWGLEntry &e)
{
    e.pfnExtARB = (PFN_WGL_GetExtensionsStringARB)::wglGetProcAddress("wglGetExtensionsStringARB");
    e.pfnExtEXT = (PFN_WGL_GetExtensionsStringEXT)::wglGetProcAddress("wglGetExtensionsStringEXT");
    e.pfnSetInterval = (PFN_WGL_SwapIntervalEXT)::wglGetProcAddress("wglSwapIntervalEXT");
    e.pfnGetInterval = (PFN_WGL_GetSwapIntervalEXT)::wglGetProcAddress("wglGetSwapIntervalEXT");
    e.pfnCreateAttribs =
        (PFN_WGL_CreateContextAttribsARB)::wglGetProcAddress("wglCreateContextAttribsARB");
    e.pfnMakeCurrentRead =
        (PFN_WGL_MakeContextCurrentARB)::wglGetProcAddress("wglMakeContextCurrentARB");
    e.pfnGetReadDC = (PFN_WGL_GetCurrentReadDCARB)::wglGetProcAddress("wglGetCurrentReadDCARB");
}

///////////////////////////////////////////////////////////////////////////////
// wglGetExtensionsStringARB / wglGetExtensionsStringEXT
///////////////////////////////////////////////////////////////////////////////
static void TestExtensionsString(CTestReport &r, const SWGLEntry &e, HDC hDC)
{
    BOOL bHasEntry = (e.pfnExtARB != NULL || e.pfnExtEXT != NULL);
    r.Check(_T("wglGetExtensionsStringARB/EXT 取址"), bHasEntry,
            !bHasEntry ? _T("两个入口都返回 NULL")
                       : (e.pfnExtARB ? _T("ARB 与 EXT 都可取址") : _T("仅 EXT 可取址")));

    const char *pszExt = NULL;
    if (e.pfnExtARB)
        pszExt = e.pfnExtARB(hDC);
    if (!pszExt && e.pfnExtEXT)
        pszExt = e.pfnExtEXT();
    if (!pszExt)
    {
        r.Check(_T("wglGetExtensionsString 内容"), FALSE, _T("返回 NULL"));
        return;
    }

    // 扩展串必须自洽：声明了某扩展就应该能取到它的入口，反之亦然
    BOOL bDeclareArbExt = (strstr(pszExt, "WGL_ARB_extensions_string") != NULL);
    BOOL bDeclareSwap = (strstr(pszExt, "WGL_EXT_swap_control") != NULL);
    SStringT strDetail;
    strDetail.Format(_T("串内 ARB_extensions_string=%d / EXT_swap_control=%d，取址 ARB=%d / swap=%d"),
                     bDeclareArbExt ? 1 : 0, bDeclareSwap ? 1 : 0, e.pfnExtARB ? 1 : 0,
                     e.pfnSetInterval ? 1 : 0);
    r.Check(_T("扩展串与取址一致"),
            bDeclareArbExt == (e.pfnExtARB != NULL) && bDeclareSwap == (e.pfnSetInterval != NULL),
            strDetail.c_str());

    SStringT strExt = S_CA2T(pszExt);
    if (strExt.GetLength() > 150)
        strExt = strExt.Left(150) + _T("...");
    r.Info(strExt.c_str());
}

///////////////////////////////////////////////////////////////////////////////
// wglSwapIntervalEXT / wglGetSwapIntervalEXT
///////////////////////////////////////////////////////////////////////////////
static void TestSwapControl(CTestReport &r, const SWGLEntry &e)
{
    if (!e.pfnSetInterval || !e.pfnGetInterval)
    {
        r.Skip(_T("wglSwapIntervalEXT/Get"), _T("驱动/WGL 层未提供 WGL_EXT_swap_control"));
        return;
    }

    int nOld = e.pfnGetInterval();
    BOOL bSet = e.pfnSetInterval(1);
    int nNow = e.pfnGetInterval();
    BOOL bNegative = e.pfnSetInterval(-1); // 负间隔非法，必须被拒
    int nAfterNegative = e.pfnGetInterval();
    BOOL bRestore = e.pfnSetInterval(nOld);

    SStringT str;
    str.Format(_T("原值=%d, Set(1) 返回 %d, 回读=%d"), nOld, bSet ? 1 : 0, nNow);
    r.Check(_T("wglSwapIntervalEXT(1) 生效"), bSet && nNow == 1, str.c_str());

    // 负间隔按 WGL_EXT_swap_control 是非法值。swinx 会明确拒绝；Windows 驱动不一定校验
    // （Intel 实测直接接受并返回 TRUE），所以只把差异记成 SKIP，不算失败。
    SStringT strNeg;
    strNeg.Format(_T("Set(-1) 返回 %d, 回读=%d（合法实现应保持 1）"), bNegative ? 1 : 0,
                  nAfterNegative);
    if (bNegative && !GLWGL_STRICT_SPEC)
        r.Skip(_T("wglSwapIntervalEXT(-1) 被拒"), strNeg.c_str());
    else
        r.Check(_T("wglSwapIntervalEXT(-1) 被拒"), !bNegative && nAfterNegative == 1, strNeg.c_str());

    str.Format(_T("恢复为 %d: 返回 %d"), nOld, bRestore ? 1 : 0);
    r.Check(_T("wglSwapIntervalEXT 恢复原值"), bRestore, str.c_str());
}

///////////////////////////////////////////////////////////////////////////////
// wglMakeContextCurrentARB / wglGetCurrentReadDCARB
///////////////////////////////////////////////////////////////////////////////
static void TestMakeCurrentRead(CTestReport &r, const SWGLEntry &e, HDC hDC, HGLRC hRC)
{
    if (!e.pfnMakeCurrentRead || !e.pfnGetReadDC)
    {
        r.Skip(_T("wglMakeContextCurrentARB"), _T("未提供 WGL_ARB_make_current_read"));
        return;
    }

    // 读写同一个窗口：两个平台都只有一个 drawable，这是唯一的可用组合
    BOOL bOK = e.pfnMakeCurrentRead(hDC, hDC, hRC);
    HDC hReadDC = e.pfnGetReadDC();
    BOOL bCtxUnchanged = (::wglGetCurrentContext() == hRC);

    SStringT str;
    str.Format(_T("返回=%d, 读 DC %s, 当前上下文%s"), bOK ? 1 : 0,
               hReadDC == hDC ? _T("就是窗口 DC") : _T("与窗口 DC 不同"),
               bCtxUnchanged ? _T("未变") : _T("被改"));
    r.Check(_T("wglMakeContextCurrentARB(读写同窗口)"), bOK && hReadDC == hDC && bCtxUnchanged,
            str.c_str());
}

///////////////////////////////////////////////////////////////////////////////
// wglCreateContextAttribsARB
///////////////////////////////////////////////////////////////////////////////
static void TestCreateContextAttribs(CTestReport &r, const SWGLEntry &e, HDC hDC, HGLRC hRC)
{
    if (!e.pfnCreateAttribs)
    {
        r.Skip(_T("wglCreateContextAttribsARB"), _T("未提供 WGL_ARB_create_context"));
        return;
    }

    // 正向：3.2 core。macOS 桌面 GL 只提供 3.2/4.1 core，Windows/Linux 驱动的下限也低于它，
    // 所以这是"应当成功"的最低版本；真的不支持时按 SKIP 处理，不冤枉驱动。
    const int attrs32[] = {WGL_CONTEXT_MAJOR_VERSION_ARB, 3,
                           WGL_CONTEXT_MINOR_VERSION_ARB, 2,
                           WGL_CONTEXT_PROFILE_MASK_ARB, WGL_CONTEXT_CORE_PROFILE_BIT_ARB,
                           0};
    HGLRC hModern = e.pfnCreateAttribs(hDC, NULL, attrs32);
    if (!hModern)
    {
        r.Skip(_T("wglCreateContextAttribsARB(3.2 core)"), _T("返回 NULL（驱动不支持 3.2 core）"));
    }
    else
    {
        BOOL bCurrent = ::wglMakeCurrent(hDC, hModern);
        int nMajor = 0, nMinor = 0;
        if (bCurrent)
        {
            const char *pszVer = (const char *)glGetString(GL_VERSION);
            if (pszVer)
                sscanf(pszVer, "%d.%d", &nMajor, &nMinor);
        }
        ::wglMakeCurrent(hDC, hRC); // 先恢复主上下文再销毁
        ::wglDeleteContext(hModern);

        SStringT str;
        str.Format(_T("MakeCurrent=%d, 新上下文 GL_VERSION=%d.%d"), bCurrent ? 1 : 0, nMajor, nMinor);
        r.Check(_T("wglCreateContextAttribsARB(3.2 core)"),
                bCurrent && nMajor * 10 + nMinor >= 32, str.c_str());
    }

    // 反向：不存在的版本必须明确失败，不能悄悄降级成 2.1 让调用方以为拿到了新上下文
    const int attrsBad[] = {WGL_CONTEXT_MAJOR_VERSION_ARB, 9,
                            WGL_CONTEXT_MINOR_VERSION_ARB, 9,
                            WGL_CONTEXT_PROFILE_MASK_ARB, WGL_CONTEXT_CORE_PROFILE_BIT_ARB,
                            0};
    HGLRC hBad = e.pfnCreateAttribs(hDC, NULL, attrsBad);
    r.Check(_T("wglCreateContextAttribsARB(9.9) 被拒"), hBad == NULL,
            hBad ? _T("返回非 NULL（悄悄降级了）") : _T("返回 NULL"));
    if (hBad)
        ::wglDeleteContext(hBad);

    // 反向：core 与 compatibility 同时置位是 WGL_ARB_create_context 规定的非法组合
    const int attrsMix[] = {WGL_CONTEXT_MAJOR_VERSION_ARB, 3,
                           WGL_CONTEXT_MINOR_VERSION_ARB, 2,
                           WGL_CONTEXT_PROFILE_MASK_ARB,
                           WGL_CONTEXT_CORE_PROFILE_BIT_ARB | WGL_CONTEXT_COMPATIBILITY_PROFILE_BIT_ARB,
                           0};
    HGLRC hMix = e.pfnCreateAttribs(hDC, NULL, attrsMix);
    r.Check(_T("core+compatibility 同置被拒"), hMix == NULL,
            hMix ? _T("返回非 NULL（非法组合被接受）") : _T("返回 NULL"));
    if (hMix)
        ::wglDeleteContext(hMix);
}

///////////////////////////////////////////////////////////////////////////////
// wglShareLists：靠"源上下文里建的纹理在目标上下文里能不能按名字查到"验证
///////////////////////////////////////////////////////////////////////////////
static void TestShareLists(CTestReport &r, HDC hDC, HGLRC hRC)
{
    HGLRC hSrc = ::wglCreateContext(hDC);
    HGLRC hDst = ::wglCreateContext(hDC);
    if (!hSrc || !hDst)
    {
        if (hSrc)
            ::wglDeleteContext(hSrc);
        if (hDst)
            ::wglDeleteContext(hDst);
        r.Check(_T("wglShareLists"), FALSE, _T("辅助上下文创建失败"));
        return;
    }

    // 必须在两个上下文都还没成为当前上下文之前调用（WGL 与 swinx 的实现都如此）
    BOOL bShare = ::wglShareLists(hSrc, hDst);
    if (!bShare)
    {
        ::wglDeleteContext(hDst);
        ::wglDeleteContext(hSrc);
        r.Check(_T("wglShareLists"), FALSE, _T("返回 FALSE"));
        return;
    }

    GLuint nTex = 0;
    BOOL bInSrc = FALSE, bInDst = FALSE;
    if (::wglMakeCurrent(hDC, hSrc))
    {
        ClearGLErrors();
        glGenTextures(1, &nTex);
        glBindTexture(GL_TEXTURE_2D, nTex);
        bInSrc = (glIsTexture(nTex) == GL_TRUE);
    }
    if (nTex && ::wglMakeCurrent(hDC, hDst))
    {
        // 不重新绑定：共享时这个名字在目标上下文的对象空间里本来就存在
        ClearGLErrors();
        bInDst = (glIsTexture(nTex) == GL_TRUE);
    }
    if (nTex)
    {
        ::wglMakeCurrent(hDC, hSrc);
        glDeleteTextures(1, &nTex);
    }
    ::wglMakeCurrent(hDC, hRC);
    ::wglDeleteContext(hDst);
    ::wglDeleteContext(hSrc);

    SStringT str;
    str.Format(_T("纹理名 %u：源上下文可见=%d，共享上下文可见=%d"), (UINT)nTex, bInSrc ? 1 : 0,
               bInDst ? 1 : 0);
    r.Check(_T("wglShareLists + 纹理共享"), bInSrc && bInDst, str.c_str());
}

///////////////////////////////////////////////////////////////////////////////
// wglSwapMultipleBuffers
///////////////////////////////////////////////////////////////////////////////
static void TestSwapMultipleBuffers(CTestReport &r, HDC hDC, HGLRC hRC)
{
    // 空参数是明确的边界：批量接口应当返回 0 而不是崩溃
    DWORD dwEmpty = ::wglSwapMultipleBuffers(0, NULL);
    SStringT str;
    str.Format(_T("返回 %u"), (UINT)dwEmpty);
    r.Check(_T("wglSwapMultipleBuffers(0, NULL)"), dwEmpty == 0, str.c_str());

    // 真实交换一项：返回值 MS 没有文档化（Windows 实测与 swinx 一样返回"成功交换的个数"），
    // 所以只校验"调用后当前上下文/GL 错误都没被动过"，不校验具体数值。
    WGLSWAP swap;
    memset(&swap, 0, sizeof(swap));
    swap.hdc = hDC;
    ClearGLErrors();
    DWORD dwRet = ::wglSwapMultipleBuffers(1, &swap);
    BOOL bCtxUnchanged = (::wglGetCurrentContext() == hRC);
    GLenum nErr = glGetError();
    str.Format(_T("返回 %u, 当前上下文%s, glGetError=0x%x"), (UINT)dwRet,
               bCtxUnchanged ? _T("未变") : _T("被改"), (UINT)nErr);
    r.Check(_T("wglSwapMultipleBuffers(1, {窗口 DC})"), bCtxUnchanged && nErr == GL_NO_ERROR,
            str.c_str());
}

///////////////////////////////////////////////////////////////////////////////
// wglGetDefaultProcAddress
///////////////////////////////////////////////////////////////////////////////
static void TestDefaultProcAddress(CTestReport &r)
{
    PFN_WGL_GetDefaultProcAddress pfn = GetDefaultProcAddressFn();
    if (!pfn)
    {
        r.Skip(_T("wglGetDefaultProcAddress"), _T("该平台没有导出这个入口"));
        return;
    }
    // 未知名字必须返回 NULL（Windows 上它本来就是查 opengl32 导出表，查不到就是 NULL）
    PROC pfnUnknown = pfn("wglNoSuchEntry_SOUI");
    r.Check(_T("wglGetDefaultProcAddress(未知名字)"), pfnUnknown == NULL,
            pfnUnknown ? _T("返回了非 NULL") : _T("返回 NULL"));
}

///////////////////////////////////////////////////////////////////////////////
// 降级入口：图层平面 + GDI 字形 + 上下文拷贝
///////////////////////////////////////////////////////////////////////////////
static void TestDegradedEntries(CTestReport &r, HDC hDC, HGLRC hRC)
{
    // ---- 图层平面（Windows 专有） ----
    HGLRC hLayer = ::wglCreateLayerContext(hDC, 1);
    LAYERPLANEDESCRIPTOR lpd;
    memset(&lpd, 0, sizeof(lpd));
    lpd.nSize = sizeof(lpd);
    lpd.nVersion = 1;
    BOOL bDescribe = ::wglDescribeLayerPlane(hDC, 1, 1, sizeof(lpd), &lpd);
    COLORREF crPalette[1] = {0};
    int nSetPal = ::wglSetLayerPaletteEntries(hDC, 1, 0, 1, crPalette);
    int nGetPal = ::wglGetLayerPaletteEntries(hDC, 1, 0, 1, crPalette);
    BOOL bRealize = ::wglRealizeLayerPalette(hDC, 1, FALSE);
    // 平面掩码用文档里的 WGL_SWAP_MAIN_PLANE（不是 PFD_ 的那组标志）
    BOOL bSwapLayer = ::wglSwapLayerBuffers(hDC, WGL_SWAP_MAIN_PLANE);
    BOOL bLayerFailed =
        (hLayer == NULL) && !bDescribe && nSetPal == 0 && nGetPal == 0 && !bRealize && !bSwapLayer;

    SStringT str;
    str.Format(_T("createLayer=%d describe=%d setPal=%d getPal=%d realize=%d swapLayer=%d"),
               hLayer ? 1 : 0, bDescribe ? 1 : 0, nSetPal, nGetPal, bRealize ? 1 : 0,
               bSwapLayer ? 1 : 0);
    r.Check(_T("图层平面入口(6 个)"), DegradedMatches(bLayerFailed), str.c_str());
    if (hLayer)
        ::wglDeleteContext(hLayer);

    // ---- 上下文拷贝 ----
    BOOL bCopy = ::wglCopyContext(hRC, hRC, GL_ALL_ATTRIB_BITS);
    r.Check(_T("wglCopyContext"), DegradedMatches(!bCopy),
            bCopy ? _T("返回 TRUE") : _T("返回 FALSE"));

    // ---- GDI 字形：swinx 上一律失败；Windows 取决于 DC 里有没有 TrueType 字体，
    //      实测 wglUseFontBitmaps 成功、wglUseFontOutlines 失败（都不影响判定） ----
    BOOL bBitmapsA = ::wglUseFontBitmapsA(hDC, 0, 1, 1000);
    BOOL bBitmapsW = ::wglUseFontBitmapsW(hDC, 0, 1, 1000);
    BOOL bOutlineA = ::wglUseFontOutlinesA(hDC, 0, 1, 1000, 0.0f, 0.0f, WGL_FONT_LINES, NULL);
    BOOL bOutlineW = ::wglUseFontOutlinesW(hDC, 0, 1, 1000, 0.0f, 0.0f, WGL_FONT_LINES, NULL);
    BOOL bFontFailed = !bBitmapsA && !bBitmapsW && !bOutlineA && !bOutlineW;
    str.Format(_T("bitmapsA=%d bitmapsW=%d outlinesA=%d outlinesW=%d"), bBitmapsA ? 1 : 0,
               bBitmapsW ? 1 : 0, bOutlineA ? 1 : 0, bOutlineW ? 1 : 0);
    r.Check(_T("GDI 字形入口(4 个)"), DegradedMatches(bFontFailed), str.c_str());
}

///////////////////////////////////////////////////////////////////////////////
SStringT CGLWnd::RunApiTests(int *pnPass, int *pnFail, int *pnSkip)
{
    SStringT strOut;
    if (pnPass)
        *pnPass = 0;
    if (pnFail)
        *pnFail = 0;
    if (pnSkip)
        *pnSkip = 0;

    if (!m_hDC || !m_hRC)
    {
        strOut = _T("跳过：OpenGL 上下文未创建，无法测试 WGL 入口\n");
        if (pnSkip)
            *pnSkip = 1;
        return strOut;
    }
    if (!::wglMakeCurrent(m_hDC, m_hRC))
    {
        strOut = _T("失败：wglMakeCurrent 失败，无法测试 WGL 入口\n");
        if (pnFail)
            *pnFail = 1;
        return strOut;
    }

    ClearGLErrors();
    SWGLEntry entry;
    LoadWGLEntries(entry);

    strOut = _T("WGL 入口契约测试（扩展 / 降级入口）\n");
    CTestReport report(strOut);
    report.Group(_T("[WGL 扩展]"));
    TestExtensionsString(report, entry, m_hDC);
    TestSwapControl(report, entry);
    TestMakeCurrentRead(report, entry, m_hDC, m_hRC);
    TestCreateContextAttribs(report, entry, m_hDC, m_hRC);
    TestShareLists(report, m_hDC, m_hRC);
    TestSwapMultipleBuffers(report, m_hDC, m_hRC);
    TestDefaultProcAddress(report);

    report.Group(_T("[降级入口]"));
    report.Info(GLWGL_DEGRADED_NOTE);
    TestDegradedEntries(report, m_hDC, m_hRC);

    // 收尾：以上测试会临时切换上下文，这里确认线程级状态已经回到主上下文
    report.Group(_T("[收尾]"));
    BOOL bStateOK = (::wglGetCurrentContext() == m_hRC) && (::wglGetCurrentDC() == m_hDC);
    SStringT strState;
    strState.Format(_T("当前上下文=%s，当前 DC=%s"),
                    ::wglGetCurrentContext() == m_hRC ? _T("主上下文") : _T("已被改变"),
                    ::wglGetCurrentDC() == m_hDC ? _T("主窗口 DC") : _T("已被改变"));
    report.Check(_T("线程级状态未被测试改变"), bStateOK, strState.c_str());
    if (!bStateOK)
        ::wglMakeCurrent(m_hDC, m_hRC);
    ClearGLErrors();

    if (pnPass)
        *pnPass = report.Pass();
    if (pnFail)
        *pnFail = report.Fail();
    if (pnSkip)
        *pnSkip = report.Skip();

    strOut.AppendFormat(_T("\n汇总: PASS %d / FAIL %d / SKIP %d\n"), report.Pass(), report.Fail(),
                        report.Skip());

    Invalidate(FALSE); // 测试期间动过当前上下文，让画面重画一帧
    return strOut;
}
