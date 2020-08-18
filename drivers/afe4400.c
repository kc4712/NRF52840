
#include <stdint.h>
#include <string.h>
#include "nordic_common.h"
#include "nrf.h"
//#include "nrf51_bitfields.h"
#include "ble_hci.h"
#include "ble_advdata.h"
#include "ble_conn_params.h"
#include "softdevice_handler.h"
#include "app_timer.h"
#include "app_button.h"
#include "ble_nus.h"
//#include "simple_uart.h"
#include "boards.h"
//#include "ble_error_log.h"
//#include "ble_debug_assert_handler.h"
#include "app_util_platform.h"
#include "heart_beat.h"



#include "afe4400.h"

#include "spi_master_previous.h"
#include "nrf_delay.h"
#include "nrf_gpio.h"
#include "common.h"

#include  "spi.h"
#include "heart_beat.h"
#include "nrf_drv_spi.h"
#include "gc_drv_spi.h"

static unsigned int *m_spi_base_address=0;
static unsigned int m_read_mode=0;


static unsigned int m_hb_led_current = HB_LED_CURRENT;
static unsigned short m_hb_led_on_count = HB_LED_ON_COUNT;
static unsigned short	m_hb_led_current_active = true;

#ifdef GC_NRF52_SPI_HR
#define GC_SPI_HR_CS_PIN   6 /**< SPI CS Pin.*/
#if 0  /* hkim 2016.7.7 */
#define GC_SPI_HR_INSTANCE  2 /**< SPI instance index. */ 
static const nrf_drv_spi_t spi_hr = NRF_DRV_SPI_INSTANCE(GC_SPI_HR_INSTANCE);  /**< SPI instance. */
#endif
#endif /* GC_NRF52_SPI_HR */
	
unsigned int* get_AFE4400_SPI_Address(void)
{
	return m_spi_base_address;
}

short get_HB_LED_current_status(void)
{

	return m_hb_led_current_active;
}


void set_HB_Led_Current_Reg(unsigned int led_current)
{

	m_hb_led_current = led_current;
}


void set_HB_led_on_count(unsigned short count)
{
	m_hb_led_on_count = count;
}



void AFE4400_Read_mode(int enable)
{
	unsigned char tx_data[4],rx_data[4];

	memset(tx_data,0,sizeof(tx_data));
	
	if(enable)
	{
		tx_data[3]=1;
		m_read_mode=true;
	}
	else
		m_read_mode=false;

	//printf("\n\r[AFE4400_Read_mode]  tx = %02x %02x %02x %02x", tx_data[0],tx_data[1],tx_data[2],tx_data[3]);

#ifdef GC_NRF52_SPI_HR
	if (nrf_drv_spi_transfer(&spi_hr, tx_data, sizeof(tx_data), rx_data, sizeof(rx_data)) != NRF_SUCCESS)
#elif defined GC_NRF51_SPI_HR	
	if(!spi_master_tx_rx(m_spi_base_address, 4, tx_data, rx_data))
#else 
#endif /*  GC_NRF52_SPI_HR */
	{
		printf("\n\r[AFE4400_Read_mode] spi rx/tx fail.");
	}
	
}

unsigned int  AFE4400_Read_Data(unsigned char address)
{
	unsigned char tx_data[4],rx_data[4];
	unsigned int data;

	memset(tx_data,0,sizeof(tx_data));
	memset(rx_data,0,sizeof(rx_data));

	tx_data[0]=address;
	
	if(m_read_mode!=true)
		AFE4400_Read_mode(true);

	//printf("\n\r[AFE4400_Read_Data]  tx = %02x %02x %02x %02x", tx_data[0],tx_data[1],tx_data[2],tx_data[3]);
#ifdef GC_NRF52_SPI_HR
	if (nrf_drv_spi_transfer(&spi_hr, tx_data, sizeof(tx_data), rx_data, sizeof(rx_data)) != NRF_SUCCESS)
#elif defined GC_NRF51_SPI_HR	
	if(!spi_master_tx_rx(m_spi_base_address, 4, tx_data, rx_data))
#else
#endif /* GC_NRF52_SPI_HR */
	{
		printf("\n\r[AFE4400_Read_Data] spi rx/tx fail.");
		return 0;
	}

	data = (rx_data[1]<<16) + (rx_data[2]<<8) + rx_data[3];
	
	//printf("\n\r[AFE4400_Read_Data]  rx = %02x %02x %02x %02x data= %02x", rx_data[0],rx_data[1],rx_data[2],rx_data[3],data);

	return data;
}

void  AFE4400_Write_Data(unsigned char address, unsigned int data)
{
	unsigned char tx_data[4],rx_data[4];

	tx_data[0]=address;
	tx_data[1]=(data&0x00ff0000)>>16;
	tx_data[2]=(data&0x00ff00)>>8;
	tx_data[3]=(data&0x00ff);

	
	//printf("\n\r[AFE4400_Write_Data]  tx = %02x %02x %02x %02x", tx_data[0],tx_data[1],tx_data[2],tx_data[3]);
	
	if(m_read_mode==true)
		AFE4400_Read_mode(false);
#ifdef GC_NRF52_SPI_HR
	if (nrf_drv_spi_transfer(&spi_hr, tx_data, sizeof(tx_data), rx_data, sizeof(rx_data)) != NRF_SUCCESS)
#elif defined GC_NRF51_SPI_HR	
	if(!spi_master_tx_rx(m_spi_base_address, 4, tx_data, rx_data))
#else
#endif /* GC_NRF52_SPI_HR */
	{
		printf("\n\r[AFE4400_Write_Data] spi rx/tx fail.");
	}

}

#ifdef GC_NRF52_SPI_HR
void Init_SPI(void)
{
	nrf_drv_spi_config_t spi_hr_config = NRF_DRV_SPI_DEFAULT_CONFIG(GC_SPI_HR_INSTANCE);
  spi_hr_config.ss_pin = GC_SPI_HR_CS_PIN;
	
	printf("\r\n** SPI HR config result");
	printf("\r\n sck_pin %d, mosi_pin %d, miso_pin %d, ss_pin %d, irq_priority %d, orc %d, freq 0x%8x, mode %d, bit-order %d",
			spi_hr_config.sck_pin, spi_hr_config.mosi_pin, spi_hr_config.miso_pin, spi_hr_config.ss_pin, spi_hr_config.irq_priority,
			spi_hr_config.orc, spi_hr_config.frequency, spi_hr_config.mode, spi_hr_config.bit_order );
	
  APP_ERROR_CHECK(nrf_drv_spi_init(&spi_hr, &spi_hr_config, NULL));
}
#elif defined GC_NRF51_SPI_HR
void init_SPI(void)
{
	m_spi_base_address = spi_master_init((SPIModuleNumber)SPI0, SPI_MODE0,  false );
	if (m_spi_base_address == 0)
	{
		printf("\n\r spi init fail.");
	}
}
#else
#endif /* GC_NRF52_SPI_HR */



void Init_AFE4400_PORT(void)
{
	//nrf_gpio_cfg_output(SENSOR_POWER);
	nrf_gpio_cfg_output(AFE_RESET_PIN);
	//nrf_gpio_pin_clear(SENSOR_POWER);
	nrf_gpio_pin_clear(AFE_RESET_PIN);

	Init_SPI();

	nrf_gpio_cfg_input(ADC_READY, NRF_GPIO_PIN_NOPULL);

	nrf_gpio_cfg_input(7, NRF_GPIO_PIN_NOPULL);

	nrf_gpio_cfg_input(8, NRF_GPIO_PIN_NOPULL);

	
	NRF_GPIOTE->CONFIG[0] =  (GPIOTE_CONFIG_POLARITY_LoToHi << GPIOTE_CONFIG_POLARITY_Pos)
	                       | (ADC_READY << GPIOTE_CONFIG_PSEL_Pos)  
	                       | (GPIOTE_CONFIG_MODE_Event << GPIOTE_CONFIG_MODE_Pos);
	NRF_GPIOTE->INTENSET  = GPIOTE_INTENSET_IN0_Set << GPIOTE_INTENSET_IN0_Pos;



	// Enable interrupt:
	NRF_GPIOTE->CONFIG[1] =  (GPIOTE_CONFIG_POLARITY_LoToHi << GPIOTE_CONFIG_POLARITY_Pos)
	                       | (7 << GPIOTE_CONFIG_PSEL_Pos)  
	                       | (GPIOTE_CONFIG_MODE_Event << GPIOTE_CONFIG_MODE_Pos);
	NRF_GPIOTE->INTENSET  = GPIOTE_INTENSET_IN1_Set << GPIOTE_INTENSET_IN1_Pos;


	NRF_GPIOTE->CONFIG[2] =  (GPIOTE_CONFIG_POLARITY_LoToHi << GPIOTE_CONFIG_POLARITY_Pos)
	                       | (8 << GPIOTE_CONFIG_PSEL_Pos)  
	                       | (GPIOTE_CONFIG_MODE_Event << GPIOTE_CONFIG_MODE_Pos);
	NRF_GPIOTE->INTENSET  = GPIOTE_INTENSET_IN2_Set << GPIOTE_INTENSET_IN2_Pos;

	NVIC_SetPriority(GPIOTE_IRQn, APP_IRQ_PRIORITY_LOW);
	NVIC_EnableIRQ(GPIOTE_IRQn);
}


void AFE4400_Power(int onoff)
{
	if(onoff)
	{
		nrf_gpio_pin_clear(AFE_RESET_PIN);
	 	//nrf_gpio_pin_set(SENSOR_POWER);
		nrf_delay_ms(1);
	 	nrf_gpio_pin_set(AFE_RESET_PIN);

		printf("\n\rAFE4400_Power ON\n\r");


		//AFE4400_Write_Data(1, 0x17c0);
		AFE4400_Write_Data(1, AFE4400_REG1);
		
		//AFE4400_Write_Data(2, 0x1f3e);
		AFE4400_Write_Data(2, AFE4400_REG2);

		//AFE4400_Write_Data(3, 0x1770);
		AFE4400_Write_Data(3, AFE4400_REG3);
		
		//AFE4400_Write_Data(4, 0x1f3f);
		AFE4400_Write_Data(4, AFE4400_REG4);
		

		AFE4400_Write_Data(5, 0x50);
		
		AFE4400_Write_Data(6, 0x7ce);

		
		//AFE4400_Write_Data(7, 0x820);
		AFE4400_Write_Data(7, 0x2760);
		
		//AFE4400_Write_Data(8, 0x0f9e);
		AFE4400_Write_Data(8, 0x2ede);
		
		//AFE4400_Write_Data(0x0b, 0xff0);
		AFE4400_Write_Data(0x0b, 0x4e70);
		//AFE4400_Write_Data(0x0c, 0x176e);
		AFE4400_Write_Data(0x0c, 0x55ee);

		
		//AFE4400_Write_Data(9,  0x07d0);
		AFE4400_Write_Data(9,  0x2710);
		
		//AFE4400_Write_Data(10, 0x0f9f);
		AFE4400_Write_Data(10, 0x2edf);
		
		AFE4400_Write_Data(0x15, 0);
		AFE4400_Write_Data(0x16, 0x05);
		
		AFE4400_Write_Data(0x0d, 0x06);
		//AFE4400_Write_Data(0x0e, 0x07cf);
		AFE4400_Write_Data(0x0e, 0x270f);
		
		//AFE4400_Write_Data(0x17, 0x07d0);
		AFE4400_Write_Data(0x17, 0x2710);
		//AFE4400_Write_Data(0x18, 0x07d5);
		AFE4400_Write_Data(0x18, 0x2715);
		
		//AFE4400_Write_Data(0x0f, 0x07d6);
		AFE4400_Write_Data(0x0f, 0x2716);
		//AFE4400_Write_Data(0x10, 0x0f9f );
		AFE4400_Write_Data(0x10, 0x4e1f );
		
		//AFE4400_Write_Data(0x19, 0x0fa0);
		AFE4400_Write_Data(0x19, 0x4e20);
		
		//AFE4400_Write_Data(0x1a, 0x0fa5);
		AFE4400_Write_Data(0x1a, 0x4e25);
		
		
		//AFE4400_Write_Data(0x11, 0x0fa6);
		AFE4400_Write_Data(0x11, 0x4e26);
		//AFE4400_Write_Data(0x12, 0x176f);
		AFE4400_Write_Data(0x12, 0x752f);
		
		//AFE4400_Write_Data(0x1b, 0x1770);
		AFE4400_Write_Data(0x1b, 0x7530);
		//AFE4400_Write_Data(0x1c, 0x1775);
		AFE4400_Write_Data(0x1c, 0x7535);
		
		//AFE4400_Write_Data(0x13, 0x1776);
		AFE4400_Write_Data(0x13, 0x7536);
		//AFE4400_Write_Data(0x14, 0x1f3f);
		AFE4400_Write_Data(0x14, 0x9c3f);

		//AFE4400_Write_Data(0x1d, 0x1f3f);
		AFE4400_Write_Data(0x1d, 0x9c3f);

		//AFE4400_Write_Data(0x21, 0x03);
		AFE4400_Write_Data(0x21, 0x7c);
		
		AFE4400_Write_Data(0x22, m_hb_led_current);
		//AFE4400_Write_Data(0x22, 0x012222);	
		//AFE4400_Write_Data(0x22, 0x011414);	// IR 5mA RED 5mA
		//AFE4400_Write_Data(0x22, 0x012503);	// IR 10mA RED 1mA
		//AFE4400_Write_Data(0x22, 0x013303);	// IR 15mA RED 1mA
		//AFE4400_Write_Data(0x22, 0x011700);	// IR 5mA RED 5mA

		AFE4400_Write_Data(0x1e, 0x102);

		AFE4400_Write_Data(0x23, 0);
		

		printf("\n\rAFE4400 default data loaded\n\r");
		
	}
	else
	{
	 	nrf_gpio_pin_clear(AFE_RESET_PIN);
	 	//nrf_gpio_pin_clear(SENSOR_POWER);
	
		printf("\n\rAFE4400_Power OFF\n\r");
	}
}


int g_offCount=0;

unsigned char AFE4400_sleep_control(unsigned char onflag)
{
	
	static unsigned char afe_sleep=false;

	if(!onflag)
	{	
		g_offCount++;

#if __GREENCOM_FITNESS__
                     if(g_offCount>30*60*20) /* 30 min */
#else
                     if(g_offCount>60*20)       /* 1 min */
#endif
		{
		
			AFE4400_Write_Data(0x23, 1);
			init_post_cal();
			g_sort_hb = 0;
			afe_sleep=true;	
		}
	}
	else
	{
		g_offCount=0;
		if(afe_sleep==true)
		{
			AFE4400_Write_Data(0x23, 0);
			afe_sleep=false;
		}
	}

	return afe_sleep;
}

extern char g_hb_external_light;

void	AFE4400_current_control(unsigned int count)
{
	unsigned int div_count;

	div_count = count%1200;
	
	if(div_count==m_hb_led_on_count)
	{
		//ks_printf("\r\nled on: external=%d", g_hb_external_light);
		if(g_hb_external_light==false)
		{
			AFE4400_Write_Data(0x22, m_hb_led_current);
			m_hb_led_current_active = true;
		}
	}
	else if(div_count==HB_LED_OFF_COUNT )
	{
		//ks_printf("\r\nled off");
		AFE4400_Write_Data(0x22, 0);
		m_hb_led_current_active = false;
	}

}	




