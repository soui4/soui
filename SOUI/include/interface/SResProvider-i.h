/**
 * Copyright (C) 2014-2050
 * All rights reserved.
 *
 * @file       SResProvider-i.h
 * @brief
 * @version    v1.0
 * @author     SOUI group
 * @date       2014/08/02
 *
 * Describe
 */

#ifndef _SRESPROVIDER_I_
#define _SRESPROVIDER_I_

#include <interface/obj-ref-i.h>
#include <interface/SRender-i.h>

#define UIRES_INDEX _T("uires.idx") /**< File name of the file mapping table index for folder resources */

#ifdef _WIN32
#define PATH_SLASH '\\'
#ifdef _UNICODE
#define TPATH_SLASH  L'\\'
#define TPATH_SLASH2 L"\\"
#else
#define TPATH_SLASH  '\\'
#define TPATH_SLASH2 "\\"
#endif
#else
#define PATH_SLASH '/'
#ifdef _UNICODE
#define TPATH_SLASH  L'/'
#define TPATH_SLASH2 L"/"
#else
#define TPATH_SLASH  '/'
#define TPATH_SLASH2 "/"
#endif
#endif /**< _WIN32 */

SNSBEGIN

typedef enum BUILTIN_RESTYPE
{
    RES_PE = 0,
    RES_FILE,
    RES_ANDROID_ASSET,
    RES_OHOS_RAWFILE,
} BUILTIN_RESTYPE;

typedef BOOL(CALLBACK *EnumResCallback)(LPCTSTR pszType, LPCTSTR pszName, LPARAM lp);
typedef BOOL(CALLBACK *EnumFileCallback)(LPCTSTR pszFileName, LPARAM lp);

/**
 * @struct     IResProvider
 * @brief      ResProvider object
 *
 * Describe   Implements loading of various resources
 */
#undef INTERFACE
#define INTERFACE IResProvider
DECLARE_INTERFACE_(IResProvider, IObjRef)
{
    /**
     * @brief Add reference
     * @return long -- reference count
     */
    STDMETHOD_(long, AddRef)(THIS) PURE;

    /**
     * @brief Release reference
     * @return long -- reference count
     */
    STDMETHOD_(long, Release)(THIS) PURE;

    /**
     * @brief Release object
     * @return void
     */
    STDMETHOD_(void, OnFinalRelease)(THIS) PURE;

    /**
     * @brief Resource initialization function
     * @param wParam WPARAM -- param 1
     * @param lParam LPARAM -- param 2
     * @return BOOL -- TRUE: success
     * @remark Every ResProvider must implement this interface
     */
    STDMETHOD_(BOOL, Init)(THIS_ WPARAM wParam, LPARAM lParam) PURE;

    /**
     * @brief Query whether a resource exists
     * @param pszType LPCTSTR -- resource type
     * @param pszResName LPCTSTR -- resource name
     * @return BOOL -- TRUE: exists, FALSE: not exists
     */
    STDMETHOD_(BOOL, HasResource)(THIS_ LPCTSTR pszType, LPCTSTR pszResName) PURE;

    /**
     * @brief Load ICON from resource
     * @param pszResName LPCTSTR -- ICON name
     * @param cx int -- ICON width
     * @param cy int -- ICON height
     * @return HICON -- returns ICON handle on success, 0 on failure
     */
    STDMETHOD_(HICON, LoadIcon)(THIS_ LPCTSTR pszResName, int cx, int cy) PURE;

    /**
     * @brief Load HBITMAP from resource
     * @param pszResName LPCTSTR -- BITMAP name
     * @return HBITMAP -- returns BITMAP handle on success, 0 on failure
     */
    STDMETHOD_(HBITMAP, LoadBitmap)(THIS_ LPCTSTR pszResName) PURE;

    /**
     * @brief Load cursor from resource
     * @param pszResName LPCTSTR -- cursor name
     * @return HCURSOR -- returns cursor handle on success, 0 on failure
     * @remark Supports animated cursors
     */
    STDMETHOD_(HCURSOR, LoadCursor)(THIS_ LPCTSTR pszResName) PURE;

    /**
     * @brief Load an IBitmap object from resource
     * @param pszType LPCTSTR -- image type
     * @param pszResName LPCTSTR -- image name
     * @return IBitmapS* -- returns an IBitmap object on success, 0 on failure
     * @remark If strType is not defined, FindImageType is used to automatically find a matching type based on name
     */
    STDMETHOD_(IBitmapS *, LoadImage)(THIS_ LPCTSTR pszType, LPCTSTR pszResName) PURE;

    /**
     * @brief Create an IImgX object from resource
     * @param pszType LPCTSTR -- image type
     * @param pszResName LPCTSTR -- image name
     * @return IImgX* -- returns an IImgX object on success, 0 on failure
     */
    STDMETHOD_(IImgX *, LoadImgX)(THIS_ LPCTSTR pszType, LPCTSTR pszResName) PURE;

    /**
     * @brief Get resource data size
     * @param pszType LPCTSTR -- resource type
     * @param pszResName LPCTSTR -- resource name
     * @return size_t -- resource size (byte), 0 on failure
     */
    STDMETHOD_(size_t, GetRawBufferSize)(THIS_ LPCTSTR pszType, LPCTSTR pszResName) PURE;

    /**
     * @brief Get resource memory block
     * @param pszType LPCTSTR -- resource type
     * @param pszResName LPCTSTR -- resource name
     * @param pBuf LPVOID -- output memory block
     * @param size size_t -- memory size
     * @return BOOL -- TRUE: success
     * @remark Should first query the resource size with GetRawBufferSize, then allocate enough space
     */
    STDMETHOD_(BOOL, GetRawBuffer)
    (THIS_ LPCTSTR pszType, LPCTSTR pszResName, LPVOID pBuf, size_t size) PURE;

    /**
     * @brief Enumerate resources
     * @param funEnumCB EnumResCallback -- callback function used for enumeration
     * @param lp LPARAM -- callback parameter
     * @return void
     */
    STDMETHOD_(void, EnumResource)(THIS_ EnumResCallback funEnumCB, LPARAM lp) PURE;

    /**
     * @brief Enumerate resource files, return file paths
     * @param funEnumCB EnumFileCallback -- callback function used for enumeration
     * @param lp LPARAM -- callback parameter
     * @return void
     */
    STDMETHOD_(void, EnumFile)(THIS_ EnumFileCallback funEnumCB, LPARAM lp) PURE;
};

SNSEND

#endif /**< _SRESPROVIDER_I_ */
