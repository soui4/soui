/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkBarriers_arm_DEFINED
#define SkBarriers_arm_DEFINED

// MSVC offers neither GCC inline assembly nor the __sync_* builtins, so on Windows/ARM64
// the ARM64 barrier intrinsics from <intrin.h> are used instead.  __dmb(_ARM64_BARRIER_SY)
// emits a real DMB SY, which is strictly stronger than the compiler-only barrier that
// SkBarriers_x86.h would otherwise provide on this weakly-ordered architecture.
// __clang__ is excluded on purpose: clang-cl understands asm volatile and __sync_synchronize,
// so it takes the portable branch below.
#if defined(_MSC_VER) && !defined(__clang__) && defined(_M_ARM64)
#  include <intrin.h>

static inline void sk_compiler_barrier() { _ReadWriteBarrier(); }

template <typename T>
T sk_acquire_load(T* ptr) {
    T val = *ptr;
    __dmb(_ARM64_BARRIER_SY);  // Issue a full barrier, which is an overkill acquire barrier.
    return val;
}

template <typename T>
T sk_consume_load(T* ptr) {
    T val = *ptr;
    // Unlike acquire, consume loads (data-dependent loads) are guaranteed not to reorder on ARM.
    // No memory barrier is needed, so we just use a compiler barrier.
    // C.f. http://preshing.com/20140709/the-purpose-of-memory_order_consume-in-cpp11/
    sk_compiler_barrier();
    return val;
}

template <typename T>
void sk_release_store(T* ptr, T val) {
    __dmb(_ARM64_BARRIER_SY);  // Issue a full barrier, which is an overkill release barrier.
    *ptr = val;
}

#else

static inline void sk_compiler_barrier() { asm volatile("" : : : "memory"); }

template <typename T>
T sk_acquire_load(T* ptr) {
    T val = *ptr;
    __sync_synchronize();  // Issue a full barrier, which is an overkill acquire barrier.
    return val;
}

template <typename T>
T sk_consume_load(T* ptr) {
    T val = *ptr;
    // Unlike acquire, consume loads (data-dependent loads) are guaranteed not to reorder on ARM.
    // No memory barrier is needed, so we just use a compiler barrier.
    // C.f. http://preshing.com/20140709/the-purpose-of-memory_order_consume-in-cpp11/
    sk_compiler_barrier();
    return val;
}

template <typename T>
void sk_release_store(T* ptr, T val) {
    __sync_synchronize();  // Issue a full barrier, which is an overkill release barrier.
    *ptr = val;
}

#endif

#endif//SkBarriers_x86_DEFINED
