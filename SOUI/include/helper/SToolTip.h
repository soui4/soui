#ifndef __STOOLTIP__H__
#define __STOOLTIP__H__

#include <interface/stooltip-i.h>
#include <core/SNativeWnd.h>
#include <helper/obj-ref-impl.hpp>

SNSBEGIN

/**
 * @class STipCtrl
 * @brief Tooltip control class, derived from IToolTip and SNativeWnd
 */
class STipCtrl
    : public IToolTip
    , public IMsgFilter
    , public SNativeWnd {
  public:
    /**
     * @brief Constructor
     */
    STipCtrl(void);

    /**
     * @brief Destructor
     */
    virtual ~STipCtrl(void);

  public: /**< IMsgFilter */
    STDMETHOD_(BOOL, PreTranslateMessage)(THIS_ MSG *pMsg) OVERRIDE;

  public: /**< IToolTip */
    STDMETHOD_(long, AddRef)(THIS) OVERRIDE
    {
        return SNativeWnd::AddRef();
    }
    STDMETHOD_(long, Release)(THIS) OVERRIDE
    {
        return SNativeWnd::Release();
    }
    STDMETHOD_(void, OnFinalRelease)(THIS) OVERRIDE
    {
        SNativeWnd::OnFinalRelease();
    }

    STDMETHOD_(IMsgFilter *, GetMsgFilter)(THIS) OVERRIDE
    {
        return this;
    }

    STDMETHOD_(BOOL, Create)(THIS) OVERRIDE;
    STDMETHOD_(BOOL, Destroy)(THIS) OVERRIDE;

    /**
     * @brief Update tooltip information
     * @param id Pointer to tooltip ID
     * @param rc Target rectangle
     * @param pszTip Tooltip text
     * @param nScale Scale ratio
     */
    STDMETHOD_(void, UpdateTip)(THIS_ const TIPID *id, RECT rc, LPCTSTR pszTip, int nScale) OVERRIDE;

    /**
     * @brief Clear tooltip information
     */
    STDMETHOD_(void, ClearTip)(THIS) OVERRIDE;

    /**
     * @brief Relay event
     * @param pMsg Pointer to message structure
     */
    STDMETHOD_(void, RelayEvent)(THIS_ const MSG *pMsg) OVERRIDE;

    /**
     * @brief Set tooltip information
     * @param rc Target rectangle
     * @param tipAlign Tooltip alignment
     * @param pszTip Tooltip text
     * @param nScale Scale ratio
     */
    STDMETHOD_(void, SetToolTip)(THIS_ LPCRECT rc, UINT tipAlign, LPCTSTR pszTip, int nScale) OVERRIDE;

  protected:
    /**
     * @brief Handle the final message
     * @param hWnd Window handle
     */
    virtual void OnFinalMessage(HWND hWnd);

    /**
     * @brief Update font
     * @param nScale Scale ratio
     */
    void UpdateFont(int nScale);

    /**
     * @brief Handle timer message
     * @param idEvent Timer ID
     */
    void OnTimer(UINT_PTR idEvent);

    /**
     * @brief Handle paint message
     * @param dc Device context handle
     */
    void OnPaint(HDC dc);

    /**
     * @brief Show or hide the tooltip
     * @param bShow Whether to show the tooltip
     */
    void ShowTip(BOOL bShow);

    BEGIN_MSG_MAP_EX(STipCtrl)
        MSG_WM_PAINT(OnPaint)
        MSG_WM_TIMER(OnTimer)
        REFLECT_NOTIFICATIONS_EX()
    END_MSG_MAP()

  protected:
    int m_nDelay;      /**< Show delay time (milliseconds) */
    int m_nShowSpan;   /**< Show duration (milliseconds) */
    SStringT m_strTip; /**< Tooltip text */
    int m_nScale;      /**< Scale */
    CRect m_rcTarget;  /**< Target rectangle */
    HFONT m_font;      /**< Font handle */

    TIPID m_id; /**< Tooltip ID */

    UINT m_tipAlign; /**< Tooltip alignment */
};

SNSEND

#endif /**< __STOOLTIP__H__ */