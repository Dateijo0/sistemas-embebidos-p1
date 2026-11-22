#include "fletcher16.h"

/* ------------------------------------------------------------------ */
/* v0: naive                                                           */
/* ------------------------------------------------------------------ */
uint16_t fletcher16_naive(const uint8_t *data, size_t len)
{
    uint32_t sum1 = 0, sum2 = 0;

    for (size_t i = 0; i < len; i++) {
        sum1 = (sum1 + data[i]) % 255;
        sum2 = (sum2 + sum1) % 255;
    }
    return (uint16_t)((sum2 << 8) | sum1);
}

/* ------------------------------------------------------------------ */
/* v1: resta condicional en vez de %                                   */
/*  O Cortex-M0+ non ten instrución de división: o % 255 chama a       */
/*  __aeabi_uidivmod (decenas de ciclos). Como sum + dato < 510,       */
/*  chega con restar 255 unha soa vez.                                 */
/* ------------------------------------------------------------------ */
uint16_t fletcher16_condsub(const uint8_t *data, size_t len)
{
    uint32_t sum1 = 0, sum2 = 0;

    for (size_t i = 0; i < len; i++) {
        sum1 += data[i];
        if (sum1 >= 255) sum1 -= 255;
        sum2 += sum1;
        if (sum2 >= 255) sum2 -= 255;
    }
    return (uint16_t)((sum2 << 8) | sum1);
}

/* ------------------------------------------------------------------ */
/* v2: módulo diferido                                                 */
/*  Cambio de algoritmo: o módulo non hai que facelo en cada paso, só  */
/*  hai que evitar desbordar os 32 bits. Con sum1,sum2 <= 254 ao       */
/*  comezar un bloque de n bytes:                                      */
/*     sum2 <= 254 + 254*n + 255*n*(n+1)/2  < 2^32  para n <= 5802     */
/*  Usamos n = 5000 (marxe). Para 100 e 1024 bytes só hai 1 reducción. */
/*                                                                     */
/*  Reducción mod 255 sen división: 256 = 1 (mod 255), así que         */
/*  x mod 255 == (x & 0xFF) + (x >> 8) (plegado), repetido.            */
/* ------------------------------------------------------------------ */
#define NMAX 5000u

static inline uint32_t mod255(uint32_t x)
{
    x = (x & 0xFFFFu) + (x >> 16);   /* <= 0x1FFFE */
    x = (x & 0xFFu)   + (x >> 8);    /* <= 766     */
    x = (x & 0xFFu)   + (x >> 8);    /* <= 257     */
    if (x >= 255) x -= 255;
    return x;
}

uint16_t fletcher16_deferred(const uint8_t *data, size_t len)
{
    uint32_t sum1 = 0, sum2 = 0;

    while (len) {
        size_t n = (len > NMAX) ? NMAX : len;
        len -= n;
        while (n--) {
            sum1 += *data++;
            sum2 += sum1;
        }
        sum1 = mod255(sum1);
        sum2 = mod255(sum2);
    }
    return (uint16_t)((sum2 << 8) | sum1);
}

/* ------------------------------------------------------------------ */
/* v3: módulo diferido + desenrolado x8                                */
/*  Elimina 7 de cada 8 comparacións/saltos/incrementos do bucle.      */
/* ------------------------------------------------------------------ */
uint16_t fletcher16_unrolled(const uint8_t *data, size_t len)
{
    uint32_t sum1 = 0, sum2 = 0;

    while (len) {
        size_t n = (len > NMAX) ? NMAX : len;
        len -= n;

        while (n >= 8) {
            sum1 += data[0]; sum2 += sum1;
            sum1 += data[1]; sum2 += sum1;
            sum1 += data[2]; sum2 += sum1;
            sum1 += data[3]; sum2 += sum1;
            sum1 += data[4]; sum2 += sum1;
            sum1 += data[5]; sum2 += sum1;
            sum1 += data[6]; sum2 += sum1;
            sum1 += data[7]; sum2 += sum1;
            data += 8;
            n -= 8;
        }
        while (n--) {
            sum1 += *data++;
            sum2 += sum1;
        }
        sum1 = mod255(sum1);
        sum2 = mod255(sum2);
    }
    return (uint16_t)((sum2 << 8) | sum1);
}
