#ifndef __SCOLOR__H__
#define __SCOLOR__H__

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif // !WIN32_LEAN_AND_MEAN

#include <windows.h>

#define CR_INVALID 0x00FFFFFF

#ifndef GetAValue
#define GetAValue(rgb) (LOBYTE(rgb >> 24))
#endif // GetAValue

#ifndef RGBA
#define RGBA(r, g, b, a) (RGB(r, g, b) | (a << 24))
#endif // RGBA

/**
 * @class SColor
 * @brief 以ARGB格式存储颜色值的类
 */
class SColor {
  public:
    /**
     * @brief 构造函数，使用RGB和Alpha值初始化颜色
     * @param r 红色分量（0-255）
     * @param g 绿色分量（0-255）
     * @param b 蓝色分量（0-255）
     * @param a 透明度分量（0-255，默认为255）
     */
    SColor(BYTE r, BYTE g, BYTE b, BYTE a = 0xFF)
        : b(b)
        , g(g)
        , r(r)
        , a(a)
    {
    }

    /**
     * @brief 构造函数，使用COLORREF和Alpha值初始化颜色
     * @param cr COLORREF颜色值
     * @param alpha 透明度分量（0-255）
     */
    SColor(COLORREF cr, BYTE alpha)
    {
        r = GetRValue(cr);
        g = GetGValue(cr);
        b = GetBValue(cr);
        a = GetAValue(cr);
        updateAlpha(alpha);
    }

    /**
     * @brief 构造函数，使用COLORREF初始化颜色
     * @param cr COLORREF颜色值
     */
    SColor(COLORREF cr)
    {
        r = GetRValue(cr);
        g = GetGValue(cr);
        b = GetBValue(cr);
        a = GetAValue(cr);
    }

    /**
     * @brief 将颜色转换为COLORREF格式
     * @return COLORREF颜色值
     */
    const COLORREF toCOLORREF() const
    {
        return RGB(r, g, b) | (a << 24);
    }

    /**
     * @brief 将颜色转换为DWORD格式（ARGB）
     * @return DWORD格式的颜色值
     */
    const DWORD toARGB() const
    {
        DWORD crRet;
        memcpy(&crRet, this, 4);
        return crRet;
    }

    /**
     * @brief 设置颜色的RGB值
     * @param cr COLORREF颜色值
     */
    void setRGB(COLORREF cr)
    {
        r = GetRValue(cr);
        g = GetGValue(cr);
        b = GetBValue(cr);
        a = GetAValue(cr);
    }

    /**
     * @brief 设置颜色的RGB值和Alpha值
     * @param cr COLORREF颜色值
     * @param alpha 透明度分量（0-255）
     */
    void setRGB(COLORREF cr, BYTE alpha)
    {
        r = GetRValue(cr);
        g = GetGValue(cr);
        b = GetBValue(cr);
        a = GetAValue(cr);
        updateAlpha(alpha);
    }

    /**
     * @brief 更新颜色的Alpha值
     * @param alpha 透明度分量（0-255）
     */
    void updateAlpha(BYTE alpha)
    {
        if (alpha != 0xFF)
        {
            if (a == 0xFF)
                a = alpha;
            else
                a = (a * alpha) / 0xFF;
        }
    }

    DWORD b : 8; ///< 蓝色分量（0-255）
    DWORD g : 8; ///< 绿色分量（0-255）
    DWORD r : 8; ///< 红色分量（0-255）
    DWORD a : 8; ///< 透明度分量（0-255）
};

#endif // __SCOLOR__H__