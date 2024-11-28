/*
 *  Copyright (c) 2012 The WebRTC project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#include <helper/SRwLock.h>
#include <windows.h>

SNSBEGIN

struct IRwLock {
    virtual ~IRwLock() {}
    virtual void LockShared(void) = 0;
    virtual void UnlockShared(void) = 0;
    virtual void LockExclusive(void) = 0;
    virtual void UnlockExclusive(void) = 0;
};

#ifdef _WIN32
static bool native_rw_locks_supported = false;
static bool module_load_attempted = false;
static HMODULE library = NULL;

typedef void (WINAPI* InitializeSRWLock)(PSRWLOCK);

typedef void (WINAPI* AcquireSRWLockExclusive)(PSRWLOCK);
typedef void (WINAPI* ReleaseSRWLockExclusive)(PSRWLOCK);

typedef void (WINAPI* AcquireSRWLockShared)(PSRWLOCK);
typedef void (WINAPI* ReleaseSRWLockShared)(PSRWLOCK);

InitializeSRWLock       initialize_srw_lock;
AcquireSRWLockExclusive acquire_srw_lock_exclusive;
AcquireSRWLockShared    acquire_srw_lock_shared;
ReleaseSRWLockShared    release_srw_lock_shared;
ReleaseSRWLockExclusive release_srw_lock_exclusive;

static bool LoadModule() {
    if (module_load_attempted) {
        return native_rw_locks_supported;
    }
    module_load_attempted = true;
    // Use native implementation if supported (i.e Vista+)
    library = LoadLibrary(TEXT("Kernel32.dll"));
    if (!library) {
        return false;
    }

    initialize_srw_lock =
        (InitializeSRWLock)GetProcAddress(library, "InitializeSRWLock");

    acquire_srw_lock_exclusive =
        (AcquireSRWLockExclusive)GetProcAddress(library,
            "AcquireSRWLockExclusive");
    release_srw_lock_exclusive =
        (ReleaseSRWLockExclusive)GetProcAddress(library,
            "ReleaseSRWLockExclusive");
    acquire_srw_lock_shared =
        (AcquireSRWLockShared)GetProcAddress(library, "AcquireSRWLockShared");
    release_srw_lock_shared =
        (ReleaseSRWLockShared)GetProcAddress(library, "ReleaseSRWLockShared");

    if (initialize_srw_lock && acquire_srw_lock_exclusive &&
        release_srw_lock_exclusive && acquire_srw_lock_shared &&
        release_srw_lock_shared) {
        native_rw_locks_supported = true;
    }
    return native_rw_locks_supported;
}

class SRwLockWinXP : public IRwLock
{
public:
    SRwLockWinXP()
    {
        m_nReaders = 0;
        InitializeCriticalSection(&m_csWriter);
        InitializeCriticalSection(&m_csReader);
        m_evtReaders = CreateEvent(NULL, TRUE, TRUE, NULL);

    }

    ~SRwLockWinXP()
    {
        WaitForSingleObject(m_evtReaders, INFINITE);
        CloseHandle(m_evtReaders);
        DeleteCriticalSection(&m_csWriter);
        DeleteCriticalSection(&m_csReader);
    }

    void LockShared(void) override
    {
        EnterCriticalSection(&m_csWriter);
        EnterCriticalSection(&m_csReader);
        if (++m_nReaders == 1)
        {
            ::ResetEvent(m_evtReaders);
        }
        LeaveCriticalSection(&m_csReader);
        LeaveCriticalSection(&m_csWriter);
    }

    void UnlockShared(void) override
    {
        EnterCriticalSection(&m_csReader);
        if (--m_nReaders == 0)
        {
            ::SetEvent(m_evtReaders);
        }
        LeaveCriticalSection(&m_csReader);
    }

    void LockExclusive(void) override
    {
        EnterCriticalSection(&m_csWriter);
        WaitForSingleObject(m_evtReaders, INFINITE);
    }

    void UnlockExclusive(void) override
    {
        LeaveCriticalSection(&m_csWriter);
    }

private:
    CRITICAL_SECTION m_csWriter;
    CRITICAL_SECTION m_csReader;
    int m_nReaders;
    HANDLE m_evtReaders;
};

class SRwLockWin7 : public IRwLock {
    SRWLOCK m_rwlock;
public:
    SRwLockWin7() {
        initialize_srw_lock(&m_rwlock);
    }

    void LockExclusive() override{
        acquire_srw_lock_exclusive(&m_rwlock);
    }

    void UnlockExclusive() override {
        release_srw_lock_exclusive(&m_rwlock);
    }

    void LockShared() override {
        acquire_srw_lock_shared(&m_rwlock);
    }

    void UnlockShared() override {
        release_srw_lock_shared(&m_rwlock);
    }
};
#else

class SRwLockPosix : public IRwLock {
    SRWLOCK m_rwlock;
public:
    SRwLockPosix() {
        InitializeSRWLock(&m_rwlock);
    }
    ~SRwLockPosix() {
        UninitializeSRWLock(&m_rwlock);
    }

    void LockExclusive() override {
        AcquireSRWLockExclusive(&m_rwlock);
    }

    void UnlockExclusive() override {
        ReleaseSRWLockExclusive(&m_rwlock);
    }

    void LockShared() override {
        AcquireSRWLockShared(&m_rwlock);
    }

    void UnlockShared() override {
        ReleaseSRWLockShared(&m_rwlock);
    }
};
#endif//_WIN32

SRwLock::SRwLock()
{
#ifdef _WIN32
    if (LoadModule()) {
        impl = new SRwLockWin7();
    }
    else
    {
        impl = new SRwLockWinXP();
    }
#else
    impl = new SRwLockPosix;
#endif
}

SRwLock::~SRwLock() {
    delete impl;
}

void SRwLock::LockExclusive()
{
    impl->LockExclusive();
}

void SRwLock::UnlockExclusive()
{
    impl->UnlockExclusive();
}

void SRwLock::LockShared()
{
    impl->LockShared();
}

void SRwLock::UnlockShared()
{
    impl->UnlockShared();
}


SNSEND

