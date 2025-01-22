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
#include "upng_internal.h"

#include <stdlib.h>
#include <string.h>
#include <limits.h>

/*Paeth predicter, used by PNG filter type 4*/
static int paeth_predictor(int a, int b, int c)
{
    int p = a + b - c;
    int pa = p > a ? p - a : a - p;
    int pb = p > b ? p - b : b - p;
    int pc = p > c ? p - c : c - p;

    if (pa <= pb && pa <= pc)
        return a;
    else if (pb <= pc)
        return b;
    else
        return c;
}

static void unfilter_scanline(upng_t *upng, uint8_t *recon, const uint8_t *scanline, const uint8_t *precon, unsigned long bytewidth, uint8_t filterType, unsigned long length)
{
    /*
        For PNG filter method 0
        unfilter a PNG image scanline by scanline. when the pixels are smaller than 1 byte, the filter works byte per byte (bytewidth = 1)
        precon is the previous unfiltered scanline, recon the result, scanline the current one
        the incoming scanlines do NOT include the filtertype byte, that one is given in the parameter filterType instead
        recon and scanline MAY be the same memory address! precon must be disjoint.
        */

    unsigned long i;
    switch (filterType)
    {
    case 0:
        for (i = 0; i < length; i++)
            recon[i] = scanline[i];
        break;
    case 1:
        for (i = 0; i < bytewidth; i++)
            recon[i] = scanline[i];
        for (i = bytewidth; i < length; i++)
            recon[i] = scanline[i] + recon[i - bytewidth];
        break;
    case 2:
        if (precon)
            for (i = 0; i < length; i++)
                recon[i] = scanline[i] + precon[i];
        else
            for (i = 0; i < length; i++)
                recon[i] = scanline[i];
        break;
    case 3:
        if (precon)
        {
            for (i = 0; i < bytewidth; i++)
                recon[i] = scanline[i] + precon[i] / 2;
            for (i = bytewidth; i < length; i++)
                recon[i] = scanline[i] + ((recon[i - bytewidth] + precon[i]) / 2);
        }
        else
        {
            for (i = 0; i < bytewidth; i++)
                recon[i] = scanline[i];
            for (i = bytewidth; i < length; i++)
                recon[i] = scanline[i] + recon[i - bytewidth] / 2;
        }
        break;
    case 4:
        if (precon)
        {
            for (i = 0; i < bytewidth; i++)
                recon[i] = (uint8_t)(scanline[i] + paeth_predictor(0, precon[i], 0));
            for (i = bytewidth; i < length; i++)
                recon[i] = (uint8_t)(scanline[i] + paeth_predictor(recon[i - bytewidth], precon[i], precon[i - bytewidth]));
        }
        else
        {
            for (i = 0; i < bytewidth; i++)
                recon[i] = scanline[i];
            for (i = bytewidth; i < length; i++)
                recon[i] = (uint8_t)(scanline[i] + paeth_predictor(recon[i - bytewidth], 0, 0));
        }
        break;
    default:
        SET_ERROR(upng, UPNG_EMALFORMED);
        break;
    }
}

static void unfilter(upng_t *upng, uint8_t *out, const uint8_t *in, unsigned w, unsigned h, unsigned bpp)
{
    /*
        For PNG filter method 0
        this function unfilters a single image (e.g. without interlacing this is called once, with Adam7 it's called 7 times)
        out must have enough bytes allocated already, in must have the scanlines + 1 filtertype byte per scanline
        w and h are image dimensions or dimensions of reduced image, bpp is bpp per pixel
        in and out are allowed to be the same memory address!
        */

    unsigned y;
    uint8_t *prevline = 0;

    unsigned long bytewidth = (bpp + 7) / 8; /*bytewidth is used for filtering, is 1 when bpp < 8, number of bytes per pixel otherwise */
    unsigned long linebytes = (w * bpp + 7) / 8;

    for (y = 0; y < h; y++)
    {
        unsigned long outindex = linebytes * y;
        unsigned long inindex = (1 + linebytes) * y; /*the extra filterbyte added to each row */
        uint8_t filterType = in[inindex];

        unfilter_scanline(upng, &out[outindex], &in[inindex + 1], prevline, bytewidth, filterType, linebytes);
        if (upng->error != UPNG_EOK)
        {
            return;
        }

        prevline = &out[outindex];
    }
}

static void remove_padding_bits(uint8_t *out, const uint8_t *in, unsigned long olinebits, unsigned long ilinebits, unsigned h)
{
    /*
        After filtering there are still padding bpp if scanlines have non multiple of 8 bit amounts. They need to be removed (except at last scanline of (Adam7-reduced) image) before working with pure image buffers for the Adam7 code, the color convert code and the output to the user.
        in and out are allowed to be the same buffer, in may also be higher but still overlapping; in must have >= ilinebits*h bpp, out must have >= olinebits*h bpp, olinebits must be <= ilinebits
        also used to move bpp after earlier such operations happened, e.g. in a sequence of reduced images from Adam7
        only useful if (ilinebits - olinebits) is a value in the range 1..7
        */
    unsigned y;
    unsigned long diff = ilinebits - olinebits;
    unsigned long obp = 0, ibp = 0; /*bit pointers */
    for (y = 0; y < h; y++)
    {
        unsigned long x;
        for (x = 0; x < olinebits; x++)
        {
            uint8_t bit = (uint8_t)((in[(ibp) >> 3] >> (7 - ((ibp)&0x7))) & 1);
            ibp++;

            if (bit == 0)
                out[(obp) >> 3] &= (uint8_t)(~(1 << (7 - ((obp)&0x7))));
            else
                out[(obp) >> 3] |= (1 << (7 - ((obp)&0x7)));
            ++obp;
        }
        ibp += diff;
    }
}

/*out must be buffer big enough to contain full image, and in must contain the full decompressed data from the IDAT chunks*/
static void post_process_scanlines(upng_t *upng, uint8_t *out, uint8_t *in, const upng_frame *frame)
{
    unsigned bpp = upng_get_bpp(upng);
    unsigned w = frame->rect.width;
    unsigned h = frame->rect.height;

    if (bpp == 0)
    {
        SET_ERROR(upng, UPNG_EMALFORMED);
        return;
    }

    if (bpp < 8 && w * bpp != ((w * bpp + 7) / 8) * 8)
    {
        unfilter(upng, in, in, w, h, bpp);
        if (upng->error != UPNG_EOK)
        {
            return;
        }
        // remove_padding_bits(out, in, w * bpp, ((w * bpp + 7) / 8) * 8, h);
        // fix for non-byte-aligned images
        unsigned aligned_width = ((w * bpp + 7) / 8) * 8;
        remove_padding_bits(in, in, aligned_width, aligned_width, h);
    }
    else
    {
        unfilter(upng, in, in, w, h, bpp); /*we can immediatly filter into the out buffer, no other steps needed */
    }
}

/*read a PNG, the result will be in the same color type as the PNG (hence "generic")*/
upng_error upng_decode_frame(upng_t *upng, const upng_frame* frame)
{
    uint8_t *compressed = NULL;
    unsigned long compressed_index = 0;
    unsigned long inflated_size;
    unsigned long chunk_offset;
    uint8_t chunk_header[12];
    upng_error error;

    /* parse the main header, if necessary */
    upng_header(upng);
    if (upng->error != UPNG_EOK)
    {
        return upng->error;
    }

    /* if we are not ready to decode the image, stop now */
    if (upng->state != UPNG_HEADER && upng->state != UPNG_DECODED)
    {
        return upng->error;
    }

    /* allocate enough space for the (compressed and filtered) image data */
    compressed = (uint8_t *)UPNG_MEM_ALLOC(frame->compressed_size);
    CHECK_RET(upng, compressed != NULL, UPNG_ENOMEM);

    /* scan through the chunks again, this time copying the values into
     * our compressed buffer.  there's no reason to validate anything a second time. */
    chunk_offset = frame->data_chunk_offset;
    while (chunk_offset < upng->source.size)
    {
        unsigned long chunk_data_offset = upng_chunk_data(chunk_offset);
        unsigned long length;

        /* read chunk header */
        CHECK_GOTO(upng, upng->source.read(upng->source.user, chunk_offset, chunk_header, 12) == 12, UPNG_EREAD, error);

        length = upng_chunk_length(chunk_header);

        /* collect data chunks */
        if (upng_chunk_type(chunk_header) == CHUNK_IDAT)
        {
            CHECK_GOTO(upng, upng->source.read(upng->source.user, chunk_data_offset, compressed + compressed_index, length) == length, UPNG_EREAD, error);
            compressed_index += length;
        }
        else if (upng_chunk_type(chunk_header) == CHUNK_FDAT)
        {
            CHECK_GOTO(upng, upng->source.read(upng->source.user, chunk_data_offset + 4, compressed + compressed_index, length - 4) == length - 4, UPNG_EREAD, error);
            compressed_index += length - 4;
        }
        else if (upng_chunk_type(chunk_header) == CHUNK_IEND || upng_chunk_type(chunk_header) == CHUNK_FCTL)
        {
            break;
        }

        chunk_offset += length + 12;
    }

    /* allocate space to store inflated (but still filtered) data */
    int width_aligned_bytes = (frame->rect.width * upng_get_bpp(upng) + 7) / 8;
    inflated_size = (width_aligned_bytes * frame->rect.height) + frame->rect.height; // pad byte
    if (upng->size < inflated_size)
    {
        if (upng->buffer != NULL)
        {
            UPNG_MEM_FREE(upng->buffer);
        }
        upng->buffer = (uint8_t*)UPNG_MEM_ALLOC(inflated_size);
        CHECK_GOTO(upng, upng->buffer != NULL, UPNG_ENOMEM, error);
        upng->size = inflated_size;
    }

    /* decompress image data */
    error = uz_inflate(upng->buffer, inflated_size, compressed, frame->compressed_size);
    CHECK_GOTO(upng, error == UPNG_EOK, error, error);
    UPNG_MEM_FREE(compressed);

    /* unfilter scanlines */
    post_process_scanlines(upng, upng->buffer, upng->buffer, frame);

    if (upng->error != UPNG_EOK)
    {
        UPNG_MEM_FREE(upng->buffer);
        upng->buffer = NULL;
        upng->size = 0;
    }
    else
    {
        upng->state = UPNG_DECODED;
        upng->decodedFrame = frame;
    }

    return upng->error;

error:
    if (compressed != NULL)
        UPNG_MEM_FREE(compressed);
    if (upng->buffer != NULL)
    {
        UPNG_MEM_FREE(upng->buffer);
        upng->buffer = NULL;
    }
    return upng->error;
}

upng_error upng_decode_default(upng_t* upng)
{
    return upng_decode_frame(upng, &upng->defaultImage);
}

unsigned upng_get_width(upng_t *upng)
{
    return upng->defaultImage.rect.width;
}

unsigned upng_get_height(upng_t *upng)
{
    return upng->defaultImage.rect.height;
}

upng_error upng_decode_next_frame(upng_t *upng)
{
    upng->current_frame = (upng->current_frame + 1) % upng->frame_count;
    upng_decode_frame(upng, &upng->frames[upng->current_frame]);
    return UPNG_EOK;
}
