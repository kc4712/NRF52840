#ifdef GC_NRF52_DEV 
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "spi_pah8002.h" //hkim 2016.7.7 
#include "spi_master_previous.h"
#include "spi.h"
#include "nrf_error.h"
#include "nrf_drv_spi.h"
#include "nrf_drv_config.h"
#include "app_util_platform.h"
#include "nrf_drv_ppi.h"
#include "nrf_ppi.h"
#include "nrf_drv_gpiote.h"
#include "nrf_delay.h"
#include "command.h"
#endif /* GC_NRF52_DEV */
#include "pah8002_comm.h"
#include "pah8002.h"
#include "spi.h"


static uint8_t		_real_bank = 0xFF;		// 0x00 ~ 0x03
static uint8_t		_spi_bank = 0xFF;		// 0x00 ~ 0x07

static int _set_bank(uint8_t addr);
static int _write_reg(uint8_t addr, uint8_t data);
static int _read_reg(uint8_t addr, uint8_t *data, uint32_t size);

//��â SPI.C���ִ� ����̹� ������̸� �� ��Ʈ�Ͻ����� �ٸ��� ����Ǿ��ֱ⿡ �� ���Ϸ� �޾ƿͼ� ��� 
extern const nrf_drv_spi_t spi_pah8002;


//read_reg�� �������� Ŀ�ǵ�� �ܿ��� ���� �Ⱦ�
uint8_t pah8002_rx_buff[4];
//extern uint8_t pah8002_reg_rx;


//��â �� ������_burst_loop_read_reg()�Լ����� �̿���
uint8_t pah8002_raw_data_buf[CH_4_MAX_BUFF];
uint8_t pah8002_chksum[4];

/*
//��â pah8002.c�� _pah8002_task �Լ����� check���̳�, �ɹ� ���� �����͸� ����⿡ �ֱ����� �̿� �ϴ� �ɷ� ���� 
extern uint8_t pah8002_ppg_data[HEART_RATE_MODE_SAMPLES_PER_READ * 4] ;

//��â �����ڰ� �������� �ɹ� �� �о���� ����Ʈ ���� �Լ�... �����Ҵ��̳�.. ��ī�ǰ� ���� �ɸ���...
static int _burst_loop_read_reg(uint8_t addr, uint8_t *rx_data, uint32_t total_rx_length, uint32_t segment)
{
	uint32_t i=0;
	int32_t err_code;
	uint8_t *p_rxbuff=NULL;
	uint8_t rx_length=segment+1;
	uint32_t loop=0;
	uint8_t tx_data[1];
		
	addr &= (0x7F); //read, bit7 = 0			==> Read, bit 7 is 0
	loop = total_rx_length / segment;

	//printf("\r\n total length, seg, loop : %d %d %d", total_rx_length, segment, loop);
	//rx_length = 21
	
	//�������� p_rxbuff[uint32_t(length)]�� �����Ŷ� ���µ� �̷� ���� �ɸ���...
	p_rxbuff = (uint8_t *)malloc(rx_length);
	
	tx_data[0] = addr;
	
	// enable chip-select 
	nrf_gpio_pin_clear(GC_SPI_PAH8002_PIN_CS);
	//nrf_delay_ms(1); // FIXME 
	
	//���� 20ȸ
	for (i=0; i < loop; i++)
	{
		//uint8 �迭 [21] ��
		memset(p_rxbuff, 0, rx_length);
    
		if (i != 0)	rx_length = segment;
		
		err_code = nrf_drv_spi_transfer(&spi_pah8002, tx_data, sizeof(tx_data), p_rxbuff, rx_length);
		if (err_code != NRF_SUCCESS)
		{
			printf("\r\nSPI transfer error(code %d)", err_code);
			free(p_rxbuff);
			return PAH_SPI_FAIL;
		}

		if (i == 0)
			memcpy(&pah8002_raw_data_buf[i*segment], p_rxbuff+1, segment);
		else
			memcpy(&pah8002_raw_data_buf[i*segment], p_rxbuff, segment);

	}
	// disable chip select 
	nrf_gpio_pin_set(GC_SPI_PAH8002_PIN_CS);

	// dump all rx 
	//DumpMemory(pah8002_raw_data_buf,total_rx_length);

	free(p_rxbuff);
	
	
	memcpy(pah8002_ppg_data, pah8002_raw_data_buf, sizeof(pah8002_ppg_data));
	return PAH_SPI_SUCCESS;
}
*/
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////



////////////////////////////////////Ŀ�ǵ� ���� �����///////////////////////////////////
//Ŀ�ǵ� �ڵ忡�� �����...
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

//��â üũ���� pah8002.c���� �������� ���� Ǯ� ����� �嵥 Ŀ�ǵ忡�� ����..
void get_pah8002_checksum(void)
{
	memset(pah8002_chksum,0,sizeof(pah8002_chksum));
	memset(pah8002_rx_buff, 0, sizeof(pah8002_rx_buff));
	
	// read checksum data 
	pah8002_write_reg(0x7f, 0x02);
	
	//pah8002_burst_read_reg(0x80, rx_buff, sizeof(rx_buff));
	pah8002_read_reg(0x80, pah8002_rx_buff); 		
	pah8002_chksum[0] = pah8002_rx_buff[1];
	
	pah8002_read_reg(0x81, pah8002_rx_buff); 		
	pah8002_chksum[1] = pah8002_rx_buff[1];
	
	pah8002_read_reg(0x82, pah8002_rx_buff); 		
	pah8002_chksum[2] = pah8002_rx_buff[1];
	
	pah8002_read_reg(0x83, pah8002_rx_buff); 		
	pah8002_chksum[3] = pah8002_rx_buff[1];
	
	#if 0 
	printf("\r\n checksum value : 0x%02x 0x%02x 0x%02x 0x%02x", 
						pah8002_chksum[0], pah8002_chksum[1],
						pah8002_chksum[2], pah8002_chksum[3]);
	#endif 
}
//��â üũ�� �� Ŀ�ǵ忡�� ����...
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

//////////////////////////////////////////////////////////////////////////////�׸��� �߰� �Լ��� ��




//////////////////////////////////////////////////////////////////////////////�Ƚ���Ʈ �����Լ�
//��â Ŀ���͸���¡
//pah8002_burst_read_reg()
//spi_write_reg()
//spi_read_reg()
//spi_burst_read_reg

uint8_t pah8002_write_reg(uint8_t addr, uint8_t data)
{
	uint8_t ret = 0;
	
	if (addr == 0x7F)
	{
		ret = _write_reg(0x7F, data);
		if (ret == 0)
		{
			_real_bank = data;
			_spi_bank = data;
		}
		return ret;
	}

	ret = _set_bank(addr);
	if (ret != 0)
		return ret;

	return _write_reg(addr, data);
}


uint8_t pah8002_read_reg(uint8_t addr, uint8_t *data)
{
	uint8_t ret = 0;
	
	ret = _set_bank(addr);
	if (ret != 0)
		return ret;

	return _read_reg(addr, data, 1);
}

//�� �κ��� task���� �ɹڰ� ������ ���µ�, �������δ� ������ ����
uint8_t pah8002_burst_read_reg(uint8_t addr, uint8_t *data, uint32_t rx_size)
{
	uint8_t ret = 0;
	
	ret = _set_bank(addr);
	if (ret != 0)
		return ret;
	
	
	//���� 
	return _read_reg(addr, data, rx_size);
	//return _burst_loop_read_reg(addr, data, rx_size, rx_size);
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

		// change bank
		return _write_reg(0x7F, spi_bank);
	}
	return 0;
}


static int _write_reg(uint8_t addr, uint8_t data)
{
	addr |= (0x80);	//write, bit7 = 1			==> Write, bit 7 is 1
	return spi_write_reg(addr, data);
}


static int _read_reg(uint8_t addr, uint8_t *data, uint32_t size)
{
	addr &= (0x7F); //read, bit7 = 0			==> Read, bit 7 is 0
	return spi_burst_read_reg(addr, data, size);
}



// �Ƚ���Ʈ���� ������ ������Ʈ�� SPI.C���Ͽ� �ִ� ����, �б� �Լ���
bool spi_write_reg(uint8_t addr, uint8_t data)
{
  uint8_t tx_buf[2];

	tx_buf[0] = addr;
	tx_buf[1] = data;
    

  uint32_t err_code = nrf_drv_spi_transfer(&spi_pah8002, tx_buf, ARRAY_SIZE(tx_buf), NULL, 0);
	//nrf_delay_ms(10); 
	if (err_code != NRF_SUCCESS)
	{
		printf("\r\nSPI transfer error(code %d)", err_code);
		return PAH_SPI_FAIL;
	}
    return PAH_SPI_SUCCESS;
}

//�Ƚ���Ʈ���� �������� �Լ��� �ѵ� ���� ���� ������Ʈ������ �����ϰ� ����
bool spi_read_reg(uint8_t addr, uint8_t *data, uint32_t rx_size)
{
	return spi_burst_read_reg(addr, data, 1);
}

bool spi_burst_read_reg(uint8_t addr, uint8_t *data, uint32_t rx_size)
{
    /* ��â 4.20 ���� spi.c �ݿ�
	  uint32_t err_code = 0;
    static const uint8_t length = 1;
    static uint8_t tx_buff[256];
    
    //memset(tx_buff, 0, rx_size);
    tx_buff[0] = addr;
	
		nrf_gpio_pin_clear(GC_SPI_PAH8002_PIN_CS);
	  nrf_delay_ms(1); 
    err_code = nrf_drv_spi_transfer(&spi_pah8002, tx_buff, length, data, length);
	  
		if (err_code != NRF_SUCCESS)
		{
			printf("\r\nSPI transfer error(code %d)", err_code);
			return PAH_SPI_FAIL;
		}
	  nrf_gpio_pin_set(GC_SPI_PAH8002_PIN_CS);
		//��â uint8_t *data�� ���� ������ �Ǹ� 
		// data[0] == �ּ�
		// data[1] == ����������
		
		// data[0] app���������� ������ �� ���� �޾ư��� ������ �־�  �Ʒ��� �⹫��� �������� �������� ��
		// rx_size�� ���󼭴� �Ʒ��� �⹫�갡 ����߿� �����Ͽ� �����Ͱ� ������ ���� ���� �� ����
    // ������ �� �ڵ忡�� ����Ʈ ����� �ٸ� ������� �����Ͽ����Ƿ� �� �Լ��� ����ϴ� rx_size 1�� �ܼ� �����ſ��� �������
		//memmove(data, data+1, rx_size);
    return PAH_SPI_SUCCESS;*/
		
		
		ret_code_t err_code = 0;
    static const uint8_t WRITE_LENGTH = 1;
    static uint8_t buf[320];
    
    buf[0] = addr;
    //nrf_gpio_pin_clear(GC_SPI_PAH8002_PIN_CS);
	  //nrf_delay_ms(1); 
    err_code = nrf_drv_spi_transfer(&spi_pah8002, buf, WRITE_LENGTH, buf, WRITE_LENGTH + rx_size);
    //nrf_delay_ms(10); 
		if (err_code != NRF_SUCCESS){
				printf("\r\n PAH8002 SPI Burst Comm ERROR" );
        return false;
		}
    //nrf_gpio_pin_set(GC_SPI_PAH8002_PIN_CS);
    memcpy(data, &buf[WRITE_LENGTH], rx_size);
		//��â memcpy �ǽð� �������϶� �������ϱ�...?
		//memmove(data, &buf[WRITE_LENGTH], rx_size);
    
    return PAH_SPI_SUCCESS;
}





