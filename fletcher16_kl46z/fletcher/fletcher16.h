#ifndef FLETCHER16_H
#define FLETCHER16_H

#include <stdint.h>
#include <stddef.h>

/* v0: naive, tal cal o enunciado (dous % 255 por byte) */
uint16_t fletcher16_naive(const uint8_t *data, size_t len);

/* v1: substitúe % por unha resta condicional (sen división) */
uint16_t fletcher16_condsub(const uint8_t *data, size_t len);

/* v2: módulo diferido: acumula en 32 bits e reduce só ao final de cada bloque */
uint16_t fletcher16_deferred(const uint8_t *data, size_t len);

/* v3: v2 + desenrolado do bucle (x8) */
uint16_t fletcher16_unrolled(const uint8_t *data, size_t len);

#endif
