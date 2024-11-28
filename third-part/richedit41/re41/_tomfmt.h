/*
 *	@doc
 *
 *	@module _tomfmt.h -- CTxtFont and CTxtPara Classes |
 *	
 *		This class implements the TOM ITextFont and ITextPara interfaces
 *	
 *	Author: <nl>
 *		Murray Sargent
 *
 *	Copyright (c) 1995-1998, Microsoft Corporation. All rights reserved.
 */

#ifndef _tomformat_H
#define _tomformat_H

#include "_range.h"

extern const BYTE g_rgREtoTOMAlign[];

// CTxtFormat: base class for CTxtFont and CTxtPara

class CTxtFormat
{
protected:
	CTxtFormat(CTxtRange *prg);
	~CTxtFormat();

	LONG		_cRefs;
	CTxtRange *	_prg;

	HRESULT	CanChange(LONG *pBool, BOOL fPara);
	HRESULT	GetParameter (LONG *pParm, DWORD dwMask, LONG Type, LONG *pValue);
	HRESULT	SetParameter (LONG *pParm, LONG Type, LONG Value);
	HRESULT	IsTrue		 (BOOL f, LONG *pB);
	BOOL	IsZombie()	 {return _prg && _prg->IsZombie();}
};


class CTxtFont : public ITextFont, CTxtFormat
{
	friend	CTxtRange;
	friend	CTxtEdit;

	CCharFormat	_CF;
	DWORD		_dwMask;			// CHARFORMAT2 mask
	TMPDISPLAYATTR	_tmpDisplayAttr;	// Temp display attribute
	union
	{
	  DWORD _dwFlags;				// All together now
	  struct
	  {
		DWORD _fApplyLater : 1;		// Delay call to _prg->CharFormatSetter()
		DWORD _fCacheParms : 1;		// Update _CF now but not on GetXs
		DWORD _fApplyToTmp : 1;		// Apply change to temp. display attributes
	  };
	};

public:
	CTxtFont(CTxtRange *prg);

	// IUnknown methods
	STDMETHODIMP QueryInterface(REFIID riid, LPVOID * ppvObj);
	STDMETHOD_(ULONG, AddRef)();
	STDMETHOD_(ULONG, Release)();

	// IDispatch methods
	STDMETHODIMP GetTypeInfoCount(UINT * pctinfo);
	STDMETHODIMP GetTypeInfo(UINT itinfo, LCID lcid, ITypeInfo ** pptinfo);
	STDMETHODIMP GetIDsOfNames(REFIID riid, OLECHAR ** rgszNames, UINT cNames,
							 LCID lcid, DISPID * rgdispid) ;
	STDMETHODIMP Invoke(DISPID dispidMember, REFIID riid, LCID lcid, WORD wFlags,
					  DISPPARAMS * pdispparams, VARIANT * pvarResult,
					  EXCEPINFO * pexcepinfo, UINT * puArgErr) ;

	// ITextFont methods
	STDMETHODIMP GetDuplicate(ITextFont **ppFont);
	STDMETHODIMP SetDuplicate(ITextFont *pFont);
	STDMETHODIMP CanChange(LONG *pB);
	STDMETHODIMP IsEqual(ITextFont *pFont, LONG *pB);
	STDMETHODIMP Reset(LONG Value);
	STDMETHODIMP GetStyle(LONG *pValue);
	STDMETHODIMP SetStyle(LONG Value);
	STDMETHODIMP GetAllCaps(LONG *pValue);
	STDMETHODIMP SetAllCaps(LONG Value);
	STDMETHODIMP GetAnimation(LONG *pValue);
	STDMETHODIMP SetAnimation(LONG Value);
	STDMETHODIMP GetBackColor(LONG *pValue);
	STDMETHODIMP SetBackColor(LONG Value);
	STDMETHODIMP GetBold(LONG *pValue);
	STDMETHODIMP SetBold(LONG Value);
	STDMETHODIMP GetEmboss(LONG *pValue);
	STDMETHODIMP SetEmboss(LONG Value);
	STDMETHODIMP GetForeColor(LONG *pValue);
	STDMETHODIMP SetForeColor(LONG Value);
	STDMETHODIMP GetHidden(LONG *pValue);
	STDMETHODIMP SetHidden(LONG Value);
	STDMETHODIMP GetEngrave(LONG *pValue);
	STDMETHODIMP SetEngrave(LONG Value);
	STDMETHODIMP GetItalic(LONG *pValue);
	STDMETHODIMP SetItalic(LONG Value);
	STDMETHODIMP GetKerning(float *pValue);
	STDMETHODIMP SetKerning(float Value);
	STDMETHODIMP GetLanguageID(LONG *pValue);
	STDMETHODIMP SetLanguageID(LONG Value);
	STDMETHODIMP GetName(BSTR *pbstr);
	STDMETHODIMP SetName(BSTR bstr);
	STDMETHODIMP GetOutline(LONG *pValue);
	STDMETHODIMP SetOutline(LONG Value);
	STDMETHODIMP GetPosition(float *pValue);
	STDMETHODIMP SetPosition(float Value);
	STDMETHODIMP GetProtected(LONG *pValue);
	STDMETHODIMP SetProtected(LONG Value);
	STDMETHODIMP GetShadow(LONG *pValue);
	STDMETHODIMP SetShadow(LONG Value);
	STDMETHODIMP GetSize(float *pValue);
	STDMETHODIMP SetSize(float Value);
	STDMETHODIMP GetSmallCaps(LONG *pValue);
	STDMETHODIMP SetSmallCaps(LONG Value);
	STDMETHODIMP GetSpacing(float *pValue);
	STDMETHODIMP SetSpacing(float Value);
	STDMETHODIMP GetStrikeThrough(LONG *pValue);
	STDMETHODIMP SetStrikeThrough(LONG Value);
	STDMETHODIMP GetSubscript(LONG *pValue);
	STDMETHODIMP SetSubscript(LONG Value);
	STDMETHODIMP GetSuperscript(LONG *pValue);
	STDMETHODIMP SetSuperscript(LONG Value);
	STDMETHODIMP GetUnderline(LONG *pValue);
	STDMETHODIMP SetUnderline(LONG Value);
	STDMETHODIMP GetWeight(LONG *pValue);
	STDMETHODIMP SetWeight(LONG Value);

//@access Private ITextFont helper methods
private:
	HRESULT	GetParameter (LONG *pParm, DWORD dwMask, LONG Type, LONG *pValue);
	HRESULT	SetParameter (LONG *pParm, DWORD dwMask, LONG Type, LONG Value);
	HRESULT	EffectGetter (LONG *ptomBool, DWORD dwMask);
	HRESULT	EffectSetter (LONG Value, DWORD dwMask, DWORD dwEffect);
	HRESULT	FormatSetter (DWORD dwMask);
	HRESULT	UpdateFormat ();
};


class CTxtPara : public ITextPara, CTxtFormat
{
	friend	CTxtRange;
	friend	CTxtEdit;

	CParaFormat	_PF;
	DWORD		_dwMask;			// PARAFORMAT2 mask
	union
	{
	  DWORD _dwFlags;				// All together now
	  struct
	  {
		DWORD _fApplyLater : 1;		// Delay call to _prg->ParaFormatSetter()
		DWORD _fCacheParms : 1;		// Update _PF now but not on GetXs
	  };
	};
	LONG		_rgxTabs[MAX_TAB_STOPS];// Place to store tabs till committed

public:
	CTxtPara(CTxtRange *prg);
	~CTxtPara();

	// IUnknown methods
	STDMETHODIMP QueryInterface(REFIID riid, LPVOID * ppvObj);
	STDMETHOD_(ULONG, AddRef)();
	STDMETHOD_(ULONG, Release)();

	// IDispatch methods
	STDMETHODIMP GetTypeInfoCount(UINT * pctinfo);
	STDMETHODIMP GetTypeInfo(UINT itinfo, LCID lcid, ITypeInfo ** pptinfo);
	STDMETHODIMP GetIDsOfNames(REFIID riid, OLECHAR ** rgszNames, UINT cNames,
							 LCID lcid, DISPID * rgdispid) ;
	STDMETHODIMP Invoke(DISPID dispidMember, REFIID riid, LCID lcid, WORD wFlags,
					  DISPPARAMS * pdispparams, VARIANT * pvarResult,
					  EXCEPINFO * pexcepinfo, UINT * puArgErr) ;

	// ITextPara methods
	STDMETHODIMP GetDuplicate(ITextPara **ppPara);
	STDMETHODIMP SetDuplicate(ITextPara *pPara);
	STDMETHODIMP CanChange(LONG *pB);
	STDMETHODIMP IsEqual(ITextPara *pPara, LONG *pB);
	STDMETHODIMP Reset(LONG Value);
	STDMETHODIMP GetStyle(LONG *pValue);
	STDMETHODIMP SetStyle(LONG Value);

	STDMETHODIMP GetAlignment(LONG *pValue);
	STDMETHODIMP SetAlignment(LONG Value);
	STDMETHODIMP GetHyphenation(LONG *pValue);
	STDMETHODIMP SetHyphenation(LONG Value);
	STDMETHODIMP GetFirstLineIndent(float *pValue);
	STDMETHODIMP GetKeepTogether(LONG *pValue);
	STDMETHODIMP SetKeepTogether(LONG Value);
	STDMETHODIMP GetKeepWithNext(LONG *pValue);
	STDMETHODIMP SetKeepWithNext(LONG Value);
	STDMETHODIMP GetLeftIndent(float *pValue);
	STDMETHODIMP GetLineSpacing(float *pValue);
	STDMETHODIMP GetLineSpacingRule(LONG *pValue);
    STDMETHODIMP GetListAlignment(LONG * pValue);
    STDMETHODIMP SetListAlignment(LONG Value);
    STDMETHODIMP GetListLevelIndex(LONG * pValue);
    STDMETHODIMP SetListLevelIndex(LONG Value);
    STDMETHODIMP GetListStart(LONG * pValue);
    STDMETHODIMP SetListStart(LONG Value);
    STDMETHODIMP GetListTab(float * pValue);
    STDMETHODIMP SetListTab(float Value);
	STDMETHODIMP GetListType(LONG *pValue);
	STDMETHODIMP SetListType(LONG Value);
	STDMETHODIMP GetNoLineNumber(LONG *pValue);
	STDMETHODIMP SetNoLineNumber(LONG Value);
	STDMETHODIMP GetPageBreakBefore(LONG *pValue);
	STDMETHODIMP SetPageBreakBefore(LONG Value);
	STDMETHODIMP GetRightIndent(float *pValue);
	STDMETHODIMP SetRightIndent(float Value);
	STDMETHODIMP SetIndents(float StartIndent, float LeftIndent, float RightIndent);
	STDMETHODIMP SetLineSpacing(LONG LineSpacingRule, float LineSpacing);
	STDMETHODIMP GetSpaceAfter(float *pValue);
	STDMETHODIMP SetSpaceAfter(float Value);
	STDMETHODIMP GetSpaceBefore(float *pValue);
	STDMETHODIMP SetSpaceBefore(float Value);
	STDMETHODIMP GetWidowControl(LONG *pValue);
	STDMETHODIMP SetWidowControl(LONG Value);

	STDMETHODIMP GetTabCount(LONG *pValue);
	STDMETHODIMP AddTab(float tpPos, LONG tbAlign, LONG tbLeader);
	STDMETHODIMP ClearAllTabs();
	STDMETHODIMP DeleteTab(float tbPos);
	STDMETHODIMP GetTab(LONG iTab, float *ptbPos, LONG *ptbAlign, LONG *ptbLeader);

	HRESULT	FormatSetter (DWORD dwMask);

//@access Private ITextPara helper methods
private:
	HRESULT	GetParameter (LONG *pParm, DWORD dwMask, LONG Type, LONG *pValue);
	HRESULT	SetParameter (LONG *pParm, DWORD dwMask, LONG Type, LONG Value);
	HRESULT	EffectGetter (LONG * ptomBool, DWORD dwMask);
	HRESULT	EffectSetter (LONG Value, DWORD dwMask);
	HRESULT	UpdateFormat ();
	void	CheckTabsAddRef();
};

#endif
