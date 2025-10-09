#ifndef _SAPP_CFG_H_
#define _SAPP_CFG_H_

#include <interface/SAppCfg-i.h>
#include <helper/obj-ref-impl.hpp>
#include <SApp.h>

SNSBEGIN

class SResDesc;
class SLogDesc;
class SMultiLangDesc;

class SOUI_EXP SAppCfg : public TObjRefImpl<IAppCfg> {
public:
    SAppCfg();
    ~SAppCfg(void);

protected:
    SStringT m_appDir;
    Render m_render;
    ImgDecoder m_imgDecoder;
    SLogDesc *m_logDesc;
    SResDesc *m_sysResDesc;
    SResDesc *m_appResDesc;
    SMultiLangDesc *m_langDesc;
    SStringT m_uidefId;
    BOOL m_enableScript;
    
public:
    // C++版本的链式调用方法，返回引用
    SAppCfg & SetRender(Render render);
    SAppCfg & SetImgDecoder(ImgDecoder decoder);
    SAppCfg & SetSysResPeHandle(HMODULE hResModule);
    SAppCfg & SetSysResPeFile(LPCTSTR pszPath);
    SAppCfg & SetSysResFile(LPCTSTR pszPath);
    SAppCfg & SetSysResZip(LPCTSTR pszZipFile, LPCSTR pszPwd=NULL);
    SAppCfg & SetSysRes7z(LPCTSTR psz7zFile, LPCSTR pszPwd=NULL);
    SAppCfg & SetAppResPeHandle(HMODULE hResModule);
    SAppCfg & SetAppResPeFile(LPCTSTR pszPath); 
    SAppCfg & SetAppResFile(LPCTSTR pszPath);
    SAppCfg & SetAppResZip(LPCTSTR pszZipFile, LPCSTR pszPwd = NULL);
    SAppCfg & SetAppRes7z(LPCTSTR psz7zFile, LPCSTR pszPwd = NULL);
    SAppCfg & SetUidefId(const SStringT &strUidefId);
    SAppCfg & SetLog(BOOL bLogEnable, int nLogLevel = LOG_LEVEL_INFO, LPCSTR pszLogName = NULL);
    SAppCfg & SetAppDir(LPCTSTR pszAppDir);
    SAppCfg & EnableScript(BOOL bEnable = TRUE);
    SAppCfg & EnableMultiLang(const SStringT &langResId,BOOL bEnable = TRUE);
    BOOL DoConfig(SApplication *pApp) const;
    
    public:
    // IAppCfg接口实现，返回指针
    STDMETHOD_(IAppCfg*, ISetRender)(Render render);
    STDMETHOD_(IAppCfg*, ISetImgDecoder)(ImgDecoder decoder);
    STDMETHOD_(IAppCfg*, ISetSysResPeHandle)(HMODULE hResModule);
    STDMETHOD_(IAppCfg*, ISetSysResPeFile)(LPCTSTR pszPath);
    STDMETHOD_(IAppCfg*, ISetSysResFile)(LPCTSTR pszPath);
    STDMETHOD_(IAppCfg*, ISetSysResZip)(LPCTSTR pszZipFile, LPCSTR pszPwd);
    STDMETHOD_(IAppCfg*, ISetSysRes7z)(LPCTSTR psz7zFile, LPCSTR pszPwd);
    STDMETHOD_(IAppCfg*, ISetAppResPeHandle)(HMODULE hResModule);
    STDMETHOD_(IAppCfg*, ISetAppResPeFile)(LPCTSTR pszPath);
    STDMETHOD_(IAppCfg*, ISetAppResFile)(LPCTSTR pszPath);
    STDMETHOD_(IAppCfg*, ISetAppResZip)(LPCTSTR pszZipFile, LPCSTR pszPwd);
    STDMETHOD_(IAppCfg*, ISetAppRes7z)(LPCTSTR psz7zFile, LPCSTR pszPwd);
    STDMETHOD_(IAppCfg*, ISetUidefId)(LPCTSTR strUidefId);
    STDMETHOD_(IAppCfg*, ISetLog)(BOOL bLogEnable, int nLogLevel, LPCSTR pszLogName);
    STDMETHOD_(IAppCfg*, ISetAppDir)(LPCTSTR pszAppDir);
    STDMETHOD_(IAppCfg*, IEnableScript)(BOOL bEnable);
    STDMETHOD_(IAppCfg*, IEnableMultiLang)(LPCTSTR langResId, BOOL bEnable);
    STDMETHOD_(BOOL, IDoConfig)(IApplication *pApp) SCONST;
};

SNSEND

#endif//_SAPP_CFG_H_