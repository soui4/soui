#ifndef __STRANSLATOR_I__H__
#define __STRANSLATOR_I__H__

#include <interface/obj-ref-i.h>
#include <interface/sstring-i.h>

SNSBEGIN

enum
{
    TR_MAX_NAME_LEN = 64
};

/**
 * @struct     ITranslator
 * @brief      Language Translation Interface
 *
 * This interface defines the methods for language translation.
 */
#undef INTERFACE
#define INTERFACE ITranslator
DECLARE_INTERFACE_(ITranslator, IObjRef)
{
    /**
     * @brief Increments the reference count.
     * @return The new reference count.
     */
    STDMETHOD_(long, AddRef)(THIS) PURE;

    /**
     * @brief Decrements the reference count.
     * @return The new reference count.
     */
    STDMETHOD_(long, Release)(THIS) PURE;

    /**
     * @brief Finalizes the release of the object.
     * @return void
     */
    STDMETHOD_(void, OnFinalRelease)(THIS) PURE;

    /**
     * @brief Loads language translation data from a resource.
     * @param pData Pointer to the resource data, interpretation depends on the implementation.
     * @param uType Type of the resource, interpretation depends on the implementation.
     * @return TRUE if loading is successful, FALSE otherwise.
     */
    STDMETHOD_(BOOL, Load)(THIS_ LPVOID pData, UINT uType) PURE;

    /**
     * @brief Retrieves the name of the translation resource.
     * @param szBuf Buffer to receive the translation resource name.
     * @return void
     */
    STDMETHOD_(void, GetName)(CTHIS_ wchar_t szBuf[TR_MAX_NAME_LEN]) SCONST PURE;

    /**
     * @brief Retrieves the name of the translation resource.
     * @param str Pointer to the IStringA object to receive the translation resource name.
     * @return void
     */
    STDMETHOD_(void, GetNameA)(CTHIS_ IStringA * str) SCONST PURE;

    /**
     * @brief Compares the translation resource name.
     * @param pszName Translation resource name to compare.
     * @return TRUE if the names are the same, FALSE otherwise.
     */
    STDMETHOD_(BOOL, NameEqual)(CTHIS_ LPCWSTR pszName) SCONST PURE;

    /**
     * @brief Compares the translation resource name.
     * @param str Pointer to the IStringA object containing the translation resource name to compare.
     * @return TRUE if the names are the same, FALSE otherwise.
     */
    STDMETHOD_(BOOL, NameEqualA)(CTHIS_ const IStringA *str) SCONST PURE;

    /**
     * @brief Retrieves the GUID of the translation resource.
     * @return GUID of the translation resource.
     */
    STDMETHOD_(GUID, guid)(CTHIS) SCONST PURE;

    /**
     * @brief Translates a string.
     * @param strSrc Pointer to the IStringW object containing the source string.
     * @param strCtx Pointer to the IStringW object containing the translation context.
     * @param pszOut Buffer to receive the translated string.
     * @param nLen Length of the buffer.
     * @return Length of the translated string, 0 if no translation, -1 if buffer size is not enough.
     */
    STDMETHOD_(int, tr)
    (CTHIS_ const IStringW *strSrc, const IStringW *strCtx, wchar_t *pszOut, int nLen) SCONST PURE;

    /**
     * @brief Retrieves the default font information for translation.
     * @param strFont Pointer to the IStringW object to receive the font information.
     * @return void
     */
    STDMETHOD_(void, getFontInfo)(CTHIS_ IStringW * strFont) SCONST PURE;
};

/**
 * @struct     ITranslatorMgr
 * @brief      Language Translation Interface Manager
 *
 * This interface defines the methods for managing language translation interfaces.
 */
#undef INTERFACE
#define INTERFACE ITranslatorMgr
DECLARE_INTERFACE_(ITranslatorMgr, IObjRef)
{
    /**
     * @brief Increments the reference count.
     * @return The new reference count.
     */
    STDMETHOD_(long, AddRef)(THIS) PURE;

    /**
     * @brief Decrements the reference count.
     * @return The new reference count.
     */
    STDMETHOD_(long, Release)(THIS) PURE;

    /**
     * @brief Finalizes the release of the object.
     * @return void
     */
    STDMETHOD_(void, OnFinalRelease)(THIS) PURE;

    /**
     * @brief Checks if the manager is valid.
     * @return TRUE if valid, FALSE otherwise.
     */
    STDMETHOD_(BOOL, IsValid)(CTHIS) SCONST PURE;

    /**
     * @brief Sets the current language for the translation module.
     * @param pszLang Language to set.
     * @return void
     * @remark Automatically clears modules with different languages.
     */
    STDMETHOD_(void, SetLanguage)(THIS_ LPCWSTR pszLang) PURE;

    /**
     * @brief Sets the current language for the translation module.
     * @param pszLang Language to set.
     * @return void
     * @remark Automatically clears modules with different languages.
     */
    STDMETHOD_(void, SetLanguageA)(THIS_ LPCSTR pszLang) PURE;

    /**
     * @brief Retrieves the current language for the translation module.
     * @param szOut Buffer to receive the current language.
     * @return void
     */
    STDMETHOD_(void, GetLanguage)(CTHIS_ wchar_t szOut[TR_MAX_NAME_LEN]) SCONST PURE;

    /**
     * @brief Retrieves the current language for the translation module.
     * @param out Pointer to the IStringA object to receive the current language.
     * @return void
     */
    STDMETHOD_(void, GetLanguageA)(CTHIS_ IStringA * out) SCONST PURE;

    /**
     * @brief Creates a language translation object.
     * @param ppTranslator Pointer to receive the language translation object.
     * @return TRUE if successful, FALSE otherwise.
     */
    STDMETHOD_(BOOL, CreateTranslator)(THIS_ ITranslator * *ppTranslator) PURE;

    /**
     * @brief Installs a language translation object into the manager.
     * @param ppTranslator Language translation object to install.
     * @return TRUE if successful, FALSE otherwise.
     */
    STDMETHOD_(BOOL, InstallTranslator)(THIS_ ITranslator * ppTranslator) PURE;

    /**
     * @brief Uninstalls a language translation object from the manager.
     * @param id GUID of the language translation object to uninstall.
     * @return TRUE if successful, FALSE otherwise.
     */
    STDMETHOD_(BOOL, UninstallTranslator)(THIS_ REFGUID id) PURE;

    /**
     * @brief Translates a string.
     * @param strSrc Pointer to the IStringW object containing the source string.
     * @param strCtx Pointer to the IStringW object containing the translation context.
     * @param pszOut Buffer to receive the translated string.
     * @param nLen Length of the buffer.
     * @return Length of the translated string, 0 if no translation, -1 if buffer size is not enough.
     */
    STDMETHOD_(int, tr)
    (CTHIS_ const IStringW *strSrc, const IStringW *strCtx, wchar_t *pszOut, int nLen) SCONST PURE;
};

SNSEND
#endif // __STRANSLATOR_I__H__