#include "MKL46Z4.h"
//prueba

// LED_GREEN = PTD5, LED_RED = PTE29 (activos a nivel bajo)
// SW1 = PTC3 (izquierdo), SW3 = PTC12 (derecho)

// Modo de ejecución:
//   0 = programa final (interrupciones)
//   1 = prueba de LEDs (ambos deben encenderse)
//   2 = prueba de botones por polling (SW1 -> rojo, SW3 -> verde)
#define MODO 0

#define LED_GREEN_PIN 5
#define LED_RED_PIN   29
#define SW_LEFT_PIN   3
#define SW_RIGHT_PIN  12

static volatile int estado = 0;   // 0: off/off, 1: R, 2: R+G, 3: G

static void led_green_init(void)
{
  SIM->SCGC5 |= SIM_SCGC5_PORTD_MASK;
  PORTD->PCR[LED_GREEN_PIN] = PORT_PCR_MUX(1);
  GPIOD->PDDR |= (1 << LED_GREEN_PIN);
  GPIOD->PSOR = (1 << LED_GREEN_PIN);          // apagado
}

static void led_red_init(void)
{
  SIM->COPC = 0;                               // desactiva el watchdog
  SIM->SCGC5 |= SIM_SCGC5_PORTE_MASK;
  PORTE->PCR[LED_RED_PIN] = PORT_PCR_MUX(1);
  GPIOE->PDDR |= (1 << LED_RED_PIN);
  GPIOE->PSOR = (1 << LED_RED_PIN);            // apagado
}

static void led_green_set(int on)
{
  if (on) GPIOD->PCOR = (1 << LED_GREEN_PIN);
  else    GPIOD->PSOR = (1 << LED_GREEN_PIN);
}

static void led_red_set(int on)
{
  if (on) GPIOE->PCOR = (1 << LED_RED_PIN);
  else    GPIOE->PSOR = (1 << LED_RED_PIN);
}

static void aplicar_estado(void)
{
  led_red_set(estado == 1 || estado == 2);
  led_green_set(estado == 2 || estado == 3);
}

static void __attribute__((unused)) buttons_init(void)
{
  SIM->SCGC5 |= SIM_SCGC5_PORTC_MASK;

  // GPIO + pull-up + interrupción en flanco de bajada (IRQC = 0b1010)
  PORTC->PCR[SW_LEFT_PIN]  = PORT_PCR_MUX(1) | PORT_PCR_PE_MASK |
                             PORT_PCR_PS_MASK | PORT_PCR_IRQC(0xA);
  PORTC->PCR[SW_RIGHT_PIN] = PORT_PCR_MUX(1) | PORT_PCR_PE_MASK |
                             PORT_PCR_PS_MASK | PORT_PCR_IRQC(0xA);

  GPIOC->PDDR &= ~((1 << SW_LEFT_PIN) | (1 << SW_RIGHT_PIN));  // entradas

  PORTC->ISFR = 0xFFFFFFFF;                    // limpia flags pendientes

#if MODO == 0
  NVIC_EnableIRQ(PORTC_PORTD_IRQn);
#endif
}

// Nombre del handler según la tabla de vectores de startup.c (vector 31)
void PORTDIntHandler(void)
{
  uint32_t flags = PORTC->ISFR;

  if (flags & (1 << SW_LEFT_PIN))
    estado = (estado + 1) % 4;
  if (flags & (1 << SW_RIGHT_PIN))
    estado = (estado + 3) % 4;

  PORTC->ISFR = flags;                         // limpiar flags (escribiendo 1)
  aplicar_estado();
}

int main(void)
{
  led_green_init();
  led_red_init();

#if MODO == 1
  // Prueba de LEDs: ambos deben quedar encendidos
  estado = 2;
  aplicar_estado();
  while (1) { }

#elif MODO == 2
  // Prueba de botones sin interrupciones: pulsado = 0 (pull-up)
  buttons_init();
  while (1) {
    led_red_set(!(GPIOC->PDIR & (1 << SW_LEFT_PIN)));
    led_green_set(!(GPIOC->PDIR & (1 << SW_RIGHT_PIN)));
  }

#else
  // Programa final
  buttons_init();
  aplicar_estado();
  while (1) {
    __WFI();
  }
#endif

  return 0;
}
