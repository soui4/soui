/*
 *	@doc	INTERNAL
 *
 *	@module	_COLEOBJ.H		The OLE Object management class | 
 *
 *	Author:	alexgo 10/24/95
 *
 *	Copyright (c) 1995-2000, Microsoft Corporation. All rights reserved.
 */

#ifndef __COLEOBJ_H__
#define __COLEOBJ_H__

#ifndef NOINKOBJECT
#include "HWXInk.h"
#endif

#include "_notmgr.h"

#ifndef NOLINESERVICES
extern "C" {
#include "plsdnode.h"
}
#endif

class CDisplay;
class CDevDesc;
class CTxtEdit;
class IUndoBuilder;

/* 
 *	COleObject
 *
 *	@class	This class manages an individual OLE object embedding.
 *
 */

class COleObject :  public IOleClientSite, public IOleInPlaceSite, 
					public IAdviseSink, public CSafeRefCount, public ITxNotify
{
//@access Public methods
public:
	//
	// IUnknown methods
	//

	STDMETHOD(QueryInterface)(REFIID riid, void **pv);
	STDMETHOD_(ULONG, AddRef)(void);
	STDMETHOD_(ULONG, Release)(void);
	
	//
	// IOleClientSite methods
	//
	
	STDMETHOD(SaveObject)(void);
	STDMETHOD(GetMoniker)(DWORD dwAssign, DWORD dwWhichMoniker,
							IMoniker **ppmk);
	STDMETHOD(GetContainer)(IOleContainer **ppContainer);
	STDMETHOD(ShowObject)(void);
	STDMETHOD(OnShowWindow)(BOOL fShow);
	STDMETHOD(RequestNewObjectLayout)(void);

	//
	//	IOleInPlaceSite methods
	//
	STDMETHOD(GetWindow)(HWND *phwnd);
	STDMETHOD(ContextSensitiveHelp)(BOOL fEnterMode);
	STDMETHOD(CanInPlaceActivate)(void);
	STDMETHOD(OnInPlaceActivate)(void);
	STDMETHOD(OnUIActivate)(void);
	STDMETHOD(GetWindowContext)(IOleInPlaceFrame **ppFrame,
							IOleInPlaceUIWindow **ppDoc, LPRECT lprcPosRect,
							LPRECT lprcClipRect, 
							LPOLEINPLACEFRAMEINFO lpFrameInfo);
	STDMETHOD(Scroll)(SIZE scrollExtant);
	STDMETHOD(OnUIDeactivate)(BOOL fUndoable);
	STDMETHOD(OnInPlaceDeactivate)(void);
	STDMETHOD(DiscardUndoState)(void);
	STDMETHOD(DeactivateAndUndo)(void);
	STDMETHOD(OnPosRectChange)(LPCRECT lprcPosRect);

	//
	// IAdviseSink methods
	//

	STDMETHOD_(void, OnDataChange)(FORMATETC *pfetc, STGMEDIUM *pmedium);
	STDMETHOD_(void, OnViewChange)(DWORD dwAspect, LONG lindex);
	STDMETHOD_(void, OnRename)(IMoniker *pmk);
	STDMETHOD_(void, OnSave)(void);
	STDMETHOD_(void, OnClose)(void);

	//
	//	ITxNotify methods
	//
    virtual void    OnPreReplaceRange( LONG cp, LONG cchDel, LONG cchNew,
                        LONG cpFormatMin, LONG cpFormatMax, NOTIFY_DATA *pNotifyData );
    virtual void    OnPostReplaceRange( LONG cp, LONG cchDel, LONG cchNew,
                        LONG cpFormatMin, LONG cpFormatMax, NOTIFY_DATA *pNotifyData );
	virtual void	Zombie();

	//
	//	internal public methods
	//

			COleObject(CTxtEdit *ped);	//@cmember Constructor
	LONG	GetCp()		{return _cp;}	//@cmember Get cp for this object
	CTxtEdit *GetPed()	{return _ped;}	//@cmember Get ped for this object
										//@cmember Fill out the REOBJECT struct
	HRESULT	GetObjectData(REOBJECT *preobj, DWORD dwFlags);
	BOOL	IsLink();					//@cmember returns TRUE if the object
										// is a link object.
										//@cmember Initialize from the given
										// object data
	HRESULT InitFromREOBJECT(LONG cp, REOBJECT *preobj);
										//@cmember Measures the object (no outgoing calls)
	void MeasureObj(LONG dvpInch, LONG dupInch, LONG & dup, LONG & dvpAscent,
					LONG & dvpDescent, SHORT dvpDescentFont, TFLOW tflow);
										//@cmember Draws the object
	void	DrawObj(const CDisplay *pdp, LONG dypInch, LONG dxpInch, HDC hdc, const RECTUV *prcClip,
					BOOL fMetafile, POINTUV *ppt, LONG yBaselineLine, LONG yDescentMaxCur, TFLOW tflow);
										//@cmember Handle the object being
										// deleted from the backing store
	void	Delete(IUndoBuilder *publdr);
	void	Restore();					//@cmember restore an object into
										// the backing store
										//@cmember Force a close on the
										// object
	void	Close(DWORD dwSaveOption);
										//@cmember Zombie the object.
    void    MakeZombie(void);
										//@cmember Activate the object
	BOOL	ActivateObj(UINT uiMsg,	WPARAM wParam, LPARAM lParam);
	HRESULT	DeActivateObj(void);		//@cmember Aeactivate the object
										//@cmember Set individual selection
										//state of object

	void GetRectuv(RECTUV &rc);

	void FetchObjectExtents(void);//@cmember Gets the size of the object
									//@cmember return REO_SELECTED state
	BOOL	FWrapTextAround(void) const {return (_pi.dwFlags & REO_WRAPTEXTAROUND) != 0;}
	BOOL	FAlignToRight(void) const {return (_pi.dwFlags & REO_ALIGNTORIGHT) != 0;}

	BOOL	GetREOSELECTED(void) {return (_pi.dwFlags & REO_SELECTED);}
										//@cmember set REO_SELECTED state
	void	SetREOSELECTED(BOOL fSelect);
								//@cmember checks for hits on frame handles
	WCHAR*  CheckForHandleHit(const POINTUV &pt, BOOL fLogical = FALSE);
								//@cmember handle object resize
	BOOL    HandleResize(const POINT &pt);
								//@cmember sets object size
	void    Resize(const SIZEUV &size, BOOL fCreateAntiEvent);
								//@cmember update _size
	void	ResetSize(const SIZEUV &size)
						{_size = size;}
								//@cmember called when object position changes
	void    OnReposition();
								//@cmember gets objects IUnknown
	IUnknown *	GetIUnknown(void) {return _punkobj;}
								//@cmember converts to the specified class
	HRESULT	Convert(REFCLSID rclsidNew, LPCSTR lpstrUserTypeNew);
								//@cmember activate as the specified class
	HRESULT	ActivateAs(REFCLSID rclsid, REFCLSID rclsidAs);
								//@cmember set aspect to use
	void	SetDvaspect(DWORD dvaspect);
								//@cmember get current aspect
	DWORD	GetDvaspect()
				{ return _pi.dvaspect; }
								//@cmember see IPersistStore::HandsOffStorage
	void	HandsOffStorage(void);
								//@cmember see IPersistStore::SaveCompleted
	void	SaveCompleted(LPSTORAGE lpstg);
								//@cmember set REO_LINKAVAILABLE flag
	HRESULT SetLinkAvailable(BOOL fAvailable);
								//@cmember Used for textize support
	LONG	WriteTextInfoToEditStream(EDITSTREAM *pes, UINT CodePage);
	
	void    SetHdata(HGLOBAL hg)	{_hdata = hg;}	//@cmember get the _hdata member

	void	GetSize(SIZEUV &size)	{size = _size;}
								//&cmember gets the _size member
	DWORD	GetFlags()	{return  _pi.dwFlags;}
								//&cmember gets the _sizel member
	DWORD	GetUser()	{return  _pi.dwUser;}
								//&cmember gets the _sizel member
	DWORD	GetAspect()	{return  _pi.dvaspect;}
								//&cmember gets the _sizel member

	// For internal use without reentering undo system.
	STDMETHOD(SafeSaveObject)(void);

	HGLOBAL GetHdata()				{return _hdata;}
	struct ImageInfo
	{
		LONG	xScale, yScale;		// @field scaling percentage aLONG axes
		SHORT	xExtGoal, yExtGoal;	// @field desired dimensions in twips for pictures
		SHORT	cBytesPerLine;		// @field # bytes per raster line, if bitmap
	};
								//@cmember set the _pimageinfo member
	void    SetImageInfo(struct ImageInfo *pim)	{_pimageinfo = pim;}
								//@cmember get the _pimageinfo member
	ImageInfo *GetImageInfo()				{return _pimageinfo;}

#ifdef DEBUG
	void    DbgDump(DWORD id);
#endif
	BOOL	GetViewChanged()	{return _fViewChange;}
	void	ResetViewChanged()	{_fViewChange = FALSE;}

//@access Private methods and data
private:
	virtual ~COleObject();		//@cmember Destructor

	void SavePrivateState(void);//@cmember Saves private information
	HRESULT ConnectObject(void);//@cmember setup advises, etc.
	void DisconnectObject(void);//@cmember tear down advises, etc.
	void CleanupState(void);	//@cmember cleans up our member data, etc.
								//@cmember draws frame around object
	void DrawFrame(const CDisplay *pdp, HDC hdc, RECT* prc);
								//@cmember helper to draw frame handles
	void DrawHandle(HDC hdc, int x, int y);
								//@cmember helper to check for hit on a handle
	void CreateDib(HDC hdc);
								//@cmember helper to create DIB for WinCE bitmaps
	void DrawDib(HDC hdc, RECT *prc);
								//@cmember helper to draw WinCE bitmaps
	BOOL InHandle(int up, int vp, const POINTUV &pt);
								//@cmember to restablish rectangle position
	enum { SE_NOTACTIVATING, SE_ACTIVATING };  // used by SetExtent to indicate
												// the context of the SetExtent call
								//@cmember Attempts to set object extents
	HRESULT SetExtent(int iActivating);

	CTxtEdit *		_ped;		//@cmember edit context for this object
	IUnknown *		_punkobj;	//@cmember pointer to the objects IUnknown.
	IStorage *		_pstg;		//@cmember storage for the object
	SIZEUV			_size;		//@cmember cached "real" size of the object
	LONG			_cp;		//@cmember position of this object
	DWORD			_dwConn;	//@cmember advise connection cookie
	HGLOBAL			_hdata;
	HBITMAP			_hdib;
	ImageInfo *		_pimageinfo;

	struct PersistedInfo
	{
		DWORD	dwFlags;		//@cmember see richole.h
		DWORD	dwUser;			//@cmember user defined
		DWORD	dvaspect;		//@cmember from the DVASPECT enumeration
	};

	PersistedInfo	_pi;

	SHORT	_dxyFrame;			//@cmember Object frame width
	WORD	_fInPlaceActive:1;	//@cmember inplace active?
	WORD	_fInUndo:1;			//@cmember in the undo stack?
	WORD	_fIsWordArt2:1;		//@cmember Is this object a WordArt 2.0
								// object? (need to do hacks for it)
	WORD 	_fIsPaintBrush:1;	//@cmember Is this object a PaintBrush
								// object? (need to do hacks for it)
	WORD	_fPBUseLocalSize:1;	
								//@cmember Signals that SetExtent
								// for a PaintBrush object failed and that
								// _size is an accurate indication of object size
	WORD	_fDraw:1;			//@cmember Should object be drawn?
	WORD	_fSetExtent:1;		//@cmember Does SetExtent need to be called
								// when activating.
	WORD	_fDeactivateCalled:1;//@cmember Whether deactivate has been called.
	WORD	_fAspectChanged:1;	//@cmember Forces FetchObjectExtents to call through when aspect changes
	WORD	_fViewChange:1;		//@cmember flag to indicate size of object changed

	WORD	_fActivateCalled:1;	//@member Are we in the middle of the activation sequence?
	WORD	_fIsInkObject:1;	//@member Is this object a Ink Object

#ifndef NOINKOBJECT
public:
	ILineInfo	*_pILineInfo;	//@member Ink object ILineInfo
	BOOL	IsInkObject() { return _fIsInkObject; };
#endif

#ifndef NOLINESERVICES
public:
	PLSDNODE _plsdnTop;			//@cmember Ptr to LS display node
#endif

//VikramM - hacks added for E-Book
private:
	BOOL _fIsEBookImage; //@flags this is a ebook image that needs special treatment
	LPARAM _EBookImageID;
	SIZE _sizeEBookImage;
public: //E-Book functions
	void IsEbookImage(BOOL fIs) { _fIsEBookImage = fIs; };
	void SetEBookImageID(LPARAM lParam) { _EBookImageID = lParam ; };
	void SetEBookImageSizeDP(SIZE size) {_sizeEBookImage = size; };
};

#endif // __COLEOBJ_H_


	
