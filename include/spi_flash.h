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
#ifndef __SPI_FLASH_H
#define __SPI_FLASH_H

#include "nrf.h"

#ifdef __cplusplus
 extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
//#include "stm32l1xx.h"

#define SPI_MANUID											0xEF
#define SPI_FLASH_ID										0x4018

#define SPI_FLASH_PageSize      				256
#define SPI_FLASH_PerWritePageSize      256

/* Private define W25Q128 winbond spi flash - 16MByte. 		*/
/*--------------------------------------------------------------------------*/
#define W25X_WriteEnable		      			0x06
#define W25X_WriteDisable		      			0x04
#define W25X_ReadStatusReg		    			0x05
#define W25X_WriteStatusReg		    			0x01
#define W25X_ReadData			        			0x03
#define W25X_FastReadData		      			0x0B
#define W25X_FastReadDual		      			0x3B
#define W25X_PageProgram		      			0x02
#define W25X_BlockErase			      			0xD8
#define W25X_SectorErase		      			0x20
#define W25X_ChipErase			      			0xC7
#define W25X_PowerDown			      			0xB9
#define W25X_ReleasePowerDown	    			0xAB
#define W25X_DeviceID			        			0xAB
#define W25X_ManufactDeviceID   				0x90
#define W25X_JedecDeviceID		    			0x9F

#define WIP_Flag                  			0x01  /* Write In Progress (WIP) flag */

#define Dummy_Byte                			0xFF

#define SPI_FLASH_CS_LOW()       				GPIO_ResetBits(GPIOA, GPIO_Pin_4)
#define SPI_FLASH_CS_HIGH()      				GPIO_SetBits(GPIOA, GPIO_Pin_4)

int spi_send_recv(uint8_t * const p_tx_data, uint8_t * const p_rx_data, const uint16_t  len);

void SPI_FLASH_Init(void);
void SPI_FLASH_SectorErase(unsigned int SectorAddr);
void SPI_FLASH_BulkErase(void);
int32_t SPI_FLASH_PageWrite(unsigned char* pBuffer, unsigned int WriteAddr);
//void gc_SPI_FLASH_PageWrite(unsigned char* tx_data, unsigned char tx_len, unsigned int WriteAddr);
//void gc_SPI_FLASH_BufferRead(unsigned char* rx_data,unsigned char rx_len, unsigned int ReadAddr);
void SPI_FLASH_BufferWrite(unsigned char* pBuffer, unsigned int WriteAddr, unsigned short NumByteToWrite);
void SPI_FLASH_BufferRead(unsigned char* pBuffer, unsigned int ReadAddr);
unsigned int SPI_FLASH_ReadID(void);
unsigned int SPI_FLASH_ReadDeviceID(void);
void SPI_FLASH_StartReadSequence(unsigned int ReadAddr);
void SPI_Flash_PowerDown(void);
void SPI_Flash_WAKEUP(void);
void gc_spi_flash_init(void);


unsigned char SPI_FLASH_ReadByte(void);
unsigned char SPI_FLASH_SendByte(unsigned char byte);
unsigned short SPI_FLASH_SendHalfWord(unsigned short HalfWord);
int32_t SPI_FLASH_WriteEnable(void);
void SPI_FLASH_WaitForWriteEnd(void);

/*------------------------------------------------------------------------------*/
/* GD25Q64B  SPI Flash supported commands */

#define sFLASH_CMD_WRITE          0x02  /* Write to Memory instruction- page program. */
#define sFLASH_CMD_WRSR           0x01  /* Write Status Register instruction */
#define sFLASH_CMD_WREN           0x06  /* Write enable instruction */
#define sFLASH_CMD_READ           0x03  /* Read from Memory instruction */
#define sFLASH_CMD_RDSR           0x05  /* Read Status Register instruction  */
#define sFLASH_CMD_RDID           0x90  /* Read identification */
#define sFLASH_CMD_SE             0x20  /* Sector Erase instruction */
//#define sFLASH_CMD_BE             0x52  /* Bulk Erase instruction */
#define sFLASH_CMD_BE             0x52  /* block Erase instruction(32K) */

#define sFLASH_WIP_FLAG           0x01  /* Write In Progress (WIP) flag */

#define sFLASH_JedecDeviceID		  0x9F

#define sFLASH_DUMMY_BYTE         0xFF//0xA5
#define sFLASH_SPI_PAGESIZE       0x100   //page size is 256

/* M25P FLASH SPI Interface pins  */
/* SPI1 to SPI3 Changed....2014.09.19  */
#define sFLASH_SPI                           SPI1
#define sFLASH_SPI_CLK                       RCC_APB2Periph_SPI1
#define sFLASH_SPI_CLK_INIT                  RCC_APB2PeriphClockCmd

#define sFLASH_SPI_SCK_PIN                   GPIO_Pin_5
#define sFLASH_SPI_SCK_GPIO_PORT             GPIOA
#define sFLASH_SPI_SCK_GPIO_CLK              RCC_AHBPeriph_GPIOA
#define sFLASH_SPI_SCK_SOURCE                GPIO_PinSource5
#define sFLASH_SPI_SCK_AF                    GPIO_AF_SPI1

#define sFLASH_SPI_MISO_PIN                  GPIO_Pin_6
#define sFLASH_SPI_MISO_GPIO_PORT            GPIOA
#define sFLASH_SPI_MISO_GPIO_CLK             RCC_AHBPeriph_GPIOA
#define sFLASH_SPI_MISO_SOURCE               GPIO_PinSource6
#define sFLASH_SPI_MISO_AF                   GPIO_AF_SPI1

#define sFLASH_SPI_MOSI_PIN                  GPIO_Pin_7
#define sFLASH_SPI_MOSI_GPIO_PORT            GPIOA
#define sFLASH_SPI_MOSI_GPIO_CLK             RCC_AHBPeriph_GPIOA
#define sFLASH_SPI_MOSI_SOURCE               GPIO_PinSource7
#define sFLASH_SPI_MOSI_AF                   GPIO_AF_SPI1

#define sFLASH_CS_PIN                        GPIO_Pin_4
#define sFLASH_CS_GPIO_PORT                  GPIOA
#define sFLASH_CS_GPIO_CLK                   RCC_AHBPeriph_GPIOA
#define sFLASH_CS_GPIO_SOURCE                GPIO_PinSource4
#define sFLASH_CS_GPIO_AF                    GPIO_AF_SPI1   //spi1 nss -

/* Exported macro ------------------------------------------------------------*/
/* Select sFLASH: Chip Select pin low */
#define sFLASH_CS_LOW()                     GPIO_ResetBits(sFLASH_CS_GPIO_PORT, sFLASH_CS_PIN)
/* Deselect sFLASH: Chip Select pin high */
#define sFLASH_CS_HIGH()                    GPIO_SetBits(sFLASH_CS_GPIO_PORT, sFLASH_CS_PIN)

#define sFLASH_BLOCK_MAX                      256   //64KB per 1Block.
#define sFLASH_SECTOR_MAX                     16    //4KB per 1sector.

#define Z_DUMMY_DATA                          0x00
#define N_DUMMY_DATA                          0x0A

#define SPI_TIMEOUT                           0x1000  //spi timeout


/* Exported functions ------------------------------------------------------- */

/* High layer functions  */
void sFLASH_DeInit(void);
void sFLASH_Init(void);
void sFLASH_EraseSector(unsigned int SectorAddr);
void sFLASH_EraseBulk(void);
void sFLASH_WritePage(unsigned char* pBuffer, unsigned int WriteAddr, unsigned short NumByteToWrite);
void sFLASH_WriteBuffer(unsigned char* pBuffer, unsigned int WriteAddr, unsigned short NumByteToWrite);
void sFLASH_ReadBuffer(unsigned char* pBuffer, unsigned int ReadAddr, unsigned short NumByteToRead);
unsigned int sFLASH_ReadID(void);

void sFLASH_StartReadSequence(unsigned int ReadAddr);

void SPI_FLASH_Block64K_Erase(unsigned int BlockAddr);  //mjhan

/* Low layer functions */
unsigned char sFLASH_ReadByte(void);
unsigned char sFLASH_SendByte(unsigned char byte);
unsigned short sFLASH_SendHalfWord(unsigned short HalfWord);
void sFLASH_WriteEnable(void);
void sFLASH_WaitForWriteEnd(void);


/* proto type */
void Init_SPI_Flash_PORT(void);

void __cmd_spi_tx(unsigned int value, int len);





#ifdef __cplusplus
}
#endif

#endif /* __SPI_FLASH_H */


/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
