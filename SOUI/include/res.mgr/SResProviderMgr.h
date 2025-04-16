#ifndef __SRESPROVIDERMGR__H__
#define __SRESPROVIDERMGR__H__

#include <windows.h>
#include <interface/SResProviderMgr-i.h>
#include <layout/SLayoutSize.h>
#include <helper/SCriticalSection.h>
#include <helper/SAutoBuf.h>

SNSBEGIN

/**
 * @class SResProviderMgr
 * @brief Resource Provider Manager
 *
 * @details Manages multiple resource providers and provides methods to load and manage resources.
 */
class SOUI_EXP SResProviderMgr : public IResProviderMgr {
  public:
    /**
     * @brief Constructor.
     */
    SResProviderMgr(void);

    /**
     * @brief Destructor.
     */
    ~SResProviderMgr(void);

  public:
    /**
     * @brief Adds a resource provider with a specified UI definition.
     * @param pResProvider Pointer to the resource provider to add.
     * @param pszUidef UI definition string (default is "uidef:xml_init").
     */
    STDMETHOD_(void, AddResProvider)(THIS_ IResProvider *pResProvider, LPCTSTR pszUidef DEF_VAL(_T("uidef:xml_init"))) OVERRIDE;

    /**
     * @brief Adds a resource provider with a specified UI definition (ANSI version).
     * @param pResProvider Pointer to the resource provider to add.
     * @param pszUidef UI definition string (default is "uidef:xml_init").
     */
    STDMETHOD_(void, AddResProviderA)(THIS_ IResProvider *pResProvider, LPCSTR pszUidef DEF_VAL("uidef:xml_init")) OVERRIDE;

    /**
     * @brief Removes a resource provider.
     * @param pResProvider Pointer to the resource provider to remove.
     */
    STDMETHOD_(void, RemoveResProvider)(THIS_ IResProvider *pResProvider) OVERRIDE;

    /**
     * @brief Removes all resource providers.
     */
    STDMETHOD_(void, RemoveAll)(THIS) OVERRIDE;

    /**
     * @brief Retrieves the first resource provider in the list.
     * @return Pointer to the first resource provider, or nullptr if the list is empty.
     */
    STDMETHOD_(IResProvider *, GetHeadResProvider)(THIS) SCONST OVERRIDE;

    /**
     * @brief Retrieves the last resource provider in the list.
     * @return Pointer to the last resource provider, or nullptr if the list is empty.
     */
    STDMETHOD_(IResProvider *, GetTailResProvider)(THIS) SCONST OVERRIDE;

    /**
     * @brief Sets the file prefix for resource paths.
     * @param pszFilePrefix File prefix to set.
     */
    STDMETHOD_(void, SetFilePrefix)(THIS_ LPCTSTR pszFilePrefix) OVERRIDE;

    //////////////////////////////////////////////////////////////////////////
    /**
     * @brief Checks if a resource exists.
     * @param pszType Type of the resource.
     * @param pszResName Name of the resource.
     * @return TRUE if the resource exists, FALSE otherwise.
     */
    STDMETHOD_(BOOL, HasResource)(THIS_ LPCTSTR pszType, LPCTSTR pszResName) OVERRIDE;

    /**
     * @brief Loads an icon resource.
     * @param pszResName Name of the icon resource.
     * @param cx Desired width of the icon (default is 0).
     * @param cy Desired height of the icon (default is 0).
     * @param bFromFile TRUE if the resource should be loaded from a file, FALSE otherwise (default is FALSE).
     * @return Handle to the loaded icon, or nullptr if loading fails.
     */
    STDMETHOD_(HICON, LoadIcon)(THIS_ LPCTSTR pszResName, int cx = 0, int cy = 0, BOOL bFromFile = FALSE) OVERRIDE;

    /**
     * @brief Loads a cursor resource.
     * @param pszResName Name of the cursor resource.
     * @param bFromFile TRUE if the resource should be loaded from a file, FALSE otherwise (default is FALSE).
     * @return Handle to the loaded cursor, or nullptr if loading fails.
     */
    STDMETHOD_(HCURSOR, LoadCursor)(THIS_ LPCTSTR pszResName, BOOL bFromFile = FALSE) OVERRIDE;

    /**
     * @brief Loads a bitmap resource.
     * @param pszResName Name of the bitmap resource.
     * @param bFromFile TRUE if the resource should be loaded from a file, FALSE otherwise (default is FALSE).
     * @return Handle to the loaded bitmap, or nullptr if loading fails.
     */
    STDMETHOD_(HBITMAP, LoadBitmap)(THIS_ LPCTSTR pszResName, BOOL bFromFile = FALSE) OVERRIDE;

    /**
     * @brief Loads an image resource as an IBitmapS object.
     * @param pszType Type of the resource.
     * @param pszResName Name of the resource.
     * @return Pointer to the loaded IBitmapS object, or nullptr if loading fails.
     */
    STDMETHOD_(IBitmapS *, LoadImage)(THIS_ LPCTSTR pszType, LPCTSTR pszResName) OVERRIDE;

    /**
     * @brief Loads an image resource as an IImgX object.
     * @param pszType Type of the resource.
     * @param pszResName Name of the resource.
     * @return Pointer to the loaded IImgX object, or nullptr if loading fails.
     */
    STDMETHOD_(IImgX *, LoadImgX)(THIS_ LPCTSTR pszType, LPCTSTR pszResName) OVERRIDE;

    /**
     * @brief Retrieves the size of the raw buffer for a resource.
     * @param pszType Type of the resource.
     * @param pszResName Name of the resource.
     * @return Size of the raw buffer, or 0 if the resource does not exist.
     */
    STDMETHOD_(size_t, GetRawBufferSize)(THIS_ LPCTSTR pszType, LPCTSTR pszResName) OVERRIDE;

    /**
     * @brief Retrieves the raw buffer for a resource.
     * @param pszType Type of the resource.
     * @param pszResName Name of the resource.
     * @param pBuf Buffer to store the raw data.
     * @param size Size of the buffer.
     * @return TRUE if the raw buffer is successfully retrieved, FALSE otherwise.
     */
    STDMETHOD_(BOOL, GetRawBuffer)(THIS_ LPCTSTR pszType, LPCTSTR pszResName, LPVOID pBuf, size_t size) OVERRIDE;

  public:
    // Helper methods

    /**
     * @brief Finds the resource provider that contains the specified resource type and name.
     * @param pszType Type of the resource.
     * @param pszResName Name of the resource.
     * @return Pointer to the resource provider, or nullptr if not found.
     */
    IResProvider *GetMatchResProvider(LPCTSTR pszType, LPCTSTR pszResName);

    /**
     * @brief Loads an image using a type:name formatted string.
     * @param strImgID Type:name formatted string.
     * @return Pointer to the loaded IBitmapS object, or nullptr if loading fails.
     */
    IBitmapS *LoadImage2(const SStringW &strImgID);

    /**
     * @brief Loads an icon using a name:size formatted string.
     * @param strIconID Name:size formatted string.
     * @return Handle to the loaded icon, or nullptr if loading fails.
     */
    HICON LoadIcon2(const SStringW &strIconID);

    /**
     * @brief Loads the raw buffer for a resource using a specified resource provider.
     * @param pszType Type of the resource.
     * @param pszResName Name of the resource.
     * @param pResProvider Pointer to the resource provider.
     * @param buf Buffer to store the raw data.
     * @return TRUE if the raw buffer is successfully loaded, FALSE otherwise.
     */
    BOOL LoadRawBuffer(LPCTSTR pszType, LPCTSTR pszResName, IResProvider *pResProvider, SAutoBuf &buf);

  protected:
#ifdef _DEBUG
    /**
     * @brief Callback function to check resource usage (debug only).
     * @param pszName Name of the resource.
     * @param pszType Type of the resource.
     * @param lp User-defined parameter.
     * @return TRUE if the resource is in use, FALSE otherwise.
     */
    static BOOL CALLBACK CheckUsage(LPCTSTR pszName, LPCTSTR pszType, LPARAM lp);
#endif

    /**
     * @brief Converts a system cursor name to its corresponding ID.
     * @param pszCursorName Name of the system cursor.
     * @return ID of the system cursor.
     */
    LPCTSTR SysCursorName2ID(LPCTSTR pszCursorName);

    /**
     * @brief Checks if the resource type is an external file.
     * @param pszType Type of the resource.
     * @return TRUE if the resource type is an external file, FALSE otherwise.
     */
    BOOL IsFileType(LPCTSTR pszType);

    SStringT m_strFilePrefix;              // File prefix for resource paths
    SList<IResProvider *> m_lstResPackage; // List of resource providers

    typedef SMap<SStringT, HCURSOR> CURSORMAP;
    CURSORMAP m_mapCachedCursor; // Map of cached cursors

    SCriticalSection m_cs; // Critical section for thread safety

#ifdef _DEBUG
    // Resource usage count map (debug only)
    SMap<SStringT, int> m_mapResUsageCount;
#endif
};

SNSEND

#endif // __SRESPROVIDERMGR__H__