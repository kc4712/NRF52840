#ifndef _GC_SLEEP_H_
#define _GC_SLEEP_H_

#define GC_HZ_SENSOR_TIMER	20
#define GC_SLEEP_TIMEOUT	60 /* sec */

#include "stdint.h"

int32_t gc_sleep_mood_control(uint8_t );
void gc_power_off_prepare(void);
void configure_ram_retention(void);
void save_info_before_mcu_off(void);
void save_info_before_reset_point(uint32_t reset_point,uint32_t	reset_info_1,uint32_t	reset_info_2, uint32_t	reset_info_3,uint32_t	reset_info_4);	/*16.12.14 for test_log hm_lee*/
void save_data_before_reset(void);
#endif 
