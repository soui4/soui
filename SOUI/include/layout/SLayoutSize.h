#ifndef __SLAYOUTSIZE__H__
#define __SLAYOUTSIZE__H__

#include <interface/slayout-i.h>

SNSBEGIN

/**
 * @class SLayoutSize
 * @brief 布局大小类
 */
class SOUI_EXP SLayoutSize : public LAYOUTSIZE {
  public:
    /**
     * @brief 默认构造函数
     * @param fSize 大小值（默认为0.0f）
     */
    SLayoutSize(float fSize = 0.0f);

    /**
     * @brief 构造函数
     * @param fSize 大小值
     * @param unit 大小单位
     */
    SLayoutSize(float fSize, Unit unit);

    SLayoutSize(const SLayoutSize &src)
    {
        *this = src;
    }
    SLayoutSize(const LAYOUTSIZE &src)
    {
        *this = src;
    }

    /**
     * @brief 设置为包裹内容大小
     */
    void setWrapContent();

    /**
     * @brief 检查是否为包裹内容大小
     * @return 如果为包裹内容大小返回TRUE，否则返回FALSE
     */
    bool isWrapContent() const;

    /**
     * @brief 设置为匹配父容器大小
     */
    void setMatchParent();

    /**
     * @brief 检查是否为匹配父容器大小
     * @return 如果为匹配父容器大小返回TRUE，否则返回FALSE
     */
    bool isMatchParent() const;

    /**
     * @brief 设置指定大小
     * @param fSize 大小值
     * @param unit 大小单位
     */
    void setSize(float fSize, Unit unit);

    /**
     * @brief 检查是否为指定大小
     * @return 如果为指定大小返回TRUE，否则返回FALSE
     */
    bool isSpecifiedSize() const;

    /**
     * @brief 设置为无效大小
     */
    void setInvalid();

    /**
     * @brief 检查是否为有效大小
     * @return 如果为有效大小返回TRUE，否则返回FALSE
     */
    bool isValid() const;

    /**
     * @brief 将大小转换为像素值
     * @param scale 缩放比例
     * @return 像素值
     */
    int toPixelSize(int scale) const;

    /**
     * @brief 将大小转换为字符串表示
     * @return 字符串表示
     */
    SStringW toString() const;

    /**
     * @brief 检查大小是否为零
     * @return 如果大小为零返回TRUE，否则返回FALSE
     */
    bool isZero() const;

    /**
     * @brief 从字符串解析大小
     * @param strSize 大小字符串
     */
    void parseString(const SStringW &strSize);

    /**
     * @brief 赋值运算符重载
     * @param src 源大小对象
     * @return 当前大小对象的引用
     */
    SLayoutSize &operator=(const SLayoutSize &src);
    SLayoutSize &operator=(const LAYOUTSIZE &src);

    /**
     * @brief 比较大小值是否相等
     * @param value 大小值
     * @return 如果大小值相等返回TRUE，否则返回FALSE
     */
    bool valueEqual(float value);

    /**
     * @brief 从字符串创建大小对象
     * @param strSize 大小字符串
     * @return 大小对象
     */
    static SLayoutSize fromString(const SStringW &strSize);

    /**
     * @brief 比较两个浮点数是否相等
     * @param a 第一个浮点数
     * @param b 第二个浮点数
     * @return 如果相等返回TRUE，否则返回FALSE
     */
    static bool fequal(float a, float b);

    /**
     * @brief 从字符串解析单位
     * @param strUnit 单位字符串
     * @return 单位枚举值
     */
    static Unit unitFromString(const SStringW &strUnit);

    static Unit defUnit; // 默认单位
};

SNSEND

#endif // __SLAYOUTSIZE__H__