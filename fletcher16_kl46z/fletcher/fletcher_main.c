/*
 * Práctica 3 - Benchmarking de Fletcher-16 co SysTick
 * Placa: FRDM-KL46Z (Cortex-M0+ @ 48 MHz), SDK de NXP.
 * N_ELEMS (100 / 1024) e o nivel de optimización pásanse desde o Makefile.
 */
#include <stdint.h>
#include <stddef.h>

#include "fsl_device_registers.h"
#include "fsl_debug_console.h"
#include "board.h"
#include "pin_mux.h"
#include "clock_config.h"

#include "fletcher16.h"

#ifndef N_ELEMS
#define N_ELEMS 100
#endif
#define REPS 10

/* ---- Medición de ciclos co SysTick (slide 13) ---- */
#define COMPILER_BARRIER() __asm volatile ("" ::: "memory")

static inline uint32_t SysTick_GetTicks(void)
{
    COMPILER_BARRIER();
    uint32_t t = SysTick->VAL;
    COMPILER_BARRIER();
    return t;
}
#define timestamp() SysTick_GetTicks()

static void systick_init(void)
{
    SysTick->LOAD = 0x00FFFFFFu;                 /* 24 bits, conta cara abaixo */
    SysTick->VAL  = 0;
    SysTick->CTRL = SysTick_CTRL_CLKSOURCE_Msk | /* reloxo do core (48 MHz)    */
                    SysTick_CTRL_ENABLE_Msk;     /* sen interrupción           */
}

/* O contador DECRECE: ciclos = inicio - fin (módulo 2^24) */
#define ELAPSED(start, end) (((start) - (end)) & 0x00FFFFFFu)

static uint8_t data[N_ELEMS];

typedef uint16_t (*fletcher_fn)(const uint8_t *, size_t);

/* Devolve o mínimo de REPS medicións */
static uint32_t bench(fletcher_fn f, uint16_t *result)
{
    uint32_t best = 0xFFFFFFFFu;
    for (int rep = 0; rep < REPS; rep++) {
        uint32_t start = timestamp();
        uint16_t r = f(data, N_ELEMS);
        uint32_t end = timestamp();
        uint32_t t = ELAPSED(start, end);
        if (t < best) best = t;
        *result = r;
    }
    return best;
}

int main(void)
{
    BOARD_InitPins();
    BOARD_BootClockRUN();
    BOARD_InitDebugConsole();
    systick_init();

    for (int i = 0; i < N_ELEMS; i++) data[i] = (uint8_t)(i * 7 + 3);

    PRINTF("\r\n=== Fletcher-16  N=%u  core=%u Hz ===\r\n",
           (unsigned)N_ELEMS, (unsigned)SystemCoreClock);

    /* Comprobación: "abcde" -> 0xC8F0 */
    PRINTF("check abcde: 0x%x (esperado 0xc8f0)\r\n",
           (unsigned)fletcher16_naive((const uint8_t *)"abcde", 5));

    /* Custo de medir: dous timestamp seguidos */
    uint32_t s = timestamp(), e = timestamp();
    uint32_t overhead = ELAPSED(s, e);
    PRINTF("overhead da medida: %u ciclos\r\n", (unsigned)overhead);

    static const struct { const char *name; fletcher_fn fn; } tests[] = {
        { "naive (%255)", fletcher16_naive    },
        { "condsub",       fletcher16_condsub  },
        { "deferred mod",  fletcher16_deferred },
        { "deferred+unr8", fletcher16_unrolled },
    };

    uint16_t ref = 0, r = 0;
    for (unsigned i = 0; i < sizeof tests / sizeof tests[0]; i++) {
        uint32_t c = bench(tests[i].fn, &r);
        if (i == 0) ref = r;
        PRINTF("%s: %u ciclos (sen overhead: %u)  chk=0x%x %s\r\n",
               tests[i].name, (unsigned)c,
               (unsigned)(c > overhead ? c - overhead : 0),
               (unsigned)r, r == ref ? "OK" : "ERRO");
    }
    PRINTF("=== fin ===\r\n");

    for (;;) { }
}
