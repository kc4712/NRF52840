#ifndef _FLASH_CONTROL_H_
#define _FLASH_CONTROL_H_

#include "nrf.h"
#include "gc_activity.h"

/* 2M flash memory layout */
#define FLASH_SECTOR_NUM										500		/* 2048000 / ( 1 page size )* (page number in sector) ) = 2048000 / (256 * 16) */
#define FLASH_PAGE_NUM_IN_SECTOR		16

#define GC_FLASH_PAGE_SIZE									254 		/* byte length of 1 page. byte, 250 (data) + 4(address) */
#define GC_FLASH_PAGE_DATA_SIZE				250
#define GC_FLASH_PAGE_ADDR_SIZE				4



/**
 * @brief table info of flash layout for iBody24 CoachFit. Following number is page index.
 *
 * Layout target is that each information maintained for at least 30 days 
 * 3 Sector is allocated to each table information 
 */
#define GC_FLASH_SECTOR_NUM_STEP		3				/* 0~15, 16~31, 31 ~ 47 page */
#define GC_FLASH_PAGE_START_STEP		0				/* page index 0(=0th page) */
#define GC_FLASH_PAGE_END_STEP				47

#define GC_FLASH_SECTOR_NUM_KCAL		3
#define GC_FLASH_PAGE_START_KCAL		48
#define GC_FLASH_PAGE_END_KCAL				95

#define GC_FLASH_SECTOR_NUM_ACT			3
#define GC_FLASH_PAGE_START_ACT			96
#define GC_FLASH_PAGE_END_ACT					143

/*#define GC_FLASH_PAGE_SN										1584		//0x63000
#define GC_FLASH_PAGE_GMT_OFFSET		1600		//0x64000
#define GC_FLASH_PAGE_STABLE_HR			1616		//0x65000
#define GC_FLASH_PAGE_PERSINFO					1632		//0x66000

#define GC_FLASH_PAGE_BAT_STEP				1648		//0x67000
#define GC_FLASH_PAGE_BAT_KCAL					1664 	//0x68000
#define GC_FLASH_PAGE_BAT_ACT					1680		//0x69000

#define GC_FLASH_PAGE_POWER_OFF			1696		//0x6a000
#define GC_FLASH_PAGE_RESET_LOG			1712		//0x6b000
#define GC_FLASH_PAGE_RESET_DATA		1728		//0x6c000*/



#define GC_FLASH_PAGE_SN										144		//0x9000
#define GC_FLASH_PAGE_GMT_OFFSET		160		//0xA000
#define GC_FLASH_PAGE_STABLE_HR			176		//0xB000
#define GC_FLASH_PAGE_PERSINFO					192		//0xC000

#define GC_FLASH_PAGE_BAT_STEP				208		//0xD000
#define GC_FLASH_PAGE_BAT_KCAL					224 	//0xE000
#define GC_FLASH_PAGE_BAT_ACT					240		//0xF000

#define GC_FLASH_PAGE_POWER_OFF			256		//0x10000
#define GC_FLASH_PAGE_RESET_LOG			272		//0x11000
#define GC_FLASH_PAGE_RESET_DATA		288		//0x12000



//규창 171017 시간데이터는 1600 부터 기록 하여 10분 후인 1760까지
/*
#define GC_FLASH_SECTOR_NUM_TIME 16
#define GC_FLASH_PAGE_START_TIME 1600
#define GC_FLASH_PAGE_END_TIME 1760
*/
//규창 171017 시간데이터는 1600 부터 기록 하여 60분 후인 2560까지

#define GC_FLASH_SECTOR_NUM_TIME 16

//1day 규창 171101
#define GC_FLASH_PAGE_START_TIME 1600
#define GC_FLASH_PAGE_END_TIME 3039


//2day 규창 171101
#define GC_FLASH_PAGE_START_DAY2_TIME 3040
#define GC_FLASH_PAGE_END_DAY2_TIME 4479

//3day 규창 171101
#define GC_FLASH_PAGE_START_DAY3_TIME 4480
#define GC_FLASH_PAGE_END_DAY3_TIME 5919



//규창 171030 하루치 시간 4등분... 탐색시 너무 오래 걸린다.... 4등분범위 5760
#define GC_FLASH_PAGE_SLICE_4_1 




//규창 171017 플래시 마지막 위치
//#define LAST FLASH PAGE 65520  //FFF0 ~ FFFF

/* flash table type */
#define GC_FLASH_TBL_TYPE_STEP										0x01
#define GC_FLASH_TBL_TYPE_KCAL										0x02
#define GC_FLASH_TBL_TYPE_ACT											0x03
#define GC_FLASH_TBL_TYPE_SN												0x04
#define GC_FLASH_TBL_TYPE_GMT_OFFSET			0x05
#define GC_FLASH_TBL_TYPE_STABLE_HR					0x06		/* stable heart rate which calculated during sleep */
#define GC_FLASH_TBL_TYPE_PERSINFO						0x07		/* personal information, age weigth etc */
#define GC_FLASH_TBL_TYPE_BAT_STEP						0x08
#define GC_FLASH_TBL_TYPE_BAT_KCAL						0x09
#define GC_FLASH_TBL_TYPE_BAT_ACT							0x0A 
#define GC_FLASH_TBL_TYPE_POWER_OFF				0x0B
#define GC_FLASH_TBL_TYPE_RESET_LOG				0x0C
#define GC_FLASH_TBL_TYPE_RESET_DATA				0x0D


/* flag for SPI interrupt context */
#define GC_FLASH_WRITE_ENABLE_TO_ERASE		1
#define GC_FLASH_WRITE_ENABLE_TO_WRITE		2
#define GC_FLASH_READ	3

/* misc */
#define GC_FLASH_NO_DATA						0xff


typedef struct gc_tbl_step
{
	uint32_t	date; /*year/month/day */
	uint32_t	step;
}_GC_TBL_STEP;


typedef struct gc_tbl_calorie
{
	uint32_t	date; 						/* year/month/day */
	uint32_t	daily;						/* daily activity */
	uint32_t	sports;					/* measurement of sports activity */
	uint32_t	sleep;					/* measure of sleeping */
	uint32_t	coach;					/* fitness */
}_GC_TBL_CALORIE;


typedef struct gc_tbl_measure_activity  /*16.10.11 h.m LEE 프로토콜과 동일한 순서로 변경*/
{
	uint32_t	time;								/* start GMT */
	
	uint32_t	calorie;							/* total consumed calorie */
	
	uint16_t	intensity_low;			/* activity intensity-low */
	uint16_t	intensity_mid;			/* activity intensity-middle */
	uint16_t	intensity_high;		/* activity intensity-high */
	uint16_t	intensity_danger;		/* activity intensity-danger */
	
	uint16_t	hr_min;							/* minimum heart rate */
	uint16_t	hr_max;							/* maximum heart rate */
	uint16_t	hr_avg;							/* average heart rate */
}_GC_TBL_MEASURE_ACTIVITY;

typedef struct gc_tbl_stable_hr
{
	uint32_t	stable_hr;
}_GC_TBL_STABLE_HR;

typedef struct gc_tbl_personal_info
{
	uint16_t	f_gender;
	uint16_t	f_age;
	uint16_t	f_height;
	uint16_t	f_weight;
}_GC_TBL_PERSINFO;

typedef struct save_for_sleep
{
		 uint32_t	sleep_start_year;       	//슬림모드 진입 년
		 uint32_t	sleep_start_month;    	//슬림모드 진입 월
		 uint32_t	sleep_start_day;        	//슬림모드 진입 일
		 uint32_t	sleep_start_hour;      		//슬림모드 진입 시
		 uint32_t	sleep_start_minute;    	//슬림모드 진입 분
		 uint32_t	sleep_start_second;  	//슬림모드 진입 초
		 uint32_t	step;                     				//걸음수
		 uint32_t	step_cal;                 			//op_mode : 1 일상 칼로리
		 uint32_t	activty_cal;              			//op_mode : 2 활동 칼로리
		 uint32_t	sleep_cal;                		//op_mode : 4  수면 칼로리
		 uint32_t	coach_cal;               		//op_mode : 5 코치 칼로리
}_GC_POWER_OFF_INFO_;

typedef struct log_for_reset
{
	uint32_t	sleep_start_year;       	
	uint32_t	sleep_start_month;    	
	uint32_t	sleep_start_day;        	
	uint32_t	sleep_start_hour;      		
	uint32_t	sleep_start_minute;    	
	uint32_t	sleep_start_second;  	
	uint32_t	step;                     				
	uint32_t	step_cal;                 			
	uint32_t	activty_cal;              			
	uint32_t	sleep_cal;                		
	uint32_t	coach_cal;               		
	uint32_t	reset_point;               		
	uint32_t	reset_info_1;
	uint32_t	reset_info_2;
	uint32_t	reset_info_3;
	uint32_t	reset_info_4;
}_GC_RESET_lLOG_;

typedef struct data_before_reset
{
	uint32_t	resert_start_year;       	
	uint32_t	resert_start_month;    	
	uint32_t	resert_start_day;        	
	uint32_t	resert_start_hour;      		
	uint32_t	resert_start_minute;    	
	uint32_t	resert_start_second;  	
	uint32_t	step;                     				
	uint32_t	step_cal;                 			
	uint32_t	activty_cal;              			
	uint32_t	sleep_cal;                		
	uint32_t	coach_cal;               		
	uint32_t	op_mode; 
	uint32_t	slp_time;
	uint32_t	rolled_count;               		
	uint32_t	awake_count;
	uint32_t	weak_count;
	uint32_t	middle_count;
	uint32_t	strong_count;
	uint32_t	danger_count;
	uint32_t  act_time;
	uint32_t	HR_max;
	uint32_t	HR_min;
	uint32_t	HR_avg;
}_GC_DATA_BRFOR_RESET;


//규창 171013 피쳐 데이터 포맷추가
typedef struct feature_format
{
	uint32_t time;		//	gmt 2015 minute time

	uint32_t 	norm_var;
	uint32_t x_var;
	uint32_t y_var;
	uint32_t z_var;
	
	uint32_t x_mean;
	uint32_t y_mean;
	uint32_t z_mean;
  uint32_t norm_mean;
	
	uint32_t 	nStep;
		
	uint32_t	jumping_rope_count;
	uint32_t small_swing_count;
	uint32_t  large_swing_count;
	
	uint32_t	pre_diff_sum;
#if 0 /* v1.3.7 hkim 2015.11.11 활성화 시키면 밴드가 부팅 중 멈춤. SN만 출력하고 더 이상 진행안됨 */
	uint32_t	amp_rtsl; 
	uint32_t	stepFreq_rtsl;
#endif
	uint32_t accumulated_step; /* v1.3.9 hkim 2015.12.05 밴드가 부팅한 이후 밴드에서 측정된 누적된 걸음수 */
	uint32_t display_step;
  
	uint32_t pressure;		//	기압센서  
	uint32_t hb;
	uint32_t battery;
}_GC_FEATURE_DATA;



//규창 171018 피쳐 저장, 전체확인 함수 
void find_recent_feature_save(struct feature_tbl_form_s featuredata);
void savefeatureshow();
struct feature_tbl_form_s find_req_feature(unsigned int req_time);
void FlashAllErase();
void show_feature(unsigned int times);
void findpaststartADDR();
void find_recent_feature_save_on_boot();
struct feature_tbl_form_s find_req_past_feature(uint16_t olderstAddr, unsigned int req_time);


void feature_flash_read(uint16_t page, void* data );
void feature_flash_fwrite(uint16_t page, void* data );
void feature_flash_erase( uint16_t  page);

void gc_flash_read(uint16_t  page_index, uint8_t * data );
int32_t gc_flash_fwrite(uint32_t  page_index, uint8_t* data );
void gc_flash_erase(uint16_t  page_index);

uint32_t gc_flash_page_start(uint32_t table_type);
uint16_t gc_flash_page_read_data(uint32_t table_type, uint32_t req_time, uint8_t *req_buf);
int32_t gc_flash_page_write_data(uint32_t table_type, uint8_t *req_buf);

uint32_t move_flash_page_write_position(uint32_t table_type);
void get_flash_next_sector(uint32_t table_type, uint32_t page_index);
uint32_t get_flash_page_write_position(uint32_t table_type);
void find_flash_page_write_position_at_boot(void);

#endif 
