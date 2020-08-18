
#include <stdint.h>
#include <string.h>
#include <stdio.h>
//#include "nordic_common.h"
//#include "nrf.h"
//#include "nrf51_bitfields.h"
//#include "ble_hci.h"
//#include "ble_advdata.h"
//#include "ble_conn_params.h"
//#include "softdevice_handler.h"
//#include "app_timer.h"
//#include "app_button.h"
//#include "ble_nus.h"
//#include "boards.h"


//#include "app_util_platform.h"


#include "define.h"
#include "print.h"
#include "font.h"
#include "protocol.h"
#include "heart_beat.h"
#include "oled.h"
#include "rtc.h"
#include "gc_activity.h"
#include "nrf_gpio.h"
#include "gc_gpio.h"

//#include "greencom_feature.h"
#include "display.h"


#include "dfu_ble_svc.h"
extern ble_gap_addr_t                    m_ble_addr; 
extern volatile short gmt_offset;
extern volatile unsigned char tuch_count ;
volatile unsigned char pre_tuch_count = 255;
volatile unsigned short OLED_power_count = OLED_ON_count_time;
volatile unsigned char OLED_switch =1 ;
extern volatile struct SLEEP_INFO slp_info;
extern volatile struct ACTIVITY_INFO act_info;

extern volatile unsigned char hr_warring;
extern volatile unsigned char hr_50;
volatile unsigned char noti_num = 0;
unsigned char vibe_num = 0;
extern volatile unsigned char op_mode;
unsigned char pre_op_mode;
unsigned char stress_switch;
extern volatile unsigned int g_total_daystep;
//extern volatile unsigned int sleep_time;
extern volatile unsigned int activty_time;
extern volatile unsigned char g_hr;
unsigned char d1,d2,h1,h2,m1,m2;
unsigned char pre_h1,pre_h2,pre_m1,pre_m2;
volatile unsigned char day_of_week =1;
extern volatile unsigned char oled_test_mode;
extern volatile struct s_hart_rate HR_info;
extern volatile struct calorie_save kcal;
extern volatile struct PERSONAL_INFO person_info;
extern volatile uint8_t g_battery;
extern volatile unsigned char is_charging;
unsigned short stress_count;
volatile unsigned short vibration_count;
volatile short g_noti_count;
char running_noti = 0;
extern NOTI_SWITCH noti_switch;
volatile char bat_notied =0;
volatile char BT_vib=0;
extern volatile char g_product_mode;

void OLED_display(int count)
{

		if(noti_num>0 && noti_switch.SMS ==2 && noti_switch.call ==2)
		{
			noti_num = 0;
		}
	//printf("\r\n tuch_count:pre_tuch_count = %d:%d",tuch_count,pre_tuch_count);
	//규창 171031 디버깅용 배터리 차지 OLED 들어냄
		if(is_charging && !g_product_mode) //충전 애니메이션
	{
		noti_num = 0;
		vibe_num=0;
		bat_notied =0;
		if(OLED_switch==0)
		{
			OLED_switch=1;
			OLED_Power(OLED_switch);
			OLED_power_count=OLED_ON_count_charging;
		}
		if(count%20==0)
		{
			OLED_charge(g_battery);			
		}
		else if(count%10==0)
		{
			OLED_charge2(g_battery);
		}
	}
	else if(op_mode==STRESS_MODE)
	{
		stress_count++;
		if(pre_op_mode!=STRESS_MODE)
		{
			OLED_switch=1;
			OLED_Power(OLED_switch);
			pre_op_mode=STRESS_MODE;
		}
		OLED_stress(stress_count);
	}
	else if(op_mode!=STRESS_MODE &&pre_op_mode==STRESS_MODE)
	{
		stress_count=0;
		OLED_switch=0;
		OLED_Power(OLED_switch);
		pre_op_mode=DAILY_MODE;
	}
	else if(noti_num>0)
	{
		printf("\r\noti_num = [%d]",noti_num);
		switch(noti_num)
		{
			case 7:
				noti_num=0;
				if(noti_switch.call ==1)
				{
					running_noti =1;
					vibe_num=7;
					vibration_count=0;
					OLED_switch=1;
					OLED_Power(OLED_switch);
					OLED_power_count = 0;
					OLED_call();
				}
				//printf("\r\ncalling");
				break;

			case 8:
			{
				noti_num=0;
				vibe_num = 0;
				running_noti =0;
				vibration_count=OLED_OFF_count+1;	
//				OLED_switch=0;
//				OLED_Power(OLED_switch);
				OLED_power_count = OLED_OFF_count-1;
				//motor OFF
				nrf_gpio_pin_clear(GC_GPIO_MOTOR_ENABLE);
			}
				break;

			case 9:
				noti_num=0;
				if(noti_switch.SMS ==1)
				{
					running_noti =1;
					vibe_num=9;
					vibration_count=0;
					OLED_switch=1;
					OLED_Power(OLED_switch);
					OLED_power_count = OLED_ON_count_noti;
					OLED_sms(g_noti_count);
				}
				break;

//			case 10:
//				noti_num=0;
//				vibe_num=10;
//				vibration_count=0;
//				OLED_switch=1;
//				OLED_Power(OLED_switch);
//				OLED_power_count = OLED_ON_count_noti;
//				OLED_sns(g_noti_count);
//				break;
			
			case 20:
			{
				noti_num=0;
				vibe_num = 0;
				running_noti =0;
				vibration_count=OLED_OFF_count+1;	
//				OLED_switch=0;
//				OLED_Power(OLED_switch);
				OLED_power_count = OLED_OFF_count-1;
				//motor OFF
				nrf_gpio_pin_clear(GC_GPIO_MOTOR_ENABLE);
			}
				break;
		}
	}

	/*Touch control*/
	else if(tuch_count!=pre_tuch_count && running_noti == 0)
	{
		printf("\r\tuch_count = [%d]",tuch_count);
		if(OLED_switch==0)
		{
			tuch_count--;

			//printf("\r\n OLED_on");
			OLED_switch=1;
			OLED_Power(OLED_switch);
		}
		
			

			tuch_count = (tuch_count>=display_page_act_info+1)? 0 : tuch_count;
			if(tuch_count>=display_page_mode&&op_mode==DAILY_MODE)
			{
				tuch_count=display_page_clock;
			}
			else if(tuch_count>=display_page_act_info&&op_mode!=ACT_MODE)
			{
				tuch_count=display_page_clock;
			}
//			if(g_product_mode)
//			{
//				tuch_count = (tuch_count > display_page_step) ? 0 : tuch_count;
//			}
			switch(tuch_count)
			{
				case display_page_clock:
					OLED_power_count = OLED_ON_count_time;
					OLED_clock();
					break;

				case display_page_step:
					OLED_power_count = OLED_ON_count_step;
					if(!g_product_mode)
					{
						OLED_step();
					}
					else
					{
						OLED_info();
					}
					break;
				
//				case display_page_hr_pct:
//					OLED_hr_pct();
//					break;
				case display_page_calorie:
					OLED_power_count = OLED_ON_count_cal;
					OLED_calorie();
					break;
				
				case display_page_mode:
				{	
					switch(op_mode)
					{
						case DAILY_MODE:
						OLED_power_count = OLED_ON_count_hr;
						OLED_hr();
						break;

						case ACT_MODE:
						OLED_power_count = OLED_ON_count_hr;
						OLED_act();
						break;
						
//						case STRESS_MODE:
//						OLED_stress();
//						break;			

						
						case SLEEP_MODE:
							OLED_power_count = OLED_ON_count_sleep;
							OLED_sleep();
							break;
			
						case COACH_MODE :
							OLED_power_count = OLED_ON_count_hr;
							OLED_coach();
							break;
					}
					break;
				}
				case display_page_act_info:
				OLED_power_count = OLED_ON_count_act_info;
				OLED_act_info();
				break;
				/*홈쇼핑으로 인한 기압표시화면 제거
				case display_page_pressure_info:
				OLED_power_count = OLED_ON_count_pressure_info;
				OLED_pressure_info();
				break;*/
				
			}
			pre_tuch_count=tuch_count;
	}
	
	if(hr_warring)
	{
		if(vibration_count >= 100)
		{
		
		}
		else if(vibration_count == 0)
		{
			OLED_switch=1;
			OLED_Power(OLED_switch);
			tuch_count = display_page_mode;
			switch(op_mode)
					{
						case DAILY_MODE:
						OLED_power_count = OLED_ON_count_hr;
						OLED_hr();
						break;

						case ACT_MODE:
						OLED_power_count = OLED_ON_count_hr;
						OLED_act();
						break;
						
//						case STRESS_MODE:
//						OLED_stress();
//						break;			

						
						case SLEEP_MODE:
							OLED_power_count = OLED_ON_count_sleep;
							OLED_sleep();
							break;
			
						case COACH_MODE :
							OLED_power_count = OLED_ON_count_hr;
							OLED_coach();
							break;
					}

				
			//motor ON
			nrf_gpio_pin_set(GC_GPIO_MOTOR_ENABLE);
			printf("\r\n hr_warring ON [%d]",vibration_count/10);
		}
		else if(vibration_count%10 == 0)
		{
			//motor ON
			nrf_gpio_pin_set(GC_GPIO_MOTOR_ENABLE);
			printf("\r\n hr_warring ON [%d]",vibration_count/10);
		}
		else if(vibration_count% 10 == 5)
		{
			//motor OFF
			nrf_gpio_pin_clear(GC_GPIO_MOTOR_ENABLE);
			printf("\r\n hr_warring OFF [%d]",vibration_count/10);		
		}
	}
	else if(hr_50==1 && running_noti == 0)
	{
		if(vibration_count >= 30 )
		{
			hr_50 = 0;	
		}
		else if(vibration_count % 10 ==0 )
		{ 
			//motor ON
			nrf_gpio_pin_set(GC_GPIO_MOTOR_ENABLE);
			printf("\r\n bat_low on [%d]",vibration_count/10);
		}
		else if(vibration_count%10 == 5)
		{
			//motor OFF
			nrf_gpio_pin_clear(GC_GPIO_MOTOR_ENABLE);	
			printf("\r\n bat_low on [%d]",vibration_count/10);
		}
	}
	else if(g_battery<=5 && !is_charging && !bat_notied) 
	{
		if(vibration_count >= 30 )
		{
			bat_notied = 1;	
		}
		else if(vibration_count % 30 ==0 )
		{ 
			//motor ON
			nrf_gpio_pin_set(GC_GPIO_MOTOR_ENABLE);
			printf("\r\n bat_low on [%d]",vibration_count/10);
		}
		else if(vibration_count%30 == 20)
		{
			//motor OFF
			nrf_gpio_pin_clear(GC_GPIO_MOTOR_ENABLE);	
			printf("\r\n bat_low on [%d]",vibration_count/10);
		}
	}
	else if(vibe_num ==7)
	{
		if(vibration_count%60 == 0)
		{
			//motor ON
			nrf_gpio_pin_set(GC_GPIO_MOTOR_ENABLE);
		}
		else if(vibration_count% 60 == 20)																																																																																																																																																																																																																																																																																																														
		{
			//motor OFF
			nrf_gpio_pin_clear(GC_GPIO_MOTOR_ENABLE);	
		}
	}
	else if(vibe_num >=8) 
	{
		if(vibration_count >= 30)
		{
			vibe_num  = 0;
			nrf_gpio_pin_clear(GC_GPIO_MOTOR_ENABLE);
		}
		else if(vibration_count % 10 ==0 )
		{
			//motor ON
			nrf_gpio_pin_set(GC_GPIO_MOTOR_ENABLE);
		}
		else if(vibration_count%10 == 5)
		{
			//motor OFF
			nrf_gpio_pin_clear(GC_GPIO_MOTOR_ENABLE);	
		}
	}
	else if(BT_vib)
	{
		nrf_gpio_pin_set(GC_GPIO_MOTOR_ENABLE);
	}
	else
	{
		nrf_gpio_pin_clear(GC_GPIO_MOTOR_ENABLE);
		vibe_num = 0;
		vibration_count = OLED_OFF_count;
	}	
	
	/*OLED OFF*/
	if((OLED_power_count >= OLED_OFF_count)&&(OLED_switch==1)&&(op_mode != STRESS_MODE))
	{
		OLED_switch=0;
		pre_op_mode=op_mode;
		if(!oled_test_mode)
		{
			OLED_Power(OLED_switch);
			printf("\r\n OLED_off");
		}
		running_noti = 0;
		//printf("\r\n OLED_off");
	}
	

	
	if(OLED_power_count<=OLED_OFF_count+5)
	{
		OLED_power_count++;
		if(is_charging)
		{
			OLED_power_count  =  OLED_ON_count_charging;
		}
	}
	if(vibration_count<=OLED_OFF_count+1)
	{
		vibration_count++;	
	}
	
	if(g_battery<=5 && vibration_count > OLED_OFF_count)
	{
		vibration_count = 0;
	}
	if((!is_charging || g_product_mode) && running_noti==0)
	{
		if(OLED_switch == 1 && tuch_count == display_page_mode && op_mode != STRESS_MODE)
		{
			if(op_mode == SLEEP_MODE)
			{
				h1 = ((slp_info.slp_time/30)/60)/10;
				h2 = ((slp_info.slp_time/30)/60)%10;
				m1 = ((slp_info.slp_time/30)%60)/10;
				m2 = ((slp_info.slp_time/30)%60)%10;
			
				if(h1 != pre_h1)
				{
					dp_mnum(3,	10,h1);
				}
				if(h2 != pre_h2)
				{
					dp_mnum(3,19,h2);
				}
				if(m1 != pre_m1)
				{
					dp_mnum(3,37,m1);
				}
				if(m2 != pre_m2)
				{				
					dp_mnum(3,46,m2);
				}
				
				pre_h1 = h1;
				pre_h2 = h2;
				pre_m1 = m1;
				pre_m2 = m2;
			}
			else if(op_mode == ACT_MODE || op_mode == COACH_MODE)
			{
				int hr1,hr2,hr3;
				int pct,pct1,pct2,pct3;
				//규창 안정심박 임시 70
				unsigned char HR_stable = 70;// (HR_info.HR_sta  && HR_info.HR_sta < 255 ) ? HR_info.HR_sta : 60;
				hr1 = g_hr/100;
				hr2 =(g_hr%100)/10;
				hr3 = g_hr%10;
				pct=(g_hr - HR_stable>0) ? (unsigned char)((g_hr - HR_stable) / (220.0f - person_info.age - HR_stable)*100) : 0;//pct=(unsigned char)((g_hr) / (220.0f - person_info.age)*100); /* 16.10.25 hm_lee	코치 운동 화면과 동일하게 심박% 계산*/
				pct1 = pct/100;
				pct2 = (pct%100)/10;
				pct3 = pct%10;
				dp_mnum(2,19,hr1);
				dp_mnum(2,28,hr2);
				dp_mnum(2,37,hr3);
				dp_mnum(4,19,pct1);
				dp_mnum(4,28,pct2);
				dp_mnum(4,37,pct3);
			}
		}
		if(OLED_switch == 1 && tuch_count == display_page_clock  && op_mode != STRESS_MODE)
		{
			dp_BT(0,38,m_ble_status.connected);
		}
	}
}


void OLED_charge(int battery)
{
	OLED_blank(0,0,64);
	OLED_blank(1,0,16);
	OLED_blank(1,48,64);
	OLED_blank(2,0,16);
	OLED_blank(2,48,64);
	OLED_blank(3,0,16);
	OLED_blank(3,48,64);
	OLED_blank(4,0,64);
	OLED_blank(5,0,64);
	
	dp_charge(1,16,battery);
}

void OLED_charge2(int battery)
{
	OLED_blank(0,0,64);
	OLED_blank(1,0,16);
	OLED_blank(1,48,64);
	OLED_blank(2,0,16);
	OLED_blank(2,48,64);
	OLED_blank(3,0,16);
	OLED_blank(3,48,64);
	OLED_blank(4,0,64);
	OLED_blank(5,0,64);
	
	dp_charge2(1,16,battery);
}

void OLED_clock(void)
{
	tTime time;
	tTime stdTime;
	get_time(&time, 1);
	convert_gmt_to_display_time(&time, &stdTime);
	//규창 171031 gmt_offset 임시 제거
	if(gmt_offset ==0)
	{
		recall_GMT_offset();
	}
	printf("\r\n gmt_offset %d",gmt_offset);
	printf("\n\r %d-%d-%d %d:%d",time.year,time.month,time.day,time.hour,time.minute);
	printf("\n\r %d-%d-%d %d:%d",stdTime.year,stdTime.month,stdTime.day,stdTime.hour,stdTime.minute);
	
	//int day_of_week2;

		 
	
	if(stdTime.year<2010)
	{
		d1 = 0;
		d2  = 0;
		h1 = 0;
		h2 = 0;
		m1 = 0;
		m2  = 0;
		day_of_week = -1;
		//규창 부호비트 없는 char로 선언해놓고 왜 -1을 사용?..
	}
	else
	{
		d1 = stdTime.day/10;
		d2  = stdTime.day%10;
		h1 = stdTime.hour/10;; //(stdTime.hour/10==0) ? 20 : stdTime.hour/10;
		h2 = stdTime.hour%10;
		m1 = stdTime.minute/10;
		m2  = stdTime.minute%10;
		day_of_week = (((get_GMT(stdTime.year,  stdTime.month,  stdTime.day,  stdTime.hour,  stdTime.minute, stdTime.second ))/(86400)-1)%7);
	}
	
	OLED_blank(0,0,64);
	OLED_blank(1,0,64);
	OLED_blank(2,0,64);
	OLED_blank(3,0,64);
	OLED_blank(4,0,64);
	OLED_blank(5,0,64);
	
	printf("day_of_week = %d",day_of_week);
	dp_week(1,4,day_of_week);
	dp_snum(1,25,d1);
	dp_snum(1,31,d2);
	dp_BT(0,38,m_ble_status.connected);
	dp_battery(0,51,g_battery);
	 
	dp_lnum(3,4,h1);
	dp_lnum(3,16,h2);
	dp_lnum(3,28,10);
	dp_lnum(3,40,m1);
	dp_lnum(3,52,m2);

//	dp_lnum(3,4,1);
//	dp_lnum(3,16,2);
//	dp_lnum(3,28,3);
//	dp_lnum(3,40,4);
//	dp_lnum(3,52,5);
}

void OLED_step(void)
{
	unsigned char n0,n1,n2,n3,n4;
	n4 = g_total_daystep / 10000;
	n3 = (g_total_daystep % 10000)/1000;
	n2 = (g_total_daystep % 1000)/100;
	n1 = (g_total_daystep % 100)/10;
	n0 = g_total_daystep % 10;
	
	OLED_blank(0,0,64);
	OLED_blank(1,0,64);
	OLED_blank(2,0,64);
	OLED_blank(3,0,64);
	OLED_blank(4,0,64);
	OLED_blank(5,0,64);
	
	dp_icon_mini(0,22,0);
	
	dp_mnum(3,11,n4);
	dp_mnum(3,20,n3);
	dp_mnum(3,29,n2);
	dp_mnum(3,38,n1);
	dp_mnum(3,47,n0);

//	dp_mnum(3,10,1);
//	dp_mnum(3,19,2);
//	dp_mnum(3,28,3);
//	dp_mnum(3,37,4);
//	dp_mnum(3,46,5);
}

void OLED_hr(void)
{	
	int hr1,hr2,hr3;
	int pct,pct1,pct2,pct3;
	//규창 안정심박 임시 70
	unsigned char HR_stable = 70;//(HR_info.HR_sta) ? HR_info.HR_sta : 60;
	
	if(0&&HR_info.HR_avg)
	{
		hr1 =  HR_info.HR_avg/100;
		hr2 = (HR_info.HR_avg%100)/10;
		hr3 = HR_info.HR_avg%10;
		//pct=(unsigned char)((HR_info.HR_avg - HR_stable) / (220.0f - person_info.age - HR_stable)*100);
		pct=(unsigned char)((HR_info.HR_avg ) / (220.0f - person_info.age)*100); /* 16.10.25 hm_lee	코치 운동 화면과 동일하게 심박% 계산*/
	}
	else
	{
		hr1 = g_hr/100;
		hr2 =(g_hr%100)/10;
		hr3 = g_hr%10;
		pct=(g_hr - HR_stable>0) ? (unsigned char)((g_hr - HR_stable) / (220.0f - person_info.age - HR_stable)*100) : 0;
		//pct=(unsigned char)((g_hr) / (220.0f - person_info.age)*100); /* 16.10.25 hm_lee	코치 운동 화면과 동일하게 심박% 계산*/
		}//심박 90: 23 실 연산 == 0.00208  ??
		
	pct1 = pct/100;
	pct2 = (pct%100)/10;
	pct3 = pct%10;


	OLED_blank(0,0,64);
	OLED_blank(1,0,64);
	OLED_blank(2,0,64);
	OLED_blank(3,0,64);
	OLED_blank(4,0,64);
	OLED_blank(5,0,64);
	
	dp_icon_small(0,24,0);

	dp_icon_small(2,2,1);
	
	dp_mnum(2,19,hr1);
	dp_mnum(2,28,hr2);
	dp_mnum(2,37,hr3);
	
	dp_icon_small(4,2,1);
	
	dp_mnum(4,19,pct1);
	dp_mnum(4,28,pct2);
	dp_mnum(4,37,pct3);
	dp_mnum(4,46,13);
}

void OLED_act(void)
{
		int hr1,hr2,hr3;
	int pct,pct1,pct2,pct3;
	//규창 안정심박 임시 70
	unsigned char HR_stable = 70;//(HR_info.HR_sta) ? HR_info.HR_sta : 60;
	
	if(0&&HR_info.HR_avg)
	{
		hr1 =  HR_info.HR_avg/100;
		hr2 = (HR_info.HR_avg%100)/10;
		hr3 = HR_info.HR_avg%10;
		pct=(unsigned char)((HR_info.HR_avg - HR_stable) / (220.0f - person_info.age - HR_stable)*100);
		//pct=(unsigned char)((HR_info.HR_avg ) / (220.0f - person_info.age)*100); /* 16.10.25 hm_lee	코치 운동 화면과 동일하게 심박% 계산*/
		//printf("\r\n0&&HR_info.HR_avg" );
	}
	else
	{
		hr1 = g_hr/100;
		hr2 =(g_hr%100)/10;
		hr3 = g_hr%10;
		pct=(g_hr - HR_stable>0) ? (unsigned char)((g_hr - HR_stable) / (220.0f - person_info.age - HR_stable)*100) : 0;
		//pct=(unsigned char)((g_hr) / (220.0f - person_info.age)*100); /* 16.10.25 hm_lee	코치 운동 화면과 동일하게 심박% 계산*/
		//printf("\r\nFAIL??????? 0&&HR_info.HR_avg" );
	}
		
	pct1 = pct/100;
	pct2 = (pct%100)/10;
	pct3 = pct%10;


	OLED_blank(0,0,64);
	OLED_blank(1,0,64);
	OLED_blank(2,0,64);
	OLED_blank(3,0,64);
	OLED_blank(4,0,64);
	OLED_blank(5,0,64);
	
	dp_icon_small(0,24,2);

	dp_icon_small(2,2,1);
	
	dp_mnum(2,19,hr1);
	dp_mnum(2,28,hr2);
	dp_mnum(2,37,hr3);
	
	dp_icon_small(4,2,1);
	
	dp_mnum(4,19,pct1);
	dp_mnum(4,28,pct2);
	dp_mnum(4,37,pct3);
	dp_mnum(4,46,13);
}

/* 홈쇼핑으로 인한 기압표시화면 제거
extern float tempResult, pressureResult, altitudeResult;
void OLED_pressure_info(void)
{
	int altitude1,altitude2,altitude3,altitude4;
	int pressure1,pressure2,pressure3,pressure4,pressure5;
	int temp1,temp2,temp3;
	
	int tempResultINT= tempResult*10;
	int pressureResultINT = pressureResult*10;
	int altitudeResultINT = altitudeResult;
	
	
	altitude1 = altitudeResultINT/1000;
	altitude2 = (altitudeResultINT%1000)/100;
	altitude3 = (altitudeResultINT%100)/10;
	altitude4 =  altitudeResultINT%10;
	
	pressure1 = pressureResultINT/10000;
	pressure2 = (pressureResultINT%10000)/1000;
	pressure3 = (pressureResultINT%1000)/100;
	pressure4 = (pressureResultINT%100)/10;
	pressure5 =  pressureResultINT%10;
	//dp_mnum(4,46,13);
	
	
		
	temp1 = tempResultINT/100;
	temp2 = (tempResultINT%100)/10;
	temp3 = tempResultINT%10;


	OLED_blank(0,0,64);
	OLED_blank(1,0,64);
	OLED_blank(2,0,64);
	OLED_blank(3,0,64);
	OLED_blank(4,0,64);
	OLED_blank(5,0,64);
	
	//dp_icon_small(0,24,2);
	dp_mnum(0,10,altitude1);
	dp_mnum(0,19,altitude2);
	dp_mnum(0,28,altitude3);
	dp_mnum(0,37,altitude4);


	dp_mnum(2,10,pressure1);
	dp_mnum(2,19,pressure2);
	dp_mnum(2,28,pressure3);
	dp_mnum(2,37,pressure4);
	dp_mnum(2,49,pressure5);
	
	
	//dp_icon_small(4,2,1);
	
	dp_mnum(4,19,temp1);
	dp_mnum(4,28,temp2);
	dp_mnum(4,39,temp3);
	//dp_mnum(4,46,13);
}
*/



void OLED_act_info(void)
{
	int h1,h2,m1,m2;
	int cal,cal1,cal2,cal3,cal4;
	//규창 안정심박 임시 70
	//unsigned char HR_stable = 70;//(HR_info.HR_sta) ? HR_info.HR_sta : 60;
	
	
	h1 =(act_info.act_time/60)/10;
	h2 =(act_info.act_time/60)%10;
	m1 =(act_info.act_time%60)/10;
	m2 =(act_info.act_time%60)%10;
	
	cal = act_info.act_calorie / 1000;
	cal1 = cal/ 1000;
	cal2 = (cal% 1000)/100;
	cal3 = (cal % 100)/10;
	cal4 = cal % 10;


	OLED_blank(0,0,64);
	OLED_blank(1,0,64);
	OLED_blank(2,0,64);
	OLED_blank(3,0,64);
	OLED_blank(4,0,64);
	OLED_blank(5,0,64);
	
	dp_icon_small(0,24,2);
	
	dp_mnum(2,8,h1);
	dp_mnum(2,18,h2);
	dp_mnum(2,28,10);
	dp_mnum(2,38,m1);
	dp_mnum(2,48,m2);
			
	dp_mnum(4,8,cal1);
	dp_mnum(4,18,cal2);
	dp_mnum(4,28,cal3);
	dp_mnum(4,38,cal4);
	dp_unit(5,47,0);
}

void OLED_sleep(void)
{
	h1 = ((slp_info.slp_time/30)/60)/10;
	h2 = ((slp_info.slp_time/30)/60)%10;
	m1 = ((slp_info.slp_time/30)%60)/10;
	m2 = ((slp_info.slp_time/30)%60)%10;
	
	OLED_blank(0,0,64);
	OLED_blank(1,0,64);
	OLED_blank(2,0,64);
	OLED_blank(3,0,64);
	OLED_blank(4,0,64);
	OLED_blank(5,0,64);

	dp_icon_mini(0,22,3);
	
	dp_mnum(3,	10,h1);
	dp_mnum(3,19,h2);
	dp_mnum(3,28,10);
	dp_mnum(3,37,m1);
	dp_mnum(3,46,m2);
	
//	dp_mnum(3,	10,1);
//	dp_mnum(3,19,2);
//	dp_mnum(3,28,10);
//	dp_mnum(3,37,3);
//	dp_mnum(3,46,4);
}
  
void OLED_coach(void)
{
	int hr1,hr2,hr3;
	int pct,pct1,pct2,pct3;
	//규창 안정심박 임시 70
	unsigned char HR_stable = 70;// (HR_info.HR_sta  && HR_info.HR_sta < 255 ) ? HR_info.HR_sta : 60;
	
	if(0&&HR_info.HR_avg)
	{
		hr1 =  HR_info.HR_avg/100;
		hr2 = (HR_info.HR_avg%100)/10;
		hr3 = HR_info.HR_avg%10;
		pct=(unsigned char)((HR_info.HR_avg - HR_stable) / (220.0f - person_info.age - HR_stable)*100);
		//pct=(unsigned char)((HR_info.HR_avg ) / (220.0f - person_info.age)*100); /* 16.10.25 hm_lee	코치 운동 화면과 동일하게 심박 계산*/
	}
	else
	{
		hr1 = g_hr/100;
		hr2 =(g_hr%100)/10;
		hr3 = g_hr%10;
		pct=(g_hr - HR_stable>0) ? (unsigned char)((g_hr - HR_stable) / (220.0f - person_info.age - HR_stable)*100) : 0;
		//pct=(unsigned char)((g_hr) / (220.0f - person_info.age)*100); /* 16.10.25 hm_lee	코치 운동 화면과 동일하게 심박% 계산*/
	}
		
	pct1 = pct/100;
	pct2 = (pct%100)/10;
	pct3 = pct%10;


	OLED_blank(0,0,64);
	OLED_blank(1,0,64);
	OLED_blank(2,0,64);
	OLED_blank(3,0,64);
	OLED_blank(4,0,64);
	OLED_blank(5,0,64);
	
	dp_icon_small(0,24,4);

	dp_icon_small(2,2,1);
	
	dp_mnum(2,19,hr1);
	dp_mnum(2,28,hr2);
	dp_mnum(2,37,hr3);
	
	dp_icon_small(4,2,1);
	
	dp_mnum(4,19,pct1);
	dp_mnum(4,28,pct2);
	dp_mnum(4,37,pct3);
	dp_mnum(4,46,13);
}
  
  
void OLED_stress(unsigned int stress_count)
{
	if(stress_count==1)
	{	
		OLED_blank(0,0,64);
		OLED_blank(1,0,64);
		OLED_blank(2,0,64);
		OLED_blank(3,0,64);
		OLED_blank(4,0,64);
		OLED_blank(5,0,64);
		
		dp_icon_mini(0,22,5);
	}
	dp_mnum(3,	10,14);
	dp_mnum(3,19,(stress_count%100>=20)?14:20);
	dp_mnum(3,28,(stress_count%100>=40)?14:20);
	dp_mnum(3,37,(stress_count%100>=60)?14:20);
	dp_mnum(3,46,(stress_count%100>=80)?14:20);
}
  
  
void OLED_call(void)
{
	OLED_blank(0,0,64);
	OLED_blank(1,0,64);
	OLED_blank(2,0,64);
	OLED_blank(3,0,64);
	OLED_blank(4,0,64);
	OLED_blank(5,0,64);
	
	dp_icon_large(1,12,6);
	
//	dp_mnum(3,19,0);
//	dp_mnum(3,28,0);
//	dp_mnum(3,37,0);	
}
  
  
void OLED_unanswer(int noti_count)
{
	int noti_count1, noti_count2, noti_count3;
	
	noti_count1 = (noti_count % 1000)/100;
	noti_count2 = (noti_count % 100)/10;
	noti_count3 = noti_count % 10;
	
	OLED_blank(0,0,64);
	OLED_blank(1,0,64);
	OLED_blank(2,0,64);
	OLED_blank(3,0,64);
	OLED_blank(4,0,64);
	OLED_blank(5,0,64);
	
	dp_icon_large(0,12,7);
	
	dp_mnum(4,19,noti_count1);
	dp_mnum(4,28,noti_count2);
	dp_mnum(4,37,noti_count3);
}
  
  
void OLED_sms(int noti_count)
{
	int noti_count1, noti_count2, noti_count3;
	
	noti_count1 = (noti_count % 1000)/100;
	noti_count2 = (noti_count % 100)/10;
	noti_count3 = noti_count % 10;
	
	OLED_blank(0,0,64);
	OLED_blank(1,0,64);
	OLED_blank(2,0,64);
	OLED_blank(3,0,64);
	OLED_blank(4,0,64);
	OLED_blank(5,0,64);
	
	dp_icon_large(0,12,8);
	
	dp_mnum(4,19,noti_count1);
	dp_mnum(4,28,noti_count2);
	dp_mnum(4,37,noti_count3);
}
  
  
//void OLED_sns(int noti_count)
//{
//	int noti_count1, noti_count2, noti_count3;
//	
//	noti_count1 = (noti_count % 1000)/100;
//	noti_count2 = (noti_count % 100)/10;
//	noti_count3 = noti_count % 10;
//	
//	OLED_blank(0,0,64);
//	OLED_blank(1,0,64);
//	OLED_blank(2,0,64);
//	OLED_blank(3,0,64);
//	OLED_blank(4,0,64);
//	OLED_blank(5,0,64);
//	
//	dp_icon_mini(0,22,9);
//	
//	dp_mnum(3,19,noti_count1);
//	dp_mnum(3,28,noti_count2);
//	dp_mnum(3,37,noti_count3);
//}

void OLED_calorie(void)
{
	int total_cal,n_cal_0,n_cal_1,n_cal_2,n_cal_3;
	
	total_cal=(kcal.activty_cal+kcal.coach_cal+kcal.sleep_cal+kcal.step_cal)/1000;
	
	
	n_cal_3 = total_cal /1000;
	n_cal_2 = (total_cal % 1000)/100;
	n_cal_1 = (total_cal % 100)/10;
	n_cal_0 = total_cal % 10;
	
	if(total_cal>10000)
	{
		n_cal_3 = n_cal_2 = n_cal_1 = n_cal_0 = 9;
	}
	
	OLED_blank(0,0,64);
	OLED_blank(1,0,64);
	OLED_blank(2,0,64);
	OLED_blank(3,0,64);
	OLED_blank(4,0,64);
	OLED_blank(5,0,64);
	
	dp_icon_mini(0,22,10);
	
	dp_mnum(3,	5,n_cal_3);
	dp_mnum(3,14,n_cal_2);
	dp_mnum(3,23,n_cal_1);
	dp_mnum(3,32,n_cal_0);
	dp_unit(4,41,0);
}

void OLED_info(void)
{
	int i;
	char s[20];
	sprintf(s,"%s",PRODUCT_VERSION);
	
	OLED_blank(0,0,64);
	OLED_blank(1,0,64);
	OLED_blank(2,0,64);
	OLED_blank(3,0,64);
	OLED_blank(4,0,64);
	OLED_blank(5,0,64);
	
	dp_schar(0,00,'a');
	dp_schar(0,06,'d');
	dp_schar(0,12,'d');
	dp_schar(0,18,'r');
	dp_schar(0,24,'e');
	dp_schar(0,30,'s');
	dp_schar(0,36,'s');
	
	dp_schar(1,00,(m_ble_addr.addr[5]/0X10 > 9)?(m_ble_addr.addr[5]/0X10)+87 : m_ble_addr.addr[5]/0X10);
	dp_schar(1,06,(m_ble_addr.addr[5]%0X10 > 9)?(m_ble_addr.addr[5]%0X10)+87 : m_ble_addr.addr[5]%0X10);
	dp_schar(1,12,10);
	dp_schar(1,18,(m_ble_addr.addr[4]/0X10 > 9)?(m_ble_addr.addr[4]/0X10)+87 : m_ble_addr.addr[4]/0X10);
	dp_schar(1,24,(m_ble_addr.addr[4]%0X10 > 9)?(m_ble_addr.addr[4]%0X10)+87 : m_ble_addr.addr[4]%0X10);
	dp_schar(1,30,10);
	dp_schar(1,36,(m_ble_addr.addr[3]/0X10 > 9)?(m_ble_addr.addr[3]/0X10)+87 : m_ble_addr.addr[3]/0X10);
	dp_schar(1,42,(m_ble_addr.addr[3]%0X10 > 9)?(m_ble_addr.addr[3]%0X10)+87 : m_ble_addr.addr[3]%0X10);
	dp_schar(1,48,10);
	
	dp_schar(2,00,(m_ble_addr.addr[2]/0X10 > 9)?(m_ble_addr.addr[2]/0X10)+87 : m_ble_addr.addr[2]/0X10);
	dp_schar(2,06,(m_ble_addr.addr[2]%0X10 > 9)?(m_ble_addr.addr[2]%0X10)+87 : m_ble_addr.addr[2]%0X10);
	dp_schar(2,12,10);
	dp_schar(2,18,(m_ble_addr.addr[1]/0X10 > 9)?(m_ble_addr.addr[1]/0X10)+87 : m_ble_addr.addr[1]/0X10);
	dp_schar(2,24,(m_ble_addr.addr[1]%0X10 > 9)?(m_ble_addr.addr[1]%0X10)+87 : m_ble_addr.addr[1]%0X10);
	dp_schar(2,30,10);
	dp_schar(2,36,(m_ble_addr.addr[0]/0X10 > 9)?(m_ble_addr.addr[0]/0X10)+87 : m_ble_addr.addr[0]/0X10);
	dp_schar(2,42,(m_ble_addr.addr[0]%0X10 > 9)?(m_ble_addr.addr[0]%0X10)+87 : m_ble_addr.addr[0]%0X10);
	
	dp_schar(4,00,'f');
	dp_schar(4,06,'w');
	dp_schar(4,12,'v');
	dp_schar(4,18,'e');
	dp_schar(4,24,'r');
	dp_schar(4,30,'s');
	dp_schar(4,36,'i');
	dp_schar(4,42,'o');
	dp_schar(4,48,'n');
	
	for(i=0;i<=10;i++)
	{
		dp_schar(5,i*6,(s[i]==46)? 11 : s[i]-48);
	}
	
//	for(i=0;i<20;i++)
//	{
//		printf("\r\n s[%d] = %d",i,s[i]-48);
//	}

}
