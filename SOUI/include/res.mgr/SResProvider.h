#ifndef __SRESPROVIDER__H__
#define __SRESPROVIDER__H__

#include <interface/SResProvider-i.h>
#include <helper/SResID.h>
#include <helper/obj-ref-impl.hpp>

SNSBEGIN

// Define system resource types
extern const TCHAR KTypeBitmap[];
extern const TCHAR KTypeCursor[];
extern const TCHAR KTypeIcon[];
extern const TCHAR KTypeHtml[];

/**
 * @class SResLoadFromMemory
 * @brief Utility class for loading resources from memory.
 */
class SOUI_EXP SResLoadFromMemory {
  public:
    /**
     * @brief Loads an image from a memory buffer.
     * @param pBuf Pointer to the memory buffer containing the image data.
     * @param size Size of the memory buffer.
     * @return Pointer to the loaded IBitmapS object, or nullptr if loading fails.
     */
    static IBitmapS *LoadImage(LPVOID pBuf, size_t size);

    /**
     * @brief Loads an image from a memory buffer as an IImgX object.
     * @param pBuf Pointer to the memory buffer containing the image data.
     * @param size Size of the memory buffer.
     * @return Pointer to the loaded IImgX object, or nullptr if loading fails.
     */
    static IImgX *LoadImgX(LPVOID pBuf, size_t size);
};

#ifdef _WIN32

/**
 * @class SResProviderPE
 * @brief Resource provider for resources embedded in a PE file (Windows-specific).
 */
class SOUI_EXP SResProviderPE : public TObjRefImpl<IResProvider> {
  public:
    /**
     * @brief Constructor.
     */
    SResProviderPE();

    /**
     * @brief Destructor.
     */
    ~SResProviderPE();

  public:
    /**
     * @brief Initializes the resource provider.
     * @param wParam Additional initialization parameter (wParam).
     * @param lParam Additional initialization parameter (lParam).
     * @return TRUE if initialization is successful, FALSE otherwise.
     */
    STDMETHOD_(BOOL, Init)(THIS_ WPARAM wParam, LPARAM lParam);

    /**
     * @brief Checks if a resource exists.
     * @param pszType Type of the resource.
     * @param pszResName Name of the resource.
     * @return TRUE if the resource exists, FALSE otherwise.
     */
    STDMETHOD_(BOOL, HasResource)(THIS_ LPCTSTR pszType, LPCTSTR pszResName);

    /**
     * @brief Loads an icon resource.
     * @param pszResName Name of the icon resource.
     * @param cx Desired width of the icon.
     * @param cy Desired height of the icon.
     * @return Handle to the loaded icon, or nullptr if loading fails.
     */
    STDMETHOD_(HICON, LoadIcon)(THIS_ LPCTSTR pszResName, int cx, int cy);

    /**
     * @brief Loads a bitmap resource.
     * @param pszResName Name of the bitmap resource.
     * @return Handle to the loaded bitmap, or nullptr if loading fails.
     */
    STDMETHOD_(HBITMAP, LoadBitmap)(THIS_ LPCTSTR pszResName);

    /**
     * @brief Loads a cursor resource.
     * @param pszResName Name of the cursor resource.
     * @return Handle to the loaded cursor, or nullptr if loading fails.
     */
    STDMETHOD_(HCURSOR, LoadCursor)(THIS_ LPCTSTR pszResName);

    /**
     * @brief Loads an image resource as an IBitmapS object.
     * @param pszType Type of the resource.
     * @param pszResName Name of the resource.
     * @return Pointer to the loaded IBitmapS object, or nullptr if loading fails.
     */
    STDMETHOD_(IBitmapS *, LoadImage)(THIS_ LPCTSTR pszType, LPCTSTR pszResName);

    /**
     * @brief Loads an image resource as an IImgX object.
     * @param pszType Type of the resource.
     * @param pszResName Name of the resource.
     * @return Pointer to the loaded IImgX object, or nullptr if loading fails.
     */
    STDMETHOD_(IImgX *, LoadImgX)(THIS_ LPCTSTR pszType, LPCTSTR pszResName);

    /**
     * @brief Retrieves the size of the raw buffer for a resource.
     * @param pszType Type of the resource.
     * @param pszResName Name of the resource.
     * @return Size of the raw buffer, or 0 if the resource does not exist.
     */
    STDMETHOD_(size_t, GetRawBufferSize)(THIS_ LPCTSTR pszType, LPCTSTR pszResName);

    /**
     * @brief Retrieves the raw buffer for a resource.
     * @param pszType Type of the resource.
     * @param pszResName Name of the resource.
     * @param pBuf Buffer to store the raw data.
     * @param size Size of the buffer.
     * @return TRUE if the raw buffer is successfully retrieved, FALSE otherwise.
     */
    STDMETHOD_(BOOL, GetRawBuffer)(THIS_ LPCTSTR pszType, LPCTSTR pszResName, LPVOID pBuf, size_t size);

    /**
     * @brief Enumerates resources of a specific type.
     * @param funEnumCB Callback function to process each resource.
     * @param lp User-defined parameter passed to the callback function.
     */
    STDMETHOD_(void, EnumResource)(THIS_ EnumResCallback funEnumCB, LPARAM lp);

    /**
     * @brief Enumerates files in the resource provider.
     * @param funEnumCB Callback function to process each file.
     * @param lp User-defined parameter passed to the callback function.
     */
    STDMETHOD_(void, EnumFile)(THIS_ EnumFileCallback funEnumCB, LPARAM lp);

  protected:
    /**
     * @brief Retrieves a pointer to the raw buffer for a resource.
     * @param strType Type of the resource.
     * @param pszResName Name of the resource.
     * @return Pointer to the raw buffer, or nullptr if the resource does not exist.
     */
    LPVOID GetRawBufferPtr(LPCTSTR strType, LPCTSTR pszResName);

    /**
     * @brief Finds a resource in the PE file.
     * @param strType Type of the resource.
     * @param pszResName Name of the resource.
     * @return Handle to the resource, or nullptr if not found.
     */
    HRSRC MyFindResource(LPCTSTR strType, LPCTSTR pszResName);

    HINSTANCE m_hResInst; // Handle to the resource instance
    BOOL m_bOwner;        // Flag indicating if the instance is owned
};

#endif // _WIN32

/**
 * @class SResLoadFromFile
 * @brief Utility class for loading resources from files.
 */
class SOUI_EXP SResLoadFromFile {
  public:
    /**
     * @brief Loads a bitmap from a file.
     * @param pszFileName Name of the file containing the bitmap.
     * @return Handle to the loaded bitmap, or nullptr if loading fails.
     */
    static HBITMAP LoadBitmap(LPCTSTR pszFileName);

    /**
     * @brief Loads an icon from a file.
     * @param pszFileName Name of the file containing the icon.
     * @param cx Desired width of the icon.
     * @param cy Desired height of the icon.
     * @return Handle to the loaded icon, or nullptr if loading fails.
     */
    static HICON LoadIcon(LPCTSTR pszFileName, int cx = 0, int cy = 0);

    /**
     * @brief Loads a cursor from a file.
     * @param pszFileName Name of the file containing the cursor.
     * @return Handle to the loaded cursor, or nullptr if loading fails.
     */
    static HCURSOR LoadCursor(LPCTSTR pszFileName);

    /**
     * @brief Loads an image from a file as an IBitmapS object.
     * @param pszFileName Name of the file containing the image.
     * @return Pointer to the loaded IBitmapS object, or nullptr if loading fails.
     */
    static IBitmapS *LoadImage(LPCTSTR pszFileName);

    /**
     * @brief Loads an image from a file as an IImgX object.
     * @param pszFileName Name of the file containing the image.
     * @return Pointer to the loaded IImgX object, or nullptr if loading fails.
     */
    static IImgX *LoadImgX(LPCTSTR pszFileName);

    /**
     * @brief Retrieves the size of the raw buffer for a file.
     * @param pszFileName Name of the file.
     * @return Size of the raw buffer, or 0 if the file does not exist.
     */
    static size_t GetRawBufferSize(LPCTSTR pszFileName);

    /**
     * @brief Retrieves the raw buffer for a file.
     * @param pszFileName Name of the file.
     * @param pBuf Buffer to store the raw data.
     * @param size Size of the buffer.
     * @return TRUE if the raw buffer is successfully retrieved, FALSE otherwise.
     */
    static BOOL GetRawBuffer(LPCTSTR pszFileName, LPVOID pBuf, size_t size);
};

/**
 * @class SResProviderFiles
 * @brief Resource provider for resources stored in files.
 */
class SOUI_EXP SResProviderFiles : public TObjRefImpl<IResProvider> {
  public:
    /**
     * @brief Constructor.
     */
    SResProviderFiles();

    /**
     * @brief Initializes the resource provider.
     * @param wParam Additional initialization parameter (wParam).
     * @param lParam Additional initialization parameter (lParam).
     * @return TRUE if initialization is successful, FALSE otherwise.
     */
    STDMETHOD_(BOOL, Init)(THIS_ WPARAM wParam, LPARAM lParam) OVERRIDE;

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
     * @param cx Desired width of the icon.
     * @param cy Desired height of the icon.
     * @return Handle to the loaded icon, or nullptr if loading fails.
     */
    STDMETHOD_(HICON, LoadIcon)(THIS_ LPCTSTR pszResName, int cx, int cy) OVERRIDE;

    /**
     * @brief Loads a bitmap resource.
     * @param pszResName Name of the bitmap resource.
     * @return Handle to the loaded bitmap, or nullptr if loading fails.
     */
    STDMETHOD_(HBITMAP, LoadBitmap)(THIS_ LPCTSTR pszResName) OVERRIDE;

    /**
     * @brief Loads a cursor resource.
     * @param pszResName Name of the cursor resource.
     * @return Handle to the loaded cursor, or nullptr if loading fails.
     */
    STDMETHOD_(HCURSOR, LoadCursor)(THIS_ LPCTSTR pszResName) OVERRIDE;

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

    /**
     * @brief Enumerates resources of a specific type.
     * @param funEnumCB Callback function to process each resource.
     * @param lp User-defined parameter passed to the callback function.
     */
    STDMETHOD_(void, EnumResource)(THIS_ EnumResCallback funEnumCB, LPARAM lp) OVERRIDE;

    /**
     * @brief Enumerates files in the resource provider.
     * @param funEnumCB Callback function to process each file.
     * @param lp User-defined parameter passed to the callback function.
     */
    STDMETHOD_(void, EnumFile)(THIS_ EnumFileCallback funEnumCB, LPARAM lp) OVERRIDE;

  protected:
    /**
     * @brief Enumerates files in a specified path.
     * @param pszPath Path to the directory containing the files.
     * @param funEnumCB Callback function to process each file.
     * @param lp User-defined parameter passed to the callback function.
     */
    void _EnumFile(LPCTSTR pszPath, EnumFileCallback funEnumCB, LPARAM lp);

    /**
     * @brief Retrieves the resource path for a given type and name.
     * @param strType Type of the resource.
     * @param pszResName Name of the resource.
     * @return Full path to the resource file, or an empty string if not found.
     */
    SStringT GetRes(LPCTSTR strType, LPCTSTR pszResName);

    SStringT m_strPath;                // Base path for resource files
    SMap<SResID, SStringT> m_mapFiles; // Map of resource IDs to file paths
};

SNSEND

#endif // __SRESPROVIDER__H__