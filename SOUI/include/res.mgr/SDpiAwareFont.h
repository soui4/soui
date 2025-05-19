#ifndef __SDPIAWAREFONT__H__
#define __SDPIAWAREFONT__H__

SNSBEGIN

/**
 * @class SDpiAwareFont
 * @brief Class for managing DPI-aware fonts.
 *        This class provides functionality to handle font scaling based on DPI settings.
 */
class SOUI_EXP SDpiAwareFont {
  public:
    /**
     * @brief Constructor.
     */
    SDpiAwareFont(void);

    /**
     * @brief Destructor.
     */
    ~SDpiAwareFont(void);

    /**
     * @brief Retrieves the pointer to the current font object.
     * @return Pointer to the IFontS object.
     */
    IFontS *GetFontPtr();

    /**
     * @brief Sets the font description and scale.
     * @param strDesc Font description string.
     * @param nScale DPI scaling factor.
     */
    void SetFontDesc(const SStringW &strDesc, int nScale);

    /**
     * @brief Updates the font based on the provided scale.
     * @param nScale DPI scaling factor.
     */
    void UpdateFont(int nScale);

  protected:
    /**
     * @brief Retrieves the font object based on the description and scale.
     * @param strDesc Font description string.
     * @param nScale DPI scaling factor.
     * @return Pointer to the IFontS object.
     */
    IFontS *OnGetFont(const SStringW &strDesc, int nScale);

    SStringW m_strDesc;           // Font description string
    SAutoRefPtr<IFontS> m_object; // Pointer to the font object
};

SNSEND

#endif // __SDPIAWAREFONT__H__