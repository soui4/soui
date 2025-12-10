/*
auPNG -- derived from LodePNG version 20100808

Copyright (c) 2005-2010 Lode Vandevenne
Copyright (c) 2010 Sean Middleditch
Copyright (c) 2019 Helco

This software is provided 'as-is', without any express or implied
warranty. In no event will the authors be held liable for any damages
arising from the use of this software.

Permission is granted to anyone to use this software for any purpose,
including commercial applications, and to alter it and redistribute it
freely, subject to the following restrictions:

                1. The origin of this software must not be misrepresented; you must not
                claim that you wrote the original software. If you use this software
                in a product, an acknowledgment in the product documentation would be
                appreciated but is not required.

                2. Altered source versions must be plainly marked as such, and must not be
                misrepresented as being the original software.

                3. This notice may not be removed or altered from any source
                distribution.
*/
#ifndef _UPNG_INTERNAL_H_
#define _UPNG_INTERNAL_H_

#include "upng.h"

#define MAKE_BYTE(b) ((b)&0xFF)
#define MAKE_WORD(a, b) ((MAKE_BYTE(a) << 8) | MAKE_BYTE(b))
#define MAKE_WORD_PTR(p) MAKE_WORD((p)[0], (p)[1])
#define MAKE_DWORD(a, b, c, d) ((MAKE_BYTE(a) << 24) | (MAKE_BYTE(b) << 16) | (MAKE_BYTE(c) << 8) | MAKE_BYTE(d))
#define MAKE_DWORD_PTR(p) MAKE_DWORD((p)[0], (p)[1], (p)[2], (p)[3])

#define CHUNK_IHDR MAKE_DWORD('I', 'H', 'D', 'R')
#define CHUNK_IDAT MAKE_DWORD('I', 'D', 'A', 'T')
#define CHUNK_TEXT MAKE_DWORD('t', 'E', 'X', 't')
#define CHUNK_tRNS MAKE_DWORD('t', 'R', 'N', 'S')
#define CHUNK_PLTE MAKE_DWORD('P', 'L', 'T', 'E')
#define CHUNK_OFFS MAKE_DWORD('o', 'F', 'F', 's')
#define CHUNK_IEND MAKE_DWORD('I', 'E', 'N', 'D')
#define CHUNK_ACTL MAKE_DWORD('a', 'c', 'T', 'L')
#define CHUNK_FCTL MAKE_DWORD('f', 'c', 'T', 'L')
#define CHUNK_FDAT MAKE_DWORD('f', 'd', 'A', 'T')

#define FRAME_INDEX_NONE UINT_MAX

#define SET_ERROR(upng, code)          \
    do                                 \
    {                                  \
        (upng)->error = (code);        \
        (upng)->error_line = __LINE__; \
    } while (0)

#define CHECK_RET(upng, condition, errorCode)   \
    do                                          \
    {                                           \
        if (!(condition)) {                     \
            SET_ERROR((upng), (errorCode));     \
            return (upng)->error;               \
        }                                       \
    } while(0)

#define CHECK_GOTO(upng, condition, errorCode, label)   \
    do                                                  \
    {                                                   \
        if (!(condition)) {                             \
            SET_ERROR((upng), (errorCode));             \
            goto label;                                 \
        }                                               \
    } while(0)

#define upng_chunk_length(chunk) MAKE_DWORD_PTR(chunk)
#define upng_chunk_type(chunk) MAKE_DWORD_PTR((chunk) + 4)
#define upng_chunk_data(chunk) ((chunk) + 8)
#define upng_chunk_critical(chunk) (((chunk)[4] & 32) == 0)

typedef enum upng_state
{
    UPNG_ERROR = -1,
    UPNG_DECODED = 0,
    UPNG_HEADER = 1,
    UPNG_NEW = 2
} upng_state;


typedef enum upng_dispose_op
{
    UPNG_DISPOSE_OP_NONE = 0,
    UPNG_DISPOSE_OP_BACKGROUND = 1,
    UPNG_DISPOSE_OP_PREVIOUS = 2,

    UPNG_LAST_DISPOSE_OP = UPNG_DISPOSE_OP_PREVIOUS
} upng_dispose_op;

typedef enum upng_blend_op
{
    UPNG_BLEND_OP_SOURCE = 0,
    UPNG_BLEND_OP_OVER = 1,

    UPNG_LAST_BLEND_OP = UPNG_BLEND_OP_OVER
} upng_blend_op;

typedef struct upng_frame
{
    upng_rect rect;
    unsigned short delay_numerator;
    unsigned short delay_denominator;
    upng_dispose_op next_frame_dispose_op;
    upng_blend_op next_frame_blend_op;

    unsigned long data_chunk_offset; // of the first data chunk
    unsigned long compressed_size;
} upng_frame;

typedef struct upng_text
{
    char* buffer; // deallocate this

    const char *keyword; // but not these
    const char *text;
} upng_text;

#define kMax_TextChunk 10
struct upng_t
{
    upng_rgb *palette;
    uint8_t palette_entries;

    uint8_t *alpha;
    uint8_t alpha_entries;

    upng_color color_type;
    unsigned color_depth;
    upng_format format;

    unsigned int play_count;
    unsigned int frame_count;
    upng_frame* frames;
    upng_frame defaultImage;

    upng_text text[kMax_TextChunk];
    unsigned int text_count;

    upng_error error;
    unsigned error_line;

    upng_state state;
    upng_source source;

    const upng_frame* decodedFrame;
    uint8_t *buffer;          // frame buffer for composited image (full image size)
    unsigned long size;       // size of frame buffer
    unsigned int current_frame;
};

upng_error uz_inflate(uint8_t *out, unsigned long outsize, const uint8_t *in, unsigned long insize);


#endif//_UPNG_INTERNAL_H_