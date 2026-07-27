#ifndef __SGAMETHEME_H__
#define __SGAMETHEME_H__
#include <map>
#include <layout/SAnchorLayout.h>


SNSBEGIN

/**
 * @brief 锚点名称空间
 * 
 * 定义游戏中各种元素的锚点名称和对应的索引值
 */
namespace AnchorName{
    /**
     * @brief 锚点索引枚举
     */
    enum{
        kAvatarSelf,        ///< 自己头像位置
        kAvatarEnemy,          ///< 对手头像位置

        kClockSelf,         ///< 自己时钟位置
        kClockEnemy,           ///< 对手时钟位置

        kCount                ///< 锚点总数
    };
    
    /**
     * @brief 锚点名称数组
     * 
     * 与锚点索引枚举对应的实际名称字符串数组
     */
static const wchar_t *kElementNames[] = {
    L"avatar_self",  // 自己头像
    L"avatar_enemy",    // 对手头像

    L"clock_self",   // 自己时钟
    L"clock_enemy",      // 对手时钟
};
}

/**
 * @brief 精灵元素命名空间
 * 
 * 定义游戏中使用的各种精灵元素名称
 */
namespace Sprites{

    static const wchar_t* connecting = L"sprite_connecting";     ///< 连接动画
    static const wchar_t* sprite_waiting = L"sprite_waiting";    ///< 等待动画
    static const wchar_t* sprite_vectory = L"sprite_vectory";    ///< 胜利动画
    static const wchar_t* img_background = L"img_background";   ///< 背景图片

    static const wchar_t* avatar_self = L"avatar_self";     ///< 自己头像
    static const wchar_t* avatar_enemy = L"avatar_enemy";   ///< 对手头像

    static const wchar_t* img_clock = L"img_clock";         ///< 时钟图片

    static const wchar_t* board_main = L"board_main";       ///< 棋盘主界面
}

/**
 * @brief 皮肤命名空间
 * 
 * 定义游戏中使用的各种皮肤资源
 */
namespace Skins{
    static const wchar_t *kvectory[] = { L"ani_vectory_1", L"ani_vectory_2", L"ani_vectory_3" };       ///< 胜利动画皮肤
}

/**
 * @brief 动画命名空间
 * 
 * 定义游戏中使用的各种动画资源
 */
namespace Animations{
    static const wchar_t *kclock_alarm = L"clock_alarm";    ///< 时钟报警动画
}

namespace Widgets{
    static const wchar_t *kVectory = L"sprite_vectory";     ///< 胜利动画
    static const wchar_t *ktab_toolbar = L"tab_toolbar";    ///< 工具栏
    static const wchar_t *kflag_pos_to = L"sprite_flag_pos_to";    ///< 位置标志动画
    static const wchar_t *kflag_pos_from = L"img_flag_pos_from";
    static const wchar_t *kbtn_start = L"btn_start";
    static const wchar_t *kbtn_req_peace = L"btn_req_peace";
    static const wchar_t *kbtn_req_surrender = L"btn_req_surrender";
    static const wchar_t *kbtn_req_regret = L"btn_req_regret";
}

/**
 * @brief 音效命名空间
 * 
 * 定义游戏中使用的各种音效资源
 */
namespace Sounds{
    /**
     * @brief 特效音效命名空间
     * 
     * 定义游戏中使用的各种特效音效名称
     */
    namespace Effects{
        static const wchar_t *kenter_game = L"进入游戏";        ///< 进入游戏音效
        static const wchar_t *kexit_game = L"退出游戏";         ///< 退出游戏音效
        static const wchar_t *kstart_game = L"开始游戏";        ///< 开始游戏音效
        static const wchar_t *kcount_down = L"倒计时";          ///< 倒计时音效
        static const wchar_t *kJiangjun = L"将军";              ///< 将军音效
        static const wchar_t *kGo = L"走子";                   ///< 走子音效
        static const wchar_t *kEat = L"吃子";                  ///< 吃子音效
        static const wchar_t *kDanger = L"danger";              ///< 危险音效
        static const wchar_t *kfireworks = L"焰火";             ///< 烟花音效
    }
}

namespace Template{
    static const wchar_t *kAvatar = L"avatar";
    static const wchar_t *kTip = L"tip";
    static const wchar_t *kChessPiece = L"piece";
    static const wchar_t *kShadow = L"shadow";
    static const wchar_t *kPosFlag = L"posflag";
    static const wchar_t *kClock = L"clock";
}
/**
 * @brief 游戏主题类
 * 
 * 管理游戏的主题资源，包括皮肤、窗口部件、锚点位置、动画和音效等
 */
class SGameTheme : public TObjRefImpl<IObjRef>
{
public:

    /**
     * @brief 窗口层级枚举
     */
    enum{
        LAYER_PIESE = 10,       ///< 棋子
        LAYER_PIESE_UP = 20,    ///< 棋子抬起
        LAYER_POSFLAG = 5,      ///< 棋子位置标志
    };

public:
    /**
     * @brief 构造函数
     */
    SGameTheme();
    
    /**
     * @brief 析构函数
     */
    ~SGameTheme();

    /**
     * @brief 加载主题资源
     * @param pszThemeDir 主题目录路径
     * @return 是否加载成功
     */
    BOOL Load(LPCTSTR pszThemeDir);
    
    /**
     * @brief 获取窗口部件
     * @param pszName 部件名称
     * @return 窗口部件指针
     */
    IWindow *GetWidget(LPCWSTR pszName);
    
    /**
     * @brief 获取锚点位置
     * @param nIndex 锚点索引
     * @return 锚点位置
     */
    AnchorPos GetAnchorPos(int nIndex){return m_mapLayout[AnchorName::kElementNames[nIndex]];}
    
    /**
     * @brief 获取模板XML节点
     * @param pszName 模板名称
     * @return XML节点
     */
    SXmlNode GetTemplate(LPCWSTR pszName);
    
    /**
     * @brief 获取动画对象
     * @param pszName 动画名称
     * @return 动画对象指针
     */
    IAnimation *GetAnimation(LPCWSTR pszName);
    
    /**
     * @brief 获取特效音效文件路径
     * @param pszName 音效名称
     * @return 音效文件路径
     */
    SStringW GetEffectSoundFile(LPCWSTR pszName);
    
  private:
    SAutoRefPtr<ISkinPool> m_pSkinPool;                                      ///< 皮肤池
    std::map<SStringW, SAutoRefPtr<IWindow> > m_mapWidgets;                  ///< 窗口部件映射表
    std::map<SStringW, AnchorPos > m_mapLayout;                              ///< 布局锚点映射表
    std::map<SStringW, SAutoRefPtr<IAnimation> > m_mapAnimations;            ///< 动画映射表
    SXmlDoc m_docTemplate;                                                   ///< 模板XML文档
    SXmlDoc m_docSounds;                                                     ///< 音效XML文档
};

SNSEND

#endif//__SGAMETHEME_H__