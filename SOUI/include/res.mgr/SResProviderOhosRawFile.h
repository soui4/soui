//============================================================================
/** SResProviderOhosRawFile - SOUI IResProvider based on OHOS NativeResourceManager rawfile API */
/** SOUI IResProvider. */
//
/** Advantages: */
/** ★ Read uires resource package directly from HAP's rawfile (prefix/{uires.idx,xml,image,values,...}) */
/** ★ No need to copy rawfile to filesDir first, saving both disk space and startup time */
//
/** Usage (aligned with SResProviderFiles' Init interface, but with different types): */
//
/** #include <rawfile/raw_file_manager.h> */
/** NativeResourceManager* mgr = */
/** OH_ResourceManager_InitNativeResourceManager(env, jsResMgr); */
//
/** SResProviderOhosRawFile* p = new SResProviderOhosRawFile(); */
/** // wParam: NativeResourceManager* */
/** // lParam: prefix path under rawfile, e.g. "uires" or "soui_sys_res" */
/** p->Init((WPARAM)mgr, (LPARAM)_T("uires")); */
//
/** GETRESPROVIDER->AddResProvider(p, _T("uidef:xml_init")); // or LoadSystemNamedResource */
//
/** Conventions: */
/** - prefix/uires.idx must exist, format consistent with standard SOUI uires.idx */
/** - path="uidef\init.xml" in uires.idx will be normalized to path="uidef/init.xml", */
/** then concatenated with prefix to get the rawfile relative path "uires/uidef/init.xml" */
/** - m_resMgr is created by the platform layer (OH_ResourceManager_InitNativeResourceManager), */
/** not owned or released by this class */
//============================================================================
#ifdef __OHOS__
#ifndef _SRESPROVIDER_OHOS_RAWFILE_H_
#define _SRESPROVIDER_OHOS_RAWFILE_H_

#include <res.mgr/SResProvider.h> /**< SResProvider.h pulls IResProvider + helper/SResID.h */
#include <rawfile/raw_file_manager.h>

SNSBEGIN

class SOUI_EXP SResProviderOhosRawFile : public TObjRefImpl<IResProvider> {
  public:
    SResProviderOhosRawFile();
    ~SResProviderOhosRawFile() override = default;

  public:
    /**
     * @param wParam (NativeResourceManager*) OHOS native resource manager pointer
     * @param lParam (LPCTSTR) prefix directory name under rawfile, e.g. _T("uires") / _T("soui_sys_res")
     */
    STDMETHOD_(BOOL, Init)
    (THIS_ WPARAM wParam, LPARAM lParam) OVERRIDE;

    STDMETHOD_(BOOL, HasResource)
    (THIS_ LPCTSTR pszType, LPCTSTR pszResName) OVERRIDE;

    STDMETHOD_(HICON, LoadIcon)
    (THIS_ LPCTSTR pszResName, int cx, int cy) OVERRIDE;

    STDMETHOD_(HBITMAP, LoadBitmap)
    (THIS_ LPCTSTR pszResName) OVERRIDE;

    STDMETHOD_(HCURSOR, LoadCursor)
    (THIS_ LPCTSTR pszResName) OVERRIDE;

    STDMETHOD_(IBitmapS *, LoadImage)
    (THIS_ LPCTSTR pszType, LPCTSTR pszResName) OVERRIDE;

    STDMETHOD_(IImgX *, LoadImgX)
    (THIS_ LPCTSTR pszType, LPCTSTR pszResName) OVERRIDE;

    STDMETHOD_(size_t, GetRawBufferSize)
    (THIS_ LPCTSTR pszType, LPCTSTR pszResName) OVERRIDE;

    STDMETHOD_(BOOL, GetRawBuffer)
    (THIS_ LPCTSTR pszType, LPCTSTR pszResName, LPVOID pBuf, size_t size) OVERRIDE;

    STDMETHOD_(void, EnumResource)
    (THIS_ EnumResCallback funEnumCB, LPARAM lp) OVERRIDE;

    STDMETHOD_(void, EnumFile)
    (THIS_ EnumFileCallback funEnumCB, LPARAM lp) OVERRIDE;

  protected:
    /**
     * Query the full rawfile relative path mapped by uires.idx according to type+name,
     * e.g. type=LAYOUT name=dlg_main → "uires/xml/dlg_main.xml"
     *
     * When strType == nullptr, concatenate pszResName as relative path directly with prefix (used to read file path directly)
     *
     * @return empty string means resource not found
     */
    SStringT GetAssetPath(LPCTSTR strType, LPCTSTR pszResName) const;

    /**
     * Recursively enumerate all files under the specified directory
     * @param dirPath directory path (rawfile relative path)
     * @param funEnumCB callback function
     * @param lp user-defined parameter
     */
    void _EnumFile(const SStringT &dirPath, EnumFileCallback funEnumCB, LPARAM lp);

    /**
     * Open RawFile and return its pointer; the caller is responsible for ultimately calling OH_ResourceManager_CloseRawFile(file).
     *
     * @param filePath Full rawfile relative path separated by / (e.g. "uires/xml/dlg_main.xml")
     */
    RawFile *OpenRawFile(const SStringT &filePath) const;

    /** Normalize the uires.idx path field: '\' -> '/', and remove the leading redundant '/' */
    static SStringT NormalizeAssetPath(const SStringT &src);

  private:
    NativeResourceManager *m_resMgr;   /**< Held by the platform layer; this class does not own or release it */
    SStringT m_prefix;                 /**< Prefix directory (without trailing slash), e.g. _T("uires") */
    SMap<SResID, SStringT> m_mapFiles; /**< SResID(type,name) -> normalized relative rawfile path */
};

SNSEND

#endif /**< _SRESPROVIDER_OHOS_RAWFILE_H_ */

#endif /**< __OHOS__ */
