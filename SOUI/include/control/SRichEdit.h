#ifndef __SRICHEDIT__H__
#define __SRICHEDIT__H__

#include <richedit.h>
#include <core/SPanel.h>
#include <proxy/SCtrlProxy.h>
#include <interface/STimer-i.h>

#include <imm.h>
#include <textserv.h>

SNSBEGIN

enum
{
    MENU_CUT = 1,
    MENU_COPY,
    MENU_PASTE,
    MENU_DEL,
    MENU_SELALL,
    //---------------
    MENU_USER = 20,
};

/**
 * @class SRichEdit
 * @brief RichEdit Control
 * @details A control that uses a windowless RichEdit to implement an edit control.
 */
class STextHost;
class SOUI_EXP SRichEdit : public TPanelProxy<IRichEdit> {
    friend class STextHost;
    friend class SApplication;

  public:
    DEF_SOBJECT(SPanel, L"richedit")

    /**
     * @brief Constructor
     */
    SRichEdit();

    /**
     * @brief Destructor
     */
    virtual ~SRichEdit()
    {
    }

  private:
    /**
     * @brief Initialize text service
     */
    static void InitTextService();

    /**
     * @brief Uninitialize text service
     */
    static void UninitTextService();

  public: // RichEdit Interface
    /**
     * @brief Save content to an RTF file
     * @param pszFileName File name to save the RTF content
     * @return Number of bytes written
     */
    STDMETHOD_(DWORD, SaveRtf)(THIS_ LPCTSTR pszFileName) OVERRIDE;

    /**
     * @brief Load content from an RTF file
     * @param pszFileName File name to load the RTF content
     * @return Number of bytes read
     */
    STDMETHOD_(DWORD, LoadRtf)(THIS_ LPCTSTR pszFileName) OVERRIDE;

    /**
     * @brief Set the selection range
     * @param nStartChar Start character position
     * @param nEndChar End character position
     * @param bNoScroll Whether to scroll to the selection
     */
    STDMETHOD_(void, SetSel)(THIS_ long nStartChar, long nEndChar, BOOL bNoScroll) OVERRIDE;

    /**
     * @brief Replace the selected text
     * @param pszText Text to replace the selection
     * @param bCanUndo Whether the operation can be undone
     */
    STDMETHOD_(void, ReplaceSel)(THIS_ LPCTSTR pszText, BOOL bCanUndo = TRUE) OVERRIDE;

    /**
     * @brief Get the word wrap setting
     * @return TRUE if word wrap is enabled, FALSE otherwise
     */
    STDMETHOD_(BOOL, GetWordWrap)(THIS) SCONST OVERRIDE;

    /**
     * @brief Set the word wrap setting
     * @param fWordWrap TRUE to enable word wrap, FALSE to disable
     */
    STDMETHOD_(void, SetWordWrap)(THIS_ BOOL fWordWrap) OVERRIDE;

    /**
     * @brief Check if the control is read-only
     * @return TRUE if read-only, FALSE otherwise
     */
    STDMETHOD_(BOOL, GetReadOnly)(THIS) SCONST OVERRIDE;

    /**
     * @brief Set the read-only state
     * @param bReadOnly TRUE to set read-only, FALSE to allow editing
     * @return TRUE if successful, FALSE otherwise
     */
    STDMETHOD_(BOOL, SetReadOnly)(THIS_ BOOL bReadOnly) OVERRIDE;

    /**
     * @brief Get the maximum text length
     * @return Maximum text length
     */
    STDMETHOD_(LONG, GetLimitText)(THIS) SCONST OVERRIDE;

    /**
     * @brief Set the maximum text length
     * @param nLength Maximum text length
     * @return TRUE if successful, FALSE otherwise
     */
    STDMETHOD_(BOOL, SetLimitText)(THIS_ int nLength) OVERRIDE;

    /**
     * @brief Get the default text alignment
     * @return Default text alignment
     */
    STDMETHOD_(WORD, GetDefaultAlign)(THIS) SCONST OVERRIDE;

    /**
     * @brief Set the default text alignment
     * @param wNewAlign New text alignment
     */
    STDMETHOD_(void, SetDefaultAlign)(THIS_ WORD wNewAlign) OVERRIDE;

    /**
     * @brief Get the rich text flag
     * @return TRUE if rich text is enabled, FALSE otherwise
     */
    STDMETHOD_(BOOL, GetRichTextFlag)(THIS) SCONST OVERRIDE;

    /**
     * @brief Set the rich text flag
     * @param fRich TRUE to enable rich text, FALSE to disable
     */
    STDMETHOD_(void, SetRichTextFlag)(THIS_ BOOL fRich) OVERRIDE;

    /**
     * @brief Get the default left indent
     * @return Default left indent
     */
    STDMETHOD_(LONG, GetDefaultLeftIndent)(THIS) SCONST OVERRIDE;

    /**
     * @brief Set the default left indent
     * @param lNewIndent New left indent
     */
    STDMETHOD_(void, SetDefaultLeftIndent)(THIS_ LONG lNewIndent) OVERRIDE;

    /**
     * @brief Set the save selection flag
     * @param fSaveSelection TRUE to save selection, FALSE otherwise
     * @return TRUE if successful, FALSE otherwise
     */
    STDMETHOD_(BOOL, SetSaveSelection)(THIS_ BOOL fSaveSelection) OVERRIDE;

    /**
     * @brief Set the default text color
     * @param cr New text color
     * @return Previous text color
     */
    STDMETHOD_(COLORREF, SetDefaultTextColor)(THIS_ COLORREF cr) OVERRIDE;

    /**
     * @brief Set the window text
     * @param lpszText New window text
     */
    STDMETHOD_(void, SetWindowText)(LPCTSTR lpszText) OVERRIDE;

    /**
     * @brief Get the window text
     * @param pBuf Buffer to store the text
     * @param nBufLen Length of the buffer
     * @param bRawText Whether to get raw text
     * @return Length of the text
     */
    STDMETHOD_(int, GetWindowText)(THIS_ TCHAR *pBuf, int nBufLen, BOOL bRawText) SCONST OVERRIDE;

    /**
     * @brief Process window messages
     * @param uMsg Message identifier
     * @param wParam Additional message-specific information
     * @param lParam Additional message-specific information
     * @param lResult Pointer to the result
     * @return TRUE if the message was processed, FALSE otherwise
     */
    STDMETHOD_(BOOL, SwndProc)(THIS_ UINT uMsg, WPARAM wParam, LPARAM lParam, LRESULT *lResult) OVERRIDE;

    /**
     * @brief Create a caret
     * @param pBmp Bitmap for the caret
     * @param nWid Width of the caret
     * @param nHeight Height of the caret
     * @return TRUE if successful, FALSE otherwise
     */
    STDMETHOD_(BOOL, CreateCaret)(THIS_ HBITMAP pBmp, int nWid, int nHeight) OVERRIDE;

  public:
    /**
     * @brief Get the window text
     * @param bRawText Whether to get raw text
     * @return Window text
     */
    SStringT GetWindowText(BOOL bRawText = FALSE) SCONST OVERRIDE;

    /**
     * @brief Set the selection range
     * @param dwSelection Selection range
     * @param bNoScroll Whether to scroll to the selection
     */
    void SetSel(DWORD dwSelection, BOOL bNoScroll = FALSE);

    /**
     * @brief Get the length of the window text
     * @return Length of the window text
     */
    int GetWindowTextLength() const;

  protected:
    /**
     * @brief Handle creation of the control
     * @param lpCreateStruct Create structure
     * @return Result of the creation
     */
    int OnCreate(LPVOID lpCreateStruct);

    /**
     * @brief Handle destruction of the control
     */
    void OnDestroy();

    /**
     * @brief Handle paint message
     * @param pRT Rendering target handle
     */
    void OnPaint(IRenderTarget *pRT);

    /**
     * @brief Handle set focus message
     * @param wndOld Previous focus window handle
     */
    void OnSetFocus(SWND wndOld);

    /**
     * @brief Handle kill focus message
     * @param wndFocus New focus window handle
     */
    void OnKillFocus(SWND wndFocus);

    /**
     * @brief Handle timer message
     * @param idEvent Timer ID
     */
    void OnTimer(char idEvent);

    /**
     * @brief Get dialog code
     * @return Dialog code
     */
    virtual UINT WINAPI OnGetDlgCode() const;

    /**
     * @brief Handle scroll message
     * @param bVertical TRUE if vertical scroll, FALSE if horizontal
     * @param uCode Scroll code
     * @param nPos Scroll position
     * @return TRUE if successful, FALSE otherwise
     */
    virtual BOOL OnScroll(BOOL bVertical, UINT uCode, int nPos);

    /**
     * @brief Handle set cursor message
     * @param pt Mouse coordinates
     * @return TRUE if successful, FALSE otherwise
     */
    virtual BOOL OnSetCursor(const CPoint &pt);

    /**
     * @brief Handle scale change message
     * @param nScale New scale factor
     */
    virtual void OnScaleChanged(int nScale);

    /**
     * @brief Handle rebuild font message
     */
    virtual void OnRebuildFont();

    /**
     * @brief Check if rich text scaling is enabled
     * @return TRUE if enabled, FALSE otherwise
     */
    BOOL IsRichScale() const;

    /**
     * @brief Handle enable message
     * @param bEnable Enable flag
     * @param nStatus Status code
     */
    void OnEnable(BOOL bEnable, UINT nStatus);

    /**
     * @brief Initialize default character format
     * @param pcf Character format structure
     * @param pFont Font to apply
     * @return Result of the initialization
     */
    HRESULT InitDefaultCharFormat(CHARFORMAT2W *pcf, IFontS *pFont = NULL);

    /**
     * @brief Initialize default paragraph format
     * @param ppf Paragraph format structure
     * @return Result of the initialization
     */
    HRESULT InitDefaultParaFormat(PARAFORMAT2 *ppf);

    /**
     * @brief Handle text notification
     * @param iNotify Notification code
     * @param pv Notification data
     * @return Result of the notification handling
     */
    virtual HRESULT OnTxNotify(DWORD iNotify, LPVOID pv);

    /**
     * @brief Handle left button down message
     * @param nFlags Mouse flags
     * @param point Mouse coordinates
     */
    void OnLButtonDown(UINT nFlags, CPoint point);

    /**
     * @brief Handle left button up message
     * @param nFlags Mouse flags
     * @param point Mouse coordinates
     */
    void OnLButtonUp(UINT nFlags, CPoint point);

    /**
     * @brief Handle right button down message
     * @param nFlags Mouse flags
     * @param point Mouse coordinates
     */
    void OnRButtonDown(UINT nFlags, CPoint point);

    /**
     * @brief Handle mouse move message
     * @param nFlags Mouse flags
     * @param point Mouse coordinates
     */
    void OnMouseMove(UINT nFlags, CPoint point);

    /**
     * @brief Handle mouse wheel message
     * @param nFlags Mouse flags
     * @param zDelta Wheel delta
     * @param pt Mouse coordinates
     * @return TRUE if successful, FALSE otherwise
     */
    BOOL OnMouseWheel(UINT nFlags, short zDelta, CPoint pt);

    /**
     * @brief Handle key down message
     * @param nChar Key code
     * @param nRepCnt Repeat count
     * @param nFlags Key flags
     */
    void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);

    /**
     * @brief Handle button click message
     * @param uMsg Message identifier
     * @param wParam Additional message-specific information
     * @param lParam Additional message-specific information
     * @return Result of the message processing
     */
    LRESULT OnButtonClick(UINT uMsg, WPARAM wParam, LPARAM lParam);

    /**
     * @brief Handle character message
     * @param nChar Character code
     * @param nRepCnt Repeat count
     * @param nFlags Character flags
     */
    void OnChar(UINT nChar, UINT nRepCnt, UINT nFlags);

    /**
     * @brief Handle set font message
     * @param font Font to apply
     * @param bRedraw Whether to redraw the control
     */
    void OnSetFont(IFontS *font, BOOL bRedraw);

    /**
     * @brief Handle set text message
     * @param uMsg Message identifier
     * @param wParam Additional message-specific information
     * @param lParam Additional message-specific information
     * @return Result of the message processing
     */
    LRESULT OnSetText(UINT uMsg, WPARAM wParam, LPARAM lParam);

    /**
     * @brief Handle set character format message
     * @param uMsg Message identifier
     * @param wParam Additional message-specific information
     * @param lParam Additional message-specific information
     * @return Result of the message processing
     */
    LRESULT OnSetCharFormat(UINT uMsg, WPARAM wParam, LPARAM lParam);

    /**
     * @brief Handle set paragraph format message
     * @param uMsg Message identifier
     * @param wParam Additional message-specific information
     * @param lParam Additional message-specific information
     * @return Result of the message processing
     */
    LRESULT OnSetParaFormat(UINT uMsg, WPARAM wParam, LPARAM lParam);

    /**
     * @brief Handle set read-only message
     * @param uMsg Message identifier
     * @param wParam Additional message-specific information
     * @param lParam Additional message-specific information
     * @return Result of the message processing
     */
    LRESULT OnSetReadOnly(UINT uMsg, WPARAM wParam, LPARAM lParam);

    /**
     * @brief Handle set limit text message
     * @param uMsg Message identifier
     * @param wParam Additional message-specific information
     * @param lParam Additional message-specific information
     * @return Result of the message processing
     */
    LRESULT OnSetLimitText(UINT uMsg, WPARAM wParam, LPARAM lParam);

    /**
     * @brief Handle non-client calculate size message
     * @param bCalcValidRects Whether to calculate valid rectangles
     * @param lParam Additional message-specific information
     * @return Result of the message processing
     */
    LRESULT OnNcCalcSize(BOOL bCalcValidRects, LPARAM lParam);

    /**
     * @brief Enable or disable drag-and-drop
     * @param bEnable Enable flag
     */
    void OnEnableDragDrop(BOOL bEnable);

    /**
     * @brief Handle get rectangle message
     * @param uMsg Message identifier
     * @param wp Additional message-specific information
     * @param lp Additional message-specific information
     * @return Result of the message processing
     */
    LRESULT OnGetRect(UINT uMsg, WPARAM wp, LPARAM lp);

    /**
     * @brief Handle set scroll position message
     * @param fnBar Scroll bar identifier
     * @param nPos Scroll position
     * @param fRedraw Whether to redraw the control
     * @return TRUE if successful, FALSE otherwise
     */
    BOOL OnTxSetScrollPos(INT fnBar, INT nPos, BOOL fRedraw);

    /**
     * @brief Handle set timer message
     * @param idTimer Timer ID
     * @param uTimeout Timeout interval
     * @return TRUE if successful, FALSE otherwise
     */
    BOOL OnTxSetTimer(UINT idTimer, UINT uTimeout);

    /**
     * @brief Handle kill timer message
     * @param idTimer Timer ID
     */
    void OnTxKillTimer(UINT idTimer);

    /**
     * @brief Handle timer timeout message
     * @param e Event arguments
     * @return TRUE if successful, FALSE otherwise
     */
    BOOL OnTimeout(IEvtArgs *e);

  protected:
    SOUI_MSG_MAP_BEGIN()
        MSG_WM_CREATE(OnCreate)
        MSG_WM_DESTROY(OnDestroy)
        MSG_WM_PAINT_EX(OnPaint)
        MSG_WM_NCCALCSIZE(OnNcCalcSize)
        MSG_WM_SETFOCUS_EX(OnSetFocus)
        MSG_WM_KILLFOCUS_EX(OnKillFocus)
        MSG_WM_TIMER_EX(OnTimer)
        MSG_WM_LBUTTONDOWN(OnLButtonDown)
        MSG_WM_LBUTTONUP(OnLButtonUp)
        MSG_WM_RBUTTONDOWN(OnRButtonDown)
        MSG_WM_MOUSEMOVE(OnMouseMove)
        MSG_WM_MOUSEWHEEL(OnMouseWheel)
        MSG_WM_KEYDOWN(OnKeyDown)
        MSG_WM_CHAR(OnChar)
        MSG_WM_SETFONT_EX(OnSetFont)
        MESSAGE_HANDLER_EX(WM_LBUTTONDBLCLK, OnButtonClick)
        MESSAGE_HANDLER_EX(WM_SETTEXT, OnSetText)
        MESSAGE_HANDLER_EX(EM_SETPARAFORMAT, OnSetParaFormat)
        MESSAGE_HANDLER_EX(EM_SETCHARFORMAT, OnSetCharFormat)
        MESSAGE_HANDLER_EX(EM_SETREADONLY, OnSetReadOnly)
        MESSAGE_HANDLER_EX(EM_EXLIMITTEXT, OnSetLimitText)
        MESSAGE_HANDLER_EX(EM_GETRECT, OnGetRect)
    SOUI_MSG_MAP_END()

  protected:
#define ATTR_RE_STYLE(attr, style, txtBit, func)        \
    if (0 == strAttribName.CompareNoCase(attr))         \
    {                                                   \
        hRet = func(strValue, style, txtBit, bLoading); \
    }                                                   \
    else

    /**
     * @brief Set text color attribute
     * @param strValue Attribute value
     * @param bLoading Loading flag
     * @return Result of the attribute setting
     */
    HRESULT OnAttrTextColor(const SStringW &strValue, BOOL bLoading);

    /**
     * @brief Set RTF attribute
     * @param strValue Attribute value
     * @param bLoading Loading flag
     * @return Result of the attribute setting
     */
    HRESULT OnAttrRTF(const SStringW &strValue, BOOL bLoading);

    /**
     * @brief Set alignment attribute
     * @param strValue Attribute value
     * @param bLoading Loading flag
     * @return Result of the attribute setting
     */
    HRESULT OnAttrAlign(const SStringW &strValue, BOOL bLoading);

    /**
     * SRichEdit::OnAttrNotifyChange
     * @brief    Handle the notify change attribute
     * @param    const SStringW &strValue -- The attribute value as a string
     * @param    BOOL bLoading -- Indicates if the attribute is being loaded
     * @return   Returns HRESULT indicating success or failure
     *
     * Describe  This function processes the "notifyChange" attribute, which determines whether the control should receive notifications when data changes.
     */
    HRESULT OnAttrNotifyChange(const SStringW &strValue, BOOL bLoading);

    /**
     * SRichEdit::OnAttrPasswordChar
     * @brief    Handle the password character attribute
     * @param    const SStringW &strValue -- The attribute value as a string
     * @param    BOOL bLoading -- Indicates if the attribute is being loaded
     * @return   Returns HRESULT indicating success or failure
     *
     * Describe  This function processes the "passwordChar" attribute, which sets the character used to display password input.
     */
    HRESULT OnAttrPasswordChar(const SStringW &strValue, BOOL bLoading);

    /**
     * SRichEdit::OnAttrEnableDragdrop
     * @brief    Handle the enable dragdrop attribute
     * @param    const SStringW &strValue -- The attribute value as a string
     * @param    BOOL bLoading -- Indicates if the attribute is being loaded
     * @return   Returns HRESULT indicating success or failure
     *
     * Describe  This function processes the "enableDragdrop" attribute, which determines whether drag-and-drop operations are enabled in the control.
     */
    HRESULT OnAttrEnableDragdrop(const SStringW &strValue, BOOL bLoading);

    /**
     * SRichEdit::OnAttrReStyle
     * @brief    Handle the restyle attribute
     * @param    const SStringW &strValue -- The attribute value as a string
     * @param    DWORD dwStyle -- The style to apply
     * @param    DWORD txtBit -- The text bit to apply
     * @param    BOOL bLoading -- Indicates if the attribute is being loaded
     * @return   Returns HRESULT indicating success or failure
     *
     * Describe  This function processes various restyle attributes, applying the specified style and text bit to the control.
     */
    HRESULT OnAttrReStyle(const SStringW &strValue, DWORD dwStyle, DWORD txtBit, BOOL bLoading);

    /**
     * SRichEdit::OnAttrReStyle2
     * @brief    Handle the restyle attribute (variant)
     * @param    const SStringW &strValue -- The attribute value as a string
     * @param    DWORD dwStyle -- The style to apply
     * @param    DWORD txtBit -- The text bit to apply
     * @param    BOOL bLoading -- Indicates if the attribute is being loaded
     * @return   Returns HRESULT indicating success or failure
     *
     * Describe  This function processes additional restyle attributes, applying the specified style and text bit to the control.
     */
    HRESULT OnAttrReStyle2(const SStringW &strValue, DWORD dwStyle, DWORD txtBit, BOOL bLoading);

  public:
    /**
     * SRichEdit::SOUI_ATTRS_BEGIN
     * @brief    Begin the attribute map for the SRichEdit class
     *
     * Describe  This macro starts the definition of the attribute map, which maps XML attributes to member variables and custom handlers.
     */
    SOUI_ATTRS_BEGIN()
        ATTR_INT(L"style", m_dwStyle, TRUE)
        ATTR_INT(L"maxBuf", m_cchTextMost, FALSE)
        ATTR_BOOL(L"transparent", m_fTransparent, TRUE)
        ATTR_BOOL(L"rich", m_fRich, TRUE)
        ATTR_BOOL(L"vertical", m_fVertical, TRUE)
        ATTR_BOOL(L"wordWrap", m_fWordWrap, TRUE)
        ATTR_BOOL(L"allowBeep", m_fAllowBeep, FALSE)
        ATTR_BOOL(L"autoWordSel", m_fEnableAutoWordSel, TRUE)
        ATTR_BOOL(L"vcenter", m_fSingleLineVCenter, TRUE)
        ATTR_BOOL(L"disableCaret", m_fDisableCaret, TRUE)
        ATTR_BOOL(L"wantTab", m_fWantTab, FALSE)
        ATTR_BOOL(L"autoSel", m_fAutoSel, FALSE)
        ATTR_CUSTOM(L"colorText", OnAttrTextColor)
        ATTR_CUSTOM(L"rtf", OnAttrRTF)
        ATTR_CUSTOM(L"align", OnAttrAlign)
        ATTR_CUSTOM(L"notifyChange", OnAttrNotifyChange)
        ATTR_RE_STYLE(L"wantReturn", ES_WANTRETURN, 0, OnAttrReStyle)
        ATTR_RE_STYLE(L"number", ES_NUMBER, 0, OnAttrReStyle)
        ATTR_RE_STYLE(L"upperCase", ES_UPPERCASE, 0, OnAttrReStyle)
        ATTR_RE_STYLE(L"lowerCase", ES_LOWERCASE, 0, OnAttrReStyle)
        ATTR_RE_STYLE(L"password", ES_PASSWORD, TXTBIT_USEPASSWORD, OnAttrReStyle)
        ATTR_RE_STYLE(L"readOnly", ES_READONLY, TXTBIT_READONLY, OnAttrReStyle)
        ATTR_RE_STYLE(L"multiLines", ES_MULTILINE, TXTBIT_MULTILINE, OnAttrReStyle)
        ATTR_RE_STYLE(L"autoHscroll", ES_AUTOHSCROLL, TXTBIT_SCROLLBARCHANGE, OnAttrReStyle2)
        ATTR_RE_STYLE(L"autoVscroll", ES_AUTOVSCROLL, TXTBIT_SCROLLBARCHANGE, OnAttrReStyle2)
        ATTR_RE_STYLE(L"hscrollBar", WS_HSCROLL, TXTBIT_SCROLLBARCHANGE, OnAttrReStyle2)
        ATTR_RE_STYLE(L"vscrollBar", WS_VSCROLL, TXTBIT_SCROLLBARCHANGE, OnAttrReStyle2)
        ATTR_CUSTOM(L"passwordChar", OnAttrPasswordChar)
        ATTR_CUSTOM(L"enableDragdrop", OnAttrEnableDragdrop)
    SOUI_ATTRS_END()

  protected:
    CHARFORMAT2W m_cfDef;   /**< Default character format  */
    PARAFORMAT2 m_pfDef;    /**< Default paragraph format  */
    DWORD m_cchTextMost;    /**< Maximum number of characters       */
    TCHAR m_chPasswordChar; /**< Character used for password input        */
    LONG m_lAccelPos;       /**< Position of accelerator      */
    SIZEL m_sizelExtent;    /**< Extent array              */
    CRect m_rcInset;        /**< Inset margin              */
    int m_nFontHeight;      /**< Height of single line text output          */
    DWORD m_dwStyle;

    UINT m_fEnableAutoWordSel : 1;              /**< Enable Word style auto word selection?  */
    UINT m_fWordWrap : 1;                       /**< Whether control should word wrap */
    UINT m_fRich : 1;                           /**< Whether control is rich text */
    UINT m_fSaveSelection : 1;                  /**< Whether to save the selection when inactive */
    UINT m_fTransparent : 1;                    /**< Whether control is transparent */
    UINT m_fVertical : 1;                       /**< Whether control is layout following vertical */
    UINT m_fAllowBeep : 1;                      /**< Whether message beep is allowed in the control */
    UINT m_fWantTab : 1;                        /**< Whether control will deal with tab input */
    UINT m_fSingleLineVCenter : 1;              /**< Whether control that is single line will be vertical
                                                   centered */
    UINT m_fScrollPending : 1;                  /**< Whether scroll is activated by richedit or by panelex */
    UINT m_fEnableDragDrop : 1;                 /**< 允许在该控件中使用拖放 */
    UINT m_fAutoSel : 1;                        /**< 有焦点时自动全选 */
    UINT m_fNotifyChange : 1;                   /**< receive re_notify when data changed */
    UINT m_fDisableCaret : 1;                   /**< disable caret flag */
    BYTE m_byDbcsLeadByte;                      /**< DBCS输入时的中文头字节*/
    SStringW m_strRtfSrc;                       /**< 在XML中指定的RTF数据源*/
    STextHost *m_pTxtHost;                      /**< Host of Richedit*/
    SMap<UINT, SAutoRefPtr<ITimer> > m_mapTimer; /**< map of timer to id*/
};

SNSEND

#endif // __SRICHEDIT__H__