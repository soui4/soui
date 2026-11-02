/**
 * @file       test_container.h
 * @brief      Shared headless harness for SOUI fun_test cases.
 *
 * SwndContainerImpl is deliberately abstract: drawing / host integration pure
 * virtuals (OnRedraw, UpdateWindow, GetMsgLoop, ...) are implemented by real
 * hosts such as SHostWnd. TestContainer stubs them out: redraws are no-ops and
 * posted tasks run inline so the tests stay deterministic. Together with
 * HeadlessApp it allows SPanel-derived controls to be created and receive
 * simulated messages without a host window.
 */

#ifndef __FUN_TEST_CONTAINER_H__
#define __FUN_TEST_CONTAINER_H__

#include <souistd.h>
#include <core/SWnd.h>
#include <core/SWndContainerImpl.h>
#include <res.mgr/SUiDef.h>
#include <commgr2.h>
#include <SouiFactory.h>
// 非 Windows 的 zip 系统资源加载需要 ZIPRES_PARAM/ZipFile（inline-safe，可多处包含）。
// 不能用 <SAppCfg.h>：它把 SAppCfg 成员函数直接定义在头里且未加 inline，是单 TU 头，
// 被多个测试 TU 包含会触发 ODR 重复定义链接错误。
#include <resprovider-zip/zipresprovider-param.h>
#include <gtest/gtest.h>
#include "common.h"

#ifdef _WIN32
#define WCHAR_ENC enc_utf16
#else
#define WCHAR_ENC enc_utf32
#endif

namespace funtest
{
    using namespace SOUI;

    class TestContainer : public SwndContainerImpl
    {
	public:
		int m_nFiredEvents;
		DWORD m_nLastEventId;
	public:
		TestContainer():m_nFiredEvents(0),m_nLastEventId(0){
		  }
        /**
         * @brief 容器层收到的事件计数。
         *
         * SWindow::FireEvent 的派发链是「本窗口事件槽 -> 脚本处理器 -> owner 窗口 ->
         * 容器 OnFireEvent」。只有当事件仍处于 bubbleUp 状态时才会走到这里，因此
         * 这两个计数可以用来验证"冒泡被终止后容器收不到事件"这类语义。
         * 仅用于测试观测，不改变容器行为（仍然返回 FALSE = 未处理）。
         */

        void ResetEventCounter()
        {
            m_nFiredEvents = 0;
            m_nLastEventId = 0;
        }

        STDMETHOD_(BOOL, OnFireEvent)(IEvtArgs *evt) OVERRIDE
        {
            ++m_nFiredEvents;
            m_nLastEventId = evt ? evt->GetID() : 0;
            return FALSE;
        }

        STDMETHOD_(IMessageLoop *, GetMsgLoop)(THIS) OVERRIDE
        {
            return NULL;
        }

        STDMETHOD_(IScriptModule *, GetScriptModule)() OVERRIDE
        {
            return NULL;
        }

        STDMETHOD_(HWND, GetHostHwnd)(CTHIS) SCONST OVERRIDE
        {
            return NULL;
        }

        STDMETHOD_(BOOL, IsTranslucent)(CTHIS) SCONST OVERRIDE
        {
            return FALSE;
        }

        STDMETHOD_(BOOL, IsSendWheel2Hover)(CTHIS) SCONST OVERRIDE
        {
            return FALSE;
        }

        STDMETHOD_(int, GetScale)(CTHIS) SCONST OVERRIDE
        {
            return 100;
        }

        STDMETHOD_(LPCWSTR, GetTranslatorContext)(CTHIS) SCONST OVERRIDE
        {
            return NULL;
        }

        STDMETHOD_(void, GetContainerRect)(CTHIS_ RECT *ret) SCONST OVERRIDE
        {
            ret->left = 0;
            ret->top = 0;
            ret->right = 4096;
            ret->bottom = 4096;
        }

        STDMETHOD_(void, UpdateRegion)(THIS_ IRegionS *rgn) OVERRIDE
        {
        }

        STDMETHOD_(void, OnRedraw)(THIS_ LPCRECT rc, BOOL bClip) OVERRIDE
        {
        }

        STDMETHOD_(BOOL, UpdateWindow)(THIS_ BOOL bForce DEF_VAL(TRUE)) OVERRIDE
        {
            return TRUE;
        }

        STDMETHOD_(void, UpdateTooltip)(THIS) OVERRIDE
        {
        }

        STDMETHOD_(void, SetToolTip)(THIS_ LPCRECT rc, UINT tipAlign, LPCTSTR pszTip) OVERRIDE
        {
        }

        STDMETHOD_(void, EnableIME)(THIS_ BOOL bEnable) OVERRIDE
        {
        }

        STDMETHOD_(void, OnUpdateCursor)(THIS) OVERRIDE
        {
        }

        STDMETHOD_(void, EnableHostPrivateUiDef)(THIS_ BOOL bEnable) OVERRIDE
        {
        }

        STDMETHOD_(BOOL, PostTask)(THIS_ IRunnable *runable, BOOL bAsync DEF_VAL(TRUE)) OVERRIDE
        {
            // Headless: no message loop to post to - run inline for determinism.
            if (runable)
                runable->run();
            return TRUE;
        }

        STDMETHOD_(int, RemoveTasksForObject)(THIS_ void *pObj) OVERRIDE
        {
            return 0;
        }

        STDMETHOD_(void, OnDropdownState)(THIS_ IHostWnd *pDropdown, BOOL bCreate) OVERRIDE
        {
        }

        void OnTimelineRequestChanged(BOOL bHasTimelineRequest) {}
    };

    struct HeadlessApp
    {
        SComMgr2 comMgr;
        SAutoRefPtr<IRenderFactory> renderFactory;
        SAutoRefPtr<IImgDecoderFactory> imgDecoderFactory; // 仅 LoadSystemResource 时惰性创建
        SApplication *app;
        SAutoRefPtr<IResProvider> provider;

        /**
         * @brief Create the headless SApplication.
         * @param pszResDir optional resource directory relative to the source
         *                  dir (e.g. _T("/e2e-res")); registered as a FILE
         *                  res provider when non-NULL.
         */
        explicit HeadlessApp(const TCHAR *pszResDir = NULL)
            : app(NULL)
        {
            EXPECT_TRUE(comMgr.CreateRender_GDI((IObjRef **)&renderFactory));
            app = new SApplication(renderFactory, NULL);

            // SPanel::OnCreate requires a builtin scrollbar skin. Inject a bare
            // SSkinScrollbar (no image is needed for headless tests).
            static const wchar_t kSkinXml[] = L"<skin><scrollbar name=\"_skin.sys.scrollbar\"/></skin>";
            SXmlDoc skinDoc;
            // ASSERT_* cannot be used inside a constructor (it returns); EXPECT is fine.
            EXPECT_TRUE(skinDoc.load_buffer(kSkinXml, sizeof(kSkinXml) - sizeof(wchar_t), xml_parse_default, WCHAR_ENC));
            SXmlNode skinNode = skinDoc.root().child(L"skin");
            EXPECT_TRUE(skinNode);
            if (skinNode)
                GETUIDEF->GetBuiltinSkinPool()->LoadSkins(&skinNode);

            if (pszResDir)
            {
                SouiFactory factory;
                provider = factory.CreateResProvider(RES_FILE);
                SStringT resourceDir = getSourceDir() + pszResDir;
                EXPECT_TRUE(provider->Init((LPARAM)resourceDir.c_str(), 0));
                app->AddResProvider(provider, NULL);
                provider->Release();
            }
        }

        /**
         * @brief 按需把系统资源 soui-sys-resource 加载进当前无头 SApplication。
         *
         * 系统资源内含全部内置皮肤（_skin.sys.scrollbar / _skin.sys.dropbtn /
         * _skin.sys.tab.page / _skin.sys.tab.header ...），SComboBox / STabCtrl 等容器控件
         * 在 OnCreate 阶段需要它们，否则创建失败（如 SComboBase::OnCreate 取不到
         * SKIN_SYS_DROPBTN 就返回 -1）。
         *
         * 这是【按需】接口：默认不加载，保持 HeadlessApp 极简（只有 scrollbar），
         * 以免改变 check_sync / rubberband 等既有用例刻意依赖的固定几何度量。
         * 需要完整内置皮肤的用例（SouiTest 夹具）显式调用即可。会顺带惰性创建
         * 图片解码器（系统资源皮肤是 PNG），并作为成员持有以维持其生命周期。
         */
        void LoadSystemResource()
        {
            if (!imgDecoderFactory)
            {
                // COM_IMGDECODER 默认 imgdecoder-stb，随 bin/ 下的 libimgdecoder-stb.dll 分发。
                if (comMgr.CreateImgDecoder((IObjRef **)&imgDecoderFactory) && imgDecoderFactory)
                    renderFactory->SetImgDecoderFactory(imgDecoderFactory);
            }
#ifdef _WIN32
            // Windows：系统资源为 PE(DLL)，与 fun_test.exe 同目录，LoadLibrary 后按 RES_PE 读。
            HMODULE hSysRes = ::LoadLibrary(_T("soui-sys-resource"));
            EXPECT_TRUE(hSysRes != NULL);
            if (!hSysRes)
                return;
            SouiFactory souiFac;
            SAutoRefPtr<IResProvider> sysRes;
            sysRes.Attach(souiFac.CreateResProvider(RES_PE));
            if (sysRes && sysRes->Init((WPARAM)hSysRes, 0))
                app->LoadSystemNamedResource(sysRes);
#else
            // 非 Windows：系统资源以 zip 分发（仓库根 soui-sys-resource.zip）。
            // 该分支未在本环境（Windows）实测，若各平台构建布局不同需调整 zip 路径。
            SAutoRefPtr<IResProvider> sysRes;
            if (comMgr.CreateResProvider_ZIP((IObjRef **)&sysRes))
            {
                ZIPRES_PARAM param;
                SStringT zipPath = getSourceDir() + _T("/../../soui-sys-resource.zip");
                ZipFile(&param, renderFactory, zipPath, NULL);
                if (sysRes->Init((WPARAM)&param, 0))
                    app->LoadSystemNamedResource(sysRes);
            }
#endif
        }

        ~HeadlessApp()
        {
            delete app;
        }
    };

    /**
     * @brief Load a layout from an in-memory XML string into a container-backed
     *        root window.
     * @note  SetContainer must happen before InitFromXml so events fired during
     *        creation reach subscribers (FireEvent and SetFocus rely on the
     *        container being set).
     */
    inline BOOL CreateRootFromXml(SWindow &root, ISwndContainer *pContainer, const wchar_t *pszXml)
    {
        SXmlDoc doc;
        if (!doc.load_buffer(pszXml, (size_t)wcslen(pszXml) * sizeof(wchar_t), xml_parse_default, WCHAR_ENC))
            return FALSE;
        SXmlNode nodeRoot = doc.root().first_child();
        if (!nodeRoot)
            return FALSE;
        root.SetContainer(pContainer);
        return root.InitFromXml(&nodeRoot);
    }
}

using funtest::TestContainer;
using funtest::HeadlessApp;
using funtest::CreateRootFromXml;

#endif /**< __FUN_TEST_CONTAINER_H__ */
