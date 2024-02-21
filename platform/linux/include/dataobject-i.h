#ifndef __DATA_OBJECT_I_H_
#define __DATA_OBJECT_I_H_

#include <unknown-i.h>
#include <storage-i.h>
#include <stream-i.h>

typedef struct tagDVTARGETDEVICE
{
    DWORD tdSize;
    WORD tdDriverNameOffset;
    WORD tdDeviceNameOffset;
    WORD tdPortNameOffset;
    WORD tdExtDevmodeOffset;
    BYTE tdData[1];
} DVTARGETDEVICE;

typedef /* [wire_marshal] */ WORD CLIPFORMAT;

typedef CLIPFORMAT *LPCLIPFORMAT;

typedef struct tagFORMATETC
{
    CLIPFORMAT cfFormat;
    DVTARGETDEVICE *ptd;
    DWORD dwAspect;
    LONG lindex;
    DWORD tymed;
} FORMATETC;

typedef struct tagFORMATETC *LPFORMATETC;

typedef struct tagSTGMEDIUM
{
    DWORD tymed;
    union {
        HBITMAP hBitmap;
        HGLOBAL hGlobal;
        // LPOLESTR lpszFileName;
        IStream *pstm;
        IStorage *pstg;
        /* Empty union arm */
    };
    IUnknown *pUnkForRelease;
} uSTGMEDIUM;

typedef /* [wire_marshal] */ uSTGMEDIUM STGMEDIUM;

#undef INTERFACE
#define INTERFACE IDataObject
DECLARE_INTERFACE_(IDataObject, IUnknown)
{
    DECLARE_CLASS_SIID(0, 0, 0, 0, 0, 2)
    STDMETHOD_(HRESULT, GetData)(THIS_ FORMATETC * pformatetcIn, STGMEDIUM * pmedium) PURE;
    STDMETHOD_(HRESULT, SetData)
    (THIS_
             /* [unique][in] */ FORMATETC
         * pformatetc,
     /* [unique][in] */ STGMEDIUM * pmedium,
     /* [in] */ BOOL fRelease) PURE;
};

#endif