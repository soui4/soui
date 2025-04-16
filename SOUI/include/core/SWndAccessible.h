#ifndef __SWNDACCESSIBLE__H__
#define __SWNDACCESSIBLE__H__

#ifdef SOUI_ENABLE_ACC
#include <oleacc.h>
#include <helper/SUnknown.h>
#include "Swnd.h"
#include "interface/sacchelper-i.h"

SNSBEGIN

/**
 * @class SAccessible
 * @brief Accessibility support for DUI windows.
 *
 * This class provides accessibility support for DUI windows by implementing the IAccessible and IAccHelper interfaces.
 * It allows screen readers and other assistive technologies to interact with the windows.
 */
class SOUI_EXP SAccessible
    : public IAccessible
    , public IAccHelper
    , public SUnknown {
protected:
    SWindow *m_pWnd; // Pointer to the associated SWindow.

public:
    /**
     * @brief Constructor.
     * @param pWnd Pointer to the SWindow to provide accessibility support for.
     */
    SAccessible(IWindow *pWnd);

    /**
     * @brief Destructor.
     */
    ~SAccessible();

protected:
    /**
     * @brief Validates the navigation start point.
     * @param pvar Pointer to the VARIANT containing the navigation start point.
     * @return TRUE if the navigation start point is valid, FALSE otherwise.
     */
    BOOL accValidateNavStart(VARIANT *pvar) const;

public:
    // Implement IAccHelper
    /**
     * @brief Sets the owner window for accessibility.
     * @param pOwner Pointer to the owner window.
     */
    STDMETHOD_(void, SetOwner)(THIS_ IWindow *pOwner) OVERRIDE {
        m_pWnd = (SWindow*)pOwner;
    }

    /**
     * @brief Gets the owner window for accessibility.
     * @return Pointer to the owner window.
     */
    STDMETHOD_(IWindow *, GetOwner)(CTHIS) SCONST OVERRIDE {
        return m_pWnd;
    }

    // Implement IAccessible
    /**
     * @brief Retrieves the parent of the accessible object.
     * @param ppdispParent Pointer to receive the parent IDispatch.
     * @return HRESULT indicating success or failure.
     */
    STDMETHODIMP get_accParent(IDispatch **ppdispParent);

    /**
     * @brief Retrieves the number of child objects.
     * @param pcountChildren Pointer to receive the count of child objects.
     * @return HRESULT indicating success or failure.
     */
    STDMETHODIMP get_accChildCount(long *pcountChildren);

    /**
     * @brief Retrieves a child object.
     * @param varChild Child object identifier.
     * @param ppdispChild Pointer to receive the child IDispatch.
     * @return HRESULT indicating success or failure.
     */
    STDMETHODIMP get_accChild(VARIANT varChild, IDispatch **ppdispChild);

    /**
     * @brief Retrieves the name of the specified object.
     * @param varChild Object identifier.
     * @param pszName Pointer to receive the name.
     * @return HRESULT indicating success or failure.
     */
    STDMETHODIMP get_accName(VARIANT varChild, BSTR *pszName);

    /**
     * @brief Retrieves the value of the specified object.
     * @param varChild Object identifier.
     * @param pszValue Pointer to receive the value.
     * @return HRESULT indicating success or failure.
     */
    STDMETHODIMP get_accValue(VARIANT varChild, BSTR *pszValue);

    /**
     * @brief Retrieves the description of the specified object.
     * @param varChild Object identifier.
     * @param pszDescription Pointer to receive the description.
     * @return HRESULT indicating success or failure.
     */
    STDMETHODIMP get_accDescription(VARIANT varChild, BSTR *pszDescription);

    /**
     * @brief Retrieves the role of the specified object.
     * @param varChild Object identifier.
     * @param pvarRole Pointer to receive the role.
     * @return HRESULT indicating success or failure.
     */
    STDMETHODIMP get_accRole(VARIANT varChild, VARIANT *pvarRole);

    /**
     * @brief Retrieves the state of the specified object.
     * @param varChild Object identifier.
     * @param pvarState Pointer to receive the state.
     * @return HRESULT indicating success or failure.
     */
    STDMETHODIMP get_accState(VARIANT varChild, VARIANT *pvarState);

    /**
     * @brief Retrieves the help string for the specified object.
     * @param varChild Object identifier.
     * @param pszHelp Pointer to receive the help string.
     * @return HRESULT indicating success or failure.
     */
    STDMETHODIMP get_accHelp(VARIANT varChild, BSTR *pszHelp);

    /**
     * @brief Retrieves the help topic for the specified object.
     * @param pszHelpFile Pointer to receive the help file path.
     * @param varChild Object identifier.
     * @param pidTopic Pointer to receive the help topic ID.
     * @return HRESULT indicating success or failure.
     */
    STDMETHODIMP get_accHelpTopic(BSTR *pszHelpFile, VARIANT varChild, long *pidTopic);

    /**
     * @brief Retrieves the keyboard shortcut for the specified object.
     * @param varChild Object identifier.
     * @param pszKeyboardShortcut Pointer to receive the keyboard shortcut.
     * @return HRESULT indicating success or failure.
     */
    STDMETHODIMP get_accKeyboardShortcut(VARIANT varChild, BSTR *pszKeyboardShortcut);

    /**
     * @brief Retrieves the currently focused object.
     * @param pvarChild Pointer to receive the focused object identifier.
     * @return HRESULT indicating success or failure.
     */
    STDMETHODIMP get_accFocus(VARIANT *pvarChild);

    /**
     * @brief Retrieves the currently selected objects.
     * @param pvarChildren Pointer to receive the selected object identifiers.
     * @return HRESULT indicating success or failure.
     */
    STDMETHODIMP get_accSelection(VARIANT *pvarChildren);

    /**
     * @brief Retrieves the default action for the specified object.
     * @param varChild Object identifier.
     * @param pszDefaultAction Pointer to receive the default action.
     * @return HRESULT indicating success or failure.
     */
    STDMETHODIMP get_accDefaultAction(VARIANT varChild, BSTR *pszDefaultAction);

    /**
     * @brief Selects the specified object.
     * @param flagsSelect Selection flags.
     * @param varChild Object identifier.
     * @return HRESULT indicating success or failure.
     */
    STDMETHODIMP accSelect(long flagsSelect, VARIANT varChild);

    /**
     * @brief Retrieves the location of the specified object.
     * @param pxLeft Pointer to receive the left coordinate.
     * @param pyTop Pointer to receive the top coordinate.
     * @param pcxWidth Pointer to receive the width.
     * @param pcyHeight Pointer to receive the height.
     * @param varChild Object identifier.
     * @return HRESULT indicating success or failure.
     */
    STDMETHODIMP accLocation(long *pxLeft, long *pyTop, long *pcxWidth, long *pcyHeight, VARIANT varChild);

    /**
     * @brief Navigates to another object in the specified direction.
     * @param navDir Navigation direction.
     * @param varStart Starting object identifier.
     * @param pvarEndUpAt Pointer to receive the object identifier at the end of navigation.
     * @return HRESULT indicating success or failure.
     */
    STDMETHODIMP accNavigate(long navDir, VARIANT varStart, VARIANT *pvarEndUpAt);

    /**
     * @brief Retrieves the object at the specified screen coordinates.
     * @param xLeft X coordinate.
     * @param yTop Y coordinate.
     * @param pvarChild Pointer to receive the object identifier.
     * @return HRESULT indicating success or failure.
     */
    STDMETHODIMP accHitTest(long xLeft, long yTop, VARIANT *pvarChild);

    /**
     * @brief Performs the default action for the specified object.
     * @param varChild Object identifier.
     * @return HRESULT indicating success or failure.
     */
    STDMETHODIMP accDoDefaultAction(VARIANT varChild);

    /**
     * @brief Sets the name of the specified object.
     * @param varChild Object identifier.
     * @param szName New name for the object.
     * @return HRESULT indicating success or failure.
     */
    STDMETHODIMP put_accName(VARIANT varChild, BSTR szName);

    /**
     * @brief Sets the value of the specified object.
     * @param varChild Object identifier.
     * @param szValue New value for the object.
     * @return HRESULT indicating success or failure.
     */
    STDMETHODIMP put_accValue(VARIANT varChild, BSTR szValue);

    // Implement IDispatch
    /**
     * @brief Retrieves the number of type information interfaces that an object provides.
     * @param pctinfo Pointer to receive the count of type information interfaces.
     * @return HRESULT indicating success or failure.
     */
    STDMETHODIMP GetTypeInfoCount(unsigned int FAR *pctinfo);

    /**
     * @brief Retrieves the type information for an object.
     * @param iTInfo Type information index.
     * @param lcid Locale identifier.
     * @param ppTInfo Pointer to receive the type information.
     * @return HRESULT indicating success or failure.
     */
    STDMETHODIMP GetTypeInfo(unsigned int iTInfo, LCID lcid, ITypeInfo FAR *FAR *ppTInfo);

    /**
     * @brief Maps a set of names to a corresponding set of dispatch identifiers.
     * @param riid Interface identifier.
     * @param rgszNames Array of names to map.
     * @param cNames Count of names to map.
     * @param lcid Locale identifier.
     * @param rgDispId Array of dispatch identifiers.
     * @return HRESULT indicating success or failure.
     */
    STDMETHODIMP GetIDsOfNames(REFIID riid, OLECHAR FAR *FAR *rgszNames, unsigned int cNames, LCID lcid, DISPID FAR *rgDispId);

    /**
     * @brief Provides access to properties and methods exposed by an object.
     * @param dispIdMember Dispatch identifier.
     * @param riid Interface identifier.
     * @param lcid Locale identifier.
     * @param wFlags Flags specifying the context of the call.
     * @param pDispParams Pointer to a structure containing an array of arguments.
     * @param pVarResult Pointer to receive the result.
     * @param pExcepInfo Pointer to receive exception information.
     * @param puArgErr Pointer to receive the index of the first argument with an error.
     * @return HRESULT indicating success or failure.
     */
    STDMETHODIMP Invoke(DISPID dispIdMember, REFIID riid, LCID lcid, WORD wFlags, DISPPARAMS FAR *pDispParams, VARIANT FAR *pVarResult, EXCEPINFO FAR *pExcepInfo, unsigned int FAR *puArgErr);

public:
    IUNKNOWN_BEGIN(IAccessible)
    IUNKNOWN_ADD_IID(IAccHelper)
    IUNKNOWN_ADD_IID(IDispatch)
    IUNKNOWN_END()
};

SNSEND

#endif // SOUI_ENABLE_ACC

#endif // __SWNDACCESSIBLE__H__