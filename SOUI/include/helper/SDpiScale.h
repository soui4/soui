#ifndef __SDPISCALE__H__
#define __SDPISCALE__H__

SNSBEGIN

/**
 * @class SDpiScale
 * @brief Class providing DPI scale factor handling
 */
class SOUI_EXP SDpiScale {
  public:
    /**
     * @brief Get the number of built-in scale factors
     * @return number of built-in scale factors
     */
    static int GetBuiltinScaleCount();

    /**
     * @brief Get the built-in scale factor array
     * @return pointer to the built-in scale factor array
     */
    static const int *GetBuiltinScales();

    /**
     * @brief Normalize the scale factor
     * @param nScale input scale factor
     * @return normalized scale factor
     *
     * @details Select the standard scale factor one step below the specified scale
     */
    static int NormalizeScale(int nScale);
};

SNSEND

#endif /**< __SDPISCALE__H__ */