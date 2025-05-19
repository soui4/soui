/**
 * @file SAxContainer.h
 * @brief Header file for ActiveX container implementation.
 *
 * This file contains the definitions for the ActiveX container classes
 * used to host and manage ActiveX controls within the SOUI framework.
 */
#ifndef ATLACTIVEXCONTAINER_HPP
#define ATLACTIVEXCONTAINER_HPP

 #include "SAxUtil.h"
 #include <atl.mini/SComCli.h>
 #include <MsHtmHst.h>
 
 SNSBEGIN
 
 /**
  * @struct IAxHostDelegate
  * @brief Delegate interface for ActiveX host operations.
  *
  * This interface defines methods that the ActiveX host must implement
  * to provide necessary functionality for the ActiveX control.
  */
 struct IAxHostDelegate
 {
	 /**
	  * @brief Get the window handle of the ActiveX host.
	  * @return Window handle of the ActiveX host.
	  */
	 virtual HWND GetAxHostWindow() const = 0;
 
	 /**
	  * @brief Notify the host that the ActiveX control is being activated.
	  * @param pCtrl Pointer to the ActiveX control.
	  */
	 virtual void OnAxActivate(IUnknown* pCtrl) = 0;
 
	 /**
	  * @brief Notify the host that the ActiveX control needs to be invalidated.
	  * @param pRect Rectangle to be invalidated.
	  * @param bErase Boolean indicating if the background should be erased.
	  */
	 virtual void OnAxInvalidate(LPCRECT pRect, BOOL bErase) = 0;
 
	 /**
	  * @brief Notify the host to set or release the mouse capture.
	  * @param fCapture Boolean indicating if the capture should be set.
	  */
	 virtual void OnAxSetCapture(BOOL fCapture) = 0;
 
	 /**
	  * @brief Get a device context for the ActiveX control.
	  * @param pRect Rectangle for which the DC is needed.
	  * @param grfFlags Flags specifying the type of DC.
	  * @param phDC Pointer to receive the device context.
	  * @return HRESULT indicating success or failure.
	  */
	 virtual HRESULT OnAxGetDC(LPCRECT pRect, DWORD grfFlags, HDC* phDC) = 0;
 
	 /**
	  * @brief Release a previously obtained device context.
	  * @param hdc Device context to be released.
	  * @return HRESULT indicating success or failure.
	  */
	 virtual HRESULT OnAxReleaseDC(HDC hdc) = 0;
 };
 
 /**
  * @class ActiveXSite
  * @brief Template class for managing an ActiveX control site.
  *
  * This class implements the necessary interfaces to host and manage an ActiveX
  * control within a container. It handles the lifecycle of the control and
  * provides the required interface implementations.
  *
  * @tparam T The derived class that will use this template.
  */
 template<class T>
 class ActiveXSite : public IOleClientSite,
					 public IOleControlSite,
					 public IOleInPlaceSiteWindowless,
					 public IAdviseSink
 {
	 friend T;
 
 public:
	 /**
	  * @brief Constructor for ActiveXSite.
	  */
	 ActiveXSite()
		 : m_pAxHostDelegate(NULL)
		 , m_bVisible(true)
		 , m_bInplaceActive(false)
		 , m_dwMiscStatus(0)
		 , m_dwOleObjSink(0)
		 , m_dwViewObjectType(0)
		 , m_grfFlags(0)
		 , m_bCaptured(FALSE)
		 , m_bFocused(FALSE)
	 {
		 memset(&m_rcPos, 0, sizeof(m_rcPos));
	 }
 
	 /**
	  * @brief Destructor for ActiveXSite.
	  */
	 virtual ~ActiveXSite()
	 {
		 Clear();
	 }
 
	 /**
	  * @brief Set the delegate for ActiveX host operations.
	  * @param pAxHost Pointer to the delegate.
	  */
	 void SetAxHost(IAxHostDelegate* pAxHost)
	 {
		 m_pAxHostDelegate = pAxHost;
	 }
 
	 /**
	  * @brief Set the external UI handler for the ActiveX control.
	  * @param pUiHandler Pointer to the UI handler.
	  */
	 void SetExternalUIHandler(IDocHostUIHandler* pUiHandler)
	 {
		 m_spDocHostUIHandler = pUiHandler;
	 }
 
	 /**
	  * @brief Get the ActiveX control.
	  * @return Pointer to the ActiveX control.
	  */
	 IUnknown* GetActiveXControl()
	 {
		 return m_spControl;
	 }
 
	 /**
	  * @brief Initialize the ActiveX control.
	  * @param pControl Pointer to the ActiveX control.
	  */
	 void Init(IUnknown* pControl)
	 {
		 m_spControl = pControl;
		 m_spOleObject = pControl;
		 m_spInPlaceObject = pControl;
	 }
 
	 /**
	  * @brief Clear the ActiveX control and release resources.
	  */
	 void Clear()
	 {
		 if (m_spControl != NULL)
		 {
			 if (m_spViewObject != NULL)
			 {
				 m_spViewObject->SetAdvise(DVASPECT_CONTENT, 0, NULL);
				 m_spViewObject.Release();
			 }
			 m_spOleObjectWindowless.Release();
			 if (m_spInPlaceObject != NULL)
			 {
				 if (m_bInplaceActive)
				 {
					 m_spInPlaceObject->InPlaceDeactivate();
					 m_bInplaceActive = false;
				 }
				 m_spInPlaceObject.Release();
			 }
			 if (m_spOleObject != NULL)
			 {
				 m_spOleObject->Unadvise(m_dwOleObjSink);
				 m_spOleObject->SetClientSite(NULL);
				 m_spOleObject->Close(OLECLOSE_NOSAVE);
				 m_spOleObject.Release();
			 }
			 m_spControl = NULL;
		 }
	 }
 
	 /**
	  * @brief Initialize the ActiveX control from a stream.
	  * @param pStream Pointer to the stream containing the control data.
	  * @return HRESULT indicating success or failure.
	  */
	 HRESULT InitControl(IStream* pStream = NULL)
	 {
		 if (m_spControl == 0) return E_NOINTERFACE;
		 HRESULT hr = E_POINTER;
		 SComQIPtr<IPersistStreamInit> spPSI(m_spControl);
		 if (spPSI != NULL)
		 {
			 if (pStream != NULL)
				 hr = spPSI->Load(pStream);
			 else
				 hr = spPSI->InitNew();
		 }
		 else if (pStream)
		 {
			 SComQIPtr<IPersistStream> spPS(m_spOleObject);
			 if (spPS)
				 hr = spPS->Load(pStream);
		 }
		 return hr;
	 }
 
	 /**
	  * @brief Activate the ActiveX control.
	  * @param pStream Pointer to the stream containing the control data.
	  * @return HRESULT indicating success or failure.
	  */
	 HRESULT ActivateAx(IStream* pStream = NULL)
	 {
		 if (m_spOleObject == 0) return E_UNEXPECTED;
		 HRESULT hr;
 
		 // this must be set even before calling IPersistStreamInit::InitNew
		 hr = m_spOleObject->GetMiscStatus(DVASPECT_CONTENT, &m_dwMiscStatus);
		 if (m_dwMiscStatus & OLEMISC_SETCLIENTSITEFIRST) {
			 hr = m_spOleObject->SetClientSite(static_cast<IOleClientSite*>(this));
			 if (FAILED(hr)) return hr;
		 }
 
		 hr = InitControl(pStream);
		 if (FAILED(hr))
		 {
			 if (m_dwMiscStatus & OLEMISC_SETCLIENTSITEFIRST)
				 m_spOleObject->SetClientSite(NULL);
			 return hr;
		 }
 
		 if (0 == (m_dwMiscStatus & OLEMISC_SETCLIENTSITEFIRST)) {
			 hr = m_spOleObject->SetClientSite(static_cast<IOleClientSite*>(this));
			 if (FAILED(hr)) return hr;
		 }
 
		 hr = DoInplaceActivate();
		 return hr;
	 }
 
	 /**
	  * @brief Check if a point is within the ActiveX control.
	  * @param pt Point to check.
	  * @return Boolean indicating if the point is within the control.
	  */
	 bool HitTest(const POINT& pt) const
	 {
		 if (m_spViewObject != NULL && m_dwViewObjectType == 7) {
			 DWORD dwHitResult = HITRESULT_OUTSIDE;
			 m_spViewObject->QueryHitPoint(DVASPECT_CONTENT, &m_rcPos, pt, 0, &dwHitResult);
			 return (dwHitResult == HITRESULT_HIT);
		 }
		 // else
		 return (::PtInRect(&m_rcPos, pt) != FALSE);
	 }
 
	 /**
	  * @brief Check if the control intersects with a clipping region.
	  * @param rcClip Clipping region to check.
	  * @return Boolean indicating if the control intersects with the region.
	  */
	 bool InsideClippingRegion(const RECT& rcClip) const
	 {
		 // true when top/let or bottom/right corners intersect into client area
		 const POINT* pts = (const POINT*)(&m_rcPos);
		 const POINT* ptc = (const POINT*)(&rcClip);
		 return (::PtInRect(&rcClip, pts[0]) || ::PtInRect(&rcClip, pts[1]) ||
				 ::PtInRect(&m_rcPos, ptc[0]) || ::PtInRect(&m_rcPos, ptc[1]));
	 }
 
	 /**
	  * @brief Perform a verb on the ActiveX control.
	  * @param verb Verb to perform.
	  * @return HRESULT indicating success or failure.
	  */
	 HRESULT DoVerb(long verb)
	 {
		 HRESULT hr = E_FAIL;
		 if ((m_dwMiscStatus & (OLEMISC_INVISIBLEATRUNTIME | OLEMISC_NOUIACTIVATE)) != 0)
		 {
			 hr = S_FALSE;
		 }
		 else if (m_spOleObject != NULL)
		 {
			 hr = m_spOleObject->DoVerb(verb, NULL, static_cast<IOleClientSite*>(this), 0, m_pAxHostDelegate->GetAxHostWindow(), &m_rcPos);
			 if (verb == OLEIVERB_INPLACEACTIVATE && SUCCEEDED(hr))
			 {
				 m_bInplaceActive = true;
			 }
			 else {
				 hr = E_UNEXPECTED;
			 }
		 }
		 return hr;
	 }
 
	 /**
	  * @brief Draw the ActiveX control.
	  * @param hDC Device context to draw on.
	  * @param lpClipRect Clipping rectangle.
	  * @return LRESULT indicating success or failure.
	  */
	 LRESULT Draw(HDC hDC, LPCRECT lpClipRect)
	 {
		 LRESULT lResult = S_FALSE;
		 if (lpClipRect && !InsideClippingRegion(*lpClipRect)) {
			 return lResult;
		 }
		 if (m_bVisible && m_spViewObject != NULL) {
			 RECTL rcPos = { m_rcPos.left,m_rcPos.top,m_rcPos.right,m_rcPos.bottom };
			 lResult = m_spViewObject->Draw(DVASPECT_CONTENT, -1, NULL, NULL, NULL, hDC,
											&rcPos, NULL, NULL, 0);
		 }
		 return lResult;
	 }
 
	 /**
	  * @brief Fire an ambient property change notification.
	  * @param dispChanged DISPID of the changed property.
	  * @return HRESULT indicating success or failure.
	  */
	 HRESULT FireAmbientPropertyChange(DISPID dispChanged)
	 {
		 HRESULT hr = S_OK;
		 SComQIPtr<IOleControl, &__uuidof(IOleControl)> spOleControl(m_spControl);
		 if (spOleControl != NULL)
			 hr = spOleControl->OnAmbientPropertyChange(dispChanged);
		 return hr;
	 }
 
	 /**
	  * @brief Handle a window message for the ActiveX control.
	  * @param uMsg Message identifier.
	  * @param wParam WPARAM for the message.
	  * @param lParam LPARAM for the message.
	  * @return LRESULT indicating success or failure.
	  */
	 LRESULT OnWindowMessage(UINT uMsg, WPARAM wParam, LPARAM lParam)
	 {
		 LRESULT lResult = 0;
		 if (m_bInplaceActive && m_spOleObjectWindowless != NULL)
		 {
			 m_spOleObjectWindowless->OnWindowMessage(uMsg, wParam, lParam, &lResult);
		 }
		 return lResult;
	 }
 
	 /**
	  * @brief Query for an interface.
	  * @param iid IID of the interface to query.
	  * @param object Pointer to receive the interface.
	  * @return HRESULT indicating success or failure.
	  */
	 STDMETHOD(QueryInterface2)(REFIID iid, void** object)
	 {
		 HRESULT hr = S_OK;
		 *object = NULL;
		 if (iid == IID_IOleClientSite) {
			 *object = static_cast<IOleClientSite*>(this);
		 }
		 else if (iid == IID_IOleControlSite) {
			 *object = static_cast<IOleControlSite*>(this);
		 }
		 else if (iid == IID_IOleInPlaceSite) {
			 *object = static_cast<IOleInPlaceSite*>(this);
		 }
		 else if (iid == IID_IOleInPlaceSiteEx) {
			 *object = static_cast<IOleInPlaceSiteEx*>(this);
		 }
		 else if (iid == IID_IOleInPlaceSiteWindowless) {
			 *object = static_cast<IOleInPlaceSiteWindowless*>(this);
		 }
		 else if (iid == IID_IAdviseSink) {
			 *object = static_cast<IAdviseSink*>(this);
		 }
		 else if (iid == IID_IDocHostUIHandler && m_spDocHostUIHandler) {
			 *object = m_spDocHostUIHandler;
		 }
		 else {
			 hr = E_NOINTERFACE;
		 }
		 return hr;
	 }
 
	 /**
	  * @brief Save the object.
	  * @return HRESULT indicating success or failure.
	  */
	 STDMETHOD(SaveObject)(void)
	 {
		 ATLTRACENOTIMPL(_T("ActiveXSite::SaveObject\n"));
	 }
 
	 /**
	  * @brief Get the moniker for the object.
	  * @param dwAssign Assignment type.
	  * @param dwWhichMoniker Type of moniker.
	  * @param ppmk Pointer to receive the moniker.
	  * @return HRESULT indicating success or failure.
	  */
	 STDMETHOD(GetMoniker)(DWORD /*dwAssign*/, DWORD dwWhichMoniker, IMoniker** ppmk)
	 {
		 return E_NOTIMPL;
	 }
 
	 /**
	  * @brief Get the container for the object.
	  * @param ppContainer Pointer to receive the container.
	  * @return HRESULT indicating success or failure.
	  */
	 STDMETHOD(GetContainer)(IOleContainer** ppContainer)
	 {
		 HRESULT hr = E_POINTER;
		 if (ppContainer != NULL) {
			 hr = static_cast<T*>(this)->QueryInterface(__uuidof(IOleContainer), reinterpret_cast<void**>(ppContainer));
		 }
		 return hr;
	 }
 
	 /**
	  * @brief Show the object.
	  * @return HRESULT indicating success or failure.
	  */
	 STDMETHOD(ShowObject)(void)
	 {
		 HWND hWnd = m_pAxHostDelegate->GetAxHostWindow();
		 HRESULT hr = E_FAIL;
		 if (::IsWindow(hWnd) && ::IsRectEmpty(&m_rcPos) == FALSE)
		 {
			 m_bVisible = true;
			 if (m_bInplaceActive)
			 {
				 m_pAxHostDelegate->OnAxInvalidate(&m_rcPos, TRUE);
				 hr = S_OK;
			 }
		 }
		 return hr;
	 }
 
	 /**
	  * @brief Notify the container of a show window event.
	  * @param fShow Boolean indicating if the window is being shown.
	  * @return HRESULT indicating success or failure.
	  */
	 STDMETHOD(OnShowWindow)(BOOL fShow)
	 {
		 m_bVisible = (fShow != FALSE);
		 return S_OK;
	 }
 
	 /**
	  * @brief Request a new object layout.
	  * @return HRESULT indicating success or failure.
	  */
	 STDMETHOD(RequestNewObjectLayout)(void)
	 {
		 ATLTRACENOTIMPL(_T("ActiveXSite::RequestNewObjectLayout\n"));
	 }
 
	 /**
	  * @brief Notify the control that its info has changed.
	  * @return HRESULT indicating success or failure.
	  */
	 STDMETHOD(OnControlInfoChanged)(void)
	 {
		 return S_OK;
	 }
 
	 /**
	  * @brief Lock the control in place active state.
	  * @param fLock Boolean indicating if the lock should be set.
	  * @return HRESULT indicating success or failure.
	  */
	 STDMETHOD(LockInPlaceActive)(BOOL /*fLock*/)
	 {
		 ATLTRACENOTIMPL(_T("ActiveXSite::LockInPlaceActive\n"));
	 }
 
	 /**
	  * @brief Get the extended control.
	  * @param ppDisp Pointer to receive the extended control.
	  * @return HRESULT indicating success or failure.
	  */
	 STDMETHOD(GetExtendedControl)(IDispatch**)
	 {
		 ATLTRACENOTIMPL(_T("ActiveXSite::GetExtendedControl\n"));
	 }
 
	 /**
	  * @brief Transform coordinates between container and control.
	  * @param pPtlHimetric Source coordinates in HIMETRIC.
	  * @param pPtfContainer Destination coordinates in container.
	  * @param dwFlags Flags specifying the transformation.
	  * @return HRESULT indicating success or failure.
	  */
	 STDMETHOD(TransformCoords)(POINTL* /*pPtlHimetric*/, POINTF* /*pPtfContainer*/, DWORD /*dwFlags*/)
	 {
		 ATLTRACENOTIMPL(_T("ActiveXSite::TransformCoords\n"));
	 }
 
	 /**
	  * @brief Translate an accelerator key.
	  * @param pMsg Message structure.
	  * @param grfModifiers Modifiers for the key.
	  * @return HRESULT indicating success or failure.
	  */
	 STDMETHOD(TranslateAccelerator)(MSG* /*pMsg*/, DWORD /*grfModifiers*/)
	 {
		 ATLTRACENOTIMPL(_T("ActiveXSite::TranslateAccelerator\n"));
	 }
 
	 /**
	  * @brief Notify the control of focus change.
	  * @param fGotFocus Boolean indicating if the control got focus.
	  * @return HRESULT indicating success or failure.
	  */
	 STDMETHOD(OnFocus)(BOOL fGotFocus)
	 {
		 // ignore this, controls should use SetFocus(flag) instead
		 return S_OK;
	 }
 
	 /**
	  * @brief Show the property frame for the control.
	  * @return HRESULT indicating success or failure.
	  */
	 STDMETHOD(ShowPropertyFrame)(void)
	 {
		 ATLTRACENOTIMPL(_T("ActiveXSite::ShowPropertyFrame\n"));
	 }
 /**
  * @brief Check if the control supports windowless activation.
  * @return S_OK if windowless activation is supported, otherwise an error code.
  */
 STDMETHOD(CanWindowlessActivate)(void)
 {
	 // support Windowless activation
	 return S_OK;
 }
 
 /**
  * @brief Check if the control currently has the mouse capture.
  * @return S_OK if the control has the capture, S_FALSE otherwise.
  */
 STDMETHOD(GetCapture)(void)
 {
	 HRESULT hr = m_bCaptured ? S_OK : S_FALSE;
	 return hr;
 }
 
 /**
  * @brief Set or release the mouse capture for the control.
  * @param fCapture Boolean indicating if the capture should be set.
  * @return S_OK if the operation is successful.
  */
 STDMETHOD(SetCapture)(BOOL fCapture)
 {
	 if (fCapture != m_bCaptured)
	 {
		 m_bCaptured = fCapture;
		 m_pAxHostDelegate->OnAxSetCapture(fCapture);
	 }
	 return S_OK;
 }
 
 /**
  * @brief Check if the control currently has focus.
  * @return S_OK if the control has focus, S_FALSE otherwise.
  */
 STDMETHOD(GetFocus)(void)
 {
	 return m_bFocused ? S_OK : S_FALSE;
 }
 
 /**
  * @brief Set or release focus for the control.
  * @param fFocus Boolean indicating if the focus should be set.
  * @return S_OK if the operation is successful.
  */
 STDMETHOD(SetFocus)(BOOL fFocus)
 {
	 if (m_bFocused != fFocus)
	 {
		 m_bFocused = fFocus;
		 InvalidateRect(NULL, true);
	 }
 
	 return S_OK;
 }
 
 /**
  * @brief Get a device context for the control.
  * @param pRect Rectangle for which the DC is needed.
  * @param grfFlags Flags specifying the type of DC.
  * @param phDC Pointer to receive the device context.
  * @return HRESULT indicating success or failure.
  */
 STDMETHOD(GetDC)(LPCRECT pRect, DWORD grfFlags, HDC* phDC)
 {
	 return m_pAxHostDelegate->OnAxGetDC(pRect, grfFlags, phDC);
 }
 
 /**
  * @brief Release a previously obtained device context.
  * @param hDC Device context to be released.
  * @return HRESULT indicating success or failure.
  */
 STDMETHOD(ReleaseDC)(HDC hDC)
 {
	 return m_pAxHostDelegate->OnAxReleaseDC(hDC);
 }
 
 /**
  * @brief Invalidate a rectangle of the control.
  * @param pRect Rectangle to be invalidated.
  * @param fErase Boolean indicating if the background should be erased.
  * @return S_OK if the operation is successful.
  */
 STDMETHOD(InvalidateRect)(LPCRECT pRect, BOOL fErase)
 {
	 // Blindly honor the specified region
	 // well behaving control should pass a valid intersect rect or NULL
	 if (pRect == NULL) pRect = &m_rcPos;
	 m_pAxHostDelegate->OnAxInvalidate(pRect, fErase);
	 return S_OK;
 }
 
 /**
  * @brief Invalidate a region of the control.
  * @param hRGN Region to be invalidated.
  * @param fErase Boolean indicating if the background should be erased.
  * @return S_OK if the operation is successful.
  */
 STDMETHOD(InvalidateRgn)(HRGN /*hRGN*/, BOOL fErase)
 {
	 return InvalidateRect(NULL, fErase);
 }
 
 /**
  * @brief Scroll a rectangle of the control.
  * @param dx Horizontal scroll amount.
  * @param dy Vertical scroll amount.
  * @param pRectScroll Rectangle to be scrolled.
  * @param pRectClip Clipping rectangle.
  * @return HRESULT indicating success or failure.
  */
 STDMETHOD(ScrollRect)(INT /*dx*/, INT /*dy*/, LPCRECT /*pRectScroll*/, LPCRECT /*pRectClip*/)
 {
	 ATLTRACENOTIMPL(_T("ActiveXSite::ScrollRect\n"));
 }
 
 /**
  * @brief Adjust a rectangle for the control.
  * @param prc Rectangle to be adjusted.
  * @return HRESULT indicating success or failure.
  */
 STDMETHOD(AdjustRect)(LPRECT /*prc*/)
 {
	 ATLTRACENOTIMPL(_T("ActiveXSite::AdjustRect\n"));
 }
 
 /**
  * @brief Handle a default window message for the control.
  * @param msg Message identifier.
  * @param wParam WPARAM for the message.
  * @param lParam LPARAM for the message.
  * @param plResult Pointer to receive the result of the message processing.
  * @return S_OK if the message is processed successfully.
  */
 STDMETHOD(OnDefWindowMessage)(UINT msg, WPARAM wParam, LPARAM lParam, LRESULT* plResult)
 {
	 *plResult = DefWindowProc(m_pAxHostDelegate->GetAxHostWindow(), msg, wParam, lParam);
	 return S_OK;
 }
 
 // IOleInPlaceSiteEx
 /**
  * @brief Activate the control in-place with extended options.
  * @param pfNoRedraw Pointer to a boolean indicating if redrawing should be suppressed.
  * @param dwFlags Flags specifying activation options.
  * @return S_OK if the control is activated successfully, otherwise an error code.
  */
 STDMETHOD(OnInPlaceActivateEx)(BOOL* pfNoRedraw, DWORD dwFlags)
 {
	 HRESULT hr = S_FALSE;
	 OleLockRunning(m_spOleObject, TRUE, FALSE);
	 if (pfNoRedraw) {
		 *pfNoRedraw = FALSE;
	 }
 
	 if ((dwFlags & ACTIVATE_WINDOWLESS) != 0) {
		 m_spOleObjectWindowless = m_spControl;
 
		 if (m_spOleObjectWindowless != NULL) {
			 m_bInplaceActive = true;
			 hr = S_OK;
		 }
	 }
	 else {
		 // object is not windowless, ensure we won't dispatch message to this control
		 m_bInplaceActive = false;
	 }
	 return hr;
 }
 
 /**
  * @brief Deactivate the control in-place with extended options.
  * @param fNoRedraw Boolean indicating if redrawing should be suppressed.
  * @return S_OK if the control is deactivated successfully.
  */
 STDMETHOD(OnInPlaceDeactivateEx)(BOOL /*fNoRedraw*/)
 {
	 m_bInplaceActive = false;
	 return S_OK;
 }
 
 /**
  * @brief Request UI activation for the control.
  * @return S_OK if the request is successful.
  */
 STDMETHOD(RequestUIActivate)(void)
 {
	 return S_OK;
 }
 
 // IOleInPlaceSite
 /**
  * @brief Check if the control can be activated in-place.
  * @return S_OK if in-place activation is possible, otherwise an error code.
  */
 STDMETHOD(CanInPlaceActivate)(void)
 {
	 return S_OK;
 }
 
 /**
  * @brief Activate the control in-place.
  * @return S_OK if the control is activated successfully, otherwise an error code.
  */
 STDMETHOD(OnInPlaceActivate)(void)
 {
	 return S_OK;
 }
 
 /**
  * @brief Activate the control's user interface.
  * @return S_OK if the UI is activated successfully, otherwise an error code.
  */
 STDMETHOD(OnUIActivate)(void)
 {
	 return S_OK;
 }
 
 /**
  * @brief Get the window context for the control.
  * @param ppFrame Pointer to receive the in-place frame.
  * @param ppDoc Pointer to receive the in-place document.
  * @param lprcPosRect Pointer to receive the position rectangle.
  * @param lprcClipRect Pointer to receive the clipping rectangle.
  * @param lpFrameInfo Pointer to receive the frame information.
  * @return S_OK if the window context is retrieved successfully.
  */
 STDMETHOD(GetWindowContext)(IOleInPlaceFrame** ppFrame,
							IOleInPlaceUIWindow** ppDoc,
							LPRECT lprcPosRect,
							LPRECT lprcClipRect,
							LPOLEINPLACEFRAMEINFO lpFrameInfo)
 {
	 if (ppFrame) {
		 *ppFrame = NULL;
	 }
	 if (ppDoc) {
		 *ppDoc = NULL;
	 }
	 if (lprcPosRect) {
		 memcpy(lprcPosRect, &m_rcPos, sizeof(m_rcPos));
	 }
	 if (lprcClipRect) {
		 memcpy(lprcClipRect, &m_rcPos, sizeof(m_rcPos));
	 }
	 if (lpFrameInfo) {
		 lpFrameInfo->fMDIApp = FALSE;
		 lpFrameInfo->hwndFrame = m_pAxHostDelegate->GetAxHostWindow();
		 lpFrameInfo->haccel = NULL;
		 lpFrameInfo->cAccelEntries = 0;
	 }
	 return S_OK;
 }
 
 /**
  * @brief Scroll the control.
  * @param scrollExtant Size of the scroll.
  * @return HRESULT indicating success or failure.
  */
 STDMETHOD(Scroll)(SIZE /*scrollExtant*/)
 {
	 ATLTRACENOTIMPL(_T("ActiveXSite::Scroll\n"));
 }
 
 /**
  * @brief Deactivate the control's user interface.
  * @param fUndoable Boolean indicating if the deactivation can be undone.
  * @return S_OK if the UI is deactivated successfully.
  */
 STDMETHOD(OnUIDeactivate)(BOOL /*fUndoable*/)
 {
	 return S_OK;
 }
 
 /**
  * @brief Deactivate the control in-place.
  * @return S_OK if the control is deactivated successfully.
  */
 STDMETHOD(OnInPlaceDeactivate)(void)
 {
	 return OnInPlaceDeactivateEx(TRUE);
 }
 
 /**
  * @brief Discard the undo state of the control.
  * @return HRESULT indicating success or failure.
  */
 STDMETHOD(DiscardUndoState)(void)
 {
	 ATLTRACENOTIMPL(_T("ActiveXSite::DiscardUndoState"));
 }
 
 /**
  * @brief Deactivate and undo the control.
  * @return HRESULT indicating success or failure.
  */
 STDMETHOD(DeactivateAndUndo)(void)
 {
	 ATLTRACENOTIMPL(_T("ActiveXSite::DeactivateAndUndo"));
 }
 
 /**
  * @brief Handle a change in the position rectangle of the control.
  * @param lprcPosRect New position rectangle.
  * @return S_OK if the position rectangle is updated successfully.
  */
 STDMETHOD(OnPosRectChange)(LPCRECT lprcPosRect)
 {
	 HRESULT hr = E_UNEXPECTED;
	 hr = S_OK;
	 if (lprcPosRect && memcmp(&m_rcPos, lprcPosRect, sizeof(m_rcPos)) != 0)
	 {
		 if (m_spInPlaceObject)
		 {
			 memcpy(&m_rcPos, lprcPosRect, sizeof(m_rcPos));
			 hr = m_spInPlaceObject->SetObjectRects(&m_rcPos, &m_rcPos);
		 }
		 if (SUCCEEDED(hr))
		 {
			 hr = SetExtent(m_rcPos.right - m_rcPos.left, m_rcPos.bottom - m_rcPos.top);
		 }
	 }
	 return hr;
 }
 
 // IOleWindow
 /**
  * @brief Get the window handle for the control.
  * @param phwnd Pointer to receive the window handle.
  * @return S_OK if the window handle is retrieved successfully.
  */
 STDMETHOD(GetWindow)(HWND* phwnd)
 {
	 *phwnd = m_pAxHostDelegate->GetAxHostWindow();
	 return S_OK;
 }
 
 /**
  * @brief Handle context-sensitive help for the control.
  * @param fEnterMode Boolean indicating if help should be entered.
  * @return HRESULT indicating success or failure.
  */
 STDMETHOD(ContextSensitiveHelp)(BOOL /*fEnterMode*/)
 {
	 ATLTRACENOTIMPL(_T("ActiveXSite::ContextSensitiveHelp\n"));
 }
 
 // IAdviseSink
 /**
  * @brief Handle a change in data.
  * @param pFormatetc Format of the data.
  * @param pStgmed Storage medium containing the data.
  */
 STDMETHOD_(void, OnDataChange)(FORMATETC* /*pFormatetc*/, STGMEDIUM* /*pStgmed*/)
 {
 }
 
 /**
  * @brief Handle a change in view.
  * @param dwAspect Aspect of the view.
  * @param lindex Index of the view.
  */
 STDMETHOD_(void, OnViewChange)(DWORD /*dwAspect*/, LONG /*lindex*/)
 {
 }
 
 /**
  * @brief Handle a rename of the object.
  * @param pmk Moniker of the object.
  */
 STDMETHOD_(void, OnRename)(IMoniker* /*pmk*/)
 {
 }
 
 /**
  * @brief Handle a save operation.
  */
 STDMETHOD_(void, OnSave)(void)
 {
 }
 
 /**
  * @brief Handle a close operation.
  */
 STDMETHOD_(void, OnClose)(void)
 {
 }
 
 protected:
	 /**
	  * @brief Set the extent (size) of the control.
	  * @param width Width of the control in pixels.
	  * @param height Height of the control in pixels.
	  * @return HRESULT indicating success or failure.
	  */
	 HRESULT SetExtent(int width, int height)
	 {
		 HRESULT hr = E_UNEXPECTED;
		 if (m_spOleObject != NULL)
		 {
			 SIZEL pxsize = { width, height };
			 SIZEL hmsize = { 0 };
			 SPixelToHiMetric(&pxsize, &hmsize);
			 hr = m_spOleObject->SetExtent(DVASPECT_CONTENT, &hmsize);
			 hr = m_spOleObject->GetExtent(DVASPECT_CONTENT, &hmsize);
			 SHiMetricToPixel(&hmsize, &pxsize);
			 m_rcPos.right = m_rcPos.left + pxsize.cx;
			 m_rcPos.bottom = m_rcPos.top + pxsize.cy;
		 }
		 return hr;
	 }
 
	 /**
	  * @brief Perform in-place activation of the control.
	  * @return HRESULT indicating success or failure.
	  */
	 HRESULT DoInplaceActivate()
	 {
		 HRESULT hr;
		 m_pAxHostDelegate->OnAxActivate(m_spControl);
		 m_dwViewObjectType = 0;
		 hr = m_spOleObject->QueryInterface(__uuidof(IViewObjectEx), reinterpret_cast<void**>(&m_spViewObject));
		 if (FAILED(hr))
		 {
			 hr = m_spOleObject->QueryInterface(__uuidof(IViewObject2), reinterpret_cast<void**>(&m_spViewObject));
			 if (SUCCEEDED(hr))
				 m_dwViewObjectType = 3;
		 }
		 else {
			 m_dwViewObjectType = 7;
		 }
 
		 if (FAILED(hr))
		 {
			 hr = m_spOleObject->QueryInterface(__uuidof(IViewObject), reinterpret_cast<void**>(&m_spViewObject));
			 if (SUCCEEDED(hr))
				 m_dwViewObjectType = 1;
		 }
		 SComQIPtr<IAdviseSink> advise_sink(m_spControl);
		 m_spOleObject->Advise(advise_sink, &m_dwOleObjSink);
		 if (m_spViewObject)
			 hr = m_spViewObject->SetAdvise(DVASPECT_CONTENT, 0, advise_sink);
		 m_spOleObject->SetHostNames(OLESTR("SOUIAXWIN"), NULL);
 
		 // Do Inplace activate if possible
		 hr = DoVerb(OLEIVERB_INPLACEACTIVATE);
		 return hr;
	 }
 
 private:
	 bool                            m_bVisible; /**< Flag indicating if the control is visible. */
	 bool                            m_bInplaceActive; /**< Flag indicating if the control is in-place active. */
	 BOOL                            m_bFocused; /**< Flag indicating if the control is focused. */
	 BOOL                            m_bCaptured; /**< Flag indicating if the control has captured the mouse. */
	 DWORD                           m_dwMiscStatus; /**< Miscellaneous status flags. */
	 DWORD                           m_dwViewObjectType; /**< Type of view object. */
	 DWORD                            m_dwOleObjSink; /**< Sink cookie for OLE object advise. */
	 DWORD                            m_grfFlags; /**< Flags for various purposes. */
 
	 RECT                            m_rcPos; /**< Rectangle position of the control. */
	 SComPtr<IUnknown>               m_spControl; /**< Smart pointer to the control object. */
	 SComPtr<IViewObjectEx>          m_spViewObject; /**< Smart pointer to the view object. */
	 SComQIPtr<IOleObject>           m_spOleObject; /**< Smart pointer to the OLE object. */
	 SComQIPtr<IOleInPlaceObject>    m_spInPlaceObject; /**< Smart pointer to the in-place object. */
	 SComQIPtr<IOleInPlaceObjectWindowless> m_spOleObjectWindowless; /**< Smart pointer to the windowless in-place object. */
	 SComPtr<IDocHostUIHandler>        m_spDocHostUIHandler; /**< Smart pointer to the document host UI handler. */
	 IAxHostDelegate* m_pAxHostDelegate; /**< Pointer to the host delegate. */
 };
 
 /**
  * @brief Implementation of an ActiveX container.
  *
  * This class implements the necessary interfaces to host ActiveX controls,
  * including IOleContainer, IBindHost, IServiceProvider, and ActiveXSite.
  */
 class SAxContainer : public IOleContainer,
					  public IBindHost,
					  public IServiceProvider,
					  public MinimumIDispatchImpl,
					  public ActiveXSite<SAxContainer>
 {
 public:
	 /**
	  * @brief Constructor for SAxContainer.
	  */
	 SAxContainer();
 
	 /**
	  * @brief Destructor for SAxContainer.
	  */
	 virtual ~SAxContainer();
 
	 /**
	  * @brief Create an ActiveX control.
	  * @param guid CLSID of the control to create.
	  * @param dwClsCtx Context in which the control is to be run.
	  * @return TRUE if successful, FALSE otherwise.
	  */
	 BOOL CreateControl(REFGUID guid, DWORD dwClsCtx = CLSCTX_INPROC_SERVER);
 
	 ///////////////////////////////////////////////////////////////////////////
	 // IOleContainer
 
	 /**
	  * @brief Enumerate objects in the container.
	  * @param grfFlags Flags indicating what objects to enumerate.
	  * @param ppenum Pointer to the IEnumUnknown interface pointer.
	  * @return HRESULT indicating success or failure.
	  */
	 STDMETHOD(EnumObjects)(DWORD /*grfFlags*/, IEnumUnknown** /*ppenum*/)
	 {
		 ATLTRACENOTIMPL(_T("ActiveXContainerImpl::EnumObjects\n"));
	 }
 
	 /**
	  * @brief Lock or unlock the container.
	  * @param fLock TRUE to lock the container, FALSE to unlock.
	  * @return HRESULT indicating success or failure.
	  */
	 STDMETHOD(LockContainer)(BOOL /*fLock*/)
	 {
		 ATLTRACENOTIMPL(_T("ActiveXContainerImpl::LockContainer\n"));
	 }
 
	 // IOleContainer::IParseDisplayName
 
	 /**
	  * @brief Parse a display name into a moniker.
	  * @param pbc Bind context.
	  * @param pszDisplayName Display name to parse.
	  * @param pchEaten Number of characters parsed.
	  * @param ppmkOut Pointer to the IMoniker interface pointer.
	  * @return HRESULT indicating success or failure.
	  */
	 STDMETHOD(ParseDisplayName)(IBindCtx* /*pbc*/, LPOLESTR /*pszDisplayName*/,
								ULONG* /*pchEaten*/, IMoniker** /*ppmkOut*/)
	 {
		 ATLTRACENOTIMPL(_T("ActiveXContainerImpl::ParseDisplayName\n"));
	 }
 
	 ///////////////////////////////////////////////////////////////////////////
	 // IBindHost
 
	 /**
	  * @brief Create a moniker from a display name.
	  * @param szName Display name.
	  * @param pBC Bind context.
	  * @param ppmk Pointer to the IMoniker interface pointer.
	  * @param dwReserved Reserved.
	  * @return HRESULT indicating success or failure.
	  */
	 STDMETHOD(CreateMoniker)(LPOLESTR szName, IBindCtx* pBC,
							 IMoniker** ppmk, DWORD dwReserved);
 
	 /**
	  * @brief Bind a moniker to a storage object.
	  * @param pMk Moniker.
	  * @param pBC Bind context.
	  * @param pBSC Bind status callback.
	  * @param riid Interface identifier.
	  * @param ppvObj Pointer to the interface pointer.
	  * @return HRESULT indicating success or failure.
	  */
	 STDMETHOD(MonikerBindToStorage)(IMoniker* pMk, IBindCtx* pBC,
									 IBindStatusCallback* pBSC, REFIID riid, void** ppvObj);
 
	 /**
	  * @brief Bind a moniker to an object.
	  * @param pMk Moniker.
	  * @param pBC Bind context.
	  * @param pBSC Bind status callback.
	  * @param riid Interface identifier.
	  * @param ppvObj Pointer to the interface pointer.
	  * @return HRESULT indicating success or failure.
	  */
	 STDMETHOD(MonikerBindToObject)(IMoniker* pMk, IBindCtx* pBC,
									IBindStatusCallback* pBSC, REFIID riid, void** ppvObj);
 
	 ///////////////////////////////////////////////////////////////////////////
	 // IServiceProvider
 
	 /**
	  * @brief Query for a service.
	  * @param guidService Service identifier.
	  * @param riid Interface identifier.
	  * @param ppvObject Pointer to the interface pointer.
	  * @return HRESULT indicating success or failure.
	  */
	 STDMETHOD(QueryService)(REFGUID guidService, REFIID riid, void** ppvObject);
 
	 /**
	  * @brief Query for an interface.
	  * @param iid Interface identifier.
	  * @param object Pointer to the interface pointer.
	  * @return HRESULT indicating success or failure.
	  */
	 STDMETHOD(QueryInterface)(REFIID iid, void** object);
 
	 /**
	  * @brief Increment the reference count.
	  * @return New reference count.
	  */
	 STDMETHOD_(ULONG, AddRef)();
 
	 /**
	  * @brief Decrement the reference count.
	  * @return New reference count.
	  */
	 STDMETHOD_(ULONG, Release)();
 
 private:
	 LONG m_lRefCnt; /**< Reference count for COM object management. */
 };
 
 
 
 SNSEND
 
 #endif //ATLACTIVEXCONTAINER_HPP