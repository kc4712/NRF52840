#ifndef PAH8002_DATA_C_H__
#define PAH8002_DATA_C_H__

#include <stdint.h>

typedef struct pah8002_data {
	uint8_t						frame_count;
	uint32_t					time;
	uint8_t						touch_flag;
	uint32_t					nf_ppg_channel;
	uint32_t					nf_ppg_per_channel;
	int32_t						*ppg_data;
	uint32_t					nf_mems;
	int16_t						*mems_data;
} pah8002_data_t;


#endif // PAH8002_DATA_H__
