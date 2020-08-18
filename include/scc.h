
#include "define.h"

int uart_get(char*);
void uart_put(char cr);


void uart_config(unsigned char txd_pin_number , unsigned char rxd_pin_number);

static void uart_irq_init(void);

#if defined(__USE_SPI_TERMINAL__)

int uart_get_bmd101(char* inchar);

#endif



