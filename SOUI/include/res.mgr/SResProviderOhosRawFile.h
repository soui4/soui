// ============================================================================
// SResProviderOhosRawFile - 基于 OHOS NativeResourceManager rawfile API 的
// SOUI IResProvider。
//
// 优点：
//   ★ 直接从 HAP 的 rawfile 中读取 uires 资源包（prefix/{uires.idx,xml,image,values,...}）
//   ★ 无需先把 rawfile 拷贝到 filesDir，既省磁盘又省启动时间
//
// 用法（与 SResProviderFiles 的 Init 接口对齐，但类型不同）：
//
//   #include <rawfile/raw_file_manager.h>
//   NativeResourceManager* mgr =
//       OH_ResourceManager_InitNativeResourceManager(env, jsResMgr);
//
//   SResProviderOhosRawFile* p = new SResProviderOhosRawFile();
//   // wParam: NativeResourceManager*
//   // lParam: rawfile 下的前缀路径，如 "uires" 或 "soui_sys_res"
//   p->Init((WPARAM)mgr, (LPARAM)_T("uires"));
//
//   GETRESPROVIDER->AddResProvider(p, _T("uidef:xml_init")); // 或 LoadSystemNamedResource
//
// 约定：
//   - prefix/uires.idx 必须存在，格式与标准 SOUI uires.idx 一致
//   - uires.idx 中的 path="uidef\\init.xml" 会被标准化为 path="uidef/init.xml"，
//     再拼接 prefix 得到 rawfile 相对路径 "uires/uidef/init.xml"
//   - m_resMgr 由平台层创建（OH_ResourceManager_InitNativeResourceManager），
//     本类不拥有、也不释放
// ============================================================================
#ifdef __OHOS__
#ifndef _SRESPROVIDER_OHOS_RAWFILE_H_
#define _SRESPROVIDER_OHOS_RAWFILE_H_

#include <res.mgr/SResProvider.h>        // SResProvider.h pulls IResProvider + helper/SResID.h
#include <rawfile/raw_file_manager.h>

SNSBEGIN

class SOUI_EXP SResProviderOhosRawFile : public TObjRefImpl<IResProvider> {
  public:
    SResProviderOhosRawFile();
    ~SResProviderOhosRawFile() override = default;

  public:
    /**
     * @param wParam (NativeResourceManager*) OHOS 原生资源管理器指针
     * @param lParam (LPCTSTR) rawfile 下的前缀目录名，如 _T("uires") / _T("soui_sys_res")
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
     * 根据 type+name 查询 uires.idx 映射的完整 rawfile 相对路径，
     * 例如 type=LAYOUT name=dlg_main → "uires/xml/dlg_main.xml"
     *
     * 当 strType == nullptr 时，把 pszResName 作为相对路径直接和 prefix 拼接（用于直接读文件路径）
     *
     * @return 空字符串表示找不到该资源
     */
    SStringT GetAssetPath(LPCTSTR strType, LPCTSTR pszResName) const;

    /**
     * 递归枚举指定目录下的所有文件
     * @param dirPath 目录路径（rawfile 相对路径）
     * @param funEnumCB 回调函数
     * @param lp 用户自定义参数
     */
    void _EnumFile(const SStringT &dirPath, EnumFileCallback funEnumCB, LPARAM lp);

    /**
     * 打开 RawFile 并返回其指针；调用方负责最终 OH_ResourceManager_CloseRawFile(file)。
     *
     * @param filePath 使用 / 分隔的完整 rawfile 相对路径（如 "uires/xml/dlg_main.xml"）
     */
    RawFile *OpenRawFile(const SStringT &filePath) const;

    /** 把 uires.idx path 字段标准化：'\\' -> '/'，并去掉开头多余的 '/' */
    static SStringT NormalizeAssetPath(const SStringT &src);

  private:
    NativeResourceManager *m_resMgr;                 // 平台层持有，本类不拥有，也不释放
    SStringT m_prefix;                               // 前缀目录（不含结尾斜杠），如 _T("uires")
    SMap<SResID, SStringT> m_mapFiles;               // SResID(type,name) -> 规范化后的相对 rawfile path
};

SNSEND

#endif // _SRESPROVIDER_OHOS_RAWFILE_H_

#endif//__OHOS__
