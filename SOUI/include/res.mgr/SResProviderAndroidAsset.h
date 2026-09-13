//============================================================================
/** SResProviderAndroidAsset - SOUI IResProvider based on Android NDK AAssetManager. */
//
/** Advantages: */
/** ★ Read uires resource package directly from APK assets (prefix/{uires.idx,xml,image,values,...}) */
/** ★ No need to copy assets to filesDir first, saving both disk space and startup time */
//
/** Usage (aligned with SResProviderFiles' Init interface, but with different types): */
//
/** #include <android/asset_manager_jni.h>  // for AAssetManager_fromJava */
/** AAssetManager* mgr = AAssetManager_fromJava(env, javaAssetManager); */
//
/** SResProviderAndroidAsset* p = new SResProviderAndroidAsset(); */
/** // wParam: AAssetManager* */
/** // lParam: prefix path under assets, e.g. "uires" or "soui_sys_res" */
/** p->Init((WPARAM)mgr, (LPARAM)_T("uires")); */
//
/** GETRESPROVIDER->AddResProvider(p, _T("uidef:xml_init")); // or LoadSystemNamedResource */
//
/** Conventions: */
/** - prefix/uires.idx must exist, format consistent with standard SOUI uires.idx */
/** - path="uidef\init.xml" in uires.idx will be normalized to path="uidef/init.xml", */
/** then concatenated with prefix to get the assets relative path "uires/uidef/init.xml" */
//============================================================================
#ifdef __ANDROID__
#ifndef _SRESPROVIDER_ANDROID_ASSET_H_
#define _SRESPROVIDER_ANDROID_ASSET_H_

#include <res.mgr/SResProvider.h> /**< SResProvider.h pulls IResProvider + helper/SResID.h */
#include <android/asset_manager.h>

SNSBEGIN

class SOUI_EXP SResProviderAndroidAsset : public TObjRefImpl<IResProvider> {
  public:
    SResProviderAndroidAsset();
    ~SResProviderAndroidAsset() override = default;

  public:
    /**
     * @param wParam (AAssetManager*) Android native AssetManager pointer
     * @param lParam (LPCTSTR) prefix directory name under assets, e.g. _T("uires") / _T("soui_sys_res")
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
     * Query the full assets relative path mapped by uires.idx according to type+name,
     * e.g. type=LAYOUT name=dlg_main → "uires/xml/dlg_main.xml"
     *
     * When strType == nullptr, concatenate pszResName as relative path directly with prefix (used to read file path directly)
     *
     * @return empty string means resource not found
     */
    SStringT GetAssetPath(LPCTSTR strType, LPCTSTR pszResName) const;

    /**
     * Recursively enumerate all files under the specified directory
     * @param dirPath directory path (assets relative path)
     * @param funEnumCB callback function
     * @param lp user-defined parameter
     */
    void _EnumFile(const SStringT &dirPath, EnumFileCallback funEnumCB, LPARAM lp);

    /**
     * Open AAsset and return its pointer; the caller is responsible for eventually calling AAsset_close(asset).
     *
     * @param assetPath full assets relative path separated by / (e.g. "uires/xml/dlg_main.xml")
     * @param mode      AASSET_MODE_BUFFER / AASSET_MODE_RANDOM ...
     */
    AAsset *OpenAsset(const SStringT &assetPath, int mode = AASSET_MODE_BUFFER) const;

    /** Normalize the uires.idx path field: '\' -> '/', and remove the leading redundant '/' */
    static SStringT NormalizeAssetPath(const SStringT &src);

  private:
    AAssetManager *m_assetMgr;         /**< Held by JVM, not owned or released by this class */
    SStringT m_prefix;                 /**< Prefix directory (without trailing slash), e.g. _T("uires") */
    SMap<SResID, SStringT> m_mapFiles; /**< SResID(type,name) -> normalized relative asset path */
};

SNSEND

#endif /**< _SRESPROVIDER_ANDROID_ASSET_H_ */

#endif /**< __ANDROID__ */