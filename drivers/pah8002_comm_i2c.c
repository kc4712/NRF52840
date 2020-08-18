#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include "bsp_btn_ble.h"
#include "nrf_delay.h"
#include "pah8002_comm.h"
#include "gc_twi.h"
#include "pah8002.h"
#include "print.h"
#include "command.h"
//#include "i2c.h"

#define printf(...)

static uint8_t		_real_bank = 0xFF;		// 0x00 ~ 0x03
static uint8_t		_spi_bank = 0xFF;		// 0x00 ~ 0x07

static int _set_bank(uint8_t addr);

extern uint8_t pah8002_rx_buff[4];
extern uint8_t pah8002_reg_rx;

uint8_t pah8002_raw_data_buf[CH_3_MAX_BUFF];
uint8_t pah8002_chksum[3];

static int _burst_read_reg(uint8_t reg_addr, uint8_t *rx_data, uint32_t rx_length)
{
	int32_t err_code;
	
	err_code = gc_twi_read(TWI_SLAVE_ADDR_PAH8002, reg_addr, rx_data, rx_length);
	if (err_code != NRF_SUCCESS)
	{
		printf("\r\n SPI transfer error(code %d)", err_code);
		return PAH_SPI_FAIL;
	}
	
	printf("\r\n TWI Rx (length %d)", rx_length);
	//DumpMemory(rx_data, rx_length);

	return PAH_SPI_SUCCESS;
}

static int _burst_loop_read_reg(uint8_t reg_addr, uint8_t *rx_data, uint32_t total_rx_length, uint32_t segment)
{
	uint32_t i=0;
	int32_t err_code;
	uint8_t *p_rxbuff=NULL;
	uint8_t rx_length=segment;
	uint32_t loop=0;
		
	memset(pah8002_raw_data_buf, 0, sizeof(pah8002_raw_data_buf));
	loop = total_rx_length / segment;

	printf("\r\n total length, seg, loop : %d %d %d", total_rx_length, segment, loop);
	
	p_rxbuff = (uint8_t *)malloc(rx_length);
	
	/* enable chip-select */
	nrf_delay_ms(1); /* FIXME */
	
	for (i=0; i < loop; i++)
	{
		memset(p_rxbuff, 0, rx_length);

		//printf("\r\n burst_loop_read : 0x%x %d", reg_addr, rx_length);
		err_code = gc_twi_read(TWI_SLAVE_ADDR_PAH8002, reg_addr+(i*rx_length), p_rxbuff, rx_length);
		//err_code = gc_twi_read(TWI_SLAVE_ADDR_PAH8002, reg_addr, p_rxbuff, rx_length);
		//DumpMemory(p_rxbuff, rx_length);
		if (err_code != NRF_SUCCESS)
		{
			printf("\r\n TWI transfer error(code %d)", err_code);
			free(p_rxbuff);
			return PAH_SPI_FAIL;
		}

		memcpy(&pah8002_raw_data_buf[i*rx_length], p_rxbuff, rx_length);
	}

	printf("\r\n");
	/* dump all rx */
	//DumpMemory(pah8002_raw_data_buf,sizeof(pah8002_raw_data_buf));

	free(p_rxbuff);
	return PAH_SPI_SUCCESS;
}

uint8_t pah8002_write_reg(uint8_t reg_addr, uint8_t data)
{
	uint8_t ret = 0;

	if (reg_addr == 0x7F)
	{
		ret = gc_twi_write(TWI_SLAVE_ADDR_PAH8002, 0x7F, data);
		if (ret == PAH_TWI_SUCCESS)
		{
			_real_bank = data;
			_spi_bank = data;
		}
		return ret;
	}

	ret = _set_bank(reg_addr);
	if (ret != PAH_TWI_SUCCESS)
		return ret;

#ifdef GC_NRF52_PAHR
	return gc_twi_write(TWI_SLAVE_ADDR_PAH8002, reg_addr, data);
#else
	return i2c_write_reg(I2C_ID_PAH8002, addr, data);
#endif 
}


uint8_t pah8002_read_reg(uint8_t reg_addr, uint8_t *data)
{
	uint8_t ret = 0;
	
	ret = _set_bank(reg_addr);
	if (ret != 0)
		return ret;
#ifdef GC_NRF52_PAHR
	return gc_twi_read(TWI_SLAVE_ADDR_PAH8002, reg_addr, data, 1);
#else
	return i2c_read_reg(I2C_ID_PAH8002, addr, data);
#endif
}

uint8_t pah8002_burst_read_reg(uint8_t reg_addr, uint8_t *rx_data, uint32_t rx_size, uint32_t loop)
{
	uint8_t ret = 0;
	
	ret = _set_bank(reg_addr);
	if (ret != 0)
		return ret;
	
	if (rx_size == 1)
		return pah8002_read_reg(reg_addr, rx_data);

	if  ( rx_data != NULL)
		return _burst_read_reg(reg_addr, rx_data, rx_size);
	else 
		return _burst_loop_read_reg(reg_addr, rx_data, rx_size, loop);
}


static int _set_bank(uint8_t addr)
{
	bool highpart = (addr >= 0x80);
	uint8_t spi_bank = _real_bank;

	if (highpart)
		spi_bank += 4;

	if (_spi_bank != spi_bank)
	{
		_spi_bank = spi_bank;
		//printf("\r\n set_bank : switch to bank %d", spi_bank);
		return gc_twi_write(TWI_SLAVE_ADDR_PAH8002, 0x7F, spi_bank);
	}
	return 0;
}

void pah8002_raw_data_checksum(void)
{
	/* get checksum */
	get_pah8002_checksum();
		
	/* disable interrupt */
	pah8002_write_reg(0x7f, 0x02);
	pah8002_write_reg(0x75, 0x01);
	pah8002_write_reg(0x75, 0x00);
		
	/* compare checksum */
	compare_pah8002_checksum();
				
}

void get_pah8002_checksum(void)
{
	printf("\r\n>>> get_pah8002_checksum");
	
	memset(pah8002_chksum,0,sizeof(pah8002_chksum));
	
	/* read checksum data */
	pah8002_write_reg(0x7f, 0x02);
	
	pah8002_burst_read_reg(0x80, pah8002_chksum, 4,0);

	#if 0
	printf("\r\n checksum value : 0x%02x 0x%02x 0x%02x 0x%02x", 
						pah8002_chksum[0], pah8002_chksum[1],
						pah8002_chksum[2], pah8002_chksum[3]);
	#endif 
		printf("\r\n<<< get_pah8002_checksum");
}

void compare_pah8002_checksum(void)
{
	uint32_t *s = NULL;
	uint32_t cks_cal;
	uint32_t cks_rx ;
	uint32_t i ;
	int samples_per_read = HEART_RATE_MODE_SAMPLES_PER_READ ;
	
	//DumpMemory(pah8002_raw_data_buf,sizeof(pah8002_raw_data_buf));
	
	s = (uint32_t *)pah8002_raw_data_buf ;
	cks_cal = *s;
	cks_rx = *((uint32_t *)pah8002_chksum) ;
	
	//checksum compare
	printf("\r\n start : cks_cal %d(0x%08x) checksum %d(0x%08x)", cks_cal, cks_cal, cks_rx, cks_rx);
	printf("\r\nsample_per_read %d", samples_per_read);
	
	for(i=1; i<samples_per_read; i++)
	{
		//printf("\r\n cks_cal %d(0x%08x), (buff-%d) %d(0x%08x)", cks_cal, cks_cal, i, *(s+i), *(s+i));
		cks_cal = cks_cal ^ (*(s+i)) ;
	}
		
	printf("\r\n * result");
	printf("\r\ncalculated %d(0x%08x)  register %d(0x%08x)", cks_cal, cks_cal, cks_rx, cks_rx);
	if(cks_cal != cks_rx)
	{
		printf("\r\nchecksum error");
	}
	else
	{
		printf("\r\nchecksum OK %d", cks_cal);
	}
}
