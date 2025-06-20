/*
 *	Unicode <--> MultiByte conversions, OLE, and other system functions
 *
 *	Copyright (c) 1995-2001, Microsoft Corporation. All rights reserved.
 */
#include <objbase.h>	// CoCreateInstance define
#include "_common.h"
#include "_font.h"
#include "_uspi.h"
#include <stdio.h>
#include <imm.h>
#include <ole2.h>

#define DEFAULT_PITCH 0
#define MAX_HKLS 256			// It will be a while before we have more KBs

static HINSTANCE g_hOleAut32 = NULL;
static HINSTANCE g_hOle32 = NULL;

#ifndef NOACCESSIBILITY 
static HINSTANCE g_hAcc = NULL;
static HINSTANCE g_hUser32 = NULL;
#endif

#define DXOFPRECT(prc)    ((prc)->right - (prc)->left)
#define DYOFPRECT(prc)    ((prc)->bottom - (prc)->top)

#ifndef DECLSPEC_IMPORT
#define DECLSPEC_IMPORT
#endif//DECLSPEC_IMPORT

#define RE_OLE32API(name)		DECLSPEC_IMPORT	HRESULT (STDAPICALLTYPE *name)
#define RE_OLE32API_(type, name) DECLSPEC_IMPORT type (STDAPICALLTYPE *name)
#define RE_OLEAUTAPI(name) 		DECLSPEC_IMPORT HRESULT (STDAPICALLTYPE *name)
#define RE_OLEAUTAPI_(type, name) 	DECLSPEC_IMPORT type (STDAPICALLTYPE *name)

class CIMM32_PROC
{
public:
	void *ImmGetCompositionStringA;
	void *ImmGetCompositionStringW;
	void *ImmGetContext;
	void *ImmSetCompositionFontA;
	void *ImmSetCompositionWindow;
	void *ImmReleaseContext;
	void *ImmGetProperty;
	void *ImmGetCandidateWindow;
	void *ImmSetCandidateWindow;
	void *ImmNotifyIME;
	void *ImmAssociateContext;
	void *ImmGetVirtualKey;
	void *ImmEscapeA;
	void *ImmEscapeW;
	void *ImmGetOpenStatus;
	void *ImmSetOpenStatus;
	void *ImmGetConversionStatus;
	void *ImmSetConversionStatus;
	void *ImmGetDefaultIMEWnd;
	void *ImmSetCompositionStringW;
	void *ImmIsIME;
};
static CIMM32_PROC	g_IMM32Proc;


class COLEAUT32_PROC
{
public:
	void *LoadRegTypeLib;
	void *LoadTypeLib;
	void *LoadTypeLibEx;
	void *SysAllocString;
	void *SysAllocStringLen;
	void *SysFreeString;
	void *SysStringLen;
	void *VariantInit;
	void *VariantClear;
};
static COLEAUT32_PROC	g_OleAut32Proc;

class COLE32_PROC
{
public:
	void *OleCreateFromData;
	void *CoTaskMemFree;
	void *CreateBindCtx;
	void *OleDuplicateData;
	void *CoTreatAsClass;
	void *ProgIDFromCLSID;
	void *OleConvertIStorageToOLESTREAM;
	void *OleConvertIStorageToOLESTREAMEx;
	void *OleSave;
	void *StgCreateDocfileOnILockBytes;
	void *CreateILockBytesOnHGlobal;
	void *OleCreateLinkToFile;
	void *CoTaskMemAlloc;
	void *CoTaskMemRealloc;
	void *OleInitialize;
	void *OleUninitialize;
	void *OleSetClipboard;
	void *OleFlushClipboard;
	void *OleIsCurrentClipboard;
	void *DoDragDrop;
	void *OleGetClipboard;
	void *RegisterDragDrop;
	void *OleCreateLinkFromData;
	void *OleCreateStaticFromData;
	void *OleDraw;
	void *OleSetContainedObject;
	void *CoDisconnectObject;
	void *WriteFmtUserTypeStg;
	void *WriteClassStg;
	void *SetConvertStg;
	void *ReadFmtUserTypeStg;
	void *ReadClassStg;
	void *OleRun;
	void *RevokeDragDrop;
	void *CreateStreamOnHGlobal;
	void *GetHGlobalFromStream;
	void *OleCreateDefaultHandler;
	void *CLSIDFromProgID;
	void *OleConvertOLESTREAMToIStorage;
	void *OleLoad;
	void *ReleaseStgMedium;
	void *CoCreateInstance;
	void *OleCreateFromFile;
};
static COLE32_PROC	g_Ole32Proc;

class CConvertStr
{
public:
    operator char *();

protected:
    CConvertStr();
    ~CConvertStr();
    void Free();

    LPSTR   _pstr;
    char    _ach[MAX_PATH * 2];
};

inline CConvertStr::operator char *()
{
    return _pstr;
}

inline CConvertStr::CConvertStr()
{
    _pstr = NULL;
}

inline CConvertStr::~CConvertStr()
{
    Free();
}

class CStrIn : public CConvertStr
{
public:
    CStrIn(LPCWSTR pwstr, UINT CodePage = CP_ACP);
    CStrIn(LPCWSTR pwstr, int cwch, UINT CodePage = CP_ACP);
    int strlen();

protected:
    CStrIn();
    void Init(LPCWSTR pwstr, int cwch, UINT CodePage = CP_ACP);

    int _cchLen;
};

inline CStrIn::CStrIn()
{
}

inline int CStrIn::strlen()
{
    return _cchLen;
}

class CStrOut : public CConvertStr
{
public:
    CStrOut(LPWSTR pwstr, int cwchBuf);
    ~CStrOut();

    int     BufSize();
    int     Convert();

private:
    LPWSTR  _pwstr;
    int     _cwchBuf;
};

inline int CStrOut::BufSize()
{
    return _cwchBuf * 2;
}

//
//	Multi-Byte ---> Unicode conversion
//

class CStrOutW : public CConvertStrW
{
public:
    CStrOutW(LPSTR pstr, int cchBuf, UINT uiCodePage);
    ~CStrOutW();

    int     BufSize();
    int     Convert();

private:

    LPSTR  	_pstr;
    int     _cchBuf;
	UINT	_uiCodePage;
};

inline int CStrOutW::BufSize()
{
    return _cchBuf;
}

DWORD CW32System::AddRef()
{
	return ++_cRefs;
}

DWORD CW32System::Release()
{
	DWORD culRefs =	--_cRefs;

	if(culRefs == 0)
	{
#ifndef NOFEPROCESSING
		FreeIME();
#endif
		if (g_hOle32)
		{
			EnterCriticalSection(&g_CriticalSection);
			OleUninitialize();
			FreeLibrary(g_hOle32);
			g_hOle32 = NULL;
			memset(&g_Ole32Proc, 0, sizeof(g_Ole32Proc));
 			LeaveCriticalSection(&g_CriticalSection);
		}
	}
	return culRefs;
}

ATOM WINAPI CW32System::RegisterREClass(
	const WNDCLASSW *lpWndClass)
{
	ATOM atom;

	TRACEBEGIN(TRCSUBSYSWRAP, TRCSCOPEINTERN, "RegisterREClass");
	atom = ::RegisterClass(lpWndClass); 
	if (!atom && GetLastError() == ERROR_CLASS_ALREADY_EXISTS)
		atom = FindAtom(lpWndClass->lpszClassName);

	return atom;
}

LONG ValidateTextRange(TEXTRANGE *pstrg);
#ifndef NOANSIWINDOWS
LRESULT CW32System::ANSIWndProc(
	HWND	hwnd,
	UINT	msg,
	WPARAM	wparam,
	LPARAM	lparam,
	BOOL	fIs10Mode)
{
	TRACEBEGIN(TRCSUBSYSHOST, TRCSCOPEINTERN, "RichEditANSIWndProc");

	#ifdef DEBUG
	Tracef(TRCSEVINFO, "hwnd %lx, msg %lx, wparam %lx, lparam %lx", hwnd, msg, wparam, lparam);
	#endif	// DEBUG

	LRESULT	lres;

	switch( msg )
	{

	case EM_REPLACESEL:
	case WM_SETTEXT:
	{	
		SETTEXTEX st = {ST_CHECKPROTECTION, 0};
				if(msg == EM_REPLACESEL)
		{
			st.flags = wparam ? ST_CHECKPROTECTION | ST_SELECTION | ST_KEEPUNDO | ST_10REPLACESEL
							  :	ST_CHECKPROTECTION | ST_SELECTION | ST_10REPLACESEL;
		}
		else if (fIs10Mode)
			st.flags |= ST_10WM_SETTEXT;		// 1.0 Mode WM_SETTEXT

		return RichEditWndProc(hwnd, EM_SETTEXTEX, (WPARAM)&st, lparam);
	}

	case EM_FINDTEXT:
	case EM_FINDTEXTEX:
		{
			// We cheat a little here because FINDTEXT and FINDTEXTEX overlap
			// with the exception of the extra out param chrgText in FINDTEXTEX
			FINDTEXTEXW ftexw;
			FINDTEXTA *pfta = (FINDTEXTA *)lparam;
			CStrInW strinw(pfta->lpstrText, W32->GetKeyboardCodePage());

			ftexw.chrg = pfta->chrg;
			ftexw.lpstrText = (WCHAR *)strinw;

			lres = RichEditWndProc(hwnd, msg, wparam, (LPARAM)&ftexw);
			
			if(msg == EM_FINDTEXTEX)
			{
				// In the FINDTEXTEX case, the extra field in the
				// FINDTEXTEX data structure is an out parameter indicating
				// the range where the text was found.  Update the 'real'
				// [in, out] parameter accordingly.	
				((FINDTEXTEXA *)lparam)->chrgText = ftexw.chrgText;
			}
			return lres;
		}
		break;				

	case EM_GETSELTEXT:
		{
			GETTEXTEX gt;
			const char chDefault = ' ';

			gt.cb = (unsigned)-1;			// Client claims to have enuf room		
			gt.flags = GT_SELECTION;		// Get selected text
			gt.codepage = (unsigned)-1;		// Use default CCharFormat codepage
			gt.lpDefaultChar = &chDefault;	// Translate other chars into blanks
			gt.lpUsedDefChar = NULL;

			return RichEditWndProc(hwnd, EM_GETTEXTEX, (WPARAM)&gt, lparam);
		}
		break;

	// case WM_GETTEXT:	Handled by Ansi filter

	// case WM_GETTEXTLENGTH: Handled by Ansi filter

	case EM_GETTEXTRANGE:
		{
			TEXTRANGEA *ptrg = (TEXTRANGEA *)lparam;

            LONG clInBuffer = ValidateTextRange((TEXTRANGEW *) ptrg);

            // If size is -1, this means that the size required is the total
            // size of the the text.
            if(-1 == clInBuffer)
            {
                // We can get this length either by digging the data out of the
                // various structures below us or we can take advantage of the
                // WM_GETTEXTLENGTH message. The first might be slightly 
                // faster but the second definitely save code size. So we
                // will go with the second.
                clInBuffer = SendMessage(hwnd, WM_GETTEXTLENGTH, 0, 0);
            }

            if(0 == clInBuffer)
            {
                // The buffer was invalid for some reason or there was not data
                // to copy. In any case, we are done.
                return 0;
            }

            // Verify that the output buffer is big enough.
            if(IsBadWritePtr(ptrg->lpstrText, clInBuffer + 1))
            {
                // Not enough space so don't copy any
                return 0;
            }

			// For EM_GETTEXTRANGE case, we again don't know how big the
			// incoming buffer is, only that it should be *at least* as
			// great as cpMax - cpMin in the text range structure.  We also
			// know that anything *bigger* than (cpMax - cpMin)*2 bytes is
			// uncessary.  So we'll just assume that it's "big enough"
			// and let WideCharToMultiByte scribble as much as it needs.
			// Memory shortages are the caller's responsibility (courtesy
			// of the RichEdit 1.0 design).
			
			CStrOutW stroutw(ptrg->lpstrText, (clInBuffer + 1) * sizeof(WCHAR), 
							 RichEditWndProc(hwnd, EM_GETCODEPAGE, 0, 0));
			TEXTRANGEW trgw;
			trgw.chrg = ptrg->chrg;
			trgw.lpstrText = (WCHAR *)stroutw;

			RichEditWndProc(hwnd, EM_GETTEXTRANGE, wparam, (LPARAM)&trgw);
			
			return stroutw.Convert();	// Return count of BYTEs converted
		}

	case EM_GETLINE:
		{
			// The size is indicated by the first word of the memory pointed
			// to by lparam
			WORD size = *(WORD *)lparam;
			CStrOutW stroutw((char *)lparam, (DWORD)size,
							 RichEditWndProc(hwnd, EM_GETCODEPAGE, 0, 0));
			WCHAR *pwsz = (WCHAR *)stroutw;
			*(WORD *)pwsz = size;

			lres = RichEditWndProc(hwnd, msg, wparam, (LPARAM)pwsz);

			if (lres < size)
				*(pwsz+lres) = L'\0';		// EM_GETLINE does not return NULL-terminated string.
			LONG cach = stroutw.Convert();
			return fIs10Mode ? cach : lres;	// Return count of BYTEs converted
		}

#ifdef DEBUG
	case WM_NCCREATE:
	case WM_CREATE:
		// These messages should be handled higher up so let everyone
		// know we got to the wrong place!
		AssertSz(FALSE, "CW32System::ANSIWndProc got WM_CREATE or WM_NCCREATE");
		break;

#endif // DEBUG

	}
	return RichEditWndProc(hwnd, msg, wparam, lparam);
}

// Note that AnsiFilter could be refined so that we could get rid
// of the ANSI window proc.
void CW32System::AnsiFilter(
	UINT &	msg,
	WPARAM &wparam,
	LPARAM lparam,
	void *pvoid,
	BOOL f10Mode
)
{
	GETTEXTEX *pgt;
	GETTEXTLENGTHEX *pgtl;
	WM_CHAR_INFO *pwmci;
	bool fAltNumPad = (GetKeyboardFlags() & ALTNUMPAD) != 0;
	UINT cpg = GetKeyboardCodePage(0xFFFFFFFF);

	switch (msg)
	{
		case WM_CHAR:
			pwmci = (WM_CHAR_INFO *) pvoid;
			pwmci->_fTrailByte = false;
			pwmci->_fLeadByte = false;
			pwmci->_fIMEChar = false;
			if (pwmci->_fAccumulate) {
				// We could do some validation here.
				pwmci->_fTrailByte = true;
				return;
			}
			// WM_CHAR > 256 on Win95; assumed to be Unicode

			if(fAltNumPad)
			{
				DWORD Number = GetKeyPadNumber();
				if(Number >= 256 || GetKeyboardFlags() & ALT0)
				{
					wparam = Number;
					if(!IN_RANGE(1250, cpg, 1258))	// Use 1252 for DBCS
						cpg = 1252;					//  codepages
				}
			}

			if(IN_RANGE(128, wparam, 255))
			{							
				bool fShift = (GetKeyboardFlags() & SHIFT) != 0;
				bool fCtrl  = (GetKeyboardFlags() & CTRL)  != 0;

				// If fAltNumPad is set, wparam is HiAnsi
				// If Shift + Alt + Ctrl, it is repeat event from Win3.1 IME
				if ((!fAltNumPad || (fShift && fCtrl)) && GetTrailBytesCount((BYTE) wparam, cpg))
				{
					pwmci->_fLeadByte = true;
					return;
				}
				WPARAM	wparamNew = 0;
				switch ( cpg )
				{				
					case CP_JAPAN:
						// for Japanese codepage, need to translate SBC if KANA mode is on					 
						if ((GetKeyState(VK_KANA) & 1) || f10Mode)		
							break;
						// If not in KANA mode, then fall thru to use 1252 codepage...
					case CP_KOREAN:
					case CP_CHINESE_TRAD:
					case CP_CHINESE_SIM:
						// use English codepage since there is no HiAnsi conversion for
						// FE systems
						cpg = 1252;
						break;				
				}

				if (cpg == 1252 && !IN_RANGE(0x80, wparam, 0x9f))
					return;

				// Convert single byte WM_CHAR messages to Unicode
				if(UnicodeFromMbcs((LPWSTR)&wparamNew, 1, (char *)&wparam, 1,
					cpg) == 1 )
				{
					wparam = wparamNew;
					if (fAltNumPad)
						SetKeyPadNumber(wparam);
				}
			}
			else if(lparam == 1 &&
				wparam > 256 && !fAltNumPad)
			{		
				// On WinNT s/w generated WM_CHAR, this should be WM_IME_CHAR message
				// for some Chinese Level 2 IME.
				if ( cpg == CP_CHINESE_SIM || cpg == CP_CHINESE_TRAD )
				{
					BYTE	bTrailByte = wparam >> 8;
					BYTE	bLeadByte = wparam;
					wparam =  (bLeadByte << 8) | bTrailByte;
					pwmci->_fIMEChar = true;
				}
			}
			return;

		case WM_GETTEXT:
			// EVIL HACK ALERT: on Win95, WM_GETTEXT should always be treated
			// as an ANSI message.
			pgt = (GETTEXTEX *) pvoid;
			pgt->cb = wparam;
			pgt->flags = GT_USECRLF;
			pgt->codepage = 0;
			pgt->lpDefaultChar = NULL;
			pgt->lpUsedDefChar = NULL;
			msg = EM_GETTEXTEX;
			wparam = (WPARAM) pgt;
			return;

		case WM_GETTEXTLENGTH:
			// EVIL HACK ALERT: on Win95, WM_GETEXTLENGTH should always
			// be treated an ANSI message because some old apps will send
			// this message to arbitrary windows (e.g., accessibility aps)
			pgtl = (GETTEXTLENGTHEX *) pvoid;
			pgtl->flags = GTL_NUMBYTES | GTL_PRECISE | GTL_USECRLF;
			pgtl->codepage = 0;
			msg = EM_GETTEXTLENGTHEX;
			wparam = (WPARAM) pgtl;
			return;
	}
}
#endif// NOANSIWINDOWS

/*
 *	CW32System::CheckChangeKeyboardLayout (iCharRep)
 *
 *	@mfunc
 *		Change keyboard for new character repertoire, or charrep at new
 *		character position.
 *
 *	@rdesc
 *		Keyboard hkl selected. 0 if failed to find keyboard
 *
 *	@comm
 *		Using only the currently loaded KBs, locate one that will support
 *		bCharSet. This is called anytime a character format	change occurs,
 *		or the caret position changes.
 *
 *	@devnote
 *		The current KB is preferred. If a previous association was made,
 *		see if the KB is still loaded in the system and if so use it.
 *		Otherwise, locate a suitable KB, preferring KB's that have
 *		the same charset ID as their default, preferred charset. If no
 *		match can be found, nothing changes.
 */
HKL CW32System::CheckChangeKeyboardLayout(
	BYTE iCharRep)
{
	return ActivateKeyboard(iCharRep);
}

HKL CW32System::GetKeyboardLayout (
	DWORD dwThreadID)
{
	if(dwThreadID == 0x0FFFFFFFF)
		RefreshKeyboardLayout();

	return _hklCurrent;
}

/*
 *	CW32System::RefreshKeyboardLayout ()
 *
 *	@mfunc
 *		Update _hklCurrent with current keyboard layout and update
 *		entry for corresponding script.
 */
void CW32System::RefreshKeyboardLayout ()
{
	_hklCurrent = ::GetKeyboardLayout(0);
	LONG iCharRep = CharRepFromLID(LOWORD(_hklCurrent));
	SetPreferredKbd(iCharRep, _hklCurrent);
}

/*
 *	CW32System::ActivateKeyboard (iCharRep)
 *
 *	@mfunc
 *		Change keyboard to that for iCharRep
 *
 *	@rdesc
 *		Keyboard hkl selected. 0 if no keyboard assigned to iCharRep
 */
HKL CW32System::ActivateKeyboard(
	LONG iCharRep)
{
	HKL hkl = 0;
	if((unsigned)iCharRep < NCHARREPERTOIRES)
	{
		hkl = GetPreferredKbd(iCharRep);
		if(hkl)
		{
			if(hkl != _hklCurrent && ActivateKeyboardLayout(hkl, 0))
				_hklCurrent = hkl;

			AssertSz(hkl == ::GetKeyboardLayout(0),
				"CW32System::ActivateKeyboard: incorrect _hklCurrent");
		}
	}
	return hkl;
}

#ifndef NOCOMPLEXSCRIPTS
/*
 *	CW32System::FindDirectionalKeyboard (fRTL)
 *
 *	@mfunc
 *		Find first keyboard with direction given by fRTL
 *
 *	@rdesc
 *		HKL of keyboard selected. 0 if no keyboard for direction given by fRTL
 */
HKL CW32System::FindDirectionalKeyboard(
	BOOL fRTL)
{
	Assert(ARABIC_INDEX == HEBREW_INDEX + 1);
	int iCharRep;

	if(fRTL)
	{
		iCharRep = CharRepFromCodePage(GetACP());
		if(!IN_RANGE(HEBREW_INDEX, iCharRep, ARABIC_INDEX))
			iCharRep = _hkl[HEBREW_INDEX] ? HEBREW_INDEX :
				  _hkl[ARABIC_INDEX] ? ARABIC_INDEX : -1;
	}
	else
		for(iCharRep = 0;
			iCharRep < NCHARREPERTOIRES && (!_hkl[iCharRep] || IN_RANGE(HEBREW_INDEX, iCharRep, ARABIC_INDEX));
			iCharRep++)
				;
	return ActivateKeyboard(iCharRep);
}
#endif


#ifndef NOFEPROCESSING
BOOL CW32System::ImmInitialize( void )
{
	// MAC Only function.
	return FALSE;
}

void CW32System::ImmTerminate( void )
{
	// MAC only function.
	return;
}
#endif	// NOFEPROCESSING

#ifndef NOACCESSIBILITY

typedef HRESULT (WINAPI *ACC_VC_CAST)(VARIANTARG FAR*, VARIANTARG FAR*);
HRESULT CW32System::VariantCopy(VARIANTARG FAR*  pvargDest, VARIANTARG FAR*  pvargSrc)
{
    static void *pVariantCopy = NULL;
	if (pVariantCopy == NULL)
		SetProcAddr( pVariantCopy, DLL_OLEAUT32, "VariantCopy" );
	if (pVariantCopy)
		return ((ACC_VC_CAST)pVariantCopy)(pvargDest, pvargSrc);

    return (E_NOINTERFACE);
}

typedef LRESULT (WINAPI *ACC_LFO_CAST)(REFIID, WPARAM, LPUNKNOWN);
LRESULT CW32System::LResultFromObject(REFIID riid, WPARAM wParam, LPUNKNOWN punk)
{
	static void *pLResultFromObject = NULL;
	if (pLResultFromObject == NULL)
		SetProcAddr( pLResultFromObject, DLL_ACC, "LresultFromObject" );
	if (pLResultFromObject)
		return ((ACC_LFO_CAST)pLResultFromObject)(riid, wParam, punk);
	return E_NOINTERFACE;
}

typedef HRESULT (WINAPI *ACC_CSAPW)(HWND, LPCWSTR, LONG, REFIID, void**);
HRESULT CW32System::CreateStdAccessibleProxyW(HWND hwnd, LPCWSTR pClassName, LONG idObject, REFIID riid, void** ppvObject)
{
	static void *pCreateStdAccessibleProxyW = NULL;
	if (pCreateStdAccessibleProxyW == NULL)
		SetProcAddr( pCreateStdAccessibleProxyW, DLL_ACC, "CreateStdAccessibleProxyW");
	if (pCreateStdAccessibleProxyW)
		return ((ACC_CSAPW)pCreateStdAccessibleProxyW)(hwnd, pClassName, idObject, riid, ppvObject);
	return E_NOINTERFACE;
}

typedef HRESULT (WINAPI *ACC_AOFW_CAST)(HWND, DWORD, REFIID, void **);
HRESULT CW32System::AccessibleObjectFromWindow (HWND hWnd, DWORD dwID, REFIID riidInterface, void ** ppvObject)
{
    static void *pAccessibleObjectFromWindow = NULL;
	if (pAccessibleObjectFromWindow == NULL)
		SetProcAddr( pAccessibleObjectFromWindow, DLL_ACC, "AccessibleObjectFromWindow" );
	if (pAccessibleObjectFromWindow)
		return ((ACC_AOFW_CAST)pAccessibleObjectFromWindow)(hWnd, dwID, riidInterface, ppvObject);
    return (E_NOINTERFACE);
}

typedef BOOL (WINAPI *ACC_BI_CAST)(BOOL);
BOOL CW32System::BlockInput (BOOL fBlock)
{
    static void *pBlockInput = NULL;
	if (pBlockInput == NULL)
		SetProcAddr( pBlockInput, DLL_USER32, "BlockInput" );
	if (pBlockInput)
		return ((ACC_BI_CAST)pBlockInput)(fBlock);

    return FALSE;
}

typedef UINT (WINAPI *ACC_SI_CAST)(UINT, LPINPUT, int);
UINT CW32System::SendInput (UINT nInputs, LPINPUT pInputs, int cbSize)
{
    static void *pSendInput = NULL;
	if (pSendInput == NULL)
		SetProcAddr( pSendInput, DLL_USER32, "SendInput" );
	if (pSendInput)
		return ((ACC_SI_CAST)pSendInput)(nInputs, pInputs, cbSize);

    return 0;
}

typedef VOID (WINAPI *ACC_NWE_CAST)(DWORD, HWND, LONG, LONG);
VOID CW32System::NotifyWinEvent(DWORD dwEvent, HWND hWnd, LONG lObjectType, LONG lObjectId)
{
	static void *pNotfiyWinEvent = NULL;
	if (pNotfiyWinEvent == INVALID_HANDLE_VALUE)
		return;

	if (pNotfiyWinEvent == NULL)
		SetProcAddr( pNotfiyWinEvent, DLL_USER32, "NotifyWinEvent" );
	if (pNotfiyWinEvent)
		((ACC_NWE_CAST)pNotfiyWinEvent)(dwEvent, hWnd, lObjectType, lObjectId);
	else 
		pNotfiyWinEvent = INVALID_HANDLE_VALUE;

}

#endif


CONVERTMODE WINAPI CW32System::DetermineConvertMode( HDC hdc, BYTE tmCharSet )
{
	CONVERTMODE cm = CVT_NONE;

	// Some fonts have problems under Win95 with the GetCharWidthW call; this
	// is a simple heuristic to determine if this problem exists.
	{
		INT		widthA, widthW;
		BOOL	fResA, fResW;

		// FE font on Non-FE Win95 cannot use
		// GetCharWidthW and ExtTextOutW
		if(IsFECharSet(tmCharSet))
			// always use ANSI call for DBC fonts.
			cm = CVT_WCTMB;
		else
		{
			fResA = GetCharWidthA( hdc, ' ', ' ', &widthA );
			fResW = GetCharWidthW( hdc, L' ', L' ', &widthW );
			if ( fResA && fResW && widthA != widthW )
				cm = CVT_WCTMB;
			else
			{
				fResA = GetCharWidthA( hdc, 'a', 'a', &widthA );
				fResW = GetCharWidthW( hdc, L'a', L'a', &widthW );
				if ( fResA && fResW && widthA != widthW )
					cm = CVT_WCTMB;
			}
		}
	}
	return cm;
}

void WINAPI CW32System::CalcUnderlineInfo(HDC hdc, CCcs *pcccs, TEXTMETRIC *ptm )
{
#ifdef _WIN32
	if (ptm->tmPitchAndFamily & TMPF_TRUETYPE)
	{
		OUTLINETEXTMETRICA otm;

		if (GetOutlineTextMetricsA(hdc, sizeof(otm), &otm))
		{
			pcccs->_dyULOffset = -otm.otmsUnderscorePosition;
			pcccs->_dyULWidth = otm.otmsUnderscoreSize;
			pcccs->_dySOOffset = -otm.otmsStrikeoutPosition;
			pcccs->_dySOWidth = otm.otmsStrikeoutSize;
			//Handle fonts like Mangal and Latha which very small underline width, but do
			//have a reasonable strikeout width.
			if (pcccs->_dyULWidth < pcccs->_dySOWidth || pcccs->_dyULWidth <= 0)
				pcccs->_dyULWidth = pcccs->_dySOWidth = max(pcccs->_dySOWidth, max(pcccs->_dyULWidth, 1));
			return;
		}
	}
#endif//_WIN32
	// Default calculation of size of underline
	SHORT dyDescent = pcccs->_yDescent;

	if (0 == dyDescent)
		dyDescent = pcccs->_yHeight >> 3;

	pcccs->_dyULWidth = (short) max(1, dyDescent / 4);
	pcccs->_dyULOffset = (dyDescent - 3 * pcccs->_dyULWidth + 1) / 2;

	if ((0 == pcccs->_dyULOffset) && (dyDescent > 1))
		pcccs->_dyULOffset = 1;

	pcccs->_dySOOffset = -ptm->tmAscent / 3;
	pcccs->_dySOWidth = pcccs->_dyULWidth;

	return;
}

BOOL WINAPI CW32System::ShowScrollBar( HWND hWnd, int wBar, BOOL bShow, LONG )
{
	return ::ShowScrollBar( hWnd, wBar, bShow );
}

BOOL WINAPI CW32System::EnableScrollBar( HWND hWnd, UINT wSBflags, UINT wArrows )
{
	return ::EnableScrollBar( hWnd, wSBflags, wArrows );
}

void AdvanceXpYp(int &xp, int &yp, UINT cch, const INT **ppdx, TFLOW tflow)
{
	LONG upAdjust = 0;
	while (cch--)
		upAdjust += *(*ppdx)++;

	switch(tflow)
	{
	case tflowES:
		xp += upAdjust;
		break;
	case tflowSW:
		yp += upAdjust;
		break;
	case tflowWN:
		xp -= upAdjust;
		break;
	case tflowNE:
		yp -= upAdjust;
		break;
	}
}

/*
 * 	ReExtTextOutW(hdc, x, y, fuOptions, lprc, lpString, cch, lpDx, uiCodePage, dwETOFlags)
 *
 *	@mfunc
 *		Patch around the Win95 FE bug and MetaFile problem.		
 *
 *	@rdesc
 *		Returns whatever ExtTextOut returns
 */
BOOL ReExtTextOutW(
    HDC			hdc,			//@parm handle to device context 
    int			xp,				//@parm x-coordinate of reference point 
    int			yp,				//@parm y-coordinate of reference point 
    UINT		fuOptions,		//@parm text-output options 
    CONST RECT *lprect,			//@parm optional clipping and/or opaquing rectangle 
    const WCHAR *lpwchString,	//@parm points to string 
    UINT		cchCount,		//@parm number of characters in string 
    CONST INT *	lpDx,			//@parm Ptr to array of intercharacter spacing values
	UINT		uiCodePage,		//@parm CodePage for converting to Ansi
	DWORD		dwETOFlags)		//@parm ExtTextOut flags
{
	// This is a portion of Word code adapted for our needs. 
	// This is a work around for Win95FE bugs that cause GPFs in GDI if multiple
	// characters above Unicode 0x7F are passed to ExtTextOutW.

	// Also, when uiCodePage is non-zero, we want to output all characters using
	// ExtTextOutA - each character at a time.

	Assert(lpDx);
	int		cch = cchCount;
	const	WCHAR *lpwchT = lpwchString;
	const	WCHAR *lpwchStart = lpwchT;

	CONST int *lpdxpCur;
	BOOL	fRet = 0;

	if (lpDx)
	{
		const	WCHAR *lpwchEnd = lpwchString + cchCount;
		while (lpwchT < lpwchEnd)
		{
			// Characters less than 0x007F do not need special treatment
			// we output then in contiguous runs
			if (*lpwchT > 0x007F || uiCodePage)
			{
				if ((cch = lpwchT - lpwchStart) > 0)
				{
					lpdxpCur = lpDx + (lpwchStart - lpwchString);

					// Output the run of chars less than 0x7F
					fRet = ExtTextOutW(hdc, xp, yp, fuOptions, lprect, lpwchStart, cch, lpdxpCur);
					if (!fRet)
						return fRet;

					fuOptions &= ~ETO_OPAQUE; // Don't erase multiple times!!!

					// Advance cch
					AdvanceXpYp(xp, yp, cch, &lpdxpCur, dwETOFlags & 3);

					lpwchStart = lpwchT;
				}

				// Output chars above 0x7F one at a time to prevent Win95 FE GPF
				lpdxpCur = lpDx + (lpwchStart - lpwchString);
				if (uiCodePage)
				{

					// Need to convert to Ansi and use ExtTextOutA
					char	chAnsi[2];

					int	cbConv = WideCharToMultiByte(uiCodePage, 0, lpwchStart, 1,
						chAnsi, 2, NULL, NULL);

					if (cbConv <= 0)
					{
						chAnsi[0] = '?';
						cbConv = 1;
					}

					fRet = ExtTextOutA(hdc, xp, yp, fuOptions, lprect, chAnsi, cbConv, lpdxpCur);
				}
				else
					fRet = ExtTextOutW(hdc, xp, yp, fuOptions, lprect, lpwchStart, 1, lpdxpCur);

				if (!fRet)
					return fRet;

				fuOptions &= ~ETO_OPAQUE; // Don't erase multiple times!!!

				// Advance 1 character
				AdvanceXpYp(xp, yp, 1, &lpdxpCur, dwETOFlags & 3);

				lpwchStart++;
			}

			lpwchT++;
		}
		cch = lpwchT - lpwchStart;		// Setup to output the final run;
	}

	// If we were called with cchCount == 0, make a call here to erase the rectangle
	if (cch > 0 || !cchCount)
		fRet = ExtTextOutW(hdc, xp, yp, fuOptions, lprect, lpwchStart, cch, lpDx ? lpDx + (lpwchStart - lpwchString) : NULL);
	
	return fRet;	
}

extern ICustomTextOut *g_pcto;

void WINAPI CW32System::REExtTextOut(
	CONVERTMODE cm,
	UINT		uiCodePage,
	HDC			hdc,
	int			x,
	int			y,
	UINT		fuOptions,
	CONST RECT *lprc,
	const WCHAR *lpString,
	UINT		cch,
	CONST INT *	lpDx,
	DWORD		dwETOFlags)
{
	bool	fConvert = false;
	BOOL	fForceGdiFont = FALSE;
	HFONT	hfont = NULL, hfontCur = NULL;

	if (dwETOFlags & fETOCustomTextOut)
	{
		// If custom ExtTextOutW fails, then call OS to handle it
		if (g_pcto->ExtTextOutW(hdc, x, y, fuOptions, lprc, lpString, cch, lpDx))
			return;
	}

	// In order to get the EURO character to print, we need to force the
	// printer to use the glyphs inside GDI
	if(lpString[0] == EURO && 
	   (GetDeviceCaps(hdc, TECHNOLOGY) != DT_RASDISPLAY || W32->IsEnhancedMetafileDC(hdc)))
	{
		fForceGdiFont = TRUE;
		hfontCur = SelectFont(hdc, GetStockObject(ANSI_VAR_FONT));
		LOGFONT lf;
		GetObject(hfontCur, sizeof(LOGFONT), &lf);
		lf.lfOutPrecision = OUT_TT_ONLY_PRECIS;
		hfont = CreateFontIndirect(&lf);
		SelectObject(hdc, hfont);
	}

	if (fConvert || (!(dwETOFlags & fETOFEFontOnNonFEWin9x)) && cm != CVT_NONE)		
	{
		if (cm == CVT_WCTMB)
		{
			if (IsFECodePage(uiCodePage))
			{
				{
					// On NonFE NT4 and Win98, we need to textout each char using
					// ExtTextOutA
					ReExtTextOutW(hdc, x, y, fuOptions, lprc, lpString, cch, lpDx, uiCodePage, dwETOFlags);
					goto LExit;
				}
			}			
		}

		// Need to convert and use ExtTextOutA
		CTempCharBuf tcb;
		CTempBuf	 tDx;
		
		// Double the buffer size 
		int cbString = (cm == CVT_LOWBYTE) ? cch : cch * 2;
			
		// String buffer for converted string - allocate on the stack 
		char *psz = tcb.GetBuf(cbString);
		INT	 *pTempDx = NULL;
		
		if (NULL == psz)
		{
			// Could not allocate buffer
			goto LExit;
		}
			
		int cbConv = 0;
			
		if(cm == CVT_WCTMB)
		{
			cbConv = WideCharToMultiByte(uiCodePage, 0, lpString, cch, 
				psz, cbString, NULL, NULL);
				
			if(!cbConv)
			{
				// The conversion failed for one reason or another.  We should
				// make every effort to use WCTMB before we fall back to
				// taking the low-byte of every wchar (below), otherwise we
				// risk dropping the high-bytes and displaying garbage.
					
				// Use the cpg from the font, since the uiCodePage passed is
				//	the requested codepage and the font-mapper may very well
				//	have mapped to a different one.
				TEXTMETRIC tm;
				if(GetTextMetrics(hdc, &tm) && tm.tmCharSet != DEFAULT_CHARSET)
				{
					UINT cpg = CodePageFromCharRep(CharRepFromCharSet(tm.tmCharSet));
					uiCodePage = (uiCodePage != cpg) ? cpg : 1252;
				}
				else
					uiCodePage = 1252;

				cbConv = WideCharToMultiByte(uiCodePage, 0, lpString, cch, 
					psz, cbString, NULL, NULL);
			}

			if (cbConv > 0 && lpDx)
			{
				pTempDx = (INT *)tDx.GetBuf(cbConv * sizeof(INT));
				
				if (pTempDx)
				{
					// Repack lpDx to handle DBC
					INT		*pDx = pTempDx;
					CONST INT*pInputDx = lpDx;
					char	*pTempChar = psz;
					INT		cNumBytes = cbConv;

					while (cNumBytes > 0)
					{
						cNumBytes--;

						if (GetTrailBytesCount(*pTempChar++, uiCodePage))
						{
							*pDx++ = *pInputDx++;						
							*pDx++ = 0;
							cNumBytes--;
							pTempChar++;
						}
						else
							*pDx++ = *pInputDx++;
					}
				}
			}
		}
		else
		{
			Assert(cm == CVT_LOWBYTE);
			// drop through and convert using only low-bytes of WCHAR's
		}
			
			// WCTMB failed OR cm == CVT_LOWBYTE
		if(!cbConv)							// Convert WCHARs to CHARs
		{									
			// FUTURE:  We come here for both SYMBOL_CHARSET fonts and for
			// DBCS bytes stuffed into wchar's (one byte per wchar) when
			// the requested code page is not installed on the machine and
			// the MBTWC fails. Instead, we could have another conversion
			// mode that collects each DBCS char as a single wchar and then
			// remaps to a DBCS string for ExtTextOutA. This would allow us
			// to display text if the system has the right font even tho it
			// doesn't have the right cpg.
				
			// If we are converting this WCHAR buffer in this manner
			// (by taking only the low-byte's of the WCHAR's), it is 
			// because:
			// 	1) cm == CVT_LOWBYTE
			//	2) WCTMB above failed for some reason or another.  It may
			// 		be the case that the string is entirely ASCII in which
			//		case dropping the high-bytes is not a big deal (otherwise
			//		we assert).
		
			cbConv = cch;
				
			while(cch--)
			{
#ifdef DEBUG
				if (uiCodePage != CP_SYMBOL && lpString[cch] > 0xFF)
					Tracef(TRCSEVWARN, "Non-zero high-byte WCHAR: %x", lpString[cch]);
#endif
				psz[cch] = lpString[cch];
			}
		}		
		::ExtTextOutA(hdc, x, y, fuOptions, lprc, psz, cbConv, pTempDx ? pTempDx : lpDx);
		goto LExit;
	}

	if (dwETOFlags & fETOFEFontOnNonFEWin9x)
		ReExtTextOutW(hdc, x, y, fuOptions, lprc, lpString, cch, lpDx, 0, dwETOFlags);
	else
		::ExtTextOutW(hdc, x, y, fuOptions, lprc, lpString, cch, lpDx);	

LExit:
	if (fForceGdiFont)
	{
		SelectObject(hdc, hfontCur);
		SideAssert(DeleteObject(hfont));
	}
}

void WINAPI CW32System::REGetCharWidth(
	HDC		hdc,
	WCHAR	ch,
	INT		*pdxp,
	UINT	cpg,	
	BOOL	fCustomTextOut)
{
	BOOL	fForceGdiFont = FALSE;
	HFONT	hfont = NULL, hfontCur = NULL;
	*pdxp = 0;

	if (fCustomTextOut && g_pcto->GetCharWidthW(hdc, ch, ch, pdxp))
		return;

	if ((cpg == CP_SYMBOL || ch <= 127) && ::GetCharWidthA(hdc, ch, ch, pdxp))
		goto Done;

	if(ch == EURO && (GetDeviceCaps(hdc, TECHNOLOGY) != DT_RASDISPLAY || W32->IsEnhancedMetafileDC(hdc)))
	{
		fForceGdiFont = TRUE;
		hfontCur = SelectFont(hdc, GetStockObject(ANSI_VAR_FONT));
		LOGFONT lf;
		GetObject(hfontCur, sizeof(LOGFONT), &lf);
		lf.lfOutPrecision = OUT_TT_ONLY_PRECIS;
		hfont = CreateFontIndirect(&lf);
		SelectObject(hdc, hfont);
	}

	// For most workarounds, we will use the workarounds on all the OSs.
	// We only use specific workaround is MBTWC conversion is needed.

	// This is a workaround for Win95 FE bugs
	// FUTURE (keithcu) This logic could be simplified.
	if (!IN_RANGE(0x80, ch, 0xFF) &&			// Not high ANSI?
		(cpg == CP_CHINESE_TRAD || cpg == CP_CHINESE_SIM))	// Chinese CodePage?
	{
		int numOfDBCS = 0;
		::GetCharWidthW(hdc, 0x4e00, 0x4e00, pdxp);
		if (IN_RANGE(0x3400, ch, 0x9FFF))
			goto Done;
			
		// Use WCTMB heuristic
		char	ansiChar[2];
		BOOL	bDefCharUsed = FALSE;
		numOfDBCS = ::WideCharToMultiByte(cpg, 0, &ch, 1, ansiChar, 2, NULL, &bDefCharUsed);

		WORD wDBCS = (BYTE)ansiChar[0];
		
		if (2 == numOfDBCS)
			wDBCS = (BYTE)ansiChar[0] << 8 | (BYTE)ansiChar[1];
		
		if (numOfDBCS > 0 && ::GetCharWidthA( hdc, wDBCS, wDBCS, pdxp))
			goto Done;
	}

	//Win '95 GPFs if you pass in 0xFFFF
	if (ch == 0xFFFF)
		ch = 0xFFFE;

	::GetCharWidthW(hdc, ch, ch, pdxp);

	if(fForceGdiFont)
	{
		SelectObject(hdc, hfontCur);
		SideAssert(DeleteObject(hfont));
	}

Done:
	return;
}

//For now, we only fetch the ANSI_CHARSET kerning pairs and convert them to unicode
DWORD WINAPI CW32System::GetKerningPairs(HDC hdc, DWORD ckp, KERNINGPAIR *pkp)
{
#ifdef _WIN32
	ckp = GetKerningPairsA(hdc, ckp, pkp);

	if (pkp) //Can be called with pkp == NULL to find out number of pairs
	{
		for (DWORD ikp = 0; ikp < ckp; ikp++)
		{
			if (pkp[ikp].wFirst > 127 || pkp[ikp].wSecond > 127)
			{
				CHAR rgch[2];
				rgch[0] = pkp[ikp].wFirst;
				rgch[1] = pkp[ikp].wSecond;

				MBTWC(1252, 0, rgch, 2, (LPWSTR)&pkp[ikp].wFirst, 2, NULL);
			}
		}
	}
	return ckp;
#else
	return 0;
#endif//_WIN32
}

BOOL WINAPI CW32System::IsEnhancedMetafileDC( HDC hDC )
{
#ifdef _WIN32
	BOOL	fEMFDC = FALSE;
	DWORD	dwObjectType;
	
	dwObjectType = ::GetObjectType( hDC );

	if ( OBJ_ENHMETADC == dwObjectType || OBJ_ENHMETAFILE == dwObjectType )
		fEMFDC = TRUE;
	else if (OBJ_DC == dwObjectType )
	{
		// HACK Alert,  Enhanced Metafile DC does not support any Escape function
		// and should return 0.
		int	iEscapeFuction = QUERYESCSUPPORT;

		if ( Escape( hDC, QUERYESCSUPPORT, sizeof(int), (LPCSTR)&iEscapeFuction, NULL) == 0 )
			fEMFDC = TRUE;
	}	

	return fEMFDC;
#else
	return FALSE;
#endif//_WIN32
}


HPALETTE WINAPI CW32System::ManagePalette(
	HDC hdc,
	CONST LOGPALETTE *plogpal,
	HPALETTE &hpalOld,
	HPALETTE &hpalNew
)
{
#ifdef _WIN32
	if (hpalNew == NULL)
	{
		hpalNew = ::CreatePalette(plogpal);
		if (hpalNew != NULL)
		{
			hpalOld = ::SelectPalette(hdc, hpalNew, TRUE);
			::RealizePalette(hdc);
		}
	}
	else
	{
		// A new palette was created previously and we are restoring the old one
		::SelectPalette(hdc, hpalOld, TRUE);
		::RealizePalette(hdc);
		DeleteObject(hpalNew);
		hpalNew = NULL;
	}
	return hpalNew;
#else
	return NULL;
#endif//_WIN32
}

BOOL WINAPI CW32System::WinLPtoDP(HDC hdc, LPPOINT lppoints, int nCount)
{
    return ::LPtoDP(hdc, lppoints, nCount);
}

BOOL WINAPI CW32System::WinDPtoLP(HDC hdc, LPPOINT lppoints, int nCount)
{
    return ::DPtoLP(hdc, lppoints, nCount);
}

LONG WINAPI CW32System::WvsprintfA( LONG cbBuf, LPSTR szBuf, LPCSTR szFmt, va_list arglist )
{
	LONG cb;
	cb = ::vsprintf( szBuf, szFmt, arglist );
	Assert(cb < cbBuf);
	return cb;
}

int WINAPI CW32System::MulDivFunc(int nNumber, int nNumerator, int nDenominator)
{
	if ((nNumerator && nNumerator == nDenominator) || (nDenominator && !nNumber))
		return nNumber;
	return ::MulDiv(nNumber, nNumerator, nDenominator);
}

/*
 *	GetFacePriorityCharSet(WCHAR* szFaceName)
 *
 *	@func
 *		return charset *really* supported by given facename
 */
#ifdef _WIN32
int CALLBACK GetFacePriCharSetProc (
	ENUMLOGFONTEX	*lpelfe,
	NEWTEXTMETRIC	*lpntm,
	int				iFontType,
	LPARAM			lParam)
{
	Assert (lParam);
	*(BYTE*)lParam = lpntm->tmCharSet;
	return 0;
}

void CW32System::GetFacePriCharSet(HDC hdc, LOGFONT* plf)
{
	::EnumFontFamiliesEx(hdc, plf, (FONTENUMPROC)GetFacePriCharSetProc, (LPARAM)&plf->lfCharSet, 0);
}
#else
void CW32System::GetFacePriCharSet(HDC hdc, LOGFONT* plf){}
#endif//_WIN32

/*
 *	CW32System::ReadRegDigitSubstitionMode(void)
 *
 *	@mfunc
 *		Get the digit substitution mode (available on BiDi/Thai platforms)
 *
 *	@rdesc
 *		0 - Context (digit shape follows preceding run or CHARFORMAT's charset
 *		1 - None (digits always show as European digit shape)
 *		2 - National (digits always show as user locale's native shape)
 */
BYTE CW32System::ReadRegDigitSubstitutionMode()
{
	BYTE	bDigitMode = 1;
#ifndef NOCOMPLEXSCRIPTS
	HKEY	hk;
	DWORD	keyDataType;
	DWORD	dwDataSize;
	BYTE	rgbValue[2];

	bDigitMode = DIGITS_NOTIMPL;			// assume "Not Implemented"

	// Perform platform check before reading registry
	if (IsComplexScriptLcid(GetThreadLocale()))
	{
		if(RegOpenKeyExA(HKEY_CURRENT_USER,
						"Control Panel\\International",
						0,		// reserved
						KEY_QUERY_VALUE,
						&hk) == ERROR_SUCCESS)
		{
			dwDataSize = 2;
			if (RegQueryValueExA(hk,
								"NumShape",
								NULL,		// reserved
								&keyDataType,
								(LPBYTE) &rgbValue,
								&dwDataSize) == ERROR_SUCCESS)
			{
				if (rgbValue[0] > 0x2f)
					bDigitMode = rgbValue[0] - 0x30 + 1;
				if (bDigitMode > DIGITS_NATIONAL)
					bDigitMode = DIGITS_NONE;
			}
			RegCloseKey(hk);
		}
	}
#endif
	return bDigitMode;
}

#ifdef DEBUG
/*
 *	TestGetCharFlags125x(iFirst, iLast)
 *
 *	@func
 *		Unit test function for GetCharFlags125x(). Assert if GetCharFlags125x()
 *		claims that any char in	Unicode range iFirst thru iLast 1) should
 *		roundtrip multibyte conversion using a codepage in the range 1250-1258
 *		when it	doesn't, or 2) shouldn't roundtrip when it does.
 */
BOOL TestGetCharFlags125x(
	int iFirst, 
	int iLast) 
{
	LONG	cch  = iLast - iFirst + 1;
	LONG	i;
	Assert(cch <= 0x700 - 0xA0);

	char 	rgach[0x700 - 0xA0];
	WCHAR	rgch [0x700 - 0xA0];
	WCHAR *	pch;

	for(i = iFirst, pch = rgch; i <= iLast;  pch++, i++)
		*pch = (WCHAR)i;

	for(int CodePage = 1250; CodePage <= 1258; CodePage++)
	{
		if(cch != WideCharToMultiByte(CodePage, 0, rgch, cch, rgach, cch, "\0", NULL) ||
		   cch != MultiByteToWideChar(CodePage, 0, rgach, cch, rgch, cch))
		{
			continue;				// Missing code page
		}
		//							 1250 1251 1252 1253  1254  1255  1256  1257   1258
		const static WORD rgMask[] = {0x2, 0x4, 0x1, 0x8, 0x10, 0x20, 0x40, 0x80, 0x100};

		DWORD dwMask = rgMask[CodePage - 1250] << 8;

		for(i = iFirst, pch = rgch; i <= iLast; pch++, i++)
		{
			AssertSz(!((*pch != (WCHAR)i) ^ !(W32->GetCharFlags125x(i) & dwMask)),
				"GetCharFlags125x() failure");
			*pch = (WCHAR)i;	// Restore value
		}
	}
	return TRUE;				// Above AssertSz() reports any errors
}
#endif

#ifndef NODRAFTMODE
bool CW32System::GetDraftModeFontInfo(
	SHORT &iFont,
	SHORT &yHeight,
	QWORD &qwFontSig,
	COLORREF &crColor
)
{
#ifdef _WIN32
	// Update information if necessary
	if (_draftModeFontInfo._iFont == 0)
	{
		NONCLIENTMETRICSA ncm;
		ncm.cbSize = sizeof(NONCLIENTMETRICSA);
		// Using A version for Win9x compatibility
		if (SystemParametersInfoA(SPI_GETNONCLIENTMETRICS,sizeof(NONCLIENTMETRICS), &ncm, 0))
		{
			CStrInW strinwFontName(ncm.lfMessageFont.lfFaceName, CP_ACP);
			_draftModeFontInfo._iFont = GetFontNameIndex((WCHAR *)strinwFontName);
			_draftModeFontInfo._yHeight = (ncm.lfMessageFont.lfHeight * LY_PER_INCH) / W32->GetYPerInchScreenDC();
			if(_draftModeFontInfo._yHeight <= 0)
				_draftModeFontInfo._yHeight = -_draftModeFontInfo._yHeight;
			if (GetFontSignatureFromFace(_draftModeFontInfo._iFont, &_draftModeFontInfo._qwFontSig) == 0)
				_draftModeFontInfo._qwFontSig = 0;		// Trouble
		}
		_draftModeFontInfo._crTextColor = ::GetSysColor(COLOR_WINDOWTEXT);
	}

	// Handle bad draft mode font
	if (_draftModeFontInfo._qwFontSig == 0)
		return false;

	// Normal return
	iFont = _draftModeFontInfo._iFont;
	yHeight = _draftModeFontInfo._yHeight;
	qwFontSig = _draftModeFontInfo._qwFontSig;
	crColor = _draftModeFontInfo._crTextColor;

	return true;
#else
	return false;
#endif//_WIN32
}
#endif


/*
 *	CW32System::InitSysParams(fUpdate)
 *
 *	@mfunc
 *		This method is used to initialize certain system wide parameters that
 *      that are used in richedit.  This can also be used as an update method
 *      if we ever handle system parameter change notifications.  The update
 *		parameter id provided for this purpose.	Also note that if we ever support
 *		SysParam updating, we may have to protect access with locks.
 */
void CW32System::InitSysParams(BOOL fUpdate)
{
	TRACEBEGIN(TRCSUBSYSUTIL, TRCSCOPEINTERN, "CW32System::InitSysParams");
	CLock lock;

	if (!_fSysParamsOk || fUpdate)
	{
		_fSysParamsOk = TRUE;
		
		const LONG dxSelBarDefaultSize = 8;
		HDC hdc = GetScreenDC();
		HFONT hfontOld;
		TEXTMETRIC tm;

		_xPerInchScreenDC = GetDeviceCaps(hdc, LOGPIXELSX); 
		if (_xPerInchScreenDC == 0)
			_xPerInchScreenDC = 0x60;
		_yPerInchScreenDC = GetDeviceCaps(hdc, LOGPIXELSY);
		if (_yPerInchScreenDC == 0)
			_yPerInchScreenDC = 0x60;
		int cPalette = GetDeviceCaps(hdc, SIZEPALETTE);

		// 256 colors is where we seem to need to use the palette.
		if (256 == cPalette)
		{
			_fUsePalette = TRUE;
		}

		// calculate a himetric selection bar for the window's host.
		_dxSelBar = W32->DeviceToHimetric(dxSelBarDefaultSize, _xPerInchScreenDC);

		RefreshKeyboardLayout();
		_hSystemFont = (HFONT)GetStockObject(SYSTEM_FONT);
		hfontOld = SelectFont(hdc, _hSystemFont);
		if(hfontOld)
		{
			W32->GetTextMetrics(hdc, &tm);
			_dupSystemFont = tm.tmAveCharWidth;
			_dvpSystemFont = tm.tmHeight;
			_ySysFontLeading = tm.tmInternalLeading;
			_bCharSetSys = tm.tmCharSet;

			SelectFont(hdc, hfontOld);
		}

		_nScrollInset = (WORD)GetProfileIntA("windows", "ScrollInset",
			DD_DEFSCROLLINSET);
		_nDragDelay = (WORD)GetProfileIntA("windows", "DragDelay",
			DD_DEFDRAGDELAY);
		_nDragMinDist = (WORD)GetProfileIntA("windows", "DragMinDist",
			DD_DEFDRAGMINDIST);
		_nScrollDelay = (WORD)GetProfileIntA("windows", "ScrollDelay",
			DD_DEFSCROLLDELAY);
		_nScrollInterval = (WORD)GetProfileIntA("windows", "ScrollInterval",
			DD_DEFSCROLLINTERVAL);
	    _nScrollVAmount = (WORD)(GetYPerInchScreenDC()*DEFSCROLLVAMOUNT)/100;
	    _nScrollHAmount = (GetXPerInchScreenDC()*DEFSCROLLHAMOUNT)/100;

		_cxBorder	= GetSystemMetrics(SM_CXBORDER);	// Unsizable window border
		_cyBorder	= GetSystemMetrics(SM_CYBORDER);	//  widths
		_cxVScroll	= GetSystemMetrics(SM_CXVSCROLL);	//  dimensions
		_cyHScroll	= GetSystemMetrics(SM_CYHSCROLL);	//

		_cxDoubleClk	= GetSystemMetrics(SM_CXDOUBLECLK);
		_cyDoubleClk	= GetSystemMetrics(SM_CYDOUBLECLK);
		_DCT			= GetDoubleClickTime();
		_sysiniflags	= ::GetProfileIntA("richedit30", "flags", 0);

#ifdef DEBUG
		if(OnWinNT5() && (_sysiniflags & SYSINI_DEBUGGCF125X))
		{
			TestGetCharFlags125x(0xA0, 0x6FF);
			TestGetCharFlags125x(0x2000, 0x2122);
		}
#endif

		_bDigitSubstMode = ReadRegDigitSubstitutionMode();
#ifndef NOCOMPLEXSCRIPTS
		if (g_pusp)
			g_pusp->ApplyDigitSubstitution(_bDigitSubstMode);
#endif

#ifndef NODRAFTMODE
		_draftModeFontInfo._iFont = 0;
#endif
	}
}

/*
 *	CW32System:GetRollerLineScrollCount()
 *
 *	@mfunc	returns the number of lines to scroll with a roller mouse wheel.
 *			-1 means scroll by pages
 *
 *	@devnote We have to do different things for different platforms; NT4.0 has
 *			built in support for this functionality.
 */

/* excerpt from new winuser.h for calls to SystemParametersInfo */
#ifndef SPI_GETWHEELSCROLLLINES
#define SPI_GETWHEELSCROLLLINES   104
#endif

LONG CW32System::GetRollerLineScrollCount()
{
	if( _cLineScroll == 0 )
	{
		SystemParametersInfoA(SPI_GETWHEELSCROLLLINES, 0, &_cLineScroll, 0);
	}

	return _cLineScroll;
}


//+---------------------------------------------------------------------------
//
//  Member:     CStrIn::CStrIn
//
//  Synopsis:   Inits the class.
//
//  NOTE:       Don't inline these functions or you'll increase code size
//              by pushing -1 on the stack for each call.
//
//----------------------------------------------------------------------------

CStrIn::CStrIn(LPCWSTR pwstr, UINT CodePage)
{
	TRACEBEGIN(TRCSUBSYSWRAP, TRCSCOPEINTERN, "CStrIn::CStrIn");

    Init(pwstr, -1, CodePage);
}

CStrIn::CStrIn(LPCWSTR pwstr, int cwch, UINT CodePage)
{
	TRACEBEGIN(TRCSUBSYSWRAP, TRCSCOPEINTERN, "CStrIn::CStrIn");

    Init(pwstr, cwch, CodePage);
}


//+---------------------------------------------------------------------------
//
//  Member:     CStrIn::Init
//
//  Synopsis:   Converts a LPWSTR function argument to a LPSTR.
//
//  Arguments:  [pwstr] -- The function argument.  May be NULL or an atom
//                              (HIWORD(pwstr) == 0).
//
//              [cwch]  -- The number of characters in the string to
//                          convert.  If -1, the string is assumed to be
//                          NULL terminated and its length is calculated.
//
//  Modifies:   [this]
//
//----------------------------------------------------------------------------

void
CStrIn::Init(
	LPCWSTR pwstr,
	int		cwch,
	UINT	CodePage)	//@parm Code page to use (CP_ACP is default)
{
	TRACEBEGIN(TRCSUBSYSWRAP, TRCSCOPEINTERN, "CStrIn::Init");

    int cchBufReq;

    _cchLen = 0;

    // Check if string is NULL or an atom.
    if (HIWORD((DWORD_PTR)pwstr) == 0)
    {
        _pstr = (LPSTR) pwstr;
        return;
    }

    Assert(cwch == -1 || cwch > 0);

    //
    // Convert string to preallocated buffer, and return if successful.
    //

    _cchLen = W32->MbcsFromUnicode(_ach, ARRAY_SIZE(_ach), pwstr, cwch, CodePage);

    if (_cchLen > 0)
    {
        if(_ach[_cchLen-1] == 0)
            _cchLen--;                // account for terminator
        _pstr = _ach;
        return;
    }

    //
    // Alloc space on heap for buffer.
    //

    TRACEINFOSZ("CStrIn: Allocating buffer for wrapped function argument.");

    cchBufReq = WideCharToMultiByte(
            CP_ACP, 0, pwstr, cwch, NULL, 0,  NULL, NULL);

    Assert(cchBufReq > 0);
    _pstr = new char[cchBufReq];
    if (!_pstr)
    {
        // On failure, the argument will point to the empty string.
        TRACEINFOSZ("CStrIn: No heap space for wrapped function argument.");
        _ach[0] = 0;
        _pstr = _ach;
        return;
    }

    Assert(HIWORD((DWORD_PTR)_pstr));
    _cchLen = -1 + W32->MbcsFromUnicode(_pstr, cchBufReq, pwstr, cwch);

    Assert(_cchLen >= 0);
}



//+---------------------------------------------------------------------------
//
//  Class:      CStrInMulti (CStrIn)
//
//  Purpose:    Converts multiple strings which are terminated by two NULLs,
//              e.g. "Foo\0Bar\0\0"
//
//----------------------------------------------------------------------------

class CStrInMulti : public CStrIn
{
public:
    CStrInMulti(LPCWSTR pwstr, UINT CodePage);
};



//+---------------------------------------------------------------------------
//
//  Member:     CStrInMulti::CStrInMulti
//
//  Synopsis:   Converts mulitple LPWSTRs to a multiple LPSTRs.
//
//  Arguments:  [pwstr] -- The strings to convert.
//
//  Modifies:   [this]
//
//----------------------------------------------------------------------------

CStrInMulti::CStrInMulti(
	LPCWSTR pwstr,
	UINT CodePage)
{
	TRACEBEGIN(TRCSUBSYSWRAP, TRCSCOPEINTERN, "CStrInMulti::CStrInMulti");

    LPCWSTR pwstrT;

    // We don't handle atoms because we don't need to.
    Assert(HIWORD((DWORD_PTR)pwstr));

    //
    // Count number of characters to convert.
    //

    pwstrT = pwstr;
    if (pwstr)
    {
        do {
            while (*pwstrT++)
                ;

        } while (*pwstrT++);
    }

    Init(pwstr, pwstrT - pwstr, CodePage);
}

//+---------------------------------------------------------------------------
//
//  Member:     CStrOut::CStrOut
//
//  Synopsis:   Allocates enough space for an out buffer.
//
//  Arguments:  [pwstr]   -- The Unicode buffer to convert to when destroyed.
//                              May be NULL.
//
//              [cwchBuf] -- The size of the buffer in characters.
//
//  Modifies:   [this].
//
//----------------------------------------------------------------------------

CStrOut::CStrOut(LPWSTR pwstr, int cwchBuf)
{
	TRACEBEGIN(TRCSUBSYSWRAP, TRCSCOPEINTERN, "CStrOut::CStrOut");

    Assert(cwchBuf >= 0);

    _pwstr = pwstr;
    _cwchBuf = cwchBuf;

    if (!pwstr)
    {
        Assert(cwchBuf == 0);
        _pstr = NULL;
        return;
    }

    Assert(HIWORD((DWORD_PTR)pwstr));

    // Initialize buffer in case Windows API returns an error.
    _ach[0] = 0;

    // Use preallocated buffer if big enough.
    if (cwchBuf * 2 <= ARRAY_SIZE(_ach))
    {
        _pstr = _ach;
        return;
    }

    // Allocate buffer.
    TRACEINFOSZ("CStrOut: Allocating buffer for wrapped function argument.");
    _pstr = new char[cwchBuf * 2];
    if (!_pstr)
    {
        //
        // On failure, the argument will point to a zero-sized buffer initialized
        // to the empty string.  This should cause the Windows API to fail.
        //

        TRACEINFOSZ("CStrOut: No heap space for wrapped function argument.");
        Assert(cwchBuf > 0);
        _pwstr[0] = 0;
        _cwchBuf = 0;
        _pstr = _ach;
        return;
    }

    Assert(HIWORD((DWORD_PTR)_pstr));
    _pstr[0] = 0;
}



//+---------------------------------------------------------------------------
//
//  Member:     CStrOut::Convert
//
//  Synopsis:   Converts the buffer from MBCS to Unicode.
//
//----------------------------------------------------------------------------

int
CStrOut::Convert()
{
	TRACEBEGIN(TRCSUBSYSWRAP, TRCSCOPEINTERN, "CStrOut::Convert");

    int cch;

    if (!_pstr)
        return 0;

    cch = MultiByteToWideChar(CP_ACP, 0, _pstr, -1, _pwstr, _cwchBuf);
    Assert(cch > 0 || _cwchBuf == 0);

    Free();

	if (cch > 0 && cch <= _cwchBuf && _pwstr[cch-1] == L'\0')
		cch--;

    return cch;
}



//+---------------------------------------------------------------------------
//
//  Member:     CStrOut::~CStrOut
//
//  Synopsis:   Converts the buffer from MBCS to Unicode.
//
//  Note:       Don't inline this function, or you'll increase code size as
//              both Convert() and CConvertStr::~CConvertStr will be called
//              inline.
//
//----------------------------------------------------------------------------

CStrOut::~CStrOut()
{
	TRACEBEGIN(TRCSUBSYSWRAP, TRCSCOPEINTERN, "CStrOut::~CStrOut");

    Convert();
}


//
//	MultiByte --> UNICODE routins
//

//+---------------------------------------------------------------------------
//
//  Member:     CConvertStr::Free
//
//  Synopsis:   Frees string if alloc'd and initializes to NULL.
//
//----------------------------------------------------------------------------

void
CConvertStr::Free()
{
	TRACEBEGIN(TRCSUBSYSWRAP, TRCSCOPEINTERN, "CConvertStr::Free");

    if (_pstr != _ach && HIWORD((DWORD_PTR)_pstr) != 0)
    {
        delete [] _pstr;
    }

    _pstr = NULL;
}

//+---------------------------------------------------------------------------
//
//  Member:     CConvertStrW::Free
//
//  Synopsis:   Frees string if alloc'd and initializes to NULL.
//
//----------------------------------------------------------------------------

void
CConvertStrW::Free()
{
	TRACEBEGIN(TRCSUBSYSWRAP, TRCSCOPEINTERN, "CConvertStrW::Free");

    if (_pwstr != _awch && HIWORD((DWORD_PTR)_pwstr) != 0 )
    {
        delete [] _pwstr;
    }

    _pwstr = NULL;
}



//+---------------------------------------------------------------------------
//
//  Member:     CStrInW::CStrInW
//
//  Synopsis:   Inits the class.
//
//  NOTE:       Don't inline these functions or you'll increase code size
//              by pushing -1 on the stack for each call.
//
//----------------------------------------------------------------------------

CStrInW::CStrInW(LPCSTR pstr)
{
	TRACEBEGIN(TRCSUBSYSWRAP, TRCSCOPEINTERN, "CStrInW::CStrInW");

    Init(pstr, -1, CP_ACP);
}

CStrInW::CStrInW(LPCSTR pstr, UINT uiCodePage)
{
	TRACEBEGIN(TRCSUBSYSWRAP, TRCSCOPEINTERN, "CStrInW::CStrInW");

    Init(pstr, -1, uiCodePage);
}

CStrInW::CStrInW(LPCSTR pstr, int cch, UINT uiCodePage)
{
	TRACEBEGIN(TRCSUBSYSWRAP, TRCSCOPEINTERN, "CStrInW::CStrInW");

    Init(pstr, cch, uiCodePage);
}


//+---------------------------------------------------------------------------
//
//  Member:     CStrInW::Init
//
//  Synopsis:   Converts a LPSTR function argument to a LPWSTR.
//
//  Arguments:  [pstr] -- The function argument.  May be NULL or an atom
//                              (HIWORD(pwstr) == 0).
//
//              [cch]  -- The number of characters in the string to
//                          convert.  If -1, the string is assumed to be
//                          NULL terminated and its length is calculated.
//
//  Modifies:   [this]
//
//----------------------------------------------------------------------------

void
CStrInW::Init(LPCSTR pstr, int cch, UINT uiCodePage)
{
	TRACEBEGIN(TRCSUBSYSWRAP, TRCSCOPEINTERN, "CStrInW::Init");

    int cchBufReq;

    _cwchLen = 0;

    // Check if string is NULL or an atom.
    if (HIWORD((DWORD_PTR)pstr) == 0)
    {
        _pwstr = (LPWSTR) pstr;
        return;
    }

    Assert(cch == -1 || cch > 0);

    //
    // Convert string to preallocated buffer, and return if successful.
    //

    _cwchLen = MultiByteToWideChar(
            uiCodePage, 0, pstr, cch, _awch, ARRAY_SIZE(_awch));

    if (_cwchLen > 0)
    {
        if(_awch[_cwchLen-1] == 0)
            _cwchLen--;                // account for terminator
        _pwstr = _awch;
        return;
    }

    //
    // Alloc space on heap for buffer.
    //

    TRACEINFOSZ("CStrInW: Allocating buffer for wrapped function argument.");

    cchBufReq = MultiByteToWideChar(
            CP_ACP, 0, pstr, cch, NULL, 0);

    Assert(cchBufReq > 0);
    _pwstr = new WCHAR[cchBufReq];
    if (!_pwstr)
    {
        // On failure, the argument will point to the empty string.
        TRACEINFOSZ("CStrInW: No heap space for wrapped function argument.");
        _awch[0] = 0;
        _pwstr = _awch;
        return;
    }

    Assert(HIWORD((DWORD_PTR)_pwstr));
    _cwchLen = -1 + MultiByteToWideChar(
            uiCodePage, 0, pstr, cch, _pwstr, cchBufReq);
    Assert(_cwchLen >= 0);
}


//+---------------------------------------------------------------------------
//
//  Member:     CStrOutW::CStrOutW
//
//  Synopsis:   Allocates enough space for an out buffer.
//
//  Arguments:  [pstr]   -- The ansi buffer to convert to when destroyed.
//                              May be NULL.
//
//              [cchBuf] -- The size of the buffer in characters.
//
//  Modifies:   [this].
//
//----------------------------------------------------------------------------

CStrOutW::CStrOutW(LPSTR pstr, int cchBuf, UINT uiCodePage)
{
	TRACEBEGIN(TRCSUBSYSWRAP, TRCSCOPEINTERN, "CStrOutW::CStrOutW");

    Assert(cchBuf >= 0);

    _pstr = pstr;
    _cchBuf = cchBuf;
	_uiCodePage = uiCodePage;

    if (!pstr)
    {
        Assert(cchBuf == 0);
        _pwstr = NULL;
        return;
    }

    Assert(HIWORD((DWORD_PTR)pstr));

    // Initialize buffer in case Windows API returns an error.
    _awch[0] = 0;

    // Use preallocated buffer if big enough.
    if (cchBuf <= ARRAY_SIZE(_awch))
    {
        _pwstr = _awch;
        return;
    }

    // Allocate buffer.
    TRACEINFOSZ("CStrOutW: Allocating buffer for wrapped function argument.");
    _pwstr = new WCHAR[cchBuf * 2];
    if (!_pwstr)
    {
        //
        // On failure, the argument will point to a zero-sized buffer initialized
        // to the empty string.  This should cause the Windows API to fail.
        //

        TRACEINFOSZ("CStrOutW: No heap space for wrapped function argument.");
        Assert(cchBuf > 0);
        _pstr[0] = 0;
        _cchBuf = 0;
        _pwstr = _awch;
        return;
    }

    Assert(HIWORD((DWORD_PTR)_pwstr));
    _pwstr[0] = 0;
}



//+---------------------------------------------------------------------------
//
//  Member:     CStrOutW::Convert
//
//  Synopsis:   Converts the buffer from Unicode to MBCS
//
//----------------------------------------------------------------------------

int
CStrOutW::Convert()
{
	TRACEBEGIN(TRCSUBSYSWRAP, TRCSCOPEINTERN, "CStrOutW::Convert");

    int cch;

    if (!_pwstr)
        return 0;

	int cchBuf = _cchBuf;

	cch = W32->MbcsFromUnicode(_pstr, cchBuf, _pwstr, -1, _uiCodePage);

    Free();

	if (cch > 0 && cch <= _cchBuf && _pstr[cch-1] == '\0')
		cch--;

    return cch;
}



//+---------------------------------------------------------------------------
//
//  Member:     CStrOutW::~CStrOutW
//
//  Synopsis:   Converts the buffer from Unicode to MBCS.
//
//  Note:       Don't inline this function, or you'll increase code size as
//              both Convert() and CConvertStr::~CConvertStr will be called
//              inline.
//
//----------------------------------------------------------------------------

CStrOutW::~CStrOutW()
{
	TRACEBEGIN(TRCSUBSYSWRAP, TRCSCOPEINTERN, "CStrOutW::~CStrOutW");

    Convert();
}

BOOL CW32System::GetVersion(
	DWORD *pdwPlatformId,
	DWORD *pdwMajorVersion,
	DWORD *pdwMinorVersion
)
{
#ifdef _WIN32
	OSVERSIONINFOA osv;
	osv.dwOSVersionInfoSize = sizeof(OSVERSIONINFOA);
	if(pdwPlatformId) *pdwPlatformId = 0;
	*pdwMajorVersion = 0;
	if (::GetVersionExA(&osv))
	{
		if(pdwPlatformId) *pdwPlatformId = osv.dwPlatformId;
		*pdwMajorVersion = osv.dwMajorVersion;
		*pdwMinorVersion = osv.dwMinorVersion;
		return TRUE;
	}
#endif//_WIN32
	return FALSE;
}

BOOL CW32System::GetStringTypes(
	LCID	lcid,
	LPCTSTR rgch,
	int		cch,
	LPWORD	lpCharType1,
	LPWORD	lpCharType3)
{
	int ich;
	for (ich = 0; ich < cch; ich++)
	{
		if (rgch[ich] <= 0xFF)
		{
			lpCharType1[ich] = rgctype1Ansi[rgch[ich]];
			lpCharType3[ich] = rgctype3Ansi[rgch[ich]];
		}
		else
			break;
	}
	if (ich == cch)
		return TRUE;
#ifdef UNICODE
	if (::GetStringTypeExW(lcid, CT_CTYPE1, rgch, cch, lpCharType1))
		return ::GetStringTypeExW(lcid, CT_CTYPE3, rgch, cch, lpCharType3);
#else
	if (::GetStringTypeExA(lcid, CT_CTYPE1, rgch, cch, lpCharType1))
		return ::GetStringTypeExA(lcid, CT_CTYPE3, rgch, cch, lpCharType3);
#endif//UNICODE
	return FALSE;
}

BOOL WINAPI CW32System::GetStringTypeEx(
	LCID	lcid,
	DWORD	dwInfoType,
	LPCTSTR rgch,
	int		cch,
	LPWORD	lpCharType
)
{
#ifdef UNICODE
	return ::GetStringTypeExW(lcid, dwInfoType, rgch, cch, lpCharType);
#else
	return ::GetStringTypeExA(lcid, dwInfoType, rgch, cch, lpCharType);
#endif//UNICODE
}

LPWSTR WINAPI CW32System::CharLower(LPWSTR pwstr)
{
	return ::CharLowerW(pwstr);
}

DWORD WINAPI CW32System::CharLowerBuff(LPWSTR pwstr, DWORD cchLength)
{
	return ::CharLowerBuffW(pwstr, cchLength);
}

DWORD WINAPI CW32System::CharUpperBuff(LPWSTR pwstr, DWORD cchLength)
{
	return ::CharUpperBuffW(pwstr, cchLength);
}

HDC WINAPI CW32System::CreateIC(
        LPCWSTR             lpszDriver,
        LPCWSTR             lpszDevice,
        LPCWSTR             lpszOutput,
        CONST DEVMODEW *    lpInitData)
{
	return ::CreateICW(lpszDriver,lpszDevice,lpszOutput,lpInitData);
}

HANDLE WINAPI CW32System::CreateFile(
	LPCWSTR                 lpFileName,
	DWORD                   dwDesiredAccess,
	DWORD                   dwShareMode,
	LPSECURITY_ATTRIBUTES   lpSecurityAttributes,
	DWORD                   dwCreationDisposition,
	DWORD                   dwFlagsAndAttributes,
	HANDLE                  hTemplateFile
)
{
	return ::CreateFileW(lpFileName,
		dwDesiredAccess,
		dwShareMode,
		lpSecurityAttributes,
		dwCreationDisposition,
		dwFlagsAndAttributes,
		hTemplateFile);
}

HFONT WINAPI CW32System::CreateFontIndirect(CONST LOGFONTW * plfw)
{
	return ::CreateFontIndirectW(plfw);
}

int WINAPI CW32System::CompareString ( 
	LCID  Locale,			// locale identifier 
	DWORD  dwCmpFlags,		// comparison-style options 
	LPCWSTR  lpString1,		// pointer to first string 
	int  cch1,			// size, in bytes or characters, of first string 
	LPCWSTR  lpString2,		// pointer to second string 
	int  cch2 			// size, in bytes or characters, of second string  
)
{
	return ::CompareStringW(Locale, dwCmpFlags, lpString1, cch1, lpString2, cch2);
}

LRESULT WINAPI CW32System::DefWindowProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
#ifdef _WIN32
	return ::DefWindowProcW(hWnd, msg, wParam, lParam);
#else
	return ::DefWindowProc(hWnd, msg, wParam, lParam);
#endif
}

int WINAPI CW32System::GetObject(HGDIOBJ hgdiObj, int cbBuffer, LPVOID lpvObj)
{
	return ::GetObjectW(hgdiObj, cbBuffer, lpvObj);
}

DWORD WINAPI CW32System::GetProfileSection(
	LPCWSTR lpAppName,
	LPWSTR lpReturnedString,
	DWORD nSize
)
{
	return ::GetProfileSectionW(lpAppName, lpReturnedString, nSize);
}

int WINAPI CW32System::GetTextFace(
        HDC    hdc,
        int    cch,
        LPWSTR lpFaceName
)
{
	return ::GetTextFaceW(hdc, cch, lpFaceName);
}

BOOL WINAPI CW32System::GetTextMetrics(HDC hdc, LPTEXTMETRICW lptm)
{
	return ::GetTextMetricsW(hdc, lptm);
}

LONG WINAPI CW32System::GetWindowLong(HWND hWnd, int nIndex)
{
	return ::GetWindowLongW(hWnd, nIndex);
}

LONG_PTR WINAPI CW32System::GetWindowLongPtr(HWND hWnd, int nIndex)
{
	return ::GetWindowLongPtrW(hWnd, nIndex);
}

DWORD WINAPI CW32System::GetClassLong(HWND hWnd, int nIndex)
{
	return ::GetClassLongW(hWnd, nIndex);
}

HBITMAP WINAPI CW32System::LoadBitmap(HINSTANCE hInstance, LPCWSTR lpBitmapName)
{
#ifdef _WIN32
	return ::LoadBitmapW(hInstance, lpBitmapName);
#else
	return 0;
#endif//_WIN32
}

HCURSOR WINAPI CW32System::LoadCursor(HINSTANCE hInstance, LPCWSTR lpCursorName)
{
	return ::LoadCursorW(hInstance, lpCursorName);
}

HINSTANCE WINAPI CW32System::LoadLibrary(LPCWSTR lpLibFileName)
{
	return ::LoadLibraryW(lpLibFileName);
}

LRESULT WINAPI CW32System::SendMessage(
	HWND    hWnd,
    UINT    Msg,
    WPARAM  wParam,
    LPARAM  lParam
)
{
	TRACEBEGIN(TRCSUBSYSWRAP, TRCSCOPEINTERN, "SendMessage");
	if (::IsWindowUnicode(hWnd))
		return ::SendMessageW(hWnd, Msg, wParam, lParam);

	// We never need Ansi to Unicode translation in our use of SendMessage
	// Our list boxes always use Unicode.
    switch (Msg)
    {
	// We don't want to translate these!
	// case LB_ADDSTRING:
    // case LB_INSERTSTRING:
    // case CB_ADDSTRING:
    // case CB_SELECTSTRING:
    // case CB_INSERTSTRING:
    // case LB_GETTEXT:
    // case CB_GETLBTEXT:

    case WM_GETTEXT:
		{
		   CStrOut str((LPWSTR)lParam, (int)wParam);
			::SendMessageA(hWnd, Msg, str.BufSize(), (LPARAM)(LPSTR)str);
			return str.Convert();
		}
		break;

    case WM_SETTEXT:
    case EM_REPLACESEL:
        Assert(FALSE);		// We never send these.  Dead code?
		break;

    case EM_SETPASSWORDCHAR:
        Assert(FALSE);		// We never send these.  Dead code?
		break;
    }
	return ::SendMessageA(hWnd, Msg, wParam, lParam);
}

LONG WINAPI CW32System::SetWindowLong(HWND hWnd, int nIndex, LONG dwNewLong)
{
	return ::SetWindowLongW(hWnd, nIndex, dwNewLong);
}

LONG WINAPI CW32System::SetWindowLongPtr(HWND hWnd, int nIndex, LONG_PTR dwNewLong)
{
	return ::SetWindowLongPtrW(hWnd, nIndex, dwNewLong);
}

BOOL WINAPI CW32System::PostMessage(
	HWND    hWnd,
    UINT    Msg,
    WPARAM  wParam,
    LPARAM  lParam
)
{
	return ::PostMessageW(hWnd, Msg, wParam, lParam);
}

BOOL WINAPI CW32System::UnregisterClass(LPCWSTR lpClassName, HINSTANCE hInstance)
{
	return ::UnregisterClassW(lpClassName, hInstance);
}

int WINAPI CW32System::lstrcmpi(LPCWSTR lpString1, LPCWSTR lpString2)
{
	return wcsicmp(lpString1, lpString2);
}

BOOL WINAPI CW32System::PeekMessage(
	LPMSG   lpMsg,
    HWND    hWnd,
    UINT    wMsgFilterMin,
    UINT    wMsgFilterMax,
    UINT    wRemoveMsg
)
{
#ifdef _WIN32
	return ::PeekMessageW(lpMsg, hWnd, wMsgFilterMin, wMsgFilterMax, wRemoveMsg);
#else
	return ::PeekMessage(lpMsg, hWnd, wMsgFilterMin, wMsgFilterMax, wRemoveMsg);
#endif
}

DWORD WINAPI CW32System::GetModuleFileName(
	HMODULE hModule,
	LPWSTR lpFilename,
	DWORD nSize
)
{
	return ::GetModuleFileNameW(hModule, lpFilename, nSize);
}


// The high bits of _yHeightUI & _yHeightOther are being used to check if
// font is installed in the system
#define NEED_TO_CHECK_FONT 0x080
struct PreferredFontInfo
{
	BYTE _bPitchAndFamilyUI;
	BYTE _yHeightUI;
	SHORT _iFontUI;
	BYTE _bPitchAndFamilyOther;
	BYTE _yHeightOther;
	SHORT _iFontOther;
};

PreferredFontInfo g_pfinfo[NCHARREPERTOIRES];

void CW32System::InitPreferredFontInfo()
{
	// For UI case, we will use Word9 UI fonts
	// For Non-UI case, we will use Word9 default email fonts
	
	short iFont;
	short iFontPlane2;

	UINT  uSysDefCodePage = GetSystemDefaultCodePage();
	CLock lock;

	// Japanese Init
	static const WCHAR lpUIJapanFontName[] = L"MS UI Gothic";
	static const WCHAR lpOthJapanFontName[]
		= {0xFF2D,0xFF33,0x0020,0xFF30,0x30B4,0x30B7,0x30C3,0x30AF, 0};
	static const WCHAR lpOthJapanFontNameEUC[] = L"MS PGothic";

	iFont = GetFontNameIndex( lpUIJapanFontName );
	SetPreferredFontInfo( SHIFTJIS_INDEX, true, iFont, 9 | NEED_TO_CHECK_FONT, 17 );
	if (uSysDefCodePage == CP_JAPAN)
		iFont = GetFontNameIndex( lpOthJapanFontName );
	else
		iFont = GetFontNameIndex( lpOthJapanFontNameEUC );
	SetPreferredFontInfo( SHIFTJIS_INDEX, false, iFont, 10 | NEED_TO_CHECK_FONT, 17 );

	// Korean Init
	static const WCHAR lpUIKoreanFontName[] = {0xAD74, 0xB9BC, 0};
	static const WCHAR lpUIKoreanFontNameEUC[] = L"Gulim";
	
	if (uSysDefCodePage == CP_KOREAN)
		iFont = GetFontNameIndex( lpUIKoreanFontName );		
	else
		iFont = GetFontNameIndex( lpUIKoreanFontNameEUC );

	SetPreferredFontInfo( HANGUL_INDEX, true, iFont, 9 | NEED_TO_CHECK_FONT, 49 );
	SetPreferredFontInfo( HANGUL_INDEX, false, iFont, 10 | NEED_TO_CHECK_FONT, 49 );

	// Traditional Chinese Init
	static const WCHAR lpUITChineseFontName[]	= {0x65B0, 0x7D30, 0x660E, 0x9AD4, 0};
	static const WCHAR lpUITChineseFontNameEUC[] = L"PMingLiU";
	iFont = GetFontNameIndex(uSysDefCodePage == CP_CHINESE_TRAD
							 ? lpUITChineseFontName : lpUITChineseFontNameEUC);
	iFontPlane2 = iFont;

	SetPreferredFontInfo( BIG5_INDEX, true, iFont, 9 | NEED_TO_CHECK_FONT, 54 );
	SetPreferredFontInfo( BIG5_INDEX, false, iFont, 10 | NEED_TO_CHECK_FONT, 54 );

	// Simplified Chinese Init
	static const WCHAR lpUISChineseFontName[] = {0x5B8B, 0x4F53, 0};
	static const WCHAR lpUISChineseFontNameEUC[] = L"SimSun";
	iFont = GetFontNameIndex(uSysDefCodePage == CP_CHINESE_SIM
							 ? lpUISChineseFontName : lpUISChineseFontNameEUC);
	SetPreferredFontInfo( GB2312_INDEX, true,  iFont,  9 | NEED_TO_CHECK_FONT, 54 );
	SetPreferredFontInfo( GB2312_INDEX, false, iFont, 10 | NEED_TO_CHECK_FONT, 54 );

	// English, EastEurope, and Viet Init
	iFont = GetFontNameIndex( szTahoma );
	SetPreferredFontInfo(ANSI_INDEX, true, iFont, 8, DEFAULT_PITCH | FF_SWISS );
	SetPreferredFontInfo(VIET_INDEX, true, iFont, 8, DEFAULT_PITCH | FF_SWISS );
	iFont = GetFontNameIndex( szArial );
	SetPreferredFontInfo(ANSI_INDEX, false, iFont, 10, DEFAULT_PITCH | FF_SWISS );
	SetPreferredFontInfo(EASTEUROPE_INDEX, false, iFont, 10, DEFAULT_PITCH | FF_SWISS );
	SetPreferredFontInfo(EASTEUROPE_INDEX, true, iFont, 8, DEFAULT_PITCH | FF_SWISS );
	SetPreferredFontInfo(VIET_INDEX, false, iFont, 10, DEFAULT_PITCH | FF_SWISS );

	// SYMBOL_CHARSET
	iFont = GetFontNameIndex( szWingdings );
	SetPreferredFontInfo(SYMBOL_INDEX, true,  iFont, 8,  DEFAULT_PITCH | FF_DONTCARE);
	SetPreferredFontInfo(SYMBOL_INDEX, false, iFont, 10, DEFAULT_PITCH | FF_DONTCARE);

	iFont = GetFontNameIndex( szTahoma );

	SetPreferredFontInfo(THAI_INDEX, true,  iFont,  8, DEFAULT_PITCH | FF_SWISS );
	iFont = GetFontNameIndex( szCordiaNew );
	SetPreferredFontInfo(THAI_INDEX, false, iFont, 14, DEFAULT_PITCH | FF_SWISS );

	iFont = GetFontNameIndex( szShruti );
	SetPreferredFontInfo(GUJARATI_INDEX, true,  iFont,  8 | NEED_TO_CHECK_FONT, DEFAULT_PITCH | FF_SWISS );
	SetPreferredFontInfo(GUJARATI_INDEX, false, iFont, 10 | NEED_TO_CHECK_FONT, DEFAULT_PITCH | FF_SWISS );

	iFont = GetFontNameIndex( szTunga );
	SetPreferredFontInfo(KANNADA_INDEX, true,  iFont,  8 | NEED_TO_CHECK_FONT, DEFAULT_PITCH | FF_SWISS );
	SetPreferredFontInfo(KANNADA_INDEX, false, iFont, 10 | NEED_TO_CHECK_FONT, DEFAULT_PITCH | FF_SWISS );

	iFont = GetFontNameIndex( szGautami );
	SetPreferredFontInfo(TELUGU_INDEX, true,  iFont,  8 | NEED_TO_CHECK_FONT, DEFAULT_PITCH | FF_SWISS );
	SetPreferredFontInfo(TELUGU_INDEX, false, iFont, 10 | NEED_TO_CHECK_FONT, DEFAULT_PITCH | FF_SWISS );

	// Devanagari Init
	iFont = GetFontNameIndex( szMangal );
	SetPreferredFontInfo(DEVANAGARI_INDEX, true,  iFont,  8 | NEED_TO_CHECK_FONT, DEFAULT_PITCH | FF_SWISS );
	SetPreferredFontInfo(DEVANAGARI_INDEX, false, iFont, 10 | NEED_TO_CHECK_FONT, DEFAULT_PITCH | FF_SWISS );

	// Tamil Init
	iFont = GetFontNameIndex( szLatha );
	SetPreferredFontInfo(TAMIL_INDEX, true,  iFont,  8 | NEED_TO_CHECK_FONT, DEFAULT_PITCH | FF_SWISS );
	SetPreferredFontInfo(TAMIL_INDEX, false, iFont, 10 | NEED_TO_CHECK_FONT, DEFAULT_PITCH | FF_SWISS );

	// Gurmukhi Init
	iFont = GetFontNameIndex( szRaavi );
	SetPreferredFontInfo(GURMUKHI_INDEX, true,  iFont,  8 | NEED_TO_CHECK_FONT, DEFAULT_PITCH | FF_SWISS );
	SetPreferredFontInfo(GURMUKHI_INDEX, false, iFont, 10 | NEED_TO_CHECK_FONT, DEFAULT_PITCH | FF_SWISS );

	_fFEFontInfo = FEDATA_NOT_INIT;

	// Syriac Init
	iFont = GetFontNameIndex( szSyriac );
	SetPreferredFontInfo(SYRIAC_INDEX, true,  iFont,  8 | NEED_TO_CHECK_FONT, DEFAULT_PITCH | FF_SWISS );
	SetPreferredFontInfo(SYRIAC_INDEX, false, iFont, 10 | NEED_TO_CHECK_FONT, DEFAULT_PITCH | FF_SWISS );

	// Thaana init
	iFont = GetFontNameIndex( szThaana );
	SetPreferredFontInfo(THAANA_INDEX, true,  iFont,  8 | NEED_TO_CHECK_FONT, DEFAULT_PITCH | FF_SWISS );
	SetPreferredFontInfo(THAANA_INDEX, false, iFont, 10 | NEED_TO_CHECK_FONT, DEFAULT_PITCH | FF_SWISS );

	{
		static const WCHAR lpCHSPlane2FontNameEUC[] = L"Simsun (Founder Extended)";
		static const WCHAR lpCHSPlane2FontName[] =
			{0x5B8B, 0x4F53, 0x2D, 0x65B9, 0x6B63, 0x8D85, 0x5927, 0x5B57, 0x7B26, 0x96C6, 0};

		iFont = GetFontNameIndex(uSysDefCodePage == CP_CHINESE_SIM
					 ? lpCHSPlane2FontName : lpCHSPlane2FontNameEUC);
	}

	for(LONG i = JPN2_INDEX; i < JPN2_INDEX + 4; i++)
	{
		SetPreferredFontInfo(i, true, i == CHS2_INDEX ? iFont : iFontPlane2,  9 | NEED_TO_CHECK_FONT, DEFAULT_PITCH | FF_SWISS);
		SetPreferredFontInfo(i, false, i == CHS2_INDEX ? iFont : iFontPlane2, 10 | NEED_TO_CHECK_FONT, DEFAULT_PITCH | FF_SWISS);
	}
	CheckInstalledKeyboards();				// Check installed keyboard layouts
}

bool CW32System::IsDefaultFontDefined(
	LONG iCharRep,
	bool fUIFont,
	SHORT &iFont)
{
	if((unsigned)iCharRep >= NCHARREPERTOIRES)
		return false;

	if (!fUIFont && g_pfinfo[iCharRep]._yHeightOther)
	{
		iFont = g_pfinfo[iCharRep]._iFontOther;
		return true;
	}
	if (fUIFont && g_pfinfo[iCharRep]._yHeightUI)
	{
		iFont = g_pfinfo[iCharRep]._iFontUI;
		return true;
	}
	return false;
}

void CW32System::CheckInstalledKeyboards()
{
	HKL	 rghkl[MAX_HKLS];
    INT	 cLayouts = ::GetKeyboardLayoutList(MAX_HKLS, rghkl);		 	
	INT  iCharRep;

	for(INT i = 0; i < cLayouts; i++)
  	{
		iCharRep = CharRepFromLID(LOWORD(rghkl[i]));
 		if(iCharRep >= 0)
			SetPreferredKbd(iCharRep, rghkl[i]);
   	}
}

/*
 *	CW32System::SetPreferredFontInfo(iCharRep, fUIFont,	iFont, yHeight, bPitchAndFamily)
 *
 *	@mfunc
 *		Store font info preferred for iCharRep.
 *
 *	@rdesc
 *		true if success
 */
bool CW32System::SetPreferredFontInfo(
	int		iCharRep,
	bool	fUIFont,
	SHORT	iFont,
	BYTE	yHeight,
	BYTE	bPitchAndFamily)
{
	if((unsigned)iCharRep >= NCHARREPERTOIRES)
		return false;

	if (fUIFont)
	{
		g_pfinfo[iCharRep]._bPitchAndFamilyUI = bPitchAndFamily;

		if (yHeight)
			g_pfinfo[iCharRep]._yHeightUI = yHeight;
		else if (!g_pfinfo[iCharRep]._yHeightUI)
			g_pfinfo[iCharRep]._yHeightUI = 10;

		g_pfinfo[iCharRep]._iFontUI = iFont;
	}
	else
	{
		g_pfinfo[iCharRep]._bPitchAndFamilyOther = bPitchAndFamily;

		if (yHeight)
			g_pfinfo[iCharRep]._yHeightOther = yHeight;
		else if (!g_pfinfo[iCharRep]._yHeightOther)
			g_pfinfo[iCharRep]._yHeightOther = 10;

		g_pfinfo[iCharRep]._iFontOther = iFont;
	}
	SetFontLegitimateSize(iFont, fUIFont, yHeight & ~NEED_TO_CHECK_FONT,
						  IsFECharRep(iCharRep));
	SetFontSignature(iFont, FontSigFromCharRep(iCharRep));
	return true;
}

/*
 *	CW32System::GetPreferredFontInfo(iCharRep, fUIFont,	&iFont, &yHeight, &bPitchAndFamily)
 *
 *	@mfunc
 *		Return font info preferred for iCharRep.
 *
 *	@rdesc
 *		Preferred font info corresponding to iCharRep for doc/UI choice
 *		specified by fUIfont.
 */
bool CW32System::GetPreferredFontInfo(
	int		iCharRep,			//@parm Character Repertoire
	bool	fUIFont,			//@parm TRUE iff UI fonts should be used
	SHORT &	iFont,				//@parm Outparm for preferred font index
	BYTE &	yHeight,			//@parm Outparm for preferred font height
	BYTE &	bPitchAndFamily)	//@parm Outparm for preferred font pitch&family
{
	// Set reasonable values for failure case
	iFont = -1;
	yHeight = 0;
	bPitchAndFamily = 0;

	if((unsigned)iCharRep >= NCHARREPERTOIRES)
		return false;

	if (fUIFont)
	{
 		if(!g_pfinfo[iCharRep]._yHeightUI)
			iCharRep = 0;					// No entry.  Try default
		bPitchAndFamily = g_pfinfo[iCharRep]._bPitchAndFamilyUI;
		yHeight = g_pfinfo[iCharRep]._yHeightUI;
		iFont = g_pfinfo[iCharRep]._iFontUI;
	}
	else
	{
		if(!g_pfinfo[iCharRep]._yHeightOther)
			iCharRep = 0;					// No entry.  Try default
		bPitchAndFamily = g_pfinfo[iCharRep]._bPitchAndFamilyOther;
		yHeight = g_pfinfo[iCharRep]._yHeightOther;
		iFont = g_pfinfo[iCharRep]._iFontOther;
	}
	
	if (yHeight & NEED_TO_CHECK_FONT)
	{
		// Check if the preferred font is installed in the system.
		CLock lock;

		HDC hDC = GetScreenDC();

		// Turn off the checkfont bit
		yHeight &= ~NEED_TO_CHECK_FONT;

		if (fUIFont)
			g_pfinfo[iCharRep]._yHeightUI &= ~NEED_TO_CHECK_FONT;
		else
			g_pfinfo[iCharRep]._yHeightOther &= ~NEED_TO_CHECK_FONT;
		
		if (hDC)
		{
			const short *pFontIndex = fUIFont ? &g_pfinfo[iCharRep]._iFontUI : &g_pfinfo[iCharRep]._iFontOther;
			if (IsFontAvail( hDC, iCharRep, fUIFont, (short *)pFontIndex))
				iFont = *pFontIndex;
		}
	}
	return true;
}

/*
 *	CW32System::GetPreferredFontHeight(fUIFont,	iCharRepOrg, iCharRepNew, yHeightOrg)
 *
 *	@mfunc
 *		called when we need the default font size when changing from one charset to another.
 *
 *	@rdesc
 *		The preferred default font size in TWIP if the Original height is same as the
 *		original charset default font size.  Otherwise, it will return the Original height.
 */
SHORT CW32System::GetPreferredFontHeight(
	bool	fUIFont,
	BYTE	iCharRepOrg, 
	BYTE	iCharRepNew, 
	SHORT	yHeightOrg)
{
	// No entry, forget it
	if (iCharRepOrg == DEFAULT_INDEX || iCharRepNew == DEFAULT_INDEX)
		return yHeightOrg;

	BYTE yHeightOrgPreferred = fUIFont ? 
		g_pfinfo[iCharRepOrg]._yHeightUI : g_pfinfo[iCharRepOrg]._yHeightOther;

	yHeightOrgPreferred &= ~NEED_TO_CHECK_FONT;

	// Get New Preferred Height
	if(yHeightOrgPreferred && yHeightOrgPreferred == yHeightOrg/TWIPS_PER_POINT)
	{
		BYTE yHeightNew = (fUIFont ? 
			g_pfinfo[iCharRepNew]._yHeightUI : g_pfinfo[iCharRepNew]._yHeightOther);

		if (yHeightNew)
		{
			yHeightNew &= ~NEED_TO_CHECK_FONT;
			return yHeightNew * TWIPS_PER_POINT;
		}
	}
	return yHeightOrg;
}

/*
 *	CW32System::CheckInstalledFEFonts()
 *
 *	@mfunc
 *		called when building FE fonts installed and User default LCID info
 *
 *	@devnote
 *		This information is necessary when we want to classify Chinese characters
 *		and Full-width characters.
 */
void CW32System::CheckInstalledFEFonts()
{
	CLock	cLock;
	HDC		hDC = GetScreenDC();

	_fFEFontInfo = 0;
	
	LONG iCharRep = CharRepFromLID(::GetUserDefaultLCID());
	if(IsFECharRep(iCharRep))
	{
		switch (iCharRep)
		{
			case SHIFTJIS_INDEX:
				_fFEFontInfo |= (FEUSER_LCID | FEUSER_CP_JPN | JPN_FONT_AVAILABLE);
				break;

			case HANGUL_INDEX:
				_fFEFontInfo |= (FEUSER_LCID | FEUSER_CP_KOR | KOR_FONT_AVAILABLE);
				break;

			case BIG5_INDEX:
				_fFEFontInfo |= (FEUSER_LCID | FEUSER_CP_BIG5 | BIG5_FONT_AVAILABLE);
				break;

			case GB2312_INDEX:
				_fFEFontInfo |= (FEUSER_LCID | FEUSER_CP_GB | GB_FONT_AVAILABLE);
				break;
		}
	}
	if (hDC)
	{
		if (!(_fFEFontInfo & JPN_FONT_AVAILABLE) && IsFontAvail(hDC, SHIFTJIS_INDEX))
			_fFEFontInfo |= JPN_FONT_AVAILABLE;

		if (!(_fFEFontInfo & KOR_FONT_AVAILABLE) && IsFontAvail(hDC, HANGUL_INDEX))
			_fFEFontInfo |= KOR_FONT_AVAILABLE;

		if (!(_fFEFontInfo & BIG5_FONT_AVAILABLE) && IsFontAvail(hDC, BIG5_INDEX))
			_fFEFontInfo |= BIG5_FONT_AVAILABLE;

		if (!(_fFEFontInfo & GB_FONT_AVAILABLE) && IsFontAvail(hDC, GB2312_INDEX))
			_fFEFontInfo |= GB_FONT_AVAILABLE;
	}
}

#ifndef NOFEPROCESSING


/*
 *	CW32System::IsFEFontInSystem( cpg )
 *
 *	@mfunc
 *		check if there is any FE font installed for the given codepage
 *
 *	@devnote
 *		This information is necessary when we want to classify Chinese characters
 *		and Full-width characters.
 */
bool CW32System::IsFEFontInSystem(int cpg)
{
	int fFontExist = 0;

	if (_fFEFontInfo == FEDATA_NOT_INIT)
		CheckInstalledFEFonts();		// Look for FE fonts in system

	// Check if font for the codepage is in the system	
	switch (cpg)
	{
	case CP_JAPAN:
		fFontExist = _fFEFontInfo & JPN_FONT_AVAILABLE;
		break;
		
	case CP_KOREAN:
		fFontExist = _fFEFontInfo & KOR_FONT_AVAILABLE;
		break;

	case CP_CHINESE_TRAD:
		fFontExist = _fFEFontInfo & BIG5_FONT_AVAILABLE;
		break;

	case CP_CHINESE_SIM:
		fFontExist = _fFEFontInfo & GB_FONT_AVAILABLE;
		break;
	}
	
	return (fFontExist != 0);
}
/*
 *	CW32System::GetFEFontInfo( void  )
 *
 *
 *	@mfunc
 *		called when classifying Chinese characters and Full-width characters
 *
 *	@devnote
 *		Chinese characters and Full-width characters can be in any
 *		of the four FE codepages.  We want to classfy them according to the 
 *		User default LCID and which FE fonts are installed in the system.
 *
 *	@rdesc
 *		CharRep for the character.
 */
UINT CW32System::GetFEFontInfo()
{
	int	iDefUserCodepage = -1;

	if (_fFEFontInfo == FEDATA_NOT_INIT)
	{
		CLock	Lock;

		// Check if FE fonts are available in the system
		CheckInstalledFEFonts();
	}

	if (_fFEFontInfo & FEUSER_LCID)
		iDefUserCodepage = (_fFEFontInfo & FEUSER_CODEPAGE);

	if (iDefUserCodepage == FEUSER_CP_BIG5)
		return BIG5_INDEX;

	if (iDefUserCodepage == FEUSER_CP_GB)
		return GB2312_INDEX;	
	
	if (iDefUserCodepage == FEUSER_CP_JPN)
		return SHIFTJIS_INDEX;

	if (iDefUserCodepage == FEUSER_CP_KOR)		
		return HANGUL_INDEX;

	// Check which font is available and return the corresponding codepage
	// We check for Simplified Chinese first since it contains more Chinese
	// characters than Traditional Chinese.
	if (_fFEFontInfo & GB_FONT_AVAILABLE)
		return GB2312_INDEX;

	if (_fFEFontInfo & BIG5_FONT_AVAILABLE)
		return BIG5_INDEX;

	if (_fFEFontInfo & JPN_FONT_AVAILABLE)
		return SHIFTJIS_INDEX;

	if (_fFEFontInfo & KOR_FONT_AVAILABLE)
		return HANGUL_INDEX;

	return GB2312_INDEX;				// Well, no FE font, tough luck.
}
#endif//NOFEPROCESSING

#ifndef NOCOMPLEXSCRIPTS
/*
 *	CW32System::IsDiacriticOrKashida(ch, wC3Type)
 *
 *	@mfunc
 *		Return TRUE if ch or wC3Type reveals that ch is a nonspacing
 *		diacritic or a kashida. Because Win9x GetStringTypeExW isn't
 *		implemented, we use range checks for Win9x.
 *
 *	@rdesc
 *		True if ch or wC3Type reveals that ch is a nonspacing diacritic
 */
BOOL CW32System::IsDiacriticOrKashida(
	WCHAR ch,		//@parm On Win9x, check ranges for ch
	WORD  wC3Type)	//@parm On WinNT, use C3-type check
{
	if(!IN_RANGE(0x300, ch, 0xe50))		// Combining diacritics of interest
		return FALSE;					//  fall in this range

	return IN_RANGE(0x300, ch, 0x36F) || IsDiacritic(ch) || IsBiDiKashida(ch);
}
#endif

/*
 *	CW32System::IsDiacritic(ch)
 *
 *	@mfunc
 *		Return TRUE if ch falls in BiDi, Thai, Devanagari or Tamil diacritic range.
 */
BOOL CW32System::IsDiacritic(
	WCHAR ch)
{
	// BiDi
	if (IsBiDiDiacritic(ch))
		return TRUE;

	// Thai
	if (IN_RANGE(0xe31, ch, 0xe4e))
		return 	IN_RANGE(0x0e47, ch, 0x0e4e) || IN_RANGE(0x0e34, ch, 0x0e3a) || ch == 0x0e31;

	// Devanagari
	if (IN_RANGE(0x0902, ch, 0x0963))
		return 	IN_RANGE(0x0941, ch, 0x0948) || IN_RANGE(0x0951, ch, 0x0954) || ch == 0x094d ||		
				IN_RANGE(0x0962, ch, 0x0963) || IN_RANGE(0x0901, ch, 0x0902) || ch == 0x093c;

	// Tamil
	if (IN_RANGE(0x0b82, ch, 0x0bcd))
		return 	ch == 0x0bcd || ch == 0x0bc0 || ch == 0x0b82;

	return FALSE;
}

void CW32System::EraseTextOut(HDC hdc, const RECT *prc)
{
	::ExtTextOutA(hdc, 0, 0, ETO_OPAQUE, prc, NULL, 0, NULL);
}

int CW32System::GetTextCharsetInfo (
  HDC hdc,                // handle to device context
  LPFONTSIGNATURE lpSig,  // pointer to structure to receive data
  DWORD dwFlags           // reserved; must be zero
)
{
#ifdef _WIN32
	return ::GetTextCharsetInfo( hdc, lpSig, dwFlags );
#else
	return DEFAULT_CHARSET;
#endif
}

UINT WINAPI CW32System::SetTextAlign(HDC hdc, UINT uAlign)
{
	return ::SetTextAlign(hdc, uAlign);
}

BOOL WINAPI CW32System::InvertRect(HDC hdc, CONST RECT *prc)
{
    return ::InvertRect(hdc, prc);
}

HGLOBAL WINAPI CW32System::GlobalAlloc( UINT uFlags, DWORD dwBytes )
{
	return ::GlobalAlloc( uFlags, dwBytes );
}

HGLOBAL WINAPI CW32System::GlobalFree( HGLOBAL hMem )
{
	return hMem ? ::GlobalFree( hMem ) : NULL;
}

UINT WINAPI CW32System::GlobalFlags( HGLOBAL hMem )
{
	return ::GlobalFlags( hMem );
}

HGLOBAL WINAPI CW32System::GlobalReAlloc( HGLOBAL hMem, DWORD dwBytes, UINT uFlags )
{
	return ::GlobalReAlloc( hMem, dwBytes, uFlags );
}

DWORD WINAPI CW32System::GlobalSize( HGLOBAL hMem )
{
	return ::GlobalSize( hMem );
}

LPVOID WINAPI CW32System::GlobalLock( HGLOBAL hMem )
{
	return ::GlobalLock( hMem );
}

HGLOBAL WINAPI CW32System::GlobalHandle( LPCVOID pMem )
{
	return ::GlobalHandle( pMem );
}

BOOL WINAPI CW32System::GlobalUnlock( HGLOBAL hMem )
{
	return ::GlobalUnlock( hMem );
}

// Return TRUE if
// hWnd is the same as GetForegroundWindow() or
// hWnd's parent is the parent of GetForegroundWindow()
BOOL CW32System::IsForegroundFrame(
	HWND	hWnd)
{

	if (hWnd == NULL)
		return FALSE;

	HWND	hForeGround = ::GetForegroundWindow();

	if (hForeGround == NULL)
		return FALSE;

	if (hForeGround == hWnd)
		return TRUE;

	// Get the Parent hwnd
	for(;;)
	{
		HWND hParent = ::GetParent( hWnd );
		if( hParent == NULL )
			break;
		hWnd = hParent;
	}

	// Get the Parent hwnd of hForeGround
	for(;;)
	{
		HWND hParent = ::GetParent( hForeGround );
		if( hParent == NULL )
			break;
		hForeGround = hParent;
	}

	return (hForeGround == hWnd);
}

VOID CALLBACK TrackMouseTimerProc(
	HWND hWnd,
	UINT uMsg,
	UINT idEvent,
	DWORD dwTime)
{
	RECT rect;
	POINT pt;

	::GetClientRect(hWnd,&rect);
	::MapWindowPoints(hWnd,NULL,(LPPOINT)&rect,2);
	::GetCursorPos(&pt);
	if (!::PtInRect(&rect,pt) || (::WindowFromPoint(pt) != hWnd) || !(CW32System::IsForegroundFrame(hWnd)))
	{
		if (!KillTimer(hWnd,RETID_VISEFFECTS))
		{
			// Error killing the timer!
		}

		CW32System::PostMessage(hWnd,WM_MOUSELEAVE,0,0);
	}
}

BOOL CW32System::TrackMouseLeave(HWND hWnd)
{
	// REVIEW JMO Should use TrackMouseEvent on NT 4, 5, and Win98
	if (::SetTimer(hWnd, RETID_VISEFFECTS, 50, (TIMERPROC)TrackMouseTimerProc))
		return TRUE;
	return FALSE;
}

// Helper function for color mixing
COLORREF CrBlend2Colors(COLORREF cr1, int nPart1, COLORREF cr2, int nPart2)
{
	int r, g, b;
	int sum = nPart1 + nPart2;

	// NOTE: there is no reason why this function would NOT work with
	// Part1 = 5 and Part2 = 23, but we expect to be doing percents.  This
	// assert can come out when this is not longer the case.
	Assert(sum == 10 || sum == 100 || sum == 1000);

	// By adding on sum/2 before dividing by sum, we properly round the value,
	// rather than truncating it, while doing integer math.

	r = (nPart1 * GetRValue(cr1) + nPart2 * GetRValue(cr2) + sum / 2) / sum;
	g = (nPart1 * GetGValue(cr1) + nPart2 * GetGValue(cr2) + sum / 2) / sum;
	b = (nPart1 * GetBValue(cr1) + nPart2 * GetBValue(cr2) + sum / 2) / sum;

	Assert(r >= 0);
	Assert(r <= 255);
	Assert(g >= 0);
	Assert(g <= 255);
	Assert(b >= 0);
	Assert(b <= 255);
	
	return RGB(r, g, b);
}

COLORREF CW32System::GetCtlBorderColor(BOOL fMousedown, BOOL fMouseover)
{
	if (fMousedown || fMouseover)
		return ::GetSysColor(COLOR_HIGHLIGHT);
	return ::GetSysColor(COLOR_BTNSHADOW);
}

COLORREF CW32System::GetCtlBkgColor(BOOL fMousedown, BOOL fMouseover)
{
	if (fMousedown)
		return CrBlend2Colors(::GetSysColor(COLOR_HIGHLIGHT), 50, ::GetSysColor(COLOR_WINDOW), 50);
	if (fMouseover)
		return CrBlend2Colors(::GetSysColor(COLOR_HIGHLIGHT), 30, ::GetSysColor(COLOR_WINDOW), 70);
	return ::GetSysColor(COLOR_BTNFACE);
}
 
COLORREF CW32System::GetCtlTxtColor(BOOL fMousedown, BOOL fMouseover, BOOL fDisabled)
{
	if (fMousedown)
		return ::GetSysColor(COLOR_HIGHLIGHTTEXT);
	if (fMouseover)
		return ::GetSysColor(COLOR_BTNTEXT);
	if (fDisabled)
		return ::GetSysColor(COLOR_GRAYTEXT);
	return ::GetSysColor(COLOR_WINDOWTEXT);
}

void CW32System::DrawBorderedRectangle(
	HDC hdc,
	RECT *prc,
	COLORREF crBorder,
	COLORREF crBackground
)
{
	if (hdc == NULL || prc == NULL)
	{
		Assert(FALSE);
		return;
	}

	HBRUSH hbrBorder = CreateSolidBrush(crBorder);
	::FrameRect(hdc, prc, hbrBorder);
	::DeleteObject(hbrBorder);

	// Draw background
	HBRUSH hbrBkg = CreateSolidBrush(crBackground);
	RECT rc = *prc;
	::InflateRect(&rc, -1, -1);
	::FillRect(hdc, &rc, hbrBkg);
	::DeleteObject(hbrBkg);
}

void CW32System::DrawArrow(
	HDC hdc,
	RECT *prc,
	COLORREF crArrow
)
{
	/* 
	dxArrow represents the width of the scrollbar.  It's value can be
	set through the Display Properties Dialog (8-100) and can change based on
	the color scheme the user chooses.
	*/
	int dxArrow = ((DXOFPRECT(prc) >> 1) << 1);	

	// Height of the scrollbar button.
	int dyArrow = DYOFPRECT(prc);

	/* 
	Value used in Windows Standard Color Scheme (dxArrow 16 or 18).  
	*/
	int dx = 7;	
	
	// For all scrollbars of 10 or less, make dx 3.
	if (dxArrow <= 10)
		dx = 3;	
	else if (dxArrow == 12 || dxArrow == 14)
		dx = 5;
	else if (dxArrow >= 20) 
		dx = 9;
	
	// Current x coord
	int x = prc->left + (dxArrow / 2) - (dx / 2); 
	// Current y coord
	int y = prc->top + (dyArrow / 2) - ((dx + 1) / 4); 
	
	// Change for the rectangle in the loop.
	LONG dTop = 1; 
	LONG dBottom = 1;
	LONG dLeft = 1;
	LONG dRight = -1;
	
	COLORREF crBackSav = SetBkColor(hdc, crArrow);
	// Draw
	RECT rc = {x, y, x + dx, y + 1};
	
	while(rc.right > rc.left)
	{
		ExtTextOut(hdc, 0, 0, ETO_OPAQUE, &rc, NULL, 0, NULL);
		rc.right += dRight;
		rc.top += dTop;
		rc.left += dLeft;
		rc.bottom += dBottom;
	}
	SetBkColor(hdc, crBackSav);
}

/*
 *	CW32System::GetTmpDisplayAttrIdx(tmpDisplayAttr)
 *
 *	@mfunc
 *		This routine returns the idx for the given temp display attr.
 *	If the item is not found, a new item will be added to the array.
 *
 *	@rdesc
 *		idx number if success.  Otherwise, returns -1
 */
short CW32System::GetTmpDisplayAttrIdx(
	TMPDISPLAYATTR &tmpDisplayAttr)
{
	if (tmpDisplayAttr.wMask == 0 && !tmpDisplayAttr.bUnderlineType)
		return -1;			// No tmp display attr.

	if (!_arTmpDisplayAttrib)
	{
		_arTmpDisplayAttrib = new CTmpDisplayAttrArray();

		if (!_arTmpDisplayAttrib)
			return -1;
	}

	LONG	idxMax = _arTmpDisplayAttrib->Count();
	LONG	idxItem;
	TMPDISPLAYATTR* pDisplayAttr;

	for (idxItem= 0; idxItem < idxMax; idxItem++)
	{
		pDisplayAttr = _arTmpDisplayAttrib->Elem(idxItem);
		if (pDisplayAttr && !memcmp(&tmpDisplayAttr, pDisplayAttr, sizeof(TMPDISPLAYATTR)))
			return idxItem;		// Find it!
	}

	// Not found, add it to the array
	pDisplayAttr = _arTmpDisplayAttrib->Add(1, &idxItem);

	if (pDisplayAttr)
	{
		*pDisplayAttr = tmpDisplayAttr;
		return idxItem;
	}
	return -1;
}

/*
 *	CW32System::GetTmpColor(idx, crTmpColor, iAction)
 *
 *	@mfunc
 *		Retrieve color from the temp. display attribute given by the idx.
 *
 *	@rdesc
 *		true if success and crTmpColor contains the requested color
 */
bool CW32System::GetTmpColor(
	SHORT idx,
	COLORREF &crTmpColor,
	INT iAction)
{
	crTmpColor = (COLORREF)tomAutoColor;

	if (!_arTmpDisplayAttrib || idx < 0)
		return false;

	Assert(idx < _arTmpDisplayAttrib->Count());

	if (idx < _arTmpDisplayAttrib->Count())
	{
		TMPDISPLAYATTR *pTmpDisplay = _arTmpDisplayAttrib->Elem(idx);

		if (pTmpDisplay)
		{
			switch (iAction)
			{
			case GET_TEMP_TEXT_COLOR:
				if (pTmpDisplay->wMask & APPLY_TMP_FORECOLOR)
				{
					crTmpColor = pTmpDisplay->crTextColor;
					return true;
				}
				break;

			case GET_TEMP_BACK_COLOR:
				if (pTmpDisplay->wMask & APPLY_TMP_BACKCOLOR)
				{
					crTmpColor = pTmpDisplay->crBackColor;
					return true;
				}
				break;

			case GET_TEMP_UL_COLOR:
				crTmpColor = pTmpDisplay->crUnderlineColor;
				return true;
			}
		}
	}
	return false;
}

/*
 *	CW32System::GetTmpUnderline(idx)
 *
 *	@mfunc
 *		Retrieve the underline style idx.
 *
 *	@rdesc
 *		Underline style idx number if success.  Otherwise, returns 0
 */
short CW32System::GetTmpUnderline(
	SHORT idx)
{
	if (!_arTmpDisplayAttrib || idx < 0)
		return 0;		// No temp. underline style

	Assert(idx < _arTmpDisplayAttrib->Count());

	if (idx < _arTmpDisplayAttrib->Count())
	{
		TMPDISPLAYATTR *pTmpDisplay = _arTmpDisplayAttrib->Elem(idx);
		if (pTmpDisplay)
			return pTmpDisplay->bUnderlineType;
	}
	return 0;
}

BOOL DisableBackgroundBitMap()
{
	return FALSE;
}

#undef new
#ifdef _WIN32
#include "GDIPlus.h"
#endif//_WIN32

HBITMAP WINAPI CW32System::GetPictureBitmap(IStream *pstm)
{
	// Background bitmaps disabled for now.
	if (DisableBackgroundBitMap())
		return NULL;

	HBITMAP hbm = NULL;
#if 0
	Gdiplus::Status status;

	Gdiplus::Image image( pstm );
	unsigned int width = image.GetWidth();
	unsigned int height = image.GetHeight();

	Gdiplus::Graphics graphics( &image );
	Gdiplus::Bitmap bitmap(width, height, &graphics);
	Gdiplus::Graphics graphics2( &bitmap );
	status = graphics2.DrawImage(&image, Gdiplus::Point(0, 0));	
	if (status == Gdiplus::Ok)
	{
		Gdiplus::Color bkcolor;
		status = bitmap.GetHBITMAP( bkcolor, &hbm );
		if (status != Gdiplus::Ok)
			hbm = NULL;
	}
#endif
	return hbm;
}

HRESULT CW32System::OleSave ( LPPERSISTSTORAGE pPS, LPSTORAGE pstg, BOOL fSameAsLoad )
{
#ifdef _WIN32
	return ::OleSave(pPS, pstg, fSameAsLoad);
#else
	return E_NOTIMPL;
#endif
}

HRESULT CW32System::OleDraw ( IUnknown *punk, DWORD dwAspect, HDC hdc, LPCRECT prect )
{
#ifdef _WIN32
	return ::OleDraw(punk, dwAspect, hdc, prect);
#else
	return E_NOTIMPL;
#endif
}
HRESULT CW32System::OleLoad ( 
							 IStorage *pstg,
							 REFIID riid,
							 IOleClientSite *psite,
							 void **ppv
							 )
{
#ifdef _WIN32
	return ::OleLoad(pstg, riid, psite, ppv);
#else
	return E_NOTIMPL;
#endif
}

HRESULT CW32System::OleSetContainedObject ( IUnknown *punk, BOOL fContained )
{
#ifdef _WIN32
	return ::OleSetContainedObject(punk, fContained);
#else
	return E_NOTIMPL;
#endif
}

HRESULT CW32System::CoDisconnectObject ( IUnknown *punk, DWORD dwres )
{
#ifdef _WIN32
	return ::CoDisconnectObject(punk, dwres);
#else
	return E_NOTIMPL;
#endif
}

void CW32System::CoTaskMemFree ( LPVOID pv )
{
	::CoTaskMemFree(pv);
}

HRESULT CW32System::SetConvertStg ( IStorage *pstg, BOOL fConv )
{
#ifdef _WIN32
	return ::SetConvertStg(pstg, fConv);
#else
	return E_NOTIMPL;
#endif
}

HRESULT CW32System::WriteFmtUserTypeStg ( IStorage *pstg, CLIPFORMAT cf, LPOLESTR pstr)
{
#ifdef _WIN32
	return ::WriteFmtUserTypeStg(pstg, cf, pstr);
#else
	return E_NOTIMPL;
#endif
}

HRESULT CW32System::WriteClassStg ( IStorage *pstg, REFCLSID rid )
{
#ifdef _WIN32
	return ::WriteClassStg(pstg, rid);
#else
	return E_NOTIMPL;
#endif
}

HRESULT CW32System::ReadFmtUserTypeStg ( IStorage *pstg, CLIPFORMAT *pcf, LPOLESTR *pstr )
{
#ifdef _WIN32
	return ::ReadFmtUserTypeStg(pstg, pcf, pstr);
#else
	return E_NOTIMPL;
#endif
}

HRESULT CW32System::ReadClassStg ( IStorage *pstg, CLSID *pclsid )
{
#ifdef _WIN32
	return ::ReadClassStg(pstg, pclsid);
#else
	return E_NOTIMPL;
#endif
}

void CW32System::ReleaseStgMedium ( LPSTGMEDIUM pmedium )
{
	return ::ReleaseStgMedium(pmedium);
}

BSTR CW32System::SysAllocString ( const OLECHAR * sz )
{
	return ::SysAllocString(sz);
}

typedef RE_OLEAUTAPI_(BSTR, SASL_CAST)(const OLECHAR *, UINT);
BSTR CW32System::SysAllocStringLen ( const OLECHAR *pch, UINT cch )
{
	return ::SysAllocStringLen(pch, cch);
}

void CW32System::SysFreeString ( BSTR bstr )
{
	::SysFreeString(bstr);
}

UINT CW32System::SysStringLen ( BSTR bstr )
{
	return ::SysStringLen(bstr);
}

void CW32System::VariantInit ( VARIANTARG * pvarg )
{
	::VariantInit(pvarg);
}

void CW32System::VariantClear ( VARIANTARG * pvarg )
{
	::VariantClear(pvarg);
}

HRESULT CW32System::CreateBindCtx ( DWORD reserved, LPBC * ppbc )
{
#ifdef _WIN32
	return ::CreateBindCtx(reserved, ppbc);
#else
	return E_NOTIMPL;
#endif
}

HRESULT CW32System::OleRun ( IUnknown *punk )
{
#ifdef _WIN32
	return ::OleRun(punk);
#else
	return E_NOTIMPL;
#endif
}

HANDLE CW32System::OleDuplicateData ( HANDLE hSrc, CLIPFORMAT cfFormat, UINT uFlags )
{
#ifdef _WIN32
	return ::OleDuplicateData(hSrc, cfFormat, uFlags);
#else
	return 0;
#endif
}

HRESULT CW32System::StgCreateDocfileOnILockBytes (
	ILockBytes *plkbyt,
	DWORD grfmode,
	DWORD res,
	IStorage **ppstg
	)
{
#ifdef _WIN32
	return ::StgCreateDocfileOnILockBytes(plkbyt, grfmode, res, ppstg);
#else
	return E_NOTIMPL;
#endif
}

HRESULT CW32System::CreateILockBytesOnHGlobal ( HGLOBAL hGlobal, BOOL fDel, ILockBytes **pplkbyt )
{
#ifdef _WIN32
	return ::CreateILockBytesOnHGlobal(hGlobal, fDel, pplkbyt);
#else
	return E_NOTIMPL;
#endif
}

HRESULT CW32System::LoadTypeLib ( const OLECHAR *szfile, ITypeLib **pptlib )
{
#ifdef _WIN32
	return ::LoadTypeLib(szfile, pptlib);
#else
	return E_NOTIMPL;
#endif
}

HRESULT CW32System::LoadTypeLibEx ( const OLECHAR *szfile, REGKIND regkind, ITypeLib **pptlib )
{
#ifdef _WIN32
	return ::LoadTypeLibEx(szfile, regkind, pptlib);
#else
	return E_NOTIMPL;
#endif
}

HRESULT CW32System::OleCreateFromData (
									   LPDATAOBJECT pDataObj,
									   REFIID riid,
									   DWORD renderopt,
									   LPFORMATETC pfetc,
									   LPOLECLIENTSITE pClientSite,
									   LPSTORAGE pStg,
									   void **ppvObj
									   )
{
#ifdef _WIN32
	return ::OleCreateFromData(pDataObj, riid, renderopt, pfetc, pClientSite, pStg, ppvObj);
#else
	return E_NOTIMPL;
#endif
}

HRESULT CW32System::OleCreateLinkToFile( 
										LPCOLESTR pstr,
										REFIID rid,
										DWORD renderopt,
										LPFORMATETC pfetc,
										LPOLECLIENTSITE psite,
										LPSTORAGE pstg, 
										void **ppstg
										)
{
#ifdef _WIN32
	return ::OleCreateLinkToFile(pstr, rid, renderopt, pfetc, psite, pstg, ppstg);
#else
	return E_NOTIMPL;
#endif
}

LPVOID CW32System::CoTaskMemAlloc ( ULONG cb )
{
	return ::CoTaskMemAlloc(cb);
}

LPVOID CW32System::CoTaskMemRealloc ( LPVOID pv, ULONG cv)
{
	return ::CoTaskMemRealloc(pv, cv);
}


HRESULT CW32System::OleSetClipboard ( IDataObject *pdo )
{
	return ::OleSetClipboard(pdo);
}

HRESULT CW32System::OleFlushClipboard ( void )
{
	return ::OleFlushClipboard();
}

HRESULT CW32System::OleIsCurrentClipboard ( IDataObject *pdo )
{
	return ::OleIsCurrentClipboard(pdo);
}

HRESULT CW32System::LoadRegTypeLib (
									REFGUID rguid,
									WORD wmajor,
									WORD wminor,
									LCID lcid,
									ITypeLib ** pptlib
									)
{
#ifdef _WIN32
	return ::LoadRegTypeLib(rguid, wmajor, wminor, lcid, pptlib);
#else
	return E_NOTIMPL;
#endif
}


HRESULT CW32System::RevokeDragDrop ( HWND hwnd )
{
	return ::RevokeDragDrop(hwnd);
}

HRESULT CW32System::RegisterDragDrop ( HWND hwnd, IDropTarget *pdt )
{
	return ::RegisterDragDrop(hwnd, pdt);
}
HRESULT CW32System::OleGetClipboard ( IDataObject **ppdo )
{
	return ::OleGetClipboard(ppdo);
}
HRESULT CW32System::OleCreateLinkFromData (
	IDataObject *pdo,
	REFIID rid,
	DWORD renderopt,
	LPFORMATETC pfetc,
	IOleClientSite *psite,
	IStorage *pstg,
	void **ppv
	)
{
#ifdef _WIN32
	return ::OleCreateLinkFromData(pdo, rid, renderopt, pfetc, psite, pstg, ppv);
#else
	return E_NOTIMPL;
#endif
}

HRESULT CW32System::OleCreateStaticFromData (
	IDataObject *pdo,
	REFIID rid,
	DWORD renderopt,
	LPFORMATETC pfetc,
	IOleClientSite *psite,
	IStorage *pstg,
	void **ppv
	)
{
#ifdef _WIN32
	return ::OleCreateStaticFromData
	(pdo, rid, renderopt, pfetc, psite, pstg, ppv);
#else
	return E_NOTIMPL;
#endif
}

HRESULT CW32System::DoDragDrop ( IDataObject *pdo, IDropSource *pds, DWORD dweffect, DWORD *pdweffect )
{
	return ::DoDragDrop(pdo, pds, dweffect, pdweffect);
}

HRESULT CW32System::CoTreatAsClass ( REFCLSID clsidold, REFCLSID clsidnew )
{
#ifdef _WIN32
	return ::CoTreatAsClass(clsidold, clsidnew);
#else
	return E_NOTIMPL;
#endif	
}

HRESULT CW32System::OleConvertOLESTREAMToIStorage (
	LPOLESTREAM pstrm,
	IStorage *pstg,
	const DVTARGETDEVICE *ptd
	)
{
#ifdef _WIN32
	return ::OleConvertOLESTREAMToIStorage(pstrm, pstg, ptd);
#else
	return E_NOTIMPL;
#endif	
}

HRESULT CW32System::CLSIDFromProgID ( LPCOLESTR pstr, LPCLSID pclsid )
{
	return ::CLSIDFromProgID(pstr, pclsid);
}

HRESULT CW32System::OleCreateDefaultHandler (
	REFCLSID clsid,
	IUnknown *punk,
	REFIID riid,
	void **ppv
	)
{
#ifdef _WIN32
	return ::OleCreateDefaultHandler(clsid, punk, riid, ppv);
#else
	return E_NOTIMPL;
#endif	
}
HRESULT CW32System::CreateStreamOnHGlobal ( HGLOBAL hglobal, BOOL fDel, IStream **ppstrm )
{
#ifdef _WIN32
	return ::CreateStreamOnHGlobal(hglobal, fDel, ppstrm);
#else
	return E_NOTIMPL;
#endif	
}

HRESULT CW32System::GetHGlobalFromStream ( IStream *pstrm, HGLOBAL *phglobal )
{
#ifdef _WIN32
	return ::GetHGlobalFromStream(pstrm, phglobal);
#else
	return E_NOTIMPL;
#endif	
}

HRESULT CW32System::OleConvertIStorageToOLESTREAM ( LPSTORAGE pstg, LPOLESTREAM lpolestream)
{
#ifdef _WIN32
	return ::OleConvertIStorageToOLESTREAM(pstg, lpolestream);
#else
	return E_NOTIMPL;
#endif	
}

HRESULT CW32System::OleConvertIStorageToOLESTREAMEx (
	LPSTORAGE pstg,
	CLIPFORMAT cf,
	LONG lwidth,
	LONG lheight,
	DWORD dwsize,
	LPSTGMEDIUM pmedium,
	LPOLESTREAM lpolestream
	)
{
#ifdef _WIN32
	return ::OleConvertIStorageToOLESTREAMEx
	(pstg, cf, lwidth, lheight, dwsize, pmedium, lpolestream);
#else
	return E_NOTIMPL;
#endif	
}


HRESULT CW32System::OleInitialize ( LPVOID pvres )
{
	return ::OleInitialize(pvres);
}

void CW32System::OleUninitialize ( void )
{
	::OleUninitialize();
}


HRESULT CW32System::ProgIDFromCLSID ( REFCLSID clsid, LPOLESTR * lplpszProgId )
{
#ifdef _WIN32
	return ::ProgIDFromCLSID(clsid, lplpszProgId);
#else
	return E_NOTIMPL;
#endif	
}


/*
 *	CW32System::IsFontAvail(hDC, iCharRep, fUIFont, piFontIndex, pFontName )
 *
 *	@mfunc
 *		called when checking if a font (UI or non-UI) is installed for a given codepage
 *
 *	@devnote
 *		We will try to create the font and verify the charset of the font
 *		actually created.  If the fontname index is supplied, we will check
 *		if the requested font is installed. If the name from GDI is different,
 *		the GDI font index will be returned in piFontIndex.
 *
 *	@rdesc
 *		true if a font that supports the given codepage is available.
 *		If pFontName is given, only return true if this font is available
 */
bool CW32System::IsFontAvail(
	HDC		hDC,				//@parm	Screen hDC
	int		iCharRep,			//@parm Character repertoire	
	bool	fUIFont,			//@parm UI font?
	short	*piFontIndex,		//@parm Font Name Index (default = NULL)
	WCHAR	*pFontName)			//@parm Font Name (default = NULL)
{
	LOGFONTW	lf;
	HFONT		hfont;
	bool		retCode = false;
	BYTE		bCharSet = CharSetFromCharRep(iCharRep);

	ZeroMemory(&lf, sizeof(lf));
    
	// We want GDI to find a font that will support this charset 
	// Unspecified entries in LOGFONT will be either default or don't care.
	lf.lfCharSet = bCharSet; 
	
	if (pFontName)
	{
		lf.lfCharSet = DEFAULT_CHARSET;
		wcscpy(lf.lfFaceName, pFontName);
	}
	else if (fUIFont)
	{
		lf.lfHeight			= g_pfinfo[iCharRep]._yHeightUI;
		lf.lfPitchAndFamily = g_pfinfo[iCharRep]._bPitchAndFamilyUI;
		wcscpy(lf.lfFaceName, GetFontName((LONG)(g_pfinfo[iCharRep]._iFontUI)));
	}
	else
	{
		lf.lfHeight			= g_pfinfo[iCharRep]._yHeightOther;
		lf.lfPitchAndFamily = g_pfinfo[iCharRep]._bPitchAndFamilyOther;
		wcscpy(lf.lfFaceName, GetFontName((LONG)(g_pfinfo[iCharRep]._iFontOther)));
	}

	lf.lfHeight &= ~NEED_TO_CHECK_FONT;
	hfont = CreateFontIndirect(&lf);
	if(hfont)
    {
		HFONT	   hfontOld = SelectFont(hDC, hfont);
		TEXTMETRIC tm;
					
		if (GetTextMetrics(hDC, &tm) && (tm.tmCharSet == bCharSet || pFontName))
		{
			if (!pFontName)
				retCode = true;

			if (piFontIndex || pFontName)
			{
				const WCHAR	*pszFontName = pFontName ? pFontName : GetFontName(*piFontIndex);

				if (pszFontName)
				{ 
					WCHAR szNewFaceName[LF_FACESIZE];
					
					// Check if GDI gives us the same font name
					szNewFaceName[0] = L'\0';
					GetTextFace(hDC, LF_FACESIZE, szNewFaceName);
					if (szNewFaceName[0])
					{
						if (wcsicmp(pszFontName, szNewFaceName))
						{
							if (piFontIndex)
								*piFontIndex = GetFontNameIndex(szNewFaceName);	// Different name
						}
						else
							retCode = true;
					}
				}
			}
		}
		SelectFont(hDC, hfontOld);
		DeleteObject(hfont);
	}
	return retCode;
}

void CW32System::FreeIME()
{
}


typedef BOOL (WINAPI*III_CAST)(HKL);
BOOL CW32System::ImmIsIME (
						   HKL hKL,
						   BOOL fAimmActivated)
{
	return ::ImmIsIME(hKL);
}


BOOL CW32System::ImmSetOpenStatus ( 
								   HIMC hIMC, 
								   BOOL fOpen,
								   BOOL fAimmActivated)
{
	return ::ImmSetOpenStatus(hIMC, fOpen);
}

BOOL CW32System::ImmGetConversionStatus ( 
	HIMC hIMC, 
	LPDWORD pdwConversion, 
	LPDWORD pdwSentence,
	BOOL fAimmActivated)
{
	return ::ImmGetConversionStatus(hIMC, pdwConversion, pdwSentence);
}

BOOL CW32System::ImmSetConversionStatus ( 
	HIMC hIMC, 
	DWORD dwConversion, 
	DWORD dwSentence,
	BOOL fAimmActivated)
{
	return ::ImmSetConversionStatus(hIMC, dwConversion, dwSentence);
}

HWND CW32System::ImmGetDefaultIMEWnd (  
									  HWND hWnd,
									  BOOL fAimmActivated)
{
	return ::ImmGetDefaultIMEWnd(hWnd);
}

BOOL CW32System::ImmSetCompositionStringW ( 
	HIMC hIMC, 
	DWORD dwIndex, 
	LPVOID lpBuf, 
	DWORD dwBufLen,
	LPVOID lpRead,
	DWORD dwReadLen,
	BOOL fAimmActivated)
{
	return ::ImmSetCompositionStringW(hIMC, dwIndex, lpBuf, dwBufLen, lpRead, dwReadLen);
}

HIMC CW32System::ImmAssociateContext ( 
									  HWND hWnd, 
									  HIMC hIMC,
									  BOOL fAimmActivated)
{
	return ::ImmAssociateContext(hWnd, hIMC);
}

UINT CW32System::ImmGetVirtualKey ( 
								   HWND hWnd,
								   BOOL fAimmActivated)
{
	return ::ImmGetVirtualKey(hWnd);
}

// NOTE: We only use ImmEscape for IME_ESC_HANJA_MODE.
// Need to fix up if other methods are used.
LRESULT CW32System::ImmEscape ( 
							HKL hKL, 
							HIMC hIMC, 
							UINT uEscape, 
							LPVOID lpData,
							BOOL fAimmActivated)
{
	return ::ImmEscapeW(hKL, hIMC, uEscape, lpData);
}

BOOL CW32System::ImmGetOpenStatus ( 
								   HIMC hIMC,
								   BOOL fAimmActivated)
{
	return ::ImmGetOpenStatus(hIMC);
}
LONG CW32System::ImmGetCompositionStringA ( 
	HIMC hIMC, 
	DWORD dwIndex, 
	LPVOID lpBuf, 
	DWORD dwBufLen,
	BOOL fAimmActivated)
{	
	return ::ImmGetCompositionStringA(hIMC, dwIndex, lpBuf, dwBufLen);
}

LONG CW32System::ImmGetCompositionStringW ( 
	HIMC hIMC, 
	DWORD dwIndex, 
	LPVOID lpBuf, 
	DWORD dwBufLen,
	BOOL fAimmActivated)
{
	return ::ImmGetCompositionStringW(hIMC, dwIndex, lpBuf, dwBufLen);
}

HIMC CW32System::ImmGetContext (
								HWND hWnd,
								BOOL fAimmActivated)
{
	return ::ImmGetContext(hWnd);
}

BOOL CW32System::ImmSetCompositionFontA (
	HIMC hIMC, 
	LPLOGFONTA lpLogFontA,
	BOOL fAimmActivated)
{
	return ::ImmSetCompositionFontA(hIMC, lpLogFontA);
}

BOOL CW32System::ImmSetCompositionWindow ( 
	HIMC hIMC, 
	LPCOMPOSITIONFORM lpCompForm,
	BOOL fAimmActivated)
{
	return ::ImmSetCompositionWindow(hIMC, lpCompForm);
}

BOOL CW32System::ImmReleaseContext (
									HWND hWnd, 
									HIMC hIMC,
									BOOL fAimmActivated)
{
	return ::ImmReleaseContext(hWnd, hIMC);
}

DWORD CW32System::ImmGetProperty ( 
								  HKL hKL, 
								  DWORD dwIndex,
								  BOOL fAimmActivated)
{
	return ::ImmGetProperty(hKL, dwIndex);
}

BOOL CW32System::ImmGetCandidateWindow ( 
										HIMC hIMC, 
										DWORD dwIndex, 
										LPCANDIDATEFORM lpCandidate,
										BOOL fAimmActivated)
{
	return ::ImmGetCandidateWindow(hIMC,dwIndex,lpCandidate);
}

BOOL CW32System::ImmSetCandidateWindow ( 
										HIMC hIMC, 
										LPCANDIDATEFORM lpCandidate ,
										BOOL fAimmActivated)
{
	return ::ImmSetCandidateWindow(hIMC, lpCandidate);
}

BOOL CW32System::ImmNotifyIME ( 
							   HIMC hIMC, 
							   DWORD dwAction, 
							   DWORD dwIndex, 
							   DWORD dwValue,
							   BOOL fAimmActivated)
{
	return ::ImmNotifyIME(hIMC, dwAction, dwIndex, dwValue);
}

HRESULT CW32System::CoCreateInstance (REFCLSID rclsid, LPUNKNOWN pUnknown,
									  DWORD dwClsContext, REFIID riid, LPVOID *ppv)
{
	return ::CoCreateInstance(rclsid, pUnknown, dwClsContext, riid, ppv);
}

HRESULT CW32System::OleCreateFromFile (
									   REFCLSID		rclsid, 
									   LPCOLESTR		lpszFileName,
									   REFIID			riid, 
									   DWORD			renderopt,
									   LPFORMATETC		pFormatEtc, 
									   LPOLECLIENTSITE pClientSite,
									   LPSTORAGE		pStg, 
									   LPVOID *		ppvObj)
{
#ifdef _WIN32
	return ::OleCreateFromFile(rclsid, lpszFileName, riid, renderopt,
		pFormatEtc, pClientSite, pStg, ppvObj);
#else
	return E_NOTIMPL;
#endif	
}