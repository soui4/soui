#ifndef _SRW_LOCK_H_
#define _SRW_LOCK_H_

#include <utilities-def.h>
#include <helper/SNoCopyable.hpp>

SNSBEGIN

struct IRwLock;

/**
 * @class SRwLock
 * @brief A read-write lock class.
 * @details This class provides methods to acquire and release shared and exclusive locks.
 * @note This class is non-copyable.
 */
class UTILITIES_API SRwLock : public SNoCopyable {
public:
    /**
     * @brief Constructor.
     * Initializes a new read-write lock.
     */
    SRwLock();

    /**
     * @brief Destructor.
     * Destroys the read-write lock.
     */
    ~SRwLock();

    /**
     * @brief Acquires an exclusive lock.
     * @details This method blocks the calling thread until the exclusive lock is available.
     */
    void LockExclusive();

    /**
     * @brief Releases an exclusive lock.
     * @details This method releases the exclusive lock, allowing other threads to acquire it.
     */
    void UnlockExclusive();

    /**
     * @brief Acquires a shared lock.
     * @details This method blocks the calling thread until the shared lock is available.
     */
    void LockShared();

    /**
     * @brief Releases a shared lock.
     * @details This method releases the shared lock, allowing other threads to acquire it.
     */
    void UnlockShared();

private:
    IRwLock* impl; /**< Pointer to the read-write lock implementation. */
};

/**
 * @class SAutoReadLock
 * @brief Auto-lock class for managing shared locks.
 * @details This class automatically acquires a shared lock upon construction and releases it upon destruction.
 * @note This class is non-copyable.
 */
class UTILITIES_API SAutoReadLock : public SNoCopyable {
public:
    /**
     * @brief Constructor.
     * @param plock Pointer to the SRwLock object to manage.
     * @details Acquires a shared lock.
     */
    SAutoReadLock(SRwLock *plock) : m_pLock(plock) {
        m_pLock->LockShared();
    }

    /**
     * @brief Destructor.
     * @details Releases the shared lock.
     */
    ~SAutoReadLock() {
        m_pLock->UnlockShared();
    }

private:
    SRwLock *m_pLock; /**< Pointer to the managed SRwLock object. */
};

/**
 * @class SAutoWriteLock
 * @brief Auto-lock class for managing exclusive locks.
 * @details This class automatically acquires an exclusive lock upon construction and releases it upon destruction.
 * @note This class is non-copyable.
 */
class UTILITIES_API SAutoWriteLock : public SNoCopyable {
public:
    /**
     * @brief Constructor.
     * @param plock Pointer to the SRwLock object to manage.
     * @details Acquires an exclusive lock.
     */
    SAutoWriteLock(SRwLock *plock) : m_pLock(plock) {
        m_pLock->LockExclusive();
    }

    /**
     * @brief Destructor.
     * @details Releases the exclusive lock.
     */
    ~SAutoWriteLock() {
        m_pLock->UnlockExclusive();
    }

private:
    SRwLock *m_pLock; /**< Pointer to the managed SRwLock object. */
};

SNSEND

#endif  // _SRW_LOCK_H_