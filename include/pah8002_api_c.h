#ifndef __PAH8002_API_C_H__
#define __PAH8002_API_C_H__

#include "pah8002_data_c.h"

#if defined(WIN32)
#	ifdef PXIALG_EXPORTS
#		define PXIALG_API	__declspec(dllexport)
#	else
#		define PXIALG_API	__declspec(dllimport)
#	endif
#else
#	define PXIALG_API
#endif	// WIN32

typedef enum {
	SENSOR_STATUS_UNRELIABLE = 0,
	SENSOR_STATUS_ACCURACY_LOW,
	SENSOR_STATUS_ACCURACY_MEDIUM,
	SENSOR_STATUS_ACCURACY_HIGH,
	SENSOR_STATUS_NO_CONTACT = (-1),
}sensor_status;

typedef enum {
	PAH8002_PARAM_IDX_SAMPLE_RATE = 0,
	PAH8002_PARAM_IDX_GSENSOR_MODE,
	PAH8002_PARAM_IDX_PPG_CH_NUM,
	PAH8002_PARAM_IDX_HAS_IR_CH,
	PAH8002_PARAM_IDX_SIGNAL_GRADE,
	PAH8002_PARAM_IDX_IIR_SMOOTH_PRED,
	PAH8002_PARAM_IDX_IIR_LP,
	PAH8002_PARAM_IDX_FIRST_HR_MULTIPLE_OFFSET_MEMS_THR,
	PAH8002_PARAM_IDX_FIRST_HR_IN_MOTION_FLAG,
	PAH8002_PARAM_IDX_FIRST_HR_MEMS_QUALITY_THR,
	PAH8002_PARAM_IDX_FIRST_HR_PPG_QUALITY_THR,
	PAH8002_PARAM_IDX_STATIC_MODE,
	PAH8002_PARAM_IDX_STATIC_HR_UP_IDX,
	PAH8002_PARAM_IDX_STATIC_HR_DN_IDX,
	PAH8002_PARAM_IDX_IS_AUTO,
	PAH8002_PARAM_IDX_IS_TAG,
	PAH8002_PARAM_IDX_IS_8002,
	PAH8002_NF_PARAM_IDX,
} pah8002_param_idx_t;

typedef enum {
	MSG_SUCCESS = 0,
	MSG_ALG_NOT_OPEN,
	MSG_ALG_REOPEN,
	MSG_MEMS_LEN_TOO_SHORT,
	MSG_NO_TOUCH,
	MSG_PPG_LEN_TOO_SHORT,
	MSG_FRAME_LOSS,
	MSG_INVALID_ARGUMENT,

	MSG_NO_MEM = 14,
	MSG_ECG_LEN_TOO_SHORT = 15,

	MSG_HR_READY = 0x30,
	MSG_SIGNAL_POOR = 0x40
} pah8002_msg_code_t;

PXIALG_API uint32_t pah8002_version(void);
PXIALG_API uint32_t pah8002_query_open_size(void);
PXIALG_API uint8_t pah8002_open(void *pBuffer);
PXIALG_API uint8_t pah8002_close(void);
PXIALG_API uint8_t pah8002_set_param(pah8002_param_idx_t idx, float p1);
PXIALG_API uint8_t pah8002_get_param(pah8002_param_idx_t idx, float *p1);
PXIALG_API uint8_t pah8002_get_hr(float *hr);
PXIALG_API uint8_t pah8002_get_hr_trust_level(int *hr_trust_level);
PXIALG_API uint8_t pah8002_entrance(pah8002_data_t *data8002);
PXIALG_API uint8_t pah8002_get_signal_grade(int16_t *grade);
PXIALG_API uint8_t pah8002_get_display_buffer(int32_t **buffer1, int32_t **buffer2, int32_t *size);

#endif
