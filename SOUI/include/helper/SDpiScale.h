#ifndef __SDPISCALE__H__
#define __SDPISCALE__H__

SNSBEGIN

/**
 * @class SDpiScale
 * @brief 提供DPI缩放比例处理的类
 */
class SOUI_EXP SDpiScale {
  public:
    /**
     * @brief 获取内置的缩放比例数量
     * @return 内置缩放比例的数量
     */
    static int GetBuiltinScaleCount();

    /**
     * @brief 获取内置的缩放比例数组
     * @return 内置缩放比例数组指针
     */
    static const int *GetBuiltinScales();

    /**
     * @brief 标准化放大比例
     * @param nScale 输入的缩放比例
     * @return 标准化后的缩放比例
     *
     * @details 选择比指定比例小一号的标准缩放比例
     */
    static int NormalizeScale(int nScale);
};

SNSEND

#endif // __SDPISCALE__H__