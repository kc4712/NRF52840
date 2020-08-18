#ifndef _PAH8002_H_
#define _PAH8002_H_
#include "stdbool.h"
#include "nrf.h"


///////그린콤 추가 선언들
/**
* @brief structure definition
*/

#ifdef GC_NRF52_SDNN
typedef struct
{
	int32_t			my_age;
	uint32_t 		timestamp_first_count;
	uint32_t 		sample_count;
	bool has_timestamp_start_collect;
	uint32_t 		timestamp_start_collect;
	bool					has_got_sdnn;
	uint32_t		collect_duration;
	uint8_t			*pBuffer;
} pxialg_sdnn_state;
#endif 

/**
* @brief value definition
*/
#define GC_HR_MIN															60
#define GC_HR_MAX														150
#define GC_HR_BASE_MOTION						1
#define GC_HR_BASE_SDNN								2
#define GC_HR_SDNN_GRANULARITY	5

#define GC_HR_SLEEP_STAT_NONE				0
#define GC_HR_SLEEP_STAT_ON						1
#define GC_HR_SLEEP_STAT_OFF						2
#define GC_HR_SLEEP_ON_DURATION		60 /* sec */
#define GC_HR_SLEEP_OFF_DURATION		1140 /* sec */

#ifdef GC_NRF52_PAHR
/* 1ch = 20 packet (1packet = 4 byte) */
#define CH_4_MAX_BUFF			320			/* 20 packet * 4 byte * 4 channel=320.  4 channel = IR On, IR Off, Green1, Green2 */
#define CH_3_MAX_BUFF			240			/* 20 packet * 4 byte * 3 channel=240.  4 channel = IR On, Green1, Green2 */
#endif /* GC_NRF52_PAHR */

#ifdef GC_NRF52_PAHR
	#define PAH_DEV_SUCCESS		1
	#define PAH_DEV_FAIL							0
	
	#define PAH_SPI_SUCCESS			0
	#define PAH_SPI_FAIL							1
	
	#define PAH_TWI_SUCCESS		0
	#define PAH_TWI_FAIL							1
#endif 

#define RW_MAX_RETRY						3


///////////////////////////////////////////////////////////////전임자가 원본에서 이동한 선언들
/* move to pah8002.h */
#define TOTAL_CHANNELS	3	//Using channel numbers
#define HEART_RATE_MODE_SAMPLES_PER_CH_READ (20)	//Numbers of PPG data per channel
#define HEART_RATE_MODE_SAMPLES_PER_READ (TOTAL_CHANNELS* HEART_RATE_MODE_SAMPLES_PER_CH_READ)	
#define TOTAL_CHANNELS_FOR_ALG	3

enum{
	SUSPEND_MODE = 0,
	TOUCH_MODE,
	NORMAL_MODE,
	NORMAL_LONG_ET_MODE,
	PAHR_STRESS_MODE,
	NONE,
};
////////////////////////////////////////////////////////////////전임자가  원본에서 이동한 선언들 끝



////////////////////////////////////////////////////////////그린콤 추가 함수들 
uint32_t get_sys_tick(void);
void print_normal_mode_reg_value(uint8_t start, uint8_t end);
void delay_ms(uint32_t ms);
uint8_t get_pah8002_hr(void);
bool pah8002_enter_normal_mode(void);
bool pah8002_enter_suspend_mode(void);
void gc_disable_pah8002(void);
void gc_enable_pah8002(void);
int32_t gc_protocol_ctrl_pah8002(uint8_t op_mode, uint8_t hr_control);

void pah8002_log_Greencomm(void);
/////////////////////////////////////////////////////////////아래는 원본
/**
* @brief function declaration
*/
bool pah8002_init(void);
void pah8002_deinit(void);
void pah8002_task(void);
void pah8002_intr_isr(void);


#endif
