/**
  ******************************************************************************
  * @file    spi_flash.h
  * @author  MCD Application Team
  * @version V1.1.0
  * @date    13-April-2012
  * @brief   This file contains all the functions prototypes for the spi_flash
  *          firmware driver.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; COPYRIGHT 2012 STMicroelectronics</center></h2>
  *
  * Licensed under MCD-ST Liberty SW License Agreement V2, (the "License");
  * You may not use this file except in compliance with the License.
  * You may obtain a copy of the License at:
  *
  *        http://www.st.com/software_license_agreement_liberty_v2
  *
  * Unless required by applicable law or agreed to in writing, software
  * distributed under the License is distributed on an "AS IS" BASIS,
  * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  * See the License for the specific language governing permissions and
  * limitations under the License.
  *
  ******************************************************************************
  */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __GC_DRV_SPI_H
#define __GC_DRV_SPI_H

#include "nrf_drv_gpiote.h"

/**************** Pin definition ****************/
#ifdef GC_COACH_FIT_PCB
	#ifdef GC_COACH_FIT_HW_1_0  
			#define GC_SPI_PAH8002_PIN_CS			6
			#define GC_SPI_PAH8002_PIN_INTR	27
			#define GC_SPI_PAH8002_PIN_RST		7
	#elif defined GC_COACH_FIT_HW_1_2
			#define GC_SPI_PAH8002_PIN_CS			31
			#define GC_SPI_PAH8002_PIN_INTR	27
			#define GC_SPI_PAH8002_PIN_RST		6
	#else
	#endif 
#elif defined GC_COACH_FIT_DK52
	#define GC_SPI_PAH8002_PIN_INTR   		8 		/**< SPI interrupt Pin.*/
  #define GC_SPI_PAH8002_PIN_RST   		22 	/**< SPI reset Pin.*/
#else
#endif 

void in_gpiote_pah8002_pin_handler(nrf_drv_gpiote_pin_t pin, nrf_gpiote_polarity_t action);
int32_t spi_pah8002_send_recv(	uint8_t * const p_tx_data, 
																										uint8_t tx_len,
																										uint8_t * p_rx_data, 
																										uint8_t rx_len);
void config_gpio_pah8002 (void);
void spi_pah8002_master_init(void);
void gc_spi_pah8002_init(void);


#endif /* __GC_DRV_SPI_H */


/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
