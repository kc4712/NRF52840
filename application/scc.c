
/* Copyright (c) 2009 Nordic Semiconductor. All Rights Reserved.
 *
 * The information contained herein is property of Nordic Semiconductor ASA.
 * Terms and conditions of usage are described in detail in NORDIC
 * SEMICONDUCTOR STANDARD SOFTWARE LICENSE AGREEMENT.
 *
 * Licensees are granted free, non-transferable use of the information. NO
 * WARRANTY of ANY KIND is provided. This heading must NOT be removed from
 * the file.
 *
 */
#include "define.h"

#include <stdint.h>

#include <string.h>

#include "nrf.h"
//#include "simple_uart.h"
//#include "nrf_delay.h"
//#include "nrf_gpio.h"

//#include "spi_master.h"
//#include "scc.h"
//#include "rtc.h"
//#include  "spi.h"
//#include "afe4400.h"


int uart_get(char* inchar)
{


	if (NRF_UART0->EVENTS_RXDRDY != 1)
	{
		return false;
	}
  
	NRF_UART0->EVENTS_RXDRDY = 0;
	*inchar = (char)NRF_UART0->RXD;
	
	
	return true;
}




void uart_put(char cr)
{

  NRF_UART0->TXD = cr;

  while (NRF_UART0->EVENTS_TXDRDY!=1)
  {
    // Wait for TXD data to be sent
  }

  NRF_UART0->EVENTS_TXDRDY=0;

  //app_uart_put(cr); //provided by SDK 11.0

}



void uart_config(unsigned char txd_pin_number, unsigned char rxd_pin_number)
{
/** @snippet [Configure UART RX and TX pin] */

/*
  nrf_gpio_cfg_output(txd_pin_number);
  nrf_gpio_cfg_input(rxd_pin_number, NRF_GPIO_PIN_NOPULL);  

  NRF_UART0->PSELTXD = txd_pin_number;
  NRF_UART0->PSELRXD = rxd_pin_number;


	NRF_UART0->BAUDRATE         = (UART_BAUDRATE_BAUDRATE_Baud115200 << UART_BAUDRATE_BAUDRATE_Pos);



  NRF_UART0->ENABLE           = (UART_ENABLE_ENABLE_Enabled << UART_ENABLE_ENABLE_Pos);
  NRF_UART0->TASKS_STARTTX    = 1;
  NRF_UART0->TASKS_STARTRX    = 1;
  NRF_UART0->EVENTS_RXDRDY    = 0;

*/
}





