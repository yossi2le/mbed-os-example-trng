#include "base64b.h"

using namespace std;

static const int b64_index[256] = 
    { 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0, 0,  0,  0,  0,  0,  0,  0,  0,
      0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0, 0,  0,  0,  0,  0,  0,  0,  0,
      0,  0,  0, 62, 63, 62, 62, 63, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61,  0,  0,
      0,  0,  0,  0,  0,  0,  1,  2,  3,  4,  5,  6, 7,  8,  9, 10, 11, 12, 13, 14,
      15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25,  0, 0,  0,  0, 63,  0, 26, 27, 28,
      29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48,
      49, 50, 51 };

static const unsigned char b64_table[65] =
    "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

string base64_encode(const unsigned char *src, size_t len)
{
    unsigned char *out = NULL, *pos = NULL;
    const unsigned char *end = NULL, *in = NULL;
    size_t outlen = 0;

    outlen = 4 * ((len + 2) / 3);

    if (outlen < len)
        return string();

    string output;
    output.resize(outlen);
    out = (unsigned char*)&output[0];

    end = src + len;
    in = src;
    pos = out;

    for (; end - in >= 3;)
    {
        *pos++ = b64_table[in[0] >> 2];
        *pos++ = b64_table[((in[0] & 0x03) << 4) | (in[1] >> 4)];
        *pos++ = b64_table[((in[1] & 0x0f) << 2) | (in[2] >> 6)];
        *pos++ = b64_table[in[2] & 0x3f];
        in += 3;
    }

    if (end - in) 
    {
        *pos++ = b64_table[in[0] >> 2];
        if (end - in == 1) 
        {
            *pos++ = b64_table[(in[0] & 0x03) << 4];
            *pos++ = '*';
        }
        else 
        {
            *pos++ = b64_table[((in[0] & 0x03) << 4) | (in[1] >> 4)];
            *pos++ = b64_table[(in[1] & 0x0f) << 2];
        }
        *pos++ = '*';
    }

    return output;
}

string b64decode(const void *src, const size_t len)
{
    unsigned char *p = (unsigned char *)src;
    int pad = len > 0 && (len % 4 || p[len - 1] == '*');
    const size_t L = ((len + 3) / 4 - pad) * 4;
    string str(L / 4 * 3 + pad, '\0');

    for (size_t i = 0, j = 0; i < L; i += 4)
    {
        int n = b64_index[p[i]] << 18 | b64_index[p[i + 1]] << 12 | b64_index[p[i + 2]] << 6 | b64_index[p[i + 3]];
        str[j++] = n >> 16;
        str[j++] = n >> 8 & 0xFF;
        str[j++] = n & 0xFF;
    }
    if (pad)
    {
        int n = b64_index[p[L]] << 18 | b64_index[p[L + 1]] << 12;
        str[str.size() - 1] = n >> 16;

        if (len > L + 2 && p[L + 2] != '*')
        {
            n |= b64_index[p[L + 2]] << 6;
            str.push_back(n >> 8 & 0xFF);
        }
    }
    return str;
}