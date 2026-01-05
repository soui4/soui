#ifndef UTILS_H
#define UTILS_H
#include <layout/SAnchorLayout.h>
SNSBEGIN

class Util
{
public:

    /**
     * @brief 偏移精灵窗口
     * @param pSprite 精灵对象
     * @param dx X轴偏移量
     * @param dy Y轴偏移量
     * @param nSpeed 动画速度
     * @return 动画对象
     */
    static SAutoRefPtr<IValueAnimator> ScaleSprite(IWindow *pSprite, float from, float to, int nSpeed);

    static SAutoRefPtr<IValueAnimator> ChangeSpriteHeight(IWindow *pSprite, float from, float to, int nSpeed);

    /**
     * @brief 偏移精灵窗口
     * @param pSprite 精灵对象
     * @param dx X轴偏移量
     * @param dy Y轴偏移量
     * @param nSpeed 动画速度
     * @return 动画对象
     */
    static SAutoRefPtr<IValueAnimator> OffsetSprite(IWindow *pSprite, float dx, float dy, int nSpeed);

    /**
     * @brief 移动精灵窗口
     * @param pSprite 精灵对象
     * @param fromPos 起始位置
     * @param toPos 目标位置
     * @param nSpeed 动画速度
     * @return 动画对象
     */
    static SAutoRefPtr<IValueAnimator> MoveSprite(IWindow *pSprite, AnchorPos fromPos, AnchorPos toPos, int nSpeed);

    /**
     * @brief 移动精灵到指定位置
     * @param pSprite 精灵对象
     * @param toPos 目标位置
     * @param nSpeed 动画速度
     * @return 动画对象
     */
    static SAutoRefPtr<IValueAnimator> MoveSpriteTo(IWindow *pSprite, AnchorPos toPos, int nSpeed);

    /**
     * @brief 移动并隐藏精灵窗口
     * @param pSprite 精灵对象
     * @param toPos 目标位置
     * @param nSpeed 动画速度
     * @return 动画对象
     */
    static SAutoRefPtr<IValueAnimator> MoveAndHideSprite(IWindow *pSprite, AnchorPos toPos, int nSpeed);

    /**
     * @brief 启动动画
     * @param pAnimator 动画对象
     * @param pSprite 卡牌对象
     */
    static void StartAnimator(IValueAnimator *pAnimator,IWindow *pSprite);
    
    /**
     * @brief 结束动画
     * @param pSprite 卡牌对象
     */
    static void EndAnimator(IWindow *pSprite);
};

SNSEND


#endif//UTILS_H