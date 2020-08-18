#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>

#include "gc_activity.h"
#include "bma2x2.h"
#include "define.h"
#include "gc_comm.h"
#include "spi_flash.h"
#include "print.h"
#include "nrf.h"
#include "flash_control.h"
#include "nrf_delay.h"
#include "protocol.h"
#include "nrf_gpio.h"
#include "gc_gpio.h"
#include "pah8002.h"
#include "display.h"
#include "an520.h"

//#include "afe4400.h"


//#define printf(...)

extern volatile unsigned short vibration_count;
extern volatile unsigned char day_of_week;
volatile unsigned char hr_warring;
volatile unsigned char hr_50 = 0;
volatile unsigned char hr_50_check = 0;
volatile unsigned char hr_under_50_check = 0;
volatile unsigned char hr_50_delay = 0;
volatile unsigned char hr_ON_OFF = 0;
volatile unsigned int hr_50_delay_count = 0;
extern volatile uint8_t pahr_mode;
unsigned int hr_50_check_count;


/*걸음수 계산 변수*/
extern volatile unsigned short g_step;
//extern unsigned char is_uart_mode; 	/* v1.4.10.0 hkim 2016.02.12 */
unsigned char stop_feature_process; /* v1.4.11.1 hkim 2016.02.17 */
volatile unsigned int g_total_daystep;
volatile unsigned int g_accumulated_step;
volatile struct SLEEP_INFO slp_info;
volatile struct STEP_INFO step;
//int acc_norm_mean;
//int acc_var_norm;

extern volatile unsigned char is_charging;
extern volatile char save_low_bat;

typedef unsigned char boolean;
/************* Defines ****************/
#define def_window_size_z 18
#define def_window_size 10
#define continuous_step_occur_count_define 12

#define averaging_win_size 100 // 0.5sec
#define var_win_size 100 // 0.5sec
/******** End Defines *******/

/******* buffer *********/
short step_count_epoch[continuous_step_occur_count_define] = { 0, };
float acc_buffer_0[def_window_size_z] = { 0, };
float acc_buffer_z[def_window_size_z] = { 0, };
float acc_buffer[def_window_size_z] = { 0, };
int acc_buffer_int[def_window_size_z] = { 0, };
float acc_buffer_norm[def_window_size] = { 0, };
int acc_buffer_norm_int[def_window_size] = { 0, };
int acc_buffer_z_int[def_window_size_z] = { 0, };

/******* End Buffer ***********/
int moon_count=0;
unsigned char daily_reset =0;
unsigned char is_daily_save = 1;                                                                                                               
unsigned char step_y_axis_mean_winsize = 10;
float acc_mean_y_axis_10 = 0;
int acc_mean_y_axis_10_int = 0;
unsigned char window_size_n = 10;
unsigned char window_size = 10;
unsigned char window_size_z = 18;
int upper_norm_thre_max = 23;
int upper_norm_thre_min = 12;
int max_step_frequency_thre = 22;
int min_step_frequency_thre = 5;
int lower_norm_thre_max = 12;
int upper_lower_time_diff_max = 16;

boolean upper_peak_occur = FALSE, lower_peak_occur = FALSE, swing_walking_flag = FALSE;
boolean previous_continuous_step_flag = FALSE;
boolean continuous_step_flag = FALSE;

#define sen_data_frequency 20
#define feature_duration 60
unsigned int data_length_per_unittime = sen_data_frequency * feature_duration;

unsigned char continuous_step_occur_count = 12;

unsigned char peak_pos = 0;
unsigned int epoch = 0;
volatile unsigned int temp_step_count = 0;
volatile unsigned short step_count = 0;
float previous_lower_peak_value = 0;
float  upper_lower_peakvalue_diff = 0.5;
	//AFE4400_current_control(count); // 심박센서

volatile unsigned short rolled_count;
volatile unsigned short awake_count;

int pre_sleep_count = 0;
extern volatile uint8_t g_battery;
uint8_t pre_g_battery = 6;
/*moon*/
unsigned char slp_chk_cnt;
unsigned char slp_start_count;
unsigned char slp_state[12];
unsigned char slp_end_count;
unsigned int non_slp_count;
extern volatile unsigned char tuch_count;
extern volatile unsigned short OLED_power_count;
extern volatile unsigned char OLED_switch;

extern volatile unsigned char go_sleep;
extern volatile unsigned char op_mode;
unsigned char battey_alarm = 0;
/*심박 계산 변수*/
//testunsigned char heart_rate = 69;
volatile unsigned char g_hr;
//void greencom_HR_process(unsigned int count)
//{
//	srand(count);
//	unsigned char heart_rate = rand()%55+75;
//	heart_rate = ((count/61)%2==0) ? heart_rate+1 : heart_rate-1;
//	printf("\r\nhr=[%d]",heart_rate);
//		get_realtime_HR(count, heart_rate);
//}

#define HR_length 10
unsigned char HR_buffer[HR_length] = { 0, };
//규창 아래 최고 최저 평균 안정 심박 산출변수들 main.c 전역변수로 이동 
//unsigned int epoch_hr=0 ,epoch_hr_1min = 0;
//unsigned char HR_max=0,HR_avg,HR_avg_1min,HR_min=0xFF,HR_sta=0;
//unsigned int HR_sum=0, HR_sum_1min=0 ;
volatile struct s_hart_rate HR_info;

//규창 최저심박 센서 이상 상태일 때 갱신용
//extern volatile bool reset_minHR;


tTime time;
tTime stdTime;

/*칼로리 계산 변수*/
volatile struct calorie_save kcal;
#define sumInterval  2 // 칼로기 계산 주기(초)
extern volatile struct PERSONAL_INFO person_info;
//volatile unsigned int sleep_time;
volatile unsigned int activty_time;
volatile struct ACTIVITY_INFO act_info;

//규창 수면체크플래그
static bool chk_rolled_sleep_flag = false;
static bool chk_awake_sleep_flag = false;

//static unsigned char former_op_mode = 0;
unsigned int 	cal_hr_1min;

/*운동 강도 계산 변수 */
unsigned short weak_count = 0,middle_count = 0,strong_count = 0,danger_count = 0;
unsigned int act_time = 0;
/* hkim 1.3.9.10   2016.1.26 세계시간 동기화 변수*/
//extern short gmt_offset;
volatile short gmt_offset;
extern volatile unsigned int g_time;
static char month_day[13]={0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};

/*걸음수 계산 로직*/
float getFloat(int integer) {
	return integer/100.0f;
}

void greencom_step_process(unsigned int count, short acc_x, short acc_y, short acc_z)
{
	//printf("\r\ngreencom_step_process");
//	struct bma2x2_accel_data sample_xyz;
//	char s[20];
	

//	if (is_uart_mode == true)
//	{
//		return;
//	}

	if (stop_feature_process == true)
	{
		return;
	}
	
	//bma2x2_read_accel_xyz(&sample_xyz);/* Read the accel XYZ data*/

	//printf("\r\nx:[%d] y[%d] z[%d]\n", sample_xyz.x, sample_xyz.y, sample_xyz.z );


	#ifndef __GREENCOM_24H_BT_SENSOR_TEST__
	get_realtime_step(count,acc_x, acc_y, acc_z );
	#endif
	
	if (epoch > data_length_per_unittime * 3500000)
	{
		epoch = 0;
	}
	//AFE4400_current_control(count);

}

/*
	sprintf(s,"%d  ",g_step);
	TextOut(0, 1,s);
*/	


//규창 171013 fixFloat 함수 추가
extern float pressureResult;
double fixFloat(double a ,int n) {
    return ((int)(a * pow(10.0, n)))/pow(10.0,n);
}

//기압 관련변수들
float pre_diff_press = 0, press_sum = 0;
unsigned char press_count = 0;
boolean set_press_sum = FALSE;
float pre_mean = 0;
float previous_press = 0;
float pre_diff = 0, pre_diff_sum = 0;

//피쳐 관련 변수들
float acc_mean_x_axis = 0, acc_mean_y_axis = 0, acc_mean_z_axis = 0, acc_norm_mean = 0;
float acc_var_x_axis = 0, acc_var_y_axis = 0, acc_var_z_axis = 0, acc_var_norm = 0;

//규창 스윙 피쳐 변수들
boolean continuous_swing_flag = FALSE;
unsigned short small_swing_count = 0;
boolean swing_peak_occur = FALSE;
boolean other_swing_peak_occur = FALSE;
unsigned char window_size_swing = 10;

unsigned char jumping_rope_thre = 18;
unsigned char continuous_swing_occur_count = 5;
unsigned char max_swing_frequency_thre = 18;
unsigned char min_swing_frequency_thre = 5;

#define continuous_step_occur_count_define 12 /* greencomm 08.05 */
#define continuous_swing_occur_count_define 5
unsigned int temp_swing_count = 0;
unsigned char swing_epoch_diff_max = 18;
short temp_swing_count_epoch_total[continuous_swing_occur_count_define] = { 0, };

unsigned char jumping_rope_thre_upper = 40;
unsigned short jumping_rope_count = 0;
unsigned char step_min_fre_thre = 5;
unsigned short large_swing_count = 0;
unsigned char large_swing_thre = 50; /* 5.3.2 08.12 */
unsigned int previous_epoch = 0;
int y_swing_front_sum_thre = -40; // 4G는 -40가 최대값.

#define window_size_y_extend 20
/* 3sec if granularity is 250msec */
#define golf_avg_time_buffer_start	0
#define golf_avg_time_buffer_end		11 
/* 2sec if granularity is 250msec */
#define golf_backswing_time_buffer_start	12
#define golf_backswing_time_buffer_end	19 
float acc_buffer_y_extend[window_size_y_extend] = {0,};
#define y_extended_avg_thre	-7 /* GC */
#define y_extended_avg_size	12 /* GC */ /* v1.2.4 6 --> 12 hkim 2015.9.30 , 3초 평균 */
float y_extended_sum=0, y_extended_avg=0; /* GC */

unsigned char window_size_y = 10;
#define def_window_size_y 10
float acc_buffer_y[def_window_size_y] = { 0, };
boolean previous_continuous_swing_flag = FALSE;
unsigned char small_swing_thre = 40;
unsigned short g_total_day_jumprope;



///////
struct feature_tbl_form_s featuredata;

void get_realtime_step(unsigned int count, short acc_x, short acc_y, short acc_z) {
	
	float x = acc_x /50.0;
	float y = acc_y /50.0;
	float z = acc_z /50.0;
	
	//규창 171013 기압변수 추가
	double press = 0;
	
	//int i=0;
	// Acc norm
	float acc_norm = sqrtf(powf(x, 2) + powf(y, 2) + powf(z, 2));
	
	epoch++;
	
	//규창 171013
	unsigned char y_backswing = 0;
	boolean swing_walking_flag = FALSE;
	
	press = fixFloat( ((double)pressureResult/(double)100), 2);
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
	//printf("\r\n press_diff:%f, press:%f",  pre_diff_press, press);

		if (epoch == 1) 
		{
			//규창 171013 피쳐추가
			acc_mean_x_axis = x;
			acc_mean_y_axis = y;
			acc_mean_z_axis = z;
			
			acc_norm_mean = acc_norm;
		}
		else if (epoch < averaging_win_size )
		{
			//규창 171013 피쳐 추가
			/*acc_mean_x_axis = ((acc_mean_x_axis * (epoch -1) + (x *100)) / epoch);
			acc_mean_y_axis = ((acc_mean_x_axis * (epoch -1) + (y *100)) / epoch);
			acc_mean_z_axis = ((acc_mean_x_axis * (epoch -1) + (z *100)) / epoch);
		
			acc_norm_mean = ((acc_norm_mean * (epoch - 1) + (acc_norm*100) )/ epoch);*/
			acc_mean_x_axis = acc_mean_x_axis * ((float) (epoch - 1) / epoch)
				+ (x / epoch);
		acc_mean_y_axis = acc_mean_y_axis * ((float) (epoch - 1) / epoch)
				+ (y / epoch);
		acc_mean_z_axis = acc_mean_z_axis * ((float) (epoch - 1) / epoch)
				+ (z / epoch);
		acc_norm_mean = acc_norm_mean * ((float) (epoch - 1) / epoch)
				+ (acc_norm / epoch);
		}
		else
		{
			//규창 171013 피쳐 추가
			/*acc_mean_x_axis = ((acc_mean_x_axis * (averaging_win_size - 1)) +(x*100)) / averaging_win_size;
			acc_mean_y_axis = ((acc_mean_y_axis * (averaging_win_size - 1)) +(y*100)) / averaging_win_size;
			acc_mean_z_axis = ((acc_mean_z_axis * (averaging_win_size - 1)) +(z*100)) / averaging_win_size;
			
			acc_norm_mean = ((acc_norm_mean  * (averaging_win_size - 1)) +(acc_norm*100)) / averaging_win_size;*/
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
	//	printf("acc_mean_x:%.2f acc_mean_y:%.2f acc_mean_z:%.2f\n",acc_mean_x_axis,acc_mean_y_axis,acc_mean_z_axis);
		
		if (epoch == 1) 
		{
			//규창 171013 피쳐추가
			acc_var_x_axis = 0;
			acc_var_y_axis = 0;
			acc_var_z_axis = 0;
			
			acc_var_norm = 0;
		} 
		else if (epoch < var_win_size)
		{
			//규창 171013 피쳐추가
			/*acc_var_x_axis = (int)((acc_var_x_axis * (epoch - 1)+ powf(acc_mean_x_axis/100 - (x), 2)) / epoch);
			acc_var_y_axis = (int)((acc_var_y_axis * (epoch - 1)+ powf(acc_mean_y_axis/100 - (y), 2)) / epoch);
			acc_var_z_axis = (int)((acc_var_z_axis * (epoch - 1)+ powf(acc_mean_z_axis/100 - (z), 2)) / epoch);
			
			acc_var_norm = (int)((acc_var_norm * (epoch - 1)+ powf(acc_norm_mean/100 - (acc_norm), 2)) / epoch);*/
			acc_var_x_axis = acc_var_x_axis * ((float) (epoch - 1) / epoch)
				+ ((powf(acc_mean_x_axis - x, 2)) / epoch);
		acc_var_y_axis = acc_var_y_axis * ((float) (epoch - 1) / epoch)
				+ ((powf(acc_mean_y_axis - y, 2)) / epoch);
		acc_var_z_axis = acc_var_z_axis * ((float) (epoch - 1) / epoch)
				+ ((powf(acc_mean_z_axis - z, 2)) / epoch);
		acc_var_norm = acc_var_norm * ((float) (epoch - 1) / epoch)
				+ ((powf(acc_norm_mean - acc_norm, 2)) / epoch);
		}
		else
		{
			//규창 171013 피쳐추가
			/*acc_var_x_axis = (int)((acc_var_x_axis * (var_win_size - 1)+ powf(acc_mean_x_axis/100 - (x), 2)) / var_win_size);
			acc_var_y_axis = (int)((acc_var_y_axis * (var_win_size - 1)+ powf(acc_mean_y_axis/100 - (y), 2)) / var_win_size);
			acc_var_z_axis = (int)((acc_var_z_axis * (var_win_size - 1)+ powf(acc_mean_z_axis/100 - (z), 2)) / var_win_size);
			
			acc_var_norm = (int)((acc_var_norm * (var_win_size - 1)+ powf(acc_norm_mean/100 - (acc_norm), 2)) / var_win_size);*/
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
		
		//printf("\r\n x:%.2f, y:%.2f, z:%.2f, norm:%.2f", x, y, z, acc_norm);
		//printf("\r\n acc_mean_x:%.2f acc_mean_y:%.2f acc_mean_z:%.2f acc_norm_mean :%.2f",acc_mean_x_axis,acc_mean_y_axis,acc_mean_z_axis, acc_norm_mean);
		//printf("\r\n acc_var_x:%.2f acc_var_y:%.2f acc_var_z:%.2f acc_var_norm:%.2f",acc_var_x_axis,acc_var_y_axis,acc_var_z_axis, acc_var_norm);
		//printf("\r\n acc_var_x:%.2f acc_var_y:%.2f acc_var_z:%.2f acc_var_norm:%.2f",acc_var_x_axis,acc_var_y_axis,acc_var_z_axis, acc_var_norm);
	//printf("acc_var_x:%.2f acc_var_y:%.2f acc_var_z:%.2f\n",acc_var_x_axis,acc_var_y_axis,acc_var_z_axis);
		
			//printf("\r\n acc_var_norm : %d acc_norm_mean : %d acc_norm : %d", acc_var_norm,acc_norm_mean/100,acc_norm);
	// buffer
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
	
	
	if (epoch <= window_size_y) {
		acc_buffer_y[epoch - 1] = y;
	} else {
		int i = 0;
		for (i = 0; i < window_size_y - 1; i++) {
			acc_buffer_y[i] = acc_buffer_y[i + 1];
		}
		acc_buffer_y[window_size_y - 1] = y;
	}
	if(epoch % 5 == 0) { /* save period : 250msec */
		int i = 0;
		for (i = 0; i < window_size_y_extend - 1; i++) {
			acc_buffer_y_extend[i] = acc_buffer_y_extend[i + 1];
		}
		acc_buffer_y_extend[window_size_y_extend - 1] = y;
	}
	
	

	if(epoch == 1) {
		acc_mean_y_axis_10 = y;
	} else if(epoch < step_y_axis_mean_winsize) {
		acc_mean_y_axis_10 = acc_mean_y_axis_10 * ((float) (epoch - 1) / epoch)
					+ (y / epoch);
	} else {
		acc_mean_y_axis_10 = acc_mean_y_axis_10 * ((float) (step_y_axis_mean_winsize - 1) / step_y_axis_mean_winsize)
					+ (y / step_y_axis_mean_winsize);
	}
	//printf("\r\n acc_mean_y_axis_10_int:[%d]",acc_mean_y_axis_10_int);
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

	// start step find...
	upper_peak_occur = lower_peak_occur = FALSE;
	
	// 규창 171013 스윙 찾기
	//스탭 찾기
	swing_peak_occur = other_swing_peak_occur = FALSE;
	int now_swing = (window_size_swing / 2) - 1;

	
	int now = (window_size / 2) - 1;
	

	if (epoch > window_size) {
		// 피크 찾기(Upper peak)
		
//		printf("\r\n [%d][%d]    [%d][%d]",acc_buffer_int[now],upper_norm_thre_min*100,acc_buffer_int[now],upper_norm_thre_max*100);
		if (acc_buffer[now] > upper_norm_thre_min && acc_buffer[now] < upper_norm_thre_max) {
			
			if (acc_buffer[now] >= acc_buffer[now - 1]
											  && acc_buffer[now] > acc_buffer[now + 1]) {
				// 왼쪽 비교
				int i = 0;
				for (i = 0; i < window_size / 2 - 2; i++) {
				//printf("\r\n Left%d:%d",acc_buffer_int[now],acc_buffer_int[i]);
					if (acc_buffer[now] < acc_buffer[i]) {
						upper_peak_occur = TRUE;
						break;
					}
				}
				//오른쪽 비교
				if (upper_peak_occur == FALSE) {
					for (i = 0; i < window_size / 2 - 1; i++) {
						//printf("\r\n Right%d:%d",acc_buffer_int[now],acc_buffer_int[now + 2 + i]);
						if (acc_buffer[now] < acc_buffer[now + 2 + i]) { // 신범주 0722 now +1 -> now +2
							upper_peak_occur = TRUE;
							break;
						}
					}
				}
				if(upper_peak_occur == TRUE)
				{
					moon_count++;
				}
				//최종 판단
//				printf("\r\n upper_peak_occur = %d",upper_peak_occur);
				
				if (upper_peak_occur == FALSE) {
						
					if((peak_pos != 1) && (abs(acc_buffer[now] - previous_lower_peak_value) > upper_lower_peakvalue_diff)) {
						temp_step_count++;
						if (temp_step_count <= continuous_step_occur_count) {
							step_count_epoch[temp_step_count - 1] = epoch;
						}else{
							int i = 0;
							for( i = 0;  i< continuous_step_occur_count-1 ; i++ ) {
								step_count_epoch[i] = step_count_epoch[i+1];
							}
							step_count_epoch[continuous_step_occur_count-1] = epoch;
						}

						peak_pos = 1;

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
						//printf("\r\nprevious_continuous_step_flag:%d\r\nprevious_continuous_step_flag%d\r\nswing_walking_flag:%d",previous_continuous_step_flag,previous_continuous_step_flag,swing_walking_flag);
						if(previous_continuous_step_flag == FALSE && continuous_step_flag == TRUE) {
							//printf("continuous_step_occur_count :[%d]",continuous_step_occur_count);
							if(swing_walking_flag == TRUE) {
								step_count = step_count + 2*(continuous_step_occur_count-1);
								g_total_daystep = g_total_daystep + 2*(continuous_step_occur_count-1);
								
								//규창 171019 플래너 피쳐 데이터에서 amp 대신 밴드 부팅이후 스텝으로 변경
								g_accumulated_step = g_accumulated_step + 2*(continuous_step_occur_count-1); /* v1.3.9 hkim 2015.12.05 */
							} else {
								step_count = step_count + (continuous_step_occur_count-1);
								g_total_daystep = g_total_daystep + (continuous_step_occur_count-1);
								
								//규창 171019 플래너 피쳐 데이터에서 amp 대신 밴드 부팅이후 스텝으로 변경
								g_accumulated_step = g_accumulated_step + (continuous_step_occur_count-1); /* v1.3.9 hkim 2015.12.05 */
		
							}
						}
						if (continuous_step_flag == TRUE) {
							if(swing_walking_flag == TRUE) {
								step_count += 2;
								g_total_daystep += 2;
								
								//규창 171019 플래너 피쳐 데이터에서 amp 대신 밴드 부팅이후 스텝으로 변경
								g_accumulated_step += 2; /* v1.3.9 hkim 2015.12.05 */
							} else {
								step_count++;
								g_total_daystep++;
								//규창 171019 플래너 피쳐 데이터에서 amp 대신 밴드 부팅이후 스텝으로 변경
								g_accumulated_step++; /* v1.3.9 hkim 2015.12.05 */
							}
						}
						previous_continuous_step_flag = continuous_step_flag;
					}
				}
			}
		}
		step.display_step = g_total_daystep ;
		if(acc_buffer[now] < lower_norm_thre_max) {
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
				if (lower_peak_occur == FALSE && peak_pos != 2) {
					previous_lower_peak_value = acc_buffer[now];
					peak_pos = 2;
				}
			}
		}
	}
	
	
	
	// 규창 171013 swing peak 검출
			//printf("\r\n acc_buffer_norm_int[now_swing] %f", acc_buffer_norm[now_swing]);
			if (acc_buffer_norm[now_swing] > jumping_rope_thre ) {
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
							temp_swing_count_epoch_total[continuous_swing_occur_count-1] = epoch;
						}

						// large swing 검출
						//printf("\r\n acc_buffer_norm[now_swing] %f", acc_buffer_norm[now_swing]);
						if(acc_buffer_norm[now_swing] > large_swing_thre) {
							// 골프 스윙 검출
							y_extended_sum = 0;
							/* v1.2.4 'y_extended_avg_size+1' -> 'y_extended_avg_size' hkim 2015.09.30 */
							for (i=golf_avg_time_buffer_start; i < golf_avg_time_buffer_end+1; i++) { 
								y_extended_sum += acc_buffer_y_extend[i];
							}
							/* detect golf address */
							y_extended_avg = y_extended_sum / y_extended_avg_size;
							
							/* detect golf back-swing */
							for (i=golf_backswing_time_buffer_start; i < golf_backswing_time_buffer_end+1; i++) {
								if (acc_buffer_y_extend[i] > 0)  {
									y_backswing = 1;
									break;
								}
							}
							
							/* acc_buffer_z[0] match to acc_buffer_y[2] */
							//printf("\r\n acc_buffer_y[2] + acc_buffer_y[3]: %f       acc_buffer_z_int[17]: %f       y_extended_avg:%f", acc_buffer_y[2] + acc_buffer_y[3], acc_buffer_z[17],  y_extended_avg);
							if ((acc_buffer_y[2] + acc_buffer_y[3] < y_swing_front_sum_thre) && (acc_buffer_z[17] < 0)  &&	(y_extended_avg < y_extended_avg_thre) && (y_backswing == 1)) {
								// golf_swing_count 대신 large_swing_count 사용.
								large_swing_count++;
							}		
						}
						// small swing 검출
						//printf("\r\n acc_buffer_norm_int[now_swing]: %f      acc_buffer_norm_int[now_swing]: %f", acc_buffer_norm[now_swing], acc_buffer_norm[now_swing]);
						if(acc_buffer_norm[now_swing] > small_swing_thre&& acc_buffer_norm[now_swing] < large_swing_thre) {
							small_swing_count++;
						}
						if (temp_swing_count > continuous_swing_occur_count-1) {

							int swing_epoch = 0;

							for(swing_epoch = 1; swing_epoch < continuous_swing_occur_count; swing_epoch++) {
								//printf("\r\n temp_swing_count_epoch_total[swing_epoch] - temp_swing_count_epoch_total[swing_epoch-1]: %d", temp_swing_count_epoch_total[swing_epoch] - temp_swing_count_epoch_total[swing_epoch-1]);
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

						if(continuous_swing_flag == TRUE) {
							jumping_rope_count++;
							g_total_day_jumprope++; /* v1.3.5 hkim 2015.10.28 */
						}
					}
					previous_continuous_swing_flag = continuous_swing_flag;
				}
			}
		
		

	

		


	
	
	
	
	
	
//	#ifndef AUTO_SLEEP
//	if(op_mode == DAILY_MODE && acc_norm < 1200 )//&& !is_charging )
//	{
//		pre_sleep_count++;
//		if(pre_sleep_count%20 ==0)
//		{
////			printf("\r\n pre_sleep_second : %d",pre_sleep_count/20);
//		}
//	}
//	else
//	{
//		pre_sleep_count = 0;
//		go_sleep = 0;
//	}
//	if(pre_sleep_count==1200)
//	{
//		pre_sleep_count = 0;
//		go_sleep = 1;
//	}
//	#endif /* AUTO_SLEEP*/
//	printf("\r\n acc_var_norm : [%d] op_mode : [%d]",acc_var_norm, op_mode);
//규창 수면 체크
		if(op_mode == SLEEP_MODE  && acc_var_norm > 8)
		{
			printf("\r\n acc_var_norm : [%.2f] op_mode : [%d] step[%d]",acc_var_norm, op_mode,step_count);

			if(chk_rolled_sleep_flag == false){
				
				printf("\r\n sleeping feature %d", step_count);
				//if(step_count < 10 )
				//{
					rolled_count++;		//뒤척임 증가
					slp_info.rolled_count = rolled_count;
					chk_rolled_sleep_flag = true;
					//printf("\r\n rolled_Count %d, %d", rolled_count, slp_info.rolled_count);
				//}	
			}
		
			if(chk_awake_sleep_flag == false){
				if(step_count > 12 )
				{
					awake_count++;		//깨어남 증가
					slp_info.awake_count = awake_count;
					chk_awake_sleep_flag = true;
					//printf("\r\n awake_Count %d, %d", awake_count, slp_info.awake_count);
				}
			}
			
		}
		else
		{
			chk_rolled_sleep_flag = false;
			chk_awake_sleep_flag = false;
			rolled_count = 0;
			awake_count = 0;
		}
		
	//	extern int today;
		
//		printf("\r\n today? %d GMT%d", today, (get_GMT(time.year,  time.month,  time.day,  time.hour,  time.minute, time.second )/(60)));
	if (count%NUM_OF_DATA ==0 )
	{
//		printf("\r\n acc_var_norm : [%d] op_mode : [%d] count[%d]",acc_var_norm, op_mode,count);
//		printf("\r\n acc_var_norm : %d",acc_var_norm);
		#ifdef POWER_SAVE_MODE_AND_AUTO_SLEEP_PROCESS
		/*#ifdef AUTO_SLEEP
		if(op_mode == SLEEP_MODE)
		{
			non_slp_count = 0;
		}
		else
		{
			non_slp_count ++;
		}
		#endif*/
		for(int i=0;i<9;i++)
		{
			slp_state[9-i]=slp_state[8	-i];
		}
		//규창 슬립조건
		if(acc_var_norm < 8 )//&& !is_charging)
		{
			slp_state[0]=1;
			slp_end_count =0;
		}
		//else /*수면 종료 조건*/
		//{
			/*#ifdef AUTO_SLEEP
			slp_state[0]=0;
			slp_end_count++;
			if(slp_end_count >= SLP_END_REF && op_mode == SLEEP_MODE)
			{
				int save_sate =0, save_count;
				uint8_t req_data[GC_FLASH_PAGE_DATA_SIZE]; // 4: address 
				_GC_TBL_STABLE_HR		_tbl_stb_hr;
				memset(&_tbl_stb_hr, 0, sizeof(_GC_TBL_STABLE_HR));
				memset(req_data, 0xff, sizeof(req_data));
				op_mode = DAILY_MODE;
				printf("\r\n SLEEP --> DAILY");
				gc_protocol_ctrl_pah8002(op_mode, 0);
				memcpy(req_data, &_tbl_stb_hr, sizeof(_GC_TBL_STABLE_HR));
				gc_flash_page_write_data(GC_FLASH_TBL_TYPE_STABLE_HR, req_data);
				tuch_count = (tuch_count == display_page_mode) ? display_page_clock : tuch_count;
			}
			#endif  AUTO_SLEEP*/
		///}
		slp_start_count = 0;
		for(int i=0;i<10;i++)
		{
//			printf("\r\n slp_state[%d] = %d",i,slp_state[i]);
			slp_start_count += slp_state[i];
		}
//		printf("\r\n slp_start_count : %d 8이상 수면 시작",slp_start_count);
//		printf("\r\n slp_end_count : %d 5이상 수면 끝",slp_end_count);
		//if(slp_start_count >=SLP_START_REF && op_mode == DAILY_MODE)
		if(slp_start_count >= 1 && op_mode == DAILY_MODE)
		{
			printf("\r\n HR CHK");
			/*#ifdef AUTO_SLEEP
			if(non_slp_count > NUM_OF_DATA*60*1)
			{
				rolled_count =0 ;
				awake_count =0 ;
				slp_info.slp_time = 0;
				slp_info.rolled_count =0;
				slp_info.awake_count =0;		
			}
			OLED_power_count=OLED_ON_count_sleep;
			#endif AUTO_SLEEP*/
			printf("\r\n hr_mode : %d ",pahr_mode);
			//gc_protocol_ctrl_pah8002(SLEEP_MODE, 1);
			//printf("\r\n hr_mode : %d ",pahr_mode);
			if(g_hr > 0)
			{
				/*#ifdef AUTO_SLEEP
				op_mode = SLEEP_MODE;
				printf("\r\n DAILY --> SLEEP");
				tuch_count=(OLED_switch) ? display_page_mode : display_page_mode+1;
				slp_end_count=0;
				#else
				gc_protocol_ctrl_pah8002(DAILY_MODE, 0);
				hr_ON_OFF = 0;
				#endif AUTO_SLEEP*/				
				for(int i=0;i<10;i++)
				{
					slp_state[i]=0;
				}
			}
			else
			{
				
				go_sleep = 1;
			}
			slp_chk_cnt++;
			printf("\r\n slp_chk_cnt : [%d]",slp_chk_cnt);
			printf("\r\n go_sleep : [%d]",go_sleep);
		}
		/*else if(slp_start_count == SLP_START_REF-1 && op_mode == DAILY_MODE)
		{
			gc_protocol_ctrl_pah8002(SLEEP_MODE, 1);
			hr_ON_OFF = 1;
//			printf("\r\n HR ON");
		}*/
		//else if(hr_ON_OFF == 1 && op_mode == DAILY_MODE)
		//{
		//	gc_protocol_ctrl_pah8002(DAILY_MODE, 0);
		//	hr_ON_OFF = 0;
//			printf("\r\n HR OFF");
		//}
		#endif //POWER_SAVE_MODE_AND_AUTO_SLEEP_PROCESS
		//printf("\r\n acc_var_norm : [%d] op_mode : [%d]",acc_var_norm, op_mode);
		/*if(op_mode == SLEEP_MODE && acc_var_norm > 8)
		{
			printf("\r\n sleeping feature %d", step_count);
			if(step_count < 10 )
			{
				rolled_count++;		//뒤척임 증가
				slp_info.rolled_count = rolled_count;

				printf("\r\n rolled_Count %d, %d", rolled_count, slp_info.rolled_count);
			}	
		
			else if(step_count > 12 )
			{
				awake_count++;		//깨어남 증가
				slp_info.awake_count = awake_count;
			}
		}
		else
		{
			rolled_count = 0;
			awake_count = 0;
		}*/
		if(op_mode == SLEEP_MODE){
			chk_rolled_sleep_flag = false;
			chk_awake_sleep_flag = false;
		}
		
	
		//규창 171016 지난1분의 피쳐 임시저장
		//tTime featuretime;
		//tTime featurestdTime;
		//get_time(&featuretime, 1);
		//convert_gmt_to_display_time(&featuretime, &featurestdTime);
		//featuredata[0].time = (get_GMT(time.year,  time.month,  time.day,  time.hour,  time.minute, time.second )/(86400))*1440;
		featuredata.time = (get_GMT(time.year,  time.month,  time.day,  time.hour,  time.minute, time.second )/(60));
		featuredata.norm_var = acc_var_norm * 100000;
		
		featuredata.x_var = acc_var_x_axis * 1000;
		featuredata.y_var = acc_var_y_axis * 1000;
		featuredata.z_var = acc_var_z_axis * 1000;
		
		featuredata.x_mean = acc_mean_x_axis * 1000;
		featuredata.y_mean = acc_mean_y_axis * 1000;
		featuredata.z_mean = acc_mean_z_axis * 1000;

		featuredata.norm_mean = acc_norm_mean * 1000;
		
		featuredata.nStep = step_count;
		featuredata.jumping_rope_count = jumping_rope_count;
		featuredata.small_swing_count		= small_swing_count;
		featuredata.large_swing_count		= large_swing_count;
		
		featuredata.pre_diff_sum = pre_diff_sum*100;
		
		featuredata.accumulated_step = g_accumulated_step;
		featuredata.display_step = g_total_daystep;
		
		featuredata.pressure = press * 100;
		featuredata.hb = g_hr;
		
		/*
	tTime time;
	tTime stdTime;
	get_time(&time, 1);
	convert_gmt_to_display_time(&time, &stdTime);
	if(gmt_offset ==0)
	{
		recall_GMT_offset();
	}*/
	/*
	printf("\r\n gmt_offset %d",gmt_offset);
	printf("\n\r %d-%d-%d %d:%d",time.year,time.month,time.day,time.hour,time.minute);
	printf("\n\r %d-%d-%d %d:%d",stdTime.year,stdTime.month,stdTime.day,stdTime.hour,stdTime.minute);
	*/
		
		
		/*printf("\r\n feature : n_var:%d pressure:%d prediff:%d x_mean:%d y_mean:%d z_mean:%d n_mean:%d x_var:%d y_var:%d z_var:%d",featuredata.norm_var, featuredata.pressure, featuredata.pre_diff_sum,featuredata.x_mean, featuredata.y_mean, featuredata.z_mean,featuredata.norm_mean,featuredata.x_var ,featuredata.y_var,	featuredata.z_var);
		printf("\r\n feature : step:%d rope_cnt:%d s_swing:%d l_swing:%d",featuredata.nStep,featuredata.jumping_rope_count ,featuredata.small_swing_count,featuredata.large_swing_count);
		printf("\r\n feature : hb:%d GMT:%d",featuredata.hb,get_GMT(time.year,  time.month,  time.day,  time.hour,  time.minute, time.second )/(60));
		*/
		
		//규창 171112 피처데이터 저장 기준
   extern volatile char g_product_mode;
	 //피쳐 저장은 gmt offset이 저장되어있어야 하고(동기화), 생산모드가 아닐때만 구동)
	 if(gmt_offset != -1 && g_product_mode == false){
		find_recent_feature_save(featuredata);
	 }
		
		step_count = 0;
		
		
					//save_feature_data( &data , time.minute );

		//규창 171013스윙 관련 변수들 초기화
		large_swing_count = 0;
			small_swing_count = 0;
			jumping_rope_count = 0;
			pre_diff_sum = 0; /* hkim Height 2015.6.5 */

//printf("\r\n hr_mode : %d ",pahr_mode);
//		tTime time;
//
//		tTime stdTime; /* hkim 1.3.9.10,  2016.1.26 세계시간 동기화 */
#if __STEP_SYNC_TEST__
		read_step_info_from_flash(FLASH_ACCUMULATED_STEP_PAGE);
#endif

		#if 1 /* hkim Height 2015.6.5,  (2015.7.22)if 0 --> if 1*/

		#else

		#endif /* end of hkim */

		

#if 1 /* v1.3.1 hkim 2015.10.6 */

#endif /* v1.3.1 */
		   			
		if(5 > g_battery  && ! is_charging && save_low_bat)
		{
			// 배터리 부족하면  저장
			
			int save_time = (get_GMT(time.year,  time.month,  time.day,  time.hour,  time.minute, time.second )/(86400))*1440;
			int save_step=0, save_cal=0;
			int i;
			
			uint8_t req_data[GC_FLASH_PAGE_DATA_SIZE]; /* 4: address */
			_GC_TBL_STEP				_tbl_step;
			_GC_TBL_CALORIE	_tbl_kcal;
			
			/* initialize structure */
			memset(&_tbl_step, 0, sizeof(_GC_TBL_STEP));
			memset(&_tbl_kcal, 0, sizeof(_GC_TBL_CALORIE));
			
			memset(req_data, 0xff, sizeof(req_data));
			
			_tbl_step.date	= save_time;
			_tbl_step.step	= step.display_step;
			memcpy(req_data, &_tbl_step, sizeof(_GC_TBL_STEP));
			
			for(i=0;i<=5;i++)
			{
				save_step = gc_flash_page_write_data(GC_FLASH_TBL_TYPE_BAT_STEP, req_data);
				nrf_delay_ms(10); /* FIXME */
				
				if(save_step)
				{
					i=6;
				}
			}				
			_tbl_kcal.date     = save_time;
			_tbl_kcal.daily		= kcal.step_cal;
			_tbl_kcal.sports	= kcal.activty_cal;
			_tbl_kcal.sleep		= kcal.sleep_cal;
			_tbl_kcal.coach	= kcal.coach_cal;
			memcpy(req_data, &_tbl_kcal, sizeof(_GC_TBL_CALORIE));
			
			for(i=0;i<=5;i++)
			{
				save_cal = gc_flash_page_write_data(GC_FLASH_TBL_TYPE_BAT_KCAL, req_data);
				nrf_delay_ms(10); /* FIXME */
				
				if(save_cal)
				{
					i=6;
				}
			}
		}
	}
//sys_synctime_get(&tyear, &tmon,&tday, &thour, &tmin, &tsec);
		if(get_time( &time, 1))
		{
			convert_gmt_to_display_time(&time, &stdTime);  /* hkim 1.3.9.10,  2016.1.26 세계시간 동기화 */
			//ks_printf("\r\n std Time =%d %d %d %d %d %d",stdTime.year, stdTime.month, stdTime.day, stdTime.hour, stdTime.minute, stdTime.second); 
			
			step.display_step =g_total_daystep ; 
			step.time = get_GMT(time.year,  time.month,  time.day,  time.hour,  time.minute, time.second )/60;
			
			/* GMT시간을 이용해 걸음수를 초기화 할 경우, 한국에서 아침 9시에 걸음수가 초기화 됨.
			    따라서 걸음수 초기화는 해당 국가의 표준시간으로 00:00 일 경우 초기화 한다 */
			//printf("\r\n %d/%d/%d %d:%d",stdTime.year,stdTime.month,stdTime.day,stdTime.hour,stdTime.minute);
			if(stdTime.hour==0 && stdTime.minute==0)
			{
				daily_reset = is_daily_save;
			}
			else
			{
				is_daily_save = 1;
			}
				//규창 171101 하루지나면 저장은 하는데 쓰는데가 없어서 무슨 의미가 있는지 모르겠다.....
			if(daily_reset == 1 && is_daily_save ==1)
			{
				
				// 하루 지나면 저장& reset
				
				
				int save_time = (get_GMT(time.year,  time.month,  time.day,  time.hour,  time.minute, time.second )/(86400))*1440;
				int week_day = ((get_GMT(time.year,  time.month,  time.day,  time.hour,  time.minute, time.second )/(86400))%7);
				int save_step=0, save_cal=0;
				int i;
				/*
				uint8_t req_data[GC_FLASH_PAGE_DATA_SIZE]; // 4: address 
				_GC_TBL_STEP				_tbl_step;
				_GC_TBL_CALORIE	_tbl_kcal;
				
				// initialize structure 
				memset(&_tbl_step, 0, sizeof(_GC_TBL_STEP));
				memset(&_tbl_kcal, 0, sizeof(_GC_TBL_CALORIE));
				
				memset(req_data, 0xff, sizeof(req_data));
				
				_tbl_step.date	= save_time;
				_tbl_step.step	= step.display_step;
				memcpy(req_data, &_tbl_step, sizeof(_GC_TBL_STEP));
				
				for(i=0;i<=5;i++)
				{
					save_step = gc_flash_page_write_data(GC_FLASH_TBL_TYPE_STEP, req_data);
					nrf_delay_ms(10); // FIXME 
					
					if(save_step)
					{
						i=6;
					}
				}
				
				_tbl_kcal.date     = save_time;
				_tbl_kcal.daily		= kcal.step_cal;
				_tbl_kcal.sports	= kcal.activty_cal;
				_tbl_kcal.sleep		= kcal.sleep_cal;
				_tbl_kcal.coach	= kcal.coach_cal;

				memcpy(req_data, &_tbl_kcal, sizeof(_GC_TBL_CALORIE));
				
				for(i=0;i<=5;i++)
				{
					save_cal = gc_flash_page_write_data(GC_FLASH_TBL_TYPE_KCAL, req_data);
					nrf_delay_ms(10); // FIXME 
					
					if(save_cal)
					{
						i=6;
					}
				}*/
				
				
				kcal.step_cal=0;
				kcal.activty_cal=0;
				kcal.sleep_cal=0;
				kcal.coach_cal=0;
				
				g_total_daystep=0;
				temp_step_count=0; //hkim 0729
				day_of_week = (week_day > 0 ) ? week_day : week_day+7;
				//day_of_week = (day_of_week>=7) ? 1: day_of_week+1;
				daily_reset = 0;
				is_daily_save = 0;
			}	
		}
	if(epoch > data_length_per_unittime*3500000) {
      //규창 171024epoch 초기화 추가
			epoch = 0;//previous_lower_peak_time =  previous_lower_peak_increase_time = previous_upper_peak_increase_time = 0;

	}		
}
		
/* hkim 1.3.9.10   2016.1.26 세계시간 동기화  로직*/
//volatile short tmp_gmt_offset;
/* pTime : GMT Time,  stdTime : 그 지역의 표준시 */
void convert_gmt_to_display_time( tTime* pTime, tTime* stdTime)
{
		unsigned int __gmt=0;
		
	//	printf("\r\n GMT =%d %d %d %d %d %d",pTime->year, pTime->month, pTime->day, pTime->hour, pTime->minute, pTime->second);
		__gmt = get_GMT(pTime->year, pTime->month, pTime->day, pTime->hour, pTime->minute, pTime->second);
	
	//printf("\r\n$$$$$$$$$$$$$ GMT offset %d", gmt_offset);
	//규창 171022 gmt_offset 임시로 한국 표준시 9시간 제공
	//gmt_offset = 540;
		__gmt = __gmt + (gmt_offset * 60);
			
		get_time_from_GMT(__gmt, stdTime);
		//ks_printf("\r\n standard time =%d %d %d %d %d %d",stdTime->year, stdTime->month, stdTime->day, stdTime->hour, stdTime->minute, stdTime->second); 
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
int is_leap_year(int a)
{
	return (((a % 4) == 0) && ((a % 100)!=0) )|| ((a % 400) == 0);  // 윤년계산 수식표현
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

char get_month_day(int year, int month)
{
	int day;

	day=month_day[month];

	if(month==2 && is_leap_year(year))
		day++;

	return day;
}

//규창 심박리셋기능으로 인해  리얼타임 가져올 때 count인수를 사용 못하게 함.
//대신 아래의 cnt를 사용
//unsigned int cnt = 0;
void get_realtime_HR(unsigned int count, unsigned char hr)
{
	//printf("\r\n get_realtime_HR enable");
//	cnt++;
	extern volatile unsigned char op_mode;
	unsigned char HR_stable = 70;//HR_info.HR_sta;
	//메인루틴.... 으로 이동
	//HR_sta = 0;//HR_info.HR_sta;
	//unsigned int gmt_flash=0;

	if((10*(hr)) > (8 * (220 - person_info.age - HR_stable))+10*HR_stable)
	{
		hr_warring = 1;
		if(vibration_count > 1200)
		{
			vibration_count = 0;
		}
	}
	else
	{
		hr_warring = 0;
	}
	
	
	/* 품평회로 인해 임시 주석
	if(hr<55)
	{
		hr_50_check_count++;
		if(hr_50_check_count > 600)
		{
			hr_50_check = 1;
		}
	}*/
	if((10*(hr)) > (5 * (220 - person_info.age - HR_stable))+10*HR_stable)
	{
		hr_50_check_count++;
		if(hr_50_check_count > 600)
		{
			hr_50_check = 1;
		}
	}
	else
	{
		hr_50_check_count =0;
		hr_50_check = 0;
		hr_under_50_check =1;
	}
	if(hr_50_check ==1 &&hr_50_delay==0 && (op_mode == ACT_MODE || op_mode ==COACH_MODE))
	{
		hr_50 = 1;
		hr_50_delay = 1;
		hr_50_delay_count  = 0;
		vibration_count = 0;
	}
	if(hr_50_delay == 1)
	{
		hr_50_delay_count++;
		if(hr_50_delay_count >6500)
		{
			hr_50_delay_count = 6500;
		}
	}
	if(hr_50_delay_count >= 6000 && hr_under_50_check ==1)
	{
		hr_50_delay = 0;
		hr_under_50_check =0;
	}
}


/*칼로리 계산 로직*/
void calorie (unsigned int step,float hr, unsigned char op_mode, unsigned int count)
{
	unsigned int cal_hr;
	unsigned int 	cal_step;
	
	if(op_mode!=DAILY_MODE)
	{
		
		if(hr||op_mode!=COACH_MODE)
		{
			cal_hr = calorie_hr(hr,op_mode);
		}
		else
		{
			cal_hr = 0;
		}
		
		cal_hr_1min += cal_hr;
		switch(op_mode)
		{
			case ACT_MODE:
			act_info.act_calorie += cal_hr;
			kcal.activty_cal += cal_hr;
			break;
			
			case SLEEP_MODE:
			kcal.sleep_cal += cal_hr;
			slp_info.slp_time++;
			break;
			
			case COACH_MODE:
			kcal.coach_cal += cal_hr;
			break;
			
			case STRESS_MODE:
			kcal.step_cal += cal_hr;
			break;
		}
	}
	
	if(count % 1200 == 0)
	{
		if(op_mode == DAILY_MODE )//|| former_op_mode == DAILY_MODE)
		{
			cal_step = calorie_step(step);
			kcal.step_cal += cal_step;
		}

//		if(op_mode == DAILY_MODE)
//		{
//			if(former_op_mode == DAILY_MODE)
//			{
//				kcal.step_cal += cal_step;
//			}
//			else
//			{
//				kcal.step_cal += (cal_step - cal_hr_1min);
//			}
//			
//			cal_hr_1min=0;
//		}

//		if(op_mode != DAILY_MODE)
//		{
//			if(former_op_mode == DAILY_MODE)
//			{
//				kcal.step_cal += (cal_step - cal_hr_1min);
//			}
//				
//			cal_hr_1min=0;
//		}
//		former_op_mode= op_mode;
	}
}

unsigned int calorie_hr (unsigned char hr, unsigned char op_mode)
{
	unsigned int cal_return = 0;
	unsigned int cal_hr = 0;
	unsigned int	cal_basal = 0;
	
		if(person_info.sex == 1) // 남성
		{
			cal_hr = (unsigned int)(((-8477.604F + (person_info.weight * 6.481F) + (hr * 51.426F) + (person_info.weight * hr * 1.018F)) * (sumInterval / 60.0F / 1000.0F))*(10.0F/9.0F)*1000.0F);
			cal_basal = (unsigned int) (88.362F+ (13.397F * person_info.weight) + (4.799F * person_info.height) - (5.677F * person_info.age))*(sumInterval/60.0F/60.0F/24.0F)*(10.0F/9.0F)*1000.0F;
		}
		else if(person_info.sex == 2) // 여성
		{
			cal_hr = (unsigned int)(((100.127F + (person_info.weight * -106.729F) + (hr * 12.580F) + (person_info.weight * hr * 1.251F)) * (sumInterval / 60.0F / 1000.0F))*(10.0F/9.0F)*1000.0F);
			cal_basal = (unsigned int) (447.593F+ (9.247F * person_info.weight) + (3.098F * person_info.height) - (4.330F * person_info.age))*(sumInterval/60.0F/60.0F/24.0F)*(10.0F/9.0F)*1000.0F;
		}
		
		cal_return = (cal_hr > cal_basal ) ? cal_hr : cal_basal;
	return cal_return;
}

unsigned int calorie_step(unsigned int step)
{
	unsigned int cal_step = 0;
	float METs;
	if(step>150)
		{
			METs = 8.3F;
		}
		else if(step>140)
		{
			METs = 7.0F;
		}
		else if(step>130)
		{
			METs = 5.0F;
		}
		else if(step>120)
		{
			METs = 4.3F;
		}
		else if(step>110)
		{
			METs = 3.5F;
		}
		else if(step>105)
		{
			METs = 3.0F;
		}
		else if(step>90)
		{
			METs = 2.8F;
		
		}else if(step>80)
		{
			METs = 2.0F;
		}
		else if(step>30)
		{
			METs = 1.3F;
		}
		else
		{
			METs = 1.2F;
		}
		cal_step = (unsigned int)((METs * person_info.weight * sumInterval *30.0F/60.0F * 0.0175F)*(10.0F/9.0F)*1000.0F);
	
	return cal_step;
}



/*운동 강도 계산 로직*/
void activity_intensity(unsigned char op_mode)
{
	unsigned char HR_stable;
	unsigned char act_percent ;
	
	if(op_mode==ACT_MODE)
	{
		//규창 안정심박 임시 70
		HR_stable = 70; //((HR_info.HR_sta > 20)&&(HR_info.HR_sta < 140)) ? HR_info.HR_sta : 60;
		//규창 디스플레이에 있는 활동강도 구하는 방식으로 변경 아직도 왜 1분마다 호출하는지는..?
		act_percent = (g_hr - HR_stable>0) ? (unsigned char)((g_hr - HR_stable) / (220.0f - person_info.age - HR_stable)*100) : 0;
		//규창 전임자의 이상한 평균 수식으로 구하는데.. 이게 현재 방식에서 제대로 나오나..?
		//act_percent = (unsigned char)((float)(HR_info.HR_avg - HR_stable) / (220.0f - person_info.age - HR_stable)*100);
		//act_percent = (unsigned char)((float)(HR_avg_1min) / (220.0f - person_info.age)*100);
//		act_time++;
//		act_info.act_time=act_time;
		act_info.act_time++;
		//printf("\r\n HR_stable[%d], act_percent[%d]", HR_stable, act_percent);
		//printf("\r\n act_info.act_time = %d",act_info.act_time);
		if(act_percent>80)
		{
			danger_count++;
			act_info.danger_count = danger_count;
		}
		else if(act_percent>60)
		{
			strong_count++;
			act_info.strong_count = strong_count;
		}
		else if(act_percent>50)
		{
			middle_count++;
			act_info.middle_count = middle_count;
		}
		else
		{
			weak_count++;
			act_info.weak_count = weak_count;
		}
	}
	//규창 심박값 초기화 플래그 함께 보도록
	else
	{
		weak_count = middle_count = strong_count = danger_count = 0;
		act_time = 0;
		act_info.act_time = 0;
	}
	
}

int f_info_recall(void)
{
	//int recall_time = get_GMT(time.year,  time.month,  time.day,  time.hour,  time.minute, time.second )/(60*60*240);
	int recall_time  = 0;
	char recall_cal =0;
	char recall_step =0;
	char i=0;

	uint8_t req_data[GC_FLASH_PAGE_DATA_SIZE]; /* 4: address */
	_GC_TBL_STEP													_tbl_step;
	_GC_TBL_CALORIE										_tbl_kcal;
//	_GC_TBL_MEASURE_ACTIVITY		_tbl_act;
	
	//convert_gmt_to_display_time(&time, &stdTime);
	get_time( &time, 1);
	printf("\r\n gmt_time =%d/%d/%d %d:%d",time.year,  time.month,  time.day,  time.hour,  time.minute);	
	recall_time  = (get_GMT(time.year,  time.month,  time.day,  time.hour,  time.minute, time.second )/(86400))*1440;				
	printf("\r\n recall_time =%d",recall_time);	
	/* initialize structure */
	memset(&_tbl_step, 0, sizeof(_GC_TBL_STEP));
	memset(&_tbl_kcal, 0, sizeof(_GC_TBL_CALORIE));
//	memset(&_tbl_act, 0, sizeof(_GC_TBL_MEASURE_ACTIVITY));
	memset(req_data, 0xff, sizeof(req_data));
	nrf_delay_ms(10);
	
		for(i=0;i<=5;i++)
		{
			recall_step = gc_flash_page_read_data(GC_FLASH_TBL_TYPE_BAT_STEP,recall_time,req_data);
			nrf_delay_ms(10); /* FIXME */
			printf("\r\n i = %d ", i);
			if(recall_step)
			{
				i=6;
			}
		}
		memcpy(&_tbl_step, req_data, sizeof(_tbl_step));
		printf("\r\n _tbl_step.date = %d",_tbl_step.date);
		if(_tbl_step.date == recall_time)
		{
			//step.display_step	=	step.display_step	+	(_tbl_step.step);
			g_total_daystep = g_total_daystep + (_tbl_step.step);
		}
	
		
		
	memset(req_data, 0xff, sizeof(req_data));
	for(i=0;i<=5;i++)
	{
		
		recall_cal = gc_flash_page_read_data(GC_FLASH_TBL_TYPE_BAT_KCAL,recall_time,req_data);
		nrf_delay_ms(10); /* FIXME */
		
		if(recall_cal)
		{
			i=6;
		}
	}
	memcpy(&_tbl_kcal, req_data, sizeof(_tbl_kcal));
	printf("\r\n _tbl_step.date = %d",_tbl_kcal.date);
	if(_tbl_kcal.date == recall_time)
	{
		kcal.step_cal	=	kcal.step_cal	+	(_tbl_kcal.daily);
		kcal.activty_cal	=	kcal.activty_cal	+	(_tbl_kcal.sports);
		kcal.sleep_cal	=	kcal.sleep_cal	+	(_tbl_kcal.sleep);
		kcal.coach_cal	=	kcal.coach_cal	+	(_tbl_kcal.coach);
	}	

//	if(gc_flash_page_read_data(GC_FLASH_TBL_TYPE_ACT,recall_time,req_data))
//	{
//		memcpy(&_tbl_act, req_data, sizeof(_tbl_act));
//		if(op_mode!=ACT_MODE)
//		{
//			act_info.act_time	=	_tbl_act.time;
//			act_info.act_calorie	=	_tbl_act.calorie;
//			act_info.danger_count	=	_tbl_act.intensity_danger;
//			act_info.strong_count	=	_tbl_act.intensity_high;
//			act_info.middle_count	=	_tbl_act.intensity_mid;
//			act_info.weak_count	=	_tbl_act.intensity_low;
//			act_info.hr_min	=	_tbl_act.hr_min;
//			act_info. hr_avg	=	_tbl_act.hr_avg;
//			act_info.hr_max	=	_tbl_act.hr_max;
//		}
//	}
					
	return true;
}


