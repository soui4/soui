#ifndef _STREAM_I_H
#define _STREAM_I_H

#include <unknown-i.h>


typedef struct tagSTATSTG
{
    wchar_t *pwcsName;
    DWORD type;
    ULARGE_INTEGER cbSize;
    FILETIME mtime;
    FILETIME ctime;
    FILETIME atime;
    DWORD grfMode;
    DWORD grfLocksSupported;
    GUID clsid;
    DWORD grfStateBits;
    DWORD reserved;
} STATSTG;

#undef INTERFACE
#define INTERFACE IStream
DECLARE_INTERFACE_(ISequentialStream,IUnknown)
{
    DECLARE_CLASS_SIID(0,0,0,0,0,2)

    STDMETHOD_(HRESULT,Read)(THIS_            
            /* [length_is][size_is][out] */ void *pv,
            /* [in] */ ULONG cb,
            /* [out] */ ULONG *pcbRead) PURE;

    STDMETHOD_(HRESULT, Write)(THIS_  
            /* [size_is][in] */ const void *pv,
            /* [in] */ ULONG cb,
            /* [out] */ ULONG *pcbWritten) PURE;
        
};

#undef IStream
#define INTERFACE IStream
DECLARE_INTERFACE_(IStream,ISequentialStream)
{
    DECLARE_CLASS_SIID(0,0,0,0,0,3)

        virtual /* [local] */ HRESULT STDMETHODCALLTYPE Seek( 
            /* [in] */ LARGE_INTEGER dlibMove,
            /* [in] */ DWORD dwOrigin,
            /* [out] */ ULARGE_INTEGER *plibNewPosition) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetSize( 
            /* [in] */ ULARGE_INTEGER libNewSize) = 0;
        
        virtual /* [local] */ HRESULT STDMETHODCALLTYPE CopyTo( 
            /* [unique][in] */ IStream *pstm,
            /* [in] */ ULARGE_INTEGER cb,
            /* [out] */ ULARGE_INTEGER *pcbRead,
            /* [out] */ ULARGE_INTEGER *pcbWritten) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Commit( 
            /* [in] */ DWORD grfCommitFlags) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Revert( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE LockRegion( 
            /* [in] */ ULARGE_INTEGER libOffset,
            /* [in] */ ULARGE_INTEGER cb,
            /* [in] */ DWORD dwLockType) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE UnlockRegion( 
            /* [in] */ ULARGE_INTEGER libOffset,
            /* [in] */ ULARGE_INTEGER cb,
            /* [in] */ DWORD dwLockType) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Stat( 
            /* [out] */ STATSTG *pstatstg,
            /* [in] */ DWORD grfStatFlag) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Clone( 
            /* [out] */ IStream **ppstm) = 0;
};

#endif