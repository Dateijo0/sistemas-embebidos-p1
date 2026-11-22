/* Test de corrección en PC: gcc -O2 test_host.c fletcher16.c -o test_host */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "fletcher16.h"

typedef uint16_t (*fn_t)(const uint8_t *, size_t);

int main(void)
{
    fn_t fn[] = { fletcher16_naive, fletcher16_condsub,
                  fletcher16_deferred, fletcher16_unrolled };
    const char *nm[] = { "naive", "condsub", "deferred", "unrolled" };
    int bad = 0;

    for (int f = 0; f < 4; f++)
        if (fn[f]((const uint8_t *)"abcde", 5) != 0xC8F0) {
            printf("%s: abcde -> FALLO\n", nm[f]); bad++;
        }

    /* aleatorios, incluídos casos extremos (todo 0xFF) e > NMAX */
    static uint8_t buf[20000];
    srand(1);
    for (int t = 0; t < 3000; t++) {
        size_t len = (t < 40) ? (size_t)t : (size_t)(rand() % 20000);
        if (t % 5 == 0) memset(buf, 0xFF, sizeof buf);
        else for (size_t i = 0; i < len; i++) buf[i] = rand();
        uint16_t ref = fn[0](buf, len);
        for (int f = 1; f < 4; f++)
            if (fn[f](buf, len) != ref) { printf("%s len=%zu FALLO\n", nm[f], len); bad++; }
    }
    printf(bad ? "ERRORES: %d\n" : "TODO OK\n", bad);
    return bad != 0;
}
