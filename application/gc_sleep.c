#include <stdio.h>
#include <string.h>
#include "nrf_error.h"
#include "nrf_drv_config.h"
#include "app_util_platform.h"
#include "nrf_gpio.h"
#include "gc_gpio.h"
#include "gc_sleep.h"
#include "pah8002.h"
#include "softdevice_handler.h"
#include "flash_control.h"
#include "spi_flash.h"
#include "gc_activity.h"			/*16.12.08 h.m Lee*/
#include "oled.h"			/*16.12.08 h.m Lee*/
#include "define.h"
#include "print.h"

static uint32_t g_sleep_count;
extern volatile uint8_t pahr_mode;
extern volatile struct calorie_save kcal;			/*16.12.08 h.m Lee*/
extern volatile unsigned int g_total_daystep;			/*16.12.08 h.m Lee*/
extern volatile struct SLEEP_INFO slp_info;
extern volatile struct ACTIVITY_INFO act_info;
extern volatile unsigned char op_mode;
extern volatile struct s_hart_rate HR_info;
extern tTime time;			/*16.12.08 h.m Lee*/
tTime slp_time;			/*16.12.08 h.m Lee*/

#define NRF52_ONRAM1_OFFRAM1  	POWER_RAM_POWER_S0POWER_On      << POWER_RAM_POWER_S0POWER_Pos      \
												      | POWER_RAM_POWER_S1POWER_On      << POWER_RAM_POWER_S1POWER_Pos      \
												      | POWER_RAM_POWER_S0RETENTION_On  << POWER_RAM_POWER_S0RETENTION_Pos  \
	                            | POWER_RAM_POWER_S1RETENTION_On  << POWER_RAM_POWER_S1RETENTION_Pos; 
												
#define NRF52_ONRAM1_OFFRAM0    POWER_RAM_POWER_S0POWER_On      << POWER_RAM_POWER_S0POWER_Pos      \
												      | POWER_RAM_POWER_S1POWER_On      << POWER_RAM_POWER_S1POWER_Pos      \
												      | POWER_RAM_POWER_S0RETENTION_Off << POWER_RAM_POWER_S0RETENTION_Pos  \
	                            | POWER_RAM_POWER_S1RETENTION_Off << POWER_RAM_POWER_S1RETENTION_Pos;														
												
#define NRF52_ONRAM0_OFFRAM0    POWER_RAM_POWER_S0POWER_Off     << POWER_RAM_POWER_S0POWER_Pos      \
												      | POWER_RAM_POWER_S1POWER_Off     << POWER_RAM_POWER_S1POWER_Pos;
														
void configure_ram_retention(void)
{
	NRF_POWER->RAM[0].POWER = NRF52_ONRAM1_OFFRAM1;
	NRF_POWER->RAM[1].POWER = NRF52_ONRAM1_OFFRAM1;
	NRF_POWER->RAM[2].POWER = NRF52_ONRAM1_OFFRAM1;
	NRF_POWER->RAM[3].POWER = NRF52_ONRAM1_OFFRAM1;
	NRF_POWER->RAM[4].POWER = NRF52_ONRAM1_OFFRAM1;
	NRF_POWER->RAM[5].POWER = NRF52_ONRAM1_OFFRAM1;
	NRF_POWER->RAM[6].POWER = NRF52_ONRAM1_OFFRAM1;
	NRF_POWER->RAM[7].POWER = NRF52_ONRAM1_OFFRAM1;

	NRF_POWER->RAM[0].POWERSET = NRF52_ONRAM1_OFFRAM1;
	NRF_POWER->RAM[1].POWERSET = NRF52_ONRAM1_OFFRAM1;
	NRF_POWER->RAM[2].POWERSET = NRF52_ONRAM1_OFFRAM1;
	NRF_POWER->RAM[3].POWERSET = NRF52_ONRAM1_OFFRAM1;
	NRF_POWER->RAM[4].POWERSET = NRF52_ONRAM1_OFFRAM1;
	NRF_POWER->RAM[5].POWERSET = NRF52_ONRAM1_OFFRAM1;
	NRF_POWER->RAM[6].POWERSET = NRF52_ONRAM1_OFFRAM1;
	NRF_POWER->RAM[7].POWERSET = NRF52_ONRAM1_OFFRAM1;
}	

void power_off_peripheral_device(void)
{
	//unsigned char tx_data[4],rx_data[4];
	//uint32_t gc_gpio=0;
	
	/* OLED power off */
	OLED_Write_Command(0xAE);
	
	/* HR power off */
	//gc_protocol_ctrl_pah8002(DAILY_MODE, 0);
	gc_disable_pah8002();
	//pah8002_deinit();
}

void save_info_before_mcu_off(void)
{
	_GC_POWER_OFF_INFO_	aa;
	uint8_t __data[GC_FLASH_PAGE_DATA_SIZE];
	
	memset(&aa,0,sizeof(_GC_POWER_OFF_INFO_));
	memset(__data, 0xff, sizeof(__data));
	get_time( &time, 1);
	convert_gmt_to_display_time(&time, &slp_time);
	
	aa.sleep_start_year 		= slp_time.year;
	aa.sleep_start_month	= slp_time.month;
	aa.sleep_start_day			= slp_time.day;
	aa.sleep_start_hour			= slp_time.hour;
	aa.sleep_start_minute	= slp_time.minute;
	aa.sleep_start_second= slp_time.second;
	aa.step												= g_total_daystep;
	aa.step_cal									= kcal.step_cal;
	aa.activty_cal								= kcal.activty_cal;
	aa.sleep_cal								= kcal.sleep_cal;
	aa.coach_cal							= kcal.coach_cal;
	
	memcpy(__data, &aa, sizeof(_GC_POWER_OFF_INFO_));
	gc_flash_page_write_data(GC_FLASH_TBL_TYPE_POWER_OFF, __data);
}

void save_info_before_reset_point(uint32_t reset_point,uint32_t	reset_info_1,uint32_t	reset_info_2, uint32_t	reset_info_3,uint32_t	reset_info_4)	/*16.12.14 for test_log hm_lee*/
{
	_GC_RESET_lLOG_	rs;
	uint8_t __data[GC_FLASH_PAGE_DATA_SIZE];
	
	memset(&rs,0,sizeof(_GC_RESET_lLOG_));
	memset(__data, 0xff, sizeof(__data));
	get_time( &time, 1);
	convert_gmt_to_display_time(&time, &slp_time);
	
	rs.sleep_start_year 		= slp_time.year;
	rs.sleep_start_month	= slp_time.month;
	rs.sleep_start_day			= slp_time.day;
	rs.sleep_start_hour			= slp_time.hour;
	rs.sleep_start_minute	= slp_time.minute;
	rs.sleep_start_second= slp_time.second;
	rs.step												= g_total_daystep;
	rs.step_cal									= kcal.step_cal;
	rs.activty_cal								= kcal.activty_cal;
	rs.sleep_cal								= kcal.sleep_cal;
	rs.coach_cal							= kcal.coach_cal;
	rs.reset_point 						= reset_point;
	rs.reset_info_1 = reset_info_1;
	rs.reset_info_2 = reset_info_2;
	rs.reset_info_3 = reset_info_3;
	rs.reset_info_4 = reset_info_4;
	memcpy(__data, &rs, sizeof(_GC_RESET_lLOG_));
	gc_flash_page_write_data(GC_FLASH_TBL_TYPE_RESET_LOG, __data);
}

void save_data_before_reset(void)	/*16.12.20 for save_befor_reset hm_lee*/
{
//#ifdef SAVE_BEFORE_RESET

	_GC_DATA_BRFOR_RESET	rs_data;
	//uint8_t __data[GC_FLASH_PAGE_DATA_SIZE];
	
	//memset(&rs_data,0,sizeof(_GC_DATA_BRFOR_RESET));
	//memset(__data, 0xff, sizeof(__data));
	uint8_t __data[GC_FLASH_PAGE_DATA_SIZE];
	//memset(&rs_data,0,sizeof(_GC_DATA_BRFOR_RESET));
	//memset(__data, 0xff, sizeof(__data));
	
	
	get_time( &time, 1);
	convert_gmt_to_display_time(&time, &slp_time);
	
	rs_data.resert_start_year 		= time.year;
	rs_data.resert_start_month	= time.month;
	rs_data.resert_start_day			= time.day;
	rs_data.resert_start_hour			= time.hour;
	rs_data.resert_start_minute	= time.minute;
	rs_data.resert_start_second= time.second;
	rs_data.step												= g_total_daystep;
	rs_data.step_cal									= kcal.step_cal;
	rs_data.activty_cal								= kcal.activty_cal;
	rs_data.sleep_cal								= kcal.sleep_cal;
	rs_data.coach_cal								= kcal.coach_cal;
	rs_data.op_mode									= op_mode;
	rs_data.slp_time									=slp_info.slp_time;
	rs_data.rolled_count						=slp_info.rolled_count;	
	rs_data.awake_count						=slp_info.awake_count;
	rs_data.weak_count							=act_info.weak_count;
	rs_data.middle_count					=act_info.middle_count;
	rs_data.strong_count						=act_info.strong_count;
	rs_data.danger_count					=act_info.danger_count;
	rs_data.act_time              		=act_info.act_time;
	printf("\r\n %d,%d",rs_data.act_time,act_info.act_time);
	rs_data.HR_max									=HR_info.HR_max;
	rs_data.HR_min										=HR_info.HR_min;
	rs_data.HR_avg										=HR_info.HR_avg;
	memcpy(__data, &rs_data, sizeof(_GC_DATA_BRFOR_RESET));
	//gc_flash_page_write_data(GC_FLASH_TBL_TYPE_RESET_DATA, __data);
	
	gc_flash_erase(GC_FLASH_PAGE_RESET_DATA);
	nrf_delay_ms(200);
	gc_flash_fwrite(GC_FLASH_PAGE_RESET_DATA, __data);
	nrf_delay_ms(100);
	
	
	uint8_t		r_page_buf[GC_FLASH_PAGE_SIZE];
	memset(r_page_buf, 0, GC_FLASH_PAGE_SIZE);
	gc_flash_read(GC_FLASH_PAGE_RESET_DATA, r_page_buf );
		
		_GC_DATA_BRFOR_RESET	rs_data1;
	memcpy(&rs_data1, r_page_buf, sizeof(_GC_DATA_BRFOR_RESET));
	
	
	printf("\r\n %d,%d,%d",rs_data1.resert_start_day, rs_data1.resert_start_hour, rs_data1.resert_start_minute);
			
			
	
//#endif /*SAVE_BEFORE_RESET*/
}

void gc_power_off_prepare(void)
{
	//configure_ram_retention();

	/* stop BLE */
	reset_prepare();
	
	/* save info to flash */
	save_info_before_mcu_off();
	
	/* configure wakeup GPIO */
	nrf_gpio_cfg_sense_input(GC_GPIO_PIN_ACC_INTR_1, NRF_GPIO_PIN_NOPULL, NRF_GPIO_PIN_SENSE_HIGH);
	//nrf_gpio_cfg_sense_input(GC_GPIO_BAT_CHARGE_DETECT, NRF_GPIO_PIN_NOPULL, NRF_GPIO_PIN_SENSE_HIGH); 
	
	power_off_peripheral_device();
}

int32_t gc_sleep_mood_control(uint8_t is_move)
{      
	static uint8_t gc_sleep=false;
	uint32_t sleep_timeout = (GC_HZ_SENSOR_TIMER * GC_SLEEP_TIMEOUT);

	 if( is_move == 0 )
	 {          
			g_sleep_count++;
		 
			if(g_sleep_count > sleep_timeout )    /* 1 min */
			{
					if (pahr_mode == TOUCH_MODE)
					{
							printf("\r\n Enter power off mode");
							pah8002_deinit();
						
							gc_power_off_prepare();
							sd_power_system_off();

					}
			}
	 }
	 else
	 {
			g_sleep_count=0;
			
	 }

	 return gc_sleep;
}


