#ifdef GC_NRF52_DEV 
#include <string.h>
#include "spi_pah8002.h" //hkim 2016.7.7 
#include "spi_master_previous.h"
#include  "spi.h"
#include "nrf_error.h"
#include "nrf_drv_spi.h"
#include "nrf_drv_config.h"
#include "app_util_platform.h"
#include "nrf_drv_ppi.h"
#include "nrf_ppi.h"
#include "nrf_drv_gpiote.h"
#endif /* GC_NRF52_DEV */
#include "pah8002_comm.h"
#include "pah8002.h"
#include "spi.h"

static uint8_t		_real_bank = 0xFF;		// 0x00 ~ 0x03
static uint8_t		_spi_bank = 0xFF;		// 0x00 ~ 0x07

static int _set_bank(uint8_t addr);
static int _write_reg(uint8_t addr, uint8_t data);
static int _read_reg(uint8_t addr, uint8_t *data, uint32_t size);

uint8_t pah8002_write_reg(uint8_t addr, uint8_t data)
{
	uint8_t ret = 0;
	
	if (addr == 0x7F)
	{
		ret = _write_reg(0x7F, data);
		if (ret == PAH_SPI_SUCCESS)
		{
			_real_bank = data;
			_spi_bank = data;
		}
		return ret;
	}

	ret = _set_bank(addr);
	if (ret != PAH_SPI_SUCCESS)
		return ret;

	return _write_reg(addr, data);
}
uint8_t pah8002_read_reg(uint8_t addr, uint8_t *data)
{
	uint8_t ret = 0;
	
	ret = _set_bank(addr);
	if (ret != 0)
		return ret;
	
#ifdef GC_NRF52_PAHR
	return _read_reg(addr, data, 2);
#else
	return _read_reg(addr, data, 1);
#endif
}
uint8_t pah8002_burst_read_reg(uint8_t addr, uint8_t *data, uint32_t rx_size)
{
	uint8_t ret = 0;
	
	ret = _set_bank(addr);
	if (ret != 0)
		return ret;
	
	if (rx_size == CH_4_MAX_BUFF)
		return _4ch_raw_data_read_reg(addr);
	else
		return _burst_read_reg(addr, data, rx_size);
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

		return _write_reg(0x7F, spi_bank);
	}
	return 0;
}
/* De-select chipset */
//nrf_drv_gpiote_out_set(GC_SPI_PAH8002_PIN_CS);
/* Set chip select signal active before SPI transfer */ 
//nrf_drv_gpiote_out_clear(GC_SPI_PAH8002_PIN_CS);
extern const nrf_drv_spi_t spi_pah8002;
extern uint8_t pah8002_spi_rx_buff[4];
extern uint8_t pah8002_reg_rx;

static int _write_reg(uint8_t addr, uint8_t data)
{
	int32_t err_code;	
	addr |= (0x80);	//write, bit7 = 1			==> Write, bit 7 is 1
#ifdef GC_NRF52_PAHR
	uint8_t tx_data[2];
	uint8_t rx_data;
	
	memset(tx_data,0,sizeof(tx_data));
	memset(&rx_data,0,sizeof(rx_data));

	tx_data[0] = addr;
	tx_data[1] = data;

	//spi_pah8002_send_recv(tx_data, &rx_data, 2);
	//spi_pah8002_send_recv(tx_data, 2, NULL, 0);
	err_code = nrf_drv_spi_transfer(&spi_pah8002, tx_data, sizeof(tx_data), NULL, 0);
	if (err_code != NRF_SUCCESS)
	{
		printf("\r\nSPI transfer error(code %d)", err_code);
		return PAH_SPI_FAIL;
	}
	return PAH_SPI_SUCCESS;
#else /* GC_NRF52_PAHR */
	return spi_write_reg(addr, data);
#endif /* GC_NRF52_PAHR */
}

static int _read_reg(uint8_t addr, uint8_t *rx_data, uint32_t size)
{
	int32_t err_code;
	addr &= (0x7F); //read, bit7 = 0			==> Read, bit 7 is 0
#ifdef GC_NRF52_PAHR
	uint8_t tx_data[1];
	
	memset(tx_data,0,sizeof(tx_data));
	
	tx_data[0]= addr;
	//printf("\r\n PAH8002 read addr 0x%x", tx_data[0]);
	//spi_pah8002_send_recv(tx_data, 2, rx_data, 2);
	memset(pah8002_spi_rx_buff,0,sizeof(pah8002_spi_rx_buff));
	err_code = nrf_drv_spi_transfer(&spi_pah8002, tx_data, 2, pah8002_spi_rx_buff, size);
	if (err_code != NRF_SUCCESS)
	{
		printf("\r\nSPI transfer error(code %d)", err_code);
		return PAH_SPI_FAIL;
	}
	//printf("\r\n Reg Rx : 0x%x", pah8002_spi_rx_buff[1]);
	return PAH_SPI_SUCCESS;
#else
	return spi_burst_read_reg(addr, data, size);
#endif
}

static uint8_t pah8002_raw_data_buf[CH_4_MAX_BUFF];
static uint8_t pah8002_chksum[4];
void get_pah8002_checksum()
{
	memset(pah8002_chksum,0,sizeof(pah8002_chksum));
	memset(pah8002_spi_rx_buff, 0, sizeof(pah8002_spi_rx_buff));
	
	/* read checksum data */
	pah8002_write_reg(0x7f, 0x02);
	//pah8002_burst_read_reg(0x80, rx_buff, sizeof(rx_buff));
	pah8002_read_reg(0x80, pah8002_spi_rx_buff); 		
	pah8002_chksum[0] = pah8002_spi_rx_buff[1];
	
	pah8002_read_reg(0x81, pah8002_spi_rx_buff); 		
	pah8002_chksum[1] = pah8002_spi_rx_buff[1];
	
	pah8002_read_reg(0x82, pah8002_spi_rx_buff); 		
	pah8002_chksum[2] = pah8002_spi_rx_buff[1];
	
	pah8002_read_reg(0x83, pah8002_spi_rx_buff); 		
	pah8002_chksum[3] = pah8002_spi_rx_buff[1];
	
	printf("\r\n checksum value : 0x%02x 0x%02x 0x%02x 0x%02x", 
						pah8002_chksum[0], pah8002_chksum[1],
						pah8002_chksum[2], pah8002_chksum[3]);

}

void compare_pah8002_checksum(void)
{
	uint32_t *s = NULL;
	uint32_t cks_cal;
	uint32_t cks_rx ;
	uint32_t i ;
	int samples_per_read = HEART_RATE_MODE_SAMPLES_PER_READ ;
	
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
		
	printf("\r\nchecksum result : %d(0x%08x)  %d(0x%08x)", cks_cal, cks_cal, cks_rx, cks_rx);
	if(cks_cal != cks_rx)
	{
		printf("\r\nchecksum error");
	}
	else
	{
		printf("\r\nchecksum OK %d", cks_cal);
	}
}
#if 0
static int _burst_read_reg(uint8_t addr, uint8_t *rx_data, uint32_t rx_length)
{
	int32_t err_code;
	addr &= (0x7F); //read, bit7 = 0			==> Read, bit 7 is 0

	uint8_t tx_data[1];
	
	memset(tx_data,0,sizeof(tx_data));
	
	tx_data[0]= addr;
	
	if (rx_length > 4)
	{
		printf("\r\n Burst read (rx_length %d)!", rx_length-1);
		
		nrf_gpio_pin_clear(GC_SPI_PAH8002_PIN_CS);

	/* rx_data of SPI response include 1 byte length of tx_data. 
	    So that reason, request rx_legnth is rx_length+1.
			Real rx_data is started from *(rx_data+1) not *(rx_data+0) */
		err_code = nrf_drv_spi_transfer(&spi_pah8002, tx_data, sizeof(tx_data), rx_data, rx_length);
		if (err_code != NRF_SUCCESS)
		{
			printf("\r\nSPI transfer error(code %d)", err_code);
			return PAH_SPI_FAIL;
		}
		nrf_gpio_pin_set(GC_SPI_PAH8002_PIN_CS);
		
		printf("\r\n SPI Rx");
		DumpMemory(rx_data,rx_length);
		
		printf("\r\n");
		printf("\r\n Saved Raw Data");
		memset(pah8002_raw_data_buf,0, sizeof(pah8002_raw_data_buf));
		memcpy(pah8002_raw_data_buf, rx_data+1, rx_length-1); 
		DumpMemory(pah8002_raw_data_buf, rx_length-1);
		printf("\r\n");
	}
	else
	{
		err_code = nrf_drv_spi_transfer(&spi_pah8002, tx_data, 2, rx_data, rx_length);
		if (err_code != NRF_SUCCESS)
		{
			printf("\r\nSPI transfer error(code %d)", err_code);
			return PAH_SPI_FAIL;
		}
	}

	return PAH_SPI_SUCCESS;
}
#else
static int _burst_read_reg(uint8_t addr, uint8_t *rx_data, uint32_t rx_length)
{
	int32_t err_code;
	addr &= (0x7F); //read, bit7 = 0			==> Read, bit 7 is 0

	uint8_t tx_data[1];
	
	memset(tx_data,0,sizeof(tx_data));
	
	tx_data[0]= addr;
	
	if (rx_length > 4)
	{
		printf("\r\n Burst read (rx_length %d)!", rx_length-1);
		
		//nrf_gpio_pin_clear(GC_SPI_PAH8002_PIN_CS);

	/* rx_data of SPI response include 1 byte length of tx_data. 
	    So that reason, request rx_legnth is rx_length+1.
			Real rx_data is started from *(rx_data+1) not *(rx_data+0) */
		err_code = nrf_drv_spi_transfer(&spi_pah8002, tx_data, sizeof(tx_data), rx_data, rx_length);
		if (err_code != NRF_SUCCESS)
		{
			printf("\r\nSPI transfer error(code %d)", err_code);
			return PAH_SPI_FAIL;
		}
		//nrf_gpio_pin_set(GC_SPI_PAH8002_PIN_CS);
		
		/* save raw data */
		//printf("\r\n");
		//printf("\r\n Saved Raw Data");
		memset(pah8002_raw_data_buf,0, sizeof(pah8002_raw_data_buf));
		memcpy(pah8002_raw_data_buf, rx_data+1, rx_length-1); 
		//DumpMemory(pah8002_raw_data_buf, rx_length-1);
		//printf("\r\n");
		
		/* get checksum */
		get_pah8002_checksum();
		
		/* disable interrupt */
		pah8002_write_reg(0x7f, 0x02);
		pah8002_write_reg(0x75, 0x01);
		pah8002_write_reg(0x75, 0x00);
		
		/* compare checksum */
		compare_pah8002_checksum();

	}
	else
	{
		err_code = nrf_drv_spi_transfer(&spi_pah8002, tx_data, 2, rx_data, rx_length);
		if (err_code != NRF_SUCCESS)
		{
			printf("\r\nSPI transfer error(code %d)", err_code);
			return PAH_SPI_FAIL;
		}
	}

	return PAH_SPI_SUCCESS;
}

#endif 

static int _4ch_raw_data_read_reg(uint8_t addr)
{
	uint32_t i=0;
	int32_t err_code;
	uint8_t rx_length=0;
	uint8_t tx_data[500];
	uint8_t rx_buff[81]; /* 1+80, 1 is tx_byte length */
	
	addr &= (0x7F); //read, bit7 = 0			==> Read, bit 7 is 0
	
	rx_length = sizeof(rx_buff)-1;
	memset(tx_data,0,sizeof(tx_data));
	memset(pah8002_raw_data_buf, 0, sizeof(pah8002_raw_data_buf));
		
	tx_data[0]= addr;

	printf("\r\n 4-channel raw data read (rx_size %d * 4)", sizeof(rx_buff));

	/* enable chip-select */
	nrf_gpio_pin_clear(GC_SPI_PAH8002_PIN_CS);
	
	for (i=0; i < 4; i++)
	{
		memset(rx_buff, 0, sizeof(rx_buff));

		err_code = nrf_drv_spi_transfer(&spi_pah8002, tx_data, sizeof(rx_buff), rx_buff, sizeof(rx_buff));
		if (err_code != NRF_SUCCESS)
		{
			printf("\r\nSPI transfer error(code %d)", err_code);
			return PAH_SPI_FAIL;
		}
		DumpMemory(rx_buff+1,sizeof(rx_buff)-1);
		
		memcpy(&pah8002_raw_data_buf[i*80], &rx_buff[1], rx_length);
	}
	/* disable chip select */
	nrf_gpio_pin_set(GC_SPI_PAH8002_PIN_CS);
	
	

	printf("\r\n");
	/* dump all rx */
	DumpMemory(pah8002_raw_data_buf,sizeof(pah8002_raw_data_buf));
#if 0
	printf("\r\n");
	{
		//uint32_t *ppg_data = (uint32_t *)pah8002_raw_data_buf;
		data_convert_4ch_to_3ch((uint32_t *)(pah8002_raw_data_buf), HEART_RATE_MODE_SAMPLES_PER_READ);
		//DumpMemory(pah8002_raw_data_buf,240);
		printf("\r\n");
		{
			uint32_t *ppg_data = (uint32_t *)pah8002_raw_data_buf ;
			for(i=0; i<240; i++)
			{
				printf("%d(0x%08x), ", *ppg_data, *ppg_data);
				ppg_data ++;
			}
			printf("\n");
		}
	}
#endif 
	return PAH_SPI_SUCCESS;
}

int32_t pah8002_raw_data_checksum(void)
{
	uint8_t rx_buff[5];
	uint8_t cks[4] ;
	uint8_t __pah8002_raw_data_buf[CH_4_MAX_BUFF];
	uint32_t *s = NULL;
	uint32_t cks_cal;
	uint32_t cks_rx ;
	uint32_t i ;
	int samples_per_read = HEART_RATE_MODE_SAMPLES_PER_READ ;
	
	memcpy(__pah8002_raw_data_buf, pah8002_raw_data_buf, sizeof(pah8002_raw_data_buf));
	memset(cks,0,sizeof(cks));
	/* read checksum data */
	pah8002_write_reg(0x7f, 0x02);
	#if 0 
	pah8002_burst_read_reg(0x80, rx_buff, sizeof(rx_buff));
	memcpy(cks, &rx_buff[1], sizeof(cks));
	#else
		pah8002_read_reg(0x80, pah8002_spi_rx_buff); 		
	cks[0] = pah8002_spi_rx_buff[1];
		pah8002_read_reg(0x81, pah8002_spi_rx_buff); 		
	cks[1] = pah8002_spi_rx_buff[1];
		pah8002_read_reg(0x82, pah8002_spi_rx_buff); 		
	cks[2] = pah8002_spi_rx_buff[1];
		pah8002_read_reg(0x83, pah8002_spi_rx_buff); 		
	cks[3] = pah8002_spi_rx_buff[1];
	#endif
	
	printf("\r\n checksum value : 0x%02x 0x%02x 0x%02x 0x%02x", cks[0], cks[1],cks[2], cks[3]);

	DumpMemory(__pah8002_raw_data_buf, sizeof(pah8002_raw_data_buf));
	
	s = (uint32_t *)__pah8002_raw_data_buf ;
	cks_cal = *s;
	cks_rx = *((uint32_t *)cks) ;
	
	//checksum compare
	printf("\r\n start : cks_cal %d(0x%08x) checksum %d(0x%08x)", cks_cal, cks_cal, cks_rx, cks_rx);
	printf("\r\nsample_per_read %d", samples_per_read);
	for(i=1; i<samples_per_read; i++)
	{
		//printf("\r\n cks_cal %d(0x%08x), (buff-%d) %d(0x%08x)", cks_cal, cks_cal, i, *(s+i), *(s+i));
		cks_cal = cks_cal ^ (*(s+i)) ;
	}
		
	printf("\r\nchecksum result : %d(0x%08x)  %d(0x%08x)", cks_cal, cks_cal, cks_rx, cks_rx);
	if(cks_cal != cks_rx)
	{
		printf("\r\nchecksum error");
	}
	else
	{
		printf("\r\nchecksum OK %d", cks_cal);
	}
				
}
#if 0 
void cmd_pah8002_log(void) 
{
	int i = 0 ;
	uint32_t *ppg_data = (uint32_t *)pah8002_raw_data_buf;
	//int16_t *mems_data = _pah8002_data.mems_data ;
	//printf("Frame Count, %d \n", _pah8002_data.frame_count);
	printf("Time, %d \n", _pah8002_data.time);
	printf("PPG, %d, %d, ", _pah8002_data.touch_flag, _pah8002_data.nf_ppg_per_channel);
	for(i=0; i<_pah8002_data.nf_ppg_channel * _pah8002_data.nf_ppg_per_channel; i++)
	{
		printf("%d, ", *ppg_data);
		ppg_data ++;
	}
	#if 0 
	printf("\n");
	printf("MEMS, %d, ", _pah8002_data.nf_mems);
	for(i=0; i<_pah8002_data.nf_mems*3; i++)
	{
		printf("%d, ", *mems_data);
		mems_data ++;
	}
	#endif 
	printf("\n");	
}
#endif 