#ifndef _GC_ADC_H_
#define _GC_ADC_H_

#include "nrf_drv_gpiote.h"

/**@brief value definition ************************/
#define GC_ADC_BATTERY_FLAG_NONE				0
#define GC_ADC_BATTERY_FLAG_START			1
#define GC_ADC_BATTERY_FLAG_FINISH				2

#define GC_ADC_READ_MAX		3
#define GC_ADC_BATTERY_READ_PERIOD_CHARGING		60 /* sec */
#define GC_ADC_BATTERY_READ_PERIOD_LOW		180 /* sec */
#define GC_ADC_BATTERY_READ_PERIOD_NOMAL	3600 /* sec */
#endif 

/**@brief function definition ************************/
// 규창 adc 이벤트 콜백 제거로 이제 안씀
//void gc_battery_sense(void);
//void gc_disable_saadc(void);
void saadc_init(void);
void init_battery_sense_event(void);

//배터리 타이머 오픈
void battery_level_meas_timeout_handler(void * p_context);

