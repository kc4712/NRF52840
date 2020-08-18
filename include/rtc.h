
#ifndef __RTC_H__
#define __RTC_H__

#include <stdint.h>

#ifdef GC_COACH_FIT_PCB
  #define RTC_INT_PIN	22
#endif 

#define RTC_DEVICE_ADDR_W		0xA2 //Module address write mode
#define RTC_DEVICE_ADDR_R		0xA3 // Module address read mode

int rtc_i2c_read(unsigned char start_reg, int n, unsigned char* array);
int rtc_i2c_write(unsigned char reg, unsigned char* data, int n);

unsigned char Bcd2Bin(unsigned char bcd);
unsigned char Bin2Bcd(unsigned char bin);

void init_rtc(void);
unsigned char  is_rtc_valid(void);

void rtc_set_time(short year, unsigned char month, unsigned char day, unsigned char hour, unsigned char min, unsigned char sec);
//void rtc_get_time(tTime* ptime);

#endif
