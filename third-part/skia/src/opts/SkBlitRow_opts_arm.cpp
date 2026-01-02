/*
 * Copyright 2012 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkBlitRow.h"
#include "SkColorPriv.h"
#include "SkDither.h"
#include "SkMathPriv.h"
#include "SkUtils.h"
#include "SkUtilsArm.h"

// Define USE_NEON_CODE to indicate that we need to build NEON routines
#define USE_NEON_CODE  (!SK_ARM_NEON_IS_NONE)

// Define USE_ARM_CODE to indicate that we need to build ARM routines
#define USE_ARM_CODE   (!SK_ARM_NEON_IS_ALWAYS)

#if USE_NEON_CODE
  #include "SkBlitRow_opts_arm_neon.h"
#endif

#if USE_ARM_CODE
#include <arm_neon.h>
static void S32A_D565_Opaque(uint16_t* SK_RESTRICT dst,
                             const SkPMColor* SK_RESTRICT src, int count,
                             U8CPU alpha, int /*x*/, int /*y*/) {
    SkASSERT(255 == alpha);

    while (count >= 8) {
        // Load 8 source pixels (32-bit RGBA)
        uint32x4_t src01 = vld1q_u32((const uint32_t*)src);
        uint32x4_t src23 = vld1q_u32((const uint32_t*)(src + 4));
        src += 8;

        // Extract color channels for first 4 pixels
        uint32x4_t a01 = vshrq_n_u32(src01, 24);  // Extract alpha
        uint32x4_t r01 = vandq_u32(vshrq_n_u32(src01, 16), vdupq_n_u32(0xFF));
        uint32x4_t g01 = vandq_u32(vshrq_n_u32(src01, 8), vdupq_n_u32(0xFF));
        uint32x4_t b01 = vandq_u32(src01, vdupq_n_u32(0xFF));

        // Extract color channels for second 4 pixels
        uint32x4_t a23 = vshrq_n_u32(src23, 24);
        uint32x4_t r23 = vandq_u32(vshrq_n_u32(src23, 16), vdupq_n_u32(0xFF));
        uint32x4_t g23 = vandq_u32(vshrq_n_u32(src23, 8), vdupq_n_u32(0xFF));
        uint32x4_t b23 = vandq_u32(src23, vdupq_n_u32(0xFF));

        // Convert to 16-bit (5-6-5 format)
        // R: 5 bits, G: 6 bits, B: 5 bits
        uint32x4_t r01_5 = vshrq_n_u32(r01, 3);  // 8-bit -> 5-bit
        uint32x4_t g01_6 = vshrq_n_u32(g01, 2);  // 8-bit -> 6-bit
        uint32x4_t b01_5 = vshrq_n_u32(b01, 3);  // 8-bit -> 5-bit

        uint32x4_t r23_5 = vshrq_n_u32(r23, 3);
        uint32x4_t g23_6 = vshrq_n_u32(g23, 2);
        uint32x4_t b23_5 = vshrq_n_u32(b23, 3);

        // Pack into 16-bit RGB565: RRRRRGGGGGGBBBBB
        uint32x4_t rgb01 = vorrq_u32(vshlq_n_u32(r01_5, 11), vorrq_u32(vshlq_n_u32(g01_6, 5), b01_5));
        uint32x4_t rgb23 = vorrq_u32(vshlq_n_u32(r23_5, 11), vorrq_u32(vshlq_n_u32(g23_6, 5), b23_5));

        // Narrow to 16-bit and store
        uint16x4_t rgb01_16 = vmovn_u32(rgb01);
        uint16x4_t rgb23_16 = vmovn_u32(rgb23);

        vst1_u16(dst, rgb01_16);
        vst1_u16(dst + 4, rgb23_16);
        dst += 8;
        count -= 8;
    }


    // Handle remaining pixels
    while (count > 0) {
        SkPMColor c = *src++;
        uint32_t a = SkGetPackedA32(c);

        if (a == 255) {
            // Opaque pixel - fast path
            uint32_t r = SkGetPackedR32(c) >> 3;
            uint32_t g = SkGetPackedG32(c) >> 2;
            uint32_t b = SkGetPackedB32(c) >> 3;
            *dst++ = (r << 11) | (g << 5) | b;
        } else if (a != 0) {
            // Semi-transparent pixel
            uint16_t d = *dst;
            uint32_t dst_b = (d & 0x001F) << 3;
            uint32_t dst_g = ((d >> 5) & 0x003F) << 2;
            uint32_t dst_r = ((d >> 11) & 0x001F) << 3;

            uint32_t scale = 255 - a;
            dst_b = (dst_b * scale + SkGetPackedB32(c) * a + 127) / 255;
            dst_g = (dst_g * scale + SkGetPackedG32(c) * a + 127) / 255;
            dst_r = (dst_r * scale + SkGetPackedR32(c) * a + 127) / 255;

            *dst++ = ((dst_r >> 3) << 11) | ((dst_g >> 2) << 5) | (dst_b >> 3);
        } else {
            // Fully transparent
            dst++;
        }
        count--;
    }
}

static void S32A_Opaque_BlitRow32_arm(SkPMColor* SK_RESTRICT dst,
                                  const SkPMColor* SK_RESTRICT src,
                                  int count, U8CPU alpha) {

    SkASSERT(255 == alpha);


    uint32x4_t mask = vdupq_n_u32(0xFF00FF);

    while (count >= 4) {
        // Load 4 source and destination pixels
        uint32x4_t src_pixels = vld1q_u32((const uint32_t*)src);
        uint32x4_t dst_pixels = vld1q_u32((uint32_t*)dst);

        // Extract alpha from source (high byte)
        uint32x4_t src_alpha = vshrq_n_u32(src_pixels, 24);

        // Calculate dst_scale = 256 - src_alpha
        uint32x4_t dst_scale = vsubq_u32(vdupq_n_u32(256), src_alpha);

        // Extract dst BR components (blue and red)
        uint32x4_t dst_br = vandq_u32(dst_pixels, mask);
        // Extract dst AG components (alpha and green)
        uint32x4_t dst_ag = vandq_u32(vshrq_n_u32(dst_pixels, 8), mask);

        // Scale dst components
        uint32x4_t scaled_br = vmulq_u32(dst_br, dst_scale);
        uint32x4_t scaled_ag = vmulq_u32(dst_ag, dst_scale);

        // Shift and mask to get final dst components
        uint32x4_t dst_br_final = vandq_u32(vshrq_n_u32(scaled_br, 8), mask);
        uint32x4_t dst_ag_final = vandq_u32(scaled_ag, vshlq_n_u32(mask, 8));

        // Combine dst_br and dst_ag
        uint32x4_t dst_scaled = vorrq_u32(dst_br_final, dst_ag_final);

        // Add source
        uint32x4_t result = vaddq_u32(src_pixels, dst_scaled);

        // Store result
        vst1q_u32((uint32_t*)dst, result);

        src += 4;
        dst += 4;
        count -= 4;
    }


    // Handle remaining pixels
    while (count > 0) {
        uint32_t src_pixel = *src++;
        uint32_t src_a = src_pixel >> 24;
        uint32_t dst_scale = 256 - src_a;

        uint32_t dst_pixel = *dst;
        uint32_t dst_br = dst_pixel & 0xFF00FF;
        uint32_t dst_ag = (dst_pixel >> 8) & 0xFF00FF;

        uint32_t scaled_br = ((dst_br * dst_scale) >> 8) & 0xFF00FF;
        uint32_t scaled_ag = (dst_ag * dst_scale) & 0xFF00FF00;

        *dst++ = src_pixel + (scaled_br | scaled_ag);
        count--;
    }
}

/*
 * ARM NEON version of S32A_Blend_BlitRow32
 */
void S32A_Blend_BlitRow32_arm(SkPMColor* SK_RESTRICT dst,
                              const SkPMColor* SK_RESTRICT src,
                              int count, U8CPU alpha) {

    uint32x4_t mask = vdupq_n_u32(0xFF00FF);
    uint32_t src_scale = alpha + 1;
    uint32x4_t src_scale_vec = vdupq_n_u32(src_scale);

    while (count >= 4) {
        // Load 4 source and destination pixels
        uint32x4_t src_pixels = vld1q_u32((const uint32_t*)src);
        uint32x4_t dst_pixels = vld1q_u32((uint32_t*)dst);

        // Extract alpha from source (high byte)
        uint32x4_t src_alpha = vshrq_n_u32(src_pixels, 24);

        // Scale source alpha by input alpha: src_a = src_a * (alpha + 1)
        uint32x4_t scaled_src_a = vmulq_u32(src_alpha, src_scale_vec);
        scaled_src_a = vshrq_n_u32(scaled_src_a, 8);

        // Calculate dst_scale = 256 - scaled_src_a
        uint32x4_t dst_scale = vsubq_u32(vdupq_n_u32(256), scaled_src_a);

        // Scale source
        uint32x4_t src_br = vandq_u32(src_pixels, mask);
        uint32x4_t src_ag = vandq_u32(vshrq_n_u32(src_pixels, 8), mask);

        uint32x4_t scaled_src_br = vmulq_u32(src_br, src_scale_vec);
        uint32x4_t scaled_src_ag = vmulq_u32(src_ag, src_scale_vec);

        uint32x4_t src_br_final = vandq_u32(vshrq_n_u32(scaled_src_br, 8), mask);
        uint32x4_t src_ag_final = vandq_u32(scaled_src_ag, vshlq_n_u32(mask, 8));

        uint32x4_t src_scaled = vorrq_u32(src_br_final, src_ag_final);

        // Scale destination
        uint32x4_t dst_br = vandq_u32(dst_pixels, mask);
        uint32x4_t dst_ag = vandq_u32(vshrq_n_u32(dst_pixels, 8), mask);

        uint32x4_t scaled_dst_br = vmulq_u32(dst_br, dst_scale);
        uint32x4_t scaled_dst_ag = vmulq_u32(dst_ag, dst_scale);

        uint32x4_t dst_br_final = vandq_u32(vshrq_n_u32(scaled_dst_br, 8), mask);
        uint32x4_t dst_ag_final = vandq_u32(scaled_dst_ag, vshlq_n_u32(mask, 8));

        uint32x4_t dst_scaled = vorrq_u32(dst_br_final, dst_ag_final);

        // Combine scaled source and destination
        uint32x4_t result = vaddq_u32(src_scaled, dst_scaled);

        // Store result
        vst1q_u32((uint32_t*)dst, result);

        src += 4;
        dst += 4;
        count -= 4;
    }


    // Handle remaining pixels
    while (count > 0) {
        uint32_t src_pixel = *src++;
        uint32_t src_a = src_pixel >> 24;

        // Scale source alpha
        uint32_t scaled_src_a = (src_a * src_scale) >> 8;
        uint32_t dst_scale = 256 - scaled_src_a;

        // Scale source
        uint32_t src_br = src_pixel & 0xFF00FF;
        uint32_t src_ag = (src_pixel >> 8) & 0xFF00FF;

        uint32_t scaled_src_br = ((src_br * src_scale) >> 8) & 0xFF00FF;
        uint32_t scaled_src_ag = (src_ag * src_scale) & 0xFF00FF00;

        uint32_t src_scaled = scaled_src_br | scaled_src_ag;

        // Scale destination
        uint32_t dst_pixel = *dst;
        uint32_t dst_br = dst_pixel & 0xFF00FF;
        uint32_t dst_ag = (dst_pixel >> 8) & 0xFF00FF;

        uint32_t scaled_dst_br = ((dst_br * dst_scale) >> 8) & 0xFF00FF;
        uint32_t scaled_dst_ag = (dst_ag * dst_scale) & 0xFF00FF00;

        *dst++ = src_scaled + (scaled_dst_br | scaled_dst_ag);
        count--;
    }
}

///////////////////////////////////////////////////////////////////////////////

static const SkBlitRow::Proc sk_blitrow_platform_565_procs_arm[] = {
    // no dither
    // NOTE: For the functions below, we don't have a special version
    //       that assumes that each source pixel is opaque. But our S32A is
    //       still faster than the default, so use it.
    S32A_D565_Opaque,   // S32_D565_Opaque
    NULL,               // S32_D565_Blend
    S32A_D565_Opaque,   // S32A_D565_Opaque
    NULL,               // S32A_D565_Blend

    // dither
    NULL,   // S32_D565_Opaque_Dither
    NULL,   // S32_D565_Blend_Dither
    NULL,   // S32A_D565_Opaque_Dither
    NULL,   // S32A_D565_Blend_Dither
};

static const SkBlitRow::Proc32 sk_blitrow_platform_32_procs_arm[] = {
    NULL,   // S32_Opaque,
    NULL,   // S32_Blend,
    S32A_Opaque_BlitRow32_arm,   // S32A_Opaque,
    S32A_Blend_BlitRow32_arm     // S32A_Blend
};

#endif // USE_ARM_CODE

SkBlitRow::Proc SkBlitRow::PlatformProcs565(unsigned flags) {
    return SK_ARM_NEON_WRAP(sk_blitrow_platform_565_procs_arm)[flags];
}

SkBlitRow::Proc32 SkBlitRow::PlatformProcs32(unsigned flags) {
    return SK_ARM_NEON_WRAP(sk_blitrow_platform_32_procs_arm)[flags];
}

///////////////////////////////////////////////////////////////////////////////
#define Color32_arm  NULL
SkBlitRow::ColorProc SkBlitRow::PlatformColorProc() {
    return SK_ARM_NEON_WRAP(Color32_arm);
}

SkBlitRow::ColorRectProc PlatformColorRectProcFactory() {
    return NULL;
}
