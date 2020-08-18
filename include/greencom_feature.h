



#ifndef __GREENCOM_FEATURE_H__
#define __GREENCOM_FEATURE_H__

#include "stdint.h"
#include "ble_nus.h"



struct feature_tbl_form_s
{
	unsigned int time;		//	gmt 2015 minute time

	unsigned int pressure;		//	기압센서 
	
	int 	norm_var;
	
	short norm_mean;
	short x_mean;
	
	short y_mean;
	short z_mean;

	short x_var;
	short y_var;

	short z_var;
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

	unsigned char		hb;
	unsigned char		battery;
	
};


/****************************************************************************/
//by skim


typedef struct
{
	unsigned short	year;
	unsigned char		month;
	unsigned char		day;
	unsigned char		hour;
	unsigned char		minute;
	unsigned char		second;
} tTime;


typedef struct
{
	unsigned char		sex;
	unsigned char		age;
	unsigned char		height;
	unsigned char		weight;
} PERSONAL_INFO;

#if 1 /* v1.3.9 hkim 2015.12.05 */
struct flash_step_info
{
	unsigned int	time;
	unsigned int step;
};
#endif /* v1.3.9 */

void set_day_step(void);
#if 1 /* v1.5.13.0 hkim 2016.3.23. fill empty feature data */
//int is_previous_feature_empty(struct feature_tbl_form_s* pdata); 
int check_feature_empty(void);
void copy_last_feature(void);
#endif 
#if 1 /* v1.5.13.0 hkim 2016.3.23. writing period 1min */
	int save_feature_data(struct feature_tbl_form_s* pdata, unsigned char min);
	int get_feature_data_page_from_top(uint32_t s_gmt, uint32_t e_gmt, unsigned short* s_page, unsigned short* e_page, unsigned int* s_time, unsigned int *e_time);
#else /* v1.5.13.0 hkim 2016.3.23. writing period 1min */
	int save_feature_data(struct feature_tbl_form_s* pdata);
#endif 
int load_feature_data(struct feature_tbl_form_s* pdata);
int erase_feature_data(int flag);
	
unsigned short get_queued_number(void);
unsigned short get_read_page_pointer(void);
unsigned short get_write_page_pointer(void);
void init_db_write_page(void);

	
void feature_flash_fwrite(unsigned short  page, void* data );
void feature_flash_read(unsigned short  page, void* data );
void feature_flash_erase(unsigned short  page);

void check_flash_db_data(char*);

int Flash_Read_Text( int page, char* flash_str);
int Flash_Write_Text( int page, char* flash_str);

void find_first_emtpy_page(void);



void get_realtime_feature(unsigned int count, short x, short y, short z);
#if 1 /* v1.2.5 2015.10.4 */
#define HR_BUF_DEF 10
#define HR_BUF_FIRST_FIX_HR	75 /* v1.3.6 */
int greencomm_hr_alarm(unsigned char __hr);
#endif


int get_time( tTime* pTime, int flag); 

int get_time_from_GMT( unsigned int gmt, tTime* pTime);
unsigned int get_GMT(unsigned int year, unsigned char month, unsigned char day, unsigned char hour, unsigned char min, unsigned char sec);

unsigned short get_feature_data_page(int gmt_min, unsigned int* time);
int get_feature_data_page_from_top(uint32_t s_gmt, uint32_t e_gmt, unsigned short* s_page, unsigned short* e_page, unsigned int* s_time, unsigned int *e_time); //첫 페이지부터 마지막까지 그냥 순차적으로 검사


/* ----------------------------- */
void bl_st_feature_make_data(struct feature_tbl_form_s *fdb_ptr);
int bl_st_feature_report_sequence(ble_nus_t*	p_nus, unsigned char sequence);
#if 1 /* v1.3.9 hkim 2015.12.05 */
#define MAX_FLASH_STEP_INFO	7 /* 7 day */
void step_flash_write(unsigned short  page, void* data );
void step_flash_read(unsigned short  page, void* data );

void read_step_info_from_flash(unsigned int address);
int get_step_info_from_flash(unsigned int address, unsigned int gmt, struct flash_step_info *step_info);
void save_step_info_to_flash(unsigned int address, unsigned int gmt_flash,unsigned int step);
#endif /* v1.3.9 */

#if 1 /* v1.4.10.0 hkim 2016.02.12 */
int get_gmt_offset(void);
int save_gmt_offset(short __gmt_offset);
void read_gmt_offset_from_flash(unsigned short  page, void* data );
#endif

#if 1 /* v1.4.11.1 hkim 2016.02.17 */
#define PAGE_NUM_IN_SECTOR								16
#define PAGE_SIZE																		256 		/* byte */
#define SECTOR_NUM_FOR_FEATURE				256

#if __FEATURE_SAVE_LONG_PERIOD__
	#define PAGE_ALL_NUM														9504
	#define SECTOR_ALL_NUM												594		/* 9504 page --> 33 day*/ /* hkim 2016.5.3 */
#else
	#define PAGE_ALL_NUM														4096
	#define SECTOR_ALL_NUM												256		/* 4096 page */  /* hkim 2016.5.3 */
#endif

#define SECTOR_SIZE																4096		/* 1 sector size (byte) */
#define FEATURE_TIME_LEN											4
#define FEATURE_NUM_IN_FLASH_PAGE		5
#define MAX_FEATURE_LEN												44
#define FEATURE_SAVE_PERIOD_MIN				5				/* 5 minutes */
#define SYS_POWER_OFF_MIN									10			/* 방전으로 판단하는 기준 : 20 분 */
#define MAX_FEAT_PER_PAGE									5		/* 1page에는 5분 피쳐 저장 가능 */
#define MAX_BT_TX_LEN														43

void print_feature_bt_tx_format(int __gmt);
void write_feature_manual(unsigned int start_gmt, unsigned int end_gmt);
void save_gmt_offset_to_flash(unsigned short  page, void* data );
#endif

void convert_gmt_to_display_time( tTime* pTime, tTime* stdTime);

extern unsigned short g_total_daystep;
extern PERSONAL_INFO		g_personal_info;

#endif

