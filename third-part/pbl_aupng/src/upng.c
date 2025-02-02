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


static upng_format determine_format(upng_t *upng)
{
    switch (upng->color_type)
    {
    case UPNG_PLT:
        switch (upng->color_depth)
        {
        case 1:
            return UPNG_INDEXED1;
        case 2:
            return UPNG_INDEXED2;
        case 4:
            return UPNG_INDEXED4;
        case 8:
            return UPNG_INDEXED8;
        default:
            return UPNG_BADFORMAT;
        }
    case UPNG_LUM:
        switch (upng->color_depth)
        {
        case 1:
            return UPNG_LUMINANCE1;
        case 2:
            return UPNG_LUMINANCE2;
        case 4:
            return UPNG_LUMINANCE4;
        case 8:
            return UPNG_LUMINANCE8;
        default:
            return UPNG_BADFORMAT;
        }
    case UPNG_RGB:
        switch (upng->color_depth)
        {
        case 8:
            return UPNG_RGB8;
        case 16:
            return UPNG_RGB16;
        default:
            return UPNG_BADFORMAT;
        }
    case UPNG_LUMA:
        switch (upng->color_depth)
        {
        case 1:
            return UPNG_LUMINANCE_ALPHA1;
        case 2:
            return UPNG_LUMINANCE_ALPHA2;
        case 4:
            return UPNG_LUMINANCE_ALPHA4;
        case 8:
            return UPNG_LUMINANCE_ALPHA8;
        default:
            return UPNG_BADFORMAT;
        }
    case UPNG_RGBA:
        switch (upng->color_depth)
        {
        case 8:
            return UPNG_RGBA8;
        case 16:
            return UPNG_RGBA16;
        default:
            return UPNG_BADFORMAT;
        }
    default:
        return UPNG_BADFORMAT;
    }
}

static void upng_free_source(upng_t *upng)
{
    if (upng->source.free != NULL)
        upng->source.free(upng->source.user);
    memset(&upng->source, 0, sizeof(upng->source));
}

/*search through the chunks, save information like palette, frames and texts*/
static upng_error upng_process_chunks(upng_t* upng)
{
    unsigned long chunk_offset;
    uint8_t chunk_header[12];
    unsigned int cur_frame_index = FRAME_INDEX_NONE;
    unsigned int next_sequence_number = 0;

    /* first byte of the first chunk after the header */
    chunk_offset = 33;

    /* scan through the chunks, finding the size of all IDAT chunks, and also
        * verify general well-formed-ness */
    while (chunk_offset < upng->source.size)
    {
        unsigned long chunk_data_offset = upng_chunk_data(chunk_offset);
        unsigned long length;

        /* make sure chunk header is not larger than the total compressed */
        CHECK_RET(upng, chunk_offset + 12 <= upng->source.size, UPNG_EMALFORMED);

        /* read chunk header */
        CHECK_RET(upng, upng->source.read(upng->source.user, chunk_offset, chunk_header, 12) == 12, UPNG_EREAD);

        /* get length; sanity check it */
        length = upng_chunk_length(chunk_header);
        CHECK_RET(upng, length < INT_MAX, UPNG_EMALFORMED);

        /* make sure chunk header+paylaod is not larger than the total compressed */
        CHECK_RET(upng, chunk_offset + length + 12 <= upng->source.size, UPNG_EMALFORMED);

        /* parse chunks */
        if (upng_chunk_type(chunk_header) == CHUNK_IDAT)
        {
            /* make sure no IDAT chunk comes after a fcTL chunk */
            CHECK_RET(upng, cur_frame_index == FRAME_INDEX_NONE || cur_frame_index == 0, UPNG_EMALFORMED);

            if (upng->defaultImage.data_chunk_offset == 0)
            {
                upng->defaultImage.data_chunk_offset = chunk_offset;
            }
            upng->defaultImage.compressed_size += length;

            /* is the main image also the first animation frame? */
            if (cur_frame_index == 0)
            {
                upng->frames[0] = upng->defaultImage;
            }
        }
        else if (upng_chunk_type(chunk_header) == CHUNK_FDAT)
        {
            /* make sure the acTL chunk was already processed at this point */
            CHECK_RET(upng, upng->frames != NULL, UPNG_EMALFORMED);

            /* check sequence number */
            uint8_t sequence_number[4];
            CHECK_RET(upng, upng->source.read(upng->source.user, chunk_data_offset, sequence_number, 4) == 4, UPNG_EREAD);
            CHECK_RET(upng, next_sequence_number == MAKE_DWORD_PTR(sequence_number), UPNG_EMALFORMED);
            next_sequence_number++;

            upng_frame* frame = &upng->frames[cur_frame_index];
            frame->compressed_size += length;
            if (frame->data_chunk_offset == 0)
                frame->data_chunk_offset = chunk_offset;
        }
        else if (upng_chunk_type(chunk_header) == CHUNK_ACTL)
        {
            /* make sure the acTL chunk is present only once and before the first IDAT */
            CHECK_RET(upng, upng->frames == NULL, UPNG_EMALFORMED);

            uint8_t data[8];
            CHECK_RET(upng, upng->source.read(upng->source.user, chunk_data_offset, data, 8) == 8, UPNG_EREAD);

            upng->frame_count = MAKE_DWORD_PTR(data);
            upng->play_count = MAKE_DWORD_PTR(data + 4);

            /* Allocate frames */
            upng->frames = (upng_frame*)UPNG_MEM_ALLOC(sizeof(upng_frame) * upng->frame_count);
            CHECK_RET(upng, upng->frames != NULL, UPNG_ENOMEM);
            memset(upng->frames, 0, sizeof(upng_frame) * upng->frame_count);
        }
        else if (upng_chunk_type(chunk_header) == CHUNK_FCTL)
        {
            /* contrary to specs acTL *has* to come before the first fcTL chunk */
            CHECK_RET(upng, upng->frames != NULL, UPNG_EUNSUPPORTED);

            uint8_t data[26];
            CHECK_RET(upng, upng->source.read(upng->source.user, chunk_data_offset, data, 26) == 26, UPNG_EREAD);

            /* check sequence number */
            unsigned int sequence_number = MAKE_DWORD_PTR(data);
            CHECK_RET(upng, next_sequence_number == sequence_number, UPNG_EMALFORMED);
            next_sequence_number++;

            /* read data into next frame structure */
            upng_frame* frame = &upng->frames[++cur_frame_index];
            frame->rect.width = MAKE_DWORD_PTR(data + 4);
            frame->rect.height = MAKE_DWORD_PTR(data + 8);
            frame->rect.x_offset = MAKE_DWORD_PTR(data + 12);
            frame->rect.y_offset = MAKE_DWORD_PTR(data + 16);
            frame->delay_numerator = MAKE_WORD_PTR(data + 20);
            frame->delay_denominator = MAKE_WORD_PTR(data + 22);
            frame->dispose_op = (upng_dispose_op)data[24];
            frame->blend_op = (upng_blend_op)data[25];
            frame->compressed_size = 0;

            /* validate data */
            CHECK_RET(upng, frame->rect.x_offset >= 0 && frame->rect.y_offset >= 0, UPNG_EMALFORMED);
            CHECK_RET(upng, frame->rect.width > 0 && frame->rect.height > 0, UPNG_EMALFORMED);
            CHECK_RET(upng, frame->rect.x_offset + frame->rect.width <= upng->defaultImage.rect.width, UPNG_EMALFORMED);
            CHECK_RET(upng, frame->rect.y_offset + frame->rect.height <= upng->defaultImage.rect.height, UPNG_EMALFORMED);
            CHECK_RET(upng, frame->dispose_op <= UPNG_LAST_DISPOSE_OP, UPNG_EUNSUPPORTED);
            CHECK_RET(upng, frame->blend_op <= UPNG_LAST_BLEND_OP, UPNG_EUNSUPPORTED);

            /* the first frame has special requirements */
            if (cur_frame_index == 0)
            {
                CHECK_RET(upng, frame->rect.x_offset == 0 && frame->rect.y_offset == 0, UPNG_EMALFORMED);
                CHECK_RET(upng, frame->rect.width == upng->defaultImage.rect.width && frame->rect.height == upng->defaultImage.rect.height, UPNG_EMALFORMED);
            }
        }
        else if (upng_chunk_type(chunk_header) == CHUNK_OFFS)
        {
            uint8_t data[8];
            CHECK_RET(upng, upng->source.read(upng->source.user, chunk_data_offset, data, 8) == 8, UPNG_EREAD);

            upng->defaultImage.rect.x_offset = MAKE_DWORD_PTR(data);
            upng->defaultImage.rect.y_offset = MAKE_DWORD_PTR(data + 4);
        }
        else if (upng_chunk_type(chunk_header) == CHUNK_PLTE)
        {
            upng->palette_entries = (uint8_t)(length / 3); // 3 bytes per color entry
            if (upng->palette)
            {
                UPNG_MEM_FREE(upng->palette);
                upng->palette = NULL;
            }
            upng->palette = UPNG_MEM_ALLOC(length);

            CHECK_RET(upng, upng->source.read(upng->source.user, chunk_data_offset, upng->palette, length) == length, UPNG_EREAD);
        }
        else if (upng_chunk_type(chunk_header) == CHUNK_tRNS)
        {
            upng->alpha_entries = (uint8_t)length;
            if (upng->alpha)
            {
                UPNG_MEM_FREE(upng->alpha);
                upng->alpha = NULL;
            }
            upng->alpha = UPNG_MEM_ALLOC(length);

            CHECK_RET(upng, upng->source.read(upng->source.user, chunk_data_offset, upng->alpha, length) == length, UPNG_EREAD);
        }
        else if (upng_chunk_type(chunk_header) == CHUNK_TEXT)
        {
            char* buffer = upng->text[upng->text_count].buffer = UPNG_MEM_ALLOC(length + 1);
            CHECK_RET(upng, buffer != NULL, UPNG_ENOMEM);

            CHECK_RET(upng, upng->source.read(upng->source.user, chunk_data_offset, buffer, length) == length, UPNG_EREAD);

            // Split into keyword and text (separated by null byte)
            char* terminator = (char*)memchr(buffer, '\0', length);
            CHECK_RET(upng, terminator != NULL, UPNG_EMALFORMED);

            upng->text[upng->text_count].keyword = buffer;
            upng->text[upng->text_count].text = terminator + 1;
            buffer[length] = '\0';

            upng->text_count++;
        }
        else if (upng_chunk_type(chunk_header) == CHUNK_IEND)
        {
            break;
        }
        else
        {
            CHECK_RET(upng, !upng_chunk_critical(chunk_header), UPNG_EUNSUPPORTED);
        }

        chunk_offset += length + 12;
    }

    return upng->error;
}

/*read the information from the header and store it in the upng_Info. return value is error*/
upng_error upng_header(upng_t *upng)
{
    /* if we have an error state, bail now */
    if (upng->error != UPNG_EOK)
    {
        return upng->error;
    }

    /* if the state is not NEW (meaning we are ready to parse the header), stop now */
    if (upng->state != UPNG_NEW)
    {
        return upng->error;
    }

    /* minimum length of a valid PNG file is 29 bytes
        * FIXME: verify this against the specification, or
        * better against the actual code below */
    uint8_t header[29];
    CHECK_RET(upng, upng->source.size >= 29, UPNG_ENOTPNG);
    CHECK_RET(upng, upng->source.read(upng->source.user, 0, header, 29) == 29, UPNG_EREAD);

    /* check that PNG header matches expected value */
    static const uint8_t PNG_HEADER[] = { 137, 80, 78, 71, 13, 10, 26, 10 };
    CHECK_RET(upng, memcmp(header, PNG_HEADER, sizeof(PNG_HEADER)) == 0, UPNG_ENOTPNG);

    /* check that the first chunk is the IHDR chunk */
    CHECK_RET(upng, MAKE_DWORD_PTR(header + 12) == CHUNK_IHDR, UPNG_EMALFORMED);

    /* read the values given in the header */
    upng->defaultImage.rect.width = MAKE_DWORD_PTR(header + 16);
    upng->defaultImage.rect.height = MAKE_DWORD_PTR(header + 20);
    upng->color_depth = header[24];
    upng->color_type = (upng_color)header[25];

    /* determine our color format */
    upng->format = determine_format(upng);
    CHECK_RET(upng, upng->format != UPNG_BADFORMAT, UPNG_EUNFORMAT);

    /* check that the compression method (byte 27) is 0 (only allowed value in spec) */
    CHECK_RET(upng, header[26] == 0, UPNG_EMALFORMED);

    /* check that the filter method (byte 27) is 0 (only allowed value in spec) */
    CHECK_RET(upng, header[27] == 0, UPNG_EMALFORMED);

    /* check that the interlace method (byte 27) is 0 (spec allows 1, meaning Adam7, but uPNG does not support it) */
    CHECK_RET(upng, header[28] == 0, UPNG_EUNINTERLACED);

    if (upng_process_chunks(upng) != UPNG_EOK)
        return upng->error;

    upng->state = UPNG_HEADER;
    return upng->error;
}

upng_t *upng_new_from_source(upng_source source)
{
    upng_t *upng;

    upng = (upng_t *)UPNG_MEM_ALLOC(sizeof(upng_t));
    if (upng == NULL)
    {
        return NULL;
    }
    memset(upng, 0, sizeof(upng_t));

    upng->color_type = UPNG_RGBA;
    upng->color_depth = 8;
    upng->format = UPNG_RGBA8;
    upng->current_frame = FRAME_INDEX_NONE;

    upng->state = UPNG_NEW;
    upng->source = source;

    return upng;
}

typedef struct upng_byte_source_context
{
    const unsigned char* buffer;
    unsigned long size;
} upng_byte_source_context;

static unsigned long upng_byte_source_read(void* user, unsigned long offset, void* out_buffer, unsigned long read_size)
{
    upng_byte_source_context* context = (upng_byte_source_context*)user;
    if (offset >= context->size)
        return 0;

    unsigned long bytes_to_copy = read_size;
    if (offset + bytes_to_copy > context->size)
        bytes_to_copy = context->size - offset;

    memcpy(out_buffer, context->buffer + offset, bytes_to_copy);
    return bytes_to_copy;
}

static void upng_byte_source_free(void* user)
{
    UPNG_MEM_FREE(user);
}

upng_t *upng_new_from_bytes(const unsigned char *raw_buffer, unsigned long size)
{
    upng_byte_source_context* context = (upng_byte_source_context*)UPNG_MEM_ALLOC(sizeof(upng_byte_source_context));
    if (context == NULL)
        return NULL;
    context->buffer = raw_buffer;
    context->size = size;

    return upng_new_from_source((upng_source) {
        .user = context,
        .size = size,
        .read = upng_byte_source_read,
        .free = upng_byte_source_free
    });
}

#ifdef UPNG_USE_STDIO
static unsigned long upng_file_source_read(void* user, unsigned long offset, void* out_buffer, unsigned long read_size)
{
    FILE* fp = (FILE*)user;
    fseek(fp, 0, SEEK_END);
    unsigned long size = ftell(fp);
    if (offset >= size)
        return 0;

    unsigned long bytes_to_read = read_size;
    if (offset + bytes_to_read > size)
        bytes_to_read = size - offset;

    fseek(fp, offset, SEEK_SET);
    return (unsigned long)fread(out_buffer, 1, bytes_to_read, fp);
}

static void upng_file_source_free(void* user)
{
    if (user != NULL)
        fclose((FILE*)user);
}

upng_t *upng_new_from_file(const char *filename)
{
    FILE* fp = fopen(filename, "rb");
    upng_t *upng = upng_new_from_source((upng_source) {
        .user = fp,
        .size = 0,
        .read = upng_file_source_read,
        .free = upng_file_source_free
    });
    if (upng == NULL)
        return NULL;

    if (upng->source.user == NULL)
    {
        SET_ERROR(upng, UPNG_ENOTFOUND);
        return upng;
    }

    fseek(fp, 0, SEEK_END);
    upng->source.size = ftell(fp);
    return upng;
}
#endif

void upng_free(upng_t *upng)
{
    /* deallocate palette buffer, if necessary */
    if (upng->palette)
    {
        UPNG_MEM_FREE(upng->palette);
    }

    /* deallocate alpha buffer, we rolled all alphas into the palette */
    if (upng->alpha)
    {
        UPNG_MEM_FREE(upng->alpha);
    }

    if (upng->frames)
    {
        UPNG_MEM_FREE(upng->frames);
    }

    if (upng->buffer)
    {
        UPNG_MEM_FREE(upng->buffer);
    }

    /* deallocate source buffer, if necessary */
    upng_free_source(upng);

    if (upng->text_count)
    {
        for (unsigned int i = 0; i < upng->text_count; i++)
            UPNG_MEM_FREE(upng->text[i].buffer);
    }
    upng->text_count = 0;

    /* deallocate struct itself */
    UPNG_MEM_FREE(upng);
}

upng_error upng_reset(upng_t *upng)
{
    upng->current_frame = FRAME_INDEX_NONE;
    if (upng->buffer)
    {
        UPNG_MEM_FREE(upng->buffer);
        upng->buffer = NULL;
    }
    return UPNG_EOK;
}

upng_error upng_get_error(const upng_t *upng)
{
    return upng->error;
}

unsigned upng_get_error_line(const upng_t *upng)
{
    return upng->error_line;
}

void upng_get_rect(const upng_t *upng, upng_rect *rect)
{
    *rect = upng->defaultImage.rect;
}

void upng_get_frame_rect(const upng_t *upng, upng_rect *rect)
{
    if (upng->decodedFrame != NULL)
        *rect = upng->decodedFrame->rect;
}

unsigned upng_get_frame_index(const upng_t *upng)
{
    return upng->current_frame;
}

unsigned upng_get_frame_count(const upng_t *upng)
{
    return upng->frame_count;
}

unsigned upng_get_plays(const upng_t *upng)
{
    return upng->play_count;
}

int upng_get_palette(const upng_t *upng, upng_rgb **palette)
{
    *palette = upng->palette;
    return upng->palette_entries;
}

int upng_get_alpha(const upng_t *upng, uint8_t **alpha)
{
    *alpha = upng->alpha;
    return upng->alpha_entries;
}

unsigned upng_get_bpp(const upng_t *upng)
{
    return upng_get_bitdepth(upng) * upng_get_components(upng);
}

unsigned upng_get_components(const upng_t *upng)
{
    switch (upng->color_type)
    {
    case UPNG_PLT:
        return 1;
    case UPNG_LUM:
        return 1;
    case UPNG_RGB:
        return 3;
    case UPNG_LUMA:
        return 2;
    case UPNG_RGBA:
        return 4;
    default:
        return 0;
    }
}

unsigned upng_get_bitdepth(const upng_t *upng)
{
    return upng->color_depth;
}

upng_format upng_get_format(const upng_t *upng)
{
    return upng->format;
}

const char* upng_get_text(const upng_t *upng, const char **text_out, unsigned int index)
{
    if (index < upng->text_count)
    {
        *text_out = upng->text[index].text;
        return upng->text[index].keyword;
    }
    return NULL;
}

const uint8_t *upng_get_frame_buffer(const upng_t *upng)
{
    return upng->buffer;
}

uint8_t* upng_move_frame_buffer(upng_t *upng)
{
    uint8_t* buffer = upng->buffer;
    upng->buffer = NULL;
    return buffer;
}

unsigned int upng_get_frame_delay(const upng_t *upng)
{
    if (upng->current_frame == FRAME_INDEX_NONE)
        return 0;
    upng_frame* frame = upng->frames+upng->current_frame;
    uint16_t delay_num = frame->delay_numerator, delay_den = frame->delay_denominator;
    uint32_t ret = 0;
    if (delay_den == 0 || delay_den == 100)
        ret = delay_num;
    else if (delay_den == 10)
        ret = delay_num * 10;
    else if (delay_den == 1000)
        ret = delay_num / 10;
    else
        ret = delay_num * 100 / delay_den;
    return ret*10;
}
