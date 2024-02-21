#ifndef _STORAGE_I_H
#define _STORAGE_I_H
#include <unknown-i.h>
#include <stream-i.h>

#undef INTERFACE
#define INTERFACE IEnumSTATSTG
DECLARE_INTERFACE_(IEnumSTATSTG, IUnknown)
{
    DECLARE_CLASS_SIID(0, 0, 0, 0, 0, 4)

    virtual /* [local] */ HRESULT STDMETHODCALLTYPE Next(
        /* [in] */ ULONG celt,
        /* [length_is][size_is][out] */ STATSTG * rgelt,
        /* [out] */ ULONG * pceltFetched)
        = 0;

    virtual HRESULT STDMETHODCALLTYPE Skip(
        /* [in] */ ULONG celt)
        = 0;

    virtual HRESULT STDMETHODCALLTYPE Reset(void) = 0;

    virtual HRESULT STDMETHODCALLTYPE Clone(
        /* [out] */ __RPC__deref_out_opt IEnumSTATSTG * *ppenum)
        = 0;
};

#undef INTERFACE
#define INTERFACE IStorage
DECLARE_INTERFACE_(IStorage, IUnknown)
{
    DECLARE_CLASS_SIID(0, 0, 0, 0, 0, 5)

    virtual HRESULT STDMETHODCALLTYPE CreateStream(
        /* [string][in] */ __RPC__in const OLECHAR *pwcsName,
        /* [in] */ DWORD grfMode,
        /* [in] */ DWORD reserved1,
        /* [in] */ DWORD reserved2,
        /* [out] */ __RPC__deref_out_opt IStream **ppstm)
        = 0;

    virtual /* [local] */ HRESULT STDMETHODCALLTYPE OpenStream(
        /* [string][in] */ const OLECHAR *pwcsName,
        /* [unique][in] */ void *reserved1,
        /* [in] */ DWORD grfMode,
        /* [in] */ DWORD reserved2,
        /* [out] */ IStream **ppstm)
        = 0;

    virtual HRESULT STDMETHODCALLTYPE CreateStorage(
        /* [string][in] */ __RPC__in const OLECHAR *pwcsName,
        /* [in] */ DWORD grfMode,
        /* [in] */ DWORD reserved1,
        /* [in] */ DWORD reserved2,
        /* [out] */ __RPC__deref_out_opt IStorage **ppstg)
        = 0;

    virtual HRESULT STDMETHODCALLTYPE OpenStorage(
        /* [string][unique][in] */ __RPC__in_opt const OLECHAR *pwcsName,
        /* [unique][in] */ __RPC__in_opt IStorage *pstgPriority,
        /* [in] */ DWORD grfMode,
        /* [unique][in] */ __RPC__deref_opt_in_opt SNB snbExclude,
        /* [in] */ DWORD reserved,
        /* [out] */ __RPC__deref_out_opt IStorage **ppstg)
        = 0;

    virtual /* [local] */ HRESULT STDMETHODCALLTYPE CopyTo(
        /* [in] */ DWORD ciidExclude,
        /* [size_is][unique][in] */ const IID *rgiidExclude,
        /* [unique][in] */ SNB snbExclude,
        /* [unique][in] */ IStorage *pstgDest)
        = 0;

    virtual HRESULT STDMETHODCALLTYPE MoveElementTo(
        /* [string][in] */ __RPC__in const OLECHAR *pwcsName,
        /* [unique][in] */ __RPC__in_opt IStorage *pstgDest,
        /* [string][in] */ __RPC__in const OLECHAR *pwcsNewName,
        /* [in] */ DWORD grfFlags)
        = 0;

    virtual HRESULT STDMETHODCALLTYPE Commit(
        /* [in] */ DWORD grfCommitFlags)
        = 0;

    virtual HRESULT STDMETHODCALLTYPE Revert(void) = 0;

    virtual /* [local] */ HRESULT STDMETHODCALLTYPE EnumElements(
        /* [in] */ DWORD reserved1,
        /* [size_is][unique][in] */ void *reserved2,
        /* [in] */ DWORD reserved3,
        /* [out] */ IEnumSTATSTG **ppenum)
        = 0;

    virtual HRESULT STDMETHODCALLTYPE DestroyElement(
        /* [string][in] */ __RPC__in const OLECHAR *pwcsName)
        = 0;

    virtual HRESULT STDMETHODCALLTYPE RenameElement(
        /* [string][in] */ __RPC__in const OLECHAR *pwcsOldName,
        /* [string][in] */ __RPC__in const OLECHAR *pwcsNewName)
        = 0;

    virtual HRESULT STDMETHODCALLTYPE SetElementTimes(
        /* [string][unique][in] */ __RPC__in_opt const OLECHAR *pwcsName,
        /* [unique][in] */ __RPC__in_opt const FILETIME *pctime,
        /* [unique][in] */ __RPC__in_opt const FILETIME *patime,
        /* [unique][in] */ __RPC__in_opt const FILETIME *pmtime)
        = 0;

    virtual HRESULT STDMETHODCALLTYPE SetClass(
        /* [in] */ __RPC__in REFCLSID clsid)
        = 0;

    virtual HRESULT STDMETHODCALLTYPE SetStateBits(
        /* [in] */ DWORD grfStateBits,
        /* [in] */ DWORD grfMask)
        = 0;

    virtual HRESULT STDMETHODCALLTYPE Stat(
        /* [out] */ __RPC__out STATSTG * pstatstg,
        /* [in] */ DWORD grfStatFlag)
        = 0;
};

#endif //_STORAGE_I_H