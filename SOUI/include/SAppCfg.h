#ifndef _SAPP_CFG_H_
#define _SAPP_CFG_H_

SNSBEGIN

class SApplication;
typedef enum _Render
{
    Render_Gdi,
    Render_Skia,
    Render_D2D
}Render;

typedef enum _ImgDecoder
{
    ImgDecoder_Stb,
    ImgDecoder_Gdip,
    ImgDecoder_WIC,
} ImgDecoder;

typedef enum _ResType
{
    ResType_Unknown = 0,
    ResType_ResFile, // ��Դ�ļ�
    ResType_PeHandle,   // ��Դģ��
    ResType_PeFile,     // ��Դģ������
    ResType_ZipFile, // zipѹ����
    ResType_7zFile,  // 7zѹ����
} ResType;

class SOUI_EXP SResDesc {
  public:
    ResType m_type;
    SStringT m_szFile;    // ��Դ�ļ���
    SStringA m_szPwd;     // ѹ��������
    HMODULE m_hResModule; // ��Դģ����

    SResDesc()
    {
        m_type = ResType_Unknown;
        m_hResModule = NULL;
    }
};

class SOUI_EXP SLogDesc{
public:
    BOOL m_bLogEnable;
    SStringA m_strLogName;  //default set to app_name
    int m_nLogLevel;
    SLogDesc()
    {
        m_bLogEnable = FALSE;
        m_nLogLevel = LOG_LEVEL_INFO;
    }
};

class SOUI_EXP SMultiLangDesc{
public:
    BOOL enable;
    SStringT langResId;
    SMultiLangDesc():enable(FALSE)
    {
    }
};
class SOUI_EXP SAppCfg {
public:
    SAppCfg();
    ~SAppCfg(void);

protected:
    SStringT m_appDir;
    Render m_render;
    ImgDecoder m_imgDecoder;
    SLogDesc m_logDesc;
    SResDesc m_sysResDesc;
    SResDesc m_appResDesc;
    SStringT m_uidefId;

    BOOL m_enableScript;
    SMultiLangDesc m_langDesc;
  public:
    SAppCfg & SetRender(Render render){
        m_render = render; 
        return *this;
    }
    SAppCfg & SetImgDecoder(ImgDecoder decoder){
        m_imgDecoder = decoder; 
        return *this;
    }

    SAppCfg & SetSysResPeHandle(HMODULE hResModule){
        m_sysResDesc.m_type = ResType_PeHandle;
        m_sysResDesc.m_hResModule = hResModule;
        return *this;
    }

    SAppCfg & SetSysResPeFile(LPCTSTR pszPath){
        m_sysResDesc.m_type = ResType_PeFile;
        m_sysResDesc.m_szFile = pszPath;
        return *this;
    }
      SAppCfg &SetSysResFile(LPCTSTR pszPath) {
          m_sysResDesc.m_type = ResType_ResFile;
          m_sysResDesc.m_szFile = pszPath;
          return *this;
      }
    SAppCfg & SetSysResZip(LPCTSTR pszZipFile, LPCSTR pszPwd=NULL){
        m_sysResDesc.m_type = ResType_ZipFile;
        m_sysResDesc.m_szFile = pszZipFile;
        if(pszPwd) m_sysResDesc.m_szPwd = pszPwd;
        return *this;
    }
    SAppCfg & SetSysRes7z(LPCTSTR psz7zFile, LPCSTR pszPwd=NULL){
        m_sysResDesc.m_type = ResType_7zFile;
        m_sysResDesc.m_szFile = psz7zFile;
        if(pszPwd) m_sysResDesc.m_szPwd = pszPwd;
        return *this;
    }
    SAppCfg & SetAppResPeHandle(HMODULE hResModule){
        m_appResDesc.m_type = ResType_PeHandle;
        m_appResDesc.m_hResModule = hResModule;
        return *this;
    }
    SAppCfg & SetAppResPeFile(LPCTSTR pszPath){ 
        m_appResDesc.m_type = ResType_PeFile;
        m_appResDesc.m_szFile = pszPath;
        return *this;
    }
    SAppCfg &SetAppResFile(LPCTSTR pszPath)
    {
        m_appResDesc.m_type = ResType_ResFile;
        m_appResDesc.m_szFile = pszPath;
        return *this;
    }
    SAppCfg &SetAppResZip(LPCTSTR pszZipFile, LPCSTR pszPwd = NULL)
    {
        m_appResDesc.m_type = ResType_ZipFile;
        m_appResDesc.m_szFile = pszZipFile;
        if (pszPwd)
            m_appResDesc.m_szPwd = pszPwd;
        return *this;
    }
    SAppCfg &SetAppRes7z(LPCTSTR psz7zFile, LPCSTR pszPwd = NULL)
    {
        m_appResDesc.m_type = ResType_7zFile;
        m_appResDesc.m_szFile = psz7zFile;
        if (pszPwd)
            m_appResDesc.m_szPwd = pszPwd;
        return *this;
    }

    SAppCfg &SetUidefId(const SStringT &strUidefId){
        m_uidefId = strUidefId;
        return *this;
    }

    SAppCfg &SetLog(BOOL bLogEnable, int nLogLevel = LOG_LEVEL_INFO, LPCSTR pszLogName = NULL){
        m_logDesc.m_bLogEnable = bLogEnable;
        m_logDesc.m_nLogLevel = nLogLevel;
        if(pszLogName) m_logDesc.m_strLogName = pszLogName;
        return *this;
    }

    SAppCfg &SetAppDir(LPCTSTR pszAppDir){
        m_appDir = pszAppDir;
        return *this;
    }

    SAppCfg &EnableScript(BOOL bEnable = TRUE){
        m_enableScript = bEnable;
        return *this;
    }
    SAppCfg &EnableMultiLang(const SStringT &langResId,BOOL bEnable = TRUE){
        m_langDesc.enable = bEnable;
        m_langDesc.langResId = langResId;
        return *this;
    }
  public:
    BOOL DoConfig(SApplication *pApp) const;
};

SNSEND

#endif//_SAPP_CFG_H_