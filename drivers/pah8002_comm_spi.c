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

//규창 SPI.C에있는 드라이버 제어용이며 본 피트니스에는 다른데 선언되어있기에 이 파일로 받아와서 사용 
extern const nrf_drv_spi_t spi_pah8002;


//read_reg용 전역변수 커맨드용 외에는 이제 안씀
uint8_t pah8002_rx_buff[4];
//extern uint8_t pah8002_reg_rx;


//규창 본 파일의_burst_loop_read_reg()함수에서 이용중
uint8_t pah8002_raw_data_buf[CH_4_MAX_BUFF];
uint8_t pah8002_chksum[4];

/*
//규창 pah8002.c의 _pah8002_task 함수에서 check섬이나, 심박 원시 데이터를 연산기에 넣기위해 이용 하는 걸로 보임 
extern uint8_t pah8002_ppg_data[HEART_RATE_MODE_SAMPLES_PER_READ * 4] ;

//규창 전임자가 만들어놓은 심박 값 읽어오는 버스트 리드 함수... 동적할당이나.. 멤카피가 조금 걸린다...
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
	
	//따지고보면 p_rxbuff[uint32_t(length)]과 같을거라 보는데 이런 선언 될리가...
	p_rxbuff = (uint8_t *)malloc(rx_length);
	
	tx_data[0] = addr;
	
	// enable chip-select 
	nrf_gpio_pin_clear(GC_SPI_PAH8002_PIN_CS);
	//nrf_delay_ms(1); // FIXME 
	
	//루프 20회
	for (i=0; i < loop; i++)
	{
		//uint8 배열 [21] 개
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



////////////////////////////////////커맨드 에서 사용중///////////////////////////////////
//커맨드 코드에서 사용중...
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

//규창 체크섬은 pah8002.c에서 지역으로 따로 풀어서 사용중 헌데 커맨드에서 쓴다..
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
//규창 체크섬 비교 커맨드에서 쓴다...
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

//////////////////////////////////////////////////////////////////////////////그린콤 추가 함수들 끝




//////////////////////////////////////////////////////////////////////////////픽스아트 제공함수
//규창 커스터마이징
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

//이 부분은 task에서 심박값 읽을때 쓰는데, 공용으로는 못쓰게 변경
uint8_t pah8002_burst_read_reg(uint8_t addr, uint8_t *data, uint32_t rx_size)
{
	uint8_t ret = 0;
	
	ret = _set_bank(addr);
	if (ret != 0)
		return ret;
	
	
	//원본 
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



// 픽스아트에서 제공한 프로젝트의 SPI.C파일에 있던 쓰기, 읽기 함수들
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

//픽스아트에서 제공받은 함수긴 한데 그쪽 예제 프로젝트에서는 사용안하고 있음
bool spi_read_reg(uint8_t addr, uint8_t *data, uint32_t rx_size)
{
	return spi_burst_read_reg(addr, data, 1);
}

bool spi_burst_read_reg(uint8_t addr, uint8_t *data, uint32_t rx_size)
{
    /* 규창 4.20 수정 spi.c 반영
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
		//규창 uint8_t *data에 값이 써지게 되면 
		// data[0] == 주소
		// data[1] == 실제데이터
		
		// data[0] app영역에서는 변수로 이 값을 받아가는 문제가 있어  아래의 멤무브로 실제값만 가져가게 함
		// rx_size에 따라서는 아래의 멤무브가 통신중에 동작하여 데이터가 깨지는 문제 있을 수 있음
    // 하지만 본 코드에서 버스트 리드는 다른 방식으로 구현하였으므로 이 함수를 사용하는 rx_size 1의 단순 명령통신에는 지장없음
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
		//규창 memcpy 실시간 전송중일때 안정적일까...?
		//memmove(data, &buf[WRITE_LENGTH], rx_size);
    
    return PAH_SPI_SUCCESS;
}





