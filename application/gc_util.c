
#include <stdint.h>
#include <string.h>
#include "nordic_common.h"
#include "nrf.h"
#include "app_button.h"
#include "ble_nus.h"
#include "app_util_platform.h"
#include "bsp_btn_ble.h"
#include "nrf_drv_config.h"
#include "define.h"
#include "print.h"
#include "gc_util.h"
#include "bma2x2.h"
#include "pah8002.h"

struct bma2x2_accel_data buffer[100] = {0,}; /* used for pah8002 */
static uint8_t acc_count;
extern volatile uint8_t hr_processing;
extern uint8_t pahr_mode;
void update_accelerometer_buffer(struct bma2x2_accel_data *xyz)
{
	uint8_t i=0;

	//acc_buffer[acc_count] = xyz;
	if (hr_processing == 1 && pahr_mode == NORMAL_MODE){
		
		//memcpy(&acc_buffer[acc_count], xyz, sizeof(struct bma2x2_accel_data));
		buffer[acc_count].x = xyz->x << 2;
		buffer[acc_count].y = xyz->y << 2;
		buffer[acc_count].z = xyz->z << 2;
		//#ifdef MEMS_ZERO
		acc_count++;
		//#endif
	}
	else if (hr_processing == 0){
		for (i=0; i <100; i++){
			buffer[i].x = 0;
			buffer[i].y = 0;
			buffer[i].z = 0;
		}
		acc_count = 0;
	}
	
	
	/*if (acc_count <= ACC_BUFF_MAX) 
	{
		memcpy(&acc_buffer[acc_count - 1], xyz, sizeof(struct bma2x2_accel_data));
	} 
	else 
	{
				
		for(i = 0; i < ACC_BUFF_MAX - 1; i++) {
			memcpy(&acc_buffer[i], &acc_buffer[i+1], sizeof(struct bma2x2_accel_data));
		}
		memcpy(&acc_buffer[ACC_BUFF_MAX-1], xyz,  sizeof(struct bma2x2_accel_data));
	}*/
	//printf("\r\n [%d] %d %d %d", acc_count, (int16_t *)acc_buffer[0] , (int16_t *)acc_buffer[1], (int16_t *)acc_buffer[2]);
	/* debug */
	#if 0
	for (i=0; i <  acc_count; i++)
		printf("\r\n [%d] %d %d %d", i, buffer[i].x, buffer[i].y, buffer[i].z);
	#endif
	/* end of debug */
}


void accelerometer_get_fifo(int16_t **mems_data, uint32_t *nf_mems)
{
	*mems_data	= (int16_t *)buffer;
	*nf_mems			= acc_count;
	//*nf_mems			= ACC_BUFF_MAX;
	acc_count = 0;
}



