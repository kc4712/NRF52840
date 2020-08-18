
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __GC_CAPSENSE_H
#define __GC_CAPSENSE_H

#include "nrf_drv_gpiote.h"

#define GC_CAPSENSE_PIN		30

void gpiote_capsense_evt_handler(nrf_drv_gpiote_pin_t pin, nrf_gpiote_polarity_t action);
void capsense_isr(void);
void init_capsense(void);


#endif /* __GC_CAPSENSE_H */


/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
