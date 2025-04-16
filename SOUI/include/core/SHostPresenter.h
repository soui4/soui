#ifndef _SHOSTPRESENTER_H_
#define _SHOSTPRESENTER_H_

#include <interface/SHostPresenter-i.h>
#include <helper/obj-ref-impl.hpp>

SNSBEGIN

// Forward declaration of SHostWnd class
class SHostWnd;

#ifdef _WIN32

// Structure to hold information for SUpdateLayeredWindowIndirect function
struct S_UPDATELAYEREDWINDOWINFO;

/**
 * @brief Helper class for window surface operations.
 * This class provides static methods for initializing and updating layered windows.
 */
class SWndSurface {
  public:
    /**
     * @brief Initializes the SWndSurface functionality.
     * @return TRUE if initialization is successful, FALSE otherwise.
     */
    static BOOL Init();

    /**
     * @brief Updates a layered window using indirect information.
     * @param hWnd Handle to the layered window.
     * @param pULWInfo Pointer to a S_UPDATELAYEREDWINDOWINFO structure containing update information.
     * @return TRUE if the update is successful, FALSE otherwise.
     */
    static BOOL SUpdateLayeredWindowIndirect(HWND hWnd, const S_UPDATELAYEREDWINDOWINFO *pULWInfo);
};

#endif // _WIN32

/**
 * @brief Presenter class for the host window.
 * This class manages the lifecycle and rendering of the host window.
 */
class SOUI_EXP SHostPresenter : public TObjRefImpl<IHostPresenter> {
  protected:
    // Pointer to the host window
    SHostWnd *m_pHostWnd;

  public:
    /**
     * @brief Constructor for SHostPresenter.
     * @param pHostWnd Pointer to the host window.
     */
    SHostPresenter(SHostWnd *pHostWnd);

    /**
     * @brief Destructor for SHostPresenter.
     */
    ~SHostPresenter(void);

  public:
    /**
     * @brief Called when the host window is created.
     */
    STDMETHOD_(void, OnHostCreate)(THIS) OVERRIDE;

    /**
     * @brief Called when the host window is destroyed.
     */
    STDMETHOD_(void, OnHostDestroy)(THIS) OVERRIDE;

    /**
     * @brief Called when the host window is resized.
     * @param szHost New size of the host window.
     */
    STDMETHOD_(void, OnHostResize)(THIS_ SIZE szHost) OVERRIDE;

    /**
     * @brief Called to present the host window.
     * @param hdc Device context for drawing.
     * @param pMemRT Pointer to the memory render target.
     * @param rcDirty Rectangle specifying the area to update.
     * @param byAlpha Alpha blending value.
     */
    STDMETHOD_(void, OnHostPresent)(THIS_ HDC hdc, IRenderTarget *pMemRT, LPCRECT rcDirty, BYTE byAlpha) OVERRIDE;

  protected:
    /**
     * @brief Updates the layer from the render target.
     * @param pRT Pointer to the render target.
     * @param byAlpha Alpha blending value.
     * @param prcDirty Optional rectangle specifying the area to update.
     */
    void UpdateLayerFromRenderTarget(IRenderTarget *pRT, BYTE byAlpha, LPCRECT prcDirty = NULL);
};

SNSEND

#endif // _SHOSTPRESENTER_H_