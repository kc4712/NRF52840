/* includes */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>

#include "nordic_common.h"
#include "nrf.h"
#include "nrf51_bitfields.h"
#include "ble_hci.h"
#include "ble_advdata.h"
#include "ble_conn_params.h"
#include "softdevice_handler.h"
#include "app_timer.h"
#include "app_button.h"
#include "ble_nus.h"
#include "simple_uart.h"
#include "boards.h"
#include "ble_error_log.h"
#include "ble_debug_assert_handler.h"
#include "app_util_platform.h"
#include "heart_beat.h"
#include "rtc.h"
#include "nrf_delay.h"

#include "define.h"

#include "protocol.h"
#include "greencom_feature.h"
#include "spi_flash.h"
#include "oled.h"

#if 1 /* v1.3.5 hkim 2015.10.28 */
unsigned short g_total_day_jumprope;
#endif
unsigned short g_total_daystep=0;
#if 1 /* v1.3.9 hkim 2015.12.05 */
unsigned int g_accumulated_step=0;
extern unsigned char g_double_move_signal;
unsigned int total_accumulated_step_info[3];

/* for debug */
unsigned char dbg_feature_send, dbg_feature_save;
unsigned char dbg_hr_average;
#endif 

//extern char g_send_signal;
extern unsigned int g_time;
extern int g_pressure;
extern unsigned char g_battery;

static volatile unsigned short m_db_write_page=0;
static volatile unsigned short m_db_read_page=0;
static volatile unsigned char m_feature_data_count=0;
unsigned char previous_feature_not_exist=0; /* v1.5.13.0 hkim 2016.3.23. fill empty feature data */

struct feature_tbl_form_s m_feature_10min[FEATURE_SAVE_PERIOD_MIN];
//struct feature_tbl_form_s backup_feature_5min[FEATURE_SAVE_PERIOD_MIN];  /* v1.5.13.0 hkim 2016.3.23. fill empty feature data */
struct feature_tbl_form_s last_feature_5min[FEATURE_SAVE_PERIOD_MIN];  /* v1.5.13.0 hkim 2016.3.23. fill empty feature data */
//unsigned int empty_5min_feature_num=0; /* v1.5.13.0 hkim 2016.3.23. fill empty feature data */

PERSONAL_INFO		g_personal_info;


/************************************************************/

#define NUM_OF_DATA 1200
#define KIST_V5_0724 /* v5.1 */
#define KIST_V5_0729 /* v5.2 */
#define KIST_V5_0805 /* v5.3 */
#define KIST_V54_0819
#define KIST_V55_0903

#ifdef KIST_V54_0819
unsigned char window_size_y = 10;
#define def_window_size_y 10
#endif


#ifdef KIST_V5_0729
#define def_window_size 10
#define def_window_size_n 10
unsigned char window_size = 10;
unsigned char window_size_n = 10;
unsigned char window_size_swing = 10;
#ifdef KIST_V5_0805
unsigned char window_size_z = 18; /* greencomm 08.05 */
#define def_window_size_z 18 /* greencomm 08.05 */
#else
unsigned char window_size_z = 18;
#define def_window_size_z 18
#endif
#else
#define window_size 10
#endif

#ifdef KIST_V55_0903
unsigned char window_size_y_golf = 10;
#endif

#define step_frequency_thre 18 
#define feature_count 15
#define sen_data_frequency 20
#define feature_duration 60

#if 1 /*hkim 0730 */
unsigned short data_length_per_unittime = sen_data_frequency * feature_duration;
unsigned char averaging_win_size = 100; // 0.5sec
unsigned char var_win_size = 100;
unsigned int averaging_win_size_step = 1;
#else
#define data_length_per_unittime	sen_data_frequency * feature_duration;
#define averaging_win_size	100; // 0.5sec
#define var_win_size						100;
#define averaging_win_size_step	1;
#endif

#ifdef KIST_V5_0805
unsigned char averaging_win_size_pres = 200;
#endif

#ifdef KIST_V5_0729
#if 1 /* hkim 0730 */
char upper_norm_thre_max = 23;
char lower_norm_thre = 100;
int upper_norm_thre_min = 12;
#else
unsigned int  upper_norm_thre_max = 23;
unsigned int  lower_norm_thre = 100;
unsigned int  upper_norm_thre_min = 12;
#endif 
#else
unsigned char upper_norm_thre_max = 24;
unsigned char lower_norm_thre = 100;
unsigned char upper_norm_thre_min = 12;
#endif

#if 1 /* hkim 0730 */
unsigned char small_swing_thre = 40;
unsigned char upper_lower_time_diff_max = 16;
float previous_lower_peak_value = 0;
#else
#define small_swing_thre	40;
unsigned int upper_lower_time_diff_max = 16;
float previous_lower_peak_value = 0;
#endif 

#ifdef KIST_V5_0805
float upper_lower_peakvalue_diff = 0.5;
#else
float upper_lower_peakvalue_diff = 1.5;
#endif

#ifdef KIST_V5_0724
unsigned int previous_lower_peak_increase_time = 0, previous_upper_peak_increase_time = 0;
unsigned char upper_lower_time_diff_min = 1;
#endif

unsigned int previous_lower_peak_time = 0;
#ifdef KIST_V5_0729
#if 0
unsigned char lower_norm_thre_max = 12;
unsigned char peak_pos = 0;
#else
unsigned int lower_norm_thre_max = 12;
unsigned int peak_pos = 0;
#endif 
#endif

unsigned char continuous_step_flag = FALSE;
unsigned char continuous_swing_flag = FALSE;
unsigned int temp_step_count = 0;
unsigned short step_count = 0;
unsigned short small_swing_count = 0;

unsigned char upper_peak_occur = FALSE, lower_peak_occur = FALSE, swing_peak_occur = FALSE;
unsigned char other_swing_peak_occur = FALSE ,previous_continuous_step_flag = FALSE;

#ifdef KIST_V5_0729
unsigned char previous_continuous_swing_flag = FALSE;
#endif

float acc_mean_x_axis = 0, acc_mean_y_axis = 0, acc_mean_z_axis = 0,
		acc_norm_mean = 0;
float acc_var_x_axis = 0, acc_var_y_axis = 0, acc_var_z_axis = 0, acc_var_norm = 0;

#ifdef KIST_V55_0903
#if 0 /* v1.2.4  because of low memory :( hkim 2015.09.30 */
float acc_mean_y_axis_short_golf = 0;
#endif
#endif

#ifdef KIST_V5_0729
unsigned char step_y_axis_mean_winsize = 10;
float acc_mean_y_axis_10 = 0;
#endif

float acc_norm_mean_step = 0;

#ifdef KIST_V5_0729
#define continuous_step_occur_count_define 12 /* greencomm 08.05 */
#define continuous_swing_occur_count_define 5
#endif

#ifdef KIST_V5_0724
unsigned char continuous_step_occur_count = 12; /* greencomm 08.05 */
#else
unsigned char continuous_step_occur_count = 10;
#endif

#ifdef KIST_V5_0724
unsigned char max_step_frequency_thre = 22;
#else
unsigned char max_step_frequency_thre = 17;
#endif

unsigned char min_step_frequency_thre = 5;
unsigned int temp_swing_count = 0;
unsigned char swing_epoch_diff_max = 18;

unsigned int epoch = 0;

#ifdef KIST_V54_0819
float acc_buffer_y[def_window_size_y] = { 0, };
#endif
#ifdef KIST_V5_0729
float acc_buffer[def_window_size_z] = { 0, };
float acc_buffer_norm[def_window_size_n] = { 0, };
float acc_buffer_z[def_window_size_z] = { 0, };
#else
float acc_buffer[window_size] = { 0, };
#endif

#ifdef KIST_V5_0729
short step_count_epoch[continuous_step_occur_count_define] = { 0, };
short temp_swing_count_epoch_total[continuous_swing_occur_count_define] = { 0, };
#else
short step_count_epoch[300] = { 0, };
short temp_swing_count_epoch_total[300] = { 0, };
#endif
//float FEATURE[feature_count] = { 0, };


// 기압 관련 추가된 변수 06.02
float pre_mean = 0;
float previous_press = 0;
float pre_diff = 0, pre_diff_sum = 0;

#ifdef KIST_V5_0729
unsigned char jumping_rope_thre = 18;
unsigned char continuous_swing_occur_count = 5;
unsigned char max_swing_frequency_thre = 18;
unsigned char min_swing_frequency_thre = 5;
#else
unsigned char jumping_rope_thre = 22;
#endif

#ifdef KIST_V54_0819
int y_swing_front_sum_thre = -40; // 4G는 -40가 최대값.
#endif
unsigned char jumping_rope_thre_upper = 40;
unsigned short jumping_rope_count = 0;
unsigned char step_min_fre_thre = 5;
unsigned short large_swing_count = 0;
unsigned char large_swing_thre = 50; /* 5.3.2 08.12 */
unsigned int previous_epoch = 0;
#if 0 /* v1.2.4  because of low memory :( hkim 2015.09.30 */
float ampulitude_rtsl = 0, stepfrequency_rtsl = 0;
#endif

#ifdef KIST_V55_0903
unsigned char y_save_period = 10;
#if 0 /* v1.2.4 hkim 2015.9.30 */
float acc_buffer_y_extend[windown_size_y] = {0,};
#else
#define window_size_y_extend 20
/* 3sec if granularity is 250msec */
#define golf_avg_time_buffer_start	0
#define golf_avg_time_buffer_end		11 
/* 2sec if granularity is 250msec */
#define golf_backswing_time_buffer_start	12
#define golf_backswing_time_buffer_end	19 
float acc_buffer_y_extend[window_size_y_extend] = {0,};
#endif

#define y_extended_avg_thre	-7 /* GC */
#define y_extended_avg_size	12 /* GC */ /* v1.2.4 6 --> 12 hkim 2015.9.30 , 3초 평균 */
float y_extended_sum=0, y_extended_avg=0; /* GC */
unsigned char window_size_x = 10;
#define def_window_size_x 10
#if 0 /* v1.2.4  because of low memory :( hkim 2015.09.30 */
float acc_buffer_x[def_window_size_x] = {0,};
#endif
#endif


#if 1 /* v5.3.2 */
float pre_diff_press = 0, press_sum = 0;
unsigned char press_count = 0;
unsigned char set_press_sum = FALSE;
#endif 


#if __GREENCOM_24H__

#if 1 /* v1.2.4 hkim 2015.09.30 */
unsigned int g_personal_max_hb_pretime;
unsigned char g_personal_max_hb_motor;
#endif

#if 1 /* v1.3.1 hkim 2015.10.6 */
unsigned char hr_avg;
unsigned int hr_alarm_count;
extern unsigned char hr_avg_feature;
unsigned short hr_alarm_buffer[HR_BUF_DEF] = {0,};
#endif  /* if 1 v1.3.1 */

/* end of hkim 2015.6.5*/

#if 1 /* v1.4.11.1 hkim 2016.02.18 */
extern unsigned char is_uart_mode;
#endif 

#if 1 /* hkim 2015.6.6 */
double fixFloat(double a ,int n) {
    return ((int)(a * pow(10.0, n)))/pow(10.0,n);
}
#endif /* end of hkim 2015.6.6 */

/* called by greencom_feature_process() main.c */
void get_realtime_feature(unsigned int count, short acc_x, short acc_y, short acc_z)
{
	float x = (float)acc_x /50.0;
	float y = (float)acc_y /50.0;
	float z = (float)acc_z /50.0;

	double press=0; /* hkim 2015.6.6 */
#if 1 /* v1.2.4 hkim 2015.09.30 */
	unsigned char y_backswing=0; 
#endif 
	
#if 0
	unsigned int total_swing_count=0; 
#endif
	//ks_printf("\r\n %d %d %d", acc_x, acc_y, acc_z);
	//printf("x:%.2f y:%.2f z:%.2f\n",x,y,z);
	// Acc norm
	float acc_norm = sqrtf(powf(x, 2) + powf(y, 2) + powf(z, 2));
#ifdef KIST_V5_0729
	unsigned char swing_walking_flag = FALSE;
#endif
	//printf("norm:%.2f\n",acc_norm);
	// Acc buffer 걸음&스윙 검출
	epoch++;
	press = fixFloat( ((double)g_pressure/(double)100), 2);

#ifdef KIST_V5_0805
#if 1 /* v5.3.2 */
if(epoch == 1)
		pre_diff_press = press;

	if(epoch > 1100 && epoch < 1200) {
		if(previous_press != press && (abs(press - previous_press)) < 5) {
			press_sum += press;
			press_count++;
		}
	} else if(epoch >= 1200 && (epoch - 1100) % data_length_per_unittime == 0) {
		set_press_sum = TRUE;
	}

	if(epoch >= 1200 && previous_press != press && set_press_sum == TRUE && (abs(press - previous_press)) < 5) {
		press_sum += press;
		press_count++;
	}

	if(epoch % data_length_per_unittime == 0) {
		pre_diff_sum = press_sum / press_count - pre_diff_press;
		pre_diff_press = press_sum / press_count;
		press_count = 0;
		press_sum = 0;
		set_press_sum = FALSE;
	}

	previous_press = press;
#else /* v5.3.2 */
 	if(epoch == 1) {
		pre_mean = press;
	} else if(epoch < averaging_win_size_pres) {
		pre_mean = pre_mean * ((float) (epoch - 1) / epoch)
						+ (press / epoch);
	} else {
		pre_mean = pre_mean * ((float) (averaging_win_size_pres - 1) / averaging_win_size_pres)
								+ (press / averaging_win_size_pres);
	}

	if(epoch > 1)
		pre_diff = pre_mean - previous_press;
	previous_press = pre_mean;

	pre_diff_sum = pre_diff_sum + pre_diff;
#endif /* v5.3.2 */
#else /* KIST_V5_0805 */
	if (epoch > 1)
		pre_diff = press - previous_press;
	previous_press = press;
	if(abs(pre_diff) < 0.3)
		pre_diff_sum = pre_diff_sum + pre_diff;
#endif
	// Mean
	if (epoch == 1) {
		acc_mean_x_axis = x;
		acc_mean_y_axis = y;
		acc_mean_z_axis = z;
		acc_norm_mean = acc_norm;
	} else if (epoch < averaging_win_size) {
		acc_mean_x_axis = acc_mean_x_axis * ((float) (epoch - 1) / epoch)
				+ (x / epoch);
		acc_mean_y_axis = acc_mean_y_axis * ((float) (epoch - 1) / epoch)
				+ (y / epoch);
		acc_mean_z_axis = acc_mean_z_axis * ((float) (epoch - 1) / epoch)
				+ (z / epoch);
		acc_norm_mean = acc_norm_mean * ((float) (epoch - 1) / epoch)
				+ (acc_norm / epoch);
	} else {
		acc_mean_x_axis = acc_mean_x_axis
				* ((float) (averaging_win_size - 1) / averaging_win_size)
				+ (x / averaging_win_size);
		acc_mean_y_axis = acc_mean_y_axis
				* ((float) (averaging_win_size - 1) / averaging_win_size)
				+ (y / averaging_win_size);
		acc_mean_z_axis = acc_mean_z_axis
				* ((float) (averaging_win_size - 1) / averaging_win_size)
				+ (z / averaging_win_size);
		acc_norm_mean = acc_norm_mean
				* ((float) (averaging_win_size - 1) / averaging_win_size)
				+ (acc_norm / averaging_win_size);
	}
	//printf("acc_mean_x:%.2f acc_mean_y:%.2f acc_mean_z:%.2f\n",acc_mean_x_axis,acc_mean_y_axis,acc_mean_z_axis);
	// variance 실시간(android)

#ifdef KIST_V55_0903 /* v1.2.4 V54 -> V55 hkim 2015.09.30 */
#if 0 /* v1.2.4  because of low memory :( hkim 2015.09.30 */
	if (epoch == 1) {
		acc_mean_y_axis_short_golf = y;
	} else if (epoch < window_size_y_golf) {
		acc_mean_y_axis_short_golf = acc_mean_y_axis_short_golf * ((float) (epoch - 1) / epoch) + (y / epoch);
	} else {
		acc_mean_y_axis_short_golf = acc_mean_y_axis_short_golf	* ((float) (window_size_y_golf - 1) / window_size_y_golf) + (y / window_size_y_golf);
	}
#endif
#endif
	
	if (epoch == 1) {
		acc_var_x_axis = 0;
		acc_var_y_axis = 0;
		acc_var_z_axis = 0;
		acc_var_norm = 0;
	} else if (epoch < var_win_size) {
		acc_var_x_axis = acc_var_x_axis * ((float) (epoch - 1) / epoch)
				+ ((powf(acc_mean_x_axis - x, 2)) / epoch);
		acc_var_y_axis = acc_var_y_axis * ((float) (epoch - 1) / epoch)
				+ ((powf(acc_mean_y_axis - y, 2)) / epoch);
		acc_var_z_axis = acc_var_z_axis * ((float) (epoch - 1) / epoch)
				+ ((powf(acc_mean_z_axis - z, 2)) / epoch);
		acc_var_norm = acc_var_norm * ((float) (epoch - 1) / epoch)
				+ ((powf(acc_norm_mean - acc_norm, 2)) / epoch);
	} else {
		acc_var_x_axis = acc_var_x_axis
				* ((float) (var_win_size - 1) / var_win_size)
				+ ((powf(acc_mean_x_axis - x, 2)) / var_win_size);
		acc_var_y_axis = acc_var_y_axis
				* ((float) (var_win_size - 1) / var_win_size)
				+ ((powf(acc_mean_y_axis - y, 2)) / var_win_size);
		acc_var_z_axis = acc_var_z_axis
				* ((float) (var_win_size - 1) / var_win_size)
				+ ((powf(acc_mean_z_axis - z, 2)) / var_win_size);
		acc_var_norm = acc_var_norm
				* ((float) (var_win_size - 1) / var_win_size)
				+ ((powf(acc_norm_mean - acc_norm, 2)) / var_win_size);
	}
	//printf("acc_var_x:%.2f acc_var_y:%.2f acc_var_z:%.2f\n",acc_var_x_axis,acc_var_y_axis,acc_var_z_axis);
#ifndef KIST_V5_0729
	if(epoch == 1) {
		acc_norm_mean_step = acc_norm;
	} else if(epoch < averaging_win_size_step) {
		acc_norm_mean_step = acc_norm_mean_step * ((float)(epoch-1)/epoch) + acc_norm/epoch;
	} else {
		acc_norm_mean_step = acc_norm_mean_step * ((float)(averaging_win_size_step-1)/averaging_win_size_step) + acc_norm/averaging_win_size_step;
	}
#endif
#ifdef KIST_V5_0729
	if (epoch <= window_size_n) {
		acc_buffer_norm[epoch - 1] = acc_norm;
	} else {
		int i = 0;
		for (i = 0; i < window_size_n - 1; i++) {
			acc_buffer_norm[i] = acc_buffer_norm[i + 1];
		}
		acc_buffer_norm[window_size_n - 1] = acc_norm;
	}
	if (epoch <= window_size_z) {
		acc_buffer_z[epoch - 1] = z;
	} else {
		int i = 0;
		for (i = 0; i < window_size_z - 1; i++) {
			acc_buffer_z[i] = acc_buffer_z[i + 1];
		}
		acc_buffer_z[window_size_z - 1] = z;
	}
#else
	if (epoch <= window_size) {
		acc_buffer[epoch - 1] = acc_norm_mean_step;
	} else {
		int i = 0;
		for (i = 0; i < window_size - 1; i++) {
			acc_buffer[i] = acc_buffer[i + 1];
		}
		acc_buffer[window_size - 1] = acc_norm_mean_step;
	}
#endif

	#ifdef KIST_V54_0819
	if (epoch <= window_size_y) {
		acc_buffer_y[epoch - 1] = y;
	} else {
		int i = 0;
		for (i = 0; i < window_size_y - 1; i++) {
			acc_buffer_y[i] = acc_buffer_y[i + 1];
		}
		acc_buffer_y[window_size_y - 1] = y;
	}
#endif
	/*int j=0;
	for(j=0; j<window_size; j++) {
		printf("acc_buffer[%d]:%.2f\n",j,acc_buffer[j]);
	}
	printf("\n");*/
	
	#ifdef KIST_V55_0903 /* v1.2.4 V54 -> V55 hkim 2015.09.30 */
#if 0 /* v1.2.4  because of low memory :( hkim 2015.09.30 */
	if (epoch <= window_size_x) {
		acc_buffer_x[epoch - 1] = x;
	} else {
		int i = 0;
		for (i = 0; i < window_size_x - 1; i++) {
			acc_buffer_x[i] = acc_buffer_x[i + 1];
		}
		acc_buffer_x[window_size_x - 1] = x;
	}
#endif /* if 0 v1.2.4 */

#if 0 /* v1.2.4 hkim 2015.9.30*/
	if(epoch % y_save_period == 0) {
		int i = 0;
		for (i = 0; i < window_size_y - 1; i++) {
			acc_buffer_y_extend[i] = acc_buffer_y_extend[i + 1];
		}
		acc_buffer_y_extend[window_size_y - 1] = y;
	}
#else
	if(epoch % 5 == 0) { /* save period : 250msec */
		int i = 0;
		for (i = 0; i < window_size_y_extend - 1; i++) {
			acc_buffer_y_extend[i] = acc_buffer_y_extend[i + 1];
		}
		acc_buffer_y_extend[window_size_y_extend - 1] = y;
	}
#endif /* if 0 v1.2.4 */
#endif /* KIST_V55_0903 */
	
#ifdef KIST_V5_0729
	// step related
	if(epoch == 1) {
		acc_mean_y_axis_10 = y;
	} else if(epoch < step_y_axis_mean_winsize) {
		acc_mean_y_axis_10 = acc_mean_y_axis_10 * ((float) (epoch - 1) / epoch)
					+ (y / epoch);
	} else {
		acc_mean_y_axis_10 = acc_mean_y_axis_10 * ((float) (step_y_axis_mean_winsize - 1) / step_y_axis_mean_winsize)
					+ (y / step_y_axis_mean_winsize);
	}
	if(acc_mean_y_axis_10 > -3) {

		memcpy(acc_buffer, acc_buffer_norm, sizeof(acc_buffer_norm));
		upper_norm_thre_max = 23;
		upper_norm_thre_min = 12;
		max_step_frequency_thre = 22;
		min_step_frequency_thre = 5;
		lower_norm_thre_max = 12;
		window_size = window_size_n;
		upper_lower_time_diff_max = 16;
		swing_walking_flag = FALSE;
	} else {
		memcpy(acc_buffer, acc_buffer_z, sizeof(acc_buffer_z));
		upper_norm_thre_max = 15;
		upper_norm_thre_min = -2;
		max_step_frequency_thre = 35;
		min_step_frequency_thre = 4;
		lower_norm_thre_max = 2;
		window_size = window_size_z;
		upper_lower_time_diff_max = 32;
		swing_walking_flag = TRUE;
	}
#endif
	//printf("acc:%f\n",acc_buffer[0]);
	// 초기화
	//스탭 찾기
	upper_peak_occur = lower_peak_occur = swing_peak_occur = other_swing_peak_occur = FALSE;
#ifdef KIST_V5_0724
	previous_lower_peak_increase_time++;
	previous_upper_peak_increase_time++;
#else
	previous_lower_peak_time++;
#endif

	//07.06 추가
	//07.06 수정 끝
	int now = (window_size / 2) - 1;

#ifdef KIST_V5_0729

	int now_swing = (window_size_swing / 2) - 1;

#endif
	//start
	if (epoch > window_size) {
		// 피크 찾기(Upper peak)
		if (acc_buffer[now] > upper_norm_thre_min && acc_buffer[now] < upper_norm_thre_max) {
			if (acc_buffer[now] >= acc_buffer[now - 1]
							&& acc_buffer[now] > acc_buffer[now + 1]) {
				// 왼쪽 비교
				int i = 0;
				for (i = 0; i < window_size / 2 - 2; i++) {
					if (acc_buffer[now] < acc_buffer[i]) {
						upper_peak_occur = TRUE;
						break;
					}
				}
				//오른쪽 비교
				if (upper_peak_occur == FALSE) {
					for (i = 0; i < window_size / 2 - 1; i++) {
						if (acc_buffer[now] < acc_buffer[now + 2 + i]) { // 신범주 0722 now +1 -> now +2
							upper_peak_occur = TRUE;
							break;
						}
					}
				}
				//최종 판단
				if (upper_peak_occur == FALSE) {

#ifdef KIST_V5_0805 
				if ((peak_pos != 1) && (abs(acc_buffer[now] - previous_lower_peak_value) > upper_lower_peakvalue_diff)) {
#else
				if (peak_pos != 1) {
#endif
						temp_step_count++;
						//ks_printf("\r\n ts %d", temp_step_count); //hkim-test
						
#ifdef KIST_V5_0729
						if (temp_step_count <= continuous_step_occur_count) {
							step_count_epoch[temp_step_count - 1] = epoch;
						}else{
							int i = 0;
							for( i = 0;  i< continuous_step_occur_count-1 ; i++ ) {
								step_count_epoch[i] = step_count_epoch[i+1];
							}
							step_count_epoch[continuous_step_occur_count-1] = epoch;
						}
#else
						step_count_epoch[temp_step_count - 1] = epoch;
#endif

#ifdef KIST_V5_0729
						previous_upper_peak_increase_time = 0;
						peak_pos = 1;
#endif
						if (temp_step_count > continuous_step_occur_count-1) {
							int step_epoch = 0;
							for(step_epoch = 1 ; step_epoch < continuous_step_occur_count; step_epoch++) {
								if(step_count_epoch[step_epoch] - step_count_epoch[step_epoch-1] > max_step_frequency_thre
										|| step_count_epoch[step_epoch] - step_count_epoch[step_epoch-1] < min_step_frequency_thre) {
									continuous_step_flag = FALSE;
									break;
								} else {
									continuous_step_flag = TRUE;
								}
							}
						}

						if(previous_continuous_step_flag == FALSE && continuous_step_flag == TRUE) {
#ifdef KIST_V5_0729
							if(swing_walking_flag == TRUE) {
								step_count = step_count + 2*(continuous_step_occur_count-1);
								g_total_daystep = g_total_daystep + 2*(continuous_step_occur_count-1);
								g_accumulated_step = g_accumulated_step + 2*(continuous_step_occur_count-1); /* v1.3.9 hkim 2015.12.05 */
							} else {
								step_count = step_count + (continuous_step_occur_count-1);
								g_total_daystep = g_total_daystep + (continuous_step_occur_count-1);
								g_accumulated_step = g_accumulated_step + (continuous_step_occur_count-1); /* v1.3.9 hkim 2015.12.05 */
							}
#else
							step_count = step_count + (continuous_step_occur_count-1);
							g_total_daystep = g_total_daystep + (continuous_step_occur_count-1);
							g_accumulated_step = g_accumulated_step + (continuous_step_occur_count-1); /* v1.3.9 hkim 2015.12.05 */
#endif
						}
						if (continuous_step_flag == TRUE) {
#ifdef KIST_V5_0729
							if(swing_walking_flag == TRUE) {

								step_count += 2;
								g_total_daystep +=2;
								g_accumulated_step += 2; /* v1.3.9 hkim 2015.12.05 */
							} else {
								step_count++;
								g_total_daystep++;
								g_accumulated_step++; /* v1.3.9 hkim 2015.12.05 */
							}
#else
							step_count++;
							g_total_daystep++;
							g_accumulated_step++;  /* v1.3.9 hkim 2015.12.05 */
#endif
							float present_ampulitude = acc_buffer[now] - previous_lower_peak_value;
							float present_stepfreuency = 1/((epoch - previous_epoch) * (1.0/sen_data_frequency));
							//averaging
#if 0 /* v1.2.4  because of low memory :( hkim 2015.09.30 */
							if(step_count == 1) {
								ampulitude_rtsl = present_ampulitude;
								stepfrequency_rtsl = present_stepfreuency;
							} else if(step_count < averaging_win_size) {
								ampulitude_rtsl = (ampulitude_rtsl * ((float)(step_count-1)/step_count)) + (present_ampulitude/step_count); 
								stepfrequency_rtsl = (stepfrequency_rtsl * ((float)(step_count-1)/step_count)) + (present_stepfreuency/step_count); 
							} else {
								ampulitude_rtsl = (ampulitude_rtsl * ((float)(averaging_win_size-1)/averaging_win_size)) + (present_ampulitude/averaging_win_size);
								stepfrequency_rtsl = (stepfrequency_rtsl * ((float)(averaging_win_size-1)/averaging_win_size)) + (present_stepfreuency/averaging_win_size); 
							}
#endif /* if 0 */
							previous_epoch = epoch;
						}
						previous_continuous_step_flag = continuous_step_flag;
					}
				}
			}
		}
#ifndef KIST_V5_0729
#ifdef KIST_V5_0724
	}
#endif
#endif
		// 피크 찾기(lower peak)
#ifdef KIST_V5_0729
				if(acc_buffer[now] < lower_norm_thre_max) {// 닫을까?
#endif
		if (acc_buffer[now] <= acc_buffer[now - 1]
				&& acc_buffer[now] < acc_buffer[now + 1]) {
			// 왼쪽 비교
			int i = 0;
			for (i = 0; i < window_size / 2 - 2; i++) {
				if (acc_buffer[now] > acc_buffer[i]) {
					lower_peak_occur = TRUE;
					break;
				}
			}
			// 오른쪽 비교
			if (lower_peak_occur == FALSE) {
				for (i = 0; i < window_size / 2 - 1; i++) {
					if (acc_buffer[now] > acc_buffer[now + 2 + i]) { // 신범주 0722 now +1 -> now +2
						lower_peak_occur = TRUE;
						break;
					}
				}
			}
			// 최종 판단
#ifdef KIST_V5_0729
			if (lower_peak_occur == FALSE && peak_pos != 2) {
#else
#ifdef KIST_V5_0724
			if (lower_peak_occur == FALSE && previous_upper_peak_increase_time > upper_lower_time_diff_min) {
#else
			if (lower_peak_occur == FALSE) {
#endif
#endif
				previous_lower_peak_value = acc_buffer[now];
#ifdef KIST_V5_0724
				previous_lower_peak_increase_time = 0;
#else
				previous_lower_peak_time = 0;
#endif
#ifdef KIST_V5_0729
				peak_pos = 2;
#endif
			}
		}
#ifdef KIST_V5_0729
			}
#endif
		// swing peak 검출
#ifdef KIST_V5_0729
			if (acc_buffer_norm[now_swing] > jumping_rope_thre) {
				if (acc_buffer_norm[now_swing] >= acc_buffer_norm[now_swing - 1]
							&& acc_buffer_norm[now_swing] > acc_buffer_norm[now_swing + 1]) {
					// 왼쪽 비교
					int i = 0;
					for (i = 0; i < window_size_swing / 2 - 2; i++) {
						if (acc_buffer_norm[now_swing] < acc_buffer_norm[i]) {
							other_swing_peak_occur = TRUE;
							break;
						}
					}
					//오른쪽 비교
					if (other_swing_peak_occur == FALSE) {
						for (i = 0; i < window_size / 2 - 1; i++) {
							if (acc_buffer_norm[now_swing] < acc_buffer_norm[now_swing + 2 + i]) {// 신범주 0722 now +1 -> now +2
								other_swing_peak_occur = TRUE;
								break;
							}
						}
					}
					// 최종 판단
					if (other_swing_peak_occur == FALSE) {
						// 스윙 검출
						temp_swing_count++;

						if (temp_step_count <= continuous_swing_occur_count) {
							temp_swing_count_epoch_total[temp_swing_count - 1] = epoch;
						}else{
							int i = 0;
							for( i = 0;  i< continuous_swing_occur_count-1 ; i++ ) {
								temp_swing_count_epoch_total[i] = temp_swing_count_epoch_total[i+1];
							}
							#ifdef KIST_V5_0805
							temp_swing_count_epoch_total[continuous_swing_occur_count-1] = epoch;
							#else
							temp_swing_count_epoch_total[continuous_step_occur_count-1] = epoch;
							#endif

						}

						// large swing 검출
						if(acc_buffer_norm[now_swing] > large_swing_thre) {
#ifdef KIST_V54_0819
							// 골프 스윙 검출
#ifdef KIST_V55_0903 /* v1.2.4 V54 -> V55 hkim 2015.09.30 */
							y_extended_sum = 0;
							/* v1.2.4 'y_extended_avg_size+1' -> 'y_extended_avg_size' hkim 2015.09.30 */
							for (i=golf_avg_time_buffer_start; i < golf_avg_time_buffer_end+1; i++) { 
								y_extended_sum += acc_buffer_y_extend[i];
							}
							/* detect golf address */
							y_extended_avg = y_extended_sum / y_extended_avg_size;
							
#if 1 /* v1.2.4 hkim 2015.09.30 */	
							/* detect golf back-swing */
							for (i=golf_backswing_time_buffer_start; i < golf_backswing_time_buffer_end+1; i++) {
								if (acc_buffer_y_extend[i] > 0)  {
									y_backswing = 1;
									break;
								}
							}
							
							/* acc_buffer_z[0] match to acc_buffer_y[2] */
							if ((acc_buffer_y[2] + acc_buffer_y[3] < y_swing_front_sum_thre) && (acc_buffer_z[17] < 0)  &&	(y_extended_avg < y_extended_avg_thre) && (y_backswing == 1)) {
								// golf_swing_count 대신 large_swing_count 사용.
								large_swing_count++;
							}
							
#else /* if 1 v1.2.4 */
							if ( (acc_buffer_y[2] + acc_buffer_y[3] < y_swing_front_sum_thre) && (y_extended_avg < y_extended_avg_thre) )
						 {
								// golf_swing_count 대신 large_swing_count 사용.
								large_swing_count++;
							}
#endif /* if 1 v1.2.4 */
							

#else /* #ifdef KIST_V55_0903 */
							if(acc_buffer_y[2] + acc_buffer_y[3] < y_swing_front_sum_thre) {
								// golf_swing_count 대신 large_swing_count 사용.
								large_swing_count++;
							}
#endif 
#else
							large_swing_count++;
#endif
						}
						// small swing 검출
						if(acc_buffer_norm[now_swing] > small_swing_thre && acc_buffer_norm[now_swing] < large_swing_thre ) {
							small_swing_count++;
						}
#ifdef KIST_V5_0729
						if (temp_swing_count > continuous_swing_occur_count-1) {

							int swing_epoch = 0;

							for(swing_epoch = 1; swing_epoch < continuous_swing_occur_count; swing_epoch++) {

								if(temp_swing_count_epoch_total[swing_epoch] - temp_swing_count_epoch_total[swing_epoch-1] > max_swing_frequency_thre
										|| temp_swing_count_epoch_total[swing_epoch] - temp_swing_count_epoch_total[swing_epoch-1] < min_swing_frequency_thre) {
									continuous_swing_flag = FALSE;
									break;
								} else {
									continuous_swing_flag = TRUE;
								}
							}
						}
						if(previous_continuous_swing_flag == FALSE && continuous_swing_flag == TRUE) {
							jumping_rope_count = jumping_rope_count + (continuous_swing_occur_count-1);
							g_total_day_jumprope = jumping_rope_count + (continuous_swing_occur_count-1); /* v1.3.5 hkim 2015.10.28 */
						}
#else
						if(temp_swing_count > 2) {
							if(temp_swing_count_epoch_total[temp_swing_count-2] - temp_swing_count_epoch_total[temp_swing_count-3] < swing_epoch_diff_max // 신범주 0722 조건문 전부 -1 추가됨 .
									&& temp_swing_count_epoch_total[temp_swing_count-1] - temp_swing_count_epoch_total[temp_swing_count-2] < swing_epoch_diff_max) {
								if(continuous_swing_flag == FALSE) {
									continuous_swing_flag = TRUE;
									jumping_rope_count = jumping_rope_count + 2;
									g_total_day_jumprope =  jumping_rope_count + 2; /* v1.3.5 hkim 2015.10.28 */
								}
							} else {
								continuous_swing_flag = FALSE;
							}
						}
#endif
						if(continuous_swing_flag == TRUE) {
							jumping_rope_count++;
							g_total_day_jumprope++; /* v1.3.5 hkim 2015.10.28 */
						}
					}
#ifdef KIST_V5_0729
					previous_continuous_swing_flag = continuous_swing_flag;
#endif
				}
			}
		}
#else
		if (acc_buffer[now] > jumping_rope_thre) {
			if (acc_buffer[now] >= acc_buffer[now - 1]
				  && acc_buffer[now] > acc_buffer[now + 1]) {
				// 왼쪽 비교
				int i = 0;
				for (i = 0; i < window_size / 2 - 2; i++) {
					if (acc_buffer[now] < acc_buffer[i]) {
						other_swing_peak_occur = TRUE;
						break;
					}
				}
				//오른쪽 비교
				if (other_swing_peak_occur == FALSE) {
					for (i = 0; i < window_size / 2 - 1; i++) {
						if (acc_buffer[now] < acc_buffer[now + 2 + i]) {// 신범주 0722 now +1 -> now +2
							other_swing_peak_occur = TRUE;
							break;
						}
					}
				}
				// 최종 판단
				if (other_swing_peak_occur == FALSE) {
					// 스윙 검출
					temp_swing_count++;
					temp_swing_count_epoch_total[temp_swing_count - 1] = epoch;
					// large swing 검출
					if(acc_buffer[now] > large_swing_thre) {
						large_swing_count++;
					}
					// small swing 검출
					if(acc_buffer[now] > small_swing_thre && acc_buffer[now] < large_swing_thre ) {
						small_swing_count++;
					}
					if(temp_swing_count > 2) {
						if(temp_swing_count_epoch_total[temp_swing_count-2] - temp_swing_count_epoch_total[temp_swing_count-3] < swing_epoch_diff_max // 신범주 0722 조건문 전부 -1 추가됨 .
								&& temp_swing_count_epoch_total[temp_swing_count-1] - temp_swing_count_epoch_total[temp_swing_count-2] < swing_epoch_diff_max) {
							if(continuous_swing_flag == FALSE) {
								continuous_swing_flag = TRUE;
								jumping_rope_count = jumping_rope_count + 2;
								g_total_day_jumprope = jumping_rope_count + 2; /* v1.3.5 hkim 2015.10.28 */
							}
						} else {
							continuous_swing_flag = FALSE;
						}
					}
					if(continuous_swing_flag == TRUE) {
						jumping_rope_count++;
						g_total_day_jumprope++; /* v1.3.5 hkim 2015.10.28 */
					}
				}
			}
		}
	}
#endif
	// feature 결과 : 1분마다 대입.(1200개)
	if ((count%NUM_OF_DATA )==0)
	{
		struct feature_tbl_form_s data;
		tTime time;
		unsigned int gmt_flash=0;
		tTime stdTime; /* hkim 1.3.9.10,  2016.1.26 세계시간 동기화 */
#if __STEP_SYNC_TEST__
		read_step_info_from_flash(FLASH_ACCUMULATED_STEP_PAGE);
#endif
		data.norm_var = acc_var_norm*100000;
		#if 1 /* hkim Height 2015.6.5,  (2015.7.22)if 0 --> if 1*/
		data.pressure = g_pressure;
		#else
		data.pressure = (int)((double)pre_diff_sum*(double)100);
		#endif /* end of hkim */
		data.x_mean = (acc_mean_x_axis*1000);
		data.y_mean = acc_mean_y_axis*1000;
		data.z_mean = acc_mean_z_axis*1000;

		data.norm_mean = acc_norm_mean*1000;
		
		data.x_var = acc_var_x_axis*1000;
		data.y_var = acc_var_y_axis*1000;
		data.z_var = acc_var_z_axis*1000;
		data.nStep = step_count;
		data.jumping_rope_count = jumping_rope_count;
		data.small_swing_count		= small_swing_count;
		data.large_swing_count		= large_swing_count;
		data.pre_diff_sum		= pre_diff_sum*100;
#if 0 /* v1.2.4  because of low memory :( hkim 2015.09.30 */
		data.amp_rtsl			= ampulitude_rtsl*100;
		data.stepFreq_rtsl		= stepfrequency_rtsl*100;
#else
		//data.amp_rtsl			= 0;
		//data.stepFreq_rtsl		= 0;
#endif
#if 1 /* v1.3.1 hkim 2015.10.6 */
		data.hb = hr_avg_feature;
#endif /* v1.3.1 */

		data.battery = g_battery;		// only debug


	   //sys_synctime_get(&tyear, &tmon,&tday, &thour, &tmin, &tsec);
#if 0 /* 1.3.8 hkim 2015.11.23 */
		if(get_time( &time, 0))
#else
		if(get_time( &time, 1))
#endif
		{
			convert_gmt_to_display_time(&time, &stdTime);  /* hkim 1.3.9.10,  2016.1.26 세계시간 동기화 */
			//ks_printf("\r\n std Time =%d %d %d %d %d %d",stdTime.year, stdTime.month, stdTime.day, stdTime.hour, stdTime.minute, stdTime.second); 
			
#if 1 /* v1.3.9 hkim 2015.12.05 */
		#if __STEP_SYNC_TEST__
			if ( time.minute % 2 == 0 ) /* 2분 마다 */
		#else
			if ( (time.hour == 23 && time.minute == 58) || ( time.hour==23 && time.minute==59) )
		#endif /* __STEP_SYNC_TEST__ */
			{
				/* 당일의 총 걸음수와 누적걸음수를 플래쉬에 저장 */
				gmt_flash = get_GMT(time.year,  time.month,  time.day,  time.hour,  time.minute, time.second )/60;
		#if __STEP_SYNC_TEST__
				ks_printf("\r\n [TEST] 2 Min before: daytep %d , accu-step %d",g_total_daystep,g_accumulated_step);
				g_total_daystep = time.day + time.minute + time.second;
				g_accumulated_step += g_total_daystep;
				ks_printf("\r\n [TEST] 2 Min after   : datstep %d (%d + %d + %d), accu-step %d", g_total_daystep,time.day,time.minute,time.second,g_accumulated_step);
				ks_printf("\r\n [%d %d %d %d %d %d] save step to flash (day %d, accu %d)", time.year,  time.month,  time.day,  time.hour,  time.minute, time.second,g_total_daystep,g_accumulated_step);
		#endif /* __STEP_SYNC_TEST__ */
				save_step_info_to_flash(FLASH_ACCUMULATED_STEP_PAGE, gmt_flash, g_accumulated_step);
			}
#endif /* v1.3.9 */
	
			
#if 0 	/* hkim 1.3.9.10,  2016.1.26 세계시간 동기화 */		
			if( time.hour==0 && ( time.minute==0 || time.minute==1) )
#else
			/* GMT시간을 이용해 걸음수를 초기화 할 경우, 한국에서 아침 9시에 걸음수가 초기화 됨.
			    따라서 걸음수 초기화는 해당 국가의 표준시간으로 00:00 일 경우 초기화 한다 */
			if( stdTime.hour==0 && ( stdTime.minute==0 || stdTime.minute==1) )
#endif 
			{
				// 하루 지나면 reset
				g_total_daystep=0;
#if 1 /* v1.3.5 hkim 2015.10.28 */
				g_total_day_jumprope=0; 
#endif 
				temp_step_count=0; //hkim 0729
				temp_swing_count=0; //hkim 0729
			}
#if 1 /* v1.3.9 hkim 2015.12.05 */
			data.accumulated_step = g_accumulated_step;
#endif 
			data.display_step =g_total_daystep  ;		// only debug
			data.time = get_GMT(time.year,  time.month,  time.day,  time.hour,  time.minute, time.second )/60;

			if (dbg_feature_save == 1) {
				ks_printf("\r\ntime = %d %d %d %d %d %d => %d", time.year,  time.month,  time.day,  time.hour,  time.minute, time.second , data.time  );
				ks_printf("\r\nmean[%d %d %d] var[%d %d %d] norm[%d %d] p=%d step=%d [JR %d LS %d SW %d]  [total step : %d, %d] [%d] hb=%d",
					data.x_mean, data.y_mean, data.z_mean, 
					data.x_var, data.y_var, data.z_var, 
					data.norm_mean, data.norm_var, 
					data.pressure, data.nStep,  
					data.jumping_rope_count,  data.large_swing_count,  data.small_swing_count, 
					data.display_step, data.accumulated_step,
					data.pre_diff_sum,data.hb  );
			}
			
#if 1 /* v1.5.13.0 hkim 2016.3.23. writing period 1min */
			save_feature_data( &data , time.minute );
#else
			save_feature_data(&data);
#endif

#if 0 /* Never do following initialization hkim 0729 */
			temp_step_count = 0; 
			temp_swing_count = 0;
#endif 

			step_count = 0;
			large_swing_count = 0;
			small_swing_count = 0;
			jumping_rope_count = 0;
			pre_diff_sum = 0; /* hkim Height 2015.6.5 */
#ifdef KIST_V5_0805
#if 0 /* v1.2.4  because of low memory :( hkim 2015.09.30 */
			stepfrequency_rtsl = 0;
			ampulitude_rtsl = 0;
#endif /* if 0 */
#endif
			
		}
	}

	//printf("\r\nx =%f y=%f z =%f", x, y, z);
	//printf("\r\nstep=%d uD=%u p_l=%u", step_count, unDbg, previous_lower_peak_time);


	//ks_printf("\r\n%d",m_feature_10min[0].time);


	if(epoch > data_length_per_unittime*3500000) {
#if 0 /* v1.2.4  because of low memory :( hkim 2015.09.30 */
#ifdef KIST_V5_0724
			epoch = previous_lower_peak_time = ampulitude_rtsl = stepfrequency_rtsl = previous_lower_peak_increase_time = previous_upper_peak_increase_time = 0;
#else
			epoch = previous_lower_peak_time = ampulitude_rtsl = stepfrequency_rtsl = 0;
#endif
#else /* if 0 v1.2.4 */
#ifdef KIST_V5_0724
			epoch = previous_lower_peak_time =  previous_lower_peak_increase_time = previous_upper_peak_increase_time = 0;
#else
			epoch = previous_lower_peak_time = 0;
#endif
#endif /* if 0 */
	}

}

#if 1 /* v1.2.5 enable motor if hr is over than 80% of max hr. hkim 2015.10.3 */
extern unsigned short personal_max_hb;

int greencomm_hr_alarm(unsigned char __hr)
{
	unsigned int i=0,__loop=0;
	unsigned int hr_sum=0;
#if 1 /* v1.3.6 */
	unsigned int __sum=0;
#endif 
	
	/* 스마트폰으로 부터 사용자 정보가 수신되지 않으면 최고 심박수를 200 으로 임의로 설정함 */
#if 1 /* v1.3.3 hkim */
	if (g_personal_info.age == 255) personal_max_hb = 200;
#endif 

#if 1 /* v1.3.6 10분 이상의 슬립모드 상태에서 밴드를 착용하면 갑자기 높은 심박값이 읽혀 진동이 발생하는 경우가 발생함 */
	     /* 이를 방지위해 알람버퍼가 비어있을 경우 버퍼의 처음값을 70 으로 강제 설정 */
	for (i = 0; i < HR_BUF_DEF; i++) {
		__sum += hr_alarm_buffer[i];
	}
	if (__sum == 0) {
		//ks_printf("\r\n All Zero HR save %d to first buffer !!!", HR_BUF_FIRST_FIX_HR);
		hr_alarm_buffer[0] = HR_BUF_FIRST_FIX_HR; 
		hr_alarm_count++;
	}
#endif /* v1.3.6 */

	/* 스마트폰에서의 심박수 요청에 대해 밴드가 응답하는 시점부터 심박수를 버퍼에 저장한다 */
	hr_alarm_count++;
		
	if (hr_alarm_count <= HR_BUF_DEF) 
	{
		hr_alarm_buffer[hr_alarm_count - 1] = __hr;
	} 
	else 
	{
				
		for(i = 0; i < HR_BUF_DEF - 1; i++) {
			hr_alarm_buffer[i] = hr_alarm_buffer[i + 1];
		}
				
		hr_alarm_buffer[HR_BUF_DEF - 1] = g_sort_hb;
	}


#if 0 /* v1.3.3 hkim 2015.10.14 */
	ks_printf("\r\n HR : %d %d %d %d %d     %d %d %d %d %d",
							hr_alarm_buffer[0], hr_alarm_buffer[1], hr_alarm_buffer[2], hr_alarm_buffer[3], hr_alarm_buffer[4], 
							hr_alarm_buffer[5], hr_alarm_buffer[6], hr_alarm_buffer[7], hr_alarm_buffer[8], hr_alarm_buffer[9]);
#endif 
	for (i = 0; i < HR_BUF_DEF; i++) {
		if (hr_alarm_buffer[i] != 0) {
			hr_sum += hr_alarm_buffer[i];
			__loop += 1;
		}
	}

	hr_avg = hr_sum / __loop;
	
	if (dbg_hr_average == 1) /* v1.3.9 hkim 2015.12.05 */
	{
		ks_printf("\r\n personal max hr : %d", personal_max_hb);
		ks_printf("\r\n HR buffer : count %d, sum %d, avg : %d",__loop, hr_sum, hr_avg);
	}

	/* enable motor if hr_avg is over than 80% of maximum hr */
#if 0 /* v1.3.7 hkim 2015.11.11 움직임이 없으면 심박수가 높더라도 진동 울리지 않음.  g_double_move_signal=1 : 움직임 있음 */
	if (hr_avg > personal_max_hb) {
#else
	if ( (hr_avg > personal_max_hb) && (g_double_move_signal == 1) ) {
#endif 
		nrf_gpio_pin_set(MOTOR_ENABLE_PIN);
		g_personal_max_hb_motor = 1;
		g_personal_max_hb_pretime = g_time;
	}

	return 0;
}
#endif /* if 1 v1.2.5 */

#endif /* #if __GREENCOM_24H__ */

/* ----------------------------------------------------------------------------------------------*/
// by Skim,  2015.4.6
/*-----------------------------------------------------------------------------------------------*/

// max page 65536
// sector => 4K
// page => 256byte



void find_first_emtpy_page(void)
{
	
	unsigned char buf[260];
	int i;
	

	for(i=0;i< MAX_DB_PAGE /* 65536 */;i=i+1)
	{
		
		SPI_FLASH_BufferRead(buf, i<<8 );

		//ks_printf("\r\n%d: %x %x %x %x",i, buf[4],buf[5],buf[6],buf[7]  ); 

		if(buf[4]==0xff && buf[5]==0xff && buf[6]==0xff && buf[7]==0xff)
		{
			m_db_write_page=i;
			//m_db_read_page=i;

			ks_printf("\r\nfirst empty page = 0x%x",m_db_write_page);
			break;
		}
	}
	
}


void set_day_step(void)
{
	unsigned short  last_page;
	struct feature_tbl_form_s load_data[5];
	tTime time, stime;
#if 1 /* v1.3.9 hkim 2015.12.05 */
	char flash_str[256];
	
	memset(flash_str,0,sizeof(flash_str));
#endif /* v1.3.9 */
#if __STEP_SYNC_TEST__
	ks_printf("\r\n m_db_write_page %d",m_db_write_page);
#endif 
	if(m_db_write_page==0)
		last_page = MAX_DB_PAGE -1;
	else
		last_page = m_db_write_page-1;
#if __STEP_SYNC_TEST__
	ks_printf("\r\n m_db_write_page %d, last_page %d",m_db_write_page,last_page);
#endif 

	feature_flash_read(last_page, load_data );

#if __STEP_SYNC_TEST
	ks_printf("\r\n display_step %d", load_data[4].display_step);
#endif 
	if( load_data[4].display_step > 50000)
	{
		ks_printf("\r\n invalid saved daystep %d",load_data[4].display_step);
	}
	else
	{
#if 0 /* v1.3.8 hkim 2015.11.23 */
		if ( is_rtc_valid() && get_time( &time, 0) )
#else
		if (get_time( &time, 1) )
#endif
		{
			get_time_from_GMT(load_data[4].time*60, &stime  );
#if __STEP_SYNC_TEST			
			ks_printf("\r\n(now/feature) year %d %d, month %d %d, day %d %d, hour %d %d, min %d %d, sec %d %d", 
												time.year, stime.year, time.month, stime.month, time.day, stime.day, time.hour, stime.hour, time.minute, stime.minute, time.second, stime.second);
#endif 
			if(stime.month==time.month &&  stime.year==time.year &&   stime.day==time.day )
			{
				g_total_daystep = load_data[4].display_step;
				ks_printf("\r\nrecovered daystep = %d", g_total_daystep);
			}
			else
				ks_printf("\r\n[time inconsistency]g_total_daystep = %d", g_total_daystep);
		}
#if 1 /* v1.3.9 hkim 2015.12.05 */
		/* 누적 걸음수 복원 */
		g_accumulated_step = load_data[4].accumulated_step;
		
		ks_printf("\r\nrecovered accumulated_step = %d",g_accumulated_step);
#endif /* v1.3.9 */
	}
}

#if 0 /* v1.5.13.0 hkim 2016.3.23.  fill empty feature data */
void save_feature_data(struct feature_tbl_form_s* pdata, unsigned char min)
{
	//unsigned short temp;
	
#if 1
	if(m_feature_data_count==0)
	{
		if( (min%5)!=0 )
		{
			if (dbg_feature_save) /* v1.4.10.0 */
				ks_printf("\r\nskip save");
			return;			
		}
	}
#endif

	
	memcpy( &m_feature_10min[m_feature_data_count], pdata, sizeof(struct feature_tbl_form_s));

	//ks_printf("\r\nsave[%d] %d %d %d %d %d",m_feature_data_count, m_feature_10min[0].time ,m_feature_10min[1].time,m_feature_10min[2].time,m_feature_10min[3].time,m_feature_10min[4].time ); 

	m_feature_data_count++;

	if(m_feature_data_count>=5)
	{
		//temp=m_db_write_page+1;

		//if(temp!=m_db_read_page)
		{
			feature_flash_fwrite(m_db_write_page, m_feature_10min );

			// 임시 debug print
			//ks_printf("\r\nafter= %d %d %d %d %d",m_feature_10min[0].time ,m_feature_10min[1].time,m_feature_10min[2].time,m_feature_10min[3].time,m_feature_10min[4].time ); 
		
			m_feature_data_count=0;
			m_db_write_page++;
			if(  m_db_write_page >= MAX_DB_PAGE)
				m_db_write_page=0;
		}
		//else
		//	ks_printf("\r\nflash full");
		
	}
	else if(m_feature_data_count==1)
	{
		if(m_db_write_page%16==0) /* hkim next page가 섹터의 시작이라면 섹터를 미리 삭제한다 */
		{
			erase_feature_data(0);
		}	
	}
	else if(m_feature_data_count==2)
	{
		if(m_db_write_page%16==0)
		{
			erase_feature_data(1);
		}
	}
		
}
#else /* v1.5.13.0 hkim 2016.3.23.  fill empty feature data */
//int check_feature_empty(struct feature_tbl_form_s* pdata)
#if 0
void copy_last_feature()
{
		if (m_db_write_page != 0)
			feature_flash_read(m_db_write_page-1, backup_feature_5min );	
		
		ks_printf("\r\ncopy last feature. time (%d %d %d %d %d)", 
															backup_feature_5min[0].time, backup_feature_5min[1].time,
															backup_feature_5min[2].time, backup_feature_5min[3].time,
															backup_feature_5min[4].time);
}
#endif 


int check_feature_empty()
{
	//struct feature_tbl_form_s feature_data[FEATURE_SAVE_PERIOD_MIN];
	unsigned int time_diff=0;
		
	if(m_db_write_page==0)
		return false;

	memset(last_feature_5min,0,sizeof(last_feature_5min));
	
	/* 가장 마지막에 저장된 피쳐데이타를 읽음 */
	if (dbg_feature_save == 1) {
		ks_printf("\r\n Check feature empty : standby page %d, last page %d", m_db_write_page, m_db_write_page-1);
	}
	feature_flash_read(m_db_write_page-1, last_feature_5min );	
	
	time_diff = m_feature_10min[0].time - last_feature_5min[0].time;
	
	if (dbg_feature_save == 1) {
		ks_printf("\r\n check feature empty : now(%d) - before(%d) = %d", 
																														m_feature_10min[0].time, last_feature_5min[0].time, time_diff);
	}

	/* 비정상적 리셋이 발생하면 현재 저장예정인 피쳐와 마지막 저장된 피쳐의 시간차는 최대 10분
	    만일 방전이 아님에도 10분 이상 차이가 난다면 비정상적인 리셋이 5분간격으로 2회 발생한것임
	    (예: 0 ~4분 1회, 5분 ~ 9분 : 또 1회 발생) */
	if (time_diff == FEATURE_SAVE_PERIOD_MIN)
	{
		return false;
	}
	else if ( time_diff > FEATURE_SAVE_PERIOD_MIN && time_diff <= SYS_POWER_OFF_MIN)
	{
		return true;
	}
	else if ( time_diff > SYS_POWER_OFF_MIN) /* 방전 */
	{
		if (dbg_feature_save == 1)
			ks_printf("\r\n !!!!!!! Power down over %d minute. No copy feature", SYS_POWER_OFF_MIN);
		return false;
	}
	else {}
	
#if 0	
	if ( time_diff > FEATURE_SAVE_PERIOD_MIN)
	{
		empty_5min_feature_num = time_diff / FEATURE_SAVE_PERIOD_MIN; /* 5분짜리 몇개를 채워야 하는지 알려줌 */
		
		if (dbg_feature_save == 1)
			ks_printf("\r\n Empty number(5min unit) : %d", empty_5min_feature_num);
		
		return true;
	}
#endif
		
	return false;
}

void save_feature_data_to_flash(int now)
{
	unsigned char i=0;
	
	if (now == 1) /* save current feature */
	{
		if (dbg_feature_save == 1)
			ks_printf("\r\n save feature(current, page %d) %d %d %d %d %d",
																																					m_db_write_page, m_feature_10min[0].time ,
																																					m_feature_10min[1].time,m_feature_10min[2].time,
																																					m_feature_10min[3].time,m_feature_10min[4].time ); 
			
		feature_flash_fwrite(m_db_write_page, m_feature_10min );
		
		m_db_write_page++;
	}
	else /* save copied feature */
	{
		if (dbg_feature_save == 1)
			ks_printf("\r\n save feature(copy, page %d) %d %d %d %d %d",
																																					m_db_write_page, last_feature_5min[0].time ,
																																					last_feature_5min[1].time,last_feature_5min[2].time,
																																					last_feature_5min[3].time,last_feature_5min[4].time ); 
#if 0
		for (i=0; i < empty_5min_feature_num; i++)
		{
			feature_flash_fwrite(m_db_write_page+i, last_feature_5min );
			m_db_write_page += (i+1);
		}
#else
		feature_flash_fwrite(m_db_write_page+i, last_feature_5min );
		m_db_write_page++;
#endif
	}
				
	if(  m_db_write_page >= MAX_DB_PAGE)
		m_db_write_page=0;
}


/* 피쳐를 저장하기 전에 필요한 작업을 분산처리 방식으로 미리 해놓는다 */
int handle_mem_feature_count(struct feature_tbl_form_s* pdata, unsigned char min)
{
	int i=0;

	switch(m_feature_data_count)
	{
		case 0:
			if( (min%5) !=0 )
			{
				if (dbg_feature_save) /* v1.4.10.0 */
					ks_printf("\r\nskip save");
				
				return false;
			}
			break;
			
		case 1:
			if(m_db_write_page%PAGE_NUM_IN_SECTOR ==0)
			{
				if (dbg_feature_save == 1) {	
					ks_printf("\r\n m_feature_data_count=1, erase sector-0&1");
				}
				erase_feature_data(0);
				erase_feature_data(1);
			}
			break;
			
		case 2:
			if (check_feature_empty() == true)
			{
				if (dbg_feature_save == 1) {	
					ks_printf("\r\n m_feature_data_count=2, feature not exist");
				}
				previous_feature_not_exist = 1;
			}
			break;
			
		case 3:
			if (previous_feature_not_exist)
			{
				if (dbg_feature_save == 1) {
					ks_printf("\r\n m_feature_data_count=3, copy feature");
				}
				
				for (i=0; i < FEATURE_SAVE_PERIOD_MIN; i++) {
					last_feature_5min[i].time += FEATURE_SAVE_PERIOD_MIN;
				}
				
				save_feature_data_to_flash(0);
				previous_feature_not_exist = 0;

				/* 현시점에 저장될 피쳐데이타를 위해 플래쉬를 미리 삭제해 준다 */
				if(m_db_write_page%PAGE_NUM_IN_SECTOR ==0)
				{
					erase_feature_data(0);
					erase_feature_data(1);
				}
			}
			break;
			
		deafult:
			break;
	}
	return true;
}

/* 5분 피쳐의 시간이 연속적인지 검사하고 수정 */
/* 테스트 결과 5분 피쳐의 시간이  다음과 같이 저장되는 경우가 있음
    10 11 12 13 15

    연속되지 않은 시간을 포함한 피쳐를 미들웨어에 응답하면 미들웨어는 동일한 시간으로 
    피쳐데이타를 달라고 다시 요청함. 이과정이 무한 반복됨. 
*/
void check_feature_time_sequence(void)
{
	int i=0;
	unsigned int initial_time=0;
	
	initial_time = m_feature_10min[i].time;
	
	if (dbg_feature_save == 1) {
		ks_printf("\r\n check feature seq : %d %d %d %d %d(initial time %d)", 
										m_feature_10min[0].time, m_feature_10min[1].time, m_feature_10min[2].time, 
										m_feature_10min[3].time, m_feature_10min[4].time, initial_time); /* hkim-debug */
	}
	
	for (i=0; i < FEATURE_SAVE_PERIOD_MIN; i++)
	{
		if ( i == 0)	continue;
		
		if ( (m_feature_10min[i].time) != (initial_time+i) )
		{
			if (dbg_feature_save == 1) {
				ks_printf("\r\n modify feature time : %d --> %d", m_feature_10min[i].time, (initial_time+i));
			}
			m_feature_10min[i].time = initial_time+i;
		}
	}
}

/* 피쳐데이타는 FEATURE_SAVE_PERIOD_MIN 만큼 메모리에 저장한 후 한꺼번에 플래쉬에 write 한다 */
int save_feature_data(struct feature_tbl_form_s* pdata, unsigned char min)
{
	int i=0;
	
	if (dbg_feature_save == 1) {
		ks_printf("\r\n Input min : %d, min%5 = %d", min, min%5);
	}
	
	if (handle_mem_feature_count(pdata,min) == false)
		return false;
	
	memcpy( &m_feature_10min[m_feature_data_count], pdata, sizeof(struct feature_tbl_form_s));
	#if 1 /* hkim-debug */
	if (dbg_feature_save == 1) {
		ks_printf("\r\ncurrent m_feature_data(count %d) : %d %d %d %d %d",	
																																							m_feature_data_count, 
																																							m_feature_10min[0].time ,
																																							m_feature_10min[1].time,m_feature_10min[2].time,
																																							m_feature_10min[3].time,m_feature_10min[4].time ); 
	}
	#endif 
	
	/* hkim-test */
	#if 0 
	m_feature_10min[1].time = m_feature_10min[1].time -1;
	m_feature_10min[2].time = m_feature_10min[2].time -2;
	m_feature_10min[3].time = m_feature_10min[3].time +2;
	m_feature_10min[4].time +=1 ;
	#endif 
	/* end of hkim-test */

	m_feature_data_count++;
	if(m_feature_data_count>=FEATURE_SAVE_PERIOD_MIN)
	{
		check_feature_time_sequence(); 
		if (dbg_feature_save == 1) {		
			ks_printf("\r\n save feature to flash!");
		}
		save_feature_data_to_flash(1);
		#if 0 
		memcpy(&backup_feature_5min, 
									&m_feature_10min, 
									sizeof(struct feature_tbl_form_s)*FEATURE_SAVE_PERIOD_MIN);
		#endif 
		
		if (dbg_feature_save == 1) {		
			ks_printf("\r\n initialize m_feature_data_count & m_feature_10min");
		}
		m_feature_data_count=0;
		
		#if 1 /* v1.5.17.0 hkim 2016.5.15 */
		for (i=0; i < FEATURE_SAVE_PERIOD_MIN; i++)
			memset(&m_feature_10min[i], 0, sizeof(struct feature_tbl_form_s));
		#endif 

	}
	return true;
}
#endif /* v1.5.13.0 hkim 2016.3.23. fill empty feature data */

/*
int load_feature_data(struct feature_tbl_form_s* pdata)
{
	struct feature_tbl_form_s feature_data[5];


	if(m_db_write_page==m_db_read_page)
	{
		return false;
	}
	

	feature_flash_read(m_db_read_page, feature_data );

	memcpy( pdata, feature_data, sizeof(struct feature_tbl_form_s)*5);

	m_db_read_page++;

	return true;
}
*/

#if __GREENCOM_24H__ /* v1.4.12.2 hkim 2016.3.10 */
extern unsigned char _uart_debug;
//extern unsigned char _uart_debug_tx;

#if 0 /* v1.5.13.0 hkim 2016.5.4 */
unsigned short get_feature_data_page_from_top(int gmt_min, unsigned int* time)
{
	struct feature_tbl_form_s feature_data[FEATURE_NUM_IN_FLASH_PAGE];
	unsigned short read_page=0;

	*time = 0xffffffff;

	for(read_page=0;read_page<(MAX_DB_PAGE-1);read_page++) 
	{
		feature_flash_read(read_page, feature_data );	
		//ks_printf("\r\n read flash from top (page %x(hex), time %d(%x))", read_page, feature_data[0].time, feature_data[0].time);
		
		if(feature_data[0].time == 0xffffffff)
			return false;
		
		if( feature_data[0].time == gmt_min )
		{
			*time= feature_data[0].time;
			if (_uart_debug)
				ks_printf("\r\nmatching read_page = %d(%02x)  time=%d  gmt_min %d",read_page, read_page, feature_data[0].time, gmt_min );
			
			return read_page;
		}
	}
	return false;
}
#else  /* v1.5.13.0 hkim 2016.5.4 */
int get_feature_data_page_from_top(uint32_t s_gmt, uint32_t e_gmt, unsigned short* s_page, unsigned short* e_page, unsigned int* out_s_gmt, unsigned int *out_e_gmt)
{
	unsigned char find_s_page=0, find_e_page=0;
	struct feature_tbl_form_s feature_data[FEATURE_NUM_IN_FLASH_PAGE];
	unsigned short read_page=0;
	uint32_t __curr_page_gmt = 0;

	*s_page = 0xffff;
	*e_page = 0xffff;
	
	*out_s_gmt = 0xffffffff;
	*out_e_gmt = 0xffffffff;

	for(read_page=0;read_page<(MAX_DB_PAGE-1);read_page++) 
	{
		if ( (find_s_page == 1) && (find_e_page == 1) )
			return true;
		
		feature_flash_read(read_page, feature_data );	
		__curr_page_gmt = feature_data[0].time;

#if 0 		
		if (_uart_debug)
			ks_printf("\r\n ***** read flash from top (page %x(hex), time %d(%x), %d(%x),%d(%x),%d(%x),%d(%x),)", 
																read_page, feature_data[0].time, feature_data[0].time,
																feature_data[1].time, feature_data[1].time,
																feature_data[2].time, feature_data[2].time,
																feature_data[3].time, feature_data[3].time,
																feature_data[4].time, feature_data[4].time );
#endif 

#if 0 /* v1.5.17.0 hkim 2016.5.17 */		
		if(feature_data[0].time == 0xffffffff)
			return false;
#endif 
		
#if 0 /* v1.5.17.0 hkim 2016.5.18 잘 동작한다 하지만 시작/종료 날짜가 정확하게 일치해야 하는 단점이 있다 */
		if ( find_s_page != 1)
		{
			if ( 	(feature_data[0].time == s_gmt ) || (feature_data[1].time == s_gmt )|| (feature_data[2].time == s_gmt ) || 
					  (feature_data[3].time == s_gmt ) || (feature_data[4].time == s_gmt ) )
			{
					*s_page = read_page;
					*out_s_gmt= feature_data[0].time;
				
					find_s_page = 1;
				
					if (_uart_debug)
						ks_printf("\r\n[start page] matching read_page = %d(%02x)  page_start_time %d",*s_page, *s_page, *out_s_gmt );
			}
		}
		
		if ( find_e_page != 1)
		{
			if ( 	(feature_data[0].time == e_gmt ) || (feature_data[1].time == e_gmt )|| (feature_data[2].time == e_gmt ) || 
						(feature_data[3].time == e_gmt ) || (feature_data[4].time == e_gmt ) )
			{	
					*e_page = read_page;
					*out_e_gmt= feature_data[0].time;
				
					find_e_page = 1;
				
					if (_uart_debug)
						ks_printf("\r\n[end page] matching read_page = %d(%02x)  page_start_time %d",*e_page, *e_page, *out_e_gmt );
			}
		}
#else /* v1.5.17.0 */
		if ( find_s_page != 1)
		{
			if (__curr_page_gmt == s_gmt)
			{
				*s_page 			= read_page;
				*out_s_gmt	= __curr_page_gmt;
				
				find_s_page = 1;
				
				if (_uart_debug)
					ks_printf("\r\n[search start page] find matching page = %d(%02x)  final GMT  %d",*s_page, *s_page, *out_s_gmt );
			}
			else if ( (__curr_page_gmt > s_gmt) && (__curr_page_gmt < *out_s_gmt) && (__curr_page_gmt < e_gmt) )
			{
				*s_page 			= read_page;
				*out_s_gmt	= __curr_page_gmt;
				
				if (_uart_debug)
					ks_printf("\r\n[search start page] output start gmt changed to %d ", *out_s_gmt);
			}
			else
			{
			}
		}
		
		if ( find_e_page != 1)
		{
			if (__curr_page_gmt == e_gmt)
			{
				*e_page 			= read_page;
				*out_e_gmt	= __curr_page_gmt;
				
				find_e_page = 1;
				
				if (_uart_debug)
					ks_printf("\r\n[search end page] find matching page = %d(%02x)  final GMT  %d",*e_page, *e_page, *out_e_gmt );
			}
			else if ( (__curr_page_gmt  < e_gmt) && (*out_e_gmt == 0xffffffff) )
			{
				*e_page 			= read_page;
				*out_e_gmt	= __curr_page_gmt;
			}
			else if ( (__curr_page_gmt  < e_gmt) && (__curr_page_gmt > *out_e_gmt) && (__curr_page_gmt > s_gmt) )
			{
				*e_page 			= read_page;
				*out_e_gmt	= __curr_page_gmt;
				
				if (_uart_debug)
					ks_printf("\r\n[search end page] output end gmt changed to %d ", *out_e_gmt);
			}
			else
			{
			}
		}
#endif /* v1.5.17.0 */
	} /* for */
	
	if (_uart_debug)
	{
		ks_printf("\r\n*** search result ***");
		ks_printf("\r\n start : page %d(0x%x), GMT %d", *s_page, *s_page, *out_s_gmt);
		ks_printf("\r\n end  : page %d(0x%x), GMT %d", *e_page, *e_page, *out_e_gmt);
	}
	
}
#endif  /* v1.5.13.0 hkim 2016.5.4 */

unsigned short get_feature_data_page(int gmt_min, unsigned int* time)
{
	struct feature_tbl_form_s feature_data[5];
	unsigned short read_count=0;
	short read_page;

	*time = 0xffffffff;

	for(read_count=1;read_count<(MAX_DB_PAGE-1);read_count++) /* v1.4.11.1 hkim 2016.3.1 */
	{
		if (_uart_debug)
				ks_printf("\r\n***************start %d(last page(hex) %x)", read_count, m_db_write_page);
		read_page = m_db_write_page-read_count;
		if(read_page<0)
			read_page=read_page+MAX_DB_PAGE;
		
		feature_flash_read(read_page, feature_data );	
		
		if (_uart_debug)
			ks_printf("\r\n read_page=%x(hex)  time=%d", read_page, feature_data[0].time);

		if(feature_data[0].time==0xffffffff)
		{
			if (_uart_debug)
				ks_printf("\r\n no data,  read_page++");
			read_page++;
			if(read_page==MAX_DB_PAGE)
				read_page=0;
			return read_page;
		}

		*time= feature_data[0].time;
		
		if( feature_data[0].time <= gmt_min )
		{
			if (_uart_debug)
				ks_printf("\r\nmatching read_page = %d(%02x)  time=%d gmt_min %d",read_page, read_page, feature_data[0].time, gmt_min );

			if(feature_data[0].time<gmt_min)
			{
				//if (_uart_debug)
					//ks_printf("\r\nread_page++");
				if (is_uart_mode == false)  /* v1.4.11.1 hkim 2016.3.1 */
				{
						/* v1.4.11.1 hkim 2016.02.17. 김섭차장(기억나지 않으나 중요한 이유로 그 다음 페이지를 리턴함  */
						read_page++;
				}
				else
				{
						/* 찿은 페이지가 빈 페이지의 바로 직전일 경우, 
								m_db_write_page 를 리턴하게 되어 에러로 처리된다 */
						if (is_uart_mode == false)
							read_page++;
				}
				
				if(read_page==MAX_DB_PAGE)
					read_page=0;				
			}
			
			return read_page;
		}

	}

	return 0;
}
#endif /* __GREENCOM_24H__ */
/*
unsigned short get_queued_number(void)
{

	return m_db_write_page-m_db_read_page;
}

unsigned short get_read_page_pointer(void)
{

	return m_db_read_page;
}
*/

unsigned short get_write_page_pointer(void)
{

	return m_db_write_page;
}



int erase_feature_data(int flag)
{
	unsigned char buf[256];
	volatile unsigned short	temp_page;

	if(flag==0)
	{
		feature_flash_read(m_db_write_page, buf );

		//ks_printf("\r\ntry erase %d",m_db_write_page );
	
		if(buf[0]==0xff && buf[1]==0xff && buf[2]==0xff && buf[3]==0xff)
			return false;

		//ks_printf("\r\nerase %d",m_db_write_page );
		feature_flash_erase(m_db_write_page);
	}
	else
	{
		temp_page=m_db_write_page+16;
		if(temp_page>=MAX_DB_PAGE)
			temp_page=0;

		//ks_printf("\r\ntry2 erase %d",temp_page );

		feature_flash_read(temp_page, buf );
		if(buf[0]==0xff && buf[1]==0xff && buf[2]==0xff && buf[3]==0xff)
			return false;

		//ks_printf("\r\nerase %d",temp_page );
		feature_flash_erase(temp_page);
	}
	
	return true;
}

void feature_flash_fwrite(unsigned short  page, void* data )
{
	unsigned char buf[260];
	unsigned int address;
	
	memset(buf,0xff,sizeof(buf));

	address = page<<8;
	memcpy(buf+4, data, sizeof(struct feature_tbl_form_s)*5 );

	//ks_printf("\r\n feature write  0x%x",address);
	
	SPI_FLASH_PageWrite(buf, address );
}


void feature_flash_read(unsigned short  page, void* data )
{
	unsigned char buf[260];
	unsigned int address;

	address = page<<8;
	SPI_FLASH_BufferRead(buf, address );
	memcpy(data,buf+4,sizeof(struct feature_tbl_form_s)*5 );
}



void feature_flash_erase(unsigned short  page)
{
	unsigned int address;

	address = page<<8;
	SPI_FLASH_SectorErase(address );	
}



void check_flash_db_data(char* string)
{
	char	flash_str[256];

	
	feature_flash_read( FLASH_COMPILE_VERSION_PAGE, flash_str);

	flash_str[100]=0;	// string 길이 제한 

	if(strcmp(string,flash_str )==0)
	{
		ks_printf("\r\nFlash Compile Version : OK (%s, %s)", string,flash_str);
		return;
	}

	memset(flash_str, 0xff, 256);
	strcpy(flash_str,string  );

	feature_flash_erase(FLASH_COMPILE_VERSION_PAGE);
	nrf_delay_ms(200);
	feature_flash_fwrite(  FLASH_COMPILE_VERSION_PAGE,  flash_str );
	nrf_delay_ms(100);
	ks_printf("\r\nFlash Compile Version : WRITE (%s, %s)", string,flash_str);
	feature_flash_erase(0);
	nrf_delay_ms(200);
	ks_printf("\r\nErase Page0");
	feature_flash_erase(MAX_DB_PAGE-1);
	nrf_delay_ms(200);
	ks_printf("\r\nErase Page %d",MAX_DB_PAGE-1 );

}

int Flash_Read_Text( int page, char* flash_str)
{

	feature_flash_read(  page,  flash_str );
	
	flash_str[255]=0;	// 길이 제한 
	if(flash_str[0]==0xff)
	{
		// empty
		return false;
	}
		
	return true;	
}


int Flash_Write_Text( int page, char* flash_str)
{

	feature_flash_erase(page);
	nrf_delay_ms(200);
	feature_flash_fwrite(  page,  flash_str );
	nrf_delay_ms(100);
	
	return true;
}

#if 1 /* v1.3.9 hkim 2015.12.05 */
void step_flash_write(unsigned short  page, void* data )
{
	unsigned char buf[260];
	unsigned int address;
	
	memset(buf,0xff,sizeof(buf));
	
	address = page<<8;
	memcpy(buf+4, data, sizeof(struct flash_step_info)*MAX_FLASH_STEP_INFO );

	SPI_FLASH_PageWrite(buf, address );
}

void step_flash_read(unsigned short  page, void* data )
{
	unsigned char buf[260];
	unsigned int address;

	address = page<<8;
	SPI_FLASH_BufferRead(buf, address );
	memcpy(data,buf+4,sizeof(struct flash_step_info) * MAX_FLASH_STEP_INFO);
}

void read_step_info_from_flash(unsigned int address)
{
	tTime time;
	struct flash_step_info load_data[MAX_FLASH_STEP_INFO];
	int i=0;
	
	memset(load_data,0,sizeof(load_data));
	
	step_flash_read( address, load_data);
	
	switch(address)
	{
		case FLASH_TODAY_TOTAL_STEP_PAGE:
			break;
		
		case FLASH_ACCUMULATED_STEP_PAGE:
				for (i=0; i < MAX_FLASH_STEP_INFO; i++) 
			{
				get_time_from_GMT(load_data[i].time*60, &time  );
				ks_printf("\r\n[%d] [%d %d %d %d %d %d] step %d",
															i, time.year,  time.month,  time.day,  time.hour,  time.minute, time.second, load_data[i].step);
			}
			break;
			
		default:
			break;
		
	}
}

int get_step_info_from_flash(unsigned int address, unsigned int gmt,struct flash_step_info *step_info)
{
	tTime in_time, out_time;
	struct flash_step_info load_data[MAX_FLASH_STEP_INFO];
	int i=0;
	
	memset(load_data,0,sizeof(load_data));
	
	get_time_from_GMT(gmt*60, &in_time  );
	ks_printf("\r\n In_time [%d %d %d %d %d %d]",
															in_time.year,  in_time.month,  in_time.day,  in_time.hour,  in_time.minute, in_time.second);
	
	step_flash_read( address, load_data);
	
	switch(address)
	{
		case FLASH_TODAY_TOTAL_STEP_PAGE:
			break;
		
		case FLASH_ACCUMULATED_STEP_PAGE:
				for (i=0; i < MAX_FLASH_STEP_INFO; i++) 
			{
				get_time_from_GMT(load_data[i].time*60, &out_time  );
				if (in_time.year == out_time.year && in_time.month == out_time.month && in_time.day == out_time.day)
				{
					ks_printf("\r\n Matched!! [%d] [%d %d %d %d %d %d] step %d",
															i, out_time.year,  out_time.month,  out_time.day,  out_time.hour,  out_time.minute, out_time.second, load_data[i].step);
					step_info->time = load_data[i].time;
					step_info->step = load_data[i].step;
					return true;
				}
			}
			break;
			
		default:
			break;
	}
	
	return false;
}

void save_step_info_to_flash(unsigned int address, unsigned int gmt_flash, unsigned int step)
{
	tTime current_time, prev_time;
	struct flash_step_info rdata[MAX_FLASH_STEP_INFO];
	struct flash_step_info wdata[MAX_FLASH_STEP_INFO];
	
	memset(rdata,0,sizeof(rdata));
	memset(wdata,0,sizeof(wdata));
		
	step_flash_read( address, rdata);
	
	get_time_from_GMT(gmt_flash*60, &current_time);
	get_time_from_GMT(rdata[0].time*60, &prev_time);
		
	/* 23:59분 걸음수를 저장하려 할떄 23:58분 걸음수가 이미 저장되어 있을때 23:58분의 걸음수 정보 삭제 */
	if (current_time.day == prev_time.day) {
			memset(&rdata[0],0xff,sizeof(struct flash_step_info));
			rdata[0].time = gmt_flash;
			rdata[0].step = step;
		
			memcpy(&wdata[0],&rdata[0],sizeof(struct flash_step_info) * (MAX_FLASH_STEP_INFO));
	}
	else
	{
		memcpy(&wdata[1],&rdata[0],sizeof(struct flash_step_info) * (MAX_FLASH_STEP_INFO-1));
	
		wdata[0].time = gmt_flash;
		wdata[0].step = step;
	}
		
	feature_flash_erase(address);
	nrf_delay_ms(200);
		
	step_flash_write(address, wdata);
	nrf_delay_ms(100);
}

#endif /* v1.3.9 */

extern unsigned int g_time;
static char month_day[13]={0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};

int is_leap_year(int a)
{
	return ((a % 4) == 0) && ((a % 100)!=0) || ((a % 400) == 0);  // 윤달계산 수식표현
}

char get_month_day(int year, int month)
{
	int day;

	day=month_day[month];

	if(month==2 && is_leap_year(year))
		day++;

	return day;
}


extern unsigned char m_rtc_valid;

// flag => 1 :  rtc 값을 상태에 관게없이 읽음 
int get_time( tTime* pTime, int flag)
{

#if (__BOARD2__ || __BOARD3__)
	unsigned char array[12];

	if(!rtc_i2c_read(4,7, array))
		return false;

	pTime->year = Bcd2Bin(array[6])+2000;
	pTime->month=Bcd2Bin(array[5]);
	pTime->day=Bcd2Bin(array[3]);
	pTime->hour=Bcd2Bin(array[2]);
	pTime->minute=Bcd2Bin(array[1]);
	pTime->second=Bcd2Bin(array[0]&0x7f);

		
	if(array[0]&0x80)
	{
		if(!flag)
		{
		pTime->hour=0;
		pTime->minute=0;
		}
		
		m_rtc_valid=false;
	}

#if 1 /* v1.3.8 hkim 2015.11.25 */
		/* hkim 1.3.9.10,  2016.1.26 ,  2015 --> 2000 */
		/* hkim v1.5.16.0 2016.5.12. year < 2000 --> year < 2015 
	    밴드가 방전된 후 부팅하면 2000년 부터 시작하므로 피쳐가 생성됨을 방지 */
	if (pTime->year <= 2000 || pTime->year >= 2030) 
		return false;
#endif /* v1.3.8 */	
#else
	int sec,temp_sec;
	int i;
	char mon_day;


	sec = g_time/20;  /* total_sec; */

	temp_sec=sec;

	for(i=0;i<100;i++)
	{
		if(is_leap_year(2000+i))  /* hkim 1.3.9.10,  2016.1.26 ,  2015 --> 2000 */
		{
			if(temp_sec<(86400*366))
				break;
			temp_sec=temp_sec-(86400*366);
		}
		else
		{
			if(temp_sec<(86400*365))
				break;	
			temp_sec=temp_sec-(86400*365);
		}

	}

	pTime->year = 2000-i;  /* hkim 1.3.9.10,  2016.1.26 ,  2015 --> 2000 */

	for(i=1;i<=12;i++)
	{
		mon_day = get_month_day(pTime->year,i);
			
		if(temp_sec<mon_day*86400)
			break;

		temp_sec= temp_sec - (mon_day*86400);

		
	}
	
	pTime->month=i;
	pTime->second=(temp_sec)%60;
	pTime->minute=(temp_sec/60)%60;
	pTime->hour=(temp_sec/3600)%24;
	pTime->day=(temp_sec/3600/24)+1;

#endif

	return true;
}



int get_time_from_GMT( unsigned int gmt, tTime* pTime)
{
	int sec,temp_sec;
	int i;
	char mon_day;


	sec = gmt;  /* total_sec; */

	temp_sec=sec;

	for(i=0;i<100;i++)
	{
		if(is_leap_year(2000+i))  /* hkim 1.3.9.10,  2016.1.26 ,  2015 --> 2000 */
		{
			if(temp_sec<(86400*366))
				break;
			temp_sec=temp_sec-(86400*366);
		}
		else
		{
			if(temp_sec<(86400*365))
				break;	
			temp_sec=temp_sec-(86400*365);
		}

	}
  
	/* hkim 1.3.9.10,  2016.1.26 ,  2015 --> 2000 */
	pTime->year = 2000+i; /* shkim 2016.1.7 '-' --> '+'. 2016년을 2014년으로 판단하는 문제 수정 */

	for(i=1;i<=12;i++)
	{
		mon_day = get_month_day(pTime->year,i);
			
		if(temp_sec<mon_day*86400)
			break;

		temp_sec= temp_sec - (mon_day*86400);

		
	}
	
	pTime->month=i;
	pTime->second=(temp_sec)%60;
	pTime->minute=(temp_sec/60)%60;
	pTime->hour=(temp_sec/3600)%24;
	pTime->day=(temp_sec/3600/24)+1;



	return true;
}





unsigned int get_GMT(unsigned int year, unsigned char month, unsigned char day, unsigned char hour, unsigned char min, unsigned char sec)
{
	int i;
	int total_day=0;
	int total_sec=0;
	
	
	if(year<2000)  /* hkim 1.3.9.10,  2016.1.26 ,  2015 --> 2000 */
		return 0;

	for(i=2000;i<year;i++)  /* hkim 1.3.9.10,  2016.1.26 ,  2015 --> 2000 */
	{
		total_day=total_day+365;
		if( is_leap_year(i) )
			total_day++;
	}

	for(i=1;i<month;i++)
	{
		
		total_day=total_day+month_day[i];
		if(i==2)
		{
			if( is_leap_year(year) )
				total_day++;			
		}
	}

	total_day=total_day+(day-1);

	total_sec = (total_day*86400) + (hour*3600)+(min*60)+sec;
	
	return total_sec;
}

#if 1 /* hkim 1.3.9.10   2016.1.26 세계시간 동기화 */
extern short gmt_offset;

/* pTime : GMT Time,  stdTime : 그 지역의 표준시 */
void convert_gmt_to_display_time( tTime* pTime, tTime* stdTime)
{
		unsigned int __gmt=0;
	
		//ks_printf("\r\n GMT =%d %d %d %d %d %d",pTime->year, pTime->month, pTime->day, pTime->hour, pTime->minute, pTime->second); 
		__gmt = get_GMT(pTime->year, pTime->month, pTime->day, pTime->hour, pTime->minute, pTime->second);
	
		//ks_printf("\r\n$$$$$$$$$$$$$ GMT offset %d", gmt_offset); 
		__gmt = __gmt + (gmt_offset * 60);
			
		get_time_from_GMT(__gmt, stdTime);
		//ks_printf("\r\n standard time =%d %d %d %d %d %d",stdTime->year, stdTime->month, stdTime->day, stdTime->hour, stdTime->minute, stdTime->second); 
}
#endif 



__align(4) static unsigned char bl_fdb_tbuf[48] = {0,}; /* __align(4) is added at 0730 */

/* float data * 10000 -> int type..converted. */
void bl_st_feature_make_data(struct feature_tbl_form_s *fdb_ptr)
{
	int norm_var;
	short norm_mean;
	short x_mean,y_mean,z_mean;
	short x_var, y_var,z_var;
	#if 1 /* hkim 2015.7.7 engineV4 */
	//unsigned short jumping_rope_count, small_swing_count, large_swing_count;
//	short	pre_diff_sum, amp_rtsl, stepFreq_rtsl; /* v1.3.8 hkim 2015.11.23 */
	#endif 

	memset(bl_fdb_tbuf,0, 48);

	norm_mean	= ((fdb_ptr->norm_mean));
	norm_var         = ((fdb_ptr->norm_var));
	x_mean		= ((fdb_ptr->x_mean));
	y_mean 		= ((fdb_ptr->y_mean));
	z_mean 		= ((fdb_ptr->z_mean));
	x_var 		= ((fdb_ptr->x_var));
	y_var 		= ((fdb_ptr->y_var));
	z_var 		= ((fdb_ptr->z_var));
#if 1 /* hkim 2015.7.7 engineV4 */
	pre_diff_sum 	= ((fdb_ptr->pre_diff_sum));
#if 0 /* v1.3.7 hkim 2015.11.11 */
	amp_rtsl					= ((fdb_ptr->amp_rtsl)); 
	stepFreq_rtsl	= ((fdb_ptr->stepFreq_rtsl));
#endif 
#endif /* hkim 2015.7.7 engineV4 */
	if (dbg_feature_send) /* v1.3.9 */
	{
		ks_printf("\r\n*** Time(flash)   : %d", fdb_ptr->time);
		ks_printf("\r\n*** BT Tx(flash)  : N/x/y/z_var --> %d, %d, %d %d,  x/y/z/N_mean --> %d, %d, %d, %d", 
														norm_var,x_var, y_var, z_var, x_mean, y_mean, z_mean, norm_mean);
		ks_printf("\r\n*** BT Tx(flash) : step/jpc/s_swing/l_swing --> %d, %d, %d, %d", 
														 fdb_ptr->nStep, fdb_ptr->jumping_rope_count, fdb_ptr->small_swing_count, fdb_ptr->large_swing_count);
		ks_printf("\r\n*** BT Tx(flash) : pds/pressure/dayStep/accuStep/hb --> %d, %d, %d, %d %d",
												 fdb_ptr->pre_diff_sum,  fdb_ptr->pressure, fdb_ptr->display_step, fdb_ptr->accumulated_step, fdb_ptr->hb);
	}
  *((unsigned int*)(&bl_fdb_tbuf[0])) = fdb_ptr->time;
	*((int*)(&bl_fdb_tbuf[4])) = norm_var;

	*((short*)(&bl_fdb_tbuf[8])) = x_var;
	*((short*)(&bl_fdb_tbuf[10])) = y_var;
	*((short*)(&bl_fdb_tbuf[12])) = z_var;

	*((short*)(&bl_fdb_tbuf[14])) = x_mean;
	*((short*)(&bl_fdb_tbuf[16])) = y_mean;
	*((short*)(&bl_fdb_tbuf[18])) = z_mean;
	*((short*)(&bl_fdb_tbuf[20])) = norm_mean;

	*((short*)(&bl_fdb_tbuf[22])) = fdb_ptr->nStep;
#if 0 /* hkim 2015.7.7 engineV4 */	
	*((short*)(&bl_fdb_tbuf[22])) = fdb_ptr->nSwing;
	*((unsigned int*)(&bl_fdb_tbuf[24])) = fdb_ptr->pressure;
	bl_fdb_tbuf[28] = fdb_ptr->hb;
#else
	*((unsigned short*)(&bl_fdb_tbuf[24])) = fdb_ptr->jumping_rope_count; 
	*((unsigned short*)(&bl_fdb_tbuf[26])) = fdb_ptr->small_swing_count;
	*((unsigned short*)(&bl_fdb_tbuf[28])) = fdb_ptr->large_swing_count;
	*((short*)(&bl_fdb_tbuf[30])) = fdb_ptr->pre_diff_sum;
	
#if 0 /* v1.3.9 hkim 2015.12.14 */

	#if 0 /* v1.3.7 hkim 2015.11.11 */
	*((short*)(&bl_fdb_tbuf[32])) = amp_rtsl;
	*((short*)(&bl_fdb_tbuf[34])) = stepFreq_rtsl;
	#else
	*((short*)(&bl_fdb_tbuf[32])) = 0;
	*((short*)(&bl_fdb_tbuf[34])) = 0;
	#endif /* v1.3.7 */ 
	
	*((unsigned int*)(&bl_fdb_tbuf[36])) = (fdb_ptr->pressure);
/*	
	*((unsigned short*)(&bl_fdb_tbuf[34])) = (fdb_ptr->pressure)&0x0ffff;
	*((unsigned short*)(&bl_fdb_tbuf[36])) = fdb_ptr->pressure>>16;
*/
	*((unsigned short*)(&bl_fdb_tbuf[40])) = (unsigned short)fdb_ptr->display_step;
	bl_fdb_tbuf[42] = fdb_ptr->hb;

#else /* v1.3.9 */

	*((unsigned int*)(&bl_fdb_tbuf[32])) = (fdb_ptr->pressure);
	*((unsigned int*)(&bl_fdb_tbuf[36])) = (unsigned int)fdb_ptr->accumulated_step;
	*((unsigned short *)(&bl_fdb_tbuf[40])) = (unsigned short)fdb_ptr->display_step;
	bl_fdb_tbuf[42] = fdb_ptr->hb;

#endif /* v1.3.9 hkim */
#endif /* hkim 2015.7.7 engineV4 */

	if (dbg_feature_send) /* v1.3.9 BBB*/
	{
		ks_printf("\r\n*** BT Tx(mem) : n/x/y/z_var --> %d, %d, %d %d,  x/y/z/N_mean  --> %d, %d, %d, %d", 
													*((int*)(&bl_fdb_tbuf[4])), *((short*)(&bl_fdb_tbuf[8])), *((short*)(&bl_fdb_tbuf[10])), *((short*)(&bl_fdb_tbuf[12])),
													*((short*)(&bl_fdb_tbuf[14])), *((short*)(&bl_fdb_tbuf[16])), *((short*)(&bl_fdb_tbuf[18])), *((short*)(&bl_fdb_tbuf[20])));
		ks_printf("\r\n *** BT Tx(mem) : step/jrc/s_swing/l_swing --> %d, %d, %d, %d",
													*((short*)(&bl_fdb_tbuf[22])), *((short*)(&bl_fdb_tbuf[24])), *((short*)(&bl_fdb_tbuf[26])), *((short*)(&bl_fdb_tbuf[28])));
		ks_printf("\r\n *** BT Tx(mem) : pds/pressure/accuStep/dayStep/hb --> %d, %d, %d, %d %d",
													*((short*)(&bl_fdb_tbuf[30])), *((unsigned int*)(&bl_fdb_tbuf[32])), *((unsigned int*)(&bl_fdb_tbuf[36])), *((unsigned short*)(&bl_fdb_tbuf[40])), *((char*)(&bl_fdb_tbuf[42])));													
													
		ks_printf("\r\n Flash-SEQ-1 : [%x] [%x] [%x] [%x] [%x] [%x] [%x] [%x] [%x] [%x] [%x] [%x] [%x] [%x] [%x] [%x]", 
									bl_fdb_tbuf[0], bl_fdb_tbuf[1], bl_fdb_tbuf[2], bl_fdb_tbuf[3], bl_fdb_tbuf[4], bl_fdb_tbuf[5], bl_fdb_tbuf[6], bl_fdb_tbuf[7], bl_fdb_tbuf[8], bl_fdb_tbuf[9],
									bl_fdb_tbuf[10], bl_fdb_tbuf[11], bl_fdb_tbuf[12], bl_fdb_tbuf[13], bl_fdb_tbuf[14], bl_fdb_tbuf[15]);
		ks_printf("\r\n Flash-SEQ-2 : [%x] [%x] [%x] [%x] [%x] [%x] [%x] [%x] [%x] [%x] [%x] [%x] [%x] [%x] [%x] [%x]", 
									bl_fdb_tbuf[16], bl_fdb_tbuf[17], bl_fdb_tbuf[18], bl_fdb_tbuf[19], bl_fdb_tbuf[20], bl_fdb_tbuf[21], bl_fdb_tbuf[22], bl_fdb_tbuf[23], bl_fdb_tbuf[24], bl_fdb_tbuf[25],
									bl_fdb_tbuf[26], bl_fdb_tbuf[27], bl_fdb_tbuf[28], bl_fdb_tbuf[29], bl_fdb_tbuf[30], bl_fdb_tbuf[31]);
		ks_printf("\r\n Flash-SEQ-3 : [%x] [%x] [%x] [%x] [%x] [%x] [%x] [%x] [%x] [%x] [%x]", 
									bl_fdb_tbuf[32], bl_fdb_tbuf[33], bl_fdb_tbuf[34], bl_fdb_tbuf[35], bl_fdb_tbuf[36], bl_fdb_tbuf[37], bl_fdb_tbuf[38], bl_fdb_tbuf[39], bl_fdb_tbuf[40],
									bl_fdb_tbuf[41], bl_fdb_tbuf[42]);
	}
}

int bl_st_feature_report_sequence(ble_nus_t*	p_nus, unsigned char sequence)
{
		static unsigned char bl_buf[32]	= {0,};
		unsigned char data_start = 0;
		unsigned char data_end 	= 0;
		int i;
		int txindex = 0;
		unsigned char frame_len = 0;

		ks_printf("\r\nseq %d  %d",sequence, bl_fdb_tbuf[0]);

		if(  (sequence%3)==1 )
		{
			data_start	= 0;
			data_end		= 15;
			frame_len 	= 16;
		}
		else if(  (sequence%3)==2 )
		{
			data_start	= 16;
			data_end		= 31;
			frame_len 	= 16;
		}		
		else
		{
			data_start	= 32;
			data_end		= 42;
			frame_len 	= 11;
		}

		
		bl_buf[0] = 0x0b;
		bl_buf[1] = 0x04; // type;
		bl_buf[2] = frame_len;	//len - seqence number include.
		bl_buf[3] = sequence;

		txindex=4;	
		for(i = data_start;i<=data_end;i++)
		{
			bl_buf[txindex] = bl_fdb_tbuf[i];
			txindex++;		
		}

/*
		ks_printf("\r\n send=  ");
		for(i=0;i<frame_len+4;i++)
		{
			ks_printf("%02x ",bl_buf[i] );
		}
	
		err_code = ble_nus_send_string(p_nus, (uint8_t*)bl_buf  , frame_len+4);
		if (err_code != NRF_ERROR_INVALID_STATE)
		{
			APP_ERROR_CHECK(err_code);
		}
*/
		ble_send((uint8_t*)bl_buf  , frame_len+4);
	
		return true;
}

#if 1 /* v1.4.10.0 hkim 2016.02.12 */
unsigned char uart_tx_header;
extern unsigned char _uart_debug;

void read_gmt_offset_from_flash(unsigned short  page, void* data )
{
	unsigned char buf[260];
	unsigned int address;

	address = page<<8;
	SPI_FLASH_BufferRead(buf, address );
	
	memcpy(data,buf+4,sizeof(short));
}

int save_gmt_offset(short __gmt_offset)
{
	short __offset=0;
	
	__offset = __gmt_offset;
	
	feature_flash_erase(FLASH_GMT_OFFSET_PAGE);
	nrf_delay_ms(200);
		
	save_gmt_offset_to_flash(FLASH_GMT_OFFSET_PAGE, &__offset);
	nrf_delay_ms(100);
	
	return true;
}

void save_gmt_offset_to_flash(unsigned short  page, void* data )
{
	unsigned char buf[260];
	unsigned int address;
	
	memset(buf,0xff,sizeof(buf));
	
	address = page<<8;
	memcpy(buf+4, data, sizeof(short) );

	SPI_FLASH_PageWrite(buf, address );
}

int get_gmt_offset()
{
	short value=0;
	
	read_gmt_offset_from_flash(FLASH_GMT_OFFSET_PAGE, &value);
	
	/* GMT 최소/최개값 -11 / 13
	    스마트폰에서 offset은 분단위로 내려주므로 -11*60, 13*60 으로 값 유효성 검사 */
	if (value == 0xffffffff || value < -660 || value > 780 ) /* not 0xffff */
	{
		ks_printf("\r\n invalid GMT offset %d(%02x).", value, value);
		
		ks_printf("\r\n Set default GMT offset %d", KR_GMT_MIN);
		save_gmt_offset(KR_GMT_MIN);
		gmt_offset = KR_GMT_MIN;
		return false;
	}
	
	gmt_offset = value;
	ks_printf("\r\n GMT offset : %d", gmt_offset);
	return true;
}

#if __GREENCOM_24H__ /* v1.4.12.2 hkim 2016.3.10 */
int send_feature_on_uart(uint32_t s_gmt, uint32_t e_gmt, unsigned short flash_page)
{
		int i=0,j=0;
		char	sn_str[256];
		unsigned int snval=0;
		unsigned char tempval=0;
		//int c=0;  /* NRF_RTC1->COUNTER */
		//unsigned char *temp=NULL;
		tTime stime;
		static struct feature_tbl_form_s load_data[MAX_FEAT_PER_PAGE];
		
		/* initialization */
		memset(sn_str, 0, sizeof(sn_str));
		memset(load_data, 0, sizeof(load_data));
	
		if (uart_tx_header == 0)
		{
			simple_uart_putstring("start\n");
	
			/******************** SN을 먼저 전송 *****************************/
			Flash_Read_Text(FLASH_SERIAL_NUMBER_PAGE, sn_str );
			sn_str[10]=0;	// serial number =>  10byte cut
			
			//for (i=0; i < 10; i++) /* SN 최대 길이는 10 */
				//simple_uart_put((unsigned char)sn_str[i]);
			
			snval = atoi(sn_str);

			if (_uart_debug)	
				ks_printf("\r\n snval %d(%x)", snval, snval);
			
			for (i=0; i < 4; i++)
			{
					tempval = (unsigned char)((snval >> (i * 8)) & 0x000000FF);
					simple_uart_put((unsigned char) tempval);
				
					if (_uart_debug)					
						ks_printf("\r\n SN(%d) %x", i, tempval);
			}
			
			//simple_uart_putstring("\n");
			uart_tx_header = 1;
		}
		
		feature_flash_read(flash_page, load_data );
		
		#if 1 /* v1.5.17.0 hkim 2016.5.17 */
		if ( (load_data[0].time <s_gmt) || (load_data[0].time > e_gmt) )
		{
			if (_uart_debug)					
				ks_printf("\r\n feature(gmt %d(%x)) not in range",load_data[0].time,load_data[0].time );
			
			return true;
		}
		if ( (load_data[0].time == 0xffffffff) || (load_data[0].time == 0xffffffff) )
		{
			if (_uart_debug)					
				ks_printf("\r\n feature(gmt %d(%x)) time is NULL",load_data[0].time,load_data[0].time );
			
			return true;
		}
		#endif /* v1.5.17.0 hkim 2016.5.17 */
		
		get_time_from_GMT(load_data[4].time*60, &stime  );
		if ( (stime.year < 2015) || (stime.year == 0xffff) )
		{
			if (_uart_debug)
				ks_printf("\r\n skip Tx (year %d(%x))", stime.year, stime.year);
			return true;
		}
	
		for (i=0; i < MAX_FEAT_PER_PAGE; i++) /* KKKKK */
		{
#if 1 
				/* BT전송 포맷으로 변형 */
				memset(bl_fdb_tbuf,0, sizeof(bl_fdb_tbuf));
				bl_st_feature_make_data(&load_data[i]);			
			
				if (_uart_debug)
					ks_printf("\r\n ******** page %d  : ", flash_page);
				for (j=0; j < MAX_BT_TX_LEN; j++)
				{
						if (_uart_debug)
						{
							//ks_printf("%02x ", bl_fdb_tbuf[j]);
						}
						else
						{
							simple_uart_put((unsigned char)bl_fdb_tbuf[j]);
						}
				}
			
#else /* 피쳐데이타 그대로 전송하는 경우 */
			
				temp = (unsigned char *)load_data;
			
				ks_printf("size %d, total %d", sizeof(struct feature_tbl_form_s), sizeof(struct feature_tbl_form_s)*MAX_FEAT_PER_PAGE);
				for (i=0; i < sizeof(struct feature_tbl_form_s)*MAX_FEAT_PER_PAGE; i++)
				{
						if (i % 16 == 0)
							ks_printf("\r\n");
						ks_printf("%x ", (unsigned char)*(temp+i));
						//simple_uart_put((unsigned char)*(temp+i));
				}
#endif
		}
		
		return true;
}
/*******************************************************************************
* Function Name  : send_feature_data_on_uart
* Description    		: Send feature through UART
* Input          				: 
*									- s_gmt : start time, e_gmt : end time
*              
* Output         : None
* Return         : true or false
* Detail operation	:
*									- 
*******************************************************************************/
int send_feature_data_on_uart (uint32_t s_gmt, uint32_t e_gmt)
{
		unsigned short s_page=0, e_page=0, __page=0;

		//unsigned int time;
		unsigned int out_s_gmt=0, out_e_gmt=0; /* 검색된 시작/종료 GMT 시간 */
		//int c;
		//c= NRF_RTC1->COUNTER;
		//unsigned char *temp=NULL;
	
#if 0 	/* v1.3.15.0 hkim 2016.5.4 */
		/************ 시작시간에 해당하는 페이지를 검색 ************/
		//s_page = get_feature_data_page(  s_gmt, &time  );
		s_page = get_feature_data_page_from_top(  s_gmt, &time  );
		if (s_page == get_write_page_pointer() || time==0xffffffff)
		{
			if (_uart_debug)
			{
				ks_printf("\r\ns_page error : %d %d", s_page, time);
				ks_printf("\r\n");
			}
			
			//simple_uart_putstring("nodata\n");
			return false;
		}
		if (_uart_debug)
			ks_printf("\r\n searched s_page %d", s_page);

		/************ 종료시간에 해당하는 페이지를 검색 ************/
		//e_page = get_feature_data_page(  e_gmt, &time  );
		e_page = get_feature_data_page_from_top(  e_gmt, &time  );
		if (e_page == get_write_page_pointer() || time==0xffffffff)
		{
			if (_uart_debug)
			{
				ks_printf("\r\ne_page error : %d %d", e_page, time);
				ks_printf("\r\n");
			}
			
			//simple_uart_putstring("nodata\n");
			return false;
		}
#else /* v1.3.15.0 hkim 2016.5.4 */
		get_feature_data_page_from_top(  s_gmt, e_gmt, &s_page, &e_page, &out_s_gmt, &out_e_gmt );
		
		if (s_page == 0xffff || out_s_gmt==0xffffffff)
		{
			if (_uart_debug)
			{
				ks_printf("\r\ns_page error : %d %d", s_page, out_s_gmt);
				ks_printf("\r\nnodata : can't find start page");
			}
			
			//simple_uart_putstring("nodata\n"); /* to PC */
			return false;
		}
		
		if (e_page == 0xffff || out_e_gmt==0xffffffff)
		{
			if (_uart_debug)
			{
				ks_printf("\r\ne_page error : %d %d", e_page, out_s_gmt);
				ks_printf("\r\nnodata : can't find end page");
			}

			//simple_uart_putstring("nodata\n");
			return false;
		}
#endif  /* v1.3.15.0 hkim 2016.5.4 */
		if (_uart_debug)
			ks_printf("\r\n searched e_page %d", e_page);
		
		if (s_page == e_page)
		{			
			//simple_uart_putstring("nodata\n");
			return false;
		}

		if (e_page > s_page)
		{
				for (__page=s_page; __page < (e_page+1); __page++)
				{
						send_feature_on_uart(s_gmt, e_gmt, __page);
				}
		}
		else if (e_page < s_page)
		{
				for (__page=s_page; __page < MAX_DB_PAGE; __page++)
					send_feature_on_uart(s_gmt, e_gmt, __page);
				
				for (__page=0; __page < (e_page+1); __page++)
					send_feature_on_uart(s_gmt, e_gmt, __page);
		}
		else {}
		
		return true;
}
#endif /* v1.4.10.0 */

#if 1 /* v1.4.11.1 2016.2.17 */
extern unsigned char stop_feature_process;
void init_db_write_page()
{
	m_db_write_page = 0;
}
/*******************************************************************************
* Function Name  : write_feature_manual
* Description    		: 임의로 설정된 피쳐를 256(4096 페이지)개의 섹터에 기록
* Input          				: start/end GMT 시간
* Output         			: None
* Return         			: None
* Detail operation	:
		- 256 개의 섹터를 모두 지움
		- 입력시간을 1씩 증가시키면서 임의로 정의된 피쳐를 섹터에 기록
    - 구현 목적은 다수의 피쳐데이타가 웹서버에 기록되는지 빠른시간내 검사(예: 한림대 프로젝트)
    - 이 함수를 실행전 'feat stop' 명령을 반드시 수행할 것
*******************************************************************************/
#if 0 /* hkim v1.5.17.0 2016.5.12 */
void write_feature_manual(unsigned int start_gmt, unsigned int end_gmt)
{
	int i=0,_sector=0,_time=0;
	volatile unsigned short	_page,temp_page;
	int address=0;
	unsigned int page_num=0, sector_num=0;
	unsigned int feat_time=0;
	struct feature_tbl_form_s	_feature;
	
	memset(&_feature, 0, sizeof(struct feature_tbl_form_s));
	
	/* 더미 피쳐데이타 정의(시간정보 제외) */
	_feature.pressure 			= 0x00018ef0;
	_feature.norm_var			= 0x00000025;
	_feature.norm_mean	= 0x26b1;
	_feature.x_mean				= 0x0120;
	_feature.y_mean				= 0x038c;
	_feature.z_mean				= 0x2683;
	_feature.x_var						= 0x0000;
	_feature.y_var						= 0x0000;
	_feature.z_var						= 0x0000;
	_feature.nStep						= 0x0000;
	_feature.jumping_rope_count		= 0x0000;
	_feature.small_swing_count			= 0x0000;
	_feature.large_swing_count			= 0x0000;
	_feature.accumulated_step			= 0x00000000;
	_feature.display_step								= 0x00;
	_feature.hb															= 0x00;
	_feature.battery												= 0x01;
	
	/* 피쳐를 저장할 페이지 갯수와 삭제할 섹터 갯수를 구한다 */
	page_num 		= ((end_gmt+1) - start_gmt) / FEATURE_NUM_IN_FLASH_PAGE;
	sector_num 	= page_num / PAGE_NUM_IN_SECTOR; 
	
	if (_uart_debug)
	{
		ks_printf("\r\n %d page allocated and erase %d sector", page_num, sector_num);
	}

	/* 기간에 해당하는 만큼만 섹터를 지움(첫번째 섹터부터 지운다)
	    sector-0 에 2016.4.1 sector-1에 2016.5.1 이런 형태로 저장되지 않으므로 
      섹터는 항상 첫번째 섹터부터 지운다. 
      단, 테스트할때 이미 저장된 시간보다 미래의 시간 데이타를 write 해야 한다 */			
	for (_sector = 0; _sector < sector_num; _sector++)
	{
			address = _sector * SECTOR_SIZE;
			ks_printf("\r\nerase sector : %x", address);
			SPI_FLASH_SectorErase(address );
			nrf_delay_ms(200);
	}

	feat_time = start_gmt;
	
	ks_printf("\r\n # of page : %d", page_num);
	
	/* write feature from page 0 */
	for (_page=0; _page < page_num; _page++)
	{
			for (i=0; i < FEATURE_NUM_IN_FLASH_PAGE; i++)
			{
					_feature.time = feat_time;
					feat_time++;
					memcpy( &m_feature_10min[i], &_feature, sizeof(struct feature_tbl_form_s));
			}
			ks_printf("\r\n write feature[addr : %x, first time %x]", _page*PAGE_SIZE,m_feature_10min[0].time);
			feature_flash_fwrite(_page, m_feature_10min );	
			nrf_delay_ms(200);
	}
	m_db_write_page = page_num;

}
#else /* hkim v1.5.17.0 2016.5.12 */
void write_feature_manual(unsigned int start_gmt, unsigned int end_gmt)
{
	#if 0 //잘 동작한다. 메모리 부족으로 임시로 막음
//	int i=0,_sector=0,_time=0;
	tTime time;
	volatile unsigned short	_page,temp_page;
//	int address=0;
//	unsigned int page_num=0, sector_num=0;
	unsigned int feat_time=0;
	struct feature_tbl_form_s	_feature;
	
	memset(&_feature, 0, sizeof(struct feature_tbl_form_s));
	memset( &m_feature_10min, 0, sizeof(&m_feature_10min)); /* v1.5.17.0 hkim 2016.5.15 */
	
	/* 더미 피쳐데이타 정의(시간정보 제외) */
	_feature.pressure 			= 0x00018ef0;
	_feature.norm_var			= 0x00000025;
	_feature.norm_mean	= 0x26b1;
	_feature.x_mean				= 0x0120;
	_feature.y_mean				= 0x038c;
	_feature.z_mean				= 0x2683;
	_feature.x_var						= 0x0000;
	_feature.y_var						= 0x0000;
	_feature.z_var						= 0x0000;
	_feature.nStep						= 0x0000;
	_feature.jumping_rope_count		= 0x0000;
	_feature.small_swing_count			= 0x0000;
	_feature.large_swing_count			= 0x0000;
	_feature.accumulated_step			= 0x00000000;
	_feature.display_step								= 0x00;
	_feature.hb															= 0x00;
	_feature.battery												= 0x01;
	
	for (feat_time=start_gmt; feat_time < end_gmt+1; feat_time++)
	{
			_feature.time = feat_time;
			get_time_from_GMT(feat_time*60, &time  );
		  ks_printf("\r\n----------------------------------");
			ks_printf("\r\n%d(%d/%d %02d:%02d) ", _feature.time,  time.month, time.day, time.hour, time.minute);
		
			save_feature_data(&_feature, time.minute);
			nrf_delay_ms(200); /* FIXME v1.5.17.0 hkim 2016.5.16 */
	}
	#endif 
}
#endif /* hkim v1.5.17.0 2016.5.12 */

#endif /* v1.4.11.1 */
#endif /* __GREENCOM_24H__ */
