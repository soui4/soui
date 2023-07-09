#include <souistd.h>
#include <SouiFactory.h>
#include <helper/SMenu.h>
#include <helper/SMenuEx.h>
#include <helper/STimer.h>

SNSBEGIN

SouiFactory::SouiFactory(void)
{
}

SouiFactory::~SouiFactory(void)
{
}

STDMETHODIMP_(IApplication *)
SouiFactory::CreateApp(THIS_ IRenderFactory *pRenderFac, HMODULE hInst, LPCTSTR pszHostClassName, BOOL bImeApp)
{
    return new SApplication(pRenderFac, hInst, pszHostClassName, SObjectDefaultRegister(), bImeApp);
}

STDMETHODIMP_(INativeWnd *) SouiFactory::CreateNativeWnd(THIS)
{
    return new SNativeWnd();
}

STDMETHODIMP_(IHostWnd *) SouiFactory::CreateHostWnd(THIS_ LPCTSTR pszResID)
{
    return new SHostWnd(pszResID);
}

STDMETHODIMP_(IHostDialog *) SouiFactory::CreateHostDialog(THIS_ LPCTSTR pszResID)
{
    return new SHostDialog(pszResID);
}

STDMETHODIMP_(IStringA *) SouiFactory::CreateStringA(THIS_ LPCSTR pszSrc)
{
    return new SStringA(pszSrc);
}

STDMETHODIMP_(IStringW *) SouiFactory::CreateStringW(THIS_ LPCWSTR pszSrc)
{
    return new SStringW(pszSrc);
}

STDMETHODIMP_(IXmlDoc *) SouiFactory::CreateXmlDoc(THIS)
{
    return new SXmlDoc();
}

IResProvider *SouiFactory::CreateResProvider(THIS_ BUILTIN_RESTYPE resType)
{
    IResProvider *pObj = NULL;
    switch (resType)
    {
    case RES_PE:
        pObj = new SResProviderPE;
        break;
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

ITimer * SouiFactory::CreateTimer(THIS_ IEvtSlot *pSlot)
{
    return new STimer(pSlot);
}

IAnimatorGroup* SouiFactory::CreateAnimatorGroup(THIS)
{
	return new SAnimatorGroup();
}

SNSEND

EXTERN_C ISouiFactoryPtr CreateSouiFactory()
{
    return new SOUI::SouiFactory();
}
