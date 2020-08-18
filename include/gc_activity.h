#ifndef _GC_ACTIVITY_H_
#define _GC_ACTIVITY_H_

#include "gc_time.h"

#define DAILY_MODE 1
#define ACT_MODE  2
#define STRESS_MODE 3
#define SLEEP_MODE 4
#define COACH_MODE  5

#define NUM_OF_DATA 1200

#ifdef AUTO_SLEEP
#define SLP_START_REF 8
#else
#define SLP_START_REF 3
#endif
#define SLP_END_REF 5

struct STEP_INFO
{
	unsigned int time;		//	gmt 2015 day time
	
	//unsigned int accumulated_step; /* v1.3.9 hkim 2015.12.05 밴드가 부팅한 이후 밴드에서 측정된 누적된 걸음수 */	
	
	unsigned int display_step; //일일 걸음수
	
};

struct PERSONAL_INFO
{
	unsigned char sex;
	unsigned char age;
	unsigned char height;
	unsigned char weight;
};

struct SLEEP_INFO
{
	unsigned int slp_time; // 총 수면 시간(분)

	unsigned short rolled_count;
	unsigned short awake_count;	
};

struct ACTIVITY_INFO
{
	unsigned int act_time; // 총 운동 시간(분)

	unsigned int act_calorie;
	
	unsigned short weak_count;

	unsigned short middle_count;
	
	unsigned short strong_count;

	unsigned short danger_count;
	
	unsigned short empty_1;	
};

struct flash_step_info
{
	unsigned int	time;

	unsigned int step;
};

struct s_hart_rate
{
	unsigned char HR_max;
	unsigned char HR_min;
	unsigned char HR_avg;
	unsigned char HR_sta;
//	unsigned int time;
//	unsigned int  non2;
};

struct calorie_save
{
	unsigned int date;			// 날짜

	unsigned int step_cal;			//op_mode : 1 일상
	unsigned int activty_cal;		//op_mode : 2 활동
	
	unsigned int sleep_cal;		//op_mode : 4  수면
	unsigned int coach_cal;	//op_mode : 5 코치
};


//규창 171013 피쳐 데이터 포맷추가
struct feature_tbl_form_s
{
	unsigned int time;		//	gmt 2015 minute time

	int 	norm_var;
	short x_var;
	short y_var;
	short z_var;
	short x_mean;	
	short y_mean;
	short z_mean;
  short norm_mean;
	
	short 	nStep;
		
	unsigned short	jumping_rope_count;
	unsigned short  small_swing_count;

	unsigned short  large_swing_count;
	short	pre_diff_sum;
#if 0 /* v1.3.7 hkim 2015.11.11 활성화 시키면 밴드가 부팅 중 멈춤. SN만 출력하고 더 이상 진행안됨 */
	short	amp_rtsl; 
	short	stepFreq_rtsl;
#endif
	unsigned int 		accumulated_step; /* v1.3.9 hkim 2015.12.05 밴드가 부팅한 이후 밴드에서 측정된 누적된 걸음수 */
	unsigned short			display_step;
  unsigned int pressure;		//	기압센서 
	unsigned char		hb;
	unsigned char		battery;
	
};



void greencom_step_process(unsigned int count, short acc_x, short acc_y, short acc_z);
void get_realtime_step(unsigned int count, short acc_x, short acc_y, short acc_z);
void convert_gmt_to_display_time( tTime* pTime, tTime* stdTime);
int get_time_from_GMT( unsigned int gmt, tTime* pTime);
char get_month_day(int year, int month);
unsigned int get_GMT(unsigned int year, unsigned char month, unsigned char day, unsigned char hour, unsigned char min, unsigned char sec);
int is_leap_year(int a);
void greencom_HR_process(unsigned int count);
void get_realtime_HR(unsigned int count, unsigned char hr);
//void step_flash_write(unsigned short  page, void* data );
//void save_step_info_to_flash(unsigned int address, unsigned int gmt_flash, unsigned int step);
//void feature_flash_fwrite(unsigned short  page, void* data );
//void feature_flash_read(unsigned short  page, void* data );
//void feature_flash_erase(unsigned short  page);

//규창 함수 원형 변경... 메인에서 평균 심박 넣어다 콜
//void activity_intensity(unsigned char op_mode, unsigned char HR_avg);
void activity_intensity(unsigned char op_mode);

int f_info_recall(void);
float getFloat(int integer);
#endif 

#if 0
void calorie (unsigned int step,float hr, unsigned char op_mode);
#else
void calorie (unsigned int step,float hr, unsigned char op_mode, unsigned int count);
unsigned int calorie_hr (unsigned char hr, unsigned char op_mode);
unsigned int calorie_step(unsigned int step);
//void get_press(void);

#endif

