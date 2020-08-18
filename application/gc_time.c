#include <stdint.h>

#include "define.h"
#include "gc_time.h"
#include "rtc.h"

extern unsigned char m_rtc_valid;

// flag => 1 :  rtc 값을 상태에 관게없이 읽음 
int get_time( tTime* pTime, int flag)
{
	unsigned char array[12];

	if(!rtc_i2c_read(4,7, array))
		return false;

	pTime->year = Bcd2Bin(array[6])+2000;
	pTime->month=Bcd2Bin(array[5]);
	pTime->day=Bcd2Bin(array[3]);
	pTime->hour=Bcd2Bin(array[2]);
	pTime->minute=Bcd2Bin(array[1]);
	pTime->second=Bcd2Bin(array[0]&0x7f);

		
	if(array[0]&0x80)
	{
		if(!flag)
		{
			pTime->hour=0;
			pTime->minute=0;
		}
		
		m_rtc_valid=false;
	}

#if 1 /* v1.3.8 hkim 2015.11.25 */
		/* hkim 1.3.9.10,  2016.1.26 ,  2015 --> 2000 */
		/* hkim v1.5.16.0 2016.5.12. year < 2000 --> year < 2015 
	    밴드가 방전된 후 부팅하면 2000년 부터 시작하므로 피쳐가 생성됨을 방지 */
	if (pTime->year <= 2000 || pTime->year >= 2030) 
		return false;
#endif /* v1.3.8 */	

	return true;
}
