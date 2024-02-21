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
#include <pthread.h>

SNSBEGIN

struct IRwLock {
    virtual ~IRwLock() {}
    virtual void LockShared(void) = 0;
    virtual void UnlockShared(void) = 0;
    virtual void LockExclusive(void) = 0;
    virtual void UnlockExclusive(void) = 0;
};

class SRwLockPosix : public IRwLock
{
public:

    SRwLockPosix()
    {
        ::pthread_rwlock_init(&m_rwlock, NULL);
    }

    ~SRwLockPosix()
    {
        ::pthread_rwlock_destroy(&m_rwlock);
    }

    void LockShared() override
    {
        ::pthread_rwlock_rdlock(&m_rwlock);
    }

    void UnlockShared(void) override
    {
        ::pthread_rwlock_unlock(&m_rwlock);
    }

    void LockExclusive(void) override
    {
        ::pthread_rwlock_wrlock(&m_rwlock);
    }

    void UnlockExclusive(void) override
    {
        ::pthread_rwlock_unlock(&m_rwlock);
    }

private:
    pthread_rwlock_t m_rwlock;
};


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
    impl = new SRwLockPosix();
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

