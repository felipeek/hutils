#include <stdio.h>
#include <stdint.h>
#include <string.h>

#define BLOCK_INTS (16)
#define BLOCK_BYTES (16 * 4)
#define DIGEST_SIZE (5)

static uint32_t digest[5] = {0x67452301, 0xEFCDAB89, 0x98BADCFE, 0x10325476, 0xC3D2E1F0};
static uint64_t transforms;

void transform(uint32_t digest[5], uint32_t block[16], uint64_t* transforms);

uint64_t u32_to_str_base16(uint32_t value, int leading_zeros, char* buffer)
{
    int i = 0;
    for (; i < 8; i += 1) {
        uint32_t f = (value & 0xf0000000) >> 28;
        if(f > 9) buffer[i] = (char)f + 0x57;
        else buffer[i] = (char)f + 0x30;
        value <<= 4;
    }
    return i;
}

void buffer_to_block(char* buffer, int length, uint32_t block[16])
{
    /* Convert the string (byte buffer) to a uint32_t array (MSB) */
    for (uint64_t i = 0; i < BLOCK_INTS; i += 1)
    {
        block[i] = ((uint32_t)(buffer[4*i+3] & 0xff) | ((uint32_t)(buffer[4*i+2] & 0xff)<<8)
                   | ((uint32_t)(buffer[4*i+1] & 0xff)<<16)
                   | ((uint32_t)(buffer[4*i+0] & 0xff)<<24));
    }
}

void sha1(char* buffer, int length, char out[20]) {
    uint64_t count = 0;

    uint64_t total_bits = (transforms*BLOCK_BYTES + (uint64_t)length) * 8;
    buffer[length] = 0x80;
    
    length += 1;
    uint64_t orig_size = (uint64_t)length;

    while ((uint64_t)length < BLOCK_BYTES) {
        buffer[length] = 0;
        length += 1;
    }

    uint32_t block[16];
    buffer_to_block(buffer, length, block);
    
    if (orig_size > BLOCK_BYTES - 8) {
        transform(digest, block, &transforms);
        for (uint64_t i = 0; i < BLOCK_INTS - 2; i += 1) {
            block[i] = 0;
        }
    }

    /* Append total_bits, split this uint64_t into two uint32_t */
    block[BLOCK_INTS - 1] = (uint32_t)total_bits;
    block[BLOCK_INTS - 2] = (uint32_t)(total_bits >> 32);
    transform(digest, block, &transforms);

    memcpy(out, digest, 20);

    int xx  = 0;
}

uint32_t rol(uint32_t value, uint32_t bits)
{
    return (value << bits) | (value >> (32 - bits));
}

uint32_t blk(uint32_t block[16], uint32_t i)
{
    return rol(block[(i+13)&15] ^ block[(i+8)&15] ^ block[(i+2)&15] ^ block[i], 1);
}

void R0(uint32_t block[16], uint32_t v, uint32_t* w, uint32_t x, uint32_t y, uint32_t* z, uint64_t i)
{
    // not checking lvalue here
    *z += ((*w&(x^y))^y) + block[i] + 0x5a827999 + rol(v, 5);
    *w = rol(*w, 30);
}


void R1(uint32_t block[16], uint32_t v, uint32_t* w, uint32_t x, uint32_t y, uint32_t* z, uint64_t i)
{
    block[i] = blk(block, (uint32_t)i);
    *z += ((*w&(x^y))^y) + block[i] + 0x5a827999 + rol(v, 5);
    *w = rol(*w, 30);
}


void R2(uint32_t block[16], uint32_t v, uint32_t* w, uint32_t x, uint32_t y, uint32_t* z, uint64_t i)
{
    block[i] = blk(block, (uint32_t)i);
    *z += (*w^x^y) + block[i] + 0x6ed9eba1 + rol(v, 5);
    *w = rol(*w, 30);
}


void R3(uint32_t block[16], uint32_t v, uint32_t* w, uint32_t x, uint32_t y, uint32_t* z, uint64_t i)
{
    block[i] = blk(block, (uint32_t)i);
    *z += (((*w|x)&y)|(*w&x)) + block[i] + 0x8f1bbcdc + rol(v, 5);
    *w = rol(*w, 30);
}


void R4(uint32_t block[16], uint32_t v, uint32_t* w, uint32_t x, uint32_t y, uint32_t* z, uint64_t i)
{
    block[i] = blk(block, (uint32_t)i);
    *z += (*w^x^y) + block[i] + 0xca62c1d6 + rol(v, 5);
    *w = rol(*w, 30);
}


void transform(uint32_t digest[5], uint32_t block[16], uint64_t* transforms)
{
    /* Copy digest[] to working vars */
    uint32_t a = digest[0];
    uint32_t b = digest[1];
    uint32_t c = digest[2];
    uint32_t d = digest[3];
    uint32_t e = digest[4];
 
    /* 4 rounds of 20 operations each. Loop unrolled. */
    R0(block, a, &b, c, d, &e,  0);
    R0(block, e, &a, b, c, &d,  1);
    R0(block, d, &e, a, b, &c,  2);
    R0(block, c, &d, e, a, &b,  3);
    R0(block, b, &c, d, e, &a,  4);
    R0(block, a, &b, c, d, &e,  5);
    R0(block, e, &a, b, c, &d,  6);
    R0(block, d, &e, a, b, &c,  7);
    R0(block, c, &d, e, a, &b,  8);
    R0(block, b, &c, d, e, &a,  9);
    R0(block, a, &b, c, d, &e, 10);
    R0(block, e, &a, b, c, &d, 11);
    R0(block, d, &e, a, b, &c, 12);
    R0(block, c, &d, e, a, &b, 13);
    R0(block, b, &c, d, e, &a, 14);
    R0(block, a, &b, c, d, &e, 15);
    R1(block, e, &a, b, c, &d,  0);
    R1(block, d, &e, a, b, &c,  1);
    R1(block, c, &d, e, a, &b,  2);
    R1(block, b, &c, d, e, &a,  3);
    R2(block, a, &b, c, d, &e,  4);
    R2(block, e, &a, b, c, &d,  5);
    R2(block, d, &e, a, b, &c,  6);
    R2(block, c, &d, e, a, &b,  7);
    R2(block, b, &c, d, e, &a,  8);
    R2(block, a, &b, c, d, &e,  9);
    R2(block, e, &a, b, c, &d, 10);
    R2(block, d, &e, a, b, &c, 11);
    R2(block, c, &d, e, a, &b, 12);
    R2(block, b, &c, d, e, &a, 13);
    R2(block, a, &b, c, d, &e, 14);
    R2(block, e, &a, b, c, &d, 15);
    R2(block, d, &e, a, b, &c,  0);
    R2(block, c, &d, e, a, &b,  1);
    R2(block, b, &c, d, e, &a,  2);
    R2(block, a, &b, c, d, &e,  3);
    R2(block, e, &a, b, c, &d,  4);
    R2(block, d, &e, a, b, &c,  5);
    R2(block, c, &d, e, a, &b,  6);
    R2(block, b, &c, d, e, &a,  7);
    R3(block, a, &b, c, d, &e,  8);
    R3(block, e, &a, b, c, &d,  9);
    R3(block, d, &e, a, b, &c, 10);
    R3(block, c, &d, e, a, &b, 11);
    R3(block, b, &c, d, e, &a, 12);
    R3(block, a, &b, c, d, &e, 13);
    R3(block, e, &a, b, c, &d, 14);
    R3(block, d, &e, a, b, &c, 15);
    R3(block, c, &d, e, a, &b,  0);
    R3(block, b, &c, d, e, &a,  1);
    R3(block, a, &b, c, d, &e,  2);
    R3(block, e, &a, b, c, &d,  3);
    R3(block, d, &e, a, b, &c,  4);
    R3(block, c, &d, e, a, &b,  5);
    R3(block, b, &c, d, e, &a,  6);
    R3(block, a, &b, c, d, &e,  7);
    R3(block, e, &a, b, c, &d,  8);
    R3(block, d, &e, a, b, &c,  9);
    R3(block, c, &d, e, a, &b, 10);
    R3(block, b, &c, d, e, &a, 11);
    R4(block, a, &b, c, d, &e, 12);
    R4(block, e, &a, b, c, &d, 13);
    R4(block, d, &e, a, b, &c, 14);
    R4(block, c, &d, e, a, &b, 15);
    R4(block, b, &c, d, e, &a,  0);
    R4(block, a, &b, c, d, &e,  1);
    R4(block, e, &a, b, c, &d,  2);
    R4(block, d, &e, a, b, &c,  3);
    R4(block, c, &d, e, a, &b,  4);
    R4(block, b, &c, d, e, &a,  5);
    R4(block, a, &b, c, d, &e,  6);
    R4(block, e, &a, b, c, &d,  7);
    R4(block, d, &e, a, b, &c,  8);
    R4(block, c, &d, e, a, &b,  9);
    R4(block, b, &c, d, e, &a, 10);
    R4(block, a, &b, c, d, &e, 11);
    R4(block, e, &a, b, c, &d, 12);
    R4(block, d, &e, a, b, &c, 13);
    R4(block, c, &d, e, a, &b, 14);
    R4(block, b, &c, d, e, &a, 15);

    /* Add the working vars back into digest[] */
    digest[0] += a;
    digest[1] += b;
    digest[2] += c;
    digest[3] += d;
    digest[4] += e;

    /* Count the number of transformations */
    transforms += 1;
}

void sha1_to_string(char in[20], char out[40]) {
    for (uint64_t i = 0; i < DIGEST_SIZE; i += 1) {
        u32_to_str_base16(((uint32_t*)in)[i], 1, (char*)out + (i * 8));
    }
}

int main() {
    unsigned char sa[256];
    sa[0] = 'a';
    sa[1] = 'b';
    sa[2] = 'c';
    
    char res[20] = {0};
    sha1(sa, 3, res);

    char str[41] = {0};
    sha1_to_string(res, str);
    printf("%s", str);

    return 0;
}