#ifndef __SSPRITE_H__
#define __SSPRITE_H__
#include <core/SSkin.h>
#include <gif/SGifPlayer.h>
#include <control/SCmnCtrl.h>
#include <layout/SAnchorLayout.h>
#include <upgradeProtocol.h>
SNSBEGIN

/**
 * @brief 发牌速度定义
 */
#define SPEED_POSTCARD			100		//发牌速度

/**
 * @brief 卡牌皮肤类
 * 
 * 继承自SSkinImgList，用于显示卡牌图像的皮肤类
 */
class SCard : public SSkinImgList
{
	DEF_SOBJECT(SSkinImgList, L"card")
public:
	/**
	 * @brief 构造函数
	 */
	SCard(void);
	
	/**
	 * @brief 析构函数
	 */
	~SCard(void);

        /**
     * @brief 获取皮肤尺寸
     * @return 皮肤尺寸
     */
    STDMETHOD_(SIZE, GetSkinSize)(THIS) SCONST OVERRIDE;

    /**
     * @brief 处理皮肤着色
     * @param cr 着色颜色
     */
    STDMETHOD_(void, OnColorize)(THIS_ COLORREF cr) OVERRIDE{}

 protected:
    /**
     * @brief 根据索引绘制皮肤
     * @param pRT 渲染目标
     * @param rcDraw 绘制区域
     * @param idx 索引
     * @param byAlpha 透明度
     */
    void _DrawByIndex(IRenderTarget *pRT, LPCRECT rcDraw, int idx, BYTE byAlpha) const override;
    
    /**
     * @brief Scales the skin.
     * @param skinObj Pointer to the skin object.
     * @param nScale Scale factor.
     */
    void _Scale(ISkinObj *skinObj, int nScale) override;



    HRESULT OnAttrSrc(const SStringW &value, BOOL bLoading);

    SOUI_ATTRS_BEGIN()
        ATTR_CUSTOM(L"src", OnAttrSrc)
    SOUI_ATTRS_END()

    SIZE m_szOrig;
};


#ifndef EVT_SPRITE_BEGIN
#define EVT_SPRITE_BEGIN (EVT_EXTERNAL_BEGIN + 700)
#endif//EVT_SPRITE_BEGIN

/**
 * @brief 精灵卡牌事件枚举
 */
enum{
    EVT_SPRITE_CARD_DRAG_START = EVT_SPRITE_BEGIN,  ///< 开始拖拽事件
    EVT_SPRITE_CARD_DRAG,                           ///< 拖拽事件
    EVT_SPRITE_CARD_DRAG_END,                       ///< 结束拖拽事件
};

/**
 * @brief 精灵卡牌开始拖拽事件
 */
DEF_EVT_EXT(EventSpriteCardDragStart, EVT_SPRITE_CARD_DRAG_START, {
    CRect rcDraging;  ///< 拖拽区域
})

/**
 * @brief 精灵卡牌拖拽事件
 */
DEF_EVT_EXT(EventSpriteCardDrag, EVT_SPRITE_CARD_DRAG, {
    CRect rcDraging;  ///< 拖拽区域
})

/**
 * @brief 精灵卡牌结束拖拽事件
 */
DEF_EVT_EXT(EventSpriteCardDragEnd, EVT_SPRITE_CARD_DRAG_END, {
    CRect rcDraging;  ///< 拖拽区域
})


/**
 * @brief 精灵卡牌类
 * 
 * 继承自SImageWnd，用于显示和操作卡牌的窗口类
 */
class SSpriteCard: public SImageWnd
{
    DEF_SOBJECT(SImageWnd, L"spriteCard")

public:
    /**
     * @brief 常量定义
     */
    enum{
        kSelectOffset = 10,        ///< 选择时的偏移量
        kMinDragDistance = 2,      ///< 最小拖拽距离
    };
    
public:
    /**
     * @brief 构造函数
     */
    SSpriteCard();
    
    /**
     * @brief 析构函数
     */
    virtual ~SSpriteCard();

    /**
     * @brief 设置卡牌值
     * @param nCard 卡牌值
     */
    void SetCard(int nCard);
    
    /**
     * @brief 获取卡牌值
     * @return 卡牌值
     */
    int GetCard() const;
    
    /**
     * @brief 反转选择状态
     */
    void InvertSelect();
    
    /**
     * @brief 设置选择状态
     * @param bSelected 是否选中
     */
    void SetSelected(BOOL bSelected);
    
    /**
     * @brief 判断是否被选中
     * @return 是否被选中
     */
    BOOL IsSelected() const;
    
    /**
     * @brief 设置遮罩皮肤
     * @param pSkin 遮罩皮肤对象
     */
    void SetMaskSkin(ISkinObj *pSkin);
    
    /**
     * @brief 设置是否可选择
     * @param bSelectable 是否可选择
     */
    void SetSelectable(BOOL bSelectable);
    
    /**
     * @brief 清除选择状态
     */
    void ClearSelect();
public:
    /**
     * @brief 属性映射表
     */
    SOUI_ATTRS_BEGIN()
        ATTR_SKIN(L"selectMask", m_skinSel, TRUE)  ///< 选择遮罩属性
    SOUI_ATTRS_END()
    
protected:
    /**
     * @brief 绘制处理
     * @param pRT 渲染目标
     */
    void OnPaint(IRenderTarget *pRT);
    
    /**
     * @brief 鼠标左键按下处理
     * @param nFlags 按键标志
     * @param pt 鼠标位置
     */
    void OnLButtonDown(UINT nFlags, CPoint pt);
    
    /**
     * @brief 鼠标左键抬起处理
     * @param nFlags 按键标志
     * @param pt 鼠标位置
     */
    void OnLButtonUp(UINT nFlags, CPoint pt);
    
    /**
     * @brief 鼠标移动处理
     * @param nFlags 按键标志
     * @param pt 鼠标位置
     */
    void OnMouseMove(UINT nFlags, CPoint pt);
    
    /**
     * @brief 消息映射表
     */
    SOUI_MSG_MAP_BEGIN()
        MSG_WM_PAINT_EX(OnPaint)
        MSG_WM_LBUTTONDOWN(OnLButtonDown)
        MSG_WM_LBUTTONUP(OnLButtonUp)
        MSG_WM_MOUSEMOVE(OnMouseMove)
    SOUI_MSG_MAP_END()
    
protected:
    /**
     * @brief 最终释放处理
     */
    void WINAPI OnFinalRelease() override;

private:
    SAutoRefPtr<ISkinObj> m_skinSel;    ///< 选择mask
    BOOL    m_bSelected;                ///< 是否选中
    BOOL    m_bDraging;                 ///< 是否正在拖拽
    CPoint  m_ptClick;                  ///< 点击位置
    BOOL    m_bSelectable;              ///< 是否可选择
};

/**
 * @brief 工具类
 * 
 * 提供卡牌动画相关的工具函数
 */
class Util{
public:
    /**
     * @brief 偏移精灵卡牌
     * @param pCard 卡牌对象
     * @param dx X轴偏移量
     * @param dy Y轴偏移量
     * @param nSpeed 动画速度
     * @return 动画对象
     */
    static SAutoRefPtr<IValueAnimator> OffsetSprite(IWindow *pCard, float dx, float dy, int nSpeed);

    /**
     * @brief 移动精灵卡牌
     * @param pCard 卡牌对象
     * @param fromPos 起始位置
     * @param toPos 目标位置
     * @param nSpeed 动画速度
     * @return 动画对象
     */
    static SAutoRefPtr<IValueAnimator> MoveSprite(IWindow *pCard, AnchorPos fromPos, AnchorPos toPos, int nSpeed);

    /**
     * @brief 移动精灵卡牌到指定位置
     * @param pCard 卡牌对象
     * @param toPos 目标位置
     * @param nSpeed 动画速度
     * @return 动画对象
     */
    static SAutoRefPtr<IValueAnimator> MoveSpriteTo(IWindow *pCard, AnchorPos toPos, int nSpeed);

    /**
     * @brief 移动并隐藏精灵卡牌
     * @param pCard 卡牌对象
     * @param toPos 目标位置
     * @param nSpeed 动画速度
     * @return 动画对象
     */
    static SAutoRefPtr<IValueAnimator> MoveAndHideSprite(IWindow *pCard, AnchorPos toPos, int nSpeed);

    /**
     * @brief 启动动画
     * @param pAnimator 动画对象
     * @param pCard 卡牌对象
     */
    static void StartAnimator(IValueAnimator *pAnimator,IWindow *pCard);
    
    /**
     * @brief 结束动画
     * @param pCard 卡牌对象
     */
    static void EndAnimator(IWindow *pCard);
};

SNSEND

#endif//__SSPRITE_H__