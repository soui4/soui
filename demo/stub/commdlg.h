
#define OFN_READONLY                 0x00000001
#define OFN_OVERWRITEPROMPT          0x00000002
#define OFN_HIDEREADONLY             0x00000004
#define OFN_NOCHANGEDIR              0x00000008
#define OFN_SHOWHELP                 0x00000010
#define OFN_ENABLEHOOK               0x00000020
#define OFN_ENABLETEMPLATE           0x00000040
#define OFN_ENABLETEMPLATEHANDLE     0x00000080
#define OFN_NOVALIDATE               0x00000100
#define OFN_ALLOWMULTISELECT         0x00000200
#define OFN_EXTENSIONDIFFERENT       0x00000400
#define OFN_PATHMUSTEXIST            0x00000800
#define OFN_FILEMUSTEXIST            0x00001000
#define OFN_CREATEPROMPT             0x00002000
#define OFN_SHAREAWARE               0x00004000
#define OFN_NOREADONLYRETURN         0x00008000
#define OFN_NOTESTFILECREATE         0x00010000
#define OFN_NONETWORKBUTTON          0x00020000
#define OFN_NOLONGNAMES              0x00040000     // force no long names for 4.x modules
#if(WINVER >= 0x0400)
#define OFN_EXPLORER                 0x00080000     // new look commdlg
#define OFN_NODEREFERENCELINKS       0x00100000
#define OFN_LONGNAMES                0x00200000     // force long names for 3.x modules
// OFN_ENABLEINCLUDENOTIFY and OFN_ENABLESIZING require
// Windows 2000 or higher to have any effect.
#define OFN_ENABLEINCLUDENOTIFY      0x00400000     // send include message to callback
#define OFN_ENABLESIZING             0x00800000
#endif /* WINVER >= 0x0400 */
#if (_WIN32_WINNT >= 0x0500)
#define OFN_DONTADDTORECENT          0x02000000
#define OFN_FORCESHOWHIDDEN          0x10000000    // Show All files including System and hidden files
#endif // (_WIN32_WINNT >= 0x0500)

//FlagsEx Values
#if (_WIN32_WINNT >= 0x0500)
#define  OFN_EX_NOPLACESBAR         0x00000001
#endif // (_WIN32_WINNT >= 0x0500)

typedef UINT_PTR (CALLBACK *LPOFNHOOKPROC) (HWND, UINT, WPARAM, LPARAM);
typedef UINT_PTR (CALLBACK *LPCCHOOKPROC) (HWND, UINT, WPARAM, LPARAM);

typedef struct tagEDITMENU
{
    HMENU   hmenu;
    WORD    idEdit;
    WORD    idCut;
    WORD    idCopy;
    WORD    idPaste;
    WORD    idClear;
    WORD    idUndo;
} EDITMENU;
typedef EDITMENU *LPEDITMENU;
typedef struct tagOFNA {
  DWORD         lStructSize;
  HWND          hwndOwner;
  HINSTANCE     hInstance;
  LPCSTR        lpstrFilter;
  LPSTR         lpstrCustomFilter;
  DWORD         nMaxCustFilter;
  DWORD         nFilterIndex;
  LPSTR         lpstrFile;
  DWORD         nMaxFile;
  LPSTR         lpstrFileTitle;
  DWORD         nMaxFileTitle;
  LPCSTR        lpstrInitialDir;
  LPCSTR        lpstrTitle;
  DWORD         Flags;
  WORD          nFileOffset;
  WORD          nFileExtension;
  LPCSTR        lpstrDefExt;
  LPARAM        lCustData;
  LPOFNHOOKPROC lpfnHook;
  LPCSTR        lpTemplateName;
  LPEDITMENU    lpEditInfo;
  LPCSTR        lpstrPrompt;
  void          *pvReserved;
  DWORD         dwReserved;
  DWORD         FlagsEx;
} OPENFILENAMEA, *LPOPENFILENAMEA;

typedef struct tagOFNW {
    DWORD         lStructSize;
    HWND          hwndOwner;
    HINSTANCE     hInstance;
    LPCWSTR       lpstrFilter;
    LPWSTR        lpstrCustomFilter;
    DWORD         nMaxCustFilter;
    DWORD         nFilterIndex;
    LPWSTR        lpstrFile;
    DWORD         nMaxFile;
    LPWSTR        lpstrFileTitle;
    DWORD         nMaxFileTitle;
    LPCWSTR       lpstrInitialDir;
    LPCWSTR       lpstrTitle;
    DWORD         Flags;
    WORD          nFileOffset;
    WORD          nFileExtension;
    LPCWSTR       lpstrDefExt;
    LPARAM        lCustData;
    LPOFNHOOKPROC lpfnHook;
    LPCWSTR       lpTemplateName;
    LPEDITMENU    lpEditInfo;
    LPCSTR        lpstrPrompt;
    void          *pvReserved;
    DWORD         dwReserved;
    DWORD         FlagsEx;
} OPENFILENAMEW, *LPOPENFILENAMEW;

#ifdef UNICODE
typedef OPENFILENAMEW OPENFILENAME;
typedef LPOPENFILENAMEW LPOPENFILENAME;
#else
typedef OPENFILENAMEA OPENFILENAME;
typedef LPOPENFILENAMEA LPOPENFILENAME;
#endif // UNICODE

BOOL  GetOpenFileNameA(LPOPENFILENAMEA);
BOOL  GetOpenFileNameW(LPOPENFILENAMEW);
#ifdef UNICODE
#define GetOpenFileName  GetOpenFileNameW
#else
#define GetOpenFileName  GetOpenFileNameA
#endif // !UNICODE
BOOL  GetSaveFileNameA(LPOPENFILENAMEA);
BOOL  GetSaveFileNameW(LPOPENFILENAMEW);
#ifdef UNICODE
#define GetSaveFileName  GetSaveFileNameW
#else
#define GetSaveFileName  GetSaveFileNameA
#endif // !UNICODE

#ifndef _MAC
typedef struct tagCHOOSECOLORA {
    DWORD        lStructSize;
    HWND         hwndOwner;
    HWND         hInstance;
    COLORREF     rgbResult;
    COLORREF*    lpCustColors;
    DWORD        Flags;
    LPARAM       lCustData;
    LPCCHOOKPROC lpfnHook;
    LPCSTR       lpTemplateName;
} CHOOSECOLORA, *LPCHOOSECOLORA;
typedef struct tagCHOOSECOLORW {
    DWORD        lStructSize;
    HWND         hwndOwner;
    HWND         hInstance;
    COLORREF     rgbResult;
    COLORREF*    lpCustColors;
    DWORD        Flags;
    LPARAM       lCustData;
    LPCCHOOKPROC lpfnHook;
    LPCWSTR      lpTemplateName;
} CHOOSECOLORW, *LPCHOOSECOLORW;
#ifdef UNICODE
typedef CHOOSECOLORW CHOOSECOLOR;
typedef LPCHOOSECOLORW LPCHOOSECOLOR;
#else
typedef CHOOSECOLORA CHOOSECOLOR;
typedef LPCHOOSECOLORA LPCHOOSECOLOR;
#endif // UNICODE
#else
typedef struct tagCHOOSECOLORA {
    DWORD        lStructSize;
    HWND         hwndOwner;
    HWND         hInstance;
    COLORREF     rgbResult;
    COLORREF*    lpCustColors;
    DWORD        Flags;
    LPARAM       lCustData;
    LPCCHOOKPROC lpfnHook;
    LPCSTR       lpTemplateName;
    LPEDITMENU   lpEditInfo;
} CHOOSECOLORA, *LPCHOOSECOLORA;
typedef struct tagCHOOSECOLORW {
    DWORD        lStructSize;
    HWND         hwndOwner;
    HWND         hInstance;
    COLORREF     rgbResult;
    COLORREF*    lpCustColors;
    DWORD        Flags;
    LPARAM       lCustData;
    LPCCHOOKPROC lpfnHook;
    LPCWSTR      lpTemplateName;
    LPEDITMENU   lpEditInfo;
} CHOOSECOLORW, *LPCHOOSECOLORW;
#ifdef UNICODE
typedef CHOOSECOLORW CHOOSECOLOR;
typedef LPCHOOSECOLORW LPCHOOSECOLOR;
#else
typedef CHOOSECOLORA CHOOSECOLOR;
typedef LPCHOOSECOLORA LPCHOOSECOLOR;
#endif // UNICODE
#endif //_MAC

BOOL ChooseColorA(LPCHOOSECOLORA);
BOOL ChooseColorW(LPCHOOSECOLORW);
#ifdef UNICODE
#define ChooseColor  ChooseColorW
#else
#define ChooseColor  ChooseColorA
#endif // !UNICODE

#define CC_RGBINIT               0x00000001
#define CC_FULLOPEN              0x00000002
#define CC_PREVENTFULLOPEN       0x00000004
#define CC_SHOWHELP              0x00000008
#define CC_ENABLEHOOK            0x00000010
#define CC_ENABLETEMPLATE        0x00000020
#define CC_ENABLETEMPLATEHANDLE  0x00000040
#if(WINVER >= 0x0400)
#define CC_SOLIDCOLOR            0x00000080
#define CC_ANYCOLOR              0x00000100
#endif /* WINVER >= 0x0400 */