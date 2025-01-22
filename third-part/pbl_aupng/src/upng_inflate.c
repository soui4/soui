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
#include <stdint.h>
#include "upng.h"

static upng_inflate_cb s_inflate = NULL;

void upng_set_inflate(upng_inflate_cb fun_inflate)
{
    s_inflate = fun_inflate;
}

extern upng_error uz_inflate(unsigned char *out, unsigned long outsize, const unsigned char *in, unsigned long insize)
{
    if (!s_inflate)
    {
        return UPNG_NOINVLATE;
    }
    /* we require two bytes for the zlib data header */
    if (insize < 2)
        return UPNG_EMALFORMED;

    /* 256 * in[0] + in[1] must be a multiple of 31, the FCHECK value is supposed to be made that way */
    if ((in[0] * 256 + in[1]) % 31 != 0)
        return UPNG_EMALFORMED;

    /*error: only compression method 8: inflate with sliding window of 32k is supported by the PNG spec */
    if ((in[0] & 15) != 8 || ((in[0] >> 4) & 15) > 7)
        return UPNG_EMALFORMED;

    /* the specification of PNG says about the zlib stream: "The additional flags shall not specify a preset dictionary." */
    if (((in[1] >> 5) & 1) != 0)
        return UPNG_EMALFORMED;

    /* create output buffer */
    return s_inflate((char*)out, (int)outsize, (const char*)in, (int)insize) == 0 ? UPNG_EOK : UPNG_EMALFORMED;
}
