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
        STDMETHOD_(BOOL, OnFireEvent)(IEvtArgs *evt) OVERRIDE
        {
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

        void OnTimelineRequestChanged(BOOL bHasTimelineRequest) override {}
    };

    struct HeadlessApp
    {
        SComMgr2 comMgr;
        SAutoRefPtr<IRenderFactory> renderFactory;
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
