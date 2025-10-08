#include <souistd.h>
#include <SouiFactory.h>
#include <helper/SMenu.h>
#include <helper/SMenuEx.h>
#include <helper/STimer.h>
#include <core/SHostDialog.h>
#include <valueAnimator/SValueAnimator.h>
#include <SAppCfg.h>

SNSBEGIN

SouiFactory::SouiFactory(void)
{
}

SouiFactory::~SouiFactory(void)
{
}

IApplication *SouiFactory::CreateApp(THIS_ IRenderFactory *pRenderFac, HMODULE hInst, LPCTSTR pszHostClassName, BOOL bImeApp)
{
    return new SApplication(pRenderFac, hInst, pszHostClassName, SObjectDefaultRegister(), bImeApp);
}

INativeWnd *SouiFactory::CreateNativeWnd(THIS)
{
    return new SNativeWnd();
}

IHostWnd *SouiFactory::CreateHostWnd(THIS_ LPCTSTR pszResID)
{
    return new SHostWnd(pszResID);
}

IHostDialog *SouiFactory::CreateHostDialog(THIS_ LPCTSTR pszResID)
{
    return new SHostDialog(pszResID);
}

IStringA *SouiFactory::CreateStringA(THIS_ LPCSTR pszSrc)
{
    return CreateIStringA(pszSrc);
}

IStringW *SouiFactory::CreateStringW(THIS_ LPCWSTR pszSrc)
{
    return CreateIStringW(pszSrc);
}

IXmlDoc *SouiFactory::CreateXmlDoc(THIS)
{
    return CreateIXmlDoc();
}

IResProvider *SouiFactory::CreateResProvider(THIS_ BUILTIN_RESTYPE resType)
{
    IResProvider *pObj = NULL;
    switch (resType)
    {
#ifdef _WIN32
    case RES_PE:
        pObj = new SResProviderPE;
        break;
#endif //_WIN32
    case RES_FILE:
        pObj = new SResProviderFiles;
        break;
    default:
        break;
    }
    return pObj;
}

IEvtSlot *SouiFactory::CreateFuncSlot(THIS_ FunCallback fun, void *ctx)
{
    return new FreeFunctionSlot(fun, ctx);
}

IMenu *SouiFactory::CreateMenu(THIS_ HMENU hMenu)
{
    return new SMenu(hMenu);
}

IMenuEx *SouiFactory::CreateMenuEx(THIS)
{
    return new SMenuEx();
}

ITimer *SouiFactory::CreateTimer(THIS_ IEvtSlot *pSlot)
{
    return new STimer(pSlot);
}

IAnimatorGroup *SouiFactory::CreateAnimatorGroup(THIS)
{
    return new SAnimatorGroup();
}

IAppCfg * SouiFactory::CreateAppCfg(THIS) {
    return new SAppCfg();
}
SNSEND

EXTERN_C ISouiFactoryPtr CreateSouiFactory()
{
    return new SNS::SouiFactory();
}
