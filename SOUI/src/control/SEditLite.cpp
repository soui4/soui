#include <souistd.h>
#include <control/SEdit.h>
#include <core/SSkinObjBase.h>
#include <core/SWindowMgr.h>
#include <helper/SUnkImpl.h>

#if defined(SOUI_EDIT_LITE)

SNSBEGIN

// In-process clipboard buffer (no OS clipboard on bare-metal targets).
static SStringT g_EditClipboard;

// The drag source of the OLE drag currently running (NULL when idle). OLE drag
// is modal, so at most one edit per UI thread can be the source; the drop
// target uses it to tell a self-drop from a cross-control drop.
static SEdit *s_pDragSource = NULL;
// Set by the drop target when a self-drop already moved the text, so the source
// does not delete the block a second time after DoDragDrop returns.
static BOOL s_bSelfDropDone = FALSE;

static void ReleaseMedium(STGMEDIUM &medium)
{
    if (medium.pUnkForRelease)
        medium.pUnkForRelease->Release();
    else if (medium.hGlobal)
        GlobalFree(medium.hGlobal);
    medium.hGlobal = NULL;
    medium.pUnkForRelease = NULL;
}

/** @brief Check whether an IDataObject carries plain text. */
static BOOL QueryTextFormat(IDataObject *pDataObj, FORMATETC *pFmt)
{
    if (!pDataObj)
        return FALSE;
    pFmt->ptd = NULL;
    pFmt->dwAspect = DVASPECT_CONTENT;
    pFmt->lindex = -1;
    pFmt->tymed = TYMED_HGLOBAL;
    pFmt->cfFormat = CF_UNICODETEXT;
    if (pDataObj->QueryGetData(pFmt) == S_OK)
        return TRUE;
    pFmt->cfFormat = CF_TEXT;
    return pDataObj->QueryGetData(pFmt) == S_OK;
}

/** @brief Read plain text (Unicode preferred, ANSI fallback) from an IDataObject. */
static BOOL GetTextFromDataObject(IDataObject *pDataObj, SStringW &strText)
{
    FORMATETC fmt;
    if (!QueryTextFormat(pDataObj, &fmt))
        return FALSE;

    STGMEDIUM medium;
    if (FAILED(pDataObj->GetData(&fmt, &medium)))
        return FALSE;

    if (fmt.cfFormat == CF_UNICODETEXT)
    {
        LPCWSTR psz = (LPCWSTR)GlobalLock(medium.hGlobal);
        if (psz)
        {
            strText = psz;
            GlobalUnlock(medium.hGlobal);
        }
    }
    else
    {
        LPCSTR psz = (LPCSTR)GlobalLock(medium.hGlobal);
        if (psz)
        {
            strText = S_CA2W(psz);
            GlobalUnlock(medium.hGlobal);
        }
    }
    ReleaseMedium(medium);
    return !strText.IsEmpty();
}

/** @brief Check whether an IDataObject carries dropped files (CF_HDROP). */
static BOOL QueryFileFormat(IDataObject *pDataObj, FORMATETC *pFmt)
{
    if (!pDataObj)
        return FALSE;
    pFmt->ptd = NULL;
    pFmt->dwAspect = DVASPECT_CONTENT;
    pFmt->lindex = -1;
    pFmt->tymed = TYMED_HGLOBAL;
    pFmt->cfFormat = CF_HDROP;
    return pDataObj->QueryGetData(pFmt) == S_OK;
}

/** @brief Read dropped file paths from an IDataObject as insertion text. */
static BOOL GetFilesFromDataObject(IDataObject *pDataObj, BOOL bMultiLine, SStringW &strText)
{
    FORMATETC fmt;
    if (!QueryFileFormat(pDataObj, &fmt))
        return FALSE;

    STGMEDIUM medium;
    if (FAILED(pDataObj->GetData(&fmt, &medium)))
        return FALSE;

    HDROP hDrop = (HDROP)GlobalLock(medium.hGlobal);
    if (!hDrop)
    {
        ReleaseMedium(medium);
        return FALSE;
    }

    UINT nFiles = ::DragQueryFileW(hDrop, (UINT)-1, NULL, 0);
    for (UINT i = 0; i < nFiles; ++i)
    {
        WCHAR szFile[MAX_PATH + 1] = {0};
        if (::DragQueryFileW(hDrop, i, szFile, MAX_PATH) == 0)
            continue;
        strText += szFile;
        // A plain-text edit has to turn the file list into visible text; a
        // newline per file matches the demo/RichEdit file-drop behaviour,
        // while a single-line edit keeps everything on one line.
        strText += bMultiLine ? L"\n" : L" ";
    }

    GlobalUnlock(medium.hGlobal);
    ReleaseMedium(medium);
    return !strText.IsEmpty();
}

/** @brief Minimal IEnumFORMATETC over the fixed format list of a text data object. */
class SEditEnumFormatEtc : public SUnkImpl<IEnumFORMATETC>
{
  public:
    SEditEnumFormatEtc(const FORMATETC *pFmts, int nCount)
        : m_nIndex(0)
        , m_nCount(nCount)
    {
        for (int i = 0; i < nCount && i < 2; ++i)
            m_arrFmt[i] = pFmts[i];
    }

    IUNKNOWN_BEGIN(IEnumFORMATETC)
    IUNKNOWN_END()

    STDMETHODIMP Next(ULONG celt, FORMATETC *rgelt, ULONG *pceltFetched)
    {
        ULONG nCopied = 0;
        while (nCopied < celt && m_nIndex < m_nCount)
            rgelt[nCopied++] = m_arrFmt[m_nIndex++];
        if (pceltFetched)
            *pceltFetched = nCopied;
        return (nCopied == celt) ? S_OK : S_FALSE;
    }

    STDMETHODIMP Skip(ULONG celt)
    {
        m_nIndex += (int)celt;
        if (m_nIndex > m_nCount)
            m_nIndex = m_nCount;
        return (m_nIndex < m_nCount) ? S_OK : S_FALSE;
    }

    STDMETHODIMP Reset()
    {
        m_nIndex = 0;
        return S_OK;
    }

    STDMETHODIMP Clone(IEnumFORMATETC **ppEnum)
    {
        if (!ppEnum)
            return E_INVALIDARG;
        *ppEnum = new SEditEnumFormatEtc(m_arrFmt, m_nCount);
        return *ppEnum ? S_OK : E_OUTOFMEMORY;
    }

  private:
    FORMATETC m_arrFmt[2];
    int m_nIndex;
    int m_nCount;
};

/** @brief IDataObject exposing the dragged selection as CF_UNICODETEXT / CF_TEXT. */
class SEditTextDataObject : public SUnkImpl<IDataObject>
{
  public:
    SEditTextDataObject(const SStringT &strText)
        : m_strText(strText)
    {
    }

    IUNKNOWN_BEGIN(IDataObject)
    IUNKNOWN_END()

    STDMETHODIMP GetData(FORMATETC *pformatetcIn, STGMEDIUM *pmedium)
    {
        if (!pformatetcIn || !pmedium)
            return E_INVALIDARG;
        if ((pformatetcIn->tymed & TYMED_HGLOBAL) == 0)
            return DV_E_FORMATETC;
        if (pformatetcIn->cfFormat == CF_UNICODETEXT)
        {
            SStringW strW = S_CT2W(m_strText.c_str());
            return PutBlock(pmedium, strW.c_str(), (int)(strW.GetLength() + 1) * sizeof(wchar_t));
        }
        if (pformatetcIn->cfFormat == CF_TEXT)
        {
            SStringA strA = S_CT2A(m_strText.c_str());
            return PutBlock(pmedium, strA.c_str(), strA.GetLength() + 1);
        }
        return DV_E_FORMATETC;
    }

    STDMETHODIMP GetDataHere(FORMATETC *, STGMEDIUM *)
    {
        return E_NOTIMPL;
    }

    STDMETHODIMP QueryGetData(FORMATETC *pformatetc)
    {
        if (!pformatetc)
            return E_INVALIDARG;
        if ((pformatetc->tymed & TYMED_HGLOBAL) == 0)
            return DV_E_FORMATETC;
        if (pformatetc->cfFormat == CF_UNICODETEXT || pformatetc->cfFormat == CF_TEXT)
            return S_OK;
        return DV_E_FORMATETC;
    }

    STDMETHODIMP GetCanonicalFormatEtc(FORMATETC *, FORMATETC *pformatetcOut)
    {
        if (pformatetcOut)
            pformatetcOut->ptd = NULL;
        return E_NOTIMPL;
    }

    STDMETHODIMP SetData(FORMATETC *, STGMEDIUM *, BOOL)
    {
        return E_NOTIMPL;
    }

    STDMETHODIMP EnumFormatEtc(DWORD dwDirection, IEnumFORMATETC **ppEnumFormatEtc)
    {
        if (!ppEnumFormatEtc)
            return E_INVALIDARG;
        if (dwDirection != DATADIR_GET)
            return E_NOTIMPL;
        FORMATETC fmts[2];
        fmts[0] = MakeFormatEtc(CF_UNICODETEXT);
        fmts[1] = MakeFormatEtc(CF_TEXT);
        *ppEnumFormatEtc = new SEditEnumFormatEtc(fmts, 2);
        return *ppEnumFormatEtc ? S_OK : E_OUTOFMEMORY;
    }

    STDMETHODIMP DAdvise(FORMATETC *, DWORD, IAdviseSink *, DWORD *)
    {
        return OLE_E_ADVISENOTSUPPORTED;
    }

    STDMETHODIMP DUnadvise(DWORD)
    {
        return OLE_E_ADVISENOTSUPPORTED;
    }

    STDMETHODIMP EnumDAdvise(IEnumSTATDATA **)
    {
        return OLE_E_ADVISENOTSUPPORTED;
    }

  private:
    static FORMATETC MakeFormatEtc(WORD cfFormat)
    {
        FORMATETC fmt;
        fmt.cfFormat = cfFormat;
        fmt.ptd = NULL;
        fmt.dwAspect = DVASPECT_CONTENT;
        fmt.lindex = -1;
        fmt.tymed = TYMED_HGLOBAL;
        return fmt;
    }

    HRESULT PutBlock(STGMEDIUM *pmedium, const void *pData, int nBytes)
    {
        HGLOBAL hMem = GlobalAlloc(GMEM_MOVEABLE | GMEM_ZEROINIT, nBytes);
        if (!hMem)
            return E_OUTOFMEMORY;
        void *pDst = GlobalLock(hMem);
        if (!pDst)
        {
            GlobalFree(hMem);
            return E_OUTOFMEMORY;
        }
        memcpy(pDst, pData, nBytes);
        GlobalUnlock(hMem);
        pmedium->tymed = TYMED_HGLOBAL;
        pmedium->hGlobal = hMem;
        pmedium->pUnkForRelease = NULL;
        return S_OK;
    }

    SStringT m_strText;
};

/** @brief IDropSource that ends the drag on release and cancels on Escape. */
class SEditDropSource : public SUnkImpl<IDropSource>
{
  public:
    IUNKNOWN_BEGIN(IDropSource)
    IUNKNOWN_END()

    STDMETHODIMP QueryContinueDrag(BOOL fEscapePressed, DWORD grfKeyState)
    {
        if (fEscapePressed)
            return DRAGDROP_S_CANCEL;
        if ((grfKeyState & MK_LBUTTON) == 0)
        {
            // The first query OLE makes can carry an empty key state before it
            // pumps the pending mouse messages (the RichEdit-based demo hits the
            // same thing and tolerates it). Fall back to the physical button
            // state, otherwise a drag would end the instant it starts.
            if ((::GetAsyncKeyState(VK_LBUTTON) & 0x8000) != 0)
                return S_OK;
            return DRAGDROP_S_DROP;
        }
        return S_OK;
    }

    STDMETHODIMP GiveFeedback(DWORD)
    {
        // Same as the RichEdit drag source: let OLE draw its default move/copy
        // cursors instead of a custom feedback.
        return DRAGDROP_S_USEDEFAULTCURSORS;
    }
};

/** @brief IDropTarget accepting plain text and dropped files for one lightweight <edit>. */
class SEditDropTarget : public SUnkImpl<IDropTarget>
{
  public:
    SEditDropTarget(SEdit *pEdit)
        : m_pEdit(pEdit)
        , m_eDrop(EDropNone)
    {
        if (m_pEdit)
            m_pEdit->AddRef();
    }

    virtual ~SEditDropTarget()
    {
        if (m_pEdit)
            m_pEdit->Release();
    }

    IUNKNOWN_BEGIN(IDropTarget)
    IUNKNOWN_END()

    STDMETHODIMP DragEnter(IDataObject *pDataObj, DWORD grfKeyState, POINTL pt, DWORD *pdwEffect)
    {
        m_eDrop = EDropNone;
        if (m_pEdit && !m_pEdit->m_fReadOnly)
        {
            FORMATETC fmt;
            // Files first: a file drag must never be mistaken for plain text,
            // otherwise the source could perform a MOVE on the real files.
            if (QueryFileFormat(pDataObj, &fmt))
                m_eDrop = EDropFiles;
            else if (QueryTextFormat(pDataObj, &fmt))
                m_eDrop = EDropText;
        }
        if (m_eDrop == EDropNone)
        {
            *pdwEffect = DROPEFFECT_NONE;
            return S_OK;
        }
        return DragOver(grfKeyState, pt, pdwEffect);
    }

    STDMETHODIMP DragOver(DWORD grfKeyState, POINTL, DWORD *pdwEffect)
    {
        if (m_eDrop == EDropText)
            *pdwEffect = (grfKeyState & MK_CONTROL) ? DROPEFFECT_COPY : DROPEFFECT_MOVE;
        else if (m_eDrop == EDropFiles)
            *pdwEffect = DROPEFFECT_LINK;
        else
            *pdwEffect = DROPEFFECT_NONE;
        return S_OK;
    }

    STDMETHODIMP DragLeave(void)
    {
        m_eDrop = EDropNone;
        return S_OK;
    }

    STDMETHODIMP Drop(IDataObject *pDataObj, DWORD grfKeyState, POINTL pt, DWORD *pdwEffect)
    {
        EDropKind eDrop = m_eDrop;
        m_eDrop = EDropNone;
        if (!m_pEdit || eDrop == EDropNone)
        {
            *pdwEffect = DROPEFFECT_NONE;
            return S_OK;
        }

        SStringW strText;
        BOOL bCopy = (grfKeyState & MK_CONTROL) != 0;
        if (eDrop == EDropText)
        {
            if (!GetTextFromDataObject(pDataObj, strText))
            {
                *pdwEffect = DROPEFFECT_NONE;
                return S_FALSE;
            }
        }
        else if (!GetFilesFromDataObject(pDataObj, m_pEdit->m_fMultiLine, strText))
        {
            *pdwEffect = DROPEFFECT_NONE;
            return S_FALSE;
        }

        // OLE reports the drop point in screen coordinates.
        CPoint ptClient(pt.x, pt.y);
        ISwndContainer *pContainer = m_pEdit->GetContainer();
        if (pContainer && pContainer->GetHostHwnd())
            ::ScreenToClient(pContainer->GetHostHwnd(), &ptClient);

        // Only text coming from this very control is a self-move: the drop
        // target then has to remove the dragged block itself.
        BOOL bSelf = (eDrop == EDropText) && (s_pDragSource == m_pEdit);
        if (!m_pEdit->DropTextAt(strText, ptClient, bCopy, bSelf))
        {
            *pdwEffect = DROPEFFECT_NONE;
            return S_OK;
        }

        if (eDrop == EDropFiles)
        {
            *pdwEffect = DROPEFFECT_LINK;
        }
        else if (bCopy)
        {
            *pdwEffect = DROPEFFECT_COPY;
        }
        else
        {
            if (bSelf)
                s_bSelfDropDone = TRUE; // the target already moved the block
            *pdwEffect = DROPEFFECT_MOVE;
        }
        return S_OK;
    }

  private:
    enum EDropKind
    {
        EDropNone,
        EDropText,
        EDropFiles
    };

    SEdit *m_pEdit;
    EDropKind m_eDrop;
};

SEdit::SEdit()
    : m_nCaret(0)
    , m_nAnchor(0)
    , m_cchTextMost(32 * 1024 - 1)
    , m_chPasswordChar(_T('*'))
    , m_lAccelPos(-1)
    , m_crText(CR_INVALID)
    , m_crCue(RGBA(0xcc, 0xcc, 0xcc, 0xff))
    , m_strCue(this)
    , m_nLineHeight(16)
    , m_nTotalHeight(0)
    , m_nContentWidth(0)
    , m_nScrollY(0)
    , m_nScrollX(0)
    , m_bLayoutDirty(TRUE)
    , m_bDragSel(FALSE)
    , m_bSelDragPending(FALSE)
    , m_bSelDragActive(FALSE)
    , m_ptSelDragDown(0, 0)
{
    // Keep the defaults in sync with SRichEdit so an <edit> renders and behaves
    // the same no matter which implementation is compiled in.
    m_pNcSkin = GETBUILTINSKIN(SKIN_SYS_BORDER);
    m_bFocusable = TRUE;
    m_bClipClient = TRUE;
    // SPanel's item drag-scroll would swallow the drag gesture used for text
    // selection, so it must stay off for an edit control.
    m_bItemDragScrollEnabled = FALSE;
    m_evtSet.addEvent(EVENTID(EventRENotify));

    m_dwStyle = ES_LEFT | ES_AUTOHSCROLL;
    m_fRich = 0;
    m_fAutoSel = TRUE;

    m_fEnableAutoWordSel = TRUE;
    m_fWordWrap = FALSE;
    m_fTransparent = FALSE;
    m_fVertical = FALSE;
    m_fAllowBeep = FALSE;
    m_fWantTab = FALSE;
    m_fSingleLineVCenter = TRUE;
    m_fScrollPending = FALSE;
    // Unlike the RichEdit-backed build, the lightweight edit takes part in OLE
    // drag & drop by default (dragging a selection out already works without any
    // XML attribute). Registering the drop target here keeps <edit> usable for
    // both kinds of drops without per-app opt-in; "enableDragdrop=0" still turns
    // it off for apps that need to block drops.
    m_fEnableDragDrop = TRUE;
    m_fNotifyChange = FALSE;
    m_fDisableCaret = FALSE;
    m_fReadOnly = FALSE;
    m_fMultiLine = FALSE;
    m_fNumber = FALSE;
    m_fUpperCase = FALSE;
    m_fLowerCase = FALSE;
    m_fPassword = FALSE;
    m_fWantReturn = FALSE;
    m_fAutoHScroll = TRUE;
    m_fAutoVScroll = FALSE;
    m_fHScrollBar = FALSE;
    m_fVScrollBar = FALSE;
    m_fSaveSelection = TRUE;
}

SEdit::~SEdit()
{
}

// ---------------------------------------------------------------------------
// Text storage (own buffer, replaces SWindow's window-text attribute)
// ---------------------------------------------------------------------------
void SEdit::SetWindowText(LPCTSTR lpszText)
{
    m_strEditText.Empty();
    if (lpszText)
    {
        int nLen = (int)_tcslen(lpszText);
        for (int i = 0; i < nLen; i++)
        {
            TCHAR ch = lpszText[i];
            if (ch == _T('\r'))
            {
                if (i + 1 < nLen && lpszText[i + 1] == _T('\n'))
                    continue;  // CRLF: keep the LF, drop the CR
                ch = _T('\n'); // lone CR
            }
            if (ch == _T('\n') && !m_fMultiLine)
            {
                // A single-line edit owns no line breaks.
                m_strEditText += _T(' ');
                continue;
            }
            m_strEditText += ch;
        }
    }
    if (m_cchTextMost > 0 && m_strEditText.GetLength() > (int)m_cchTextMost)
        m_strEditText = m_strEditText.Left((int)m_cchTextMost);
    m_nCaret = m_strEditText.GetLength();
    m_nAnchor = m_nCaret;
    // An externally assigned text starts a new edit history.
    m_arrUndo.RemoveAll();
    m_arrRedo.RemoveAll();
    m_bLayoutDirty = TRUE;
    UpdateScrollBar();
    UpdateCaretPos();
    Invalidate();
    FireTextChanged();
}

int SEdit::GetWindowText(TCHAR *pBuf, int nBufLen, BOOL bRawText) const
{
    int nLen = m_strEditText.GetLength();
    if (pBuf && nBufLen > 0)
    {
        int nCopy = (nLen < nBufLen - 1) ? nLen : (nBufLen - 1);
        if (nCopy > 0)
            memcpy(pBuf, (LPCTSTR)m_strEditText, (size_t)nCopy * sizeof(TCHAR));
        pBuf[nCopy] = 0;
    }
    return nLen;
}

SStringT SEdit::GetWindowText(BOOL bRawText) const
{
    return m_strEditText;
}

int SEdit::GetWindowTextLength() const
{
    return m_strEditText.GetLength();
}

// ---------------------------------------------------------------------------
// IRichEdit: plain-text relevant
// ---------------------------------------------------------------------------
void SEdit::SetSel(long nStartChar, long nEndChar, BOOL bNoScroll)
{
    // EM_SETSEL semantics (see CTxtEdit::OnSetSel in richedit41): a negative
    // start does NOT mean "from the beginning", it collapses the selection.
    // Dropping file names calls SetSel(-1, -1) to park the caret at the end of
    // the text before ReplaceSel(); mapping that to 0..len would select all and
    // turn the append into a replace.
    int nLen = m_strEditText.GetLength();
    if (nStartChar < 0)
    {
        int nPos = (nEndChar < 0) ? nLen : m_nCaret;
        if (nPos > nLen)
            nPos = nLen;
        if (nPos < 0)
            nPos = 0;
        m_nAnchor = nPos;
        m_nCaret = nPos;
    }
    else
    {
        if (nStartChar > nLen)
            nStartChar = nLen;
        if (nEndChar < 0 || nEndChar > nLen)
            nEndChar = nLen;
        m_nAnchor = (int)nStartChar;
        m_nCaret = (int)nEndChar;
    }
    if (!bNoScroll)
        EnsureCaretVisible();
    UpdateCaretPos();
    Invalidate();
}

void SEdit::SetSel(DWORD dwSelection, BOOL bNoScroll)
{
    // Same packing as SRichEdit::SetSel(DWORD, BOOL): low word = start,
    // high word = end. Forward to the IRichEdit form so lite and RichEdit
    // backed builds select the same range.
    SetSel((long)LOWORD(dwSelection), (long)HIWORD(dwSelection), bNoScroll);
}

void SEdit::ReplaceSel(LPCTSTR pszText, BOOL bCanUndo)
{
    if (m_fReadOnly)
        return;
    if (pszText && *pszText)
    {
        // Deleting the selection and inserting the new text is one undo step;
        // InsertText must not push a second snapshot.
        InsertText(pszText, bCanUndo);
        return;
    }
    if (m_nCaret != m_nAnchor)
    {
        if (bCanUndo)
            PushUndo();
        DeleteSelection();
        FireTextChanged();
    }
    UpdateCaretPos();
    EnsureCaretVisible();
    Invalidate();
}

BOOL SEdit::GetWordWrap() const
{
    return m_fWordWrap;
}

void SEdit::SetWordWrap(BOOL fWordWrap)
{
    m_fWordWrap = fWordWrap;
    m_bLayoutDirty = TRUE;
    UpdateScrollBar();
    UpdateCaretPos();
    Invalidate();
}

BOOL SEdit::GetReadOnly() const
{
    return m_fReadOnly;
}

BOOL SEdit::SetReadOnly(BOOL bReadOnly)
{
    m_fReadOnly = bReadOnly;
    return TRUE;
}

LONG SEdit::GetLimitText() const
{
    return m_cchTextMost;
}

BOOL SEdit::SetLimitText(int nLength)
{
    m_cchTextMost = nLength;
    return TRUE;
}

WORD SEdit::GetDefaultAlign() const
{
    return (WORD)(m_dwStyle & (ES_CENTER | ES_RIGHT));
}

void SEdit::SetDefaultAlign(WORD wNewAlign)
{
    m_dwStyle &= ~(ES_CENTER | ES_RIGHT);
    m_dwStyle |= (wNewAlign & (ES_CENTER | ES_RIGHT));
    Invalidate();
}

BOOL SEdit::GetRichTextFlag() const
{
    return m_fRich;
}

void SEdit::SetRichTextFlag(BOOL fRich)
{
    // Rich text is unsupported; keep plain.
    m_fRich = 0;
}

LONG SEdit::GetDefaultLeftIndent() const
{
    return 0;
}

void SEdit::SetDefaultLeftIndent(LONG lNewIndent)
{
    // No paragraph indent in the plain editor.
}

BOOL SEdit::SetSaveSelection(BOOL fSaveSelection)
{
    m_fSaveSelection = fSaveSelection;
    return TRUE;
}

COLORREF SEdit::SetDefaultTextColor(COLORREF cr)
{
    COLORREF crOld = m_crText;
    m_crText = cr;
    Invalidate();
    return crOld;
}

DWORD SEdit::SaveRtf(LPCTSTR pszFileName)
{
    return 0;
}

DWORD SEdit::LoadRtf(LPCTSTR pszFileName)
{
    return 0;
}

// ---------------------------------------------------------------------------
// Editing primitives
// ---------------------------------------------------------------------------
void SEdit::PushUndo()
{
    size_t nCount = m_arrUndo.GetCount();
    if (nCount >= (size_t)UNDO_LIMIT)
        m_arrUndo.RemoveAt(0, nCount - (size_t)UNDO_LIMIT + 1);

    SEditUndoState state;
    state.strText = m_strEditText;
    state.nCaret = m_nCaret;
    state.nAnchor = m_nAnchor;
    m_arrUndo.Add(state);

    // A fresh edit makes every undone step unreachable.
    if (!m_arrRedo.IsEmpty())
        m_arrRedo.RemoveAll();
}

BOOL SEdit::Undo()
{
    if (m_fReadOnly || m_arrUndo.IsEmpty())
        return FALSE;

    SEditUndoState cur;
    cur.strText = m_strEditText;
    cur.nCaret = m_nCaret;
    cur.nAnchor = m_nAnchor;
    m_arrRedo.Add(cur);

    SEditUndoState state = m_arrUndo[m_arrUndo.GetCount() - 1];
    m_arrUndo.RemoveAt(m_arrUndo.GetCount() - 1);
    ApplyUndoState(state);
    return TRUE;
}

BOOL SEdit::Redo()
{
    if (m_fReadOnly || m_arrRedo.IsEmpty())
        return FALSE;

    SEditUndoState cur;
    cur.strText = m_strEditText;
    cur.nCaret = m_nCaret;
    cur.nAnchor = m_nAnchor;
    m_arrUndo.Add(cur);

    SEditUndoState state = m_arrRedo[m_arrRedo.GetCount() - 1];
    m_arrRedo.RemoveAt(m_arrRedo.GetCount() - 1);
    ApplyUndoState(state);
    return TRUE;
}

void SEdit::ApplyUndoState(const SEditUndoState &state)
{
    m_strEditText = state.strText;
    int nLen = m_strEditText.GetLength();
    m_nCaret = (state.nCaret < 0) ? 0 : (state.nCaret > nLen ? nLen : state.nCaret);
    m_nAnchor = (state.nAnchor < 0) ? 0 : (state.nAnchor > nLen ? nLen : state.nAnchor);
    m_bLayoutDirty = TRUE;
    UpdateScrollBar();
    UpdateCaretPos();
    EnsureCaretVisible();
    Invalidate();
    FireTextChanged();
}

void SEdit::DeleteSelection()
{
    int nStart = (m_nCaret < m_nAnchor) ? m_nCaret : m_nAnchor;
    int nEnd = (m_nCaret < m_nAnchor) ? m_nAnchor : m_nCaret;
    if (nStart == nEnd)
        return;
    m_strEditText.Delete(nStart, nEnd - nStart);
    m_nCaret = nStart;
    m_nAnchor = nStart;
    m_bLayoutDirty = TRUE;
}

void SEdit::InsertText(LPCTSTR pszText, BOOL bCanUndo)
{
    if (m_fReadOnly || !pszText || !(*pszText))
        return;

    SStringT strIns;
    int nSrcLen = (int)_tcslen(pszText);
    for (int i = 0; i < nSrcLen; i++)
    {
        TCHAR ch = pszText[i];
        if (ch == _T('\r'))
        {
            if (i + 1 < nSrcLen && pszText[i + 1] == _T('\n'))
                continue;
            ch = _T('\n');
        }
        if (ch == _T('\n') && !m_fMultiLine)
        {
            strIns += _T(' ');
            continue;
        }
        strIns += ch;
    }
    int nInsLen = strIns.GetLength();
    if (m_cchTextMost > 0)
    {
        int nSel = (m_nCaret < m_nAnchor) ? (m_nAnchor - m_nCaret) : (m_nCaret - m_nAnchor);
        int nRemain = m_cchTextMost - m_strEditText.GetLength() + nSel;
        if (nInsLen > nRemain)
            strIns = strIns.Left(nRemain > 0 ? nRemain : 0);
        nInsLen = strIns.GetLength();
    }
    if (nInsLen <= 0)
    {
        // A non-empty input can still become empty when truncating at the
        // maxBuf limit; a selected range must still be replaced by nothing.
        if (m_nCaret != m_nAnchor)
        {
            if (bCanUndo)
                PushUndo();
            DeleteSelection();
            FireTextChanged();
            EnsureCaretVisible();
            Invalidate();
        }
        UpdateCaretPos();
        return;
    }
    // Only take an undo snapshot once the edit is known to change the text.
    if (bCanUndo)
        PushUndo();
    DeleteSelection();
    int nStart = (m_nCaret < m_nAnchor) ? m_nCaret : m_nAnchor;
    m_strEditText.Insert(nStart, strIns);
    m_nCaret = nStart + nInsLen;
    m_nAnchor = m_nCaret;
    m_bLayoutDirty = TRUE;
    FireTextChanged();
    UpdateCaretPos();
    EnsureCaretVisible();
    Invalidate();
}

void SEdit::InsertChar(TCHAR ch)
{
    if (m_fNumber)
    {
        if (!(ch >= _T('0') && ch <= _T('9')))
            return;
    }
    if (m_fUpperCase)
        ch = (TCHAR)::towupper(ch);
    else if (m_fLowerCase)
        ch = (TCHAR)::towlower(ch);
    TCHAR sz[2] = { ch, 0 };
    InsertText(sz);
}

void SEdit::CopySel()
{
    int nStart = (m_nCaret < m_nAnchor) ? m_nCaret : m_nAnchor;
    int nEnd = (m_nCaret < m_nAnchor) ? m_nAnchor : m_nCaret;
    if (nStart == nEnd)
        return;
    g_EditClipboard = m_strEditText.Mid(nStart, nEnd - nStart);
}

void SEdit::CutSel()
{
    if (m_fReadOnly || m_nCaret == m_nAnchor)
        return;
    PushUndo();
    CopySel();
    DeleteSelection();
    FireTextChanged();
    UpdateCaretPos();
    EnsureCaretVisible();
    Invalidate();
}

void SEdit::PasteSel()
{
    if (m_fReadOnly || g_EditClipboard.IsEmpty())
        return;
    InsertText(g_EditClipboard);
}

void SEdit::SelectAll()
{
    m_nCaret = m_strEditText.GetLength();
    m_nAnchor = 0;
    EnsureCaretVisible();
    UpdateCaretPos();
    Invalidate();
}

void SEdit::FireTextChanged()
{
    if (!m_fNotifyChange)
        return;
    EventRENotify evt(this);
    evt.iNotify = EN_CHANGE;
    evt.pv = NULL;
    evt.hr = S_OK;
    FireEvent(evt);
}

// ---------------------------------------------------------------------------
// Layout
// ---------------------------------------------------------------------------
void SEdit::RecalcLayout(IRenderTarget *pRT)
{
    SPainter painter;
    BeforePaint(pRT, painter);

    TEXTMETRIC tm;
    pRT->GetFontMetrics(&tm);
    m_nLineHeight = tm.tmHeight + tm.tmExternalLeading;
    if (m_nLineHeight <= 0)
        m_nLineHeight = 16;

    CRect rcClient;
    GetClientRect(&rcClient);
    CRect rcPad = GetStyle().GetPadding();
    int nClientW = rcClient.Width() - rcPad.left - rcPad.right;
    if (nClientW < 1)
        nClientW = 1;

    m_arrLineStart.RemoveAll();
    m_arrLineEnd.RemoveAll();

    int nLen = m_strEditText.GetLength();
    // Password display uses a mask glyph per character, so layout must measure
    // the mask width; otherwise wrap points and click hit-testing disagree
    // with what is painted.
    int nMaskW = 0;
    if (m_fPassword)
    {
        TCHAR szMaskCh[2] = { (TCHAR)((m_chPasswordChar != 0) ? m_chPasswordChar : _T('*')), 0 };
        SIZE szMask;
        pRT->MeasureText(szMaskCh, 1, &szMask);
        nMaskW = szMask.cx;
    }
    int i = 0;
    while (i <= nLen)
    {
        int iHardEnd = i;
        while (iHardEnd < nLen && m_strEditText[iHardEnd] != _T('\n'))
            iHardEnd++;

        if (!m_fWordWrap || !m_fMultiLine)
        {
            m_arrLineStart.Add(i);
            m_arrLineEnd.Add(iHardEnd);
            i = iHardEnd + 1;
        }
        else
        {
            int lineStart = i;
            while (lineStart < iHardEnd)
            {
                int best = lineStart;
                int cur = lineStart;
                while (cur < iHardEnd)
                {
                    int cx;
                    if (m_fPassword)
                    {
                        cx = (cur - lineStart + 1) * nMaskW;
                    }
                    else
                    {
                        SStringT sub = m_strEditText.Mid(lineStart, cur - lineStart + 1);
                        SIZE sz;
                        pRT->MeasureText(sub, sub.GetLength(), &sz);
                        cx = sz.cx;
                    }
                    if (cx > nClientW)
                        break;
                    best = cur + 1;
                    cur++;
                }
                if (best == lineStart)
                    best = lineStart + 1; // a single char longer than the width
                m_arrLineStart.Add(lineStart);
                m_arrLineEnd.Add(best);
                lineStart = best;
            }
            if (iHardEnd == i)
            {
                m_arrLineStart.Add(i);
                m_arrLineEnd.Add(i);
            }
            i = iHardEnd + 1;
        }
    }
    if (m_arrLineStart.GetCount() == 0)
    {
        m_arrLineStart.Add(0);
        m_arrLineEnd.Add(0);
    }

    int nLines = (int)m_arrLineStart.GetCount();
    m_nTotalHeight = nLines * m_nLineHeight;
    m_nContentWidth = 0;
    for (int l = 0; l < nLines; l++)
    {
        int ls = m_arrLineStart[l];
        int le = m_arrLineEnd[l];
        if (le > ls)
        {
            int cx;
            if (m_fPassword)
            {
                cx = (le - ls) * nMaskW;
            }
            else
            {
                SStringT sub = m_strEditText.Mid(ls, le - ls);
                SIZE sz;
                pRT->MeasureText(sub, sub.GetLength(), &sz);
                cx = sz.cx;
            }
            if (cx > m_nContentWidth)
                m_nContentWidth = cx;
        }
    }
    m_bLayoutDirty = FALSE;
    AfterPaint(pRT, painter);
}

void SEdit::EnsureLayout()
{
    if (!m_bLayoutDirty)
        return;
    IRenderTarget *pRT = GetRenderTarget(NULL, GRT_NODRAW, TRUE);
    if (!pRT)
        return; // keep the dirty flag; the next paint recalculates with a real RT
    RecalcLayout(pRT);
    ReleaseRenderTarget(pRT);
}

int SEdit::LineFromChar(int nChar) const
{
    int nLines = (int)m_arrLineStart.GetCount();
    for (int l = 0; l < nLines; l++)
    {
        int ls = m_arrLineStart[l];
        int le = m_arrLineEnd[l];
        if (nChar >= ls && nChar < le)
            return l;
        // The caret sitting right before a hard line break belongs to that
        // line; at a soft wrap point it belongs to the following line.
        if (nChar == le && (l == nLines - 1 || (le < m_strEditText.GetLength() && m_strEditText[le] == _T('\n'))))
            return l;
    }
    return nLines > 0 ? nLines - 1 : 0;
}

void SEdit::UpdateScrollBar()
{
    EnsureLayout();

    CRect rcClient;
    SWindow::GetClientRect(&rcClient);
    CRect rcPad = GetStyle().GetPadding();
    int nW = rcClient.Width() - rcPad.left - rcPad.right;
    int nH = rcClient.Height() - rcPad.top - rcPad.bottom;
    if (nW < 1)
        nW = 1;
    if (nH < 1)
        nH = 1;

    BOOL bWordWrap = m_fWordWrap && m_fMultiLine;

    int nContentH = m_fMultiLine ? m_nTotalHeight : m_nLineHeight;
    if (nContentH < nH)
        nContentH = nH;
    // Centered / right-aligned text cannot be scrolled horizontally: it
    // overflows symmetrically (or leftwards) and is clipped instead.
    int nContentW = (bWordWrap || (m_dwStyle & (ES_CENTER | ES_RIGHT))) ? nW : m_nContentWidth;
    if (nContentW < nW)
        nContentW = nW;

    UINT wBarOld = m_wBarVisible;
    m_wBarVisible = SSB_NULL;
    if (m_pSkinSb)
    {
        if (m_fVScrollBar || (m_fAutoVScroll && m_fMultiLine && nContentH > nH))
            m_wBarVisible |= SSB_VERT;
        if (m_fHScrollBar && nContentW > nW)
            m_wBarVisible |= SSB_HORZ;
    }

    if (m_wBarVisible != wBarOld)
    {
        // Let SPanel shrink the client rect to make room for the scrollbar.
        SSendMessage(WM_NCCALCSIZE);

        CRect rcNew;
        GetClientRect(&rcNew);
        nW = rcNew.Width() - rcPad.left - rcPad.right;
        nH = rcNew.Height() - rcPad.top - rcPad.bottom;
        if (nW < 1)
            nW = 1;
        if (nH < 1)
            nH = 1;

        if (bWordWrap)
        {
            // The wrapping width changed; lay the text out again.
            m_bLayoutDirty = TRUE;
            EnsureLayout();
            nContentH = m_nTotalHeight;
            if (nContentH < nH)
                nContentH = nH;
        }
    }

    m_siVer.nPage = nH;
    SetScrollRange(TRUE, 0, nContentH - 1, TRUE);
    SetScrollPos(TRUE, m_nScrollY, TRUE);
    // SetScrollRange()/SetScrollPos() may clamp silently (content shrank), and
    // SPanel::OnScroll() only reports a change when nPos actually moves. Read the
    // effective value back so the render/caret code never uses a stale offset.
    m_nScrollY = m_siVer.nPos;

    m_siHoz.nPage = nW;
    SetScrollRange(FALSE, 0, nContentW - 1, TRUE);
    SetScrollPos(FALSE, m_nScrollX, TRUE);
    m_nScrollX = m_siHoz.nPos;
}

void SEdit::UpdateCaretPos()
{
    if (!IsFocused() || m_fDisableCaret)
        return;
    if (m_arrLineStart.GetCount() == 0)
        return;

    int nLine = LineFromChar(m_nCaret);

    CRect rcClient;
    GetClientRect(&rcClient);
    CRect rcPad = GetStyle().GetPadding();
    CRect rcInner = rcClient;
    rcInner.DeflateRect(rcPad.left, rcPad.top, rcPad.right, rcPad.bottom);

    int nOffset = m_nCaret - m_arrLineStart[nLine];
    int nInnerW = rcInner.Width();
    if (nInnerW < 1)
        nInnerW = 1;
    int nCx = 0;
    IRenderTarget *pRT = GetRenderTarget(NULL, GRT_NODRAW, TRUE);
    if (pRT)
    {
        SPainter painter;
        BeforePaint(pRT, painter);
        nCx = GetLineLeft(pRT, nLine, nInnerW) + ChartX(pRT, nLine, nOffset);
        AfterPaint(pRT, painter);
        ReleaseRenderTarget(pRT);
    }

    int x = rcInner.left + nCx;
    int y;
    if (!m_fMultiLine && m_fSingleLineVCenter)
        y = rcInner.top + (rcInner.Height() - m_nLineHeight) / 2;
    else
        y = rcInner.top + nLine * m_nLineHeight - m_nScrollY;
    SetCaretPos(x, y);
}

static int CharWidth(IRenderTarget *pRT, LPCTSTR psz, int cch)
{
    if (cch <= 0)
        return 0;
    SIZE sz;
    pRT->MeasureText(psz, cch, &sz);
    return sz.cx;
}

int SEdit::ChartX(IRenderTarget *pRT, int nLine, int iChar)
{
    int ls = m_arrLineStart[nLine];
    int le = m_arrLineEnd[nLine];
    int nMax = le - ls;
    if (iChar > nMax)
        iChar = nMax;
    if (iChar <= 0)
        return 0;
    if (m_fPassword)
    {
        // Match RecalcLayout(): a password line is a run of identical mask
        // glyphs, so its width is (count * single-glyph width).
        TCHAR szMaskCh[2] = { (TCHAR)((m_chPasswordChar != 0) ? m_chPasswordChar : _T('*')), 0 };
        SIZE szMask;
        pRT->MeasureText(szMaskCh, 1, &szMask);
        return iChar * szMask.cx;
    }
    return CharWidth(pRT, (LPCTSTR)m_strEditText + ls, iChar);
}

// Horizontal position of a line's first character, relative to rcInner.left.
// Left alignment also applies the horizontal scroll offset; centered and
// right-aligned text ignores scrolling (see UpdateScrollBar).
int SEdit::GetLineLeft(IRenderTarget *pRT, int nLine, int nInnerW)
{
    if (!(m_dwStyle & (ES_CENTER | ES_RIGHT)))
        return -m_nScrollX;
    int nLineW = ChartX(pRT, nLine, m_arrLineEnd[nLine] - m_arrLineStart[nLine]);
    if (m_dwStyle & ES_CENTER)
        return (nInnerW - nLineW) / 2;
    return nInnerW - nLineW;
}

int SEdit::CharFromPoint(CPoint pt, IRenderTarget *pRT)
{
    if (m_bLayoutDirty)
        RecalcLayout(pRT);
    int nLines = (int)m_arrLineStart.GetCount();
    if (nLines <= 0)
        return 0;

    CRect rcClient;
    GetClientRect(&rcClient);
    CRect rcPad = GetStyle().GetPadding();
    CRect rcInner = rcClient;
    rcInner.DeflateRect(rcPad.left, rcPad.top, rcPad.right, rcPad.bottom);
    int nInnerW = rcInner.Width();
    if (nInnerW < 1)
        nInnerW = 1;

    int yRel;
    if (!m_fMultiLine && m_fSingleLineVCenter)
        yRel = 0;
    else
        yRel = pt.y - rcInner.top + m_nScrollY;
    int nLine = yRel / m_nLineHeight;
    if (nLine < 0)
        nLine = 0;
    if (nLine > nLines - 1)
        nLine = nLines - 1;

    int ls = m_arrLineStart[nLine];
    int le = m_arrLineEnd[nLine];
    // Measure with the control's font, exactly like painting does; otherwise
    // the hit test would use whatever font the render target happens to hold.
    SPainter painter;
    BeforePaint(pRT, painter);
    int xLeft = rcInner.left + GetLineLeft(pRT, nLine, nInnerW);
    int xRel = pt.x - xLeft;

    // Snap to the nearest caret boundary: the left half of a character places
    // the caret before it, the right half after it. This lets a click on the
    // first character reach offset 0 instead of always landing behind it.
    int nHit = le;
    int nPrevW = 0;
    for (int k = ls; k <= le; k++)
    {
        int w = ChartX(pRT, nLine, k - ls);
        if (w >= xRel)
        {
            nHit = (xRel - nPrevW <= w - xRel) ? k - 1 : k;
            if (nHit < ls)
                nHit = ls;
            else if (nHit > le)
                nHit = le;
            break;
        }
        nPrevW = w;
    }
    AfterPaint(pRT, painter);
    return nHit;
}

void SEdit::EnsureCaretVisible()
{
    EnsureLayout();
    if (m_arrLineStart.GetCount() == 0)
        return;

    CRect rcClient;
    GetClientRect(&rcClient);
    CRect rcPad = GetStyle().GetPadding();
    int nH = rcClient.Height() - rcPad.top - rcPad.bottom;
    int nW = rcClient.Width() - rcPad.left - rcPad.right;
    if (nH < 1)
        nH = 1;
    if (nW < 1)
        nW = 1;

    int nLine = LineFromChar(m_nCaret);

    int nLineTop = nLine * m_nLineHeight;
    if (nLineTop < m_nScrollY)
        m_nScrollY = nLineTop;
    else if (nLineTop + m_nLineHeight > m_nScrollY + nH)
        m_nScrollY = nLineTop + m_nLineHeight - nH;
    if (m_nScrollY < 0)
        m_nScrollY = 0;

    int nCx = 0;
    IRenderTarget *pRT = GetRenderTarget(NULL, GRT_NODRAW, TRUE);
    if (pRT)
    {
        SPainter painter;
        BeforePaint(pRT, painter);
        nCx = ChartX(pRT, nLine, m_nCaret - m_arrLineStart[nLine]);
        AfterPaint(pRT, painter);
        ReleaseRenderTarget(pRT);
    }
    if (m_dwStyle & (ES_CENTER | ES_RIGHT))
    {
        // Aligned text is never scrolled horizontally.
        m_nScrollX = 0;
    }
    else
    {
        if (nCx < m_nScrollX)
            m_nScrollX = nCx;
        else if (nCx > m_nScrollX + nW)
            m_nScrollX = nCx - nW;
        if (m_nScrollX < 0)
            m_nScrollX = 0;
    }

    UpdateScrollBar();
    UpdateCaretPos();
}

COLORREF SEdit::GetTextColor() const
{
    if (m_crText != CR_INVALID)
        return m_crText;
    int iState = SState2Index::GetDefIndex(((SEdit *)this)->GetState(), TRUE);
    COLORREF cr = GetStyle().GetTextColor(iState);
    if (cr == CR_INVALID)
        return RGBA(0, 0, 0, 255);
    return cr;
}

// ---------------------------------------------------------------------------
// Message / virtual handlers
// ---------------------------------------------------------------------------
int SEdit::OnCreate(LPVOID lpCreateStruct)
{
    // SPanel::OnCreate() reports failure when the built-in scrollbar skin is
    // unavailable (e.g. a resource-less host). A plain-text editor has to work
    // there too, so the result is intentionally ignored; UpdateScrollBar()
    // keeps the scrollbars hidden while m_pSkinSb is NULL.
    __baseCls::OnCreate(NULL);
    m_bLayoutDirty = TRUE;
    // SWindow::OnAttrText only fills the base class text while loading, so the
    // XML "text" attribute has to be pushed into the editor buffer here. This
    // mirrors SRichEdit::OnCreate.
    SetWindowText(SWindow::GetWindowText());
    // Register the OLE drop target for editable controls. Drag & drop is on by
    // default in the lightweight build (see the constructor) and can be turned
    // off with the XML "enableDragdrop" attribute.
    OnEnableDragDrop(!m_fReadOnly && m_fEnableDragDrop);
    return 0;
}

void SEdit::OnDestroy()
{
    OnEnableDragDrop(FALSE);
    ShowCaret(FALSE);
    KillFocus();
    __baseCls::OnDestroy();
}

void SEdit::OnPaint(IRenderTarget *pRT)
{
    CRect rcClient;
    GetClientRect(&rcClient);
    pRT->PushClipRect(&rcClient, RGN_AND);

    SPainter painter;
    BeforePaint(pRT, painter);

    BOOL bWasDirty = m_bLayoutDirty;
    if (bWasDirty)
        RecalcLayout(pRT);

    CRect rcPad = GetStyle().GetPadding();
    CRect rcInner = rcClient;
    rcInner.DeflateRect(rcPad.left, rcPad.top, rcPad.right, rcPad.bottom);
    if (rcInner.Width() < 1 || rcInner.Height() < 1)
        rcInner = rcClient;

    int nLineHeight = m_nLineHeight;
    if (nLineHeight <= 0)
        nLineHeight = 16;
    int nLines = (int)m_arrLineStart.GetCount();

    // y of the first visual line, in window coordinates.
    int nYBase;
    if (!m_fMultiLine && m_fSingleLineVCenter)
        nYBase = rcInner.top + (rcInner.Height() - nLineHeight) / 2;
    else
        nYBase = rcInner.top - m_nScrollY;

    int nFirstLine = 0;
    int nLastLine = nLines - 1;
    if (m_fMultiLine)
    {
        nFirstLine = m_nScrollY / nLineHeight;
        nLastLine = (m_nScrollY + rcInner.Height()) / nLineHeight;
        if (nFirstLine < 0)
            nFirstLine = 0;
        if (nFirstLine > nLines - 1)
            nFirstLine = nLines - 1;
        if (nLastLine > nLines - 1)
            nLastLine = nLines - 1;
    }

    int nInnerW = rcInner.Width();
    int selMin = (m_nCaret < m_nAnchor) ? m_nCaret : m_nAnchor;
    int selMax = (m_nCaret < m_nAnchor) ? m_nAnchor : m_nCaret;
    // Like a native RichEdit, the selection is kept but not painted while the
    // control does not have focus (no ES_NOHIDESEL equivalent here).
    BOOL bShowSel = IsFocused();

    COLORREF crText = GetTextColor();
    COLORREF crOld = pRT->SetTextColor(crText);

    for (int l = nFirstLine; l <= nLastLine; l++)
    {
        int ls = m_arrLineStart[l];
        int le = m_arrLineEnd[l];
        int yTop = nYBase + l * nLineHeight;

        SStringT disp;
        if (le > ls)
        {
            if (m_fPassword)
            {
                TCHAR chMask = (m_chPasswordChar != 0) ? m_chPasswordChar : _T('*');
                for (int k = ls; k < le; k++)
                    disp += chMask;
            }
            else
            {
                disp = m_strEditText.Mid(ls, le - ls);
            }
        }

        int xLeft = rcInner.left + GetLineLeft(pRT, l, nInnerW);

        // Selection highlight. xLeft already is a window coordinate, so the
        // character offsets must not add rcInner.left again.
        if (bShowSel && selMin < selMax && le > ls)
        {
            int s0 = (selMin > ls) ? selMin : ls;
            int s1 = (selMax < le) ? selMax : le;
            if (s0 < s1)
            {
                int x0 = xLeft + ChartX(pRT, l, s0 - ls);
                int x1 = xLeft + ChartX(pRT, l, s1 - ls);
                CRect rcSel(x0, yTop, x1, yTop + nLineHeight);
                pRT->FillSolidRect(&rcSel, RGBA(0x33, 0x66, 0xcc, 0x80));
            }
        }

        if (!disp.IsEmpty())
        {
            CRect rcLine(xLeft, yTop, rcInner.right, yTop + nLineHeight);
            pRT->DrawText(disp, disp.GetLength(), &rcLine, DT_LEFT | DT_SINGLELINE | DT_VCENTER);
        }
    }
    pRT->SetTextColor(crOld);

    // Cue (hint) text overlay when the edit is empty and not focused.
    if (GetWindowTextLength() == 0 && !IsFocused())
    {
        SStringT strCue = GetCueText();
        if (!strCue.IsEmpty())
        {
            COLORREF crOldCue = pRT->SetTextColor(m_crCue);
            pRT->DrawText(strCue, strCue.GetLength(), &rcInner, GetCueTextAlign());
            pRT->SetTextColor(crOldCue);
        }
    }

    AfterPaint(pRT, painter);

    if (bWasDirty)
    {
        UpdateScrollBar();
        UpdateCaretPos();
    }
}

void SEdit::OnSetFocus(SWND wndOld)
{
    __baseCls::OnSetFocus(wndOld);

    if (m_fAutoSel)
    {
        m_nAnchor = 0;
        m_nCaret = m_strEditText.GetLength();
    }

    if (!m_fDisableCaret)
    {
        EnsureLayout();
        // Let the framework own the caret: it handles the blinking itself.
        CreateCaret(NULL, 1, m_nLineHeight);
        ShowCaret(TRUE);
        UpdateCaretPos();
    }

    if (m_fPassword || m_fNumber || m_fReadOnly)
    {
        if (GetContainer())
            GetContainer()->EnableIME(FALSE);
    }
    else if (GetContainer())
    {
#if defined(__ANDROID__) || defined(__IOS__) || defined(__OHOS__) || \
    defined(SOUI_PLATFORM_FREERTOS) || defined(FREERTOS_HOST)
        ShowSoftKeyboard(GetContainer()->GetHostHwnd(), TRUE);
#endif
    }

    // Repaint immediately: the cue text and the selection highlight both
    // depend on the focus state.
    Invalidate();
}

void SEdit::OnKillFocus(SWND wndFocus)
{
    ShowCaret(FALSE);
    m_bSelDragPending = FALSE;
    m_bSelDragActive = FALSE;

    if (!m_fSaveSelection)
    {
        m_nCaret = 0;
        m_nAnchor = 0;
    }

    if (GetContainer())
    {
        if (m_fPassword || m_fNumber)
            GetContainer()->EnableIME(TRUE);
#if defined(__ANDROID__) || defined(__IOS__) || defined(__OHOS__) || \
    defined(SOUI_PLATFORM_FREERTOS) || defined(FREERTOS_HOST)
        ShowSoftKeyboard(GetContainer()->GetHostHwnd(), FALSE);
#endif
        // Prevent the cursor from being hidden while editing.
        GetContainer()->OnUpdateCursor();
    }

    __baseCls::OnKillFocus(wndFocus);

    // Repaint immediately so the selection highlight disappears as soon as the
    // focus moves to the next control (as the RichEdit-backed edit does).
    Invalidate();
}

void SEdit::OnSize(UINT nType, CSize size)
{
    __baseCls::OnSize(nType, size);
    m_bLayoutDirty = TRUE;
    UpdateScrollBar();
    UpdateCaretPos();
    Invalidate();
}

void SEdit::OnLButtonDown(UINT nFlags, CPoint point)
{
    // A click that moves focus here must not throw away the select-all done by
    // OnSetFocus when autoSel is on; a click inside an already focused edit
    // still positions the caret normally.
    BOOL bHadFocus = IsFocused();
    m_bSelDragPending = FALSE;
    m_bSelDragActive = FALSE;
    // Give SPanel the first chance: a click on the scrollbar must not move the
    // caret. SPanel also sets focus and captures the mouse for us.
    if (__baseCls::OnLButtonDown(nFlags, point))
        return;
    // SWindow::OnLButtonDown turns on WndState_PushDown, but the nc/border skin
    // only has normal + hover frames: a pushdown index falls back to normal and
    // the border flickers on every click. SRichEdit never enters the pushdown
    // state, so drop it again to keep the hover look while the button is down.
    ModifyState(0, WndState_PushDown, TRUE);

    if (!bHadFocus && m_fAutoSel && m_strEditText.GetLength() > 0)
    {
        m_bDragSel = FALSE;
        Invalidate();
        return;
    }

    EnsureLayout();
    IRenderTarget *pRT = GetRenderTarget(NULL, GRT_NODRAW, TRUE);
    if (pRT)
    {
        int nPos = CharFromPoint(point, pRT);
        ReleaseRenderTarget(pRT);

        int nSelMin = (m_nCaret < m_nAnchor) ? m_nCaret : m_nAnchor;
        int nSelMax = (m_nCaret < m_nAnchor) ? m_nAnchor : m_nCaret;
        if (!m_fReadOnly && (nFlags & MK_SHIFT) == 0 && nSelMax > nSelMin && nPos >= nSelMin
            && nPos < nSelMax)
        {
            // Pressing inside the selection must not collapse it: wait for the
            // mouse to move far enough to start dragging the block.
            m_bSelDragPending = TRUE;
            m_ptSelDragDown = point;
            m_bDragSel = FALSE;
            return;
        }

        if ((nFlags & MK_SHIFT) == 0)
            m_nAnchor = nPos;
        m_nCaret = nPos;
        m_bDragSel = TRUE;
        EnsureCaretVisible();
        Invalidate();
    }
}

void SEdit::OnLButtonDblClk(UINT nFlags, CPoint point)
{
    SetFocus();
    SetCapture();
    EnsureLayout();
    IRenderTarget *pRT = GetRenderTarget(NULL, GRT_NODRAW, TRUE);
    if (pRT)
    {
        int nPos = CharFromPoint(point, pRT);
        ReleaseRenderTarget(pRT);

        int iStart = nPos;
        int iEnd = nPos;
        int nLen = m_strEditText.GetLength();
        if (m_fEnableAutoWordSel)
        {
            while (iStart > 0 && !_istspace(m_strEditText[iStart - 1]) && m_strEditText[iStart - 1] != _T('\n'))
                iStart--;
            while (iEnd < nLen && !_istspace(m_strEditText[iEnd]) && m_strEditText[iEnd] != _T('\n'))
                iEnd++;
        }
        else
        {
            while (iStart > 0 && m_strEditText[iStart - 1] != _T('\n'))
                iStart--;
            while (iEnd < nLen && m_strEditText[iEnd] != _T('\n'))
                iEnd++;
        }
        m_nAnchor = iStart;
        m_nCaret = iEnd;
        m_bDragSel = TRUE;
        EnsureCaretVisible();
        Invalidate();
    }
}

void SEdit::OnLButtonUp(UINT nFlags, CPoint point)
{
    // Ignore the click the OLE modal loop may replay to the source control.
    if (s_pDragSource == this)
        return;
    if (m_bSelDragActive)
    {
        m_bSelDragActive = FALSE;
        m_bSelDragPending = FALSE;
        DropSelDrag(point, (nFlags & MK_CONTROL) != 0);
        m_bDragSel = FALSE;
        __baseCls::OnLButtonUp(nFlags, point);
        // Restore the text cursor without waiting for the next WM_SETCURSOR.
        __baseCls::OnSetCursor(point);
        return;
    }
    if (m_bSelDragPending)
    {
        // The press stayed inside the selection and never turned into a drag:
        // collapse the selection to the click point, as a native edit does.
        m_bSelDragPending = FALSE;
        EnsureLayout();
        IRenderTarget *pRT = GetRenderTarget(NULL, GRT_NODRAW, TRUE);
        if (pRT)
        {
            int nPos = CharFromPoint(point, pRT);
            ReleaseRenderTarget(pRT);
            m_nCaret = nPos;
            m_nAnchor = nPos;
            EnsureCaretVisible();
            Invalidate();
        }
    }
    m_bDragSel = FALSE;
    __baseCls::OnLButtonUp(nFlags, point);
}

SEdit *SEdit::HitTestEditAt(CPoint pt) const
{
    SWindow *pRoot = GetRoot();
    if (!pRoot)
        return NULL;
    // Windows and message points share the same coordinate space (see
    // SWindow::GetClientRect), so the release point can be tested directly.
    SWND hWnd = pRoot->SwndFromPoint(pt);
    SWindow *pWnd = SWindowMgr::GetWindow(hWnd);
    // The hit window may be a child of the edit; walk up to the control itself.
    while (pWnd)
    {
        if (pWnd->IsClass(L"edit"))
            return static_cast<SEdit *>(pWnd);
        pWnd = pWnd->GetParent();
    }
    return NULL;
}

void SEdit::DropSelDrag(CPoint pt, BOOL bCopy)
{
    if (m_fReadOnly)
        return;

    int nSelMin = (m_nCaret < m_nAnchor) ? m_nCaret : m_nAnchor;
    int nSelMax = (m_nCaret < m_nAnchor) ? m_nAnchor : m_nCaret;
    if (nSelMax <= nSelMin)
        return;

    // There is no OLE drag source: this control keeps the mouse capture for the
    // whole gesture, so a release over *another* <edit> never reaches that edit
    // as a drop. Resolve the window under the release point instead.
    SEdit *pTarget = HitTestEditAt(pt);
    if (!pTarget)
        return; // dropped on empty space / a non-edit control: cancel

    SStringT strSel = m_strEditText.Mid(nSelMin, nSelMax - nSelMin);

    if (pTarget != this)
    {
        // Cross-control drop: insert at the release point of the target and, for
        // a move (no Ctrl), remove the block from this control afterwards.
        if (pTarget->m_fReadOnly)
            return;
        pTarget->EnsureLayout();
        IRenderTarget *pRT = pTarget->GetRenderTarget(NULL, GRT_NODRAW, TRUE);
        if (!pRT)
            return;
        int nPos = pTarget->CharFromPoint(pt, pRT);
        pTarget->ReleaseRenderTarget(pRT);

        int nInsLen = strSel.GetLength();
        if (pTarget->m_cchTextMost > 0)
        {
            int nRemain = (int)pTarget->m_cchTextMost - pTarget->m_strEditText.GetLength();
            if (nInsLen > nRemain)
                nInsLen = (nRemain > 0) ? nRemain : 0;
        }
        if (nInsLen <= 0)
            return;

        pTarget->PushUndo();
        pTarget->m_strEditText.Insert(nPos, strSel.Left(nInsLen));
        pTarget->m_nCaret = nPos + nInsLen;
        pTarget->m_nAnchor = pTarget->m_nCaret;
        pTarget->m_bLayoutDirty = TRUE;
        pTarget->FireTextChanged();
        pTarget->EnsureCaretVisible();
        pTarget->Invalidate();

        if (!bCopy)
        {
            PushUndo();
            m_strEditText.Delete(nSelMin, nSelMax - nSelMin);
            m_nCaret = nSelMin;
            m_nAnchor = nSelMin;
            m_bLayoutDirty = TRUE;
            FireTextChanged();
            UpdateCaretPos();
            EnsureCaretVisible();
            Invalidate();
        }
        return;
    }

    // Drop inside this control: only a point in the client area counts, and a
    // drop onto the selection itself changes nothing.
    CRect rcClient;
    GetClientRect(&rcClient);
    if (!rcClient.PtInRect(pt))
        return;

    EnsureLayout();
    IRenderTarget *pRT = GetRenderTarget(NULL, GRT_NODRAW, TRUE);
    if (!pRT)
        return;
    int nPos = CharFromPoint(pt, pRT);
    ReleaseRenderTarget(pRT);

    if (nPos >= nSelMin && nPos <= nSelMax)
        return; // dropped onto the selection itself: nothing changes

    int nSelLen = nSelMax - nSelMin;

    PushUndo();
    if (!bCopy)
    {
        m_strEditText.Delete(nSelMin, nSelLen);
        if (nPos > nSelMax)
            nPos -= nSelLen;
    }

    int nInsLen = strSel.GetLength();
    if (m_cchTextMost > 0)
    {
        int nRemain = (int)m_cchTextMost - m_strEditText.GetLength();
        if (nInsLen > nRemain)
            nInsLen = (nRemain > 0) ? nRemain : 0;
    }
    if (nInsLen > 0)
    {
        m_strEditText.Insert(nPos, strSel.Left(nInsLen));
        m_nCaret = nPos + nInsLen;
    }
    else
    {
        m_nCaret = nPos;
    }
    m_nAnchor = m_nCaret;
    m_bLayoutDirty = TRUE;
    FireTextChanged();
    UpdateCaretPos();
    EnsureCaretVisible();
    Invalidate();
}

// ---------------------------------------------------------------------------
// OLE drag & drop (same mechanism as the RichEdit-backed build)
// ---------------------------------------------------------------------------
void SEdit::OnEnableDragDrop(BOOL bEnable)
{
    if (!GetContainer())
    {
        return;
    }
    if (bEnable)
    {
        SEditDropTarget *pDropTarget = new SEditDropTarget(this);
        BOOL bRet = GetContainer()->RegisterDragDrop(m_swnd, pDropTarget);
        pDropTarget->Release();
    }
    else
    {
        BOOL bRet = GetContainer()->UnregisterDragDrop(m_swnd);
    }
}

void SEdit::StartOleDrag()
{
    int nSelMin = (m_nCaret < m_nAnchor) ? m_nCaret : m_nAnchor;
    int nSelMax = (m_nCaret < m_nAnchor) ? m_nAnchor : m_nCaret;
    if (nSelMax <= nSelMin)
        return;

    SStringT strSel = m_strEditText.Mid(nSelMin, nSelMax - nSelMin);
    if (strSel.IsEmpty())
    {
        return;
    }

    SEditTextDataObject *pDataObj = new SEditTextDataObject(strSel);
    SEditDropSource *pDropSrc = new SEditDropSource();

    // OLE takes over the mouse: the drop target under the cursor (possibly
    // another edit or another process) has to see the mouse messages.
    ReleaseCapture();

    s_pDragSource = this;
    s_bSelfDropDone = FALSE;
    DWORD dwEffect = DROPEFFECT_NONE;
    HRESULT hr = ::DoDragDrop(pDataObj, pDropSrc, DROPEFFECT_COPY | DROPEFFECT_MOVE, &dwEffect);
    s_pDragSource = NULL;

    pDropSrc->Release();
    pDataObj->Release();

    if (!s_bSelfDropDone && (dwEffect & DROPEFFECT_MOVE))
    {
        // Cross-control move: the target inserted the text, so remove the block
        // from this control. A self-drop already did that in the drop target,
        // and a copy / cancelled drag must leave the selection untouched.
        PushUndo();
        m_strEditText.Delete(nSelMin, nSelMax - nSelMin);
        m_nCaret = nSelMin;
        m_nAnchor = nSelMin;
        m_bLayoutDirty = TRUE;
        FireTextChanged();
        UpdateCaretPos();
        EnsureCaretVisible();
        Invalidate();
    }
}

BOOL SEdit::DropTextAt(const SStringW &strText, CPoint pt, BOOL bCopy, BOOL bSelf)
{
    if (m_fReadOnly || strText.IsEmpty())
        return FALSE;

    EnsureLayout();
    IRenderTarget *pRT = GetRenderTarget(NULL, GRT_NODRAW, TRUE);
    if (!pRT)
        return FALSE;
    int nPos = CharFromPoint(pt, pRT);
    ReleaseRenderTarget(pRT);

    int nSelMin = (m_nCaret < m_nAnchor) ? m_nCaret : m_nAnchor;
    int nSelMax = (m_nCaret < m_nAnchor) ? m_nAnchor : m_nCaret;

    // Dropping the block onto the selection itself changes nothing.
    if (bSelf && nSelMax > nSelMin && nPos >= nSelMin && nPos <= nSelMax)
        return FALSE;

    SStringT strIns = S_CW2T(strText.c_str());
    int nInsLen = strIns.GetLength();
    if (m_cchTextMost > 0)
    {
        int nRemain = (int)m_cchTextMost - m_strEditText.GetLength();
        if (nInsLen > nRemain)
            nInsLen = (nRemain > 0) ? nRemain : 0;
    }
    if (nInsLen <= 0)
        return FALSE;

    PushUndo();
    if (bSelf && !bCopy && nSelMax > nSelMin)
    {
        // Move inside the same control: remove the block first and fix the
        // insertion index for the shifted text.
        int nSelLen = nSelMax - nSelMin;
        m_strEditText.Delete(nSelMin, nSelLen);
        if (nPos > nSelMax)
            nPos -= nSelLen;
    }

    m_strEditText.Insert(nPos, strIns.Left(nInsLen));
    m_nCaret = nPos + nInsLen;
    m_nAnchor = m_nCaret;
    m_bLayoutDirty = TRUE;
    FireTextChanged();
    UpdateCaretPos();
    EnsureCaretVisible();
    Invalidate();
    return TRUE;
}

void SEdit::OnMouseMove(UINT nFlags, CPoint point)
{
    // DoDragDrop runs a modal loop; ignore the mouse messages it replays to the
    // source control while the drag is in flight.
    if (s_pDragSource == this)
        return;
    if (m_bSelDragPending)
    {
        if ((nFlags & MK_LBUTTON) == 0)
        {
            m_bSelDragPending = FALSE;
        }
        else
        {
            int nDx = point.x - m_ptSelDragDown.x;
            int nDy = point.y - m_ptSelDragDown.y;
            if (nDx < 0)
                nDx = -nDx;
            if (nDy < 0)
                nDy = -nDy;
            int nThreshold = 4 * GetScale() / 100;
            if (nThreshold < 1)
                nThreshold = 1;
            if (nDx > nThreshold || nDy > nThreshold)
            {
                // Far enough from the press point: this is a selection drag, not
                // a click. With a real host window the gesture is handed to OLE,
                // exactly like the RichEdit-backed build; a window-less host
                // (headless tests / bare metal) keeps the in-process fallback.
                m_bSelDragPending = FALSE;
                ISwndContainer *pContainer = GetContainer();
                HWND hHost = pContainer ? pContainer->GetHostHwnd() : NULL;
                if (hHost)
                {
                    StartOleDrag();
                }
                else
                {
                    m_bSelDragActive = TRUE;
                    // Update the cursor at once instead of waiting for the next
                    // WM_SETCURSOR, so the drag is visible as soon as it starts.
                    OnSetCursor(point);
                }
            }
            return;
        }
    }
    if (m_bSelDragActive)
        return;
    if (m_bDragSel && (nFlags & MK_LBUTTON))
    {
        EnsureLayout();
        IRenderTarget *pRT = GetRenderTarget(NULL, GRT_NODRAW, TRUE);
        if (pRT)
        {
            m_nCaret = CharFromPoint(point, pRT);
            ReleaseRenderTarget(pRT);
            EnsureCaretVisible();
            Invalidate();
        }
        return;
    }
    __baseCls::OnMouseMove(nFlags, point);
}

BOOL SEdit::OnMouseWheel(UINT nFlags, short zDelta, CPoint pt)
{
    // SPanel scrolls the scrollbar (with fade in/out) and falls back to the
    // parent window when this control cannot scroll.
    return __baseCls::OnMouseWheel(nFlags, zDelta, pt);
}

BOOL SEdit::OnScroll(BOOL bVertical, UINT uCode, int nPos)
{
    BOOL bRet = __baseCls::OnScroll(bVertical, uCode, nPos);
    if (bRet)
    {
        if (bVertical)
            m_nScrollY = m_siVer.nPos;
        else
            m_nScrollX = m_siHoz.nPos;
        UpdateCaretPos();
    }
    return bRet;
}

int SEdit::GetScrollLineSize(BOOL bVertical)
{
    if (bVertical && m_nLineHeight > 0)
        return m_nLineHeight;
    return 16;
}

void SEdit::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
    BOOL bCtrl = (GetKeyState(VK_CONTROL) & 0x8000) != 0;
    BOOL bShift = (GetKeyState(VK_SHIFT) & 0x8000) != 0;
    int nLen = m_strEditText.GetLength();

    if (bCtrl)
    {
        switch (nChar)
        {
        case 'A':
        case 'a':
            SelectAll();
            return;
        case 'C':
        case 'c':
            CopySel();
            return;
        case 'X':
        case 'x':
            CutSel();
            return;
        case 'V':
        case 'v':
            PasteSel();
            return;
        case 'Z':
        case 'z':
            // Ctrl+Shift+Z is the common alternative redo shortcut.
            if (bShift)
                Redo();
            else
                Undo();
            return;
        case 'Y':
        case 'y':
            Redo();
            return;
        default:
            break;
        }
    }

    switch (nChar)
    {
    case VK_LEFT:
    {
        if (m_nCaret > 0)
            m_nCaret--;
        if (!bShift)
            m_nAnchor = m_nCaret;
        UpdateCaretPos();
        EnsureCaretVisible();
        Invalidate();
        break;
    }
    case VK_RIGHT:
    {
        if (m_nCaret < nLen)
            m_nCaret++;
        if (!bShift)
            m_nAnchor = m_nCaret;
        UpdateCaretPos();
        EnsureCaretVisible();
        Invalidate();
        break;
    }
    case VK_UP:
    case VK_DOWN:
    {
        EnsureLayout();
        int nLines = (int)m_arrLineStart.GetCount();
        int nCur = LineFromChar(m_nCaret);
        int nOffset = m_nCaret - m_arrLineStart[nCur];
        int nTarget = (nChar == VK_UP) ? nCur - 1 : nCur + 1;
        if (nTarget >= 0 && nTarget < nLines)
        {
            int tls = m_arrLineStart[nTarget];
            int tle = m_arrLineEnd[nTarget];
            int nMaxOff = tle - tls;
            m_nCaret = tls + (nOffset < nMaxOff ? nOffset : nMaxOff);
        }
        if (!bShift)
            m_nAnchor = m_nCaret;
        UpdateCaretPos();
        EnsureCaretVisible();
        Invalidate();
        break;
    }
    case VK_HOME:
    {
        if (bCtrl)
        {
            m_nCaret = 0;
        }
        else
        {
            EnsureLayout();
            m_nCaret = m_arrLineStart[LineFromChar(m_nCaret)];
        }
        if (!bShift)
            m_nAnchor = m_nCaret;
        UpdateCaretPos();
        EnsureCaretVisible();
        Invalidate();
        break;
    }
    case VK_END:
    {
        if (bCtrl)
        {
            m_nCaret = nLen;
        }
        else
        {
            EnsureLayout();
            m_nCaret = m_arrLineEnd[LineFromChar(m_nCaret)];
        }
        if (!bShift)
            m_nAnchor = m_nCaret;
        UpdateCaretPos();
        EnsureCaretVisible();
        Invalidate();
        break;
    }
    case VK_BACK:
    {
        if (m_fReadOnly)
            break;
        if (m_nCaret != m_nAnchor)
        {
            PushUndo();
            DeleteSelection();
        }
        else if (m_nCaret > 0)
        {
            PushUndo();
            m_strEditText.Delete(m_nCaret - 1, 1);
            m_nCaret--;
            m_nAnchor = m_nCaret;
            m_bLayoutDirty = TRUE;
        }
        else
        {
            break; // nothing to delete, no undo step either
        }
        FireTextChanged();
        UpdateCaretPos();
        EnsureCaretVisible();
        Invalidate();
        break;
    }
    case VK_DELETE:
    {
        if (m_fReadOnly)
            break;
        if (m_nCaret != m_nAnchor)
        {
            PushUndo();
            DeleteSelection();
        }
        else if (m_nCaret < nLen)
        {
            PushUndo();
            m_strEditText.Delete(m_nCaret, 1);
            m_bLayoutDirty = TRUE;
        }
        else
        {
            break; // nothing to delete, no undo step either
        }
        FireTextChanged();
        UpdateCaretPos();
        EnsureCaretVisible();
        Invalidate();
        break;
    }
    case VK_TAB:
    {
        if (m_fWantTab && !m_fReadOnly)
            InsertText(_T("\t"));
        else
            __baseCls::OnKeyDown(nChar, nRepCnt, nFlags);
        break;
    }
    default:
        __baseCls::OnKeyDown(nChar, nRepCnt, nFlags);
        break;
    }
}

void SEdit::OnChar(UINT nChar, UINT nRepCnt, UINT nFlags)
{
    if (m_fReadOnly)
        return;
    BOOL bCtrl = (GetKeyState(VK_CONTROL) & 0x8000) != 0;
    switch (nChar)
    {
        // Ctrl-Return generates Ctrl-J (LF); treat it as an ordinary return.
    case 0x0A:
    case VK_RETURN:
        if (!bCtrl && !(m_fMultiLine && m_fWantReturn))
            return;
        if (m_fMultiLine)
            InsertText(_T("\n"));
        return;
    case VK_TAB:
        if (m_fWantTab)
            InsertText(_T("\t"));
        return;
    case VK_ESCAPE:
        return;
    default:
        break;
    }
    if (nChar < 0x20)
        return; // other control characters
    InsertChar((TCHAR)nChar);
}

LRESULT SEdit::OnImeChar(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    if (m_fReadOnly)
        return 0;
    InsertChar((TCHAR)wParam);
    return 0;
}

LRESULT SEdit::OnGetSel(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    int nStart = (m_nCaret < m_nAnchor) ? m_nCaret : m_nAnchor;
    int nEnd = (m_nCaret < m_nAnchor) ? m_nAnchor : m_nCaret;
    if (wParam)
        *(DWORD *)wParam = (DWORD)nStart;
    if (lParam)
        *(DWORD *)lParam = (DWORD)nEnd;
    return MAKELONG(nStart, nEnd);
}

LRESULT SEdit::OnSetSel(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    // EM_SETSEL semantics: wParam = start, lParam = end. A negative start
    // collapses the selection (end < 0 collapses it at the text end).
    SetSel((long)(int)wParam, (long)(int)lParam, FALSE);
    return 0;
}

LRESULT SEdit::OnReplaceSel(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    if (m_fReadOnly)
        return 0;
    ReplaceSel((LPCTSTR)lParam, (BOOL)wParam);
    return 0;
}

LRESULT SEdit::OnSetEventMask(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    // Mirror the RichEdit EM_SETEVENTMASK semantics for the lightweight build:
    // wParam == 0 means "replace the whole mask" with the mask in lParam.
    // SEdit only tracks the ENM_CHANGE bit, which gates FireTextChanged().
    if (wParam == 0)
    {
        m_fNotifyChange = (lParam & ENM_CHANGE) ? TRUE : FALSE;
    }
    return 0;
}

LRESULT SEdit::OnUndo(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    Undo();
    return 0;
}

LRESULT SEdit::OnCanUndo(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    return (!m_fReadOnly && !m_arrUndo.IsEmpty()) ? TRUE : FALSE;
}

LRESULT SEdit::OnRedo(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    return Redo() ? TRUE : FALSE;
}

LRESULT SEdit::OnEmptyUndoBuffer(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    m_arrUndo.RemoveAll();
    m_arrRedo.RemoveAll();
    return 0;
}

void SEdit::OnSetFont(IFontS *font, BOOL bRedraw)
{
    m_bLayoutDirty = TRUE;
    UpdateScrollBar();
    if (bRedraw)
        Invalidate();
    // SPanel/SWindow have no OnSetFont handler: MSG_WM_SETFONT_EX dispatches
    // WM_SETFONT straight to this method, so there is no base call to forward
    // to (SRichEdit::OnSetFont likewise does not chain to a base).
}

void SEdit::OnEnable(BOOL bEnable, UINT nStatus)
{
    __baseCls::OnEnable(bEnable, nStatus);
    Invalidate();
}

UINT WINAPI SEdit::OnGetDlgCode() const
{
    UINT uCode = SC_WANTCHARS | SC_WANTARROWS;
    if (m_fWantTab)
        uCode |= SC_WANTTAB;
    if (m_fWantReturn)
        uCode |= SC_WANTRETURN;
    return uCode;
}

BOOL SEdit::CancelCaptureMode(int reason)
{
    // A selection drag keeps the mouse captured until the release point is
    // hit-tested (it may be another edit). Letting an outer scroll container
    // steal the capture would end the gesture silently.
    if (m_bSelDragPending || m_bSelDragActive)
        return FALSE;
    return __baseCls::CancelCaptureMode(reason);
}

BOOL SEdit::OnSetCursor(const CPoint &pt)
{
    if (m_bSelDragActive)
    {
        // Emulate what the OLE default drag cursors show in re41: a move cursor
        // over a valid drop target and a "no drop" cursor elsewhere.
        HCURSOR hCursor =
            HitTestEditAt(pt) ? GETRESPROVIDER->LoadCursor(IDC_SIZEALL) : GETRESPROVIDER->LoadCursor(IDC_NO);
        ::SetCursor(hCursor);
        return TRUE;
    }
    return __baseCls::OnSetCursor(pt);
}

void SEdit::OnCaptureChanged(BOOL bCaptured)
{
    if (!bCaptured)
    {
        m_bSelDragPending = FALSE;
        m_bSelDragActive = FALSE;
        m_bDragSel = FALSE;
    }
    __baseCls::OnCaptureChanged(bCaptured);
}

void SEdit::OnScaleChanged(int nScale)
{
    __baseCls::OnScaleChanged(nScale);
    m_bLayoutDirty = TRUE;
    UpdateScrollBar();
    if (IsFocused() && !m_fDisableCaret)
    {
        EnsureLayout();
        CreateCaret(NULL, 1, m_nLineHeight);
        ShowCaret(TRUE);
    }
    UpdateCaretPos();
    Invalidate();
}

void SEdit::OnRebuildFont()
{
    __baseCls::OnRebuildFont();
    m_bLayoutDirty = TRUE;
    UpdateScrollBar();
    if (IsFocused() && !m_fDisableCaret)
    {
        EnsureLayout();
        CreateCaret(NULL, 1, m_nLineHeight);
        ShowCaret(TRUE);
    }
    UpdateCaretPos();
    Invalidate();
}

HRESULT SEdit::OnLanguageChanged()
{
    HRESULT hr = __baseCls::OnLanguageChanged();
    m_strCue.TranslateText();
    return hr;
}

void SEdit::GetCueText(IStringT *pStr) const
{
    SStringT str = m_strCue.GetText(FALSE);
    pStr->Copy(&str);
}

SStringT SEdit::GetCueText(BOOL bRawText) const
{
    return m_strCue.GetText(bRawText);
}

UINT SEdit::GetCueTextAlign()
{
    UINT algin = SWindow::GetTextAlign();
    algin &= ~(DT_CENTER | DT_RIGHT);
    if (m_dwStyle & ES_CENTER)
        algin |= DT_CENTER;
    else if (m_dwStyle & ES_RIGHT)
        algin |= DT_RIGHT;
    return algin;
}

// ---------------------------------------------------------------------------
// Attribute handlers (mirror SRichEdit's surface)
// ---------------------------------------------------------------------------
void SEdit::OnEditAttrChanged()
{
    m_bLayoutDirty = TRUE;
    UpdateScrollBar();
    UpdateCaretPos();
    Invalidate();
}

HRESULT SEdit::OnAttrTextColor(const SStringW &strValue, BOOL bLoading)
{
    m_crText = GETCOLOR(strValue);
    if (!bLoading)
        Invalidate();
    return S_OK;
}

HRESULT SEdit::OnAttrRtf(const SStringW &strValue, BOOL bLoading)
{
    // Rich text is not supported by the lightweight build; keep the raw value
    // so the XML keeps loading exactly like it does for SRichEdit.
    m_strRtfSrc = strValue;
    return S_OK;
}

HRESULT SEdit::OnAttrRich(const SStringW &strValue, BOOL bLoading)
{
    // Plain text only: rich="1" is accepted but has no effect.
    m_fRich = 0;
    return S_OK;
}

HRESULT SEdit::OnAttrWordWrap(const SStringW &strValue, BOOL bLoading)
{
    m_fWordWrap = (strValue.CompareNoCase(L"1") == 0 || strValue.CompareNoCase(L"true") == 0);
    if (!bLoading)
        OnEditAttrChanged();
    return S_OK;
}

HRESULT SEdit::OnAttrAlign(const SStringW &strValue, BOOL bLoading)
{
    m_dwStyle &= ~(ES_CENTER | ES_RIGHT);
    SStringW v = strValue;
    v.MakeLower();
    if (v.CompareNoCase(L"center") == 0)
        m_dwStyle |= ES_CENTER;
    else if (v.CompareNoCase(L"right") == 0)
        m_dwStyle |= ES_RIGHT;
    if (!bLoading)
        Invalidate();
    return S_OK;
}

HRESULT SEdit::OnAttrNotifyChange(const SStringW &strValue, BOOL bLoading)
{
    m_fNotifyChange = (strValue.CompareNoCase(L"1") == 0 || strValue.CompareNoCase(L"true") == 0);
    return S_OK;
}

HRESULT SEdit::OnAttrWantReturn(const SStringW &strValue, BOOL bLoading)
{
    m_fWantReturn = (strValue.CompareNoCase(L"1") == 0 || strValue.CompareNoCase(L"true") == 0);
    if (m_fWantReturn)
        m_dwStyle |= ES_WANTRETURN;
    return S_OK;
}

HRESULT SEdit::OnAttrNumber(const SStringW &strValue, BOOL bLoading)
{
    m_fNumber = (strValue.CompareNoCase(L"1") == 0 || strValue.CompareNoCase(L"true") == 0);
    if (m_fNumber)
        m_dwStyle |= ES_NUMBER;
    return S_OK;
}

HRESULT SEdit::OnAttrUpperCase(const SStringW &strValue, BOOL bLoading)
{
    m_fUpperCase = (strValue.CompareNoCase(L"1") == 0 || strValue.CompareNoCase(L"true") == 0);
    if (m_fUpperCase)
        m_dwStyle |= ES_UPPERCASE;
    return S_OK;
}

HRESULT SEdit::OnAttrLowerCase(const SStringW &strValue, BOOL bLoading)
{
    m_fLowerCase = (strValue.CompareNoCase(L"1") == 0 || strValue.CompareNoCase(L"true") == 0);
    if (m_fLowerCase)
        m_dwStyle |= ES_LOWERCASE;
    return S_OK;
}

HRESULT SEdit::OnAttrPassword(const SStringW &strValue, BOOL bLoading)
{
    m_fPassword = (strValue.CompareNoCase(L"1") == 0 || strValue.CompareNoCase(L"true") == 0);
    if (m_fPassword)
        m_dwStyle |= ES_PASSWORD;
    return S_OK;
}

HRESULT SEdit::OnAttrReadOnly(const SStringW &strValue, BOOL bLoading)
{
    m_fReadOnly = (strValue.CompareNoCase(L"1") == 0 || strValue.CompareNoCase(L"true") == 0);
    if (m_fReadOnly)
        m_dwStyle |= ES_READONLY;
    return S_OK;
}

HRESULT SEdit::OnAttrMultiLines(const SStringW &strValue, BOOL bLoading)
{
    m_fMultiLine = (strValue.CompareNoCase(L"1") == 0 || strValue.CompareNoCase(L"true") == 0);
    if (m_fMultiLine)
        m_dwStyle |= ES_MULTILINE;
    else
        m_dwStyle &= ~ES_MULTILINE;
    if (!bLoading)
        OnEditAttrChanged();
    return S_OK;
}

HRESULT SEdit::OnAttrAutoHscroll(const SStringW &strValue, BOOL bLoading)
{
    m_fAutoHScroll = (strValue.CompareNoCase(L"1") == 0 || strValue.CompareNoCase(L"true") == 0);
    if (m_fAutoHScroll)
        m_dwStyle |= ES_AUTOHSCROLL;
    else
        m_dwStyle &= ~ES_AUTOHSCROLL;
    if (!bLoading)
        OnEditAttrChanged();
    return S_OK;
}

HRESULT SEdit::OnAttrAutoVscroll(const SStringW &strValue, BOOL bLoading)
{
    m_fAutoVScroll = (strValue.CompareNoCase(L"1") == 0 || strValue.CompareNoCase(L"true") == 0);
    if (m_fAutoVScroll)
        m_dwStyle |= ES_AUTOVSCROLL;
    else
        m_dwStyle &= ~ES_AUTOVSCROLL;
    if (!bLoading)
        OnEditAttrChanged();
    return S_OK;
}

HRESULT SEdit::OnAttrHscrollBar(const SStringW &strValue, BOOL bLoading)
{
    m_fHScrollBar = (strValue.CompareNoCase(L"1") == 0 || strValue.CompareNoCase(L"true") == 0);
    if (m_fHScrollBar)
        m_dwStyle |= WS_HSCROLL;
    else
        m_dwStyle &= ~WS_HSCROLL;
    if (!bLoading)
        OnEditAttrChanged();
    return S_OK;
}

HRESULT SEdit::OnAttrVscrollBar(const SStringW &strValue, BOOL bLoading)
{
    m_fVScrollBar = (strValue.CompareNoCase(L"1") == 0 || strValue.CompareNoCase(L"true") == 0);
    if (m_fVScrollBar)
        m_dwStyle |= WS_VSCROLL;
    else
        m_dwStyle &= ~WS_VSCROLL;
    if (!bLoading)
        OnEditAttrChanged();
    return S_OK;
}

HRESULT SEdit::OnAttrPasswordChar(const SStringW &strValue, BOOL bLoading)
{
    if (strValue.IsEmpty())
        m_chPasswordChar = 0;
    else
        m_chPasswordChar = (TCHAR)strValue[0];
    if (!bLoading)
        Invalidate();
    return S_OK;
}

HRESULT SEdit::OnAttrEnableDragdrop(const SStringW &strValue, BOOL bLoading)
{
    m_fEnableDragDrop = (strValue.CompareNoCase(L"1") == 0 || strValue.CompareNoCase(L"true") == 0);
    if (!bLoading && GetContainer())
        OnEnableDragDrop(!m_fReadOnly && m_fEnableDragDrop);
    return S_OK;
}

SNSEND
#endif // SOUI_EDIT_LITE
