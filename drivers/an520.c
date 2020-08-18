/* Copyright (c) 2015 Nordic Semiconductor. All Rights Reserved.
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

/** @file
 * @defgroup tw_sensor_example main.c
 * @{
 * @ingroup nrf_twi_example
 * @brief TWI Sensor Example main file.
 *
 * This file contains the source code for a sample application using TWI.
 *
 */


#include "an520.h"
//////////////////////////////////////////////

//규창 메인에 선언된 가속센서 TWI 핸들러(I2C);
extern const nrf_drv_twi_t m_twi_master_1;
/* Indicates if reading operation from accelerometer has ended. */
static volatile bool m_xfer_done = true;
/* Indicates if setting mode operation has ended. */
static volatile bool m_set_mode_done = false;
/* TWI instance. */
static const nrf_drv_twi_t m_twi_mma_ms5637 = NRF_DRV_TWI_INSTANCE(0);

uint8_t i2c_rx_buffer[I2C_RX_BUF_SIZE] = {0};


//규창 공분산 연산용 전역변수 용도는 crc_init에서 공분산 값 채우고 실 연산에 이용
uint16_t C[8] = {0,}; // calibration coefficients


/**
 * @brief TWI events handler.
 */
void twi_handler(nrf_drv_twi_evt_t const * p_event, void * p_context)
{   
   // ret_code_t err_code;
   // static sample_t m_sample;
    
    switch(p_event->type)
    {
        case NRF_DRV_TWI_EVT_DONE:
       /*     if ((p_event->type == NRF_DRV_TWI_EVT_DONE) &&
                (p_event->xfer_desc.type == NRF_DRV_TWI_XFER_TX))
            {
                if(m_set_mode_done != true)
                {
                    m_set_mode_done  = true;
                    return;
                }
                m_xfer_done = false;
               // Read 4 bytes from the specified address. 
                err_code = nrf_drv_twi_rx(&m_twi_mma_ms5637, MS5637_ADDR, (uint8_t*)&i2c_rx_buffer, sizeof(m_sample));
                APP_ERROR_CHECK(err_code);
            }
            else
            {
                read_data(&m_sample);
                m_xfer_done = true;
            }*/
						m_xfer_done = true;
            break;
        default:
						m_xfer_done = true;
            break;        
    }   
}

/**
 * @brief UART initialization.
 */
void twi_init (void)
{
    ret_code_t err_code;
    
    const nrf_drv_twi_config_t twi_mma_ms5837_config = {
       .scl                = MS5637_I2C_SCL_PIN,
       .sda                = MS5637_I2C_SDA_PIN,
       .frequency          = NRF_TWI_FREQ_100K,
       .interrupt_priority = APP_IRQ_PRIORITY_LOW
    };
    
    err_code = nrf_drv_twi_init(&m_twi_mma_ms5637, &twi_mma_ms5837_config, twi_handler, NULL);
    APP_ERROR_CHECK(err_code);
    
    nrf_drv_twi_enable(&m_twi_mma_ms5637);
}

//********************************************************
//! @brief send command using I2C hardware interface
//!
//! @return none
//********************************************************
void i2c_send(uint8_t cmd)
{
		ret_code_t err_code;

		/*while(!m_xfer_done);
		m_xfer_done = false;
		err_code = nrf_drv_twi_tx(&m_twi_mma_ms5637, MS5637_ADDR, &cmd, 1, false);  
    APP_ERROR_CHECK(err_code);*/
	
	
	err_code = nrf_drv_twi_tx(&m_twi_master_1, AN520_I2C_ADDR, &cmd, 1, true);
	if (err_code != NRF_SUCCESS)
	{
	#ifndef _FIT_ETRI_TEST_
			//printf("\n\nAN520 TWI write error(error code %d)", err_code);
	#endif
	}
	
	
}

void i2c_read(uint8_t length)
{
		ret_code_t err_code;
		
		/*while(!m_xfer_done);
		m_xfer_done = false;
		err_code = nrf_drv_twi_rx(&m_twi_mma_ms5637, MS5637_ADDR, (uint8_t*)i2c_rx_buffer, length);
		APP_ERROR_CHECK(err_code);
		while(!m_xfer_done);*/
	
	err_code = nrf_drv_twi_rx(&m_twi_master_1, AN520_I2C_ADDR, (uint8_t*)i2c_rx_buffer, length);
	if (err_code != NRF_SUCCESS)
	{
	#ifndef _FIT_ETRI_TEST_
			//printf("\n\r AN520 TWI read(get adc) error(error code %d)", err_code);
	#endif
	}
}


void AN520cmd_reset(void)
{
	 i2c_send(CMD_RESET); // send reset sequence
		nrf_delay_ms(3);
}

//********************************************************
//! @brief preform adc conversion
//!
//! @return 24bit result
//********************************************************
uint32_t  cmd_adc(uint8_t cmd)
{
	uint32_t temp=0;
	
	i2c_send(CMD_ADC_CONV+cmd); // send conversion command
	switch (cmd & 0x0f) // wait necessary conversion time
	{
		case CMD_ADC_256 : nrf_delay_ms(1); break;
		case CMD_ADC_512 : nrf_delay_ms(3); break;
		case CMD_ADC_1024: nrf_delay_ms(4); break;
		case CMD_ADC_2048: nrf_delay_ms(6); break;
		case CMD_ADC_4096: nrf_delay_ms(10); break;
		case CMD_ADC_8192: nrf_delay_ms(20); break;
		default:
			nrf_delay_ms(10); break;
	}
	i2c_send(CMD_ADC_READ);
	
	 i2c_read(3);
   
	 temp = i2c_rx_buffer[0] * 65536;
	 temp = temp + (256*i2c_rx_buffer[1]);
	 temp = temp + i2c_rx_buffer[2];
	
	 return temp;
}

//********************************************************
//! @brief Read calibration coefficients
//!
//! @return coefficient
//********************************************************
uint16_t cmd_prom(uint8_t coef_num)
{
//	ret_code_t err_code;
	uint16_t temp=0;
	
	// ----------------------------------------------------------------
	// Prochild 추가 코드.
	// ----------------------------------------------------------------
	if(coef_num==0)
	{
		i2c_send(CMD_PROM_RD); // send PROM READ command
		
		i2c_read(2);
		temp = 256*i2c_rx_buffer[0];
		temp = temp + i2c_rx_buffer[1];
	}
	// ----------------------------------------------------------------
	
	i2c_send(CMD_PROM_RD+coef_num*2); // send PROM READ command
	
	i2c_read(2);
	temp=0;
	temp = 256*i2c_rx_buffer[0];
	temp = temp + i2c_rx_buffer[1];
	return temp;
}

//********************************************************
//! @brief calculate the CRC code
//!
//! @return crc code
//********************************************************
uint8_t AN520crc4(uint16_t n_prom[])
{
	uint16_t cnt; // simple counter
	uint16_t n_rem=0; // crc remainder
	uint8_t n_bit;
	
	n_prom[0]=((n_prom[0]) & 0x0FFF); // CRC byte is replaced by 0
	n_prom[7]=0;//(0x0FFF & (n_prom[7])); //0 // CRC byte is replaced by 00; // Subsidiary value, set to 0
	
	for (cnt = 0; cnt < 16; cnt++) // operation is performed on bytes
	{ // choose LSB or MSB
		if (cnt%2==1) n_rem ^= (uint16_t) ((n_prom[cnt>>1]) & 0x00FF);
		else n_rem ^= (uint16_t) (n_prom[cnt>>1]>>8);
		for (n_bit = 8; n_bit > 0; n_bit--)
		{
			if (n_rem & (0x8000)) n_rem = (n_rem << 1) ^ 0x3000;
			else n_rem = (n_rem << 1);
		}
	}
	n_rem= ((n_rem >> 12) & 0x000F); // final 4-bit remainder is CRC code
	return (n_rem ^ 0x00);
}

//규창 메인루틴에 있던 루틴 분할
void AN520_crc_init(void){
	uint8_t read_crc;
	uint8_t n_crc; // crc value of the prom
	
	for (int i=0;i<8;i++)
	{ C[i]=cmd_prom(i);
			__ASM { nop }
	} // read coefficients
	
		read_crc = (C[0] >> 12) & 0x000F;
		n_crc=AN520crc4(C); // calculate the CRC
		
		// ----------------------------------------------------------------
		// Prochild add
		// ----------------------------------------------------------------
		if(read_crc != n_crc)
		{
			//CRC fail
			printf("AN520 CRC init error [%d != %d]",read_crc, n_crc);
			//asm("nop");
			__ASM { nop }
		}
		else
		{
			//CRC OK
			//asm("nop");
			__ASM { nop }
		}
		// ----------------------------------------------------------------
}



//규창 메인루틴에 있던 루틴 분할
void AN520_Result(float *temp,  float *pressure, float *altitude){
	uint32_t D1; // ADC value of the pressure conversion
	uint32_t D2; // ADC value of the temperature conversion
	double dT; // difference between actual and measured temperature
	double T; // compensated temperature value
	
	double OFF; // offset at actual temperature
	double SENS; // sensitivity at actual temperature
	double P; // compensated pressure value
	
	//printf("\n\rTWI sensor example\r\n");
	
	D1=0;
	D2=0;
	D2=cmd_adc(CMD_ADC_D2+CMD_ADC_4096); // read D2
	D1=cmd_adc(CMD_ADC_D1+CMD_ADC_4096); // read D1
	// calcualte 1st order pressure and temperature (MS5607 1st order algorithm)
	dT=(D2)-C[5]*pow(2,8);
	OFF=C[2]*pow(2,17)+dT*C[4]/pow(2,6);
	SENS=C[1]*pow(2,16)+dT*C[3]/pow(2,7);
	T=(2000+(dT*C[6])/pow(2,23))/100;
	P=(((D1*SENS)/pow(2,21)-OFF)/pow(2,15))/100;

	//printf("\r\n SGC AN520 Result!!!! %2.5f, %4.3f \r\n", T, P);
	*altitude = (1-pow((P/1013.25),.190284))*145366.45*.3048; 
	*temp = T;
	//*pressure = P;
	*pressure = (P)*100;
}
