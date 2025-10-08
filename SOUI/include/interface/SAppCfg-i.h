#ifndef __SAPPCFG_I__H__
#define __SAPPCFG_I__H__

#include <interface/obj-ref-i.h>
#include <interface/sapp-i.h>


SNSBEGIN

typedef enum _Render
{
    Render_Gdi=0,
    Render_Skia,
    Render_D2d,
} Render;

typedef enum _ImgDecoder
{
    ImgDecoder_Stb=0,
    ImgDecoder_Gdip,
    ImgDecoder_WIC,
} ImgDecoder;

typedef enum _ResType
{
    ResType_Unknown = 0,
    ResType_ResFile,   // 资源文件
    ResType_PeHandle,  // 资源模块
    ResType_PeFile,    // 资源模块名称
    ResType_ZipFile,   // zip压缩包
    ResType_7zFile,    // 7z压缩包
} ResType;

#undef INTERFACE
#define INTERFACE IAppCfg
DECLARE_INTERFACE_(IAppCfg, IObjRef)
{
    /**
     * @brief 增加引用计数
     * @return 新引用计数
     */
    STDMETHOD_(long, AddRef)(THIS) PURE;

    /**
     * @brief 减少引用计数
     * @return 新引用计数
     */
    STDMETHOD_(long, Release)(THIS) PURE;

    /**
     * @brief 释放对象
     * @return void
     */
    STDMETHOD_(void, OnFinalRelease)(THIS) PURE;

    //////////////////////////////////////////////////////////////////////////

    /**
     * @brief 设置渲染器类型
     * @param render 渲染器类型
     * @return IAppCfg* 返回this指针以支持链式调用
     */
    STDMETHOD_(IAppCfg*, ISetRender)(THIS_ Render render) PURE;

    /**
     * @brief 设置图像解码器类型
     * @param decoder 图像解码器类型
     * @return IAppCfg* 返回this指针以支持链式调用
     */
    STDMETHOD_(IAppCfg*, ISetImgDecoder)(THIS_ ImgDecoder decoder) PURE;

    /**
     * @brief 设置系统资源为PE模块句柄
     * @param hResModule PE模块句柄
     * @return IAppCfg* 返回this指针以支持链式调用
     */
    STDMETHOD_(IAppCfg*, ISetSysResPeHandle)(THIS_ HMODULE hResModule) PURE;

    /**
     * @brief 设置系统资源为PE文件
     * @param pszPath PE文件路径
     * @return IAppCfg* 返回this指针以支持链式调用
     */
    STDMETHOD_(IAppCfg*, ISetSysResPeFile)(THIS_ LPCTSTR pszPath) PURE;

    /**
     * @brief 设置系统资源为资源文件
     * @param pszPath 资源文件路径
     * @return IAppCfg* 返回this指针以支持链式调用
     */
    STDMETHOD_(IAppCfg*, ISetSysResFile)(THIS_ LPCTSTR pszPath) PURE;

    /**
     * @brief 设置系统资源为ZIP文件
     * @param pszZipFile ZIP文件路径
     * @param pszPwd ZIP文件密码
     * @return IAppCfg* 返回this指针以支持链式调用
     */
    STDMETHOD_(IAppCfg*, ISetSysResZip)(THIS_ LPCTSTR pszZipFile, LPCSTR pszPwd) PURE;

    /**
     * @brief 设置系统资源为7z文件
     * @param psz7zFile 7z文件路径
     * @param pszPwd 7z文件密码
     * @return IAppCfg* 返回this指针以支持链式调用
     */
    STDMETHOD_(IAppCfg*, ISetSysRes7z)(THIS_ LPCTSTR psz7zFile, LPCSTR pszPwd) PURE;

    /**
     * @brief 设置应用资源为PE模块句柄
     * @param hResModule PE模块句柄
     * @return IAppCfg* 返回this指针以支持链式调用
     */
    STDMETHOD_(IAppCfg*, ISetAppResPeHandle)(THIS_ HMODULE hResModule) PURE;

    /**
     * @brief 设置应用资源为PE文件
     * @param pszPath PE文件路径
     * @return IAppCfg* 返回this指针以支持链式调用
     */
    STDMETHOD_(IAppCfg*, ISetAppResPeFile)(THIS_ LPCTSTR pszPath) PURE;

    /**
     * @brief 设置应用资源为资源文件
     * @param pszPath 资源文件路径
     * @return IAppCfg* 返回this指针以支持链式调用
     */
    STDMETHOD_(IAppCfg*, ISetAppResFile)(THIS_ LPCTSTR pszPath) PURE;

    /**
     * @brief 设置应用资源为ZIP文件
     * @param pszZipFile ZIP文件路径
     * @param pszPwd ZIP文件密码
     * @return IAppCfg* 返回this指针以支持链式调用
     */
    STDMETHOD_(IAppCfg*, ISetAppResZip)(THIS_ LPCTSTR pszZipFile, LPCSTR pszPwd) PURE;

    /**
     * @brief 设置应用资源为7z文件
     * @param psz7zFile 7z文件路径
     * @param pszPwd 7z文件密码
     * @return IAppCfg* 返回this指针以支持链式调用
     */
    STDMETHOD_(IAppCfg*, ISetAppRes7z)(THIS_ LPCTSTR psz7zFile, LPCSTR pszPwd) PURE;

    /**
     * @brief 设置UI定义ID
     * @param strUidefId UI定义ID
     * @return IAppCfg* 返回this指针以支持链式调用
     */
    STDMETHOD_(IAppCfg*, ISetUidefId)(THIS_ LPCTSTR strUidefId) PURE;

    /**
     * @brief 设置日志配置
     * @param bLogEnable 是否启用日志
     * @param nLogLevel 日志级别
     * @param pszLogName 日志名称
     * @return IAppCfg* 返回this指针以支持链式调用
     */
    STDMETHOD_(IAppCfg*, ISetLog)(THIS_ BOOL bLogEnable, int nLogLevel, LPCSTR pszLogName) PURE;

    /**
     * @brief 设置应用目录
     * @param pszAppDir 应用目录路径
     * @return IAppCfg* 返回this指针以支持链式调用
     */
    STDMETHOD_(IAppCfg*, ISetAppDir)(THIS_ LPCTSTR pszAppDir) PURE;

    /**
     * @brief 启用脚本支持
     * @param bEnable 是否启用
     * @return IAppCfg* 返回this指针以支持链式调用
     */
    STDMETHOD_(IAppCfg*, IEnableScript)(THIS_ BOOL bEnable) PURE;

    /**
     * @brief 启用多语言支持
     * @param langResId 语言资源ID
     * @param bEnable 是否启用
     * @return IAppCfg* 返回this指针以支持链式调用
     */
    STDMETHOD_(IAppCfg*, IEnableMultiLang)(THIS_ LPCTSTR langResId, BOOL bEnable) PURE;

    /**
     * @brief 执行配置
     * @param pApp SApplication对象指针
     * @return BOOL 配置是否成功
     */
    STDMETHOD_(BOOL, IDoConfig)(CTHIS_ IApplication * pApp) SCONST PURE;
};

SNSEND

#endif // __SAPPCFG_I__H__