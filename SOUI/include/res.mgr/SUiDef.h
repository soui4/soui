#ifndef __SUIDEF__H__
#define __SUIDEF__H__

#include <core/SSingleton2.h>
#include <interface/obj-ref-i.h>
#include <interface/SResProvider-i.h>

#include <res.mgr/SFontPool.h>
#include <res.mgr/SSkinPool.h>
#include <res.mgr/SStylePool.h>
#include <res.mgr/SGradientPool.h>
#include <res.mgr/SObjDefAttr.h>
#include <res.mgr/SNamedValue.h>

SNSBEGIN

enum
{
    UDI_GLOBAL = 1 << 0,
    UDI_SKIN = 1 << 1,
    UDI_STYLE = 1 << 2,
    UDI_TEMPLATE = 1 << 3,
    UDI_COLOR = 1 << 4,
    UDI_STRING = 1 << 5,
    UDI_DIMENSION = 1 << 6,
    UDI_FONT = 1 << 7,
    UDI_GRADIENT = 1 << 8,
};

/**
 * @interface IUiDefInfo
 * @brief Interface for UI definition information.
 */
struct IUiDefInfo : IObjRef
{
    /**
     * @brief Initializes the UI definition from a resource provider.
     * @param pResProvider Pointer to the resource provider.
     * @param pszUidef UI definition resource ID.
     * @return Number of resources initialized.
     */
    virtual UINT Init(IResProvider *pResProvider, LPCTSTR pszUidef) = 0;

    /**
     * @brief Initializes the UI definition from an XML node.
     * @param pNode XML node containing the UI definition.
     * @param bGlobalDomain TRUE if the domain is global, FALSE otherwise.
     * @param pResProvider Pointer to the resource provider (optional).
     * @return Number of resources initialized.
     */
    virtual UINT Init2(IXmlNode *pNode, BOOL bGlobalDomain, IResProvider *pResProvider = NULL) = 0;

    /**
     * @brief Retrieves the skin pool.
     * @return Pointer to the skin pool.
     */
    virtual SSkinPool *GetSkinPool() = 0;

    /**
     * @brief Retrieves the style pool.
     * @return Pointer to the style pool.
     */
    virtual SStylePool *GetStylePool() = 0;

    /**
     * @brief Retrieves the template pool.
     * @return Pointer to the template pool.
     */
    virtual STemplatePool *GetTemplatePool() = 0;

    /**
     * @brief Retrieves the gradient pool.
     * @return Pointer to the gradient pool.
     */
    virtual SGradientPool *GetGradientPool() = 0;

    /**
     * @brief Retrieves the named color manager.
     * @return Reference to the named color manager.
     */
    virtual SNamedColor &GetNamedColor() = 0;

    /**
     * @brief Retrieves the named string manager.
     * @return Reference to the named string manager.
     */
    virtual SNamedString &GetNamedString() = 0;

    /**
     * @brief Retrieves the named dimension manager.
     * @return Reference to the named dimension manager.
     */
    virtual SNamedDimension &GetNamedDimension() = 0;

    /**
     * @brief Retrieves the named font manager.
     * @return Reference to the named font manager.
     */
    virtual SNamedFont &GetNamedFont() = 0;

    /**
     * @brief Retrieves the default font information.
     * @return Default font information as a string.
     */
    virtual SStringW GetDefFontInfo() = 0;

    /**
     * @brief Retrieves the caret information.
     * @return XML node containing the caret information.
     */
    virtual SXmlNode GetCaretInfo() = 0;

    /**
     * @brief Retrieves the object default attribute manager.
     * @return Pointer to the object default attribute manager.
     */
    virtual SObjDefAttr *GetObjDefAttr() = 0;
};

#define GETUIDEF SNS::SUiDef::getSingletonPtr()

#define GETSTYLE(p)                GETUIDEF->GetStyle(p)
#define GETSKIN(p1, scale)         GETUIDEF->GetSkin(p1, scale)
#define GETBUILTINSKIN(p1)         GETUIDEF->GetBuiltinSkin(p1, 100)
#define GETBUILTINSKIN2(p1, scale) GETUIDEF->GetBuiltinSkin(p1, scale)
#define GETCOLOR(x)                GETUIDEF->GetColor(x)
#define GETSTRING(x)               GETUIDEF->GetString(x)
#define GETLAYOUTSIZE(x)           GETUIDEF->GetLayoutSize(x)

/**
 * @class SUiDef
 * @brief Manages UI definitions, including skins, styles, templates, gradients, and named values.
 *
 * @details This class provides functionality to manage and retrieve various UI resources such as skins, styles, templates,
 *          gradients, and named values. It also handles the initialization and management of default UI definitions.
 */
class SOUI_EXP SUiDef
    : public SSingleton2<SUiDef>
    , public SFontPool {
    SINGLETON2_TYPE(SINGLETON_UIDEF)

  public:
    /**
     * @brief Constructor.
     * @param fac Pointer to the render factory.
     */
    SUiDef(IRenderFactory *fac);

    /**
     * @brief Destructor.
     */
    ~SUiDef(void);

  public:
    /**
     * @brief Creates a new UI definition information object.
     * @return Pointer to the created UI definition information object.
     */
    static IUiDefInfo *CreateUiDefInfo();

    /**
     * @brief Creates a new skin pool.
     * @param bAutoScale TRUE if automatic scaling is enabled, FALSE otherwise.
     * @return Pointer to the created skin pool.
     */
    static ISkinPool *CreateSkinPool(BOOL bAutoScale = TRUE);

    /**
     * @brief Initializes the default UI definition.
     * @param pResProvider Pointer to the resource provider.
     * @param pszUiDef UI definition resource ID.
     * @return TRUE if initialization is successful, FALSE otherwise.
     */
    BOOL InitDefUiDef(IResProvider *pResProvider, LPCTSTR pszUiDef);

    /**
     * @brief Retrieves the default UI definition.
     * @return Pointer to the default UI definition information object.
     */
    IUiDefInfo *GetUiDef();

    /**
     * @brief Sets the default UI definition.
     * @param pUiDefInfo Pointer to the UI definition information object.
     * @param bUpdateDefFont TRUE if the default font should be updated, FALSE otherwise.
     */
    void SetUiDef(IUiDefInfo *pUiDefInfo, bool bUpdateDefFont);

  public:
    /**
     * @brief Pushes a new UI definition information object onto the stack.
     * @param pUiDefInfo Pointer to the UI definition information object.
     * @param bPreivate TRUE if the UI definition is private, FALSE otherwise.
     */
    void PushUiDefInfo(IUiDefInfo *pUiDefInfo, BOOL bPreivate = FALSE);

    /**
     * @brief Pops a UI definition information object from the stack.
     * @param pUiDefInfo Pointer to the UI definition information object to pop. If NULL, pops the last object.
     * @param bPreivate TRUE if the UI definition is private, FALSE otherwise.
     * @return TRUE if the UI definition is successfully popped, FALSE otherwise.
     */
    BOOL PopUiDefInfo(IUiDefInfo *pUiDefInfo, BOOL bPreivate = FALSE);

    /**
     * @brief Pushes a new skin pool onto the stack.
     * @param pSkinPool Pointer to the skin pool.
     */
    void PushSkinPool(ISkinPool *pSkinPool);

    /**
     * @brief Pops a skin pool from the stack.
     * @param pSkinPool Pointer to the skin pool to pop.
     * @return TRUE if the skin pool is successfully popped, FALSE otherwise.
     */
    BOOL PopSkinPool(ISkinPool *pSkinPool);

  public:
    /**
     * @brief Retrieves a skin object by name and scale.
     * @param strSkinName Name of the skin.
     * @param nScale Scale factor for the skin.
     * @return Pointer to the skin object, or nullptr if not found.
     */
    ISkinObj *GetSkin(const SStringW &strSkinName, int nScale);

    /**
     * @brief Retrieves a built-in skin object by ID and scale.
     * @param uID ID of the built-in skin.
     * @param nScale Scale factor for the skin.
     * @return Pointer to the built-in skin object, or nullptr if not found.
     */
    ISkinObj *GetBuiltinSkin(SYS_SKIN uID, int nScale);

    /**
     * @brief Retrieves the built-in skin pool.
     * @return Pointer to the built-in skin pool.
     */
    ISkinPool *GetBuiltinSkinPool();

    /**
     * @brief Retrieves a style XML node by name.
     * @param strName Name of the style.
     * @return XML node containing the style, or an invalid node if not found.
     */
    SXmlNode GetStyle(const SStringW &strName);

    /**
     * @brief Retrieves a template string by name.
     * @param strName Name of the template.
     * @return Template string, or an empty string if not found.
     */
    SStringW GetTemplateString(const SStringW &strName);

    /**
     * @brief Retrieves a gradient object by name.
     * @param strName Name of the gradient.
     * @return Pointer to the gradient object, or nullptr if not found.
     */
    IGradient *GetGradient(const SStringW &strName);

    /**
     * @brief Retrieves a color value from a string.
     * @param strColor Color string (e.g., @color/red, rgba(r,g,b,a), rgb(r,g,b), #ff0000).
     * @return Color value as COLORREF.
     */
    COLORREF GetColor(const SStringW &strColor);

    /**
     * @brief Retrieves a color value by index from the default resource package.
     * @param idx Index of the color.
     * @return Color value as COLORREF.
     */
    COLORREF GetColor(int idx);

    /**
     * @brief Retrieves a string value from a string name.
     * @param strString String name (e.g., @string/title).
     * @return String value, or an empty string if not found.
     */
    SStringW GetString(const SStringW &strString);

    /**
     * @brief Retrieves a string value by index from the default resource package.
     * @param idx Index of the string.
     * @return String value, or an empty string if not found.
     */
    SStringW GetString(int idx);

    /**
     * @brief Retrieves a layout size from a string name.
     * @param strSize String name (e.g., @dim/dimname).
     * @return Layout size as SLayoutSize.
     */
    SLayoutSize GetLayoutSize(const SStringW &strSize);

    /**
     * @brief Retrieves a layout size by index from the default resource package.
     * @param idx Index of the layout size.
     * @return Layout size as SLayoutSize.
     */
    SLayoutSize GetLayoutSize(int idx);

    /**
     * @brief Retrieves a font description from a string name.
     * @param strFont String name (e.g., @font/fontname).
     * @return Font description as a string.
     */
    SStringW GetFontDesc(const SStringW &strFont);

    /**
     * @brief Retrieves a font description by index from the default resource package.
     * @param idx Index of the font description.
     * @return Font description as a string.
     */
    SStringW GetFontDesc(int idx);

  public:
    /**
     * @brief Retrieves a font object by description and scale.
     * @param strFont Font description string.
     * @param scale Scale factor for the font.
     * @return Pointer to the font object, or nullptr if not found.
     *
     * @details Description string format example: face:宋体,bold:0,italic:1,underline:1,strike:1,adding:10
     */
    IFontPtr GetFont(const SStringW &strFont, int scale);

    /**
     * @brief Sets the default font information.
     * @param strFontInfo Default font description.
     */
    void SetDefFontInfo(const SStringW &strFontInfo);

    /**
     * @brief Retrieves the default font information.
     * @return Font information as FontInfo.
     */
    FontInfo GetDefFontInfo() const;

  protected:
    SAutoRefPtr<IUiDefInfo> m_defUiDefInfo;   // Default UI definition information
    SList<IUiDefInfo *> m_lstUiDefInfo;       // List of UI definition information objects
    SList<ISkinPool *> m_lstSkinPools;        // List of skin pools
    SAutoRefPtr<ISkinPool> m_bulitinSkinPool; // Built-in skin pool
    mutable SCriticalSection m_cs;            // Critical section for thread safety
};

SNSEND

#endif // __SUIDEF__H__