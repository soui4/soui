#ifndef __SRESPROVIDERMGR_I__H__
#define __SRESPROVIDERMGR_I__H__
#include <interface/SResProvider-i.h>

SNSBEGIN

#undef INTERFACE
#define INTERFACE IResProviderMgr
DECLARE_INTERFACE(IResProviderMgr)
{
    /**
     * @brief Add resource provider
     * @param pResProvider IResProvider* -- resource provider object
     * @param pszUidef LPCTSTR -- UI definition, default value is "uidef:xml_init"
     * @return void
     */
    STDMETHOD_(void, AddResProvider)(THIS_ IResProvider * pResProvider, LPCTSTR pszUidef DEF_VAL(_T("uidef:xml_init"))) PURE;

    /**
     * @brief Add resource provider
     * @param pResProvider IResProvider* -- resource provider object
     * @param pszUidef LPCSTR -- UI definition, default value is "uidef:xml_init"
     * @return void
     */
    STDMETHOD_(void, AddResProviderA)(THIS_ IResProvider * pResProvider, LPCSTR pszUidef DEF_VAL("uidef:xml_init")) PURE;

    /**
     * @brief Remove resource provider
     * @param pResProvider IResProvider* -- resource provider object
     * @return void
     */
    STDMETHOD_(void, RemoveResProvider)(THIS_ IResProvider * pResProvider) PURE;

    /**
     * @brief Remove all resource providers
     * @return void
     */
    STDMETHOD_(void, RemoveAll)(THIS) PURE;

    /**
     * @brief Get head resource provider
     * @return IResProvider* -- head resource provider object
     */
    STDMETHOD_(IResProvider *, GetHeadResProvider)(CTHIS) SCONST PURE;

    /**
     * @brief Get tail resource provider
     * @return IResProvider* -- tail resource provider object
     */
    STDMETHOD_(IResProvider *, GetTailResProvider)(CTHIS) SCONST PURE;

    /**
     * @brief Set file prefix
     * @param pszFilePrefix LPCTSTR -- file prefix
     * @return void
     */
    STDMETHOD_(void, SetFilePrefix)(THIS_ LPCTSTR pszFilePrefix) PURE;

    /**
     * @brief Retrieves the file prefix for resource paths.
     * @param pszFilePrefix Pointer to a buffer to receive the file prefix.
     * @param cchMax Maximum size of the buffer.
     * @return File prefix string.
     */
    STDMETHOD_(BOOL, GetFilePrefix)(THIS_ LPTSTR pszFilePrefix, int cchMax) SCONST PURE;

    ///////////////////////////////////////////////////////////////////////

    /**
     * @brief Query whether resource exists
     * @param pszType LPCTSTR -- resource type
     * @param pszResName LPCTSTR -- resource name
     * @return BOOL -- TRUE: exists, FALSE: not exists
     */
    STDMETHOD_(BOOL, HasResource)(THIS_ LPCTSTR pszType, LPCTSTR pszResName) PURE;

    /**
     * @brief Load ICON from resource
     * @param pszResName LPCTSTR -- ICON name
     * @param cx int -- ICON width, default value is 0
     * @param cy int -- ICON height, default value is 0
     * @param bFromFile BOOL -- whether to load from file, default value is FALSE
     * @return HICON -- returns ICON handle on success, 0 on failure
     */
    STDMETHOD_(HICON, LoadIcon)(THIS_ LPCTSTR pszResName, int cx DEF_VAL(0), int cy DEF_VAL(0), BOOL bFromFile DEF_VAL(FALSE)) PURE;

    /**
     * @brief Load cursor from resource
     * @param pszResName LPCTSTR -- cursor name
     * @param bFromFile BOOL -- whether to load from file, default value is FALSE
     * @return HCURSOR -- returns cursor handle on success, 0 on failure
     */
    STDMETHOD_(HCURSOR, LoadCursor)(THIS_ LPCTSTR pszResName, BOOL bFromFile DEF_VAL(FALSE)) PURE;

    /**
     * @brief Load HBITMAP from resource
     * @param pszResName LPCTSTR -- BITMAP name
     * @param bFromFile BOOL -- whether to load from file, default value is FALSE
     * @return HBITMAP -- returns BITMAP handle on success, 0 on failure
     */
    STDMETHOD_(HBITMAP, LoadBitmap)(THIS_ LPCTSTR pszResName, BOOL bFromFile DEF_VAL(FALSE)) PURE;

    /**
     * @brief Load an IBitmap object from resource
     * @param pszType LPCTSTR -- image type
     * @param pszResName LPCTSTR -- image name
     * @return IBitmapS* -- returns an IBitmap object on success, 0 on failure
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
    STDMETHOD_(BOOL, GetRawBuffer)(THIS_ LPCTSTR pszType, LPCTSTR pszResName, LPVOID pBuf, size_t size) PURE;
};

SNSEND
#endif /**< __SRESPROVIDERMGR_I__H__ */
