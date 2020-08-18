/* Includes ------------------------------------------------------------------*/
#include <stdio.h>
#include <string.h>
#include "spi_pah8002.h"
#include "spi_master_previous.h"
#include  "spi.h"
#include "nrf_delay.h"
#ifdef GC_NRF52_SPI_FLASH
	#include "nrf_error.h"
	#include "nrf_drv_spi.h"
	#include "nrf_drv_config.h"
	#include "app_util_platform.h"
	#include "nrf_drv_ppi.h"
	#include "nrf_ppi.h"
	#include "nrf_drv_gpiote.h"
	#include "pah8002.h"
#endif /* GC_NRF52_SPI_FLASH */

const nrf_drv_spi_t spi_pah8002 = NRF_DRV_SPI_INSTANCE(GC_SPI_PAH8002_INSTANCE);  /**< SPI instance. */

static volatile bool spi_xfer_done;  /**< Flag used to indicate that SPI instance completed the transfer. */
nrf_ppi_channel_t pah8002_ppi_channel_workaround;
static volatile bool m_transfer_completed = true; /**< A flag to inform about completed transfer. */

//uint8_t pah8002_rx_buff[4];
//uint8_t pah8002_reg_rx;



//////////////////////////////////////////////////전임자 미사용 코드///////////////////////////////////////////////////////////////////

#if 0 /* hkim 2016.7.17 */
/**@brief Function for SPI master event callback.
 *
 * Upon receiving an SPI transaction complete event, checks if received data are valid.
 *
 * @param[in] spi_master_evt    SPI master driver event. 
 */
static void spi_master_pah8002_event_handler(nrf_drv_spi_evt_t const * p_event)
{
    uint32_t err_code;
		err_code = NRF_SUCCESS;
		printf("\r\n SPI-8002 event : %d", p_event->type);
    switch (p_event->type)
    {
        
			  case NRF_DRV_SPI_EVENT_DONE:   
						
						//disable the ppi channel
            err_code = nrf_drv_ppi_channel_disable(pah8002_ppi_channel_workaround);
						
						//disable a GPIOTE output pin task
						nrf_drv_gpiote_out_task_disable(SPI2_CONFIG_SCK_PIN);
						nrf_drv_gpiote_in_uninit(SPI2_CONFIG_SCK_PIN); //added by hkim 2016.6.2
				
						printf("\r\n SPI Rx : 0x%x 0x%x 0x%x 0x%x", pah8002_rx_buff[0], pah8002_rx_buff[1], pah8002_rx_buff[2], pah8002_rx_buff[3]);
						
						memcpy(&pah8002_reg_rx, &pah8002_rx_buff[1],sizeof(uint8_t));
						memset(pah8002_rx_buff, 0, sizeof(pah8002_rx_buff));
				
					   printf("\r\nReg Rx : 0x%x", pah8002_reg_rx);
						// Check if data are valid.
            //err_code = bsp_indication_set(BSP_INDICATE_RCV_OK);
            //APP_ERROR_CHECK(err_code);

            // Inform application that transfer is completed.
            spi_xfer_done = true;
            break;
				
				//if event is not equal to NRF_DRV_SPI_EVENT_DONE, then nothing happens
        default:
            // No implementation needed.
            break;
    }
}

void in_gpiote_pah8002_pin_handler(nrf_drv_gpiote_pin_t pin, nrf_gpiote_polarity_t action)
{
	nrf_drv_gpiote_out_toggle(SPI2_CONFIG_SCK_PIN);
}

int32_t spi_pah8002_send_recv(	uint8_t * const p_tx_data, 
																										uint8_t tx_len,
																										uint8_t * p_rx_data, 
																										uint8_t rx_len)
{
	uint8_t dump_tx[1];
	
	uint32_t err_code=0;
	NRF_SPIM_Type * p_8002_spim = spi_pah8002.p_registers;
	
	memcpy(dump_tx, p_tx_data, tx_len);
	printf("\r\n [Dev-SPI] tx-data 0x%02x, tx_len %d rx_len %d", dump_tx[0], tx_len, rx_len);
	
	if (tx_len == 1){
		ret_code_t err_code;
 
    // initializes the GPIOTE channel so that SCK toggles generates events
    nrf_drv_gpiote_in_config_t config = GPIOTE_CONFIG_IN_SENSE_TOGGLE(true);
    err_code = nrf_drv_gpiote_in_init(SPI2_CONFIG_SCK_PIN, &config, in_gpiote_pah8002_pin_handler);
		if (err_code != NRF_SUCCESS)
		{
			printf("\r\nGPIOTE in-init error(code %d)", err_code);
			return false;
		}
    
		//allocates the first unused PPI Channel
		err_code = nrf_drv_ppi_channel_alloc(&pah8002_ppi_channel_workaround);
		if (err_code != NRF_SUCCESS)
		{
			printf("\r\nPPI-ch alloc error(code %d)", err_code);
			return false;
		}
				
		err_code = nrf_drv_ppi_channel_assign(pah8002_ppi_channel_workaround, 
																					nrf_drv_gpiote_in_event_addr_get(SPI2_CONFIG_SCK_PIN), 
																					(uint32_t)&p_8002_spim->TASKS_STOP);
		if (err_code != NRF_SUCCESS)
		{
			printf("\r\nPPI-ch assign error(code %d)", err_code);
			return false;
		}
    
		//enable the PPI channel.
		err_code = nrf_drv_ppi_channel_enable(pah8002_ppi_channel_workaround);
		if (err_code != NRF_SUCCESS)
			printf("\r\nPPI-ch enable error(code %d)", err_code);

		//enable the GPIOTE output pin task.
		nrf_drv_gpiote_in_event_enable(SPI2_CONFIG_SCK_PIN, false);

		//Start transfer of data
		spi_xfer_done = false;
		err_code = nrf_drv_spi_transfer(&spi_pah8002, p_tx_data, tx_len, p_rx_data, rx_len);
		printf("\r\n PAH8002-Tx result(1 Byte) : %d", err_code);
		if (err_code != NRF_SUCCESS)
		{
			printf("\r\nSPI transfer error(code %d)", err_code);
			return false;
		}
#if 0
		err_code = nrf_drv_ppi_channel_disable(pah8002_ppi_channel_workaround);
		if (err_code != NRF_SUCCESS)
		{
			printf("\r\nPPI-ch disable error(code %d)", err_code);
			return false;
		}
		
		nrf_drv_gpiote_out_task_disable(SPI2_CONFIG_SCK_PIN);
		nrf_drv_gpiote_in_uninit(SPI2_CONFIG_SCK_PIN); //added by hkim 2016.6.21 
#endif 
    //uninitializes the gpiote channel
		//the gpiote channel is represented by a 32-bit variable
    //not sure if we need to unintitialize the pin, or if we can just disable it
    //nrf_drv_gpiote_out_uninit(SPIM0_SCK_PIN); 
	}
  else
	{
		// Start transfer.
		spi_xfer_done = false;
		err_code = nrf_drv_spi_transfer(&spi_pah8002, p_tx_data, tx_len, p_rx_data, rx_len);
		printf("\r\n PAH8002-Tx result : %d", err_code);
		if (err_code != NRF_SUCCESS)
		{
			printf("\r\nSPI transfer error(code %d)", err_code);
			return false;
		}
  }
 
	return NRF_SUCCESS;
	//nrf_delay_ms(delay);
}
#endif /* if 0 hkim 2016.7.17 */


//////////////////////////////////////////////////전임자 미사용 코드///////////////////////////////////////////////////////////////////













//실제 쓰는 함수들
void gpiote_in_evt_handler(nrf_drv_gpiote_pin_t pin, nrf_gpiote_polarity_t action)
{
	//printf("\r\ngpiote_in_evt_handler%d != %d 1",pin, action);
	
	//printf("\r\n GC_SPI_PAH8002_PIN_INTR[%d]/// pin[%d]",GC_SPI_PAH8002_PIN_INTR, pin);
	//printf("\r\n NRF_GPIOTE_POLARITY_LOTOHI[%d]/// action[%d]",NRF_GPIOTE_POLARITY_LOTOHI, action);
	
	if(pin == GC_SPI_PAH8002_PIN_INTR && action == NRF_GPIOTE_POLARITY_LOTOHI )
	//if(pin == GC_SPI_PAH8002_PIN_INTR && action == NRF_GPIOTE_POLARITY_HITOLO )
	//if(pin == GC_SPI_PAH8002_PIN_INTR)
  {
		//printf("\r\ngpiote_in_evt_handler%d != %d 2",pin,action);
		pah8002_intr_isr();                     
  }
	/*if(nrf_drv_gpiote_in_is_set(pin)){
		
		printf("\r\n NRF_GPIOTE_POLARITY_LOTOHI[%d]/// action[%d]",NRF_GPIOTE_POLARITY_LOTOHI, action);
		pah8002_intr_isr();    
	}
	else{
		printf("\r\n NRF_GPIOTE_POLARITY_HITOLO[%d]/// action[%d]",NRF_GPIOTE_POLARITY_HITOLO, action);
		pah8002_intr_isr();    
	}*/
}

void config_gpio_pah8002 (void)
{
//	int32_t err_code=0;
	/* reset */
	nrf_gpio_cfg_output(GC_SPI_PAH8002_PIN_RST);
	nrf_gpio_pin_clear(GC_SPI_PAH8002_PIN_RST);
	nrf_delay_ms(100);
 	nrf_gpio_pin_set(GC_SPI_PAH8002_PIN_RST);
	//nrf_delay_ms(100);
	//nrf_gpio_cfg_input(GC_SPI_PAH8002_PIN_INTR, NRF_GPIO_PIN_NOPULL);
	nrf_gpio_cfg_input(GC_SPI_PAH8002_PIN_INTR, NRF_GPIO_PIN_PULLUP);
	//nrf_delay_ms(100);
	nrf_drv_gpiote_in_config_t in_config = GPIOTE_CONFIG_IN_SENSE_LOTOHI(true);
	//nrf_drv_gpiote_in_config_t in_config = GPIOTE_CONFIG_IN_SENSE_HITOLO(true);
	//nrf_drv_gpiote_in_config_t in_config = GPIOTE_CONFIG_IN_SENSE_TOGGLE(true);

	//printf("\r\n * config PAH INT GPIOTE");
#ifndef _FIT_ETRI_TEST_ 
	uint32_t err_code;
 err_code =	nrf_drv_gpiote_in_init(GC_SPI_PAH8002_PIN_INTR, &in_config, gpiote_in_evt_handler);
	//APP_ERROR_CHECK(err_code);
	if ( err_code != NRF_SUCCESS )
		printf("\r\n spi GPIOTE input init. fail%d != %d err_code= %d ?= %d ", nrf_drv_gpiote_in_init(GC_SPI_PAH8002_PIN_INTR, &in_config, gpiote_in_evt_handler),NRF_SUCCESS,err_code,GC_SPI_PAH8002_PIN_INTR);
#else
	//uint32_t err_code;
 //err_code =	
	nrf_drv_gpiote_in_init(GC_SPI_PAH8002_PIN_INTR, &in_config, gpiote_in_evt_handler);
	//APP_ERROR_CHECK(err_code);
	//if ( err_code != NRF_SUCCESS )
	//	printf("\r\n spi GPIOTE input init. fail%d != %d err_code= %d ?= %d ", nrf_drv_gpiote_in_init(GC_SPI_PAH8002_PIN_INTR, &in_config, gpiote_in_evt_handler),NRF_SUCCESS,err_code,GC_SPI_PAH8002_PIN_INTR);
#endif
	
	nrf_drv_gpiote_in_event_enable(GC_SPI_PAH8002_PIN_INTR, true);
}

void spi_pah8002_master_init(void)
{
	int32_t err_code=0;
	nrf_drv_spi_config_t spi_pah8002_config = NRF_DRV_SPI_DEFAULT_CONFIG(GC_SPI_PAH8002_INSTANCE);
  
	spi_pah8002_config.sck_pin 				= SPI2_CONFIG_SCK_PIN;
	spi_pah8002_config.mosi_pin      = SPI2_CONFIG_MOSI_PIN;
	spi_pah8002_config.miso_pin      = SPI2_CONFIG_MISO_PIN;
	spi_pah8002_config.ss_pin 					= GC_SPI_PAH8002_PIN_CS;
	spi_pah8002_config.irq_priority = APP_IRQ_PRIORITY_HIGH;
	spi_pah8002_config.orc            = 0xFF;
	spi_pah8002_config.frequency    = NRF_DRV_SPI_FREQ_125K;   
  spi_pah8002_config.mode         = NRF_DRV_SPI_MODE_3;
	spi_pah8002_config.bit_order    = NRF_DRV_SPI_BIT_ORDER_MSB_FIRST;
  
	//원본 설정은 좀 아래와 같이 좀 다르며 SPI핀 설정 디폴트 이용중
	//spi_pah8002_config.irq_priority = APP_IRQ_PRIORITY_HIGH;
	//spi_pah8002_config.frequency    = NRF_DRV_SPI_FREQ_125K;    // NRF_DRV_SPI_FREQ_125K, NRF_DRV_SPI_FREQ_250K, NRF_DRV_SPI_FREQ_500K, ..., NRF_DRV_SPI_FREQ_4M
  
	
	#if 0 
	printf("\r\n** SPI PAH8002 config result");
	printf("\r\n sck_pin %d, mosi_pin %d, miso_pin %d, ss_pin %d, irq_priority %d, orc %d, freq 0x%8x, mode %d, bit-order %d",
			spi_pah8002_config.sck_pin, spi_pah8002_config.mosi_pin, spi_pah8002_config.miso_pin, spi_pah8002_config.ss_pin, spi_pah8002_config.irq_priority,
			spi_pah8002_config.orc, spi_pah8002_config.frequency, spi_pah8002_config.mode, spi_pah8002_config.bit_order );
	#endif 
	
  //APP_ERROR_CHECK(nrf_drv_spi_init(&spi_pah8002, &spi_pah8002_config, spi_master_pah8002_event_handler));
	err_code = nrf_drv_spi_init(&spi_pah8002, &spi_pah8002_config, NULL);
#ifndef _FIT_ETRI_TEST_ 
	if (err_code == NRF_SUCCESS)
		printf("\r\n PAH8002 SPI initialization. done");
	else
		printf("\r\n PAH8002 SPI initialization. fail");
#endif
}


void gc_spi_pah8002_init(void)
{
	config_gpio_pah8002();
  spi_pah8002_master_init();
}
