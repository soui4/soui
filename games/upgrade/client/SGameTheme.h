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
        kCardCenter = 0,      ///< 底牌中心位置
        kScoreCard,           ///< 得分牌位置

        kCardBottom,          ///< 自己的手牌位置
        kCardLeft,            ///< 左手牌位置
        kCardTop,             ///< 对手牌位置
        kCardRight,           ///< 右手牌位置

        kAvatarBottom,        ///< 自己头像位置
        kAvatarLeft,          ///< 左手头像位置
        kAvatarTop,           ///< 对手头像位置
        kAvatarRight,         ///< 右手头像位置

        kPutCardBottom,       ///< 自己出牌区位置
        kPutCardLeft,         ///< 左手出牌区位置
        kPutCardTop,          ///< 对手出牌区位置
        kPutCardRight,        ///< 右手出牌区位置

        kClockBottom,         ///< 自己时钟位置
        kClockLeft,           ///< 左手时钟位置
        kClockTop,            ///< 对手时钟位置
        kClockRight,          ///< 右手时钟位置

        kColorBottom,         ///< 自己亮主花色位置
        kColorLeft,           ///< 左手亮主花色位置
        kColorTop,            ///< 对手亮主花色位置
        kColorRight,          ///< 右手亮主花色位置

        kCount                ///< 锚点总数
    };
    
    /**
     * @brief 锚点名称数组
     * 
     * 与锚点索引枚举对应的实际名称字符串数组
     */
static const wchar_t *kElementNames[] = {
    L"card_center",    // 底牌
    L"score_card",      // 得分牌

    L"card_bottom",    // 自己的手牌
    L"card_left",      // 左手牌
    L"card_top",       // 对手牌
    L"card_right",     // 右手牌

    L"avatar_bottom",  // 自己头像
    L"avatar_left",    // 左手头像
    L"avatar_top",     // 对手头像
    L"avatar_right",   // 右手头像

    L"putcard_bottom", // 自己出牌区
    L"putcard_left",   // 左手出牌区
    L"putcard_top",    // 对手出牌区
    L"putcard_right",  // 右手出牌区

    L"clock_bottom",   // 自己时钟
    L"clock_left",     // 左手时钟
    L"clock_top",      // 对手时钟
    L"clock_right",     // 右手时钟

    L"color_bottom",   // 自己亮主花色
    L"color_left",     // 左手亮主花色
    L"color_top",      // 对手亮主花色
    L"color_right",     // 右手亮主花色
};
}

/**
 * @brief 精灵元素命名空间
 * 
 * 定义游戏中使用的各种精灵元素名称
 */
namespace Sprites{
    /**
     * @brief 按钮ID枚举
     */
    enum{
        btn_diamond = 100,    ///< 方块按钮
        btn_heart,            ///< 红桃按钮
        btn_spade,            ///< 黑桃按钮
        btn_club,             ///< 梅花按钮
        btn_little_joker,     ///< 小王按钮
        btn_big_joker,        ///< 大王按钮
        btn_pass,             ///< 放弃按钮
        gif_highlight = 200,  ///< 高亮动画
    };
    
    static const wchar_t* connecting = L"sprite_connecting";     ///< 连接动画
    static const wchar_t* shuffle = L"sprite_shuffle";           ///< 洗牌动画
    static const wchar_t* btn_start = L"btn_start";              ///< 开始按钮
    static const wchar_t* sprite_waiting = L"sprite_waiting";    ///< 等待动画
    static const wchar_t* sprite_vectory = L"sprite_vectory";    ///< 胜利动画

    static const wchar_t* win_show_main_box = L"win_show_main_box";     ///< 亮主窗口
    static const wchar_t* btn_put_card = L"btn_put_card";               ///< 出牌按钮
    static const wchar_t* btn_set_bottom = L"btn_set_bottom";           ///< 扣底牌按钮
    static const wchar_t* win_score_container = L"win_score_container"; ///< 分数容器窗口
    static const wchar_t* btn_prev_round = L"btn_prev_round";           ///< 上一轮按钮
    static const wchar_t* btn_see_bottom = L"btn_see_bottom";           ///< 看底牌按钮

    static const wchar_t* avatar_bottom = L"avatar_bottom";     ///< 底部头像
    static const wchar_t* avatar_left = L"avatar_left";         ///< 左侧头像
    static const wchar_t* avatar_top = L"avatar_top";           ///< 顶部头像
    static const wchar_t* avatar_right = L"avatar_right";       ///< 右侧头像

    static const wchar_t* img_clock = L"img_clock";                     ///< 时钟图片
    static const wchar_t* win_bottom_score_multi = L"win_bottom_score_multi"; ///< 底牌得分倍数窗口
}

/**
 * @brief 皮肤命名空间
 * 
 * 定义游戏中使用的各种皮肤资源
 */
namespace Skins{
    static const wchar_t *kwaiting[] = { L"ani_wait_1", L"ani_wait_2", L"ani_wait_3", L"ani_wait_4" }; ///< 等待动画皮肤
    static const wchar_t *kvectory[] = { L"ani_vectory_1", L"ani_vectory_2", L"ani_vectory_3" };       ///< 胜利动画皮肤
}

/**
 * @brief 动画命名空间
 * 
 * 定义游戏中使用的各种动画资源
 */
namespace Animations{
    static const wchar_t *kclock_alarm = L"clock_alarm";    ///< 时钟报警动画
    static const wchar_t *kbottom_multi = L"bottom_multi";   ///< 底牌倍数动画
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
        static const wchar_t *kshuffle = L"洗牌";              ///< 洗牌音效
        static const wchar_t *kpost_card = L"发牌";            ///< 发牌音效
        static const wchar_t *kset_bottom = L"扣底牌";          ///< 扣底牌音效
        static const wchar_t *kauto_color = L"抽牌";           ///< 抽牌音效
        static const wchar_t *kbottom_multi = L"翻倍";          ///< 翻倍音效
        static const wchar_t *kcount_down = L"倒计时";          ///< 倒计时音效
        static const wchar_t *kput_card_error = L"出牌错误";     ///< 出牌错误音效
        static const wchar_t *kput_card_1_2 = L"出牌1~2";       ///< 出1-2张牌音效
        static const wchar_t *kput_card_3_5 = L"出牌3~5";       ///< 出3-5张牌音效
        static const wchar_t *kput_card_6 = L"出牌>6";          ///< 出6张以上牌音效
        static const wchar_t *klevel_card = L"级牌";            ///< 级牌音效
        static const wchar_t *kround_max = L"本轮最大";          ///< 本轮最大音效
        static const wchar_t *kbutton_focus = L"按钮焦点";       ///< 按钮焦点音效
        static const wchar_t *kbanker = L"庄家";               ///< 庄家音效
        static const wchar_t *krebels_main = L"反主";           ///< 反 chủ音效
        static const wchar_t *kfireworks = L"焰火";            ///< 焰火音效
        static const wchar_t *kget_score = L"得分";            ///< 得分音效
        static const wchar_t *ktractor = L"拖拉机";            ///< 拖拉机音效
        static const wchar_t *kcut_card = L"杀牌";             ///< 杀牌音效
        static const wchar_t *kthrow_card = L"甩牌";           ///< 甩牌音效
        static const wchar_t *kwaiting_turn = L"等待_转身";      ///< 等待转身音效
        static const wchar_t *kwaiting_walk = L"等待_走动";      ///< 等待走动音效
    }
    
    /**
     * @brief 过程语音命名空间
     * 
     * 定义游戏中使用的各种过程语音名称
     */
    namespace Processes{
        static const wchar_t *kshow_main = L"亮主";     ///< 亮主语音
        static const wchar_t *krebels_main = L"反主";    ///< 反主语音
        static const wchar_t *ktractor = L"拖拉机";     ///< 拖拉机语音
        static const wchar_t *kcut_card = L"杀牌";      ///< 杀牌语音
        static const wchar_t *kthrow_card = L"甩牌";     ///< 甩牌语音
    }
}

/**
 * @brief 游戏主题类
 * 
 * 管理游戏的主题资源，包括皮肤、窗口部件、锚点位置、动画和音效等
 */
class SGameTheme : public TObjRefImpl<IObjRef>
{
public:


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
     * @brief 获取卡牌尺寸
     * @return 卡牌尺寸
     */
    SIZE GetCardSize() const;
    
    /**
     * @brief 获取特效音效文件路径
     * @param pszName 音效名称
     * @return 音效文件路径
     */
    SStringW GetEffectSoundFile(LPCWSTR pszName);
    
    /**
     * @brief 获取过程语音文件路径
     * @param bMale 是否为男性语音
     * @param pszName 语音名称
     * @return 语音文件路径
     */
    SStringW GetProcSoundFile(BOOL bMale, LPCWSTR pszName);

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