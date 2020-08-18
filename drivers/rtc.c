

#include "define.h"

#include	<stdio.h>
#include	<string.h>
#include	<stdlib.h>

#ifdef GC_NRF52_TWI
	#include "nrf_drv_twi.h"
#elif defined GC_NRF51_TWI
	#include "twi_master.h"
#endif /* GC_NRF52_TWI */
#include "nrf_delay.h"

#include "rtc.h"

 #ifdef GC_NRF52_TWI
extern const nrf_drv_twi_t m_twi_master_1;
#endif
unsigned char m_rtc_valid=0;
unsigned char m_rtc_valid_logical=1;

 #ifdef GC_NRF52_TWI
extern const nrf_drv_twi_t m_twi_master_1;
#endif

unsigned char  is_rtc_valid(void)
{
#if (__BOARD2__ || __BOARD3__)
	return m_rtc_valid;
#else
	return true;
#endif
}

int rtc_i2c_read(unsigned char start_reg, int n, unsigned char* array)
{
	int32_t err_code;
/*
	if(twi_master_transfer(RTC_DEVICE_ADDR_W, &cmd, 1, TWI_ISSUE_STOP)==false)
	{
		printf("\n\rrtc_i2c_send_I2C_bus_write error: i2c_send");
	}	
*/
	array[0]=start_reg;

#ifdef GC_NRF52_TWI
	err_code = nrf_drv_twi_tx(&m_twi_master_1,  (RTC_DEVICE_ADDR_W >> 1), &array[0], 1, true);
	if (err_code != NRF_SUCCESS)
	{
			printf("\n\r RTC TWI write error(error code %d)", err_code);
			return err_code;
	}
	err_code = nrf_drv_twi_rx(&m_twi_master_1, (RTC_DEVICE_ADDR_R >> 1), array, n);
	if (err_code != NRF_SUCCESS)
	{
			printf("\n\r RTC TWI read error(error code %d)", err_code);
			return err_code;
	}
	//APP_ERROR_CHECK(err_code);
#elif defined GC_NRF51_TWI 
	if(twi_master_transfer(RTC_DEVICE_ADDR_W, &array[0], 1, TWI_DONT_ISSUE_STOP)==false)
	{
		printf("\n\rRTC_I2C_error write");
		return false;
	}

	if(twi_master_transfer(RTC_DEVICE_ADDR_R, array, n, TWI_ISSUE_STOP)==false)
	{
		printf("\n\rRTC_I2C_error read");
		return false;
	}
#endif /* GC_NRF52_TWI */

	return true;
}



int32_t rtc_i2c_write(unsigned char reg, unsigned char* data, int n)
{
	int32_t 	err_code=0;
	unsigned char array[12] = {0};

	array[0]=reg;

	memcpy(&array[1],data,n);

#ifdef GC_NRF52_TWI
	err_code = nrf_drv_twi_tx(&m_twi_master_1, (RTC_DEVICE_ADDR_W >> 1), array, n+1, false);
	if (err_code != NRF_SUCCESS)
	{
			printf("\n\r RTC TWI write error(error code %d)", err_code);
			return false;
			
	}
#elif defined GC_NRF51_TWI 	
	if(twi_master_transfer(RTC_DEVICE_ADDR_W, array, n+1, TWI_ISSUE_STOP)==false)
	{
		printf("\n\r rtc_i2c_write error");
		return;
	}
#endif /* GC_NRF52_TWI */

	return true;
}


/**
 * BCD를 binanry으로 변환
 * @param   bcd                 unsigned char, 변환할 hexa데이터
 * @return  unsigned char,      binary 형식으로 리턴.
 */
unsigned char Bcd2Bin(unsigned char bcd)
{
    return ( ( (bcd&0xF0) >> 4 ) * 10 + (bcd&0x0F) ); 
}


/**
 * binary를 BCD로 변환
 * @param   bin                 unsigned char, 변환할 binary데이터
 * @return  unsigned char,      BCD 형식으로 리턴.
 */
unsigned char Bin2Bcd(unsigned char bin)
{
    return ( ( (bin/10) << 4 ) + bin%10 );
}





void rtc_set_time(short year, unsigned char month, unsigned char day, unsigned char hour, unsigned char min, unsigned char sec)
{
	unsigned char arr[12];

	arr[0]=Bin2Bcd(sec);;
	arr[1]=Bin2Bcd(min);;
	arr[2]=Bin2Bcd(hour);;
	arr[3]=Bin2Bcd(day);;
	arr[4]=0;
	arr[5]=Bin2Bcd(month);
	arr[6]=Bin2Bcd(year-2000);
	rtc_i2c_write(4, arr, 7);

	m_rtc_valid=true;
	m_rtc_valid_logical=true;
}


/*

void rtc_get_time(tTime* ptime)
{
	unsigned char array[12];

	rtc_i2c_read(4,7, array);

	printf("\r\n rtc = ");
	for(i=0;i<7;i++)
		printf("%02x ",array[i]  );

	arr[0]=Bin2Bcd(sec);;
	arr[1]=Bin2Bcd(min);;
	arr[2]=Bin2Bcd(hour);;
	arr[3]=Bin2Bcd(day);;
	arr[4]=0;
	arr[5]=Bin2Bcd(month);
	arr[6]=Bin2Bcd(year-2000);
	rtc_i2c_write(4, arr, 7);

	m_rtc_valid=true;
}

*/


void init_rtc(void)
{
	unsigned char arr[8];

	arr[0]=1;
	arr[1]=0;

	rtc_i2c_write(0, arr, 2);
	rtc_i2c_read(4,1,arr);

	if( !(arr[0]&0x80) )
		m_rtc_valid=true;
#ifndef _FIT_ETRI_TEST_ 
	printf("\n\r init RTC : valid=%d",m_rtc_valid);
#endif
}

#if 1 /* hkim 1.3.9.10 2016.1.26 세계시간 동기화 */	
int is_rtc_zero_time()
{
	unsigned short	year;
	unsigned char array[12];

	memset(array,0,sizeof(array));
	
	if(!rtc_i2c_read(4,7, array))
		return true;

	year = Bcd2Bin(array[6])+2000;
	
	/* hkim v1.5.16.0 2016.5.12. year < 2000 --> year <= 2000 
	    밴드가 방전된 후 부팅하면 2000년 부터 시작하므로 피쳐가 생성됨을 방지 */
	if (year <= 2000 || year >= 2030) 
		return true;
	
	return false;
}
#endif /* hkim 1.3.9.10 2016.1.26 */












