#ifndef _GC_UTIL_H_
#define _GC_UTIL_H_

#include "nrf.h"
#include "bma2x2.h"

#define ACC_AXIS_NUM	3
//#define ACC_BUFF_MAX	HEART_RATE_MODE_SAMPLES_PER_CH_READ
#define ACC_BUFF_MAX	25


void update_accelerometer_buffer(struct bma2x2_accel_data *xyz);
void accelerometer_get_fifo(int16_t **mems_data, uint32_t *nf_mems);


#endif 
