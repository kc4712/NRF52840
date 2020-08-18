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
// ��â adc �̺�Ʈ �ݹ� ���ŷ� ���� �Ⱦ�
//void gc_battery_sense(void);
//void gc_disable_saadc(void);
void saadc_init(void);
void init_battery_sense_event(void);

//���͸� Ÿ�̸� ����
void battery_level_meas_timeout_handler(void * p_context);

