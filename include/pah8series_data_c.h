#ifndef __PAH8SERIES_DATA_C_H__
#define __PAH8SERIES_DATA_C_H__

#include <stdint.h>

typedef struct {
	uint8_t						frame_count;
	uint32_t					time;
	uint8_t						touch_flag;
	uint32_t					nf_ppg_channel;
	uint32_t					nf_ppg_per_channel;
	int32_t						*ppg_data;
	uint32_t					nf_mems;
	int16_t						*mems_data;
} pah8series_data_t;


#endif // PAH8011_DATA_H__
