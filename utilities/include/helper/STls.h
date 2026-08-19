#ifndef __STLS__H__
#define __STLS__H__

#include <utilities-def.h>

SNSBEGIN

/**
 * @brief Thread-local storage ID. 0 means invalid/uninitialized.
 * @details Declared as a static variable and passed by address to STls
 *          methods. The ID is allocated lazily on first use, in a
 *          thread-safe manner.
 *          Usage:
 *            static STlsId s_id = 0;
 */
typedef volatile long STlsId;

/**
 * @brief Destructor callback invoked by STls::Cleanup for each non-NULL value.
 * @param value The pointer previously stored via STls::Set.
 */
typedef void (*STlsDestructor)(void *value);

/**
 * @class STls
 * @brief Thread-local storage helper, modeled after SDL3 TLS.
 * @details Provides lazy ID allocation, per-thread independent storage with
 *          destructor callbacks, and manual cleanup on thread exit.
 *          The storage layout is a growable per-thread array indexed by a
 *          globally allocated slot index, mirroring SDL3's design.
 *
 *          Typical usage:
 *          @code
 *          static STlsId s_id = 0;
 *          void *val = STls::Get(&s_id);
 *          STls::Set(&s_id, myData, myDestructor);
 *          // before the thread exits:
 *          STls::Cleanup();
 *          @endcode
 */
class UTILITIES_API STls
{
public:
    /**
     * @brief Gets the TLS value for the current thread.
     * @param id Pointer to a STlsId. Lazily allocated on first Set; Get on
     *           an unallocated ID returns NULL.
     * @return The stored value, or NULL if not set.
     */
    static void *Get(STlsId *id);

    /**
     * @brief Sets the TLS value for the current thread.
     * @param id Pointer to a STlsId. Lazily allocated on first use.
     * @param value Pointer to store (may be NULL to clear).
     * @param destructor Optional destructor called by Cleanup. Passing NULL
     *                   removes the previous destructor.
     * @return TRUE on success, FALSE on allocation failure.
     */
    static BOOL Set(STlsId *id, void *value, STlsDestructor destructor);

    /**
     * @brief Cleans up all TLS data for the current thread.
     * @details Invokes registered destructors in slot order, then frees the
     *          per-thread storage. Should be called before a thread exits.
     *          Safe to call when no TLS was ever set (no-op).
     */
    static void Cleanup();
};

SNSEND

#endif // __STLS__H__
