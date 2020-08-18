
#ifndef __GC_TIME_H__
#define __GC_TIME_H__

typedef struct
{
	unsigned short year;
	unsigned char	month;
	unsigned char	day;
	unsigned char	hour;
	unsigned char	minute;
	unsigned char	second;
} tTime;


int get_time( tTime* pTime, int flag);
#endif




