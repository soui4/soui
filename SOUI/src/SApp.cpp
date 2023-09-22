#include "souistd.h"
#include "SApp.h"
#include "core/SNativeWnd.h"
#include "core/SWindowMgr.h"

#include "res.mgr/sfontpool.h"
#include "res.mgr/SUiDef.h"
#include "res.mgr/SObjDefAttr.h"

#include "helper/STimerGenerator.h"
#include "helper/SAutoBuf.h"
#include "helper/SToolTip.h"
#include "helper/SAppDir.h"
#include "helper/SwndFinder.h"
#include "helper/SHostMgr.h"
#include "event/SNotifyCenter.h"

#include "control/Smessagebox.h"
#include "updatelayeredwindow/SUpdateLayeredWindow.h"
#include "helper/splitstring.h"

#include "core/SSkin.h"
#include "control/souictrls.h"
#include "layout/SouiLayout.h"
#include "layout/SLinearLayout.h"
#include "layout/SGridLayout.h"
#include "animation/SInterpolatorImpl.h"
#include "core/SWndAccessible.h"

#include "msaa/SAccProxyWindow.h"
#include "msaa/SAccProxyCmnCtrl.h"

#include "animation/SAnimation.h"
#include "animation/SAnimationSet.h"
#include "animation/ScaleAnimation.h"
#include "animation/SRotateAnimation.h"
#include "animation/SAlphaAnimation.h"
#include "animation/STranslateAnimation.h"
#include "valueAnimator/SValueAnimator.h"
#include "core/SHostPresenter.h"

SNSBEGIN

class SNullTranslator : public TObjRefImpl<ITranslatorMgr> {
  public:
    STDMETHOD_(BOOL, IsValid)(CTHIS) SCONST
    {
        return FALSE;
    }

    STDMETHOD_(void, SetLanguage)(THIS_ LPCWSTR strLang)
    {
    }
    STDMETHOD_(void, SetLanguageA)(THIS_ LPCSTR strLang)
    {
    }
    STDMETHOD_(void, GetLanguage)(THIS_ wchar_t szOut[TR_MAX_NAME_LEN]) SCONST
    {
        szOut[0] = 0;
    }
    STDMETHOD_(void, GetLanguageA)(THIS_ IStringA *out) SCONST
    {
    }

    STDMETHOD_(BOOL, CreateTranslator)(THIS_ ITranslator **ppTranslator)
    {
        return FALSE;
    }
    STDMETHOD_(BOOL, InstallTranslator)(THIS_ ITranslator *ppTranslator)
    {
        return FALSE;
    }
    STDMETHOD_(BOOL, UninstallTranslator)(THIS_ REFGUID id)
    {
        return FALSE;
    }
    STDMETHOD_(int, tr)
    (THIS_ const IStringW *strSrc, const IStringW *strCtx, wchar_t *pszOut, int nLen) SCONST
    {
        return 0;
    }
};

class SDefToolTipFactory : public TObjRefImpl<IToolTipFactory> {
  public:
    STDMETHOD_(IToolTip *, CreateToolTip)(THIS_ HWND hHost) OVERRIDE
    {
        STipCtrl *pTipCtrl = new STipCtrl;
        if (!pTipCtrl->Create())
        {
            delete pTipCtrl;
            return NULL;
        }
        return pTipCtrl;
    }

    STDMETHOD_(void, DestroyToolTip)(THIS_ IToolTip *pToolTip) OVERRIDE
    {
        if (pToolTip)
        {
            STipCtrl *pTipCtrl = (STipCtrl *)pToolTip;
            pTipCtrl->DestroyWindow();
        }
    }
};

class SDefMsgLoopFactory : public TObjRefImpl<IMsgLoopFactory> {
  public:
    STDMETHOD_(HRESULT, CreateMsgLoop)(THIS_ IMessageLoop **ppRet, IMessageLoop *pParentLoop) OVERRIDE
    {
        *ppRet = new SMessageLoop(pParentLoop);
        return S_OK;
    }
};

void SObjectDefaultRegister::RegisterWindows(SObjectFactoryMgr *objFactory) const
{
    objFactory->TplRegisterFactory<SWindow>();
    objFactory->TplRegisterFactory<SPanel>();
    objFactory->TplRegisterFactory<SStatic>();
    objFactory->TplRegisterFactory<SButton>();
    objFactory->TplRegisterFactory<SImageWnd>();
    objFactory->TplRegisterFactory<SProgress>();
    objFactory->TplRegisterFactory<SImageButton>();
    objFactory->TplRegisterFactory<SLine>();
    objFactory->TplRegisterFactory<SCheckBox>();
    objFactory->TplRegisterFactory<SIconWnd>();
    objFactory->TplRegisterFactory<SRadioBox>();
    objFactory->TplRegisterFactory<SRadioGroup>();
    objFactory->TplRegisterFactory<SLink>();
    objFactory->TplRegisterFactory<SGroup>();
    objFactory->TplRegisterFactory<SAnimateImgWnd>();
    objFactory->TplRegisterFactory<SScrollView>();
    objFactory->TplRegisterFactory<SRealWnd>();
    objFactory->TplRegisterFactory<SToggle>();
    objFactory->TplRegisterFactory<SCaption>();
    objFactory->TplRegisterFactory<STabCtrl>();
    objFactory->TplRegisterFactory<STabPage>();
    objFactory->TplRegisterFactory<SActiveX>();
    // objFactory->TplRegisterFactory<SFlashCtrl>();
    objFactory->TplRegisterFactory<SSplitPane>();
    objFactory->TplRegisterFactory<SSplitWnd>();
    objFactory->TplRegisterFactory<SSplitWnd_Col>();
    objFactory->TplRegisterFactory<SSplitWnd_Row>();
    objFactory->TplRegisterFactory<SSliderBar>();
    objFactory->TplRegisterFactory<STreeCtrl>();
    objFactory->TplRegisterFactory<SScrollBar>();
    objFactory->TplRegisterFactory<SHeaderCtrl>();
    objFactory->TplRegisterFactory<SListCtrl>();
    objFactory->TplRegisterFactory<SListBox>();
    objFactory->TplRegisterFactory<SRichEdit>();
    objFactory->TplRegisterFactory<SEdit>();
    objFactory->TplRegisterFactory<SHotKeyCtrl>();
    objFactory->TplRegisterFactory<SComboEdit>();
    objFactory->TplRegisterFactory<SComboBox>();
    objFactory->TplRegisterFactory<SSpinButtonCtrl>();
    objFactory->TplRegisterFactory<SListView>();
    objFactory->TplRegisterFactory<SComboView>();
    objFactory->TplRegisterFactory<SMCListView>();
    objFactory->TplRegisterFactory<STileView>();
    objFactory->TplRegisterFactory<STreeView>();
    objFactory->TplRegisterFactory<SMenuBar>();
    objFactory->TplRegisterFactory<SCalendar>();
    objFactory->TplRegisterFactory<SDateTimePicker>();
    objFactory->TplRegisterFactory<SFrame>();
    objFactory->TplRegisterFactory<SStackView>();
    objFactory->TplRegisterFactory<SStackPage>();
}

void SObjectDefaultRegister::RegisterSkins(SObjectFactoryMgr *objFactory) const
{
    objFactory->TplRegisterFactory<SSkinImgList>();
    objFactory->TplRegisterFactory<SSkinImgCenter>();
    objFactory->TplRegisterFactory<SSkinImgFrame>();
    objFactory->TplRegisterFactory<SSkinImgFrame2>();
    objFactory->TplRegisterFactory<SSkinButton>();
    objFactory->TplRegisterFactory<SSkinGradation>();
    objFactory->TplRegisterFactory<SSkinGradation2>();
    objFactory->TplRegisterFactory<SSkinScrollbar>();
    objFactory->TplRegisterFactory<SSkinColorRect>();
    objFactory->TplRegisterFactory<SSkinShape>();
    objFactory->TplRegisterFactory<SSKinGroup>();
}

void SObjectDefaultRegister::RegisterLayouts(SObjectFactoryMgr *objFactory) const
{
    objFactory->TplRegisterFactory<SouiLayout>();
    objFactory->TplRegisterFactory<SLinearLayout>();
    objFactory->TplRegisterFactory<SHBox>();
    objFactory->TplRegisterFactory<SVBox>();
    objFactory->TplRegisterFactory<SGridLayout>();
}

void SObjectDefaultRegister::RegisterInterpolator(SObjectFactoryMgr *objFactory) const
{
    objFactory->TplRegisterFactory<SLinearInterpolator>();
    objFactory->TplRegisterFactory<SAccelerateInterpolator>();
    objFactory->TplRegisterFactory<SDecelerateInterpolator>();
    objFactory->TplRegisterFactory<SAccelerateDecelerateInterpolator>();
    objFactory->TplRegisterFactory<SAnticipateInterpolator>();
    objFactory->TplRegisterFactory<SAnticipateOvershootInterpolator>();
    objFactory->TplRegisterFactory<SBounceInterpolator>();
    objFactory->TplRegisterFactory<SCycleInterpolator>();
    objFactory->TplRegisterFactory<SOvershootInterpolator>();
}

void SObjectDefaultRegister::RegisterAnimation(SObjectFactoryMgr *objFactory) const
{
    objFactory->TplRegisterFactory<SAnimation>();
    objFactory->TplRegisterFactory<SAnimationSet>();
    objFactory->TplRegisterFactory<SAlphaAnimation>();
    objFactory->TplRegisterFactory<SScaleAnimation>();
    objFactory->TplRegisterFactory<SRotateAnimation>();
    objFactory->TplRegisterFactory<STranslateAnimation>();
}
void SObjectDefaultRegister::RegisterValueAnimator(SObjectFactoryMgr *objFactory) const
{
    objFactory->TplRegisterFactory<SIntAnimator>();
    objFactory->TplRegisterFactory<SFloatAnimator>();
    objFactory->TplRegisterFactory<SColorAnimator>();
    objFactory->TplRegisterFactory<SPointAnimator>();
    objFactory->TplRegisterFactory<SSizeAnimator>();
    objFactory->TplRegisterFactory<SRectAnimator>();
}
//////////////////////////////////////////////////////////////////////////
// SApplication

template <>
SApplication *SSingleton<SApplication>::ms_Singleton = 0;

SApplication::SApplication(IRenderFactory *pRendFactory, HINSTANCE hInst, LPCTSTR pszHostClassName, const ISystemObjectRegister &sysObjRegister, BOOL bImeApp)
    : m_hInst(hInst)
    , m_RenderFactory(pRendFactory)
    , m_hMainWnd(NULL)
    , m_cbCreateObj(NULL)
    , m_cbCreateTaskLoop(NULL)
{
    SWndSurface::Init();
    memset(m_pSingletons, 0, sizeof(m_pSingletons));
    _CreateSingletons(hInst, pszHostClassName, bImeApp);

    m_translator.Attach(new SNullTranslator);
    m_tooltipFactory.Attach(new SDefToolTipFactory);
    m_msgLoopFactory.Attach(new SDefMsgLoopFactory);

    SAppDir appDir(hInst);
    m_strAppDir = appDir.AppDir();

    SAutoRefPtr<IMessageLoop> pMsgLoop;
    GetMsgLoopFactory()->CreateMsgLoop(&pMsgLoop);
    AddMsgLoop(pMsgLoop);
    sysObjRegister.RegisterLayouts(this);
    sysObjRegister.RegisterSkins(this);
    sysObjRegister.RegisterWindows(this);
    sysObjRegister.RegisterInterpolator(this);
    sysObjRegister.RegisterAnimation(this);
    sysObjRegister.RegisterValueAnimator(this);
}

SApplication::~SApplication(void)
{
    SResProviderMgr::RemoveAll();
    _DestroySingletons();
}

void SApplication::_CreateSingletons(HINSTANCE hInst, LPCTSTR pszHostClassName, BOOL bImeApp)
{
    m_pSingletons[SUiDef::GetType()] = new SUiDef(m_RenderFactory);
    m_pSingletons[SWindowMgr::GetType()] = new SWindowMgr();
    m_pSingletons[STimerGenerator::GetType()] = new STimerGenerator();
    m_pSingletons[SWindowFinder::GetType()] = new SWindowFinder();
    m_pSingletons[STextServiceHelper::GetType()] = new STextServiceHelper();
    m_pSingletons[SRicheditMenuDef::GetType()] = new SRicheditMenuDef();
    m_pSingletons[SNativeWndHelper::GetType()] = new SNativeWndHelper(hInst, pszHostClassName, bImeApp);

    m_pSingletons[SHostMgr::GetType()] = new SHostMgr();
}

#define DELETE_SINGLETON(x)                  \
    delete (x *)m_pSingletons[x::GetType()]; \
    m_pSingletons[x::GetType()] = NULL;

void SApplication::_DestroySingletons()
{
    if (m_pSingletons[SNotifyCenter::GetType()])
        DELETE_SINGLETON(SNotifyCenter);

    DELETE_SINGLETON(SHostMgr);
    DELETE_SINGLETON(SNativeWndHelper);
    DELETE_SINGLETON(SRicheditMenuDef);
    DELETE_SINGLETON(STextServiceHelper);
    DELETE_SINGLETON(SWindowFinder);
    DELETE_SINGLETON(SUiDef);
    DELETE_SINGLETON(STimerGenerator);
    DELETE_SINGLETON(SWindowMgr);
}

IAccProxy *SApplication::CreateAccProxy(SWindow *pWnd) const
{
#ifdef SOUI_ENABLE_ACC
    if (pWnd->IsClass(SProgress::GetClassName()))
    {
        return new SAccProxyProgress(pWnd);
    }
    else if (pWnd->IsClass(SSliderBar::GetClassName()))
    {
        return new SAccProxySlideBar(pWnd);
    }
    else if (pWnd->IsClass(SButton::GetClassName()) || pWnd->IsClass(SImageButton::GetClassName()))
    {
        return new SAccProxyButton(pWnd);
    }
    else if (pWnd->IsClass(SComboBox::GetClassName()))
    {
        return new SAccProxyCombobox(pWnd);
    }
    else if (pWnd->IsClass(SRichEdit::GetClassName()))
    {
        return new SAccProxyEdit(pWnd);
    }
    else if (pWnd->IsClass(SCheckBox::GetClassName()))
    {
        return new SAccProxyCheckButton(pWnd);
    }
    else if (pWnd->IsClass(SRadioBox::GetClassName()))
    {
        return new SAccProxyRadioButton(pWnd);
    }

    return new SAccProxyWindow(pWnd);
#else
    return NULL;
#endif // SOUI_ENABLE_ACC
}

IAccessible *SApplication::CreateAccessible(SWindow *pWnd) const
{
#ifdef SOUI_ENABLE_ACC
    return new SAccessible(pWnd);
#else
    return NULL;
#endif // SOUI_ENABLE_ACC
}

void *SApplication::GetInnerSingleton(SingletonType nType)
{
    if (nType < 0 || nType >= SINGLETON_COUNT)
        return NULL;
    return m_pSingletons[nType];
}

BOOL SApplication::_LoadXmlDocment(LPCTSTR pszXmlName, LPCTSTR pszType, SXmlDoc &xmlDoc, IResProvider *pResProvider /* = NULL*/)
{
    SAutoBuf xmlBuf;
    if (!LoadRawBuffer(pszType, pszXmlName, pResProvider, xmlBuf))
        return FALSE;
    bool bLoad = xmlDoc.load_buffer(xmlBuf, xmlBuf.size(), xml_parse_default, enc_auto);
    if (!bLoad)
    {
        XmlParseResult result;
        xmlDoc.GetParseResult(&result);
        SASSERT_FMTW(bLoad, L"parse xml error! xmlName=%s,desc=%s,offset=%d", pszXmlName, S_CA2W(SXmlDoc::GetErrDesc(result.status)).c_str(), result.offset);
    }
    return bLoad;
}

IXmlDoc *SApplication::LoadXmlDocment(LPCTSTR strResId)
{
    SXmlDoc *xmlDoc = new SXmlDoc;
    BOOL bRet = LoadXmlDocment(*xmlDoc, strResId);
    if (bRet)
    {
        return xmlDoc;
    }
    else
    {
        delete xmlDoc;
        return NULL;
    }
}

BOOL SApplication::LoadXmlDocment(SXmlDoc &xmlDoc, const SStringT &strResId, IResProvider *pResProvider /*=NULL*/)
{
    SStringTList strLst;
    if (2 == ParseResID(strResId, strLst))
        return _LoadXmlDocment(strLst[1], strLst[0], xmlDoc, pResProvider);
    else
        return _LoadXmlDocment(strResId, NULL, xmlDoc, pResProvider);
}

IAnimation *SApplication::LoadAnimation(LPCTSTR strResId)
{
    SXmlDoc xml;
    if (!LoadXmlDocment(xml, strResId))
        return NULL;
    IAnimation *pRet = CreateAnimationByName(xml.root().first_child().name());
    if (!pRet)
        return NULL;
    SXmlNode xmlNode = xml.root().first_child();
    pRet->InitFromXml(&xmlNode);
    return pRet;
}

IValueAnimator *SApplication::LoadValueAnimator(LPCTSTR strResId)
{
    SXmlDoc xml;
    if (!LoadXmlDocment(xml, strResId))
        return NULL;
    IValueAnimator *pRet = CreateValueAnimatorByName(xml.root().first_child().name());
    if (!pRet)
        return NULL;
    SXmlNode xmlNode = xml.root().first_child();
    pRet->InitFromXml(&xmlNode);
    return pRet;
}

ITranslator *SApplication::LoadTranslator(THIS_ LPCTSTR strResId)
{
    if (!GetTranslator())
        return NULL;
    SXmlDoc xml;
    if (!LoadXmlDocment(xml, strResId))
        return NULL;
    ITranslator *pRet = NULL;
    if (!GetTranslator()->CreateTranslator(&pRet))
        return NULL;
    SXmlNode xmlNode = xml.root().child(L"language");
    pRet->Load(&xmlNode, 1); // LD_XML==1
    return pRet;
}

BOOL SApplication::InstallTranslator(THIS_ ITranslator *trModule)
{
    ITranslatorMgr *pTransMgr = GetTranslator();
    if (!pTransMgr)
        return FALSE;
    if (!pTransMgr->InstallTranslator(trModule))
        return FALSE;

    SStringW strFontInfo;
    trModule->getFontInfo(&strFontInfo);
    if (!strFontInfo.IsEmpty())
    {
        GETUIDEF->SetDefFontInfo(strFontInfo);
    }
    SHostMgr::getSingletonPtr()->DispatchMessage(true, UM_SETLANGUAGE);
    return TRUE;
}

BOOL SApplication::UnnstallTranslator(THIS_ REFGUID langId)
{
    ITranslatorMgr *pTransMgr = GetTranslator();
    if (!pTransMgr)
        return FALSE;
    return pTransMgr->UninstallTranslator(langId);
}

UINT SApplication::LoadSystemNamedResource(IResProvider *pResProvider)
{
    UINT uRet = 0;
    AddResProvider(pResProvider, NULL);
    // load system skins
    {
        SXmlDoc xmlDoc;
        if (_LoadXmlDocment(_T("SYS_XML_SKIN"), _T("XML"), xmlDoc, pResProvider))
        {
            ISkinPool *p = GETUIDEF->GetBuiltinSkinPool();
            p->LoadSkins(&xmlDoc.root().child(L"skin"));
        }
        else
        {
            uRet |= 0x01;
        }
    }
    // load edit context menu
    {
        SXmlDoc xmlDoc;
        if (_LoadXmlDocment(_T("SYS_XML_EDITMENU"), _T("XML"), xmlDoc, pResProvider))
        {
            SRicheditMenuDef::getSingleton().SetMenuXml(xmlDoc.root().child(L"editmenu"));
        }
        else
        {
            uRet |= 0x02;
        }
    }
    // load messagebox template
    {
        SXmlDoc xmlDoc;
        if (!_LoadXmlDocment(_T("SYS_XML_MSGBOX"), _T("XML"), xmlDoc, pResProvider) || !SetMsgTemplate(xmlDoc.root().child(L"SOUI")))
        {
            uRet |= 0x04;
        }
    }
    RemoveResProvider(pResProvider);
    return uRet;
}

int SApplication::Run(HWND hMainWnd)
{
    m_hMainWnd = hMainWnd;
    SAutoRefPtr<IMessageLoop> pMsgLoop = GetMsgLoop(GetCurrentThreadId());
    SASSERT(pMsgLoop);
    int nRet = pMsgLoop->Run();
    if (::IsWindow(m_hMainWnd))
    {
        DestroyWindow(m_hMainWnd);
    }
    return nRet;
}

void SApplication::Quit(int nCode)
{
    PostQuitMessage(nCode);
}

HMODULE SApplication::GetModule() const
{
    return m_hInst;
}

void SApplication::SetTranslator(ITranslatorMgr *pTrans)
{
    m_translator = pTrans;
}

ITranslatorMgr *SApplication::GetTranslator()
{
    return m_translator;
}

void SApplication::SetScriptFactory(IScriptFactory *pScriptFactory)
{
    m_pScriptFactory = pScriptFactory;
}

HRESULT SApplication::CreateScriptModule(IScriptModule **ppScriptModule)
{
    if (!m_pScriptFactory)
        return E_FAIL;
    return m_pScriptFactory->CreateScriptModule(ppScriptModule);
}

IRenderFactory *SApplication::GetRenderFactory()
{
    return m_RenderFactory;
}

BOOL SApplication::SetRenderFactory(THIS_ IRenderFactory *renderFac)
{
    if (m_RenderFactory || !renderFac)
        return FALSE;
    m_RenderFactory = renderFac;
    GETUIDEF->SetRenderFactory(renderFac);
    return TRUE;
}

void SApplication::SetRealWndHandler(IRealWndHandler *pRealHandler)
{
    m_pRealWndHandler = pRealHandler;
}

IRealWndHandler *SApplication::GetRealWndHander()
{
    return m_pRealWndHandler;
}

IToolTipFactory *SApplication::GetToolTipFactory()
{
    return m_tooltipFactory;
}

void SApplication::SetToolTipFactory(IToolTipFactory *pToolTipFac)
{
    m_tooltipFactory = pToolTipFac;
}

HWND SApplication::GetMainWnd()
{
    return m_hMainWnd;
}

BOOL SApplication::SetMsgLoopFactory(IMsgLoopFactory *pMsgLoopFac)
{
    m_msgLoopFactory = pMsgLoopFac;
    RemoveMsgLoop();
    SAutoRefPtr<IMessageLoop> pMsgLoop;
    m_msgLoopFactory->CreateMsgLoop(&pMsgLoop);
    AddMsgLoop(pMsgLoop);
    return TRUE;
}

IMsgLoopFactory *SApplication::GetMsgLoopFactory()
{
    return m_msgLoopFactory;
}

void SApplication::InitXmlNamedID(const SNamedID::NAMEDVALUE *pNamedValue, int nCount, BOOL bSorted)
{
    m_namedID.Init2(pNamedValue, nCount, bSorted);
}

void SApplication::InitXmlNamedID(const LPCWSTR *pNames, const int *nIds, int nCount)
{
    m_namedID.Init3(pNames, nIds, nCount, TRUE);
}

int SApplication::Str2ID(const SStringW &str)
{
    return m_namedID.String2Value(str);
}

SStringW SApplication::tr(const SStringW &strSrc, const SStringW &strCtx) const
{
    int nRet = m_translator->tr(&strSrc, &strCtx, NULL, 0);
    if (nRet == 0)
        return strSrc;
    SStringW strRet;
    wchar_t *pBuf = strRet.GetBufferSetLength(nRet - 1);
    m_translator->tr(&strSrc, &strCtx, pBuf, nRet);
    strRet.ReleaseBuffer();
    return strRet;
}

IWindow *SApplication::CreateWindowByName(LPCWSTR pszWndClass) const
{ //支持使用类似button.ok这样的控件名来创建控件，对于这种格式自动应用button.ok为class属性.
    SStringW strClsName = pszWndClass;
    int nPos = strClsName.ReverseFind(L'.');
    if (nPos != -1)
        strClsName = strClsName.Left(nPos);
    IWindow *pRet = (IWindow *)CreateObject(strClsName, Window);
    if (pRet && nPos != -1)
    {
        pRet->SetAttribute(L"class", pszWndClass, TRUE);
    }
    return pRet;
}

ISkinObj *SApplication::CreateSkinByName(LPCWSTR pszSkinClass) const
{
    return (ISkinObj *)CreateObject(pszSkinClass, Skin);
}

IInterpolator *SApplication::CreateInterpolatorByName(LPCWSTR pszName) const
{
    return (IInterpolator *)CreateObject(pszName, Interpolator);
}

IAnimation *SApplication::CreateAnimationByName(LPCWSTR pszName) const
{
    return (IAnimation *)CreateObject(pszName, Animation);
}

IValueAnimator *SApplication::CreateValueAnimatorByName(LPCWSTR pszName) const
{
    return (IValueAnimator *)CreateObject(pszName, ValueAnimator);
}

void SApplication::SetLogManager(ILogMgr *pLogMgr)
{
    m_logManager = pLogMgr;
}

ILogMgr *SApplication::GetLogManager()
{
    return m_logManager;
}

SStringT SApplication::GetAppDir() const
{
    return m_strAppDir;
}

void SApplication::SetAppDir(const SStringT &strAppDir)
{
    m_strAppDir = strAppDir;
}

IAttrStorageFactory *SApplication::GetAttrStorageFactory()
{
    return m_pAttrStroageFactory;
}

void SApplication::SetAttrStorageFactory(IAttrStorageFactory *pAttrStorageFactory)
{
    m_pAttrStroageFactory = pAttrStorageFactory;
}

BOOL SApplication::AddMsgLoop(IMessageLoop *pMsgLoop, BOOL bReplace)
{
    SAutoLock autoLock(m_cs);
    SASSERT(pMsgLoop != NULL);
    DWORD dwThreadID = ::GetCurrentThreadId();
    if (m_msgLoopMap.Lookup(dwThreadID) != NULL && !bReplace)
        return false; // in map yet

    m_msgLoopMap[dwThreadID] = pMsgLoop;
    return true;
}

BOOL SApplication::RemoveMsgLoop()
{
    SAutoLock autoLock(m_cs);
    MsgLoopMap::CPair *p = m_msgLoopMap.Lookup(::GetCurrentThreadId());
    if (!p)
    {
        return FALSE;
    }
    m_msgLoopMap.RemoveKey(p->m_key);
    return TRUE;
}

IMessageLoop *SApplication::GetMsgLoop(DWORD dwThreadID /*= ::GetCurrentThreadId()*/) const
{
    SAutoLock autoLock(m_cs);
    const MsgLoopMap::CPair *p = m_msgLoopMap.Lookup(dwThreadID);
    if (!p)
        return NULL;
    return p->m_value;
}

IResProviderMgr *SApplication::GetResProviderMgr(THIS)
{
    return this;
}

void SApplication::EnableNotifyCenter(THIS_ BOOL bEnable, int interval)
{
    if (bEnable)
    {
        if (m_pSingletons[SINGLETON_NOTIFYCENTER])
            return;
        m_pSingletons[SINGLETON_NOTIFYCENTER] = new SNotifyCenter(interval);
    }
    else
    {
        if (m_pSingletons[SINGLETON_NOTIFYCENTER])
        {
            DELETE_SINGLETON(SNotifyCenter);
        }
    }
}

IObject *SApplication::CreateObject(LPCWSTR pszName, SObjectType nType) const
{
    if (m_cbCreateObj)
    {
        IObject *pRet = m_cbCreateObj(this, pszName, nType);
        if (pRet)
        {
            return pRet;
        }
    }
    SObjectInfo objInfo = ObjInfo_New(pszName, nType);
    return SObjectFactoryMgr::CreateObject(objInfo);
}

void SApplication::SetCreateObjectCallback(THIS_ FunCreateObject cbCreateObj)
{
    m_cbCreateObj = cbCreateObj;
}

BOOL SApplication::RegisterObjFactory(THIS_ const IObjectFactory *objFac, BOOL bReplace)
{
    return RegisterFactory(objFac, bReplace);
}

BOOL SApplication::UnregisterObjFactory(THIS_ const IObjectFactory *objFac)
{
    return UnregisterFactory(objFac->GetObjectInfo());
}

void SApplication::SetDefaultFontInfo(THIS_ LPCWSTR pszFontInfo)
{
    GETUIDEF->SetDefFontInfo(pszFontInfo);
}

void SApplication::SetCreateTaskLoopCallback(THIS_ FunCrateTaskLoop cbCreateTaskLoop)
{
    m_cbCreateTaskLoop = cbCreateTaskLoop;
}

BOOL SApplication::CreateTaskLoop(THIS_ int nCount, Priority priority, BOOL bAutoStart)
{
    if (!m_cbCreateTaskLoop)
        return FALSE;
    if (!m_lstTaskLoop.IsEmpty())
        return FALSE;
    m_lstTaskLoop.SetCount(nCount);
    for (int i = 0; i < nCount; i++)
    {
        m_lstTaskLoop[i].Attach(m_cbCreateTaskLoop());
        if (bAutoStart && m_lstTaskLoop[i])
        {
            m_lstTaskLoop[i]->start(SStringA().Format("taskloop_%d", i).c_str(), priority);
        }
    }
    return TRUE;
}

ITaskLoop *SApplication::GetTaskLoop(THIS_ int iTaskLoop)
{
    if (iTaskLoop >= 0 && iTaskLoop < (int)m_lstTaskLoop.GetCount())
    {
        return m_lstTaskLoop[iTaskLoop];
    }
    else
    {
        return NULL;
    }
}

SNSEND