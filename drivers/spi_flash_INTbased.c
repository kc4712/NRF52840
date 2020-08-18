/**
  ******************************************************************************
  * @file    SPI/SPI_FLASH/spi_flash.c
  * @author  MCD Application Team
  * @version V1.1.0
  * @date    13-April-2012
  * @brief   This file provides a set of functions needed to manage the SPI M25Pxxx
  *          FLASH memory.
  *
  *          ===================================================================
  *          Notes:
  *           - There is no SPI FLASH memory available in STM322xG-EVAL board,
  *             to use this driver you have to build your own hardware.
  *          ===================================================================
  *
  *          It implements a high level communication layer for read and write
  *          from/to this memory. The needed STM32 hardware resources (SPI and
  *          GPIO) are defined in spi_flash.h file, and the initialization is
  *          performed in sFLASH_LowLevel_Init() function.
  *
  *          You can easily tailor this driver to any development board, by just
  *          adapting the defines for hardware resources and sFLASH_LowLevel_Init()
  *          function.
  *
  *          +-----------------------------------------------------------+
  *          |                     Pin assignment                        |
  *          +-----------------------------+---------------+-------------+
  *          |  STM32 SPI Pins             |     sFLASH    |    Pin      |
  *          +-----------------------------+---------------+-------------+
  *          | sFLASH_CS_PIN               | ChipSelect(/S)|    1        |
  *          | sFLASH_SPI_MISO_PIN / MISO  |   DataOut(Q)  |    2        |
  *          |                             |   VCC         |    3 (3.3 V)|
  *          |                             |   GND         |    4 (0 V)  |
  *          | sFLASH_SPI_MOSI_PIN / MOSI  |   DataIn(D)   |    5        |
  *          | sFLASH_SPI_SCK_PIN / SCK    |   Clock(C)    |    6        |
  *          |                             |    VCC        |    7 (3.3 V)|
  *          |                             |    VCC        |    8 (3.3 V)|
  *          +-----------------------------+---------------+-------------+
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

/* 					Description : W25Q128FVSG  : 16Mbyte.  														*/
/*					Block : 0 to 255, 256 Max Block Number.														*/
/*					1 Block Size: 64Kbytes.
						1 Block : 0 to 15 Sectors , 16 Max Sectors.
						1 Sectors Size : size 4Kbytes

*/

/* Includes ------------------------------------------------------------------*/
#include <stdio.h>
#include <string.h>
#include "spi_flash.h"
#include "spi_master_previous.h"
#include  "spi.h"
#ifdef GC_NRF52_SPI_FLASH
	#include "nrf52.h" /* hkim 2016.7.7 */
	#include "nrf_error.h"
	#include "nrf_spim.h" //GC_NRF52_PAHR
	#include "nrf_drv_spi.h"
	#include "nrf_drv_config.h"
	#include "app_util_platform.h"
	#include "nrf_drv_ppi.h"
	#include "nrf_ppi.h"
	#include "nrf_drv_gpiote.h"
	#include "flash_control.h"
#endif /* GC_NRF52_SPI_FLASH */

#ifdef GC_NRF52_SPI_FLASH 

//extern const nrf_drv_spi_t spi;
#ifdef GC_COACH_FIT_PCB
  #define GC_SPI_FLASH_CS_PIN   16 /**< SPI CS Pin.*/
#elif defined GC_COACH_FIT_DK52
  #define GC_SPI_FLASH_CS_PIN   27 /**< SPI CS Pin.*/
#else
#endif

/* GC_NRF52_PAHR */
volatile uint8_t gc_flash_status;
static uint32_t gc_flash_sectoraddr;
static uint8_t gc_flash_tx[255];


void gc_spi_flash_sector_erase(void);
void gc_spi_flash_write(void);
/* end of GC_NRF52_PAHR */

static const nrf_drv_spi_t spi_flash = NRF_DRV_SPI_INSTANCE(GC_SPI_FLASH_INSTANCE);  /**< SPI instance. */

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
#if 0
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
#endif		

void spi_master_event_handler(nrf_drv_spi_evt_t const * p_event)
{
	printf("\r\n flash isr : SPI evt %d", p_event->type);
  switch (p_event->type)
  {
		//if event is equal to NRF_DRV_SPI_EVENT_DONE, then this block is executed to check if data is valid
 
    case NRF_DRV_SPI_EVENT_DONE:   

			//disable the ppi channel
			nrf_drv_ppi_channel_disable(ppi_channel_workaround);

			//disable a GPIOTE output pin task.
			nrf_drv_gpiote_out_task_disable(SPI2_CONFIG_SCK_PIN);
			nrf_drv_gpiote_in_uninit(SPI2_CONFIG_SCK_PIN); //added by hkim 2016.6.21 
		
			if (gc_flash_status == GC_FLASH_READ)
			{
				printf("\r\n flash isr : read done");
				gc_flash_status = 0;
			}
			else if (gc_flash_status == GC_FLASH_WRITE_ENABLE_TO_ERASE)
			{
				printf("\r\n flash isr : write enable to erase done");
				gc_flash_status = 0;
				gc_spi_flash_sector_erase();
			}
			else if (gc_flash_status == GC_FLASH_WRITE_ENABLE_TO_WRITE)
			{
				printf("\r\n flash isr : write enable to write done");
				gc_flash_status = 0;
				gc_spi_flash_write();
			}
			else {}

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
	/* GPIOTE output pin을 toggle 한다 */
	nrf_drv_gpiote_out_toggle(SPI2_CONFIG_SCK_PIN);
}
#if 1

static void spi_send_recv(uint8_t * const p_tx_data,
                          uint8_t * const p_rx_data,
                          const uint16_t  len)
{
	uint32_t err_code=0;
	NRF_SPIM_Type * p_spim = spi_flash.p_registers;
	
	if (len == 1){
		ret_code_t err_code;
 
    // initializes the GPIOTE channel so that SCK toggles generates events
		//GPIOTE의 Task 와 이벤트를 할당. 
    nrf_drv_gpiote_in_config_t config = GPIOTE_CONFIG_IN_SENSE_TOGGLE(true);
		/* GPIOTE input pin 초기화. 두번재 파라미터의 config가 발생하면 in_pin_hander가 호출됨 */
    err_code = nrf_drv_gpiote_in_init(SPI2_CONFIG_SCK_PIN, &config, in_pin_handler);
		if (err_code != NRF_SUCCESS)
			printf("\r\nGPIOTE in-init error(code %d)", err_code);
    
		//allocates the first unused PPI Channel
		err_code = nrf_drv_ppi_channel_alloc(&ppi_channel_workaround);
		if (err_code != NRF_SUCCESS)
			printf("\r\nPPI-ch alloc error(code %d)", err_code);
				
		err_code = nrf_drv_ppi_channel_assign(ppi_channel_workaround, 
																					nrf_drv_gpiote_in_event_addr_get(SPI2_CONFIG_SCK_PIN), 
																					(uint32_t)&p_spim->TASKS_STOP);
		if (err_code != NRF_SUCCESS)
			printf("\r\nPPI-ch assign error(code %d)", err_code);
    
		//enable the PPI channel.
		err_code = nrf_drv_ppi_channel_enable(ppi_channel_workaround);
		if (err_code != NRF_SUCCESS)
			printf("\r\nPPI-ch enable error(code %d)", err_code);

		//enable the GPIOTE output pin task.
		nrf_drv_gpiote_in_event_enable(SPI2_CONFIG_SCK_PIN, false);

		//Start transfer of data
		spi_xfer_done = false;
		err_code = nrf_drv_spi_transfer(&spi_flash, p_tx_data, len, p_rx_data, len);
		printf("\r\n SPI result(1 Byte) : %d", err_code);
		if (err_code != NRF_SUCCESS)
			printf("\r\nSPI transfer error(code %d)", err_code);
#if 0 
		err_code = nrf_drv_ppi_channel_disable(ppi_channel_workaround);
		if (err_code != NRF_SUCCESS)
			printf("\r\nPPI-ch disable error(code %d)", err_code);
		
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
		err_code = nrf_drv_spi_transfer(&spi_flash, p_tx_data, len, p_rx_data, (uint8_t)len);
		printf("\r\n SPI result : %d", err_code);
		if (err_code != NRF_SUCCESS)
			printf("\r\nSPI transfer error(code %d)", err_code);
  }
 
	//nrf_delay_ms(delay);
}
#else

#endif 

void gc_spi_flash_init(void)
{
	int32_t err_code=0;
	
	nrf_drv_spi_config_t spi_flash_config = NRF_DRV_SPI_DEFAULT_CONFIG(GC_SPI_FLASH_INSTANCE);
	#if 1 /* GC_NRF52_PAHR */
  spi_flash_config.ss_pin = GC_SPI_FLASH_CS_PIN;
	spi_flash_config.frequency    = NRF_DRV_SPI_FREQ_4M;
	spi_flash_config.orc    = 0x00;
	#endif 
	#if 0 
	printf("\r\n** SPI Flash config result");
	printf("\r\n sck_pin %d, mosi_pin %d, miso_pin %d, ss_pin %d, irq_priority %d, orc %d, freq 0x%8x, mode %d, bit-order %d",
			spi_flash_config.sck_pin, spi_flash_config.mosi_pin, spi_flash_config.miso_pin, spi_flash_config.ss_pin, spi_flash_config.irq_priority,
			spi_flash_config.orc, spi_flash_config.frequency, spi_flash_config.mode, spi_flash_config.bit_order );
	#endif 
	err_code = nrf_drv_spi_init(&spi_flash, &spi_flash_config, spi_master_event_handler);
  if (err_code == NRF_SUCCESS)
		printf("\r\n Flash SPI initialization. done");
	else
		printf("\r\n Flash SPI initialization. fail");
	
	/* 
			NRF52 can't transmit one byte through SPIM. 
			Workaround function is setup_workaround_for_ftpan_58()
			Following link privide you for more information.
			http://infocenter.nordicsemi.com/pdf/nRF52832_Errata_v1.1.pdf
	*/
}
#elif defined GC_NRF51_SPI_FLASH
static unsigned int *m_spi1_base_address=0;

void Init_SPI_1(void)
{
	m_spi1_base_address = spi_master_init((SPIModuleNumber)SPI1, SPI_MODE0,  false );
	if (m_spi1_base_address == 0)
	{
		printf("\n\r spi1 init fail.");
	}
}

void Init_SPI_Flash_PORT(void)
{

	Init_SPI_1();

}
#else /* GC_NRF52_SPI_FLASH */
#endif /* GC_NRF52_SPI_FLASH */

/** @addtogroup STM32F2xx_StdPeriph_Examples
  * @{
  */

/** @addtogroup SPI_FLASH
  * @{
  */
extern void tmp_delay(int delay);

extern int sys_spi_get(unsigned char *pc);

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/


void sFLASH_TxByte(unsigned char byte);

/* Private functions ---------------------------------------------------------*/

void sFLASH_delay(int delay)
{
/*
  sFLASH_TxByte(0x00);

	tmp_delay(delay);
*/
	return;
}
/**
  * @brief  DeInitializes the peripherals used by the SPI FLASH driver.
  * @param  None
  * @retval None
  */




/*-------------------------------------------------------------------------*/
/* bsp_spi_flash.c reference.. */

/**************************************************************************
* Function Name  : SPI_FLASH_SectorErase
* Description    : Erases the specified FLASH sector.
* Input          : SectorAddr: address of the sector to erase.
* Output         : None
* Return         : None
*******************************************************************************/
void gc_spi_flash_sector_erase(void)  //GC_NRF52_PAHR
{
	unsigned char tx_data[4];
	unsigned char rx_data[4];

	// 4k erase
	tx_data[0]=W25X_SectorErase;
	tx_data[1]=(gc_flash_sectoraddr & 0xFF0000) >> 16;
	tx_data[2]=(gc_flash_sectoraddr& 0xFF00) >> 8;
	tx_data[3]=gc_flash_sectoraddr & 0xFF;

	spi_send_recv( tx_data,rx_data,4);
}
void SPI_FLASH_SectorErase(unsigned int SectorAddr)
{
//	uint8_t tx_data[4];
	//uint8_t rx_data[4];

	gc_flash_status = GC_FLASH_WRITE_ENABLE_TO_ERASE; //GC_NRF52_PAHR
	gc_flash_sectoraddr = SectorAddr; //GC_NRF52_PAHR
	SPI_FLASH_WriteEnable();
#if 0 	
	// 4k erase
	tx_data[0]=W25X_SectorErase;
	tx_data[1]=(SectorAddr & 0xFF0000) >> 16;
	tx_data[2]=(SectorAddr& 0xFF00) >> 8;
	tx_data[3]=SectorAddr & 0xFF;

#ifdef GC_NRF52_SPI_FLASH
	//if (nrf_drv_spi_transfer(&spi_flash, tx_data, sizeof(tx_data), rx_data, sizeof(rx_data)) != NRF_SUCCESS)
	spi_send_recv( tx_data,rx_data,4);
#elif defined GC_NRF51_SPI_FLASH
	if(!spi_master_tx_rx(m_spi1_base_address, 4, tx_data, rx_data))
	{
		printf("\n\r[SPI_FLASH_PageWrite] spi rx/tx fail.");
	}
#else /* GC_NRF52_SPI_FLASH */
#endif  /* GC_NRF52_SPI_FLASH */	
#endif 
}

#if 1
/**************************************************************************
* Function Name  : SPI_FLASH_BLOCK32K_Erase
* Description    : Erases the specified Block 64Ksector.
* Input          : SectorAddr: address of the sector to erase.
* Output         : None
* Return         : None
***************************************************************************/

void SPI_FLASH_Block64K_Erase(unsigned int BlockAddr)
{
#if 0
  /* Send write enable instruction */
  SPI_FLASH_WriteEnable();
  SPI_FLASH_WaitForWriteEnd();
  /* Sector Erase */
  /* Select the FLASH: Chip Select low */
  SPI_FLASH_CS_LOW();
  /* Send Sector Erase instruction */
  SPI_FLASH_SendByte(W25X_BlockErase);
  /* Send SectorAddr high nibble address byte */
  SPI_FLASH_SendByte((BlockAddr & 0xFF0000) >> 16);
  /* Send SectorAddr medium nibble address byte */
  SPI_FLASH_SendByte((BlockAddr & 0xFF00) >> 8);
  /* Send SectorAddr low nibble address byte */
  SPI_FLASH_SendByte(BlockAddr & 0xFF);
  /* Deselect the FLASH: Chip Select high */
  SPI_FLASH_CS_HIGH();
  /* Wait the end of Flash writing */
  SPI_FLASH_WaitForWriteEnd();
#endif
}

#endif

/*******************************************************************************
* Function Name  : SPI_FLASH_BulkErase
* Description    : Erases the entire FLASH.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void SPI_FLASH_BulkErase(void)
{
#if 0
  /* Send write enable instruction */
  SPI_FLASH_WriteEnable();

  /* Bulk Erase */
  /* Select the FLASH: Chip Select low */
  SPI_FLASH_CS_LOW();
  /* Send Bulk Erase instruction  */
  SPI_FLASH_SendByte(W25X_ChipErase);
  /* Deselect the FLASH: Chip Select high */
  SPI_FLASH_CS_HIGH();

  /* Wait the end of Flash writing */
  SPI_FLASH_WaitForWriteEnd();
  #endif
}

/*******************************************************************************
* Function Name  : SPI_FLASH_PageWrite
* Description    : Writes more than one byte to the FLASH with a single WRITE
*                  cycle(Page WRITE sequence). The number of byte can't exceed
*                  the FLASH page size.
* Input          : - pBuffer : pointer to the buffer  containing the data to be
*                    written to the FLASH.
*                  - WriteAddr : FLASH's internal address to write to.
*                  - NumByteToWrite : number of bytes to write to the FLASH,
*                    must be equal or less than "SPI_FLASH_PageSize" value.
* Output         : None
* Return         : None
*******************************************************************************/
void gc_spi_flash_write(void)  //GC_NRF52_PAHR
{
	//unsigned char tx_data[255];
	unsigned char rx_data[255];
	
	gc_flash_tx[0]=W25X_PageProgram;
	gc_flash_tx[1]=(gc_flash_sectoraddr & 0xFF0000) >> 16;
	gc_flash_tx[2]=(gc_flash_sectoraddr& 0xFF00) >> 8;
	gc_flash_tx[3]=gc_flash_sectoraddr & 0xFF;

	spi_send_recv(gc_flash_tx, rx_data, 255); 
}
void SPI_FLASH_PageWrite(unsigned char* pBuffer, unsigned int WriteAddr)
{
#ifdef GC_NRF52_SPI_FLASH
	//unsigned char rx_data[255];
#elif defined GC_NRF51_SPI_FLASH
	//unsigned char rx_data[260];
#else 
#endif 

	/* Enable the write access to the FLASH */
	gc_flash_status = GC_FLASH_WRITE_ENABLE_TO_WRITE; //GC_NRF52_PAHR
	gc_flash_sectoraddr = WriteAddr; //GC_NRF52_PAHR
	memcpy(gc_flash_tx, pBuffer,255);
	SPI_FLASH_WriteEnable();
#if 0 //GC_NRF52_PAHR
	pBuffer[0]=W25X_PageProgram;
	pBuffer[1]=(WriteAddr & 0xFF0000) >> 16;
	pBuffer[2]=(WriteAddr& 0xFF00) >> 8;
	pBuffer[3]=WriteAddr & 0xFF;

#ifdef GC_NRF52_SPI_FLASH
	//if (nrf_drv_spi_transfer(&spi_flash, pBuffer, 255, rx_data, sizeof(rx_data)) != NRF_SUCCESS)
	spi_send_recv(pBuffer, rx_data, 255); 
#elif defined GC_NRF51_SPI_FLASH
	if(!spi_master_tx_rx(m_spi1_base_address, 260, pBuffer, rx_data))
	{
		printf("\n\r[SPI_FLASH_PageWrite] spi rx/tx fail.");
	}
#else /* GC_NRF52_SPI_FLASH */
#endif  /* GC_NRF52_SPI_FLASH */
	//ks_printf("\n\r[spi_flash]  rx = %06x", data);

	//DumpMemory(pBuffer,260);
#endif /* if 0 */
}

/*******************************************************************************
* Function Name  : SPI_FLASH_BufferWrite
* Description    : Writes block of data to the FLASH. In this function, the
*                  number of WRITE cycles are reduced, using Page WRITE sequence.
* Input          : - pBuffer : pointer to the buffer  containing the data to be
*                    written to the FLASH.
*                  - WriteAddr : FLASH's internal address to write to.
*                  - NumByteToWrite : number of bytes to write to the FLASH.
* Output         : None
* Return         : None
*******************************************************************************/
void SPI_FLASH_BufferWrite(unsigned char* pBuffer, unsigned int WriteAddr, unsigned short NumByteToWrite)
{
#if 0
  unsigned char NumOfPage = 0, NumOfSingle = 0, Addr = 0, count = 0, temp = 0;

  Addr = WriteAddr % SPI_FLASH_PageSize;
  count = SPI_FLASH_PageSize - Addr;
  NumOfPage =  NumByteToWrite / SPI_FLASH_PageSize;
  NumOfSingle = NumByteToWrite % SPI_FLASH_PageSize;

  if (Addr == 0) /* WriteAddr is SPI_FLASH_PageSize aligned  */
  {
    if (NumOfPage == 0) /* NumByteToWrite < SPI_FLASH_PageSize */
    {
      SPI_FLASH_PageWrite(pBuffer, WriteAddr, NumByteToWrite);
    }
    else /* NumByteToWrite > SPI_FLASH_PageSize */
    {
      while (NumOfPage--)
      {
        SPI_FLASH_PageWrite(pBuffer, WriteAddr, SPI_FLASH_PageSize);
        WriteAddr +=  SPI_FLASH_PageSize;
        pBuffer += SPI_FLASH_PageSize;
      }

      SPI_FLASH_PageWrite(pBuffer, WriteAddr, NumOfSingle);
    }
  }
  else /* WriteAddr is not SPI_FLASH_PageSize aligned  */
  {
    if (NumOfPage == 0) /* NumByteToWrite < SPI_FLASH_PageSize */
    {
      if (NumOfSingle > count) /* (NumByteToWrite + WriteAddr) > SPI_FLASH_PageSize */
      {
        temp = NumOfSingle - count;

        SPI_FLASH_PageWrite(pBuffer, WriteAddr, count);
        WriteAddr +=  count;
        pBuffer += count;

        SPI_FLASH_PageWrite(pBuffer, WriteAddr, temp);
      }
      else
      {
        SPI_FLASH_PageWrite(pBuffer, WriteAddr, NumByteToWrite);
      }
    }
    else /* NumByteToWrite > SPI_FLASH_PageSize */
    {
      NumByteToWrite -= count;
      NumOfPage =  NumByteToWrite / SPI_FLASH_PageSize;
      NumOfSingle = NumByteToWrite % SPI_FLASH_PageSize;

      SPI_FLASH_PageWrite(pBuffer, WriteAddr, count);
      WriteAddr +=  count;
      pBuffer += count;

      while (NumOfPage--)
      {
        SPI_FLASH_PageWrite(pBuffer, WriteAddr, SPI_FLASH_PageSize);
        WriteAddr +=  SPI_FLASH_PageSize;
        pBuffer += SPI_FLASH_PageSize;
      }

      if (NumOfSingle != 0)
      {
        SPI_FLASH_PageWrite(pBuffer, WriteAddr, NumOfSingle);
      }
    }
  }
  #endif
}

/*******************************************************************************
* Function Name  : SPI_FLASH_BufferRead
* Description    : Reads a block of data from the FLASH.
* Input          : - pBuffer : pointer to the buffer that receives the data read
*                    from the FLASH.
*                  - ReadAddr : FLASH's internal address to read from.
*                  - NumByteToRead : number of bytes to read from the FLASH.
* Output         : None
* Return         : None
*******************************************************************************/
void SPI_FLASH_BufferRead(unsigned char* pBuffer, unsigned int ReadAddr)
{
#ifdef GC_NRF52_SPI_FLASH
	unsigned char tx_data[255];
#elif defined GC_NRF51_SPI_FLASH
	unsigned char tx_data[260];
#else 
#endif 
	memset(tx_data,0,sizeof(tx_data));

	tx_data[0]=W25X_ReadData;
	tx_data[1]=(ReadAddr & 0xFF0000) >> 16;
	tx_data[2]=(ReadAddr& 0xFF00) >> 8;
	tx_data[3]=ReadAddr & 0xFF;

#ifdef GC_NRF52_SPI_FLASH
	//if (nrf_drv_spi_transfer(&spi_flash, tx_data, sizeof(tx_data), pBuffer, 255) != NRF_SUCCESS)
	spi_send_recv(tx_data, pBuffer, 250);
#elif defined GC_NRF51_SPI_FLASH
	if(!spi_master_tx_rx(m_spi1_base_address, 260, tx_data, pBuffer))
	{
		printf("\n\r[SPI_FLASH_BufferRead] spi rx/tx fail.");
	}
#else /* GC_NRF52_SPI_FLASH */
#endif /* GC_NRF52_SPI_FLASH */
}

/*******************************************************************************
* Function Name  : SPI_FLASH_ReadID
* Description    : Reads FLASH identification.
* Input          : None
* Output         : None
* Return         : FLASH identification
*******************************************************************************/
unsigned int SPI_FLASH_ReadID(void)
{
	unsigned char tx_data[4],rx_data[4];
	unsigned int data;

	tx_data[0]=W25X_JedecDeviceID;
	tx_data[1]=0;
	tx_data[2]=0;
	tx_data[3]=0;

	
	//ks_printf("\n\r[spi_flash]  tx = %02x %02x %02x %02x", tx_data[0],tx_data[1],tx_data[2],tx_data[3]);
#ifdef GC_NRF52_SPI_FLASH
	//if (nrf_drv_spi_transfer(&spi_flash, tx_data, sizeof(tx_data), rx_data, sizeof(rx_data)) != NRF_SUCCESS)
	spi_send_recv(tx_data, rx_data, 4);
#elif defined GC_NRF51_SPI_FLASH
	if(!spi_master_tx_rx(m_spi1_base_address, 4, tx_data, rx_data))
	{
		printf("\n\r[SPI_FLASH] spi rx/tx fail.");
	}
#else /* GC_NRF52_SPI_FLASH */
#endif /* GC_NRF52_SPI_FLASH */
	data = (rx_data[1]<<16) + (rx_data[2]<<8) + rx_data[3];

	//ks_printf("\n\r[spi_flash]  rx = %06x", data);
	
  return data;
}
/*******************************************************************************
* Function Name  : SPI_FLASH_ReadID
* Description    : Reads FLASH identification.
* Input          : None
* Output         : None
* Return         : FLASH identification
*******************************************************************************/
unsigned int SPI_FLASH_ReadDeviceID(void)
{
  unsigned int Temp = 0;
#if 0
  /* Select the FLASH: Chip Select low */
  SPI_FLASH_CS_LOW();

  /* Send "RDID " instruction */
  SPI_FLASH_SendByte(W25X_DeviceID);
  SPI_FLASH_SendByte(Dummy_Byte);
  SPI_FLASH_SendByte(Dummy_Byte);
  SPI_FLASH_SendByte(Dummy_Byte);

  /* Read a byte from the FLASH */
  Temp = SPI_FLASH_SendByte(Dummy_Byte);

  /* Deselect the FLASH: Chip Select high */
  SPI_FLASH_CS_HIGH();
#endif
  return Temp;
}
/*******************************************************************************
* Function Name  : SPI_FLASH_StartReadSequence
* Description    : Initiates a read data byte (READ) sequence from the Flash.
*                  This is done by driving the /CS line low to select the device,
*                  then the READ instruction is transmitted followed by 3 bytes
*                  address. This function exit and keep the /CS line low, so the
*                  Flash still being selected. With this technique the whole
*                  content of the Flash is read with a single READ instruction.
* Input          : - ReadAddr : FLASH's internal address to read from.
* Output         : None
* Return         : None
*******************************************************************************/
void SPI_FLASH_StartReadSequence(unsigned int ReadAddr)
{
#if 0
  /* Select the FLASH: Chip Select low */
  SPI_FLASH_CS_LOW();

  /* Send "Read from Memory " instruction */
  SPI_FLASH_SendByte(W25X_ReadData);

  /* Send the 24-bit address of the address to read from -----------------------*/
  /* Send ReadAddr high nibble address byte */
  SPI_FLASH_SendByte((ReadAddr & 0xFF0000) >> 16);
  /* Send ReadAddr medium nibble address byte */
  SPI_FLASH_SendByte((ReadAddr& 0xFF00) >> 8);
  /* Send ReadAddr low nibble address byte */
  SPI_FLASH_SendByte(ReadAddr & 0xFF);
  #endif
}

/*******************************************************************************
* Function Name  : SPI_FLASH_ReadByte
* Description    : Reads a byte from the SPI Flash.
*                  This function must be used only if the Start_Read_Sequence
*                  function has been previously called.
* Input          : None
* Output         : None
* Return         : Byte Read from the SPI Flash.
*******************************************************************************/
unsigned char SPI_FLASH_ReadByte(void)
{
  return (SPI_FLASH_SendByte(Dummy_Byte));
}

/*******************************************************************************
* Function Name  : SPI_FLASH_SendByte
* Description    : Sends a byte through the SPI interface and return the byte
*                  received from the SPI bus.
* Input          : byte : byte to send.
* Output         : None
* Return         : The value of the received byte.
*******************************************************************************/
unsigned char SPI_FLASH_SendByte(unsigned char byte)
{
#if 0
  /* Loop while DR register in not emplty */
  while (SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_TXE) == RESET);

  /* Send byte through the SPI1 peripheral */
  SPI_I2S_SendData(SPI1, byte);

  /* Wait to receive a byte */
  while (SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_RXNE) == RESET);

#if 1  //mjhan.

#endif
  /* Return the byte read from the SPI bus */
  return SPI_I2S_ReceiveData(SPI1);
#endif
return 1;
}

/*******************************************************************************
* Function Name  : SPI_FLASH_SendHalfWord
* Description    : Sends a Half Word through the SPI interface and return the
*                  Half Word received from the SPI bus.
* Input          : Half Word : Half Word to send.
* Output         : None
* Return         : The value of the received Half Word.
*******************************************************************************/
unsigned short SPI_FLASH_SendHalfWord(unsigned short HalfWord)
{
#if 0
  /* Loop while DR register in not emplty */
  while (SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_TXE) == RESET);

  /* Send Half Word through the SPI1 peripheral */
  SPI_I2S_SendData(SPI1, HalfWord);

  /* Wait to receive a Half Word */
  while (SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_RXNE) == RESET);

  /* Return the Half Word read from the SPI bus */
  
  return SPI_I2S_ReceiveData(SPI1);
#endif

return 0;

}

/*******************************************************************************
* Function Name  : SPI_FLASH_WriteEnable
* Description    : Enables the write access to the FLASH.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void SPI_FLASH_WriteEnable(void)
{
	unsigned char tx_data[4],rx_data[4];

	tx_data[0]=W25X_WriteEnable;
	tx_data[1]=0;
	tx_data[2]=0;
	tx_data[3]=0;
	
	//ks_printf("\n\rSPI_FLASH_WriteEnable");
#ifdef GC_NRF52_SPI_FLASH
	//if (nrf_drv_spi_transfer(&spi_flash, &tx_data[0], 1, rx_data, sizeof(rx_data)) != NRF_SUCCESS)
	spi_send_recv(tx_data, rx_data, 1);
#elif defined GC_NRF51_SPI_FLASH
	if(!spi_master_tx_rx(m_spi1_base_address, 1, tx_data, rx_data))
	{
		printf("\n\r[SPI_FLASH_WriteEnable] spi rx/tx fail.");
	}
#else /* GC_NRF52_SPI_FLASH */
#endif  /* GC_NRF52_SPI_FLASH */
}

/*******************************************************************************
* Function Name  : SPI_FLASH_WaitForWriteEnd
* Description    : Polls the status of the Write In Progress (WIP) flag in the
*                  FLASH's status  register  and  loop  until write  opertaion
*                  has completed.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void SPI_FLASH_WaitForWriteEnd(void)
{
#if 0
  unsigned char FLASH_Status = 0;

  /* Select the FLASH: Chip Select low */
  SPI_FLASH_CS_LOW();

  /* Send "Read Status Register" instruction */
  SPI_FLASH_SendByte(W25X_ReadStatusReg);

  /* Loop as long as the memory is busy with a write cycle */
  do
  {
    /* Send a dummy byte to generate the clock needed by the FLASH
    and put the value of the status register in FLASH_Status variable */
    FLASH_Status = SPI_FLASH_SendByte(Dummy_Byte);
  }
  while ((FLASH_Status & WIP_Flag) == SET); /* Write in progress */

  /* Deselect the FLASH: Chip Select high */
  SPI_FLASH_CS_HIGH();
  #endif
}


//power down mode 입력.
void SPI_Flash_PowerDown(void)
{
#if 0
  /* Select the FLASH: Chip Select low */
  SPI_FLASH_CS_LOW();

  /* Send "Power Down" instruction */
  SPI_FLASH_SendByte(W25X_PowerDown);

  /* Deselect the FLASH: Chip Select high */
  SPI_FLASH_CS_HIGH();
  #endif
}

void SPI_Flash_WAKEUP(void)
{
#if 0
  /* Select the FLASH: Chip Select low */
  SPI_FLASH_CS_LOW();

  /* Send "Power Down" instruction */
  SPI_FLASH_SendByte(W25X_ReleasePowerDown);

  /* Deselect the FLASH: Chip Select high */
  SPI_FLASH_CS_HIGH();                   //等待TRES1
  #endif
}

/* GC_NRF52_PAHR */
void __cmd_spi_tx(uint32_t value, int len)
{
	unsigned char tx_data[4];
	unsigned char rx_data[4];
	NRF_SPIM_Type * p_spim = spi_flash.p_registers;

	tx_data[0]= value & 0x000000FF;
	tx_data[1]= (value & 0x0000FF00) >> 8;
	tx_data[2]= (value & 0x00FF0000) >> 16;
	tx_data[3]= (value & 0xFF000000) >>  24;
	
	printf("\r\n Maxcount Tx/Rx %d, %d", p_spim->TXD.MAXCNT, p_spim->RXD.MAXCNT);
	printf("\r\n Tx 0x%x 0x%x 0x%x 0x%x", tx_data[0], tx_data[1], tx_data[2], tx_data[3]);
	printf("\r\n Tx %u %u %u %u", &tx_data[0], &tx_data[1], &tx_data[2], &tx_data[3]);
	
	nrf_gpio_pin_clear(GC_SPI_FLASH_CS_PIN);
	spi_send_recv( tx_data,rx_data,len);
	nrf_gpio_pin_set(GC_SPI_FLASH_CS_PIN);

}
/* end of GC_NRF52_PAHR */




/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
