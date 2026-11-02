/*
 * soui_main.cpp -- SOUI on WebAssembly 演示（阶段 2：渲染器与 XML 资源进场）
 *
 * 与阶段 0 的 main.cpp（裸 Win32 窗口 + 手写 GDI 绘制）相比，这个入口换了三层：
 *
 *   1) 应用对象：SApplication + SAppCfg::DoConfig()。渲染器由 components/render-gdi
 *      静态提供、图片解码器由 components/imgdecoder-stb 静态提供。浏览器里没有
 *      dlopen，所以根 CMakeLists 在 EMSCRIPTEN 下强制 SOUI_ENABLE_COM_LIB=ON，
 *      这两个组件被编进模块本体。
 *   2) 资源：SAppCfg::SetSysResFile()/SetAppResFile() 走 SOUI 核心的
 *      SResProviderFiles（读目录下的 uires.idx），不依赖 resprovider-zip。
 *      目录由 build-soui.sh 用 --preload-file 挂进 MEMFS 的 /opt/soui 下。
 *   3) 窗口：SHostWnd 子类 + uires/xml/dlg_main.xml 布局，控件树、皮肤、事件映射
 *      全部由 SOUI 负责；本文件只做宿主接线（上屏回调、消息泵、输入注入）和
 *      三页数据控件的填充（MVC / 表格 / 图表）。
 *
 * 没有调用 SApplication::Run()：那是个阻塞消息循环，而浏览器主线程不能被阻塞。
 * 改成 emscripten_set_main_loop(pump, 0, 1) 由 requestAnimationFrame 驱动同一个
 * PeekMessage/TranslateMessage/DispatchMessage 序列——wasm 的
 * SConnection::peekMsg() 在取消息前会先 updateMsgQueue()，所以 WM_TIMER 依旧能到。
 *
 * 上屏：和 main.cpp 一样注册 SwinxSetFbPresentCb()，帧缓冲仍是 cairo ARGB32
 * 预乘像素，canvas 侧的换算在 soui.html 里。
 *
 * 文字：字体不在仓库里。build-soui.sh 在构建机上找一个 TTF 预置成
 * /opt/soui/fonts/demo.ttf，这里用 AddFontResource() 把它交给 fontconfig
 * （swinx 的 cairo GDI 后端在 AddFontResourceExA 里调 FcConfigAppFontAddFile）。
 * 一个字体都没预置时，文本仍能测量出宽高，但字形是空白。
 */
#include <emscripten.h>
#include <stdio.h>
#include <string.h>
#include <algorithm>
#include <vector>

#include <windows.h>
#include <souistd.h>
#include <SApp.h>
#include <tchar.h>

#include <commask.h>
/* SCOM_MASK 必须在 include <SAppCfg.h> 之前定义。commgr2.h 里 #ifndef SCOM_MASK
 * 会把默认值设成 scom_mask_scom_all，而 SAppCfg::DoConfig() 的 switch 无条件引用
 * CreateResProvider_ZIP/CreateResProvider_7ZIP：全掩码会让这两个组件的
 * SCreateInstance 变成真实链接依赖，wasm 构建图里没有它们（resprovider-zip 依赖的
 * third-part/zlib 与 swinx/thirdparty 的 zlib_static 是同一批符号的两份拷贝，
 * 链进同一个模块会 duplicate symbol）。按位关掉后对应分支编译成 return FALSE，
 * 不产生任何引用。 */
#define SCOM_MASK (scom_mask_render_gdi | scom_mask_imgdecoder_stb)

#include <SAppCfg.h>
#include <core/SHostWnd.h>
#include <core/SModalViewSession.h>
#include <helper/SDpiScale.h>
#include <helper/SDpiHelper.hpp>
#include <control/SRichEdit.h>
#include <richedit.h>
#include "SCtrlsRegister.h"
#include "SHexEdit.h"
#include "SProgressRing.h"
/* MVC 页要用的适配器契约（数据源 → ILvAdapter → SItemPanel 复用）和三个虚拟列表
   控件；表格页/图表页的控件都在 ExtendCtrls 里，头文件路径相对 controls.extend，
   demo 目标的 include 目录已经带了它。 */
#include <helper/SAdapterBase.h>
#include <control/SListView.h>
#include <control/STileView.h>
#include <control/SMCListView.h>
#include <control/STreeView.h>
#include <control/SCmnCtrl.h>
#include "gridctrl/SGridCtrl.h"
#include "hellocharts/model/SAxis.h"
#include "hellocharts/view/SLineChartView.h"
#include "hellocharts/view/SColumnChartView.h"
#include "hellocharts/view/SPieChartView.h"
/* 更多控件页的折叠分组列表：SGroupList 只负责建模板实例和收发事件，
 * 条目文字要由宿主在 EventGroupListInitGroup/InitItem 回调里填。 */
#include "SGroupList.h"
/* 动画页要演示的三种形态都在这三个头里：SAnimation 族由 SApplication::LoadAnimation
 * 从 anim: 资源建，SValueAnimator 由 LoadValueAnimator 从 valueAni: 资源建，
 * SPropAnimator（SPropertyAnimator）是 Android ObjectAnimator 风格的静态工厂，
 * 不进动画器工厂表，直接 new。SColor 用来把 COLORREF 拆成 RGBA 写回属性。 */
#include <valueAnimator/SValueAnimator.h>
#include <valueAnimator/SPropertyAnimator.h>
#include <helper/SColor.h>

#include "framebuffer.h"
#include "input.h"

/* components/ScriptModule-LUA 的工厂入口。浏览器里没有 COM 注册表，桌面版靠
 * CoCreateInstance 拿到 SIScriptFactory，这里直接调导出的 C 函数；工厂交给
 * SApplication::SetScriptFactory，之后 SHostWnd 解析 <script> 节点时就会用它
 * 创建模块（见 SOUI/src/core/shostwnd.cpp:437）。 */
#ifndef SOUI_WASM_HAS_LUA
#define SOUI_WASM_HAS_LUA 1
#endif
#ifndef SOUI_WASM_HAS_SCINTILLA
#define SOUI_WASM_HAS_SCINTILLA 1
#endif
#if SOUI_WASM_HAS_LUA
extern "C" BOOL Script_Lua_SCreateInstance(IObjRef **ppScript);
#endif

using namespace SNS;

/* MEMFS 挂载点，与 build-soui.sh 里的 --preload-file 目标路径一一对应 */
static const TCHAR kSysResDir[] = _T("/opt/soui/soui-sys-resource");
static const TCHAR kAppResDir[] = _T("/opt/soui/uires");
static const TCHAR kFontPath[] = _T("/opt/soui/fonts/demo.ttf");

/* 帧缓冲尺寸运行期取值（物理分辨率 = 逻辑 800x600 x DPI 档位），
 * 宿主窗口的创建尺寸见 main()（逻辑），上屏用 SwinxFbWidth/Height。 */

static HWND g_hWnd = NULL;
class CWasmHostWnd;
static CWasmHostWnd *g_pHost = NULL;

/* main() 算出的渲染档位（dpr 向上贴到内置档位后的结果），帧缓冲与布局都用它。
 * 不走 SDpiScale::NormalizeScale —— 那是"向下取整到内置档位"，浏览器缩放 110% /
 * 120% 会被压成 100%，于是帧缓冲按 800x600 建、却被浏览器放大 1.1 倍显示（发虚）；
 * 而只把 main() 改成精确档位又会和 ScaleHost 归一后的值打架（画布比布局大、右下黑边）。
 * 见 main() 与 HandleScaleChange。 */
static int g_nWasmDpiScale = 100;

/* 一次 SwinxDemoPump 最多工作多久。要大于单帧重绘的耗时（本机 SVG 密集页约
 * 35~107ms），否则每帧都超预算、等于一次 pump 只处理一条消息；也要远小于把人
 * 感觉成"卡住"的阈值。 */
static const double kPumpBudgetMs = 100.0;

/* ------------------------------------------------------------------------- */
/* MVC 页的数据源：一份行数组，三个视图共用                                    */
/* ------------------------------------------------------------------------- */

/* 行数据放在文件级静态存储期，而不是 CWasmHostWnd 的成员：控件通过 SAutoRefPtr
 * 持有适配器（SListView.h:438 / STileView.h:462 / SMCListView.h:483），适配器比宿主
 * 对象的成员活得久——~SHostWnd 在派生类成员析构之后才跑，控件那时还会回调 getView。
 * 数据挂成员就会读到已经析构的对象；挂静态存储期，适配器什么时候被释放都安全。 */
struct DemoRow
{
    SStringT strName;
    SStringT strKind;
    SStringT strVersion;
    SStringT strNote;
    SStringT strStat;
    int nScore;
};

static std::vector<DemoRow> g_lvRows;

/* 500 行「资源商店」数据：20 个前缀 × 25 个品类正好铺满，名字不重复。
 * 数据量是这一页的重点——列表类控件只建可视区里的行窗口，500 行和 5 行的
 * 内存/绘制开销几乎一样，滚动条拖到哪里就按需取哪一段。 */
static void mvcSeedRows()
{
    static const TCHAR *kPrefix[] = {
        _T("极简"), _T("流光"), _T("青云"), _T("远山"), _T("星河"),
        _T("墨白"), _T("苍穹"), _T("木棉"), _T("琥珀"), _T("松风"),
        _T("暖阳"), _T("霜叶"), _T("浮光"), _T("听雨"), _T("归舟"),
        _T("素问"), _T("长歌"), _T("青柠"), _T("拾光"), _T("云栖")};
    static const TCHAR *kNoun[] = {
        _T("仪表盘"), _T("数据面板"), _T("侧边导航"), _T("登录页"), _T("换肤中心"),
        _T("文件管理器"), _T("聊天窗口"), _T("播放器"), _T("图表墙"), _T("设置向导"),
        _T("订单列表"), _T("排班日历"), _T("监控大屏"), _T("统计报表"), _T("消息中心"),
        _T("通讯录"), _T("编辑器主题"), _T("弹窗套件"), _T("图标库"), _T("表单模板"),
        _T("搜索页"), _T("个人主页"), _T("计费中心"), _T("工单系统"), _T("资源商店")};
    static const TCHAR *kNote[] = {
        _T("顶栏 + 侧栏 + 内容区，换掉数据源就能用"),
        _T("多列指标卡 + 折线趋势，数字全部可绑定"),
        _T("可折叠侧边栏，选中态跟随路由高亮"),
        _T("双栏登录页，左侧插画右侧表单"),
        _T("深浅色主题实时预览，皮肤走 SVG"),
        _T("树形目录 + 面包屑，支持拖拽排序"),
        _T("左右分栏会话列表与消息流，输入区固定"),
        _T("播放器控制条 + 播放列表，纯自绘"),
        _T("九宫格图表墙，卡片可按数据重排"),
        _T("分步设置向导，进度条与按钮联动"),
        _T("带筛选表头的订单列表，行可展开"),
        _T("月视图排班表，单元格内可编辑"),
        _T("大屏监控模板，深色底 + 高亮指标"),
        _T("报表筛选区 + 表格 + 汇总行"),
        _T("消息中心，未读角标与分页加载"),
        _T("按拼音分组的通讯录，右侧字母索引"),
        _T("编辑器配色方案，含 12 套预设"),
        _T("通用弹窗与提示条套件，动画可配"),
        _T("线性图标库，统一 24px 网格"),
        _T("表单模板合集，校验规则开箱即用"),
        _T("搜索页，历史记录与热门词推荐"),
        _T("个人主页，头像卡片 + 动态时间线"),
        _T("计费中心，套餐对比与账单明细"),
        _T("工单系统，状态流转与处理人分配"),
        _T("资源商店首页，分类导航 + 推荐位")};
    static const TCHAR *kKind[] = {_T("桌面模板"), _T("移动端模板"), _T("控件皮肤"), _T("组件库")};

    const int nPrefix = (int)(sizeof(kPrefix) / sizeof(kPrefix[0]));
    const int nNoun = (int)(sizeof(kNoun) / sizeof(kNoun[0]));

    g_lvRows.clear();
    g_lvRows.reserve(500);
    for (int i = 0; i < 500; ++i)
    {
        int nP = i % nPrefix;
        int nN = (i / nPrefix) % nNoun;
        DemoRow row;
        row.strName.Format(_T("%s%s"), kPrefix[nP], kNoun[nN]);
        row.strKind = kKind[(nP + nN) % 4];
        row.strVersion.Format(_T("1.%d.%d"), nN, nP);
        row.strNote = kNote[nN];
        row.strStat.Format(_T("下载 %d.%dk"), 1 + (i * 7) % 90, (i * 3) % 10);
        row.nScore = 45 + (i * 37) % 54;
        g_lvRows.push_back(row);
    }
}

namespace
{
    /* listview 和 tileview 用同一个适配器：两种控件都把 <template> 节点整体交给
     * SItemPanel::InitFromXml，模板里子控件的名字一样，getView 一份代码就能同时
     * 喂两个视图。行模板里放了头像、名称、来源、说明、热度条和下载量，证明
     * 列表项不是"一行文本"，而是任意控件树。 */
    class CDemoLvAdapter : public SAdapterBase
    {
    public:
        STDMETHOD_(int, getCount)(THIS) OVERRIDE { return (int)g_lvRows.size(); }

        STDMETHOD_(void, getView)(int position, SItemPanel *pItem, SXmlNode xmlTemplate) OVERRIDE
        {
            if (!pItem)
                return;
            /* 回收复用的行已经有子控件了，再 InitFromXml 会叠出第二份。 */
            if (pItem->GetChildrenCount() == 0)
                pItem->InitFromXml(&xmlTemplate);
            if (position < 0 || position >= (int)g_lvRows.size())
                return;
            const DemoRow &row = g_lvRows[position];
            SWindow *pInitial = pItem->FindChildByName(_T("item_initial"));
            if (pInitial)
                pInitial->SetWindowText(row.strName.Left(1));
            SWindow *pName = pItem->FindChildByName(_T("item_name"));
            if (pName)
                pName->SetWindowText(row.strName);
            SWindow *pKind = pItem->FindChildByName(_T("item_kind"));
            if (pKind)
                pKind->SetWindowText(row.strKind);
            SWindow *pNote = pItem->FindChildByName(_T("item_note"));
            if (pNote)
                pNote->SetWindowText(row.strNote);
            SWindow *pStat = pItem->FindChildByName(_T("item_stat"));
            if (pStat)
                pStat->SetWindowText(row.strStat);
            SProgress *pScore = pItem->FindChildByName2<SProgress>(_T("item_score"));
            if (pScore)
                pScore->SetValue(row.nScore);
        }
    };

    /* mclistview 多一条契约：控件按表头宽度，把名为 col1/col2/… 的子窗口逐个
     * Move 到对应列矩形（SMCListView.cpp:659-683），而列名来自适配器的
     * GetColumnName。模板里的窗口名和这里对不上，列就永远不被搬到位。 */
    class CDemoMcAdapter : public SMcAdapterBase
    {
    public:
        STDMETHOD_(int, getCount)(THIS) OVERRIDE { return (int)g_lvRows.size(); }

        STDMETHOD_(SStringW, GetColumnName)(int iCol) const OVERRIDE
        {
            return SStringW().Format(L"col%d", iCol + 1);
        }

        STDMETHOD_(void, getView)(int position, SItemPanel *pItem, SXmlNode xmlTemplate) OVERRIDE
        {
            if (!pItem)
                return;
            if (pItem->GetChildrenCount() == 0)
                pItem->InitFromXml(&xmlTemplate);
            if (position < 0 || position >= (int)g_lvRows.size())
                return;
            const DemoRow &row = g_lvRows[position];
            SWindow *pCol1 = pItem->FindChildByName(_T("mc_c1"));
            if (pCol1)
                pCol1->SetWindowText(row.strName);
            SWindow *pCol2 = pItem->FindChildByName(_T("mc_c2"));
            if (pCol2)
                pCol2->SetWindowText(row.strKind);
            SWindow *pCol3 = pItem->FindChildByName(_T("mc_c3"));
            if (pCol3)
                pCol3->SetWindowText(row.strVersion);
            SWindow *pCol4 = pItem->FindChildByName(_T("mc_c4"));
            if (pCol4)
                pCol4->SetWindowText(row.strNote);
            SProgress *pScore = pItem->FindChildByName2<SProgress>(_T("mc_score"));
            if (pScore)
                pScore->SetValue(row.nScore);
        }
    };

    /* treeview（MVC 页的树视图）：契约同 uieditor 的 CFileTreeAdapter
     * （demos/uieditor/FileTreeAdapter.h）——继承 STreeAdapterBase，子类只实现
     * getView；展开/收起（ExpandItem）、HasChildren、插入项（InsertItem）都由
     * 基类默认实现处理。树数据挂在适配器自己的 CSTree 里，不走文件级静态存储。
     * 条目文字按 196dp 宽的树列裁过：两层缩进后只剩约 150dp，写太长会看不到尾巴。 */
    struct TreeDemoData
    {
        SStringT strName;
    };

    class CTreeDemoAdapter : public STreeAdapterBase<TreeDemoData>
    {
    public:
        void BuildDemoData()
        {
            static const TCHAR *kCore[] = {_T("SWindow 控件基类"),
                                           _T("SHostWnd 宿主窗口"),
                                           _T("SEventSet 事件订阅"),
                                           _T("SAdapterBase 数据绑定"),
                                           _T("STimelineHandler 动画")};
            static const TCHAR *kBuild[] = {_T("render-gdi (cairo)"),
                                            _T("imgdecoder-stb"),
                                            _T("resprovider-zip"),
                                            _T("平台适配")};
            static const TCHAR *kPlatform[] = {_T("swinx（Win32 → POSIX）"),
                                               _T("Android / OHOS PlatformAPI")};
            static const TCHAR *kComp[] = {_T("SListView / STileView"),
                                           _T("SMCListView 多列表格"),
                                           _T("STreeView 树"),
                                           _T("SRichEdit 富文本"),
                                           _T("Scintilla 编辑器")};

            /* 平台适配这一支故意不展开：树的三级缩进和展开箭头要能就地看到。 */
            HSTREEITEM hG1 = InsertItem(TreeDemoData{_T("SOUI 核心（5）")});
            for (int i = 0; i < (int)(sizeof(kCore) / sizeof(kCore[0])); ++i)
                InsertItem(TreeDemoData{kCore[i]}, hG1);
            SetItemExpanded(hG1, TRUE);

            HSTREEITEM hG2 = InsertItem(TreeDemoData{_T("构建与渲染（4）")});
            for (int i = 0; i < (int)(sizeof(kBuild) / sizeof(kBuild[0])); ++i)
            {
                HSTREEITEM hChild = InsertItem(TreeDemoData{kBuild[i]}, hG2);
                if (i + 1 == (int)(sizeof(kBuild) / sizeof(kBuild[0])))
                {
                    for (int j = 0; j < (int)(sizeof(kPlatform) / sizeof(kPlatform[0])); ++j)
                        InsertItem(TreeDemoData{kPlatform[j]}, hChild);
                }
            }
            SetItemExpanded(hG2, TRUE);

            HSTREEITEM hG3 = InsertItem(TreeDemoData{_T("组件（5）")});
            for (int i = 0; i < (int)(sizeof(kComp) / sizeof(kComp[0])); ++i)
                InsertItem(TreeDemoData{kComp[i]}, hG3);
            SetItemExpanded(hG3, TRUE);
        }

        STDMETHOD_(void, getView)(THIS_ HSTREEITEM hItem, SItemPanel *pItem, SXmlNode xmlTemplate) OVERRIDE
        {
            if (hItem == ITEM_ROOT || !pItem)
                return;
            /* 回收复用的行已经有子控件了，再 InitFromXml 会叠出第二份。 */
            if (pItem->GetChildrenCount() == 0)
                pItem->InitFromXml(&xmlTemplate);
            SWindow *pTxt = pItem->FindChildByName(_T("txt_label"));
            if (pTxt)
                pTxt->SetWindowText(GetItemData(hItem).strName);
        }
    };
} // namespace

/* ------------------------------------------------------------------------- */
/* 宿主窗口：布局来自 uires/xml/dlg_main.xml，资源 id 见 uires/uires.idx      */
/* ------------------------------------------------------------------------- */

class CWasmHostWnd : public SHostWnd
    , public SDpiHandler<CWasmHostWnd>
    , public IAnimatorUpdateListener
{
public:
    CWasmHostWnd();

    void OnMvcShuffle();
    void OnMvcAdd();
    /* 动画页的三个按钮：分别演示补间动画、值动画、属性动画。
     * SAnimation 只作用于 SWindow；SValueAnimator 只产生值，由下面这个监听回调
     * 消费成背景色和进度；SPropAnimator 直接把值写回窗口/布局属性。 */
    void OnAnimTween();
    void OnAnimValue();
    void OnAnimProp();
    /* 模态表单页：用 SModalRoot/SModalView（不是 SHostDialog::DoModal——wasm 的
     * 内层消息循环拿不到浏览器的新事件）弹出一张表单，提交后把结果写回页面。 */
    void OnModalOpen();
    /* IAnimatorUpdateListener：值动画每帧回调一次（约 10ms 一拍）。
     * 接口上的监听器是裸指针、不 AddRef，所以监听者必须比动画器活得久——
     * 宿主窗口贯穿整个进程生命周期，满足这个前提。 */
    virtual void WINAPI onAnimationUpdate(IValueAnimator *pAnimator) OVERRIDE;
    void OnTblAddRow();
    void OnTblToggleCheck();
    void OnChartRandom();
    void OnChartAnim();

    /* 按控件名读窗口文字（UTF-8 原始字节，不追加 '\0'），返回实际写入 pBuf 的字节数。
     * 给页面上的说明行（lua_hint / edit_hint）和各页状态行复用。 */
    void setLabel(LPCTSTR pszName, const SStringT &strText);

    /* 顶栏品牌位：SOUI 的版本号以 SOUI/include/soui-version.h 为唯一来源，
     * 不在 XML 里抄一份字面量——升版本时不用再回来改布局。 */
    void fillBrand();
    /* 给 MVC 页的四个视图挂适配器：三个虚拟列表（数据已经备在 g_lvRows 里）
     * 加一个树（数据由 CTreeDemoAdapter 自己建）。必须在布局解析完、控件树
     * 建好之后调，否则 FindChildByName 找不到控件。 */
    void setupMvcDemo();
    void notifyMvcViews();
    void refreshMvcStatus();
    /* <gridctrl> 的单元格文字只能走 SetItemText，XML 里没有内联单元格的语法；
     * 不填就是一张只有格线的空表，和"控件没建出来"在画面上一样。 */
    void fillGridDemo();
    void refreshGridStatus();
    /* HelloChart 的构造函数自带一份 dummy 数据，所以不接线也有画面。这里换成
     * 自己建的数据模型，是为了让"换一批数据"按钮有东西可换。 */
    void fillChartDemo();
    void refreshChartStatus();

    /* 更多控件页：grouplist 的条目文字由宿主填。三条 EVENT_ID_HANDLER 在
     * EVENT_MAP 里按 id=301 接 EventGroupListInitGroup/InitItem/GroupStateChanged，
     * 事件在布局解析期间逐个触发，靠冒泡到宿主的消息映射被接住。 */
    BOOL OnGlInitGroup(IEvtArgs *e);
    BOOL OnGlInitItem(IEvtArgs *e);
    BOOL OnGlGroupState(IEvtArgs *e);

    /* <hexedit> 的 XML 只能配外观，数据必须由宿主喂（桌面 demo 也在
     * MainDlg_Init.cpp 里 SetData）。不填数据时它只画出地址栏底色，看起来像
     * 一块空白 —— 这里塞 0..127，正好能看出十六进制列和 ASCII 列的对齐。 */
    void fillHexDemo();
    /* <listctrl> 的行只能由代码插入，见 fillListDemo 上的说明。 */
    void fillListDemo();
    /* 编译期被关掉的可选能力（SOUI_BUILD_WASM_LUA / SOUI_BUILD_WASM_SCINTILLA）在
     * 页面上留一句说明：标签还在 XML 里、只是没注册或被 no-op 掉，画面表现为页内
     * 一块空白。空白和"布局坏了"在截图上分不出来，所以要把原因写在页面上。 */
    void noteDisabledFeatures();

protected:
    /* DPI（参考 games/cnchess 的 SDpiHandler 用法）：WM_INITDIALOG 时
     * SDpiHandler::ScaleHost 从 GetWindowScale(hWnd)（= swinx GetDpiForWindow，
     * wasm 报告 96 x 档位）还原缩放档位。桌面实现以原窗口中心放大 desRect，
     * 可能产生负原点；wasm 宿主钉在 (0,0)、尺寸必须等于帧缓冲物理分辨率，
     * 所以只借它的档位，desRect 换成 fb 矩形。 */
    virtual void HandleScaleChange(WORD nScale, const RECT *desRect) OVERRIDE
    {
        (void)desRect;
        /* ScaleHost 传下来的 nScale 已经被 SDpiScale::NormalizeScale 向下归一过
         * （110 → 100，120 → 100，175 → 150）。但帧缓冲是按 g_nWasmDpiScale 建的，
         * 两个数不一致就会"画布比布局大"：右下留黑边。以帧缓冲的档位为准。 */
        int nWant = g_nWasmDpiScale > 0 ? g_nWasmDpiScale : (int)nScale;
        if (nWant == GetRoot()->GetScale())
            return;
        RECT rcFb = {0, 0, SwinxFbWidth(), SwinxFbHeight()};
        SetScale(nWant, &rcFb);
    }

    /* SOUI 层事件：子控件通过反射通知上来的 clicked 命令。
     * EVENT_ID_HANDLER 的 id 是布局 XML 里 grouplist 的 id 属性（301）。 */
    EVENT_MAP_BEGIN()
        EVENT_NAME_COMMAND(L"btn_mvc_shuffle", OnMvcShuffle)
        EVENT_NAME_COMMAND(L"btn_mvc_add", OnMvcAdd)
        EVENT_NAME_COMMAND(L"btn_tbl_addrow", OnTblAddRow)
        EVENT_NAME_COMMAND(L"btn_tbl_check", OnTblToggleCheck)
        EVENT_NAME_COMMAND(L"btn_chart_random", OnChartRandom)
        EVENT_NAME_COMMAND(L"btn_chart_anim", OnChartAnim)
        EVENT_NAME_COMMAND(L"btn_anim_tween", OnAnimTween)
        EVENT_NAME_COMMAND(L"btn_anim_value", OnAnimValue)
        EVENT_NAME_COMMAND(L"btn_anim_prop", OnAnimProp)
        EVENT_NAME_COMMAND(L"btn_modal_open", OnModalOpen)
        EVENT_ID_HANDLER(301, EventGroupListInitGroup::EventID, OnGlInitGroup)
        EVENT_ID_HANDLER(301, EventGroupListInitItem::EventID, OnGlInitItem)
        EVENT_ID_HANDLER(301, EventGroupStateChanged::EventID, OnGlGroupState)
    EVENT_MAP_END2(SHostWnd)

    /* 宿主窗口本身不再拦任何消息：WM_PAINT/WM_TIMER/WM_CHAR 都交回 SHostWnd
     * 自己的处理（绘制、动画心跳、按键分发）。SDpiHandler 在最前面接
     * WM_INITDIALOG（触发 ScaleHost）与 WM_DPICHANGED（wasm 上不会来，留着
     * 与桌面同一份消息映射）。 */
    BEGIN_MSG_MAP_EX(CWasmHostWnd)
        CHAIN_MSG_MAP(SDpiHandler<CWasmHostWnd>)
        CHAIN_MSG_MAP(SHostWnd)
        REFLECT_NOTIFICATIONS_EX()
    END_MSG_MAP()

private:
    SWindow *findChild(LPCTSTR pszName);

    /* "换一批数据"每点一次 +1：折线图点数、柱高、饼图扇区都跟着它变。 */
    int m_nChartSeq;

    /* 动画页的一次性动画器。宿主持有一份引用，重复点击时先 end() 旧的再换新的，
     * 避免同一目标窗口上叠两条动画。SAnimation 不在这里留引用：SetAnimation 后
     * 立刻 Release 加载引用，生命周期由目标窗口自己管。 */
    SAutoRefPtr<IValueAnimator> m_valueColorAni;
    SAutoRefPtr<IValueAnimator> m_valueFloatAni;
    SAutoRefPtr<IValueAnimator> m_propAni;
};

CWasmHostWnd::CWasmHostWnd()
    : SHostWnd(_T("LAYOUT:XML_MAINWND"))
    , m_nChartSeq(0)
{
}

SWindow *CWasmHostWnd::findChild(LPCTSTR pszName)
{
    /* SHostWnd::FindChildByName 直接走 GetRoot()：宿主窗口还没建起来（或者建到
     * 一半失败）时 root 是空的，跳进去就是越界访问。 */
    if (!m_hWnd)
        return NULL;
    return FindChildByName(pszName);
}

void CWasmHostWnd::setLabel(LPCTSTR pszName, const SStringT &strText)
{
    SWindow *pWnd = findChild(pszName);
    if (pWnd)
        pWnd->SetWindowText(strText);
}

void CWasmHostWnd::fillBrand()
{
    SStringT strBrand;
    strBrand.Format(_T("SOUI %d.%d.%d.%d,sc:%d"), SOUI_VER1, SOUI_VER2, SOUI_VER3, SOUI_VER4,GetScale());
    setLabel(_T("app_brand"), strBrand);
}

/* ------------------------------------------------------------------------- */
/* MVC 页：适配器接线与重绘通知                                                */
/* ------------------------------------------------------------------------- */

void CWasmHostWnd::setupMvcDemo()
{
    mvcSeedRows();

    /* SetAdapter 的调用点是控件成员 SAutoRefPtr<ILvAdapter> m_adapter 的赋值，
     * 赋值时已经 AddRef，所以这里放掉 new 带出来的那份创建引用；宿主不再保存
     * 裸指针，之后一律通过控件的 GetAdapter() 回去看状态——这样"适配器到底挂上
     * 没有"是问控件得到的答案，而不是宿主自己记的一笔账。 */
    SListView *pLv = FindChildByName2<SListView>(_T("mvc_lv"));
    if (pLv)
    {
        ILvAdapter *pAd = new CDemoLvAdapter;
        pLv->SetAdapter(pAd);
        pAd->Release();
    }

    STileView *pTile = FindChildByName2<STileView>(_T("mvc_tile"));
    if (pTile)
    {
        ILvAdapter *pAd = new CDemoLvAdapter;
        pTile->SetAdapter(pAd);
        pAd->Release();
    }

    SMCListView *pMc = FindChildByName2<SMCListView>(_T("mvc_mclv"));
    if (pMc)
    {
        IMcAdapter *pAd = new CDemoMcAdapter;
        pMc->SetAdapter(pAd);
        pAd->Release();
    }

    /* treeview 也是这一页的 MVC 视图，但树节点本来就不是平表，走另一棵
     * 适配器数据（CTreeDemoAdapter::BuildDemoData）。 */
    STreeView *pTree = FindChildByName2<STreeView>(_T("mvc_tree"));
    if (pTree)
    {
        CTreeDemoAdapter *pAd = new CTreeDemoAdapter();
        pAd->BuildDemoData();
        pTree->SetAdapter(pAd);
        pAd->Release();
    }

    refreshMvcStatus();
}

void CWasmHostWnd::refreshMvcStatus()
{
    int nLv = 0, nTile = 0, nMc = 0;
    SListView *pLv = FindChildByName2<SListView>(_T("mvc_lv"));
    if (pLv && pLv->GetAdapter())
        nLv = pLv->GetAdapter()->getCount();
    STileView *pTile = FindChildByName2<STileView>(_T("mvc_tile"));
    if (pTile && pTile->GetAdapter())
        nTile = pTile->GetAdapter()->getCount();
    SMCListView *pMc = FindChildByName2<SMCListView>(_T("mvc_mclv"));
    if (pMc && pMc->GetAdapter())
        nMc = pMc->GetAdapter()->getCount();

    SStringT strFirst = g_lvRows.empty() ? SStringT(_T("(空)")) : g_lvRows[0].strName;
    SStringT str;
    str.Format(_T("共 %d 行 · 列表%d / 平铺%d / 多列%d ｜ 首行 %s"),
               (int)g_lvRows.size(), nLv, nTile, nMc, strFirst.c_str());
    setLabel(_T("mvc_status"), str);
}

/* 改完数据必须逐个视图通知。notifyDataSetChanged() 不在 ILvAdapter 接口上，它定义在
 * 实现模板 LvAdatperImpl（SAdapterBase.h:117）里，所以要从控件还回来的接口指针往下
 * 拿：这条继承链 ILvAdapter → LvAdatperImpl<ILvAdapter> → TObjRefImpl<...> →
 * SAdapterBase 是单继承、无歧义的，static_cast 就能算对偏移，不必依赖 RTTI。 */
void CWasmHostWnd::notifyMvcViews()
{
    SListView *pLv = FindChildByName2<SListView>(_T("mvc_lv"));
    if (pLv && pLv->GetAdapter())
        static_cast<SAdapterBase *>(pLv->GetAdapter())->notifyDataSetChanged();

    STileView *pTile = FindChildByName2<STileView>(_T("mvc_tile"));
    if (pTile && pTile->GetAdapter())
        static_cast<SAdapterBase *>(pTile->GetAdapter())->notifyDataSetChanged();

    SMCListView *pMc = FindChildByName2<SMCListView>(_T("mvc_mclv"));
    if (pMc && pMc->GetAdapter())
        static_cast<SMcAdapterBase *>(pMc->GetAdapter())->notifyDataSetChanged();
}

void CWasmHostWnd::OnMvcShuffle()
{
    if (g_lvRows.size() > 1)
        std::rotate(g_lvRows.begin(), g_lvRows.begin() + 1, g_lvRows.end());
    notifyMvcViews();
    refreshMvcStatus();
}

void CWasmHostWnd::OnMvcAdd()
{
    DemoRow row;
    row.strName.Format(_T("新增模板 %d"), (int)g_lvRows.size() + 1);
    row.strKind = _T("追加行");
    row.strVersion = _T("1.0.0");
    row.strNote = _T("notifyDataSetChanged 后由控件回收复用行窗口");
    row.strStat = _T("下载 0.1k");
    row.nScore = 60;
    g_lvRows.push_back(row);
    notifyMvcViews();
    refreshMvcStatus();
}

/* ------------------------------------------------------------------------- */
/* 更多控件页：grouplist 的条目文字                                            */
/* ------------------------------------------------------------------------- */

/* SGroupList 建分组/条目时只发事件不改文字——模板里的 txt_label 由宿主填。
 * 事件在布局解析（CreateChildren）期间就逐个触发，靠 EVENT_MAP 的冒泡接住：
 * 根窗口和容器在 InitFromXml 期间已经存在，事件能一路冒到宿主。 */

BOOL CWasmHostWnd::OnGlInitGroup(IEvtArgs *e)
{
    EventGroupListInitGroup *pEvt = sobj_cast<EventGroupListInitGroup>(e);
    if (!pEvt || !pEvt->pItem)
        return FALSE;
    pEvt->pItem->FindChildByName(_T("txt_label"))
        ->SetWindowText(pEvt->pGroupInfo->strText);
    return TRUE;
}

BOOL CWasmHostWnd::OnGlInitItem(IEvtArgs *e)
{
    EventGroupListInitItem *pEvt = sobj_cast<EventGroupListInitItem>(e);
    if (!pEvt || !pEvt->pItem)
        return FALSE;
    SWindow *pLabel = pEvt->pItem->FindChildByName(_T("txt_label"));
    if (pLabel)
        pLabel->SetWindowText(pEvt->pItemInfo->strText);
    /* check2 类条目的勾选态由控件自己管理，这里只填文字。 */
    return TRUE;
}

/* 分组展开/折叠后，把分组行上的 toggle 箭头同步到新状态：SGroupList 自己只
 * 切 container 的可见性（SGroupList.cpp:211），模板里的 toggle 归宿主管。 */
BOOL CWasmHostWnd::OnGlGroupState(IEvtArgs *e)
{
    EventGroupStateChanged *pEvt = sobj_cast<EventGroupStateChanged>(e);
    if (!pEvt || !pEvt->pItem)
        return FALSE;
    SToggle *pTgl = pEvt->pItem->FindChildByName2<SToggle>(_T("tgl_switch"));
    if (pTgl)
        pTgl->SetToggle(!pEvt->pGroupInfo->bCollapsed);
    return TRUE;
}

/* ------------------------------------------------------------------------- */
/* 表格页：gridctrl 的单元格与复选列                                           */
/* ------------------------------------------------------------------------- */

static const int kGridCheckCol = 4; /* 最后一列是复选框列，见 dlg_main.xml 的 tbl_grid */

void CWasmHostWnd::fillGridDemo()
{
    if (!m_hWnd)
        return;
    SGridCtrl *pGrid = FindChildByName2<SGridCtrl>(_T("tbl_grid"));
    if (!pGrid)
        return;

    static const TCHAR *kCol[] = {_T("模块"), _T("归属"), _T("入口"), _T("渲染"), _T("已验证")};
    static const TCHAR *kRow[] = {_T("窗口"), _T("布局"), _T("渲染"), _T("资源"), _T("脚本"), _T("编辑器")};

    int nRow = pGrid->GetRowCount();
    int nCol = pGrid->GetColumnCount();
    for (int c = 0; c < nCol && c < (int)(sizeof(kCol) / sizeof(kCol[0])); ++c)
        pGrid->SetItemText(0, c, kCol[c]);
    for (int r = 1; r < nRow && r - 1 < (int)(sizeof(kRow) / sizeof(kRow[0])); ++r)
    {
        pGrid->SetItemText(r, 0, kRow[r - 1]);
        pGrid->SetItemText(r, 1, (r % 2) ? _T("SOUI 核心") : _T("controls.extend"));
        /* 数值单元格：只演示 CELL_TYPE_NUMERIC 的输入过滤，不参与勾选统计。 */
        pGrid->SetItemText(r, 2, SStringT().Format(_T("%d"), r * 37).c_str());
        pGrid->SetItemText(r, 3, _T("cairo / skia / apple"));
    }

    /* 单元格类型是替换整个 cell 对象，换完文字要重设：复选框单元格自己存勾选态，
     * 文字仍走 SetItemText。URL / COLOR / DATETIMEPICKER 三种不选：URL 要
     * ShellExecute、后两种要真弹出窗口，都依赖 HWND，wasm 后端没有。 */
    for (int r = 1; r < nRow; ++r)
    {
        if (pGrid->SetCellType(r, kGridCheckCol, CELL_TYPE_CHECKBOX))
        {
            SAutoRefPtr<IGridCellCheck> pChk = pGrid->GetCellOp<IGridCellCheck>(r, kGridCheckCol);
            if (pChk)
                pChk->SetCheck((r % 2) == 1 ? TRUE : FALSE);
        }
    }
    pGrid->SetCellType(1, 2, CELL_TYPE_NUMERIC);
    pGrid->SetEditable(TRUE);
    pGrid->Invalidate();
    refreshGridStatus();
}

void CWasmHostWnd::refreshGridStatus()
{
    SGridCtrl *pGrid = FindChildByName2<SGridCtrl>(_T("tbl_grid"));
    if (!pGrid)
        return;
    int nRow = pGrid->GetRowCount();
    int nCol = pGrid->GetColumnCount();
    int nChecked = 0;
    for (int r = 1; r < nRow; ++r)
    {
        SAutoRefPtr<IGridCellCheck> pChk = pGrid->GetCellOp<IGridCellCheck>(r, nCol - 1);
        if (pChk && pChk->GetCheck())
            ++nChecked;
    }
    SStringT str;
    str.Format(_T("单元格 %d 行 × %d 列 ｜ 勾选 %d 项"), nRow, nCol, nChecked);
    setLabel(_T("tbl_status"), str);
}

void CWasmHostWnd::OnTblAddRow()
{
    SGridCtrl *pGrid = FindChildByName2<SGridCtrl>(_T("tbl_grid"));
    if (!pGrid)
        return;
    int nNew = pGrid->GetRowCount();
    if (!pGrid->SetRowCount(nNew + 1))
        return;
    pGrid->SetItemText(nNew, 0, _T("新增"));
    pGrid->SetItemText(nNew, 1, _T("宿主插入"));
    pGrid->SetItemText(nNew, 2, SStringT().Format(_T("%d"), nNew * 37).c_str());
    pGrid->SetItemText(nNew, 3, _T("SetRowCount + SetItemText"));
    if (pGrid->SetCellType(nNew, kGridCheckCol, CELL_TYPE_CHECKBOX))
    {
        SAutoRefPtr<IGridCellCheck> pChk = pGrid->GetCellOp<IGridCellCheck>(nNew, kGridCheckCol);
        if (pChk)
            pChk->SetCheck(TRUE);
    }
    pGrid->Invalidate();
    refreshGridStatus();
}

void CWasmHostWnd::OnTblToggleCheck()
{
    SGridCtrl *pGrid = FindChildByName2<SGridCtrl>(_T("tbl_grid"));
    if (!pGrid)
        return;
    for (int r = 1; r < pGrid->GetRowCount(); ++r)
    {
        SAutoRefPtr<IGridCellCheck> pChk = pGrid->GetCellOp<IGridCellCheck>(r, kGridCheckCol);
        if (pChk)
            pChk->SetCheck(pChk->GetCheck() ? FALSE : TRUE);
    }
    pGrid->Invalidate();
    refreshGridStatus();
}

/* ------------------------------------------------------------------------- */
/* 图表页：HelloChart 的数据模型                                               */
/* ------------------------------------------------------------------------- */

/* 数值全部由 seq 推出来，不用 rand()/time()：无头烟测要在同一份构建上复现同一个
 * 结果，点数和数值都得是确定的。seq 每点一次"换一批数据"加 1，折线点数在 5..8 之间
 * 循环，状态行随之变化——点数不变就说明按钮或数据交接根本没生效。 */
static SLineChartData *makeLineData(int seq)
{
    int nPt = 5 + seq % 4;
    SLineChartData *pData = new SLineChartData;

    SLine *pUp = new SLine;
    for (int i = 0; i < nPt; ++i)
        pUp->AddValue(new SPointValue((float)i, (float)(15 + (i * (seq + 3) * 7) % 70)));
    pUp->SetColor(RGBA(37, 99, 235, 255));
    pUp->SetStrokeWidth(2);
    pUp->SetCubic(TRUE);
    pUp->SetHasPoints(TRUE);
    pData->AddLine(pUp);

    SLine *pFill = new SLine;
    for (int i = 0; i < nPt; ++i)
        pFill->AddValue(new SPointValue((float)i, (float)(45 - (i * (seq + 2) * 5) % 30)));
    pFill->SetColor(RGBA(147, 197, 253, 160));
    pFill->SetStrokeWidth(1);
    pFill->SetFilled(TRUE);
    pData->AddLine(pFill);

    /* 轴的所有权同样交给数据对象（SChartData::~SChartData 里 delete），
     * 所以这里 new 完直接 Set，不再自己释放。 */
    SAxis *pAxisX = new SAxis;
    pAxisX->SetHasLines(TRUE);
    pAxisX->SetName(_T("序号"));
    for (int i = 0; i < nPt; ++i)
        pAxisX->AddValue(new SAxisValue((float)i, SStringT().Format(_T("%d"), i).c_str()));
    pData->SetAxisX(pAxisX);

    SAxis *pAxisY = new SAxis;
    pAxisY->SetHasLines(TRUE);
    pAxisY->SetName(_T("数值"));
    pData->SetAxisY(pAxisY);
    return pData;
}

static SColumnChartData *makeColumnData(int seq)
{
    int nCol = 4 + seq % 3;
    static const COLORREF kColors[] = {RGBA(51, 181, 229, 255), RGBA(76, 175, 80, 255),
                                       RGBA(255, 193, 7, 255), RGBA(156, 39, 176, 255),
                                       RGBA(255, 87, 87, 255), RGBA(37, 99, 235, 255)};
    SColumnChartData *pData = new SColumnChartData;
    for (int c = 0; c < nCol; ++c)
    {
        /* SColumn 的构造函数直接接管传入指针数组（析构时逐个 delete），
         * 所以本地 vector 只负责收集，不能再释放一次。 */
        std::vector<SSubcolumnValue *> vals;
        vals.push_back(new SSubcolumnValue((float)(10 + (c * (seq + 5) * 11) % 80)));
        SColumn *pCol = new SColumn(vals);
        pCol->SetColor(kColors[c % (int)(sizeof(kColors) / sizeof(kColors[0]))]);
        pCol->SetHasLabels(TRUE);
        pData->AddColumn(pCol);
    }
    SAxis *pAxisY = new SAxis;
    pAxisY->SetHasLines(TRUE);
    pData->SetAxisY(pAxisY);
    return pData;
}

static SPieChartData *makePieData(int seq)
{
    static const TCHAR *kLabel[] = {_T("SOUI 核心"), _T("controls.extend"), _T("components"),
                                    _T("swinx"), _T("third-part")};
    static const COLORREF kColors[] = {RGBA(37, 99, 235, 255), RGBA(5, 150, 105, 255),
                                       RGBA(245, 158, 11, 255), RGBA(239, 68, 68, 255),
                                       RGBA(124, 58, 237, 255)};
    int nSlice = 3 + seq % 3;
    SPieChartData *pData = new SPieChartData;
    for (int i = 0; i < nSlice; ++i)
    {
        int n = i < (int)(sizeof(kLabel) / sizeof(kLabel[0])) ? i : i % (int)(sizeof(kLabel) / sizeof(kLabel[0]));
        pData->AddValue(new SSliceValue((float)(12 + (i + 1) * (seq + 2) * 5 % 40), kColors[n], kLabel[n]));
    }
    pData->SetHasLabels(TRUE);
    return pData;
}

void CWasmHostWnd::fillChartDemo()
{
    if (!m_hWnd)
        return;
    SLineChartView *pLine = FindChildByName2<SLineChartView>(_T("chart_line"));
    /* SetLineChartData 会把旧数据 delete 掉并接管新数据，控件析构时再释放一次；
     * 所以这里 new 出来的对象不再由宿主持有引用。 */
    if (pLine)
        pLine->SetLineChartData(makeLineData(m_nChartSeq));

    SColumnChartView *pCol = FindChildByName2<SColumnChartView>(_T("chart_column"));
    if (pCol)
        pCol->SetColumnChartData(makeColumnData(m_nChartSeq));

    /* 饼图构造时 m_pData 是 NULL，不喂数据就是一片空白（折线和柱状至少有
     * dummy 数据），这正是必须接线的原因。 */
    SPieChartView *pPie = FindChildByName2<SPieChartView>(_T("chart_pie"));
    if (pPie)
    {
        pPie->SetPieChartData(makePieData(m_nChartSeq));
        pPie->SetCircleFillRatio(0.45f); /* 中间挖空做成环形 */
        pPie->SetLabelsEnabled(TRUE);
    }
    refreshChartStatus();
}

void CWasmHostWnd::refreshChartStatus()
{
    int nPt = 0, nCol = 0, nSlice = 0;
    SLineChartView *pLine = FindChildByName2<SLineChartView>(_T("chart_line"));
    if (pLine && pLine->GetLineChartData() && (int)pLine->GetLineChartData()->GetLineCount() > 0)
        nPt = (int)pLine->GetLineChartData()->GetLine(0)->GetValueCount();
    SColumnChartView *pCol = FindChildByName2<SColumnChartView>(_T("chart_column"));
    if (pCol && pCol->GetColumnChartData())
        nCol = (int)pCol->GetColumnChartData()->GetColumnCount();
    SPieChartView *pPie = FindChildByName2<SPieChartView>(_T("chart_pie"));
    if (pPie && pPie->GetPieChartData())
        nSlice = (int)pPie->GetPieChartData()->GetValueCount();

    SStringT str;
    str.Format(_T("折线 %d 点 ｜ 柱 %d 组 ｜ 扇区 %d 块"), nPt, nCol, nSlice);
    setLabel(_T("chart_status"), str);
}

void CWasmHostWnd::OnChartRandom()
{
    ++m_nChartSeq;
    fillChartDemo();
}

void CWasmHostWnd::OnChartAnim()
{
    SLineChartView *pLine = FindChildByName2<SLineChartView>(_T("chart_line"));
    if (pLine)
        pLine->StartDataAnimation(400);
    SColumnChartView *pCol = FindChildByName2<SColumnChartView>(_T("chart_column"));
    if (pCol)
        pCol->StartDataAnimation(400);
    SPieChartView *pPie = FindChildByName2<SPieChartView>(_T("chart_pie"));
    if (pPie)
        pPie->StartDataAnimation(400);
}

void CWasmHostWnd::fillHexDemo()
{
    if (!m_hWnd)
        return;
    SHexEdit *pHex = FindChildByName2<SHexEdit>(_T("hex_demo"));
    if (!pHex)
        return;
    /* XML 里只有 AddrBkColor/HexBkColor/AsciiBkColor 三个背景色属性，文字色要靠
     * Set*Col：不设就是 Scintilla 的默认深色文字，在深色底上读不出来。
     * 前两个参数是 (背景, 文字)，bUpdate 留默认 false，跟下面的 SetData 一起重绘。 */
    pHex->SetAdrCol(RGBA(0xf1, 0xf5, 0xf9, 255), RGBA(0x47, 0x55, 0x69, 255));
    pHex->SetHexCol(RGBA(0xff, 0xff, 0xff, 255), RGBA(0x1f, 0x29, 0x37, 255));
    pHex->SetAsciiCol(RGBA(0xec, 0xfd, 0xf5, 255), RGBA(0x06, 0x5f, 0x46, 255));
    BYTE data[128];
    for (int i = 0; i < (int)sizeof(data); ++i)
        data[i] = (BYTE)i;
    pHex->SetData(data, sizeof(data));
}

/* <listctrl> 的行不支持内联 <item>（表头可以，见 XML 里的 headerStyle/items），
 * 数据必须由宿主喂，桌面 demo 也是这样。这里填四行两列，用来证明表头、选中带与
 * 滚动条在 wasm 上都能正常布局——表格里空着和"控件没建出来"在画面上一样。 */
void CWasmHostWnd::fillListDemo()
{
    if (!m_hWnd)
        return;
    SListCtrl *pList = FindChildByName2<SListCtrl>(_T("lc_demo"));
    if (!pList)
        return;
    static const TCHAR *kNames[] = {_T("SWindow"), _T("STreeCtrl"), _T("SScintillaView"), _T("SHexEdit")};
    static const TCHAR *kKinds[] = {_T("SOUI 核心"), _T("SOUI 核心"), _T("controls.extend"), _T("controls.extend")};
    for (int i = 0; i < (int)(sizeof(kNames) / sizeof(kNames[0])); ++i)
    {
        pList->InsertItem(i, kNames[i]);
        pList->SetSubItemText(i, 1, kKinds[i]);
    }
}

/* ------------------------------------------------------------------------- */
/* 动画页：SAnimation / SValueAnimator / SPropAnimator                         */
/* ------------------------------------------------------------------------- */

/* 把 anim: 资源挂到目标窗口。SetAnimation 会接管一份引用（m_animation），所以
 * 加载出来的这一份要立刻释放；同一窗口上已有动画时 SetAnimation 内部会先
 * ClearAnimation。动画播完由目标窗口自己的 SAnimationHandler 收尾，XML 里没有
 * 设 fillAfter 就不残留变换。 */
static void playTween(SWindow *pTarget, LPCTSTR pszResId)
{
    if (!pTarget)
        return;
    IAnimation *pAni = SApplication::getSingletonPtr()->LoadAnimation(pszResId);
    if (!pAni)
        return;
    pTarget->SetAnimation(pAni);
    pAni->Release();
}

void CWasmHostWnd::OnAnimTween()
{
    if (!m_hWnd)
        return;
    /* 三个目标窗口各挂一条 SAnimation，同时起步：
     *   alpha    先淡出再淡入（两段 <alpha> 用 startOffset 串联）
     *   rotate   绕自身中心转一圈
     *   translate 左移 80dp 再回来
     * 三份 XML 都在 uires.idx 的 <anim> 组里，名字与 <file name="..."> 一致。 */
    playTween(findChild(_T("tween_alpha")), _T("anim:anim_demo_alpha"));
    playTween(findChild(_T("tween_rotate")), _T("anim:anim_demo_rotate"));
    playTween(findChild(_T("tween_translate")), _T("anim:anim_demo_translate"));
    setLabel(_T("anim_status"), _T("SAnimation：alpha / rotate / translate 三个目标窗口同时播放"));
}

void CWasmHostWnd::OnAnimValue()
{
    if (!m_hWnd)
        return;
    /* 值动画只产生值，不碰控件——应用逻辑在 onAnimationUpdate 里做。
     * 重复点击时先 end() 上一次的动画：end() 会把值推到终态并触发
     * onAnimationEnd，之后容器释放引用，这里换成新加载的一份。 */
    if (m_valueColorAni && m_valueColorAni->isRunning())
        m_valueColorAni->end();
    if (m_valueFloatAni && m_valueFloatAni->isRunning())
        m_valueFloatAni->end();

    SApplication *pApp = SApplication::getSingletonPtr();
    m_valueColorAni.Attach(pApp->LoadValueAnimator(_T("valueAni:anim_demo_color")));
    if (m_valueColorAni)
    {
        m_valueColorAni->addUpdateListener(this);
        m_valueColorAni->start(this);
    }
    m_valueFloatAni.Attach(pApp->LoadValueAnimator(_T("valueAni:anim_demo_float")));
    if (m_valueFloatAni)
    {
        m_valueFloatAni->addUpdateListener(this);
        m_valueFloatAni->start(this);
    }
    setLabel(_T("anim_status"), _T("SValueAnimator：colorAnimator 换底色，floatAnimator 推进行度条"));
}

void CWasmHostWnd::OnAnimProp()
{
    SWindow *pBox = findChild(_T("prop_box"));
    if (!pBox)
        return;

    /* 三个关键帧的起点与 XML 里的原样一致（110dp 宽 / 完全不透明 / 橙色），
     * 所以动画开始不会跳。中间帧改宽度、透明度和底色，结束回到起点，
     * 连点几次画面也不会漂移。
     *   WIDTH       布局属性，写回 ILayoutParam 后由父 vbox 调 UpdateChildrenPosition
     *   ALPHA       窗口属性，要求 PROP_TYPE_BYTE（ofByte）
     *   COLOR_BKGND 窗口属性，直接改样式并重绘 */
    SLayoutSize widths[] = {SLayoutSize(110.0f, dp), SLayoutSize(210.0f, dp), SLayoutSize(110.0f, dp)};
    BYTE alphas[] = {255, 96, 255};
    COLORREF colors[] = {RGBA(245, 158, 11, 255), RGBA(236, 72, 153, 255), RGBA(245, 158, 11, 255)};

    IPropertyValuesHolder *holders[3];
    holders[0] = SPropertyValuesHolder::ofLayoutSize(LayoutProperty::WIDTH, widths, 3);
    holders[1] = SPropertyValuesHolder::ofByte(WindowProperty::ALPHA, alphas, 3);
    holders[2] = SPropertyValuesHolder::ofColorRef(WindowProperty::COLOR_BKGND, colors, 3);

    if (m_propAni && m_propAni->isRunning())
        m_propAni->end();

    /* ofPropertyValuesHolder 对每个 holder 各 AddRef 一次，所以动画器建出来后
     * 本地这三份创建引用要还掉（做法同 demos/propanimator_demo）。 */
    m_propAni.Attach(SPropertyAnimator::ofPropertyValuesHolder(pBox, holders, 3));
    for (int i = 0; i < 3; ++i)
        holders[i]->Release();

    if (m_propAni)
    {
        m_propAni->setDuration(1200);
        m_propAni->start(this);
    }
    setLabel(_T("anim_status"), _T("SPropAnimator：width / alpha / colorBkgnd 一组关键帧，共 1200ms"));
}

/* 值动画每帧回调（约 10ms 一拍）。动画器不绑定控件，值由这里消费：
 * 颜色动画把当前 COLORREF 变回 RGBA 串写进 value_swatch 的 colorBkgnd，
 * 浮点动画把 0..1 映射成进度条的 0..100 与数值文字。 */
void CWasmHostWnd::onAnimationUpdate(IValueAnimator *pAnimator)
{
    if (SColorAnimator *pColorAni = sobj_cast<SColorAnimator>(pAnimator))
    {
        SWindow *pSwatch = findChild(_T("value_swatch"));
        if (pSwatch)
        {
            SColor cr(pColorAni->getValue());
            SStringW strColor;
            strColor.Format(L"RGBA(%d,%d,%d,%d)", cr.r, cr.g, cr.b, cr.a);
            pSwatch->SetAttribute(L"colorBkgnd", strColor);
        }
        return;
    }
    if (SFloatAnimator *pFloatAni = sobj_cast<SFloatAnimator>(pAnimator))
    {
        float fValue = pFloatAni->getValue();
        int nProgress = (int)(fValue * 100.f + 0.5f);
        SProgress *pProgress = FindChildByName2<SProgress>(_T("value_progress"));
        if (pProgress)
            pProgress->SetValue(nProgress);
        SStringT str;
        str.Format(_T("floatAnimator: %d / 100"), nProgress);
        setLabel(_T("value_num"), str);
    }
}

/* ------------------------------------------------------------------------- */
/* 模态表单页：SModalRoot + SModalView                                         */
/* ------------------------------------------------------------------------- */

/* 打开 xml/modal_form.xml 的表单。wasm 不能用 SHostDialog::DoModal：内层
 * PeekMessage 拿不到浏览器的新事件，模态必须挂在 SRootWindow 的模态容器里，
 * 由普通命中测试拦截输入（SModalRoot/SModalView 就是为此设计的）。
 *
 * 引用计数：BeginModalViewSession(pszLayout) 内部 CreateWindowByName 得到的那份
 * 引用在插进窗口树时转交给父节点，模态栈自己再 AddRef 一份；返回的指针不额外
 * 持有引用，所以这里不 Release、也不存到成员里。表单控件指针只在本次会话内
 * 使用——会话结束时 SHostWnd 会移除并销毁整棵子树。 */
void CWasmHostWnd::OnModalOpen()
{
    if (!m_hWnd)
        return;
    /* 同一时刻只留一个演示会话：栈顶已有会话时再开一个会叠遮罩，
     * 而且这里订阅的 exitCode 无法和旧会话区分。 */
    if (GetLastModalViewSessionID() != 0)
    {
        setLabel(_T("modal_status"), _T("已有一个表单会话在进行中，请先提交或取消"));
        return;
    }

    SModalRoot *pModal = BeginModalViewSession(_T("layout:XML_MODAL_FORM"));
    if (!pModal)
    {
        setLabel(_T("modal_status"), _T("打开失败：layout:XML_MODAL_FORM 没有加载出来"));
        return;
    }

    ModalViewSessionID sessionID = pModal->GetSessionID();
    setLabel(_T("modal_status"), _T("表单已打开，主界面输入已被 SModalRoot 拦截"));
    SEdit *pName = pModal->FindChildByName2<SEdit>(_T("modal_name"));
    SEdit *pEmail = pModal->FindChildByName2<SEdit>(_T("modal_email"));
    SComboBox *pDept = pModal->FindChildByName2<SComboBox>(_T("modal_dept"));
    SWindow *pErr = pModal->FindChildByName(_T("modal_err"));

    SWindow *pSubmit = pModal->FindChildByName(_T("btn_modal_submit"));
    if (pSubmit)
    {
        pSubmit->SubscribeEvent(EventCmd::EventID, [=](IEvtArgs *e) {
            SStringT strName = pName ? pName->GetWindowText() : SStringT();
            SStringT strEmail = pEmail ? pEmail->GetWindowText() : SStringT();
            if (strName.IsEmpty() || strEmail.IsEmpty())
            {
                if (pErr)
                    pErr->SetWindowText(strName.IsEmpty() ? _T("姓名不能为空")
                                                          : _T("邮箱不能为空"));
                return TRUE;
            }
            /* IDOK 随 EventExitModalView 传回；退场动画播完才真正清理窗口，
             * 所以退出处理里仍能读到表单控件的值。 */
            EndModalViewSession(sessionID, IDOK);
            return TRUE;
        });
    }

    SWindow *pCancel = pModal->FindChildByName(_T("btn_modal_cancel"));
    if (pCancel)
    {
        pCancel->SubscribeEvent(EventCmd::EventID, [=](IEvtArgs *e) {
            EndModalViewSession(sessionID, IDCANCEL);
            return TRUE;
        });
    }

    pModal->SubscribeEvent(EventExitModalView::EventID, [=](IEvtArgs *e) {
        EventExitModalView *pExit = sobj_cast<EventExitModalView>(e);
        if (!pExit)
            return TRUE;
        if (pExit->exitCode == IDOK && pName && pEmail && pDept)
        {
            SStringT strDept;
            int nDept = pDept->GetCurSel();
            pDept->GetItemText(nDept, FALSE, &strDept);
            SStringT strResult;
            strResult.Format(_T("已提交：%s / %s / %s"), pName->GetWindowText().c_str(),
                             pEmail->GetWindowText().c_str(), strDept.c_str());
            setLabel(_T("modal_status"), strResult);
        }
        else
        {
            setLabel(_T("modal_status"), _T("已取消，表单未提交"));
        }
        return TRUE;
    });
}

void CWasmHostWnd::noteDisabledFeatures()
{
#if !SOUI_WASM_HAS_LUA
    setLabel(_T("lua_hint"),
             _T("本构建没带 Lua 脚本模块（-DSOUI_BUILD_WASM_LUA=OFF）：这一页的三个游戏（消消乐 / 俄罗斯方块 / 坦克大战）整页内容都由 lua/games.lua 在 on_init 里建，现在只剩一行说明和空的页签。"));
#endif
#if !SOUI_WASM_HAS_SCINTILLA
    setLabel(_T("edit_hint"),
             _T("本构建没带 Scintilla（-DSOUI_BUILD_WASM_SCINTILLA=OFF）：下面的 <scintilla> 标签没有注册窗口类，会被布局静默跳过。richedit(<edit>) 和 hexedit 不受这个开关影响。"));
#endif
}

/* ------------------------------------------------------------------------- */
/* 宿主接线：上屏、消息泵、输入注入                                          */
/* ------------------------------------------------------------------------- */
static void host_fatal(const char *why)
{
    printf("[soui-wasm] FATAL: %s\n", why);
    EM_ASM({
        if (typeof Module.souiFatal === 'function')
            Module.souiFatal(UTF8ToString($0));
    }, why);
}

/* 上屏回调：commitCanvas() 之后被调用，脏矩形是屏幕坐标；canvas 像素 == 屏幕坐标
 * （窗口建在 (0,0) 且尺寸等于帧缓冲），所以 JS 侧不需要再加原点换算。 */
static void host_present(const RECT *dirty)
{
    if (!dirty)
        return;
    EM_ASM({
        if (typeof Module.swinxPresent === 'function')
            Module.swinxPresent($0, $1, $2, $3);
    }, dirty->left, dirty->top, dirty->right, dirty->bottom);
}

extern "C"
{

    /* 导出给 soui.html 的宿主入口只有六个功能口：
     *   SwinxDemoPump        消息泵（emscripten_set_main_loop 的回调）
     *   SwinxDemoMouse/Key   浏览器 pointer/keyboard 事件注入
     *   SwinxDemoWheel       滚轮注入（WM_MOUSEWHEEL 的 wParam 打包与鼠标不同）
     *   SwinxDemoText        输入法成品文本注入（compositionend → WM_IME_CHAR）
     *   SwinxDemoCaretPos    读插入符位置，页面把 IME 代理框钉在光标处
     * 连同 framebuffer.h 的 SwinxFb*（帧缓冲地址/尺寸/上屏计数）。 */
    void SwinxDemoPump(void)
    {
        MSG msg;
        // 一次 pump 只工作有限时间，剩下的消息留给下一次调用。
        // SOUI 的动画脉冲是 10ms 一个 tick（ITimelineHandler::kPulseInterval），
        // 而 wasm 里一帧重绘要几十毫秒，所以只要有动画在跑，每次 PeekMessage 都
        // 发现定时器已过期、于是重新塞一条 WM_TIMER，紧跟着 InvalidateRect 又塞
        // 一条 WM_PAINT —— 队列永远非空。不限时的话这个 while 永不退出：浏览器里
        // SwinxDemoPump 就是 requestAnimationFrame 回调本身（见 main 里的
        // emscripten_set_main_loop），它不返回等于整个标签页冻住。有限动画（特效）
        // 播完就停，看不出问题；repeatCount="-1" 的选中脉冲是无限动画，一点「提示」
        // 就永久卡死。
        const double tDeadline = emscripten_get_now() + kPumpBudgetMs;
        while (PeekMessageA(&msg, NULL, 0, 0, PM_REMOVE))
        {
            if (msg.message == WM_QUIT)
                break;
            TranslateMessage(&msg);
            DispatchMessageA(&msg);
            // 至少派发一条再判超时：单条 WM_PAINT 本身就可能超过预算，
            // 提前退出会让画面一帧都不动。
            if (emscripten_get_now() >= tDeadline)
                break;
        }
    }

    void SwinxDemoMouse(UINT message, int x, int y, WPARAM fwKeys)
    {
        SwinxInjectMouse(g_hWnd, message, x, y, fwKeys);
    }

    /* 滚轮也是鼠标消息，但 wParam 布局不同：低字仍是 MK_ 键位，高字是带符号的
     * 滚动量。SwinxInjectMouse 会把这个值原样交给窗口，所以由这里打包（不新增
     * swinx 接口）。zDelta 已由页面换算成 WHEEL_DELTA 的整数倍。 */
    void SwinxDemoWheel(int x, int y, int zDelta, UINT fwKeys)
    {
        SwinxInjectMouse(g_hWnd, WM_MOUSEWHEEL, x, y, MAKEWPARAM((WORD)fwKeys, (WORD)zDelta));
    }

    void SwinxDemoKey(UINT message, UINT vk, UINT scanCode)
    {
        SwinxInjectKey(g_hWnd, message, vk, scanCode);
    }

    /* 输入法提交的文本（UTF-8）。JS 侧在 compositionend / insertText 上拿到成品
     * 字符串，交给 swinx 逐码点发 WM_IME_CHAR。为什么不发 WM_CHAR：字符是输入法
     * 给的，不是按键推算出来的，而 TranslateMessage 那条路只能按键盘布局猜（见
     * swinx wasm SConnection.cpp 的 vkToChar）。也不动键盘状态：一次提交不是一次
     * 物理按键，伪造 down/up 会让 GetKeyState 和编辑框的 Ctrl 组合判断跟着错。 */
    void SwinxDemoText(const char *pszUtf8)
    {
        SwinxInjectText(g_hWnd, pszUtf8);
    }

    /* 当前插入符位置（宿主窗口坐标 == 帧缓冲坐标：窗口建在 (0,0)）。SOUI 的 SCaret
     * 在每次光标移动时调 ::SetCaretPos，值就存在 wasm SConnection 里。JS 侧用它把
     * 隐藏的组合输入代理钉在光标处，浏览器的候选词窗才会出现在文字旁边——否则
     * 候选窗跟着一个固定在画布外的代理，用户看不见自己正在往哪儿输入。
     * 返回 0 表示还没有窗口创建过插入符（焦点不在编辑类控件上时就是这样）。 */
    int SwinxDemoCaretPos(int *pXY)
    {
        POINT pt;
        if (!::GetCaretPos(&pt))
            return 0;
        if (pXY)
        {
            pXY[0] = pt.x;
            pXY[1] = pt.y;
        }
        return 1;
    }

}

int main()
{
    /* HiDPI（SOUI 原生方案）：浏览器 devicePixelRatio 向上贴到内置档位，然后
     *   1) SwinxSetCanvasScale(nScale/100)：帧缓冲的物理分辨率 = 逻辑 800x600 x
     *      档位，swinx SConnection::GetDpi 按 96 基准报告同一档位（URL 加 ?dpr=1
     *      可强制 100% 供对比）；
     *   2) 缩放本体走 SDpiHandler（参考 games/cnchess/client/MainDlg.h）：建窗后
     *      显式发 WM_INITDIALOG → ScaleHost 从 GetWindowScale 还原档位 →
     *      HandleScaleChange（见 CWasmHostWnd，desRect 钉在 fb 物理矩形）→
     *      SHostWnd::SetScale 广播 UM_SETSCALE，布局/字体/皮肤全部按档位放大
     *      —— GDI 层不做任何缩放，绘制坐标就是物理像素。
     *
     * 档位取内置档位（100/125/150/200/250/300）里**不小于** dpr 的最小那一档，即向上贴：
     *   · 帧缓冲 ≥ 真实设备像素 ⇒ 浏览器只会把画布缩下来（超采样），永远不会拉伸发虚；
     *   · 与内置皮肤目录（125/150…）对齐，SSkinPool 挑资源时不用再降级。
     * 但不能一路往上贴：超采样是拿填充率换锐度（面积按倍率的平方涨），所以倍率超过
     * kMaxOverSample 就退化成 dpr 的精确百分点 —— 那也已经是 1:1 设备像素，不会糊。
     * 不能过 SDpiScale::NormalizeScale —— 它是**向下**贴，浏览器缩放 110%/120% 会被压成
     * 100%，帧缓冲只有 800x600 却被按 1.1 倍物理像素放大，表现就是标题栏 sc:100 + 整页发虚。
     * 典型例子：Windows 125% + 浏览器缩放 90% ⇒ dpr 1.125，向上贴到 125%（画 1000x750、
     * 由浏览器缩到 900x675），既保住观感尺寸，又拿到 1.11 倍超采样的锐度。 */
    int nScale = 100;
    {
        double scale = 0.0;
        const char *q = emscripten_run_script_string(
            "location.search.includes('dpr=') ? location.search.split('dpr=')[1].split('&')[0] : ''");
        if (q && *q)
            scale = atof(q);
        else
        {
            double dpr = emscripten_get_device_pixel_ratio();
            if (dpr > 0.5)
                scale = dpr;
        }
        if (scale <= 0.5)
            scale = 1.0;
        int nExact = (int)(scale * 100.0 + 0.5);
        if (nExact < 100)
            nExact = 100; /* 缩小屏（dpr<1）不缩排：字会糊，直接按 100% 画 */
        const int *pScales = SDpiScale::GetBuiltinScales();
        int nCount = SDpiScale::GetBuiltinScaleCount();
        nScale = pScales[nCount - 1];
        for (int i = 0; i < nCount; i++)
        {
            if (pScales[i] >= nExact)
            {
                nScale = pScales[i];
                break;
            }
        }
        /* 超采样倍率上限：1.15 时像素面积已 +32%，再往上就是白烧填充率。 */
        const double kMaxOverSample = 1.15;
        if (nScale > nExact * kMaxOverSample)
            nScale = nExact;
        g_nWasmDpiScale = nScale;
        SwinxSetCanvasScale(nScale / 100.0);
        printf("[soui-wasm] dpi dpr=%.3f -> scale=%d%% (fb %dx%d physical)\n", scale, nScale,
               SwinxFbWidth(), SwinxFbHeight());
    }

    /* 先把底色刷进帧缓冲、再挂上屏回调，最后才建窗口：顺序反过来的话
     * Create 里那次同步绘制会因为回调为空而丢帧。 */
    SwinxFbClear(0xFFF4F6FAu);
    SwinxSetFbPresentCb(host_present);

    /* 字体是可选的：build-soui.sh 没找到 TTF 时这里只是注册失败，
     * 控件树、皮肤和鼠标事件都照常工作，文字部分画不出字形。
     * 用通用名 AddFontResource：wasm 构建的 SOUI_USE_UNICODE=OFF，TCHAR 就是
     * char，宏会落到 AddFontResourceA。 */
    int nFontOk = AddFontResource(kFontPath) ? 1 : 0;
    printf("[soui-wasm] font=%d path=%s\n", nFontOk, kFontPath);

    /* SApplication 是进程级单例（SApplication::getSingletonPtr()），SHostWnd 的
     * 每一次绘制、每一次资源查找都要经过它。而 emscripten_set_main_loop 的
     * simulate_infinite_loop=1 会让 main() 以异常方式退出并展开栈——放在栈上的
     * app 会在主循环刚开始时就被析构。所以这里刻意 new 出来并且永不 delete：
     * 浏览器标签页关掉时整个 wasm 实例连同线性内存一起消失，不需要析构路径。 */
    SApplication *app = new SApplication(GetModuleHandle(NULL));
    printf("[soui-wasm] soui %d.%d\n", SOUI_VER1, SOUI_VER2);

    SAppCfg cfg;
    cfg.SetRender(Render_Gdi)
        .SetImgDecoder(ImgDecoder_Stb)
        .SetSysResFile(kSysResDir)
        .SetAppResFile(kAppResDir)
        /* app dir 只被 GetAppDir() 读到，脚本用它拼资源外的绝对路径（games.lua 的
         * 音效就是 GetAppDir() .. "/snd/x.wav"）。预载把 snd/ 挂在 /opt/soui/snd，
         * 所以这里必须与之一致；不设的话 GetAppDir() 是空串，拼出来是 "/snd/x.wav"，
         * swinx 的 PlaySound 打不开文件、返回 FALSE，游戏静默无声。 */
        .SetAppDir(_T("/opt/soui"))
        .SetUidefId(_T("uidef:xml_init"));
    if (!cfg.DoConfig(app))
    {
        host_fatal("SAppCfg::DoConfig() 失败：渲染器/图片解码器/资源目录三者之一没起来");
        return 1;
    }
    if (!app->GetRenderFactory())
    {
        host_fatal("DoConfig 通过但 render factory 为空");
        return 1;
    }
    printf("[soui-wasm] render=%p\n", (void *)app->GetRenderFactory());

    /* 扩展控件（controls.extend）不是自注册的：桌面构建由各 demo 自己调
     * SCtrlsRegister::RegisterCtrls，这里只有这一处调用点。必须在 Create() 解析
     * 布局之前完成，否则 <scintilla>/<progressRing> 这些标签在 SApp.cpp 的
     * RegisterWindows 里找不到对应窗口类，控件会被跳过（不报错，只是消失）。 */
    SCtrlsRegister::RegisterCtrls(app);
    /* <progressRing> 不在 SCtrlsRegister 的名单里：桌面 demo 是在 demo.cpp 里自己
     * RegisterWindowClass 的。不注册的话标签被静默跳过，画面上只剩空位。 */
    app->RegisterWindowClass<SProgressRing>();
    printf("[soui-wasm] extend ctrls registered\n");

    /* 脚本工厂：顺序不能动。SHostWnd::Create() 内部解析 <script src="lua:..."> 时才
     * 调 SApplication::CreateScriptModule()，工厂这时必须已经在 app 上。浏览器里没有
     * COM，SCreateInstance 直接 new 出 SIScriptFactory；SetScriptFactory 内部持有
     * 一份引用，这里的局部引用随即释放。
     * SOUI_BUILD_WASM_LUA=OFF 时这三样都不在构建图里，布局里的 <script> 节点会走到
     * SApplication::CreateScriptModule() 的 "没有工厂就返回 E_FAIL" 分支（SApp.cpp:760），
     * 是个无害的 no-op —— 所以 XML 不需要跟着开关变。 */
#if SOUI_WASM_HAS_LUA
    IObjRef *pScriptFactory = NULL;
    if (Script_Lua_SCreateInstance(&pScriptFactory) && pScriptFactory)
    {
        app->SetScriptFactory(static_cast<IScriptFactory *>(pScriptFactory));
        pScriptFactory->Release();
        printf("[soui-wasm] script factory=lua\n");
    }
    else
    {
        host_fatal("Script_Lua_SCreateInstance() 失败：脚本页面会没有脚本引擎");
    }
#else
    printf("[soui-wasm] script factory=none (SOUI_BUILD_WASM_LUA=OFF)\n");
#endif

    /* 宿主窗口建在**逻辑**尺寸（800x600）：DPI 缩放由 SDpiHandler 在
     * WM_INITDIALOG 里接手——ScaleHost 把窗口矩形放大到物理分辨率（SetWindowPos
     * 触发 WM_SIZE，swinx 原地把窗口 canvas 重建为物理尺寸）并广播 UM_SETSCALE。 */
    g_pHost = new CWasmHostWnd();
    g_hWnd = g_pHost->Create(NULL, 0, 0, SwinxFbLogicalWidth(), SwinxFbLogicalHeight());
    if (!g_hWnd)
    {
        /* GetLastError 是这里唯一有用的线索：Create() 里 SNativeWnd::CreateNative
         * 失败（窗口类/样式问题）和 InitFromXml 失败（布局 XML 问题）都只返回
         * NULL，SOUI 自己不报错。 */
        char msg[192];
        snprintf(msg, sizeof(msg),
                 "SHostWnd::Create() 失败 (err=%lu)：布局 XML 或控件类注册有问题",
                 (unsigned long)GetLastError());
        host_fatal(msg);
        return 2;
    }
    /* wasm 的 Create 不发 WM_INITDIALOG（SHostDialog 才发）：参考 games/cnchess
     * 显式发一次，SDpiHandler 的 ScaleHost 在这里触发（读 GetDpiForWindow 还原
     * 档位 → SetScale 广播）。 */
    g_pHost->SendMessage(WM_INITDIALOG);
    g_pHost->ShowWindow(SW_SHOW);
    g_pHost->fillBrand();
    g_pHost->fillHexDemo();
    g_pHost->fillListDemo();
    /* 数据控件页都靠宿主喂内容：MVC 的行来自适配器（列表与树）、gridctrl 的单元格来自
     * SetItemText、图表的数据模型来自 SetXxxChartData。不接线的话页面上要么
     * 只有空模板，要么饼图整块是空白。grouplist 的条目文字不走这里：
     * 它的事件在布局解析期间就发完了，由 EVENT_MAP 直接接住。 */
    g_pHost->setupMvcDemo();
    g_pHost->fillGridDemo();
    g_pHost->fillChartDemo();
    g_pHost->noteDisabledFeatures();

    /* fps=0 跟随 requestAnimationFrame；simulate_infinite_loop=1 让 main() 以
     * "主循环已被接管"的方式退出，JS 侧的 Promise 仍会 resolve。 */
    emscripten_set_main_loop(SwinxDemoPump, 0, 1);
    return 0;
}
