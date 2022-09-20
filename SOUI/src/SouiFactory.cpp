#include <souistd.h>
#include <SouiFactory.h>
#include <helper/SMenu.h>
#include <helper/SMenuEx.h>

SNSBEGIN

SouiFactory::SouiFactory(void)
{
}

SouiFactory::~SouiFactory(void)
{
}

STDMETHODIMP_(HRESULT)
SouiFactory::CreateApp(THIS_ IApplication **ppRet,
                       IRenderFactory *pRenderFac,
                       HMODULE hInst,
                       LPCTSTR pszHostClassName,
                       BOOL bImeApp)
{
    *ppRet
        = new SApplication(pRenderFac, hInst, pszHostClassName, SObjectDefaultRegister(), bImeApp);
    return S_OK;
}

STDMETHODIMP_(HRESULT) SouiFactory::CreateNativeWnd(THIS_ INativeWnd **ppRet)
{
    *ppRet = new SNativeWnd();
    return S_OK;
}

STDMETHODIMP_(HRESULT) SouiFactory::CreateHostWnd(THIS_ IHostWnd **ppRet, LPCTSTR pszResID)
{
    *ppRet = new SHostWnd(pszResID);
    return S_OK;
}

STDMETHODIMP_(HRESULT) SouiFactory::CreateHostDialog(THIS_ IHostDialog **ppRet, LPCTSTR pszResID)
{
    *ppRet = new SHostDialog(pszResID);
    return S_OK;
}

STDMETHODIMP_(HRESULT) SouiFactory::CreateStringA(THIS_ IStringA **ppRet, LPCSTR pszSrc)
{
    *ppRet = new SStringA(pszSrc);
    return S_OK;
}

STDMETHODIMP_(HRESULT) SouiFactory::CreateStringW(THIS_ IStringW **ppRet, LPCWSTR pszSrc)
{
    *ppRet = new SStringW(pszSrc);
    return S_OK;
}

STDMETHODIMP_(HRESULT) SouiFactory::CreateXmlDoc(THIS_ IXmlDoc **ppRet)
{
    *ppRet = new SXmlDoc();
    return S_OK;
}

HRESULT SouiFactory::CreateResProvider(THIS_ BUILTIN_RESTYPE resType, IObjRef **pObj)
{
    *pObj = NULL;
    switch (resType)
    {
    case RES_PE:
        *pObj = new SResProviderPE;
        break;
    case RES_FILE:
        *pObj = new SResProviderFiles;
        break;
    default:
        break;
    }
    return *pObj != NULL ? S_OK : E_INVALIDARG;
}

HRESULT SouiFactory::CreateFuncSlot(THIS_ FunCallback fun, void *ctx, IEvtSlot **ppSlot)
{
    *ppSlot = new FreeFunctionSlot(fun, ctx);
    return S_OK;
}

HRESULT SouiFactory::CreateMenu(THIS_ IMenu**ppRet, HMENU hMenu)
{
	*ppRet = new SMenu(hMenu);
	return S_OK;
}

HRESULT SouiFactory::CreateMenuEx(THIS_ IMenuEx**ppRet)
{
	*ppRet = new SMenuEx();
	return S_OK;
}

SNSEND

EXTERN_C HRESULT CreateSouiFactory(IObjRef **ppRet)
{
    *ppRet = new SOUI::SouiFactory();
    return S_OK;
}
