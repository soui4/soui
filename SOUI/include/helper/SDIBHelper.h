#ifndef __SDIBHELPER__H__
#define __SDIBHELPER__H__

#include "interface/SRender-i.h"

SNSBEGIN

/**
 * @class SDIBHelper
 * @brief 提供对位图进行颜色处理的辅助类
 * @details 简洁版本，简版去掉了对8、16、24位的支持
 */
class SOUI_EXP SDIBHelper {
  public:
    /**
     * @brief 对位图进行着色处理
     * @param pBmp 位图对象指针
     * @param crRef 目标颜色
     * @return 成功返回TRUE，失败返回FALSE
     */
    static bool Colorize(IBitmapS *pBmp, COLORREF crRef);

    /**
     * @brief 对单个颜色进行着色处理
     * @param crTarget 目标颜色引用
     * @param crRef 参考颜色
     * @return 成功返回TRUE，失败返回FALSE
     */
    static bool Colorize(COLORREF &crTarget, COLORREF crRef);

    /**
     * @brief 将位图转换为灰度图像
     * @param pBmp 位图对象指针
     * @return 成功返回TRUE，失败返回FALSE
     */
    static bool GrayImage(IBitmapS *pBmp);

    /**
     * @brief 计算图片的平均颜色
     * @param pBmp 图片源位图对象指针
     * @param nPercent 有效值百分比，90代表最高和最低5%的值会丢掉，不参与平均
     * @param nBlockSize 分块大小，每次计算一个块的颜色平均值
     * @return 计算得到的平均颜色
     */
    static COLORREF CalcAvarageColor(IBitmapS *pBmp, int nPercent = 90, int nBlockSize = 5);
};

SNSEND
#endif // __SDIBHELPER__H__