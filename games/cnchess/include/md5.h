#ifndef _MD5_H_
#define _MD5_H_

#include <stdint.h>
#include <string.h>

// MD5上下文
typedef struct {
    uint32_t lo, hi;
    uint32_t a, b, c, d;
    unsigned char buffer[64];
    size_t used;
} MD5_CTX;

static const uint32_t MD5_T[] = {
    0xd76aa478, 0xe8c7b756, 0x242070db, 0xc1bdceee,
    0xf57c0faf, 0x4787c62a, 0xa8304613, 0xfd469501,
    0x698098d8, 0x8b44f7af, 0xffff5bb1, 0x895cd7be,
    0x6b901122, 0xfd987193, 0xa679438e, 0x49b40821,
    0xf61e2562, 0xc040b340, 0x265e5a51, 0xe9b6c7aa,
    0xd62f105d, 0x02441453, 0xd8a1e681, 0xe7d3fbc8,
    0x21e1cde6, 0xc33707d6, 0xf4d50d87, 0x455a14ed,
    0xa9e3e905, 0xfcefa3f8, 0x676f02d9, 0x8d2a4c8a,
    0xfffa3942, 0x8771f681, 0x6d9d6122, 0xfde5380c,
    0xa4beea44, 0x4bdecfa9, 0xf6bb4b60, 0xbebfbc70,
    0x289b7ec6, 0xeaa127fa, 0xd4ef3085, 0x04881d05,
    0xd9d4d039, 0xe6db99e5, 0x1fa27cf8, 0xc4ac5665,
    0xf4292244, 0x432aff97, 0xab9423a7, 0xfc93a039,
    0x655b59c3, 0x8f0ccc92, 0xffeff47d, 0x85845dd1,
    0x6fa87e4f, 0xfe2ce6e0, 0xa3014314, 0x4e0811a1,
    0xf7537e82, 0xbd3af235, 0x2ad7d2bb, 0xeb86d391
};

static const int MD5_S[] = {
    7, 12, 17, 22, 7, 12, 17, 22, 7, 12, 17, 22, 7, 12, 17, 22,
    5, 9, 14, 20, 5, 9, 14, 20, 5, 9, 14, 20, 5, 9, 14, 20,
    4, 11, 16, 23, 4, 11, 16, 23, 4, 11, 16, 23, 4, 11, 16, 23,
    6, 10, 15, 21, 6, 10, 15, 21, 6, 10, 15, 21, 6, 10, 15, 21
};

#define MD5_ROTL(x, n) (((x) << (n)) | ((x) >> (32 - (n))))

static void MD5_Transform(MD5_CTX *ctx, const unsigned char block[64]) {
    uint32_t a = ctx->a, b = ctx->b, c = ctx->c, d = ctx->d;
    uint32_t M[16];
    int i;

    for (i = 0; i < 16; i++) {
        M[i] = (uint32_t)block[i*4] | ((uint32_t)block[i*4+1] << 8) |
               ((uint32_t)block[i*4+2] << 16) | ((uint32_t)block[i*4+3] << 24);
    }

    for (i = 0; i < 64; i++) {
        uint32_t f;
        int g;
        if (i < 16) {
            f = (b & c) | (~b & d);
            g = i;
        } else if (i < 32) {
            f = (d & b) | (~d & c);
            g = (5*i + 1) % 16;
        } else if (i < 48) {
            f = b ^ c ^ d;
            g = (3*i + 5) % 16;
        } else {
            f = c ^ (b | ~d);
            g = (7*i) % 16;
        }
        uint32_t temp = d;
        d = c;
        c = b;
        uint32_t sum = a + MD5_T[i] + M[g] + f;
        b = b + MD5_ROTL(sum, MD5_S[i]);
        a = temp;
    }

    ctx->a += a;
    ctx->b += b;
    ctx->c += c;
    ctx->d += d;
}

static void MD5_Init(MD5_CTX *ctx) {
    ctx->lo = 0x67452301;
    ctx->hi = 0xefcdab89;
    ctx->a = 0x67452301;
    ctx->b = 0xefcdab89;
    ctx->c = 0x98badcfe;
    ctx->d = 0x10325476;
    ctx->used = 0;
}

static void MD5_Update(MD5_CTX *ctx, const void *data, size_t size) {
    const unsigned char *p = (const unsigned char *)data;
    while (size > 0) {
        size_t take = 64 - ctx->used;
        if (take > size) take = size;
        memcpy(ctx->buffer + ctx->used, p, take);
        ctx->used += take;
        p += take;
        size -= take;
        if (ctx->used == 64) {
            MD5_Transform(ctx, ctx->buffer);
            ctx->used = 0;
            ctx->lo += 512;
            if (ctx->lo < 512) ctx->hi++;
        }
    }
}

static void MD5_Final(unsigned char digest[16], MD5_CTX *ctx) {
    uint64_t bit_count = ((uint64_t)ctx->hi << 32) | ctx->lo;
    bit_count += ctx->used * 8;

    // 填充
    unsigned char pad[64];
    memset(pad, 0, sizeof(pad));
    pad[0] = 0x80;
    size_t pad_len = (ctx->used < 56) ? (56 - ctx->used) : (120 - ctx->used);
    MD5_Update(ctx, pad, pad_len);

    // 追加长度（小端）
    unsigned char len_bytes[8];
    for (int i = 0; i < 8; i++) {
        len_bytes[i] = (unsigned char)(bit_count >> (i * 8));
    }
    MD5_Update(ctx, len_bytes, 8);

    // 输出摘要
    uint32_t vals[4] = {ctx->a, ctx->b, ctx->c, ctx->d};
    for (int i = 0; i < 4; i++) {
        digest[i*4]   = (unsigned char)(vals[i]);
        digest[i*4+1] = (unsigned char)(vals[i] >> 8);
        digest[i*4+2] = (unsigned char)(vals[i] >> 16);
        digest[i*4+3] = (unsigned char)(vals[i] >> 24);
    }
}

// 计算数据的MD5
static void MD5_Buffer(const void *data, size_t size, unsigned char digest[16]) {
    MD5_CTX ctx;
    MD5_Init(&ctx);
    MD5_Update(&ctx, data, size);
    MD5_Final(digest, &ctx);
}

// 计算文件的MD5
static bool MD5_File(const char *path, unsigned char digest[16]) {
    FILE *f = fopen(path, "rb");
    if (!f) return false;
    MD5_CTX ctx;
    MD5_Init(&ctx);
    unsigned char buf[4096];
    size_t n;
    while ((n = fread(buf, 1, sizeof(buf), f)) > 0) {
        MD5_Update(&ctx, buf, n);
    }
    fclose(f);
    MD5_Final(digest, &ctx);
    return true;
}

// 比较两个MD5
static bool MD5_Compare(const unsigned char a[16], const unsigned char b[16]) {
    return memcmp(a, b, 16) == 0;
}

static void MD5_ToHexString(const unsigned char digest[16], char hex[33]) {
    static const char hex_chars[] = "0123456789abcdef";
    for (int i = 0; i < 16; i++) {
        hex[i*2] = hex_chars[(digest[i] >> 4) & 0xF];
        hex[i*2 + 1] = hex_chars[digest[i] & 0xF];
    }
    hex[32] = '\0';
}

#endif //_MD5_H_
