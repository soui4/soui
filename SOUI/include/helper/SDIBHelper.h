#ifndef __SDIBHELPER__H__
#define __SDIBHELPER__H__

#include "interface/SRender-i.h"

SNSBEGIN

/**
 * @class SDIBHelper
 * @brief Helper class providing color processing for bitmaps
 * @details Simplified version; the lite version drops support for 8, 16, and 24 bit
 */
class SOUI_EXP SDIBHelper {
  public:
    /**
     * @brief Apply color tinting to the bitmap
     * @param pBmp bitmap object pointer
     * @param crRef target color
     * @return Returns TRUE on success, FALSE on failure
     */
    static bool Colorize(IBitmapS *pBmp, COLORREF crRef);

    /**
     * @brief Apply color tinting to a single color
     * @param crTarget target color reference
     * @param crRef reference color
     * @return Returns TRUE on success, FALSE on failure
     */
    static bool Colorize(COLORREF &crTarget, COLORREF crRef);

    /**
     * @brief Convert the bitmap to a grayscale image
     * @param pBmp bitmap object pointer
     * @return Returns TRUE on success, FALSE on failure
     */
    static bool GrayImage(IBitmapS *pBmp);

    /**
     * @brief Convert the bitmap to a disabled-style image
     * @param pBmp bitmap object pointer
     * @return Returns TRUE on success, FALSE on failure
     */
    static bool DisabledStyleImage(IBitmapS *pBmp);

    /**
     * @brief Calculate the average color of the image
     * @param pBmp source image bitmap object pointer
     * @param nPercent valid value percentage; 90 means the top and bottom 5% of values are discarded and excluded from averaging
     * @param nBlockSize block size; the color average of one block is computed each time
     * @return computed average color
     */
    static COLORREF CalcAvarageColor(IBitmapS *pBmp, int nPercent = 90, int nBlockSize = 5);
};

SNSEND
#endif /**< __SDIBHELPER__H__ */