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
        kChessBoard = 0,      ///< 棋盘位置
        kChessPiece,          ///< 棋子位置

        kAvatarBottom,        ///< 自己头像位置
        kAvatarTop,           ///< 对手头像位置

        kClockBottom,         ///< 自己时钟位置
        kClockTop,            ///< 对手时钟位置

        kCount                ///< 锚点总数
    };
    
    /**
     * @brief 锚点名称数组
     * 
     * 与锚点索引枚举对应的实际名称字符串数组
     */
static const wchar_t *kElementNames[] = {
    L"chess_board",    // 棋盘
    L"chess_piece",    // 棋子

    L"avatar_bottom",  // 自己头像
    L"avatar_top",     // 对手头像

    L"clock_bottom",   // 自己时钟
    L"clock_top",      // 对手时钟
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
        btn_red = 100,        ///< 红方按钮
        btn_black,            ///< 黑方按钮
    };
}

/**
 * @brief 游戏主题类
 * 
 * 负责加载和管理游戏的主题资源，包括皮肤、动画、声音等
 */
class SGameTheme : public SObject
{
    SOUI_CLASS_NAME(SGameTheme, L"theme")
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
     * @param strPath 主题资源路径
     * @return 是否加载成功
     */
    BOOL Load(const SStringT & strPath);

    /**
     * @brief 获取皮肤对象
     * @param name 皮肤名称
     * @return 皮肤对象指针
     */
    ISkinObj * GetSkin(const SStringW & name);

    /**
     * @brief 获取动画对象
     * @param name 动画名称
     * @return 动画对象指针
     */
    IAnimation * GetAnimation(const SStringW & name);

    /**
     * @brief 获取布局对象
     * @param name 布局名称
     * @return 布局对象指针
     */
    ILayoutParam * GetLayout(const SStringW & name);

    /**
     * @brief 获取精灵对象
     * @param name 精灵名称
     * @return 精灵对象指针
     */
    ISprite * GetSprite(const SStringW & name);

    /**
     * @brief 获取声音对象
     * @param name 声音名称
     * @return 声音对象指针
     */
    IAudioPlayer * GetSound(const SStringW & name);

protected:
    /**
     * @brief 加载皮肤资源
     * @param strPath 皮肤资源路径
     * @return 是否加载成功
     */
    BOOL LoadSkins(const SStringT & strPath);

    /**
     * @brief 加载窗口部件资源
     * @param strPath 窗口部件资源路径
     * @return 是否加载成功
     */
    BOOL LoadWindowPart(const SStringT & strPath);

    /**
     * @brief 加载锚点布局资源
     * @param strPath 锚点布局资源路径
     * @return 是否加载成功
     */
    BOOL LoadAnchorLayout(const SStringT & strPath);

    /**
     * @brief 加载动画资源
     * @param strPath 动画资源路径
     * @return 是否加载成功
     */
    BOOL LoadAnims(const SStringT & strPath);

    /**
     * @brief 加载精灵资源
     * @param strPath 精灵资源路径
     * @return 是否加载成功
     */
    BOOL LoadSprites(const SStringT & strPath);

    /**
     * @brief 加载声音资源
     * @param strPath 声音资源路径
     * @return 是否加载成功
     */
    BOOL LoadSounds(const SStringT & strPath);

protected:
    typedef std::map<SStringW, SAutoRefPtr<ISkinObj> > SKIN_MAP;                 ///< 皮肤映射表
    typedef std::map<SStringW, SAutoRefPtr<ILayoutParam> > LAYOUT_MAP;           ///< 布局映射表
    typedef std::map<SStringW, SAutoRefPtr<IAnimation> > ANIMATION_MAP;          ///< 动画映射表
    typedef std::map<SStringW, SAutoRefPtr<ISprite> > SPRITE_MAP;                ///< 精灵映射表
    typedef std::map<SStringW, SAutoRefPtr<IAudioPlayer> > SOUND_MAP;            ///< 声音映射表

    SKIN_MAP        m_mapSkins;      ///< 皮肤映射表
    LAYOUT_MAP      m_mapLayouts;    ///< 布局映射表
    ANIMATION_MAP   m_mapAnims;      ///< 动画映射表
    SPRITE_MAP      m_mapSprites;    ///< 精灵映射表
    SOUND_MAP       m_mapSounds;     ///< 声音映射表
};

SNSEND

#endif // __SGAMETHEME_H__