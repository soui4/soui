#ifndef __SSKINPOOL__H__
#define __SSKINPOOL__H__

#include <core/SSingletonMap.h>
#include <interface/SSkinPool-i.h>
#include <helper/obj-ref-impl.hpp>

SNSBEGIN

/**
 * @file SSkinPool.h
 * @brief Skin Object Pool Management
 * @version v1.0
 * @author SOUI团队
 * @date 2014-05-28
 *
 * @details Manages a pool of skin objects, providing functionality to load, retrieve, and manage skins.
 */

// System built-in skin names
enum SYS_SKIN
{
    SKIN_SYS_CHECKBOX = 0,    // L"_skin.sys.checkbox"
    SKIN_SYS_RADIO,           // L"_skin.sys.radio"
    SKIN_SYS_FOCUSCHECKBOX,   // L"_skin.sys.focuscheckbox"
    SKIN_SYS_FOCUSRADIO,      // L"_skin.sys.focusradio"
    SKIN_SYS_BTN_NORMAL,      // L"_skin.sys.btn.normal"
    SKIN_SYS_SCROLLBAR,       // L"_skin.sys.scrollbar"
    SKIN_SYS_BORDER,          // L"_skin.sys.border"
    SKIN_SYS_DROPBTN,         // L"_skin.sys.dropbtn"
    SKIN_SYS_TREE_TOGGLE,     // L"_skin.sys.tree.toggle"
    SKIN_SYS_TREE_CHECKBOX,   // L"_skin.sys.tree.checkbox"
    SKIN_SYS_TREE_LINES,      // L"_skin.sys.tree.lines"
    SKIN_SYS_TAB_PAGE,        // L"_skin.sys.tab.page"
    SKIN_SYS_HEADER,          // L"_skin.sys.header"
    SKIN_SYS_SPLIT_VERT,      // L"_skin.sys.split.vert"
    SKIN_SYS_SPLIT_HORZ,      // L"_skin.sys.split.horz"
    SKIN_SYS_PROG_BKGND,      // L"_skin.sys.prog.bkgnd"
    SKIN_SYS_PROG_BAR,        // L"_skin.sys.prog.bar"
    SKIN_SYS_VERT_PROG_BKGND, // L"_skin.sys.vert.prog.bkgnd"
    SKIN_SYS_VERT_PROG_BAR,   // L"_skin.sys.vert.prog.bar"
    SKIN_SYS_SLIDER_THUMB,    // L"_skin.sys.slider.thumb"

    SKIN_SYS_BTN_CLOSE,    // L"_skin.sys.btn.close"
    SKIN_SYS_BTN_MINIMIZE, // L"_skin.sys.btn.minimize"
    SKIN_SYS_BTN_MAXMIZE,  // L"_skin.sys.btn.maxmize"
    SKIN_SYS_BTN_RESTORE,  // L"_skin.sys.btn.restore"

    SKIN_SYS_MENU_CHECK,  // L"_skin.sys.menu.check"
    SKIN_SYS_MENU_SEP,    // L"_skin.sys.menu.sep"
    SKIN_SYS_MENU_ARROW,  // L"_skin.sys.menu.arrow"
    SKIN_SYS_MENU_BORDER, // L"_skin.sys.menu.border"
    SKIN_SYS_MENU_SKIN,   // L"_skin.sys.menu.skin"
    SKIN_SYS_ICONS,       // L"_skin.sys.icons"
    SKIN_SYS_WND_BKGND,   // L"_skin.sys.wnd.bkgnd"

    SKIN_SYS_BTN_PREV,
    SKIN_SYS_BTN_NEXT,
    SKIN_SYS_SPIN_DOWN,
    SKIN_SYS_SPIN_UP,

    SKIN_SYS_SWITCH,
    SKIN_SYS_SWITCH_BG,

    SKIN_SYS_COUNT,
};

typedef ISkinObj *SSkinPtr;

/**
 * @struct SkinKey
 * @brief Key for identifying a skin object in the pool.
 */
class SkinKey {
  public:
    SStringW strName; // Name of the skin
    int scale;        // Scale factor for the skin
};

/**
 * @class CElementTraits<SkinKey>
 * @brief Template specialization for SkinKey to provide hash and comparison functions.
 */
template <>
class CElementTraits<SkinKey> : public CElementTraitsBase<SkinKey> {
  public:
    /**
     * @brief Computes the hash value for a SkinKey.
     * @param skinKey SkinKey object.
     * @return Hash value.
     */
    static ULONG Hash(INARGTYPE skinKey)
    {
        ULONG nHash = CElementTraits<SStringW>::Hash(skinKey.strName);
        nHash <<= 5;
        nHash += skinKey.scale;
        return nHash;
    }

    /**
     * @brief Compares two SkinKey objects for equality.
     * @param element1 First SkinKey object.
     * @param element2 Second SkinKey object.
     * @return TRUE if the objects are equal, FALSE otherwise.
     */
    static bool CompareElements(INARGTYPE element1, INARGTYPE element2)
    {
        return element1.strName == element2.strName && element1.scale == element2.scale;
    }

    /**
     * @brief Compares two SkinKey objects in ordered manner.
     * @param element1 First SkinKey object.
     * @param element2 Second SkinKey object.
     * @return Comparison result (-1 if element1 < element2, 0 if equal, 1 if element1 > element2).
     */
    static int CompareElementsOrdered(INARGTYPE element1, INARGTYPE element2)
    {
        int nRet = element1.strName.CompareNoCase(element2.strName);
        if (nRet == 0)
            nRet = element1.scale - element2.scale;
        return nRet;
    }
};

/**
 * @class SSkinPool
 * @brief Manages the mapping of skin names to ISkinObj objects.
 *
 * @details This class provides functionality to load, retrieve, and manage skin objects. It supports automatic scaling
 *          and maintains a pool of skin objects for efficient reuse.
 */
class SOUI_EXP SSkinPool
    : public TObjRefImpl<ISkinPool>
    , protected SCmnMap<SSkinPtr, SkinKey> {
  public:
    /**
     * @brief Constructor.
     * @param bAutoScale TRUE if automatic scaling is enabled, FALSE otherwise.
     */
    SSkinPool(BOOL bAutoScale = TRUE);

    /**
     * @brief Destructor.
     */
    virtual ~SSkinPool();

  public:
    /**
     * @brief Retrieves a skin object by name and scale.
     * @param strSkinName Name of the skin.
     * @param nScale Scale factor for the skin.
     * @return Pointer to the ISkinObj object, or nullptr if not found.
     */
    STDMETHOD_(ISkinObj *, GetSkin)(THIS_ LPCWSTR strSkinName, int nScale) OVERRIDE;

    /**
     * @brief Loads skins from an XML node.
     * @param xmlNode XML node containing the skin definitions.
     * @return Number of skins loaded.
     */
    STDMETHOD_(int, LoadSkins)(THIS_ IXmlNode *xmlNode) OVERRIDE;

    /**
     * @brief Adds a skin object to the pool.
     * @param skin Pointer to the skin object to add.
     * @return TRUE if the skin is successfully added, FALSE otherwise.
     */
    STDMETHOD_(BOOL, AddSkin)(THIS_ ISkinObj *skin) OVERRIDE;

    /**
     * @brief Removes a skin object from the pool.
     * @param skin Pointer to the skin object to remove.
     * @return TRUE if the skin is successfully removed, FALSE otherwise.
     */
    STDMETHOD_(BOOL, RemoveSkin)(THIS_ ISkinObj *skin) OVERRIDE;

    /**
     * @brief Removes all skin objects from the pool.
     */
    STDMETHOD_(void, RemoveAll)(THIS) OVERRIDE;

  protected:
    /**
     * @brief Loads skins from an XML node.
     * @param xmlNode XML node containing the skin definitions.
     * @return Number of skins loaded.
     */
    int _LoadSkins(SXmlNode xmlNode);

    /**
     * @brief Loads a single skin from an XML node.
     * @param xmlNode XML node containing the skin definition.
     * @param nScale Scale factor for the skin.
     * @return Pointer to the loaded ISkinObj object, or nullptr if loading fails.
     */
    ISkinObj *_LoadSkin(SXmlNode xmlNode, int nScale);

    /**
     * @brief Callback function called when a skin object is removed from the pool.
     * @param obj Pointer to the removed skin object.
     */
    static void OnKeyRemoved(const SSkinPtr &obj);

#ifdef _DEBUG
    SMap<SkinKey, int> m_mapSkinUseCount; // Skin usage count map (debug only)
#endif

    BOOL m_bAutoScale; // Flag indicating if automatic scaling is enabled
};

SNSEND

#endif // __SSKINPOOL__H__