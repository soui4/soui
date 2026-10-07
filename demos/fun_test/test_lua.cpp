/**
 * test_lua.cpp —— script_lua 模块导出 SOUI 对象的创建/释放调试测试。
 *
 * 设计约束（按需求）：
 *   1. 测试 TU 不链接 lua 库、不 include 任何 lua 头：对象全部由脚本创建，
 *      脚本执行统一走 IScriptModule::executeScriptBuffer（模块内部 pcall，
 *      出错时 lua_tinker 打印并弹栈、state 仍可用）。
 *   2. 每个被导出的 SOUI 对象类型一个用例，用例只做"创建 + 释放"。
 *   3. C++ 对象有没有被正确释放由人工在调试器里判定：每个用例注释标注了
 *      预期的释放路径与断点位置，跑用例时在对应析构函数上设断点即可。
 *
 * 成败探针：脚本末尾向工作目录写一个 flag 文件；脚本中途出错（对象创建
 * 失败、assert 抛错）则 dobuffer 中止脚本，flag 不会出现 → 用例失败。
 * 这是纯 IScriptModule 接口下唯一的可观测通道（executeScriptBuffer 返回
 * void、executeScriptFile 恒返回 TRUE、executeScriptedEventHandler 会在
 * handler 返回真值时解引用 pEvt——均不可用）。
 *
 * 释放路径分三类（人工分析重点，见各用例注释）：
 *   [GC]    对象经 lua_tinker::push_gcnew / constructor_lstate 入栈（userdata
 *           挂 __gc）：o=nil + collectgarbage → __gc → val2user 析构 → delete
 *           （纯包装对象，不经引用计数）。
 *           【绝不能】在脚本里对这类对象再调 :Release()，否则双释放。
 *   [Rel]   对象经非托管 push<T*> 入栈（无 __gc，GC 永不回收）：必须显式
 *           o:Release()（对象是 IObjRef 系，初始引用=1，脚本独占）。
 *           忘调 Release 就是泄漏 —— 这类用例正是泄漏排查的主战场。
 *   [None]  对象非托管 push 且没有可调的 Release 绑定（纯接口监听器）：
 *           释放无处安放，预期泄漏，需人工确认并决定补救方案。
 *
 * 断点速查（均在 scriptmodule-lua / SOUI 源码内）：
 *   加载   SComMgr2::CreateScrpit_Lua 返回 IScriptFactory 工厂（DLL 分支经
 *          SComLoader/LoadLibrary 调 DLL 导出 SCreateInstance → new
 *          SIScriptFactory；静态 COM 分支直链同名函数）——注意它【不是】
 *          IScriptModule！工厂 CreateScriptModule → new SScriptModule_Lua
 *          （ctor 内 luaL_newstate + SOUI_Export_Lua 注册全部导出）。
 *   创建   LuaValueAnimator()/LuaAnimatorGroup() 等 class_con 构造器
 *          （constructor_lstate）及各对象工厂（CreateStringW / ... ）。
 *   释放   [GC]  collectgarbage → lua_tinker::user::~user → val2user 析构
 *                → delete（push_gcnew / constructor_lstate，不经引用计数）。
 *          [Rel] IObjRef::Release → OnFinalRelease → delete → ~具体类。
 *          [None] 无路径 —— 泄漏。
 *   收尾   TearDown → IScriptModule::Release → ~SScriptModule_Lua →
 *          lua_close（对仍存活对象逐一 __gc）。
 *   app    soui_lua_app 夹具 C++ 侧 new/Release SApplication：
 *          ~SApplication（_DestroySingletons / SResProviderMgr::RemoveAll）。
 */
#include <gtest/gtest.h>
#include <souistd.h>
#include <SouiFactory.h>
#include <commgr2.h>
#include <interface/SScriptModule-i.h>
#include <SApp.h>
#include <string>
#include <stdio.h>

using namespace SNS;

// demo uires 目录（CMake 注入，正斜杠），供 LoadAnimation/LoadValueAnimator 用例
#ifndef SOUI_FUN_TEST_DEMO_UIRES
#define SOUI_FUN_TEST_DEMO_UIRES "demo/uires"
#endif

// ------------------------------------------------------------------------
// 成败探针 helper
// ------------------------------------------------------------------------
static ::testing::AssertionResult RunLua(IScriptModule *mod, const char *tag,
                                         const std::string &body)
{
    std::string flag = std::string("soui_lua_flag_") + tag + ".tmp";
    remove(flag.c_str());
    std::string lua = body;
    lua += "\nlocal __f=io.open(\"" + flag + "\",\"w\"); if __f then __f:write(\"ok\"); __f:close() end\n";
    mod->executeScriptBuffer(lua.c_str(), lua.size());
    FILE *f = fopen(flag.c_str(), "rb");
    if (!f)
        return ::testing::AssertionFailure() << "lua script aborted before finish: " << tag
                                             << " (对象创建/断言失败，详见 stderr 的 lua 错误输出)";
    fclose(f);
    remove(flag.c_str());
    return ::testing::AssertionSuccess();
}

// ------------------------------------------------------------------------
// 夹具：每个用例独立的 scriptmodule-lua 模块实例
// ------------------------------------------------------------------------
class soui_lua : public ::testing::Test {
protected:
    SComMgr2 m_comMgr;      // 先声明：成员按声明逆序析构，保证 m_module 先释放、
                            // SComLoader 后 FreeLibrary（否则 Release 踩已卸载代码）
    IScriptModule *m_module;

    soui_lua() : m_module(NULL) {}

    void SetUp() override
    {
        IObjRef *pObj = NULL;
        ASSERT_TRUE(m_comMgr.CreateScrpit_Lua(&pObj));
        ASSERT_TRUE(pObj != NULL);
        // CreateScrpit_Lua 返回的是 IScriptFactory，不是 IScriptModule！
        IScriptFactory *factory = static_cast<IScriptFactory *>(pObj);
        HRESULT hr = factory->CreateScriptModule(&m_module);
        pObj->Release();
        ASSERT_TRUE(SUCCEEDED(hr));
        ASSERT_TRUE(m_module != NULL);
    }

    void TearDown() override
    {
        if (m_module)
        {
            // 断点：~SScriptModule_Lua → lua_close（对仍存活对象逐一 __gc）
            m_module->Release();
            m_module = NULL;
        }
    }

    ::testing::AssertionResult Run(const char *tag, const std::string &body)
    {
        return RunLua(m_module, tag, body);
    }
};

// ------------------------------------------------------------------------
// 夹具：带 IApplication 的模块（app 由 C++ 侧创建/释放）
// ------------------------------------------------------------------------
class soui_lua_app : public soui_lua {
protected:
    SApplication *m_app;

    soui_lua_app() : m_app(NULL) {}

    void SetUp() override
    {
        // SApplication ctor：注册窗口类 + _InitApp（创建 SUiDef/SWindowMgr/
        // STimerGenerator/SHostMgr 等单例 + SObjectDefaultRegister 注册全部
        // 对象工厂）。无渲染工厂、不建窗口，headless 安全。
        m_app = new SApplication(GetModuleHandle(NULL));
        ASSERT_TRUE(m_app != NULL);
        // 装默认日志（log4z 组件）：GetLogMgr 用例的前提；同时脚本出错时
        // lua_tinker 的 print_error 输出经 SLog 进入日志可见，便于人工分析
        // （不装的话 lua 错误被静默丢弃，只能靠 flag 文件判断成败）。
        ILogMgr *pLog = NULL;
        if (m_comMgr.CreateLog4z((IObjRef**)&pLog) && pLog)
        {
            m_app->SetLogManager(pLog); // 内部 AddRef
            pLog->Release();
        }
        SouiFactory factory;
        IResProvider * m_resProvider =factory.CreateResProvider(RES_FILE);
        ASSERT_TRUE(m_resProvider != NULL);
        SStringT dir = S_CA2T(SOUI_FUN_TEST_DEMO_UIRES, CP_UTF8);
        ASSERT_TRUE(m_resProvider->Init((LPARAM)dir.c_str(), 0));
        m_app->AddResProvider(m_resProvider, NULL);
		m_resProvider->Release();
		soui_lua::SetUp();
    }

    void TearDown() override
    {
		soui_lua::TearDown();
        if (m_app)
        {
            delete m_app;
            m_app = NULL;
        }
    }
};

// ========================================================================
// A 组：不需要 IApplication 的对象（soui_lua 夹具）
// ========================================================================

// [GC] lua_tinker::push_gcnew（val2user 语义）：o=nil + collectgarbage → __gc
//      → delete → ~LuaValueAnimator（摘监听、Release 所持 IValueAnimator）。
//      断点：LuaValueAnimator::~LuaValueAnimator / val2user<T>::~val2user
TEST_F(soui_lua, lua_value_animator_gc_release)
{
    EXPECT_TRUE(Run("lva",
        "local o = LuaValueAnimator()\n"
        "assert(o ~= nil)\n"
        "o = nil\n"
        "collectgarbage(\"collect\")\n"));
}

// [GC] LuaAnimatorGroup（LuaAnimatorGroup() 构造，GC 直接 delete）。
//      断点：LuaAnimatorGroup::~LuaAnimatorGroup（Release 全部子动画器）
TEST_F(soui_lua, lua_animator_group_gc_release)
{
    EXPECT_TRUE(Run("lag",
        "local o = LuaAnimatorGroup(0)\n"
        "assert(o ~= nil)\n"
        "o = nil\n"
        "collectgarbage(\"collect\")\n"));
}

// [Rel] CreateAnimatorGroup（exp_global → new SAnimatorGroup）非托管 push，
//       IAnimatorGroup inh IObjRef → 显式 :Release()。
//       断点：SAnimatorGroup::OnFinalRelease → ~SAnimatorGroup
TEST_F(soui_lua, ianimator_group_release)
{
    EXPECT_TRUE(Run("iag",
        "local o = CreateAnimatorGroup(0)\n"
        "assert(o ~= nil)\n"
        "o:Release()\n"
        "o = nil\n"
        "collectgarbage(\"collect\")\n"));
}

// [Rel] CreateAnimatorSet → new SAnimatorSet（TValueAnimatorProxy 引用计数）。
//       断点：~SAnimatorSet
TEST_F(soui_lua, ianimator_set_release)
{
    EXPECT_TRUE(Run("ias",
        "local o = CreateAnimatorSet()\n"
        "assert(o ~= nil)\n"
        "o:Release()\n"
        "o = nil\n"
        "collectgarbage(\"collect\")\n"));
}

// [Rel] CreatePropertyAnimator → new SPropertyAnimator（TValueAnimatorProxy）。
//       断点：~SPropertyAnimator
TEST_F(soui_lua, iproperty_animator_release)
{
    EXPECT_TRUE(Run("ipa",
        "local o = CreatePropertyAnimator()\n"
        "assert(o ~= nil)\n"
        "o:Release()\n"
        "o = nil\n"
        "collectgarbage(\"collect\")\n"));
}

// [Rel] CreatePropertyValuesHolder → new SPropertyValuesHolder（TObjRefImpl）。
//       断点：~SPropertyValuesHolder
TEST_F(soui_lua, iproperty_values_holder_release)
{
    EXPECT_TRUE(Run("ipv",
        "local o = CreatePropertyValuesHolder()\n"
        "assert(o ~= nil)\n"
        "o:Release()\n"
        "o = nil\n"
        "collectgarbage(\"collect\")\n"));
}

// [Rel] CreateTranslatorMgr → SComMgr2::CreateTranslator（DLL 模式运行时加载
//       translator 组件；静态 COM 直链）。断点：~STranslatorMgr
TEST_F(soui_lua, itranslator_mgr_release)
{
    EXPECT_TRUE(Run("itm",
        "local o = CreateTranslatorMgr()\n"
        "assert(o ~= nil)\n"
        "o:Release()\n"
        "o = nil\n"
        "collectgarbage(\"collect\")\n"));
}

// 获取 ISouiFactory
TEST_F(soui_lua, isoui_factory)
{
    EXPECT_TRUE(Run("isf",
        "local f = CreateSouiFactory()\n"
        "assert(f ~= nil)\n"
        "f:Release()\n"
    ));
}

// ========================================================================
// B 组：需要 IApplication 的对象（soui_lua_app 夹具）
// ========================================================================

// IApplication 由夹具 C++ 侧 new / Release：本用例验证脚本可达性，创建/释放
// 在 SetUp/TearDown。断点：~SApplication（在 TearDown）
TEST_F(soui_lua_app, iapplication_getapp)
{
    EXPECT_TRUE(Run("iapp",
        "local a = GetApp()\n"
        "assert(a ~= nil)\n"));
}

// [Rel] ISouiFactory::CreateStringA → CreateIStringA（引用计数字符串对象）。
//       断点：SStringA 实现类的 OnFinalRelease / ~IStringA
TEST_F(soui_lua_app, istringa_release)
{
    EXPECT_TRUE(Run("isa",
        "local f = CreateSouiFactory()\n"
        "local o = f:CreateStringA(\"soui_lua\")\n"
        "f:Release()\n"
        "assert(o ~= nil)\n"
        "o:Release()\n"
        "o = nil\n"
        "collectgarbage(\"collect\")\n"));
}

// [Rel] ISouiFactory::CreateStringW（lua string 经 read<wchar_t*> 特化转 UTF-16）。
//       断点：SStringW 实现类的 OnFinalRelease / ~IStringW
TEST_F(soui_lua_app, istringw_release)
{
    EXPECT_TRUE(Run("isw",
        "local f = CreateSouiFactory()\n"
        "local o = f:CreateStringW(\"soui_lua\")\n"
        "f:Release()\n"
        "assert(o ~= nil)\n"
        "o:Release()\n"
        "o = nil\n"
        "collectgarbage(\"collect\")\n"));
}

// [Rel] ISouiFactory::CreateXmlDoc → CreateIXmlDoc。
//       断点：SXmlDoc 的 OnFinalRelease / ~SXmlDoc
TEST_F(soui_lua_app, ixmldoc_release)
{
    EXPECT_TRUE(Run("ixd",
        "local f = CreateSouiFactory()\n"
        "local o = f:CreateXmlDoc()\n"
        "f:Release()\n"
        "assert(o ~= nil)\n"
        "o:Release()\n"
        "o = nil\n"
        "collectgarbage(\"collect\")\n"));
}

// [Rel] ISouiFactory::CreateResProvider(RES_FILE) + InitFileResProvider，脚本侧
//       :Release() 自行销毁（不交给 app 的 ResProviderMgr）。
//       断点：~SResProviderFILE
TEST_F(soui_lua_app, iresprovider_release)
{
    EXPECT_TRUE(Run("irp",
        "local f = CreateSouiFactory()\n"
        "local o = f:CreateResProvider(1)\n"                    // 1 == RES_FILE
        "f:Release()\n"
        "assert(o ~= nil)\n"
        "assert(InitFileResProvider(o, \"" SOUI_FUN_TEST_DEMO_UIRES "\"))\n"
        "o:Release()\n"
        "o = nil\n"
        "collectgarbage(\"collect\")\n"));
}

// [Rel] ISouiFactory::CreateTimer(slot)。STimer ctor 里 m_evtSlot.Attach(pSlot->
//       Clone())——timer 持有的是 slot 的【克隆】，脚本创建的原 slot 仍归脚本
//       所有，须各自 :Release()。断点：~STimer / ~LuaFunctionSlot ×2
TEST_F(soui_lua_app, itimer_release)
{
    EXPECT_TRUE(Run("itm2",
        "local f = CreateSouiFactory()\n"
        "local slot = CreateEventSlot(\"onTimer\")\n"
        "local o = f:CreateTimer(slot)\n"
        "f:Release()\n"
        "assert(o ~= nil)\n"
        "o:Release()\n"
        "slot:Release()\n"
        "o = nil\n"
        "slot = nil\n"
        "f = nil\n"
        "collectgarbage(\"collect\")\n"));
}

// [Rel] ISouiFactory::CreateMenu() → new SMenu(0)。断点：~SMenu
TEST_F(soui_lua_app, imenu_release)
{
    EXPECT_TRUE(Run("imn",
        "local f = CreateSouiFactory()\n"
        "local o = f:CreateMenu()\n"
        "f:Release()\n"
        "assert(o ~= nil)\n"
        "o:Release()\n"
        "o = nil\n"
        "collectgarbage(\"collect\")\n"));
}

// [Rel] ISouiFactory::CreateMenuEx() → new SMenuEx。断点：~SMenuEx
TEST_F(soui_lua_app, imenuex_release)
{
    EXPECT_TRUE(Run("imx",
        "local f = CreateSouiFactory()\n"
        "local o = f:CreateMenuEx()\n"
        "f:Release()\n"
        "assert(o ~= nil)\n"
        "o:Release()\n"
        "o = nil\n"
        "collectgarbage(\"collect\")\n"));
}

// ILogMgr 由 app 拥有（GetLogManager 返回 app 成员）——不释放，仅验证可达。
// app 析构时随 app 释放（~SApplication 断点）
TEST_F(soui_lua_app, ilogmgr_get)
{
    EXPECT_TRUE(Run("ilog",
        "local o = GetLogMgr()\n"
        "assert(o ~= nil)\n"));
}

// [Rel] GetApp():LoadAnimation（exp_IApp.h class_def 直绑，非托管 push）加载
//       anim:alpha_in（root <set> → SAnimationSet）。脚本接管初始引用=1，
//       必须显式 :Release()。断点：SAnimationSet::OnFinalRelease
TEST_F(soui_lua_app, ianimation_load_release)
{
    EXPECT_TRUE(Run("iani",
        "local a = GetApp()\n"
        "assert(a ~= nil)\n"
        "local o = a:LoadAnimation(\"anim:alpha_in\")\n"
        "assert(o ~= nil)\n"
        "o:Release()\n"
        "o = nil\n"));
}

// [Rel] GetApp():LoadValueAnimator（exp_IApp.h class_def 直绑）加载
//       valueAni:alphaAni（root <floatAnimator> → SFloatAnimator）。
//       断点：SFloatAnimator::OnFinalRelease
TEST_F(soui_lua_app, ivalue_animator_app_load_release)
{
    EXPECT_TRUE(Run("iva1",
        "local a = GetApp()\n"
        "assert(a ~= nil)\n"
        "local o = a:LoadValueAnimator(\"valueAni:alphaAni\")\n"
        "assert(o ~= nil)\n"
        "o:Release()\n"
        "o = nil\n"));
}

// [Rel] 全局 LoadValueAnimator（exp_global → exp_IAnimatorSet.h 的
//       Lua_LoadValueAnimator → SApplication::LoadValueAnimator）非托管 push，
//       必须显式 :Release()。断点：SFloatAnimator::OnFinalRelease
TEST_F(soui_lua_app, ivalue_animator_global_load_release)
{
    EXPECT_TRUE(Run("iva2",
        "local o = LoadValueAnimator(\"valueAni:alphaAni\")\n"
        "assert(o ~= nil)\n"
        "o:Release()\n"
        "o = nil\n"
        "collectgarbage(\"collect\")\n"));
}

// [None] LuaAnimationListener（IAnimationListener 是无基类纯接口，非托管 push、
//        无 Release 绑定、无 __gc）→ 预期泄漏，人工确认。
//        观察：用例跑完后对象仍存活，直到 lua_close 也不回收。
TEST_F(soui_lua_app, lua_animation_listener_no_owner)
{
    EXPECT_TRUE(Run("llal",
        "local o = CreateAnimationListener(1)\n"
        "assert(o ~= nil)\n"
        "o = nil\n"
        "collectgarbage(\"collect\")\n"));
}

// [None] LuaAnimatorListener（IAnimatorListener 无基类纯接口）→ 预期泄漏
TEST_F(soui_lua_app, lua_animator_listener_no_owner)
{
    EXPECT_TRUE(Run("llvl",
        "local o = CreateValueAnimatorListener(1)\n"
        "assert(o ~= nil)\n"
        "o = nil\n"
        "collectgarbage(\"collect\")\n"));
}

// [None] LuaAnimatorUpdateListener（IAnimatorUpdateListener 无基类纯接口）→ 预期泄漏
TEST_F(soui_lua_app, lua_animator_update_listener_no_owner)
{
    EXPECT_TRUE(Run("llvu",
        "local o = CreateValueAnimatorUpdateListener(1)\n"
        "assert(o ~= nil)\n"
        "o = nil\n"
        "collectgarbage(\"collect\")\n"));
}

// [Rel] LuaLvAdapter : SAdapterBase : TObjRefImpl<LvAdatperImpl<ILvAdapter>>，
//       非托管 push → 显式 :Release()。断点：~LuaLvAdapter
TEST_F(soui_lua_app, lua_lv_adapter_release)
{
    EXPECT_TRUE(Run("llva",
        "local o = CreateLvAdapter(1)\n"
        "assert(o ~= nil)\n"
        "o:Release()\n"
        "o = nil\n"
        "collectgarbage(\"collect\")\n"));
}

// [Rel] LuaMcAdapter : SMcAdapterBase : TObjRefImpl<...> → :Release()。
//       断点：~LuaMcAdapter
TEST_F(soui_lua_app, lua_mc_adapter_release)
{
    EXPECT_TRUE(Run("llmc",
        "local o = CreateMcAdapter(1)\n"
        "assert(o ~= nil)\n"
        "o:Release()\n"
        "o = nil\n"
        "collectgarbage(\"collect\")\n"));
}

// [Rel] LuaTvAdapter : STreeAdapterBase<int> : TObjRefImpl<...> → :Release()。
//       断点：~LuaTvAdapter
TEST_F(soui_lua_app, lua_tv_adapter_release)
{
    EXPECT_TRUE(Run("lltv",
        "local o = CreateTvAdapter(1)\n"
        "assert(o ~= nil)\n"
        "o:Release()\n"
        "o = nil\n"
        "collectgarbage(\"collect\")\n"));
}

// [Rel] LuaFunctionSlot : TObjRefImpl<IEvtSlot>，非托管 push → :Release()。
//       断点：~LuaFunctionSlot
TEST_F(soui_lua_app, lua_function_slot_release)
{
    EXPECT_TRUE(Run("llfs",
        "local o = CreateEventSlot(\"onLuaEvent\")\n"
        "assert(o ~= nil)\n"
        "o:Release()\n"
        "o = nil\n"
        "collectgarbage(\"collect\")\n"));
}
