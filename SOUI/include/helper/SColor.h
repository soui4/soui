#ifndef __SCOLOR__H__
#define __SCOLOR__H__

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif /**< !WIN32_LEAN_AND_MEAN */

#include <windows.h>

#define CR_INVALID 0x00FFFFFF

#ifndef GetAValue
#define GetAValue(rgb) (LOBYTE(rgb >> 24))
#endif /**< GetAValue */

#ifndef RGBA
#define RGBA(r, g, b, a) ((r) | (g) << 8 | (b) << 16 | ((uint32_t)a) << 24)
#endif /**< RGBA */

/**
 * @class SColor
 * @brief Class storing color values in ARGB format
 */
class SColor {
  public:
    /**
     * @brief Constructor; initialize the color using RGB and Alpha values
     * @param r red component (0-255)
     * @param g green component (0-255)
     * @param b blue component (0-255)
     * @param a alpha component (0-255, default 255)
     */
    SColor(BYTE r, BYTE g, BYTE b, BYTE a = 0xFF)
        : b(b)
        , g(g)
        , r(r)
        , a(a)
    {
    }

    /**
     * @brief Constructor; initialize the color using COLORREF and Alpha values
     * @param cr COLORREF color value
     * @param alpha alpha component (0-255)
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
     * @brief Constructor; initialize the color using COLORREF
     * @param cr COLORREF color value
     */
    SColor(COLORREF cr)
    {
        r = GetRValue(cr);
        g = GetGValue(cr);
        b = GetBValue(cr);
        a = GetAValue(cr);
    }

    /**
     * @brief Convert the color to COLORREF format
     * @return COLORREF color value
     */
    const COLORREF toCOLORREF() const
    {
        return RGBA(r, g, b, a);
    }

    /**
     * @brief Convert the color to DWORD format (ARGB)
     * @return color value in DWORD format
     */
    const DWORD toARGB() const
    {
        DWORD crRet;
        memcpy(&crRet, this, 4);
        return crRet;
    }

    /**
     * @brief Set the RGB value of the color
     * @param cr COLORREF color value
     */
    void setRGB(COLORREF cr)
    {
        r = GetRValue(cr);
        g = GetGValue(cr);
        b = GetBValue(cr);
        a = GetAValue(cr);
    }

    /**
     * @brief Set the RGB value and Alpha value of the color
     * @param cr COLORREF color value
     * @param alpha alpha component (0-255)
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
     * @brief Update the Alpha value of the color
     * @param alpha alpha component (0-255)
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

    DWORD b : 8; /**<  Blue component (0-255) */
    DWORD g : 8; /**<  Green component (0-255) */
    DWORD r : 8; /**<  Red component (0-255) */
    DWORD a : 8; /**<  Alpha component (0-255) */
};

#endif /**< __SCOLOR__H__ */