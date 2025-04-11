        #if defined( __arm__ ) || defined( _M_ARM )
        #   error CMAKE_TARGET_ARCH_arm
        #elif defined( __aarch64__ ) || defined( _M_ARM64 ) || defined(_M_ARM64EC)
        #   error CMAKE_TARGET_ARCH_arm64
        #elif defined( __i386__ ) || defined( _M_IX86 )
        #   error CMAKE_TARGET_ARCH_x86
        #elif defined( __x86_64__ ) || defined( _M_X64 )
        #   error CMAKE_TARGET_ARCH_x64
        #else
        #   error CMAKE_TARGET_ARCH_unknown
        #endif
