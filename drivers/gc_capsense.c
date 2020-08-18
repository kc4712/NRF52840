#include <stdio.h>
#include <string.h>
#include "nrf_error.h"
#include "nrf_drv_config.h"
#include "app_util_platform.h"
#include "nrf_drv_gpiote.h"
#include "gc_capsense.h"

//volatile unsigned char tuch_count =0;
extern volatile unsigned char OLED_state;

void capsense_isr(void)
{
	printf("\r\n Cap-sense!");
	//
	//tuch_count++;
}

void gpiote_capsense_evt_handler(nrf_drv_gpiote_pin_t pin, nrf_gpiote_polarity_t action)
{
	if(pin == GC_CAPSENSE_PIN  && action == NRF_GPIOTE_POLARITY_LOTOHI )
  {                                                               
		capsense_isr();                     
  }
}

void init_capsense(void)
{
	nrf_gpio_cfg_input(GC_CAPSENSE_PIN, NRF_GPIO_PIN_NOPULL); 
	
	nrf_drv_gpiote_in_config_t in_config = GPIOTE_CONFIG_IN_SENSE_LOTOHI(true);
	//in_config.pull = NRF_GPIO_PIN_PULLUP;

	if ( nrf_drv_gpiote_in_init(GC_CAPSENSE_PIN, &in_config, gpiote_capsense_evt_handler) != NRF_SUCCESS )
		printf("\r\n cap GPIOTE input init. fail");
	
	nrf_drv_gpiote_in_event_enable(GC_CAPSENSE_PIN, true);

}
