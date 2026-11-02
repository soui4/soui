/**
 * @file       test_soui_uitest_common.h
 * @brief      fun_test 侧 SouiTest 夹具：无头宿主 + XML 布局 + 控件树定位
 *
 * 复用 fun_test 既有的无头基座（test_container.h）：
 *   - funtest::TestContainer : 继承 SwndContainerImpl，桩掉绘制/宿主纯虚函数
 *   - funtest::HeadlessApp   : 创建 SApplication + GDI 渲染工厂 + 内置 skin
 *   - funtest::CreateRootFromXml : 先 SetContainer 再 InitFromXml（事件才能派发）
 *
 * 该夹具让 Tier A（SSendMessage 框架级）用例在无显示环境下跑，归入 soui-headless 门禁。
 * 注意：无头模式下 FireEvent 同步派发（GetContainer 非空即分发），无需消息泵收敛；
 * 因此本夹具不提供 pump()——若将来需要驱动异步定时器/动画，再接 IMessageLoop。
 */
#ifndef __TEST_SOUITEST_COMMON_H__
#define __TEST_SOUITEST_COMMON_H__

#include <souistd.h>
#include <core/SWnd.h>
#include <event/SEvents.h>
#include <gtest/gtest.h>

#include <vector>

#include "souitest/souitest.h"
#include "test_container.h"

namespace souitest
{
using namespace SOUI; // 让类体内的 SWindow / CRect / SButton 等类型无需 SOUI:: 前缀

class SouiTestFixture : public ::testing::Test
{
  public:
    SouiTestFixture()
        : m_app(NULL)
        , m_container(NULL)
    {
    }

  protected:
    void SetUp()
    {
        m_app = new funtest::HeadlessApp();
        m_container = new funtest::TestContainer();
        // 所有 SouiTest 无头用例都先加载系统资源（内置皮肤），再跑用例。
        m_app->LoadSystemResource();
    }

    void TearDown()
    {
        // SWindow::~SWindow 不递归销毁子树，需先 DestroyAllChildren 再删根。
        for (size_t i = 0; i < m_roots.size(); ++i)
        {
            SWindow *root = m_roots[i];
            if (root)
            {
                root->DestroyAllChildren();
                delete root;
            }
        }
        m_roots.clear();
        delete m_container;
        m_container = NULL;
        delete m_app;
        m_app = NULL;
    }

    /**
     * @brief 从内联 XML 构建无头控件树并布局。
     * @param xml  根窗口 XML（<window ...>...</window> 形式）
     * @param rc   布局矩形；默认 800x600 视口
     * @return 根窗口指针（用 SouiTest::findByName/findByClass 在其下定位）
     */
    SWindow *loadUi(const wchar_t *xml, const CRect &rc = CRect(0, 0, 800, 600))
    {
        SWindow *root = new SWindow();
        m_roots.push_back(root);
        EXPECT_TRUE(funtest::CreateRootFromXml(*root, m_container, xml));
        root->SetVisible(TRUE);
        root->OnRelayout(rc);
        return root;
    }

    funtest::TestContainer *container() { return m_container; }

  private:
    funtest::HeadlessApp *m_app;
    funtest::TestContainer *m_container;
    std::vector<SWindow *> m_roots;
};
} // namespace souitest

using souitest::SouiTestFixture;

#endif // __TEST_SOUITEST_COMMON_H__
