#ifndef __SFONTINFO__H__
#define __SFONTINFO__H__

SNSBEGIN

/**
 * @union FONTSTYLE
 * @brief Font style union
 *
 * Describes the style of a font using a union of a 64-bit integer and a bitfield structure.
 */
union FONTSTYLE {
    uint64_t syle; // DWORD version of the style

    /**
     * @struct attr
     * @brief Bitfield structure for font style attributes
     */
    struct
    {
        uint64_t byCharset : 8;    /**< Character set */
        uint64_t byWeight : 8;     /**< Weight/4 */
        uint64_t fItalic : 1;      /**< Italic flag */
        uint64_t fUnderline : 1;   /**< Underline flag */
        uint64_t fBold : 1;        /**< Bold flag */
        uint64_t fStrike : 1;      /**< Strike-through flag */
        uint64_t fEscapement : 12; /**< Angle, in 0.1 degree units */
        uint64_t reserved : 13;    /**< Reserved */
        uint64_t szIsAdding : 1;   /**< cSize is adding */
        uint64_t szUnit : 2;       /**< cSize unit (0-3) */
        uint64_t nSize : 16;       /**< Font size, as a signed short */
    } attr;

    /**
     * @brief Constructor for FONTSTYLE
     * @param _style Initial style value (default is 0)
     */
    FONTSTYLE(uint64_t _style = 0)
        : syle(_style)
    {
    }
};

/**
 * @struct FontInfo
 * @brief Font information structure
 *
 * Contains details about a font, including style, face name, property extensions, and scale.
 */
struct FontInfo
{
    FONTSTYLE style;      /**< Font style */
    SStringW strFaceName; /**< Font face name */
    SStringW strPropEx;   /**< Font property extensions */
    int scale;            /**< Font scale */
};

SNSEND

#endif // __SFONTINFO__H__