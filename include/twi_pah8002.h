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
#ifndef __TWI_PAH8002_H
#define __TWI_PAH8002_H

#include "nrf_drv_gpiote.h"

/**************** Pin definition ****************/
#ifdef GC_COACH_FIT_PCB
  #define GC_TWI_PAH8002_PIN_INTR   		27 	/**< SPI interrupt Pin.*/
	#define GC_TWI_PAH8002_PIN_RST   		6 	/**< reset Pin.*/
#elif defined GC_COACH_FIT_DK52
  #define GC_TWI_PAH8002_PIN_INTR   		8 	/**< SPI interrupt Pin.*/
  #define GC_TWI_PAH8002_PIN_RST   		22 	/**< SPI reset Pin.*/
#else
#endif 
	 
void in_gpiote_pah8002_pin_handler(nrf_drv_gpiote_pin_t pin, nrf_gpiote_polarity_t action);

void config_gpio_pah8002 (void);
void gc_twi_pah8002_init(void);
void twi_pah8002_master_init(void);

#endif /* __TWI_PAH8002_H */


/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
