#include <stdio.h>
#include <string.h>
#include "nrf_error.h"
#include "nrf_drv_config.h"
#include "app_util_platform.h"
#include "nrf_drv_gpiote.h"
#include "nrf_drv_twi.h"
#include "pah8002.h"
#include "twi_pah8002.h"
#include "define.h"

const nrf_drv_twi_t m_twi_master_2 = NRF_DRV_TWI_INSTANCE(1);

uint8_t pah8002_rx_buff[4];
uint8_t pah8002_reg_rx;

/** 
* @brief TWI events handler. 
*/ 
void twi_pah8002_handler(nrf_drv_twi_evt_t const * p_event, void * p_context) 
{    
     switch(p_event->type) 
     { 
         case NRF_DRV_TWI_EVT_DONE: 
             // If EVT_DONE (event done) is received a device is found and responding on that particular address 
             //printf("\r\n!****************************!\r\nDevice found at 7-bit address: !\r\n!****************************!\r\n\r\n");
             //device_found = true; 
             break; 
         case NRF_DRV_TWI_EVT_ADDRESS_NACK: 
             printf("No address ACK on address!\r\n");
             break; 
				 
         case NRF_DRV_TWI_EVT_DATA_NACK: 
             printf("No data ACK on address: !\r\n");
             break; 
				 
         default: 
             break;         
     }    
} 


void gpiote_in_evt_handler(nrf_drv_gpiote_pin_t pin, nrf_gpiote_polarity_t action)
{
	if(pin == GC_TWI_PAH8002_PIN_INTR  && action == NRF_GPIOTE_POLARITY_LOTOHI )
  {                                                               
		pah8002_intr_isr();                     
  }
}

void config_gpio_pah8002 (void)
{
//	int32_t err_code=0;
	/* reset */
	nrf_gpio_cfg_output(GC_TWI_PAH8002_PIN_RST);
	nrf_gpio_pin_clear(GC_TWI_PAH8002_PIN_RST);
	nrf_delay_ms(100);
 	nrf_gpio_pin_set(GC_TWI_PAH8002_PIN_RST);

	nrf_drv_gpiote_in_config_t in_config = GPIOTE_CONFIG_IN_SENSE_LOTOHI(true);
	//in_config.pull = NRF_GPIO_PIN_PULLUP;

	//printf("\r\n * config PAH INT GPIOTE");
#ifndef _FIT_ETRI_TEST_ 
	if ( nrf_drv_gpiote_in_init(GC_TWI_PAH8002_PIN_INTR, &in_config, gpiote_in_evt_handler) != NRF_SUCCESS )
		printf("\r\n twi GPIOTE input init. fail");
#else
	nrf_drv_gpiote_in_init(GC_TWI_PAH8002_PIN_INTR, &in_config, gpiote_in_evt_handler);
#endif
	
	nrf_drv_gpiote_in_event_enable(GC_TWI_PAH8002_PIN_INTR, true);
}

void twi_pah8002_master_init(void)
{
	ret_code_t err_code;
	
	const nrf_drv_twi_config_t twi_master2_config = {
       .scl                			= TWI1_CONFIG_SCL,
       .sda                		= TWI1_CONFIG_SDA,
       .frequency          = NRF_TWI_FREQ_400K,
       .interrupt_priority = APP_IRQ_PRIORITY_HIGH
  };
	
	err_code = nrf_drv_twi_init(&m_twi_master_2, &twi_master2_config, NULL, NULL);
	if (err_code == NRF_SUCCESS)
	{
		printf("\r\n I2C PAHR init(id %d). done", m_twi_master_2.drv_inst_idx);
	}
	else
	{
		printf("\r\n I2C PAHR. fail");
	}
  APP_ERROR_CHECK(err_code);
		
  nrf_drv_twi_enable(&m_twi_master_2);
}


void gc_twi_pah8002_init(void)
{
	config_gpio_pah8002();
	
  twi_pah8002_master_init();
	
}
