#ifndef _CTRL_TYPE_H_
#define _CTRL_TYPE_H_

#include <ctypes.h>

typedef struct tagNMHDR
{
    HWND      hwndFrom;
    UINT_PTR  idFrom;
    UINT      code;         // NM_ code
}   NMHDR;
typedef NMHDR FAR * LPNMHDR;


/*
 * Scroll bar messages
 */
#define SBM_SETPOS                  0x00E0 /*not in win3.1 */
#define SBM_GETPOS                  0x00E1 /*not in win3.1 */
#define SBM_SETRANGE                0x00E2 /*not in win3.1 */
#define SBM_SETRANGEREDRAW          0x00E6 /*not in win3.1 */
#define SBM_GETRANGE                0x00E3 /*not in win3.1 */
#define SBM_ENABLE_ARROWS           0x00E4 /*not in win3.1 */
#define SBM_SETSCROLLINFO           0x00E9
#define SBM_GETSCROLLINFO           0x00EA
#define SBM_GETSCROLLBARINFO        0x00EB

#define SIF_RANGE           0x0001
#define SIF_PAGE            0x0002
#define SIF_POS             0x0004
#define SIF_DISABLENOSCROLL 0x0008
#define SIF_TRACKPOS        0x0010
#define SIF_ALL             (SIF_RANGE | SIF_PAGE | SIF_POS | SIF_TRACKPOS)

typedef struct tagSCROLLINFO
{
    UINT    cbSize;
    UINT    fMask;
    int     nMin;
    int     nMax;
    UINT    nPage;
    int     nPos;
    int     nTrackPos;
}   SCROLLINFO, FAR *LPSCROLLINFO;
typedef SCROLLINFO CONST FAR *LPCSCROLLINFO;


#define TVE_COLLAPSE            0x0001
#define TVE_EXPAND              0x0002
#define TVE_TOGGLE              0x0003
#define TVE_EXPANDPARTIAL       0x4000
#define TVE_COLLAPSERESET       0x8000


#define NM_FIRST                (0U-  0U)       // generic to all controls
#define NM_LAST                 (0U- 99U)

#define LVN_FIRST               (0U-100U)       // listview
#define LVN_LAST                (0U-199U)

// Property sheet reserved      (0U-200U) -  (0U-299U) - see prsht.h

#define HDN_FIRST               (0U-300U)       // header
#define HDN_LAST                (0U-399U)

#define TVN_FIRST               (0U-400U)       // treeview
#define TVN_LAST                (0U-499U)

#define TTN_FIRST               (0U-520U)       // tooltips
#define TTN_LAST                (0U-549U)

#define TCN_FIRST               (0U-550U)       // tab control
#define TCN_LAST                (0U-580U)

// Shell reserved               (0U-580U) -  (0U-589U)

#define CDN_FIRST               (0U-601U)       // common dialog (new)
#define CDN_LAST                (0U-699U)

#define TBN_FIRST               (0U-700U)       // toolbar
#define TBN_LAST                (0U-720U)

#define UDN_FIRST               (0U-721U)        // updown
#define UDN_LAST                (0U-729U)
#define DTN_FIRST               (0U-740U)       // datetimepick
#define DTN_LAST                (0U-745U)       // DTN_FIRST - 5

#define MCN_FIRST               (0U-746U)       // monthcal
#define MCN_LAST                (0U-752U)       // MCN_FIRST - 6

#define DTN_FIRST2              (0U-753U)       // datetimepick2
#define DTN_LAST2               (0U-799U)

#define CBEN_FIRST              (0U-800U)       // combo box ex
#define CBEN_LAST               (0U-830U)

#define RBN_FIRST               (0U-831U)       // rebar
#define RBN_LAST                (0U-859U)
#define IPN_FIRST               (0U-860U)       // internet address
#define IPN_LAST                (0U-879U)       // internet address

#define SBN_FIRST               (0U-880U)       // status bar
#define SBN_LAST                (0U-899U)

#define PGN_FIRST               (0U-900U)       // Pager Control
#define PGN_LAST                (0U-950U)

#define WMN_FIRST               (0U-1000U)
#define WMN_LAST                (0U-1200U)
#define BCN_FIRST               (0U-1250U)
#define BCN_LAST                (0U-1350U)
#define TRBN_FIRST              (0U-1501U)       // trackbar
#define TRBN_LAST               (0U-1519U)

#define NM_OUTOFMEMORY          (NM_FIRST-1)
#define NM_CLICK                (NM_FIRST-2)    // uses NMCLICK struct
#define NM_DBLCLK               (NM_FIRST-3)
#define NM_RETURN               (NM_FIRST-4)
#define NM_RCLICK               (NM_FIRST-5)    // uses NMCLICK struct
#define NM_RDBLCLK              (NM_FIRST-6)
#define NM_SETFOCUS             (NM_FIRST-7)
#define NM_KILLFOCUS            (NM_FIRST-8)
#define NM_CUSTOMDRAW           (NM_FIRST-12)
#define NM_HOVER                (NM_FIRST-13)
#define NM_NCHITTEST            (NM_FIRST-14)   // uses NMMOUSE struct
#define NM_KEYDOWN              (NM_FIRST-15)   // uses NMKEY struct
#define NM_RELEASEDCAPTURE      (NM_FIRST-16)
#define NM_SETCURSOR            (NM_FIRST-17)   // uses NMMOUSE struct
#define NM_CHAR                 (NM_FIRST-18)   // uses NMCHAR struct
#define NM_TOOLTIPSCREATED      (NM_FIRST-19)   // notify of when the tooltips window is create
#define NM_LDOWN                (NM_FIRST-20)
#define NM_RDOWN                (NM_FIRST-21)
#define NM_THEMECHANGED         (NM_FIRST-22)
#define NM_FONTCHANGED          (NM_FIRST-23)
#define NM_CUSTOMTEXT           (NM_FIRST-24)   // uses NMCUSTOMTEXT struct
#define NM_TVSTATEIMAGECHANGING (NM_FIRST-24)   // uses NMTVSTATEIMAGECHANGING struct, defined after HTREEITEM


/*
 * MEASUREITEMSTRUCT for ownerdraw
 */
typedef struct tagMEASUREITEMSTRUCT {
    UINT       CtlType;
    UINT       CtlID;
    UINT       itemID;
    UINT       itemWidth;
    UINT       itemHeight;
    ULONG_PTR  itemData;
} MEASUREITEMSTRUCT, NEAR *PMEASUREITEMSTRUCT, FAR *LPMEASUREITEMSTRUCT;

/*
 * DRAWITEMSTRUCT for ownerdraw
 */
typedef struct tagDRAWITEMSTRUCT {
    UINT        CtlType;
    UINT        CtlID;
    UINT        itemID;
    UINT        itemAction;
    UINT        itemState;
    HWND        hwndItem;
    HDC         hDC;
    RECT        rcItem;
    ULONG_PTR   itemData;
} DRAWITEMSTRUCT, NEAR *PDRAWITEMSTRUCT, FAR *LPDRAWITEMSTRUCT;

/*
 * DELETEITEMSTRUCT for ownerdraw
 */
typedef struct tagDELETEITEMSTRUCT {
    UINT       CtlType;
    UINT       CtlID;
    UINT       itemID;
    HWND       hwndItem;
    ULONG_PTR  itemData;
} DELETEITEMSTRUCT, NEAR *PDELETEITEMSTRUCT, FAR *LPDELETEITEMSTRUCT;

/*
 * COMPAREITEMSTUCT for ownerdraw sorting
 */
typedef struct tagCOMPAREITEMSTRUCT {
    UINT        CtlType;
    UINT        CtlID;
    HWND        hwndItem;
    UINT        itemID1;
    ULONG_PTR   itemData1;
    UINT        itemID2;
    ULONG_PTR   itemData2;
    DWORD       dwLocaleId;
} COMPAREITEMSTRUCT, NEAR *PCOMPAREITEMSTRUCT, FAR *LPCOMPAREITEMSTRUCT;


typedef struct tagTEXTMETRIC
{
    LONG        tmHeight;
    LONG        tmAscent;
    LONG        tmDescent;
    LONG        tmInternalLeading;
    LONG        tmExternalLeading;
    LONG        tmAveCharWidth;
    LONG        tmMaxCharWidth;
    LONG        tmWeight;
    LONG        tmOverhang;
    LONG        tmDigitizedAspectX;
    LONG        tmDigitizedAspectY;
    BYTE        tmFirstChar;
    BYTE        tmLastChar;
    BYTE        tmDefaultChar;
    BYTE        tmBreakChar;
    BYTE        tmItalic;
    BYTE        tmUnderlined;
    BYTE        tmStruckOut;
    BYTE        tmPitchAndFamily;
    BYTE        tmCharSet;
} TEXTMETRIC, *PTEXTMETRIC, NEAR *NPTEXTMETRIC, FAR *LPTEXTMETRIC;


#define HDF_LEFT                0x0000 // Same as LVCFMT_LEFT
#define HDF_RIGHT               0x0001 // Same as LVCFMT_RIGHT
#define HDF_CENTER              0x0002 // Same as LVCFMT_CENTER
#define HDF_JUSTIFYMASK         0x0003 // Same as LVCFMT_JUSTIFYMASK
#define HDF_RTLREADING          0x0004 // Same as LVCFMT_LEFT

#define HDF_BITMAP              0x2000
#define HDF_STRING              0x4000
#define HDF_OWNERDRAW           0x8000 // Same as LVCFMT_COL_HAS_IMAGES
#define HDF_IMAGE               0x0800 // Same as LVCFMT_IMAGE
#define HDF_BITMAP_ON_RIGHT     0x1000 // Same as LVCFMT_BITMAP_ON_RIGHT
#define HDF_SORTUP              0x0400
#define HDF_SORTDOWN            0x0200

#define HDF_CHECKBOX            0x0040
#define HDF_CHECKED             0x0080
#define HDF_FIXEDWIDTH          0x0100 // Can't resize the column; same as LVCFMT_FIXED_WIDTH
#define HDF_SPLITBUTTON      0x1000000 // Column is a split button; same as LVCFMT_SPLITBUTTON


#define HOTKEYF_SHIFT           0x01
#define HOTKEYF_CONTROL         0x02
#define HOTKEYF_ALT             0x04
#define HOTKEYF_EXT             0x08

#define HKCOMB_NONE             0x0001
#define HKCOMB_S                0x0002
#define HKCOMB_C                0x0004
#define HKCOMB_A                0x0008
#define HKCOMB_SC               0x0010
#define HKCOMB_SA               0x0020
#define HKCOMB_CA               0x0040
#define HKCOMB_SCA              0x0080


#define SB_LINEUP           0
#define SB_LINELEFT         0
#define SB_LINEDOWN         1
#define SB_LINERIGHT        1
#define SB_PAGEUP           2
#define SB_PAGELEFT         2
#define SB_PAGEDOWN         3
#define SB_PAGERIGHT        3
#define SB_THUMBPOSITION    4
#define SB_THUMBTRACK       5
#define SB_TOP              6
#define SB_LEFT             6
#define SB_BOTTOM           7
#define SB_RIGHT            7
#define SB_ENDSCROLL        8

#define EN_SETFOCUS         0x0100
#define EN_KILLFOCUS        0x0200
#define EN_CHANGE           0x0300
#define EN_UPDATE           0x0400
#define EN_ERRSPACE         0x0500
#define EN_MAXTEXT          0x0501
#define EN_HSCROLL          0x0601
#define EN_VSCROLL          0x0602

#endif