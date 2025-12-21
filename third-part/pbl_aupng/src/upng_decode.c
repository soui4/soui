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
		}else{
			// remove_padding_bits(out, in, w * bpp, ((w * bpp + 7) / 8) * 8, h);
			// fix for non-byte-aligned images
			unsigned aligned_width = ((w * bpp + 7) / 8) * 8;
			remove_padding_bits(in, in, aligned_width, aligned_width, h);
		}
    }
    else
    {
        unfilter(upng, in, in, w, h, bpp); /*we can immediatly filter into the out buffer, no other steps needed */
    }
}

/* Blend frame data into frame buffer according to next_frame_blend_op and offset */
/* Convert RGB (no alpha) to RGBA8 format */
static uint8_t* convert_rgb_to_rgba(const uint8_t *rgb_data, unsigned width, unsigned height, unsigned *out_size)
{
    unsigned long rgba_size = width * height * 4;
    uint8_t *rgba_data = NULL;
    unsigned i, pixel_count;

    rgba_data = (uint8_t*)UPNG_MEM_ALLOC(rgba_size);
    if (rgba_data == NULL)
    {
        return NULL;
    }

    *out_size = rgba_size;
    pixel_count = width * height;

    /* Convert RGB to RGBA by adding opaque alpha */
    for (i = 0; i < pixel_count; i++)
    {
        rgba_data[i * 4 + 0] = rgb_data[i * 3 + 0]; /* R */
        rgba_data[i * 4 + 1] = rgb_data[i * 3 + 1]; /* G */
        rgba_data[i * 4 + 2] = rgb_data[i * 3 + 2]; /* B */
        rgba_data[i * 4 + 3] = 255;                  /* A = opaque */
    }

    return rgba_data;
}

/* Convert grayscale+alpha (LUMA) to RGBA8 format */
static uint8_t* convert_luma_to_rgba(const uint8_t *luma_data, unsigned width, unsigned height, unsigned *out_size)
{
    unsigned long rgba_size = width * height * 4;
    uint8_t *rgba_data = NULL;
    unsigned i, pixel_count;

    rgba_data = (uint8_t*)UPNG_MEM_ALLOC(rgba_size);
    if (rgba_data == NULL)
    {
        return NULL;
    }

    *out_size = rgba_size;
    pixel_count = width * height;

    /* Convert LA to RGBA by duplicating luminance to RGB */
    for (i = 0; i < pixel_count; i++)
    {
        uint8_t lum = luma_data[i * 2 + 0];
        uint8_t alpha = luma_data[i * 2 + 1];

        rgba_data[i * 4 + 0] = lum;   /* R = luminance */
        rgba_data[i * 4 + 1] = lum;   /* G = luminance */
        rgba_data[i * 4 + 2] = lum;   /* B = luminance */
        rgba_data[i * 4 + 3] = alpha; /* A */
    }

    return rgba_data;
}

/* Convert grayscale (LUM, no alpha) to RGBA8 format */
static uint8_t* convert_lum_to_rgba(const uint8_t *lum_data, unsigned width, unsigned height, unsigned bitdepth, unsigned *out_size)
{
    unsigned long rgba_size = width * height * 4;
    uint8_t *rgba_data = NULL;
    unsigned x, y;

    rgba_data = (uint8_t*)UPNG_MEM_ALLOC(rgba_size);
    if (rgba_data == NULL)
    {
        return NULL;
    }

    *out_size = rgba_size;

    /* Convert grayscale to RGBA */
    if (bitdepth == 8)
    {
        /* 8-bit grayscale */
        for (y = 0; y < height; y++)
        {
            for (x = 0; x < width; x++)
            {
                unsigned pixel_pos = y * width + x;
                uint8_t lum = lum_data[pixel_pos];

                rgba_data[pixel_pos * 4 + 0] = lum; /* R */
                rgba_data[pixel_pos * 4 + 1] = lum; /* G */
                rgba_data[pixel_pos * 4 + 2] = lum; /* B */
                rgba_data[pixel_pos * 4 + 3] = 255; /* A = opaque */
            }
        }
    }
    else if (bitdepth == 16)
    {
        /* 16-bit grayscale - use high byte */
        for (y = 0; y < height; y++)
        {
            for (x = 0; x < width; x++)
            {
                unsigned pixel_pos = y * width + x;
                uint8_t lum = lum_data[pixel_pos * 2]; /* High byte */

                rgba_data[pixel_pos * 4 + 0] = lum; /* R */
                rgba_data[pixel_pos * 4 + 1] = lum; /* G */
                rgba_data[pixel_pos * 4 + 2] = lum; /* B */
                rgba_data[pixel_pos * 4 + 3] = 255; /* A = opaque */
            }
        }
    }
    else
    {
        /* 1, 2, 4-bit grayscale */
        for (y = 0; y < height; y++)
        {
            for (x = 0; x < width; x++)
            {
                unsigned pixel_pos = y * width + x;
                uint8_t lum = 0;

                if (bitdepth == 4)
                {
                    unsigned byte_pos = pixel_pos / 2;
                    unsigned bit_offset = (pixel_pos % 2) ? 0 : 4;
                    lum = (lum_data[byte_pos] >> bit_offset) & 0x0F;
                    lum = (lum << 4) | lum; /* Scale 4-bit to 8-bit */
                }
                else if (bitdepth == 2)
                {
                    unsigned byte_pos = pixel_pos / 4;
                    unsigned bit_offset = 6 - ((pixel_pos % 4) * 2);
                    lum = (lum_data[byte_pos] >> bit_offset) & 0x03;
                    lum = (lum << 6) | (lum << 4) | (lum << 2) | lum; /* Scale 2-bit to 8-bit */
                }
                else if (bitdepth == 1)
                {
                    unsigned byte_pos = pixel_pos / 8;
                    unsigned bit_offset = 7 - (pixel_pos % 8);
                    lum = (lum_data[byte_pos] >> bit_offset) & 0x01;
                    lum = lum ? 255 : 0; /* 1-bit: 0 or 255 */
                }

                rgba_data[pixel_pos * 4 + 0] = lum; /* R */
                rgba_data[pixel_pos * 4 + 1] = lum; /* G */
                rgba_data[pixel_pos * 4 + 2] = lum; /* B */
                rgba_data[pixel_pos * 4 + 3] = 255; /* A = opaque */
            }
        }
    }

    return rgba_data;
}

/* Convert palette indexed image data to RGBA8 format */
static uint8_t* convert_palette_to_rgba(upng_t *upng, const uint8_t *indexed_data, unsigned width, unsigned height, unsigned *out_size)
{
    unsigned bpp = upng_get_bpp(upng);
    unsigned bitdepth = upng_get_bitdepth(upng);
    unsigned long rgba_size = width * height * 4; /* RGBA = 4 bytes per pixel */
    uint8_t *rgba_data = NULL;
    unsigned x, y;
    upng_rgb *palette = upng->palette;
    uint8_t *alpha = upng->alpha;
    unsigned palette_entries = upng->palette_entries;
    unsigned alpha_entries = upng->alpha_entries;

    if (palette == NULL)
    {
        SET_ERROR(upng, UPNG_EMALFORMED);
        return NULL;
    }

    rgba_data = (uint8_t*)UPNG_MEM_ALLOC(rgba_size);
    if (rgba_data == NULL)
    {
        SET_ERROR(upng, UPNG_ENOMEM);
        return NULL;
    }

    *out_size = rgba_size;

    /* Convert indexed pixels to RGBA */
    for (y = 0; y < height; y++)
    {
        for (x = 0; x < width; x++)
        {
            unsigned palette_index = 0;
            unsigned pixel_pos = y * width + x;
            unsigned rgba_offset = pixel_pos * 4;

            /* Extract palette index based on bit depth */
            if (bitdepth == 8)
            {
                palette_index = indexed_data[pixel_pos];
            }
            else if (bitdepth == 4)
            {
                unsigned byte_pos = pixel_pos / 2;
                unsigned bit_offset = (pixel_pos % 2) ? 0 : 4;
                palette_index = (indexed_data[byte_pos] >> bit_offset) & 0x0F;
            }
            else if (bitdepth == 2)
            {
                unsigned byte_pos = pixel_pos / 4;
                unsigned bit_offset = 6 - ((pixel_pos % 4) * 2);
                palette_index = (indexed_data[byte_pos] >> bit_offset) & 0x03;
            }
            else if (bitdepth == 1)
            {
                unsigned byte_pos = pixel_pos / 8;
                unsigned bit_offset = 7 - (pixel_pos % 8);
                palette_index = (indexed_data[byte_pos] >> bit_offset) & 0x01;
            }

            /* Bounds check */
            if (palette_index >= palette_entries)
            {
                palette_index = 0; /* Default to first color if out of bounds */
            }

            /* Set RGB from palette */
            rgba_data[rgba_offset + 0] = palette[palette_index].r;
            rgba_data[rgba_offset + 1] = palette[palette_index].g;
            rgba_data[rgba_offset + 2] = palette[palette_index].b;

            /* Set alpha from tRNS chunk if available, otherwise opaque */
            if (alpha != NULL && palette_index < alpha_entries)
            {
                rgba_data[rgba_offset + 3] = alpha[palette_index];
            }
            else
            {
                rgba_data[rgba_offset + 3] = 255; /* Opaque */
            }
        }
    }

    return rgba_data;
}

static void blend_frame_over_buffer(upng_t *upng, uint8_t *frameBuffer, const uint8_t *frameData, const upng_frame *frame, unsigned actual_bpp)
{
    unsigned bpp = actual_bpp;
    unsigned bytes_per_pixel = (bpp + 7) / 8;
    unsigned frame_width = frame->rect.width;
    unsigned frame_height = frame->rect.height;
    unsigned image_width = upng->defaultImage.rect.width;
    unsigned x_offset = frame->rect.x_offset;
    unsigned y_offset = frame->rect.y_offset;
    unsigned y;

    if (frame->next_frame_blend_op == UPNG_BLEND_OP_SOURCE)
    {
        /* Replace: directly copy frame data to frame buffer at offset position */
        for (y = 0; y < frame_height; y++)
        {
            unsigned dest_y = y_offset + y;
            unsigned dest_offset = (dest_y * image_width + x_offset) * bytes_per_pixel;
            unsigned src_offset = y * frame_width * bytes_per_pixel;
            memcpy(frameBuffer + dest_offset, frameData + src_offset, frame_width * bytes_per_pixel);
        }
    }
    else /* UPNG_BLEND_OP_OVER */
    {
        /* Alpha blend: blend frame data over existing frame buffer */
        for (y = 0; y < frame_height; y++)
        {
            unsigned x;
            unsigned dest_y = y_offset + y;
            for (x = 0; x < frame_width; x++)
            {
                unsigned dest_x = x_offset + x;
                unsigned dest_offset = (dest_y * image_width + dest_x) * bytes_per_pixel;
                unsigned src_offset = (y * frame_width + x) * bytes_per_pixel;

                if (bpp == 32) /* RGBA8 */
                {
                    uint8_t src_r = frameData[src_offset + 0];
                    uint8_t src_g = frameData[src_offset + 1];
                    uint8_t src_b = frameData[src_offset + 2];
                    uint8_t src_a = frameData[src_offset + 3];

                    if (src_a == 255)
                    {
                        frameBuffer[dest_offset + 0] = src_r;
                        frameBuffer[dest_offset + 1] = src_g;
                        frameBuffer[dest_offset + 2] = src_b;
                        frameBuffer[dest_offset + 3] = src_a;
                    }
                    else if (src_a != 0)
                    {
                        uint8_t dst_r = frameBuffer[dest_offset + 0];
                        uint8_t dst_g = frameBuffer[dest_offset + 1];
                        uint8_t dst_b = frameBuffer[dest_offset + 2];
                        uint8_t dst_a = frameBuffer[dest_offset + 3];

                        unsigned inv_alpha = 255 - src_a;
                        frameBuffer[dest_offset + 0] = (src_r * src_a + dst_r * inv_alpha) / 255;
                        frameBuffer[dest_offset + 1] = (src_g * src_a + dst_g * inv_alpha) / 255;
                        frameBuffer[dest_offset + 2] = (src_b * src_a + dst_b * inv_alpha) / 255;
                        frameBuffer[dest_offset + 3] = src_a + (dst_a * inv_alpha) / 255;
                    }
                }
                else
                {
                    /* For non-RGBA formats, just copy */
                    memcpy(frameBuffer + dest_offset, frameData + src_offset, bytes_per_pixel);
                }
            }
        }
    }
}

/*read a PNG, the result will be in the same color type as the PNG (hence "generic")*/
static upng_error upng_decode_frame(upng_t *upng, const upng_frame* frame, uint8_t *out, uint32_t out_size)
{
    uint8_t *compressed = NULL;
    uint8_t *frameData = NULL;
    unsigned long compressed_index = 0;
    unsigned long inflated_size;
    unsigned long chunk_offset;
    uint8_t chunk_header[12];
    upng_error error;
    int width_aligned_bytes;
    unsigned bpp;
    unsigned bytes_per_pixel;
    unsigned image_width, image_height;
    unsigned long frameBufferSize;
    unsigned is_first_frame;
    int needs_conversion = 0; /* Flag for formats that need conversion to RGBA8 */
    unsigned original_bpp;
    unsigned original_bytes_per_pixel;
	uint8_t * prev = NULL;

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

    /* Check if this format needs conversion to RGBA8 */
    /* Convert: Palette, RGB (no alpha), LUMA (grayscale+alpha), LUM (grayscale) */
    needs_conversion = (upng->color_type == UPNG_PLT ||
                        upng->color_type == UPNG_RGB ||
                        upng->color_type == UPNG_LUMA ||
                        upng->color_type == UPNG_LUM);

    /* Store original bpp for decoding */
    original_bpp = upng_get_bpp(upng);
    original_bytes_per_pixel = (original_bpp + 7) / 8;

    /* If format needs conversion, we'll convert to RGBA8, so use RGBA8 dimensions for buffer */
    if (needs_conversion)
    {
        bpp = 32; /* RGBA8 */
        bytes_per_pixel = 4;
    }
    else
    {
        bpp = original_bpp;
        bytes_per_pixel = original_bytes_per_pixel;
    }

    image_width = upng->defaultImage.rect.width;
    image_height = upng->defaultImage.rect.height;
    frameBufferSize = image_width * image_height * bytes_per_pixel;
    is_first_frame = (upng->decodedFrame == NULL);

    /* Allocate frame buffer for full image if not already allocated */
    if (upng->buffer == NULL)
    {//using upng->buffer to hold the decoded image.
        upng->buffer = (uint8_t*)UPNG_MEM_ALLOC(frameBufferSize);
        CHECK_RET(upng, upng->buffer != NULL, UPNG_ENOMEM);
        upng->size = frameBufferSize;
        memset(upng->buffer, 0, frameBufferSize);
    }
    if(frame->next_frame_dispose_op == UPNG_DISPOSE_OP_PREVIOUS && out){
        //backup the previous frame
        prev = (uint8_t*)UPNG_MEM_ALLOC(frameBufferSize);
        memcpy(prev,out,frameBufferSize);
    }
    /* Initialize buffer for first frame */
    if (is_first_frame)
    {
        /* Clear frame buffer for first frame */
        memset(upng->buffer, 0, frameBufferSize);
    }


    /* allocate enough space for the (compressed and filtered) image data */
    compressed = (uint8_t *)UPNG_MEM_ALLOC(frame->compressed_size);
    CHECK_GOTO(upng, compressed != NULL, UPNG_ENOMEM, error);

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

    /* allocate space to store inflated (but still filtered) frame data */
    /* Use original bpp for decompression */
    width_aligned_bytes = (frame->rect.width * original_bpp + 7) / 8;
    inflated_size = (width_aligned_bytes * frame->rect.height) + frame->rect.height; // pad byte
    frameData = (uint8_t*)UPNG_MEM_ALLOC(inflated_size);
    CHECK_GOTO(upng, frameData != NULL, UPNG_ENOMEM, error);

    /* decompress image data */
    error = uz_inflate(frameData, inflated_size, compressed, frame->compressed_size);
    CHECK_GOTO(upng, error == UPNG_EOK, error, error);
    UPNG_MEM_FREE(compressed);
    compressed = NULL;

    /* unfilter scanlines */
    post_process_scanlines(upng, frameData, frameData, frame);

    if (upng->error != UPNG_EOK)
    {
        goto error;
    }

    /* Convert to RGBA8 if needed */
    if (needs_conversion)
    {
        uint8_t *rgba_data = NULL;
        unsigned rgba_size = 0;

        /* Convert based on color type */
        if (upng->color_type == UPNG_PLT)
        {
            /* Palette to RGBA8 */
            rgba_data = convert_palette_to_rgba(upng, frameData, frame->rect.width, frame->rect.height, &rgba_size);
        }
        else if (upng->color_type == UPNG_RGB)
        {
            /* RGB to RGBA8 (add opaque alpha) */
            rgba_data = convert_rgb_to_rgba(frameData, frame->rect.width, frame->rect.height, &rgba_size);
        }
        else if (upng->color_type == UPNG_LUMA)
        {
            /* Grayscale+Alpha to RGBA8 */
            rgba_data = convert_luma_to_rgba(frameData, frame->rect.width, frame->rect.height, &rgba_size);
        }
        else if (upng->color_type == UPNG_LUM)
        {
            /* Grayscale to RGBA8 */
            rgba_data = convert_lum_to_rgba(frameData, frame->rect.width, frame->rect.height, upng->color_depth, &rgba_size);
        }

        if (rgba_data == NULL)
        {
            SET_ERROR(upng, UPNG_ENOMEM);
            goto error;
        }

        /* Free original data and replace with RGBA data */
        UPNG_MEM_FREE(frameData);
        frameData = rgba_data;

        /* Use RGBA8 bpp for blending converted data */
        bpp = 32;
        bytes_per_pixel = 4;
    }

    /* Blend frame data into frame buffer according to offset and next_frame_blend_op */
    blend_frame_over_buffer(upng, upng->buffer, frameData, frame, bpp);

    UPNG_MEM_FREE(frameData);
    frameData = NULL;

    /* Copy the rendered frame to output buffer (for caller to display) */
    if(out != NULL && out_size >= frameBufferSize)
        memcpy(out, upng->buffer, frameBufferSize);

    if (frame->next_frame_dispose_op == UPNG_DISPOSE_OP_BACKGROUND)
    {
        unsigned y;
        for (y = 0; y < frame->rect.height; y++)
        {
            unsigned dest_y = frame->rect.y_offset + y;
            unsigned dest_offset = (dest_y * image_width + frame->rect.x_offset) * bytes_per_pixel;
            memset(upng->buffer + dest_offset, 0, frame->rect.width * bytes_per_pixel);
        }
    }
    else if (frame->next_frame_dispose_op == UPNG_DISPOSE_OP_PREVIOUS && prev)
    {
        memcpy(upng->buffer, prev, frameBufferSize);
        UPNG_MEM_FREE(prev);
    }

    upng->state = UPNG_DECODED;
    upng->decodedFrame = frame;
    return upng->error;

error:
    if (compressed != NULL)
        UPNG_MEM_FREE(compressed);
    if (frameData != NULL)
        UPNG_MEM_FREE(frameData);
    return upng->error;
}

upng_error upng_decode_default(upng_t* upng, uint8_t *out, uint32_t out_size)
{
    return upng_decode_frame(upng, &upng->defaultImage, out, out_size);
}

unsigned upng_get_width(upng_t *upng)
{
    return upng->defaultImage.rect.width;
}

unsigned upng_get_height(upng_t *upng)
{
    return upng->defaultImage.rect.height;
}

upng_error upng_decode_next_frame(upng_t *upng, uint8_t *out, uint32_t out_size)
{
    upng->current_frame = (upng->current_frame + 1) % upng->frame_count;
    return upng_decode_frame(upng, &upng->frames[upng->current_frame],out,out_size);
}
