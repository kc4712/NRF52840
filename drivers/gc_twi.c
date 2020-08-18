#include <stdint.h>
#include <string.h>
#include "nordic_common.h"
#include "nrf.h"
#include "ble_hci.h"
#include "ble_advdata.h"
#include "ble_advertising.h"
#include "ble_conn_params.h"
#include "softdevice_handler.h"
#include "app_timer.h"
#include "app_button.h"
#include "ble_nus.h"
#include "app_uart.h"
#include "app_util_platform.h"
#include "bsp.h"
#include "bsp_btn_ble.h"
#include "app_timer.h"
#include "device_manager.h"
#include "pstorage.h"
#include "define.h"
#include  "scc.h"
#include "nrf_drv_twi.h"
#include "gc_twi.h"
#include "gc_twi.h"
#include "command.h" 
#include "print.h"
#include "bma2x2.h"
#include "oled.h"
#include "rtc.h"
#include "gc_time.h"
#include "spi_flash.h"
#include "font.h"
#include "pah8002.h"

extern const nrf_drv_twi_t m_twi_master_1;
#ifdef GC_COACH_FIT_PCB_PAHR_IF_TWI 
extern const nrf_drv_twi_t m_twi_master_2;
#endif 

uint8_t gc_twi_write(uint8_t slave_addr, uint8_t reg_addr, uint8_t data)
{
	uint8_t i=0;
	const nrf_drv_twi_t *twi_instance=NULL;
	uint32_t err_code=0;
	uint8_t	tx_data[2];
	uint8_t	tx_len=0;
	
	memset(tx_data, 0, sizeof(tx_data));
	
	tx_data[0] 	= reg_addr;
	tx_data[1] 	= data;
	tx_len					= 2;

#ifdef GC_COACH_FIT_PCB_PAHR_IF_TWI 	
	if (slave_addr == TWI_SLAVE_ADDR_PAH8002)
		twi_instance = &m_twi_master_2;
	else
		twi_instance = &m_twi_master_1;
#else
	twi_instance = &m_twi_master_1;
#endif

	//SEGGER_RTT_printf(0, "\r\n twiw-1");
	delay_ms(1);
	for (i=0; i <= RW_MAX_RETRY; i++)
	{
		err_code = nrf_drv_twi_tx(twi_instance, slave_addr, (const uint8_t *)tx_data, tx_len, true);
		if (err_code != NRF_SUCCESS)		
		{
			printf("\r\n TWI write(0x%x) retry %d", reg_addr, i);
			delay_ms(1);
			continue;
		}
		else	break;
	}
	//SEGGER_RTT_printf(0, "\r\n twiw-2");
	if (i == RW_MAX_RETRY)
	{
		printf("\n\rTWI write(0x%x) finish(error code %d)", reg_addr, err_code);
		return err_code;
	}

	return NRF_SUCCESS;
}

uint8_t gc_twi_read(uint8_t slave_addr, uint8_t reg_addr, uint8_t *rx_data, uint32_t length)
{
	int32_t err_code=0;
	uint8_t i=0;
	const nrf_drv_twi_t *twi_instance=NULL;

#ifdef GC_COACH_FIT_PCB_PAHR_IF_TWI 
	if (slave_addr == TWI_SLAVE_ADDR_PAH8002)
		twi_instance = &m_twi_master_2;
	else
		twi_instance = &m_twi_master_1;
#else
	twi_instance = &m_twi_master_1;
#endif 
		
	//printf("\r\n instance %d 0x%x 0x%x %d", twi_instance->drv_inst_idx,slave_addr, reg_addr, length);
	delay_ms(1);
	for (i=0; i < RW_MAX_RETRY; i++)
	{
		err_code = nrf_drv_twi_tx(twi_instance, slave_addr, (const uint8_t *)&reg_addr, 1, true);
		if (err_code != NRF_SUCCESS)		
		{
			printf("\r\n [R1]TWI write(0x%x) retry %d",reg_addr, i);
			delay_ms(1);
			continue;
		}
		else	break;
	}
	if (i == RW_MAX_RETRY)
	{
		printf("\n\r[R] TWI write(0x%x) finish (code %d)", reg_addr, err_code);
		return err_code;
	}
	
	delay_ms(1);
	for (i=0; i < RW_MAX_RETRY; i++)
	{
		err_code = nrf_drv_twi_rx(twi_instance, slave_addr, rx_data, length);
		if (err_code != NRF_SUCCESS)		
		{
			printf("\r\n [R2]TWI write retry %d", i);
			delay_ms(1);
			continue;
		}
		else	break;
	}
	if (i == RW_MAX_RETRY)
	{
		printf("\n\rTWI read finish(error code %d)", err_code);
		return err_code;
	}
	
	return NRF_SUCCESS;
}
