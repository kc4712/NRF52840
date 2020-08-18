#include <stdio.h>
#include "spi_flash.h"
#include "spi_master_previous.h"
#include  "spi.h"
#include "nrf_error.h"
#include "nrf_drv_spi.h"
#include "nrf_drv_config.h"
#include "app_util_platform.h"
#include "nrf_drv_ppi.h"
#include "nrf_ppi.h"
#include "nrf_drv_gpiote.h"
#include "gc_drv_spi.h"

//extern const nrf_drv_spi_t spi;

static const nrf_drv_spi_t spi_flash = NRF_DRV_SPI_INSTANCE(GC_SPI_FLASH_INSTANCE);  /**< SPI instance. */
static const nrf_drv_spi_t spi_hr 	= NRF_DRV_SPI_INSTANCE(GC_SPI_HR_INSTANCE);  /**< SPI instance. */

static volatile bool spi_xfer_done;  /**< Flag used to indicate that SPI instance completed the transfer. */
nrf_ppi_channel_t ppi_channel_workaround;

/**
 * @brief Work-around for transmitting 1 byte with SPIM.
 *
 * @param spim: The SPIM instance that is in use.
 * @param ppi_channel: An unused PPI channel that will be used by the
        workaround.
 * @param gpiote_channel: An unused GPIOTE channel that will be used by
        the workaround.
 *
 * @warning Must not be used when transmitting multiple bytes.
 * @warning After this workaround is used, the user must reset the PPI
        channel and the GPIOTE channel before attempting to transmit multiple
        bytes.
 */
void setup_workaround_for_ftpan_58(NRF_SPIM_Type *spim, 
                                   uint32_t ppi_channel, 
                                   uint32_t gpiote_channel){

  // Create an event when SCK toggles.
  NRF_GPIOTE->CONFIG[gpiote_channel] = 
      (GPIOTE_CONFIG_MODE_Event << GPIOTE_CONFIG_MODE_Pos) | 
      (spim->PSEL.SCK << GPIOTE_CONFIG_PSEL_Pos) | 
      (GPIOTE_CONFIG_POLARITY_Toggle <<GPIOTE_CONFIG_POLARITY_Pos);

  // Stop the spim instance when SCK toggles.
  NRF_PPI->CH[ppi_channel].EEP = (uint32_t)&NRF_GPIOTE->EVENTS_IN[gpiote_channel];
  NRF_PPI->CH[ppi_channel].TEP = (uint32_t)&spim->TASKS_STOP;
  NRF_PPI->CHENSET = 1U << ppi_channel;
}
																	 
void spi_master_event_handler(nrf_drv_spi_evt_t const * p_event)
{
	uint32_t err_code = NRF_SUCCESS;
  switch (p_event->type)
  {
		//if event is equal to NRF_DRV_SPI_EVENT_DONE, then this block is executed to check if data is valid
    case NRF_DRV_SPI_EVENT_DONE:   

			//disable the ppi channel
			err_code = nrf_drv_ppi_channel_disable(ppi_channel_workaround);

			//disable a GPIOTE output pin task.
			nrf_drv_gpiote_out_task_disable(SPI1_CONFIG_SCK_PIN);

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

void in_pin_handler(nrf_drv_gpiote_pin_t pin, nrf_gpiote_polarity_t action)
{
	nrf_drv_gpiote_out_toggle(SPI1_CONFIG_SCK_PIN);
}

#if 0
/* tx_data may include command, register address and etc */
 void spi_send_recv(uint8_t * const p_tx_data, uint8_t * const p_rx_data, const uint16_t  len)
{
	uint32_t err_code=0;
	NRF_SPIM_Type * p_spim = spi_flash.p_registers;
	
	if (len == 1){
		ret_code_t err_code;
	
    // initializes the GPIOTE channel so that SCK toggles generates events
    nrf_drv_gpiote_in_config_t config = GPIOTE_CONFIG_IN_SENSE_TOGGLE(true);
    err_code = nrf_drv_gpiote_in_init(SPI1_CONFIG_SCK_PIN, &config, in_pin_handler);
		if (err_code != NRF_SUCCESS)
			printf("\r\nGPIOTE in-init error(code %d)", err_code);
    
		//allocates the first unused PPI Channel
		err_code = nrf_drv_ppi_channel_alloc(&ppi_channel_workaround);
		if (err_code != NRF_SUCCESS)
			printf("\r\nPPI-ch alloc error(code %d)", err_code);
				
		err_code = nrf_drv_ppi_channel_assign(ppi_channel_workaround, 
																					nrf_drv_gpiote_in_event_addr_get(SPI1_CONFIG_SCK_PIN), 
																					(uint32_t)&p_spim->TASKS_STOP);
		if (err_code != NRF_SUCCESS)
			printf("\r\nPPI-ch assign error(code %d)", err_code);
    
		//enable the PPI channel.
		err_code = nrf_drv_ppi_channel_enable(ppi_channel_workaround);
		if (err_code != NRF_SUCCESS)
			printf("\r\nPPI-ch enable error(code %d)", err_code);

		//enable the GPIOTE output pin task.
		nrf_drv_gpiote_in_event_enable(SPI1_CONFIG_SCK_PIN, false);

		//Start transfer of data
		err_code = nrf_drv_spi_transfer(&spi_flash, p_tx_data, len, p_rx_data, len);
		if (err_code != NRF_SUCCESS)
			printf("\r\nSPI transfer error(code %d)", err_code);

		err_code = nrf_drv_ppi_channel_disable(ppi_channel_workaround);
		if (err_code != NRF_SUCCESS)
			printf("\r\nPPI-ch disable error(code %d)", err_code);
		
		nrf_drv_gpiote_out_task_disable(SPI1_CONFIG_SCK_PIN);
		nrf_drv_gpiote_in_uninit(SPI1_CONFIG_SCK_PIN); //added by hkim 2016.6.21 

    //uninitializes the gpiote channel
		//the gpiote channel is represented by a 32-bit variable
    //not sure if we need to unintitialize the pin, or if we can just disable it
    //nrf_drv_gpiote_out_uninit(SPIM0_SCK_PIN); 
	}
  else
	{
		// Start transfer.
		err_code = nrf_drv_spi_transfer(&spi_flash, p_tx_data, len, p_rx_data, (uint8_t)len);
		if (err_code != NRF_SUCCESS)
			printf("\r\nSPI transfer error(code %d)", err_code);
  }
 
	//nrf_delay_ms(delay);
}
#else
 void spi_send_recv(	uint8_t const spi_instance, uint8_t * const p_tx_data, uint8_t * const p_rx_data, const uint16_t  len)
{
	uint32_t err_code=0;
	NRF_SPIM_Type * p_flash_spim = spi_flash.p_registers;
	
	if (len == 1){
		ret_code_t err_code;
	
    // initializes the GPIOTE channel so that SCK toggles generates events
    nrf_drv_gpiote_in_config_t config = GPIOTE_CONFIG_IN_SENSE_TOGGLE(true);
    err_code = nrf_drv_gpiote_in_init(SPI1_CONFIG_SCK_PIN, &config, in_pin_handler);
		if (err_code != NRF_SUCCESS)
			printf("\r\nGPIOTE in-init error(code %d)", err_code);
    
		//allocates the first unused PPI Channel
		err_code = nrf_drv_ppi_channel_alloc(&ppi_channel_workaround);
		if (err_code != NRF_SUCCESS)
			printf("\r\nPPI-ch alloc error(code %d)", err_code);
				
		err_code = nrf_drv_ppi_channel_assign(ppi_channel_workaround, 
																					nrf_drv_gpiote_in_event_addr_get(SPI1_CONFIG_SCK_PIN), 
																					(uint32_t)&p_flash_spim->TASKS_STOP);
		if (err_code != NRF_SUCCESS)
			printf("\r\nPPI-ch assign error(code %d)", err_code);
    
		//enable the PPI channel.
		err_code = nrf_drv_ppi_channel_enable(ppi_channel_workaround);
		if (err_code != NRF_SUCCESS)
			printf("\r\nPPI-ch enable error(code %d)", err_code);

		//enable the GPIOTE output pin task.
		nrf_drv_gpiote_in_event_enable(SPI1_CONFIG_SCK_PIN, false);

		//Start transfer of data
		switch (spi_instance)
		{
			case GC_SPI_FLASH_INSTANCE:
				err_code = nrf_drv_spi_transfer(&spi_flash, p_tx_data, len, p_rx_data, len);
			break;
			
			case GC_SPI_HR_INSTANCE:
				err_code = nrf_drv_spi_transfer(&spi_hr, p_tx_data, len, p_rx_data, len);
			break;
			
			default:
				break;
		}
		if (err_code != NRF_SUCCESS)
			printf("\r\nSPI transfer error(instance %d, code %d)", spi_instance, err_code);

		err_code = nrf_drv_ppi_channel_disable(ppi_channel_workaround);
		if (err_code != NRF_SUCCESS)
			printf("\r\nPPI-ch disable error(code %d)", err_code);
		
		nrf_drv_gpiote_out_task_disable(SPI1_CONFIG_SCK_PIN);
		nrf_drv_gpiote_in_uninit(SPI1_CONFIG_SCK_PIN); //added by hkim 2016.6.21 

    //uninitializes the gpiote channel
		//the gpiote channel is represented by a 32-bit variable
    //not sure if we need to unintitialize the pin, or if we can just disable it
    //nrf_drv_gpiote_out_uninit(SPIM0_SCK_PIN); 
	}
  else
	{
		// Start transfer.
		switch (spi_instance)
		{
			case GC_SPI_FLASH_INSTANCE:
				err_code = nrf_drv_spi_transfer(&spi_flash, p_tx_data, len, p_rx_data, (uint8_t)len);
			break;
			
			case GC_SPI_HR_INSTANCE:
				err_code = nrf_drv_spi_transfer(&spi_hr, p_tx_data, len, p_rx_data, (uint8_t)len);
			break;
			
			default:
				break;
		}
		if (err_code != NRF_SUCCESS)
			printf("\r\nSPI transfer error(code %d)", err_code);
  }
 
	//nrf_delay_ms(delay);
}
#endif 

void gc_spi_flash_init(void)
{
	int32_t err_code=0;
	nrf_drv_spi_config_t spi_flash_config = NRF_DRV_SPI_DEFAULT_CONFIG(GC_SPI_FLASH_INSTANCE);
  spi_flash_config.ss_pin = GC_SPI_FLASH_CS_PIN;
	
	printf("\r\n** SPI Flash config result");
	printf("\r\n sck_pin %d, mosi_pin %d, miso_pin %d, ss_pin %d, irq_priority %d, orc %d, freq 0x%8x, mode %d, bit-order %d",
			spi_flash_config.sck_pin, spi_flash_config.mosi_pin, spi_flash_config.miso_pin, spi_flash_config.ss_pin, spi_flash_config.irq_priority,
			spi_flash_config.orc, spi_flash_config.frequency, spi_flash_config.mode, spi_flash_config.bit_order );
	
  APP_ERROR_CHECK(nrf_drv_spi_init(&spi_flash, &spi_flash_config, NULL));
	
	/* 
			NRF52 can't transmit one byte through SPIM. 
			Workaround function is setup_workaround_for_ftpan_58()
			Following link privide you for more information.
			http://infocenter.nordicsemi.com/pdf/nRF52832_Errata_v1.1.pdf
	*/
  err_code = nrf_drv_ppi_init();
	if (err_code != NRF_SUCCESS)
			printf("\r\nPPI init error(code %d)", err_code);

  err_code = nrf_drv_gpiote_init();
	if (err_code != NRF_SUCCESS)
			printf("\r\nGPIOTE init error(code %d)", err_code);
}
