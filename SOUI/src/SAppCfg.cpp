#include <souistd.h>
#include "SAppCfg.h"
#include "SApp.h"
#include <resprovider-zip/SResProviderZip.h>
#include <resprovider-7zip/SResProvider7Zip.h>
#define COM_MASK scom_mask_scom_all
#include <commgr2.h>
#include <SouiFactory.h>
#include <helper/slog.h>
#define kLogTag "SAppCfg"

SNSBEGIN

static SComMgr2 g_comMgr;

class SResLoader {
  public:
    SResLoader(IRenderFactory *pRenderFactory);
    ~SResLoader(void);
    BOOL LoadResFromFile(LPCTSTR pszResFile);
    BOOL LoadResFromZip(LPCTSTR pszZipFile, LPCSTR pszZipPwd = NULL);
    BOOL LoadResFrom7z(LPCTSTR pszZipFile, LPCSTR pszZipPwd = NULL);
    BOOL LoadResFromRes(HMODULE hResModule);

    SAutoRefPtr<IResProvider> GetResProvider(void) const
    {
        return m_pResProvider;
    }

  public:
    SAutoRefPtr<IResProvider> m_pResProvider;
    SAutoRefPtr<IRenderFactory> m_renderFactory;
};

SResLoader::SResLoader(IRenderFactory *pRenderFactory)
    : m_renderFactory(pRenderFactory)
{
    m_pResProvider = NULL;
}

SResLoader::~SResLoader(void)
{
}

BOOL SResLoader::LoadResFrom7z(LPCTSTR pszZipFile, LPCSTR pszZipPwd)
{
    BOOL bLoaded = g_comMgr.CreateResProvider_7ZIP((IObjRef **)&m_pResProvider);
    SASSERT_FMT(bLoaded, _T("load interface [%s] failed!"), _T("resprovider_7zip"));
    if (!bLoaded)
        return FALSE;
    ZIP7RES_PARAM param;
    Zip7File(&param, m_renderFactory, _T("uires.zip"), pszZipPwd);
    bLoaded = m_pResProvider->Init((WPARAM)&param, 0);
    SASSERT(bLoaded);
    return bLoaded;
}

BOOL SResLoader::LoadResFromZip(LPCTSTR pszZipFile, LPCSTR pszZipPwd)
{
    BOOL bLoaded = g_comMgr.CreateResProvider_ZIP((IObjRef **)&m_pResProvider);
    SASSERT_FMT(bLoaded, _T("load interface [%s] failed!"), _T("resprovider_zip"));
    if (!bLoaded)
        return FALSE;
    ZIPRES_PARAM param;
    ZipFile(&param, m_renderFactory, pszZipFile, pszZipPwd);
    bLoaded = m_pResProvider->Init((WPARAM)&param, 0);
    SASSERT(bLoaded);
    return bLoaded;
}

BOOL SResLoader::LoadResFromFile(LPCTSTR pszResFile)
{
    SouiFactory souiFac;
    m_pResProvider.Attach(souiFac.CreateResProvider(RES_FILE));
    return m_pResProvider->Init((LPARAM)pszResFile, 0);
}

BOOL SResLoader::LoadResFromRes(HMODULE hResModule)
{
    SouiFactory souiFac;
    m_pResProvider.Attach(souiFac.CreateResProvider(RES_PE));
    return m_pResProvider->Init((WPARAM)hResModule, 0);
}

//-------------------------------------------------------------

SAppCfg::SAppCfg()
    : m_render(Render_Skia)
    , m_imgDecoder(ImgDecoder_Stb)
    , m_uidefId(_T("uidef:xml_init"))
    , m_enableScript(FALSE)
{
}

SAppCfg ::~SAppCfg(void)
{
}

BOOL SAppCfg::DoConfig(SApplication *pApp) const
{
    if (m_appResDesc.m_type == ResType_Unknown)
    {
        SLOGW()<<"app resource file not specified";
        return FALSE;
    }
    if (m_sysResDesc.m_type == ResType_Unknown)
    {
        SLOGW()<<"system resource file not specified";
        return FALSE;
    }
    if (!m_appDir.IsEmpty() && GetFileAttributes(m_appDir) != INVALID_FILE_ATTRIBUTES)
    {
        pApp->SetAppDir(m_appDir);
    }
    SAutoRefPtr<IRenderFactory> pRendFac;
    switch (m_render)
    {
    case Render_Gdi:
        g_comMgr.CreateRender_GDI((IObjRef **)&pRendFac);
        break;
    case Render_D2D:
        g_comMgr.CreateRender_D2D((IObjRef **)&pRendFac);
        break;
    case Render_Skia:
    default:
        g_comMgr.CreateRender_Skia((IObjRef **)&pRendFac);
        break;
    }
    if (!pRendFac)
    {
        SLOGW()<<"Create render factory failed! render: "<<m_render;
        return FALSE;
    }    
    SAutoRefPtr<IImgDecoderFactory> pImgDecoderFac;
#if !defined(WIN32) || defined(__MINGW32__)
    static const LPCTSTR  kImgDecoderPath[] =
    {
        _T("libimgdecoder-stb"),
        _T("libimgdecoder-gdip"),
        _T("libimgdecoder-wic"),
    };
#else
    static const LPCTSTR  kImgDecoderPath[] =
    {
        _T("imgdecoder-stb"),
        _T("imgdecoder-wic"),
        _T("imgdecoder-gdip"),
    };
#endif
    if(m_imgDecoder < 0 || m_imgDecoder>= ARRAYSIZE(kImgDecoderPath))
    {
        SLOGW()<<"Invalid img decoder index: "<<m_imgDecoder;
        return FALSE;
    }
     g_comMgr.CreateImgDecoder((IObjRef **)&pImgDecoderFac, kImgDecoderPath[m_imgDecoder]);
    if (!pImgDecoderFac)
    {
        SLOGW()<<"Create img decoder factory failed! img decoder: "<<m_imgDecoder<<" path: "<<kImgDecoderPath[m_imgDecoder];
        return FALSE;
    }

    pRendFac->SetImgDecoderFactory(pImgDecoderFac);
    pApp->SetRenderFactory(pRendFac);

    {
        HMODULE hModResource = 0;
        // load system resource
        SResLoader resLoader(pRendFac);
        BOOL bLoaded = FALSE;
        switch (m_sysResDesc.m_type)
        {
        case ResType_7zFile:
            bLoaded = resLoader.LoadResFrom7z(m_sysResDesc.m_szFile, m_sysResDesc.m_szPwd);
            break;
        case ResType_ZipFile:
            bLoaded = resLoader.LoadResFromZip(m_sysResDesc.m_szFile, m_sysResDesc.m_szPwd);
            break;
        case ResType_ResFile:
            bLoaded = resLoader.LoadResFromFile(m_sysResDesc.m_szFile);
            break;
        case ResType_PeHandle:
            bLoaded = resLoader.LoadResFromRes(m_sysResDesc.m_hResModule);
            break;
        case ResType_PeFile:
            hModResource = LoadLibrary(m_sysResDesc.m_szFile);
            if(hModResource){
                bLoaded = resLoader.LoadResFromRes(hModResource);
            }
            break;
        }
        SASSERT(bLoaded);
        if (!bLoaded)
        {
            SLOGW()<<"Load system resource failed! sys res type: "<<m_sysResDesc.m_type;
            return FALSE;
        }
        pApp->LoadSystemNamedResource(resLoader.GetResProvider());
        if(hModResource){
            FreeLibrary(hModResource);
        }
    }

    {
        // load application resource.
        SResLoader resLoader(pRendFac);
        BOOL bLoaded = FALSE;
        switch (m_appResDesc.m_type)
        {
        case ResType_7zFile:
            bLoaded = resLoader.LoadResFrom7z(m_appResDesc.m_szFile, m_appResDesc.m_szPwd);
            break;
        case ResType_ZipFile:
            bLoaded = resLoader.LoadResFromZip(m_appResDesc.m_szFile, m_appResDesc.m_szPwd);
            break;
        case ResType_ResFile:
            bLoaded = resLoader.LoadResFromFile(m_appResDesc.m_szFile);
            break;
        case ResType_PeHandle:
            bLoaded = resLoader.LoadResFromRes(m_appResDesc.m_hResModule);
            break;
        case ResType_PeFile:
            {
                HMODULE hModule = LoadLibrary(m_appResDesc.m_szFile);
                if(hModule){
                    bLoaded = resLoader.LoadResFromRes(hModule);
                }
            }
            break;
        }
        SASSERT(bLoaded);
        if (!bLoaded)
        {
            SLOGW()<<"Load application resource failed! app res type: "<<m_appResDesc.m_type;
            return FALSE;
        }
        pApp->AddResProvider(resLoader.GetResProvider(), m_uidefId);
    }
    if (m_logDesc.m_bLogEnable)
    {
        SAutoRefPtr<ILogMgr> pLogMgr; // log4z对象
        if (g_comMgr.CreateLog4z((IObjRef **)&pLogMgr) && pLogMgr)
        {
            // uncomment next line to disable log mgr to output debug string.
            if(!m_logDesc.m_strLogName.IsEmpty())
                pLogMgr->setLoggerName(m_logDesc.m_strLogName);
            // uncomment next line to record info level log.
            pLogMgr->setLoggerLevel(m_logDesc.m_nLogLevel);
            pLogMgr->start();
            pApp->SetLogManager(pLogMgr);
        }
    }
    if(m_langDesc.enable){
        SAutoRefPtr<ITranslatorMgr> trans;
        BOOL bLoaded = g_comMgr.CreateTranslator((IObjRef **)&trans);
        if (trans && bLoaded) { //加载语言翻译包
            pApp->SetTranslator(trans);
            SXmlDoc xmlLang;
            if (pApp->LoadXmlDocment(xmlLang, m_langDesc.langResId)) {
                SAutoRefPtr<ITranslator> langCN;
                trans->CreateTranslator(&langCN);
                SXmlNode nodeLang = xmlLang.root().child(L"language");
                langCN->Load(&nodeLang, 1); // 1=LD_XML
                trans->InstallTranslator(langCN);
            }
        }
    }
    if(m_enableScript){
        SAutoRefPtr<IScriptFactory> pScriptLuaFactory;
        BOOL bLoaded = g_comMgr.CreateScrpit_Lua((IObjRef **)&pScriptLuaFactory);
        if(bLoaded && pScriptLuaFactory){
            pApp->SetScriptFactory(pScriptLuaFactory);
        }
    }
    return TRUE;
}

SNSEND
