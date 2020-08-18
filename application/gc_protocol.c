#include <stdio.h>
#include <string.h>
#include "protocol.h"
#include "gc_activity.h"
#include "define.h"
#include "oled.h"
#include "nrf_gpio.h"
#include "ble_nus.h"
#include "app_error.h"
#include "command.h"
#include "rtc.h"
#include "spi_flash.h"
#include "nrf_delay.h"
#include "display.h"
#include "flash_control.h"
#include "gc_gpio.h"
#include "pah8002.h"
#include "gc_sleep.h"	/*16.12.14 for test_log hm_lee*/
//#define printf(...)

#define test_code 0

extern volatile int is_recall;
extern volatile unsigned char OLED_switch;
volatile struct PERSONAL_INFO person_info;
extern volatile struct ACTIVITY_INFO act_info;
extern volatile struct calorie_save kcal;
extern volatile struct SLEEP_INFO slp_info;
extern volatile struct s_hart_rate HR_info;
extern volatile struct STEP_INFO step;
//extern volatile unsigned int sleep_time;
extern volatile unsigned int activty_time;
extern volatile unsigned int act_time;
extern volatile unsigned short rolled_count;
extern volatile unsigned short awake_count;
extern volatile unsigned char noti_num;
extern volatile short g_noti_count;
volatile char save_low_bat = 0;
extern volatile int is_recall_act;

extern volatile uint8_t g_battery;
extern volatile char g_product_mode;
extern volatile unsigned char day_of_week;
extern volatile int32_t g_stress_level; 
extern char BLE_SSID[16];

extern volatile short gmt_offset;
extern volatile unsigned char tuch_count;
extern volatile unsigned short OLED_power_count;
extern volatile unsigned short display_step;

int request_time;

//규창 피쳐 동기화 시간들 
int feature_request_time;
int pastfeature_request_firsttime;
int pastfeature_request_time;
int pastfeature_request_lasttime;
int feature_request_today_time;
bool past_feature_flag = false;

volatile unsigned char is_info_recall=0;
int is_trc=0;
volatile unsigned char op_mode = DAILY_MODE;

//static ble_nus_t                        m_nus2;            
char stress;
char s[20];
int g_total_tx_busy_count=0;
int glob_elapsed_time=0; /* 1.3.9 hkim 2015.12.5 for debuging */
extern tTime time;
extern tTime stdTime;
int today;
extern volatile unsigned char is_charging;
extern volatile uint8_t hr_processing;
extern volatile uint8_t gc_conn_os; /* hkim 2016.11.21 */
extern volatile char BT_vib;

//규창 171016 피쳐데이터 전송부 추가
//0xa1 , 0xa2 , 0xa3 총 3개의 리턴을 가진다
extern volatile struct feature_tbl_form_s featuredata;



//규창 심박값 초기화 플래그
//bool sendinitVarFlag = false;

BLE_STATUS	m_ble_status;
REQ_BASE_CMD	req_acc_cmd;
REQ_BASE_CMD	req_rtc_cmd;
REQ_SN_CMD		req_sn_cmd;
NOTI_SWITCH noti_switch;

char response[20]={0};

int ble_receive_handler(unsigned char* pdata, unsigned short len)
{
			
	//SEGGER_RTT_printf(0, "\r\n ** Protocol 0x%x", pdata[0]);
	printf("BLE RX = %x",pdata[0]);
	switch(pdata[0])
		{
		case 0x3f:
		//printf("\n\r\n 1st chk");		
		if(pdata[1]==0x3f)
		{
			//printf("\n\r\n 2nd chk");
		}
			break;
				
			case 0x00:
			case 0x01:
				
			case 0x02:
			{
				//시간동기화
				
				printf("\n\r\n ble_receive_handler 0x02");
				//printf("\n\r\n %d.%d.%d %d:%d:%d, gmt offset : %d", pdata[2] |  pdata[3]<<8,pdata[4],pdata[5],pdata[6],pdata[7],pdata[8],pdata[10] | pdata[11] << 8);
				
				//printf("\n\r\n [%d][%d]yyyy/[%d][%d]mm/[%d][%d]dd/[%d][%d]hh/[%d][%d]mm/[%d][%d]ss/[%d][%d]/[%d][%d]"
				//, pdata[2],pdata[3],pdata[4],pdata[5],pdata[6],pdata[7],pdata[8],pdata[9],pdata[10],pdata[11],pdata[12],pdata[13],pdata[14],pdata[15],pdata[16],pdata[17]);
				//printf("\n\r\n [%s]", &pdata[2]);
				is_trc = ble_rcv_rtc(pdata,18);
				
				if(!is_info_recall && is_trc)
				{
					is_info_recall=f_info_recall();
				}
				save_low_bat = 1;
				return is_trc;
				//break;

			}
			case 0x03:
			{
				//사용자 정보
				uint8_t req_data[GC_FLASH_PAGE_DATA_SIZE]; /* 4: address */
				_GC_TBL_PERSINFO		_tbl_psinfo;
				memset(&_tbl_psinfo, 0, sizeof(_GC_TBL_PERSINFO));
				memset(req_data, 0xff, sizeof(req_data));
				
				person_info.sex = pdata[2];
				person_info.age = pdata[4];
				person_info.height = pdata[6];
				person_info.weight = pdata[8];
				
				_tbl_psinfo.f_gender = pdata[2];
				_tbl_psinfo.f_age = pdata[4];
				_tbl_psinfo.f_height = pdata[6];
				_tbl_psinfo.f_weight = pdata[8];

				
				memcpy(req_data, &_tbl_psinfo, sizeof(_GC_TBL_PERSINFO));
				gc_flash_page_write_data(GC_FLASH_TBL_TYPE_PERSINFO, req_data);

				//printf("\r\n profile response");
				//printf("\r\n sex:%d, age:%d, height%d, weight:%d",person_info.sex,person_info.age,person_info.height,person_info.weight);
				
				break;
			}
			
			case 0x04:
			{
				//배터리 잔량
										
			//short status; //1=사용중 2=충전중 3=충전완료
			
			s[0] = 0x04;
			s[1] = 0x00;

			if(!is_charging)
			{
				s[2] = 1;
				s[3] = 0x00;
				s[4] = g_battery;
				s[5] = 0x00;
			}
			else
			{
				if(g_battery==100)
				{
					s[2] = 3;
					s[3] = 0x00;
					s[4] = 100;
					s[5] = 0x00;
				}
				else
				{
					s[2] = 2;
					s[3] = 0x00;
					s[4] = g_battery;
					s[5] = 0x00;
				}
			}

			
//moom//			printf("\r\n battery response");
	//moom//		printf("\r\n s:%s",&s[0]);

			ble_send( (uint8_t*)s  , 6);
			
			break;
			}
			case 0x05:
			{
				unsigned char SN_data[GC_FLASH_PAGE_SIZE];
				int i,j;
				int lenth = 10;
				//시리얼 넘버
			#if test_code
				
				//printf("\r\n sn response");
				//printf("\r\n sn : %s",&pdata[3]);
			#endif /*test code*/
			
				for(j=0;j<lenth;j++)
				{
					s[j] = pdata[j];
				}
				ble_send( (uint8_t*)s  , lenth);
			
				gc_flash_page_write_data(GC_FLASH_TBL_TYPE_SN,&pdata[2]);
				
				
				
				for (i=0;i<5;i++)
				{
					if(gc_flash_page_read_data(GC_FLASH_TBL_TYPE_SN,1,SN_data))
					{
//						SN_data[15]=0;
//						ble_send( (uint8_t*)SN_data  , 6);
						save_data_before_reset(); /*16.12.20 for save_before_reset hm_lee*/
						nrf_delay_ms(10); /* FIXME */
						save_info_before_reset_point(4,0,0,0,0);	/*16.12.14 for test_log hm_lee*/
						NVIC_SystemReset();    
						i=6;
					}					
				}
							
			break;
		}
			case 0x06:
			{
				//걸음수 및 칼로리
	

			
			#if test_code
			//printf("\r\n step response");
			#endif /*test code*/

				unsigned char step_buf[GC_FLASH_PAGE_SIZE];
				unsigned char cal_buf[GC_FLASH_PAGE_SIZE];
								
				request_time = pdata[2] | pdata[3] << 8 | pdata[4] << 16 | pdata[5] << 24 ;
				printf("\r\n requestTime %d", request_time);
				get_time(&time, 1);
				convert_gmt_to_display_time(&time, &stdTime);
				today = (get_GMT(time.year,  time.month,  time.day,  time.hour,  time.minute, time.second )/(60));
				
				if(request_time==today || request_time==today-1)
				{
					//printf("\r\n request IF1111111111111111111111111111111111111");
					s[0]=0x06;
					s[1]= 0x00;
					
					s[2]= step.display_step&0x00ff;
					s[3]= (step.display_step&0xff00)>>8;				
					
					s[4] = kcal.activty_cal&0x000000ff;
					s[5] = (kcal.activty_cal&0x0000ff00)>>8;
					s[6] = (kcal.activty_cal&0x00ff0000)>>16;
					s[7] = (kcal.activty_cal&0xff000000)>>24;
					
					s[8] = kcal.sleep_cal&0x000000ff;
					s[9] = (kcal.sleep_cal&0x0000ff00)>>8;
					s[10] = (kcal.sleep_cal&0x00ff0000)>>16;
					s[11] = (kcal.sleep_cal&0xff000000)>>24;
					
					s[12] = kcal.step_cal&0x000000ff;
					s[13] = (kcal.step_cal&0x0000ff00)>>8;
					s[14] = (kcal.step_cal&0x00ff0000)>>16;
					s[15] = (kcal.step_cal&0xff000000)>>24;
					
					s[16] = kcal.coach_cal&0x000000ff;
					s[17] = (kcal.coach_cal&0x0000ff00)>>8;
					s[18] = (kcal.coach_cal&0x00ff0000)>>16;
					s[19] = (kcal.coach_cal&0xff000000)>>24;
					
//moom//					printf("\r\n cal.step_cal %d",kcal.step_cal);
//moom//					printf("\r\n cal.activty_cal %d",kcal.activty_cal);
//moom//					printf("\r\n cal.sleep_cal %d",kcal.sleep_cal);
//moom//					printf("\r\n cal.coach_cal %d",kcal.coach_cal);
//moom//					printf("\r\n step.display_step %d",step.display_step);
				}
				//규창 171101 여기는 부르면 앱크러싱 나는 루틴 인데 왜 있는지 모르겠다.....
				else
				{
					//printf("\r\n request else 2222222222222222222222");
					if(gc_flash_page_read_data(GC_FLASH_TBL_TYPE_STEP,request_time,step_buf))
					{
						//printf("\r\n request else inner if 333333333333333333333333");
					s[0] = 0x0a;
					s[1] = 0x00;
					
					s[2] = step_buf[4];
					s[3] = step_buf[5];
//moom//					printf("\r\n step not today ");
					}
					if(gc_flash_page_read_data(GC_FLASH_TBL_TYPE_KCAL,request_time,cal_buf))
					{
						//printf("\r\n request else inner if 4444444444444444444444");
					s[0] = 0x0a;
					s[1] = 0x00;
					
					s[4] = cal_buf[4];
					s[5] = cal_buf[5];
					s[6] = cal_buf[6];
					s[7] = cal_buf[7];
					
					s[8] = cal_buf[8];
					s[9] = cal_buf[9];
					s[10] = cal_buf[10];
					s[11] = cal_buf[11];
					
					s[12] =cal_buf[12];
					s[13] =cal_buf[13];
					s[14] =cal_buf[14];
					s[15] =cal_buf[15];
					
					s[16] =cal_buf[16];
					s[17] =cal_buf[17];
					s[18] =cal_buf[18];
					s[19] =cal_buf[19];
					}
//moom//					printf("\r\n cal not today ");
					
				}
				/*for(int i=0; i<20 ; i++) {
					printf("\r\n show me what yougot %c", (uint8_t*)s[i]); 
				 }*/
			ble_send( (uint8_t*)s  , 20 );
			
			
			break;
			}
			
			case 0x07:
			{
				//진동모터 제어
			//Action 1=동작 2=정지
			
				#if test_code
				//printf("\r\n motor response");
				//printf("\r\n %s",(pdata[2]==1) ? "on" : "off");
				#endif /*test code*/
			
			
				
			if(pdata[2]==1)
			{
				nrf_gpio_pin_set(GC_GPIO_MOTOR_ENABLE);
				BT_vib =1;
			}
			else
			{
				nrf_gpio_pin_clear(GC_GPIO_MOTOR_ENABLE);
				BT_vib =0;
				
			}
			
			break;
		}
			case 0x08:
			{
				//밴드 동작 모드
			//Action 1=운용모드 2=생산시험모드
			
				#if test_code

				
							
				//printf("\r\n pd response");
				//printf("\r\n %s",(pdata[2]==1) ? "운용모드" : "생산시험모드");
			
			
			#endif /*test code*/
			
			if(len>=2)
			{
				if(pdata[2]==0x02 ) 
				{
					g_product_mode=true;
				}
				else if(pdata[2]==0x01 ) 
				{
					g_product_mode=false;
					//NVIC_SystemReset(); 
				}
			return true;
			}
			break;
		}
			case 0x09:
			{
				//가속도 1회 요청
		
			#if test_code
			//printf("\r\n acc response");
			#endif /*test code*/
				
			s[0]= 0x09;
			s[1]= 0x00;
			s[2]= 11;
			s[3]= 0x00;
			s[4]= 22;
			s[5]= 0x00;
			s[6]= 33;
			s[7]= 0x00;
						

			ble_send( (uint8_t*)s  , 8 );

			
//			if(pdata[1]==0x01 && pdata[2]==0x04 && len==3)
//			{
//				req_acc_cmd.cmd = 1;
//		
//				return true;
//			}
			break;
		}
			case 0x0a:
			{
				//칼로리
				//date
				
				#if test_code
				//printf("\r\n cal response");
				#endif /*test code*/

				unsigned char act_buf[GC_FLASH_PAGE_SIZE];
				
				request_time = pdata[2] | pdata[3] << 8 | pdata[4] << 16 | pdata[5] << 24 ;
				
				get_time(&time, 1);
				convert_gmt_to_display_time(&time, &stdTime);
				today = (get_GMT(time.year,  time.month,  time.day,  time.hour,  time.minute, time.second )/(60));
				
				if(request_time==today ||request_time==today -1)
				{
					s[0] = 0x0a;
					s[1] = 0x00;
					
					s[2] = kcal.activty_cal&0x000000ff;
					s[3] = (kcal.activty_cal&0x0000ff00)>>8;
					s[4] = (kcal.activty_cal&0x00ff0000)>>16;
					s[5] = (kcal.activty_cal&0xff000000)>>24;
					
					s[6] = kcal.sleep_cal&0x000000ff;
					s[7] = (kcal.sleep_cal&0x0000ff00)>>8;
					s[8] = (kcal.sleep_cal&0x00ff0000)>>16;
					s[9] = (kcal.sleep_cal&0xff000000)>>24;
					
					s[10] = kcal.step_cal&0x000000ff;
					s[11] = (kcal.step_cal&0x0000ff00)>>8;
					s[12] = (kcal.step_cal&0x00ff0000)>>16;
					s[13] = (kcal.step_cal&0xff000000)>>24;
					
					s[14] = kcal.coach_cal&0x000000ff;
					s[15] = (kcal.coach_cal&0x0000ff00)>>8;
					s[16] = (kcal.coach_cal&0x00ff0000)>>16;
					s[17] = (kcal.coach_cal&0xff000000)>>24;
					
//moom//					printf("\r\n cal.step_cal %d",kcal.step_cal);
//moom//					printf("\r\n cal.activty_cal %d",kcal.activty_cal);
//moom//					printf("\r\n cal.sleep_cal %d",kcal.sleep_cal);
//moom//					printf("\r\n cal.coach_cal %d",kcal.coach_cal);
				}
				//규창 171101 어제자 칼로리 요청안하던데... 무슨 의미가 있는지 모르겠다...
				else
				{
					if(gc_flash_page_read_data(GC_FLASH_TBL_TYPE_KCAL,request_time,act_buf))
					{
						s[0] = 0x0a;
						s[1] = 0x00;
						
						s[2] = act_buf[4];
						s[3] = act_buf[5];
						s[4] = act_buf[6];
						s[5] = act_buf[7];
						
						s[6] = act_buf[8];
						s[7] = act_buf[9];
						s[8] = act_buf[10];
						s[9] = act_buf[11];
						
						s[10] =act_buf[12];
						s[11] =act_buf[13];
						s[12] =act_buf[14];
						s[13] =act_buf[15];
						
						s[14] =act_buf[16];
						s[15] =act_buf[17];
						s[16] =act_buf[18];
						s[17] =act_buf[19];
					}
//moom//					printf("\r\n cal not today ");
				}
				
				

				
				ble_send( (uint8_t*)s,18);
				break;
		}
			case 0x0b:
			{
				int save_sate=0, save_count;
				uint8_t req_data[GC_FLASH_PAGE_DATA_SIZE]; /* 4: address */
				_GC_TBL_MEASURE_ACTIVITY		_tbl_act;
				memset(&_tbl_act, 0, sizeof(_GC_TBL_MEASURE_ACTIVITY));
				memset(req_data, 0xff, sizeof(req_data));
				//활동
			//action 1=시작 2=종료 3=정보요청

			#if test_code
				//printf("\r\n act response");
			#endif /*test code*/
				
			if(pdata[2]==1)
			{
				op_mode = (op_mode == DAILY_MODE ) ? ACT_MODE : op_mode; 
				#ifdef DEBUG_PRINT
				printf("\r\n DAILY --> ACT");
				printf("\r\n is_recall_act %d",is_recall_act);
				#endif /*DEBUG_PRINT*/
				gc_protocol_ctrl_pah8002(op_mode, 1);
				OLED_power_count=OLED_ON_count_hr;
				tuch_count=(OLED_switch) ? display_page_mode : display_page_mode+1;	
				
				get_time(&time, 1);
				convert_gmt_to_display_time(&time, &stdTime);
				//act_info.act_time = (get_GMT(time.year,  time.month,  time.day,  time.hour,  time.minute, time.second )/(60));
				
				if(is_recall_act == 1)
				{
					is_recall_act = 0;
				#ifdef DEBUG_PRINT
					//act_info.act_time = 10;
					printf("\r\n %d",act_info.act_time);
				#endif /*DEBUG_PRINT*/
				}
				else
				{	
					act_info.weak_count = 0;
					act_info.middle_count = 0;
					act_info.strong_count = 0;
					act_info.danger_count = 0;
					act_info.act_calorie = 0;
					HR_info.HR_min = 0xFF;
					HR_info.HR_max = 0;
					HR_info.HR_avg = 0;
					activty_time = 0;
					act_time = 0;
					act_info.act_time = 0;
					
				}
			}
			else if(pdata[2]==2)
			{
				op_mode = (op_mode == ACT_MODE ) ? DAILY_MODE : op_mode; 
				#ifdef DEBUG_PRINT
				printf("\r\n ACT --> DAILY");
				#endif /*DEBUG_PRINT*/
				gc_protocol_ctrl_pah8002(op_mode, 0);
				
				tuch_count = (tuch_count == display_page_mode) ? display_page_clock : tuch_count;
				
				_tbl_act.time											= act_info.act_time;
				_tbl_act.calorie								= act_info.act_calorie ;
				_tbl_act.intensity_danger	= act_info.danger_count ;
				_tbl_act.intensity_high			= act_info.strong_count ;
				_tbl_act.intensity_mid				= act_info.middle_count ;
				_tbl_act.intensity_low				= act_info.weak_count ;
				_tbl_act.hr_min									= HR_info.HR_min ;
				_tbl_act.hr_avg									= HR_info.HR_avg ;
				_tbl_act.hr_max								= HR_info.HR_max  ;

				
				memcpy(req_data, &_tbl_act, sizeof(_GC_TBL_MEASURE_ACTIVITY));
				for(save_count = 0; save_count <= 5 ; save_count++)
					{
						save_sate = gc_flash_page_write_data(GC_FLASH_TBL_TYPE_ACT, req_data);
						if(save_sate)
						{
							save_count = 6;
						}
						nrf_delay_ms(10); /* FIXME */
					}
				
			}
		
			if(pdata[2]==3)
			{
				if(request_time==today ||request_time==today -1)
				{
					s[0] = 0x0b; //cmd
					s[1] = 0x00;
					
					s[2] = act_info.act_calorie&0x000000ff;
					s[3] = (act_info.act_calorie&0x0000ff00)>>8;
					s[4] = (act_info.act_calorie&0x00ff0000)>>16;
					s[5] = (act_info.act_calorie&0xff000000)>>24;
					
					s[6] = act_info.weak_count&0x00ff;
					s[7] = (act_info.weak_count&0xff00)>>8;
					
					s[8] = act_info.middle_count&0x00ff;
					s[9] = (act_info.middle_count&0xff00)>>8;
					
					s[10] = act_info.strong_count&0x00ff;
					s[11] = (act_info.strong_count&0xff00)>>8;
					
					s[12] = act_info.danger_count&0x00ff;
					s[13] = (act_info.danger_count&0xff00)>>8;
					
					if (HR_info.HR_min  == 0xFF){
						s[14] = 0;
					}else{
						s[14] = HR_info.HR_min;
					}
					s[15] = 0x00;
					s[16] = HR_info.HR_max;
					s[17] = 0x00;
					s[18] = HR_info.HR_avg;
					s[19] = 0x00;
							
					ble_send( (uint8_t*)s,20);
			}
			else
			{
					s[0] = 0x0b; //cmd
					s[1] = 0x00;
					
					s[2] = act_info.act_calorie&0x000000ff;
					s[3] = (act_info.act_calorie&0x0000ff00)>>8;
					s[4] = (act_info.act_calorie&0x00ff0000)>>16;
					s[5] = (act_info.act_calorie&0xff000000)>>24;
					
					s[6] = act_info.weak_count&0x00ff;
					s[7] = (act_info.weak_count&0xff00)>>8;
					
					s[8] = act_info.middle_count&0x00ff;
					s[9] = (act_info.middle_count&0xff00)>>8;
					
					s[10] = act_info.strong_count&0x00ff;
					s[11] = (act_info.strong_count&0xff00)>>8;
					
					s[12] = act_info.danger_count&0x00ff;
					s[13] = (act_info.danger_count&0xff00)>>8;
					
					
					if (HR_info.HR_min  == 0xFF){
						s[14] = 0;
					}else{
						s[14] = HR_info.HR_min;
					}
					
					s[15] = 0x00;
					s[16] = HR_info.HR_max;
					s[17] = 0x00;
					s[18] = HR_info.HR_avg;
					s[19] = 0x00;
							
					ble_send( (uint8_t*)s,20);
			}
			//규창 심박값 초기화 플래그
//			sendinitVarFlag = true;
		}
			
			break;
		}
			case 0x0c:
			{
				int sleep_time=0, save_sate =0, save_count;
				int i=0;
				uint8_t req_data[GC_FLASH_PAGE_DATA_SIZE]; /* 4: address */
				_GC_TBL_STABLE_HR		_tbl_stb_hr;
				memset(&_tbl_stb_hr, 0, sizeof(_GC_TBL_STABLE_HR));
				memset(req_data, 0xff, sizeof(req_data));
				//수면
			//action1=시작 2=종료 3=정보요청
			
				#if test_code	
					//printf("\r\n sleep response");
				#endif /*test code*/


			
				if(pdata[2]==1)
				{
					#ifndef AUTO_SLEEP
					op_mode = (op_mode == DAILY_MODE ) ? SLEEP_MODE : op_mode; 
					#ifdef DEBUG_PRINT
					printf("\r\n DAILY --> SLEEP");
					#endif /*DEBUG_PRINT*/
					gc_protocol_ctrl_pah8002(op_mode, 1);
					
					OLED_power_count=OLED_ON_count_sleep;
					tuch_count=(OLED_switch) ? display_page_mode : display_page_mode+1;
					if(is_recall)
					{						
					}
					else
					{
						sleep_time = 0;
						
						rolled_count =0 ;
						awake_count =0 ;
						slp_info.slp_time = 0;
						slp_info.rolled_count =0;
						slp_info.awake_count =0;		
					}
					#endif /*AUTO_SLEEP*/
				}
				else if(pdata[2]==2)
				{
					#ifndef AUTO_SLEEP
					op_mode = (op_mode == SLEEP_MODE)? DAILY_MODE : op_mode;
					#ifdef DEBUG_PRINT
					printf("\r\n SLEEP --> DAILY");
					#endif /*DEBUG_PRINT*/
					gc_protocol_ctrl_pah8002(op_mode, 0);
					
					memcpy(req_data, &_tbl_stb_hr, sizeof(_GC_TBL_STABLE_HR));
					gc_flash_page_write_data(GC_FLASH_TBL_TYPE_STABLE_HR, req_data);
					tuch_count = (tuch_count == display_page_mode) ? display_page_clock : tuch_count;
					#endif /*AUTO_SLEEP*/
				}	
				else if(pdata[2]==3)
				{
					sleep_time=(slp_info.slp_time/30);
					s[0] = 0x0c; //cmd
					s[1]	= 0x00;
					s[2] = slp_info.rolled_count&0x00ff;
					s[3] = (slp_info.rolled_count&0xff00)>>8;
					s[4] = slp_info.awake_count&0x00ff; 
					s[5] = (slp_info.awake_count&0xff00)>>8;
					s[6] = (HR_info.HR_sta==0 || HR_info.HR_sta > 140) ? 60 : HR_info.HR_sta;
					s[7] = 0x00;
					s[8] = sleep_time&0x000000ff;
					s[9] = (sleep_time&0x0000ff00)>>8;
					s[10] = (sleep_time&0x00ff0000)>>16;
					s[11] = (sleep_time&0xff000000)>>24;
					
//					s[0] = 0x0c; //cmd
//					s[1]	= 0x00;
//					s[2] = 1;
//					s[3] = 1;
//					s[4] = 2;
//					s[5] = 2;
//					s[6] = 60;
//					s[7] = 0x00;
//					s[8] = 3;
//					s[9] = 3;
//					s[10] = 0;
//					s[11] = 0;
					
					ble_send( (uint8_t*)s,12);
					printf("\r\n SLEEP INFO TR\n");
					_tbl_stb_hr.stable_hr = HR_info.HR_sta;
					
					memcpy(req_data, &_tbl_stb_hr, sizeof(_GC_TBL_STABLE_HR));
					for(save_count = 0; save_count <= 5 ; save_count++)
					{
						save_sate = gc_flash_page_write_data(GC_FLASH_TBL_TYPE_STABLE_HR, req_data);
						if(save_sate)
						{
							save_count = 6;
						}
						nrf_delay_ms(10); /* FIXME */
					}
					for(i=0;i<12;i++)
					{
						printf("\r\n s[%d]=%d",i,s[i]);
					}
//					rolled_count =0 ;
//					awake_count =0 ;
//					slp_info.slp_time = 0;
//					slp_info.rolled_count =0;
//					slp_info.awake_count =0;	
					//규창 심박값 초기화 플래그					
//					sendinitVarFlag = true;
				}

			
			
		
			
			
			break;
	}
			case 0x0d:
			{	//펌웨어 조회
			
				s[0]=0x0d;
				s[1]=0x00;
				s[2]=strlen(PRODUCT_VERSION);
				s[3]=0x00;
				sprintf(&s[4],"%s",PRODUCT_VERSION);
			
				#if test_code
				//printf("\r\n ver response");
				//printf("\r\nstrlen = %d",s[2]);
				//printf("\r\nPRODUCT_VERSION =%s",&s[4]);
				#endif /*test code*/
				
				ble_send( (uint8_t*)s  , 4 + strlen(PRODUCT_VERSION) );
			
				break;
			}	
			case 0x0e:
			case 0x0f:
				 
			case 0x10:
			{
				//Raw Data 전송 시작 및 중지
				//short Action; //1=전송시작 2=전송중지
			
				#if test_code
				
				#endif /*test code*/

				extern char send_rawdata;
				send_rawdata=pdata[2];	
				#if 0 /* hkim 2016.9.26 */
				if(pdata[2]==1)
				#else
				if ( pdata[1] == 1 || pdata[2] == 1 )
				#endif 
				{
					//printf("\r\n [CMD] enable send raw data");
					op_mode = (op_mode == DAILY_MODE ) ? COACH_MODE : op_mode; 
					#ifdef DEBUG_PRINT
					printf("\r\n DAILY --> COACH");
					#endif /*DEBUG_PRINT*/
					gc_protocol_ctrl_pah8002(op_mode, 1);
					
					OLED_power_count=OLED_ON_count_hr;
					tuch_count=(OLED_switch) ? display_page_mode : display_page_mode+1;
					
					ble_send_enable(pdata,len); /* hkim 2016.9.26 */
//moom//					printf("\r\nraw TR start");
				}
				else
				{
					//printf("\r\n [CMD] disable send raw data");
					op_mode = (op_mode == COACH_MODE ) ? DAILY_MODE : op_mode;
					#ifdef DEBUG_PRINT
					printf("\r\n COACH --> DAILY");
					#endif /*DEBUG_PRINT*/
					gc_protocol_ctrl_pah8002(op_mode, 0);
					tuch_count = (tuch_count == display_page_mode) ? display_page_clock : tuch_count;
					
//moom//					printf("\r\nraw TR end");
					ble_send_enable(pdata,len); /* hkim 2016.9.26 */
					//규창 심박값 초기화 플래그
//					sendinitVarFlag = true;
				}
				break;
			}
			case 0x11:
			{
				//데이터 저장공간 삭제
				if(pdata[1]==0x01) 
			{
//				feature_flash_erase(0); //hkim 0813
//				nrf_delay_ms(200);
//				feature_flash_erase(MAX_DB_PAGE-1); //hkim 0813
//				nrf_delay_ms(200);
//				NVIC_SystemReset();
			}
			
			#if test_code
			
//moom//			printf("\r\n del response");
			
			#endif /*test code*/
			break;
		}
			case 0x12:
			{
				
			#ifdef __GREENCOM_24H_BT_SENSOR_TEST__ /* hkim 0729 */
			return ble_rcv_acc_xyz_info(pdata,len);
			#else
			//코치 칼로리
			//int kcal;
			#if test_code
			//printf("\r\n %d+%d=",kcal.coach_cal,(pdata[2]|pdata[3]<<8|pdata[4]<<16|pdata[5]<<24));
			#endif /*test code*/				
			
			//kcal.coach_cal += (pdata[2]|pdata[3]<<8|pdata[4]<<16|pdata[5]<<24);

			#if test_code
			//printf("\r\n coach_cal response");
			//printf("\r\n coach_cal : %d",(kcal.coach_cal ));
			#endif /*test code*/
			#endif /*__GREENCOM_24H_BT_SENSOR_TEST__*/		
			break;
			}		
			case  0x13:
			{
				//스트레스
			
			//action 1=시작 2=종료 3=정보요청
			#if test_code
			if(pdata[2]==1)
			{						
				//printf("\r\n stress start");
			}
			else if(pdata[2]==2)
			{
				//printf("\r\n stress stop");
			}
			else if(pdata[2]==3)
			{
				//printf("\r\n stress info");
			}
			//printf("\r\n stress response");
			#endif /*test code*/
			if(pdata[2]==1)
			{
				op_mode = STRESS_MODE;
				#ifdef DEBUG_PRINT
				printf("\r\n STRESS Mode start");
				#endif /*DEBUG_PRINT*/
				gc_protocol_ctrl_pah8002(op_mode, 1); 
				
				ble_send_enable(pdata,len); /*hmLee 2016.10.24 스트레스 측정 mw로 이전 */
			}
			else if(pdata[2]==2)
			{
				op_mode = (op_mode == STRESS_MODE)? DAILY_MODE : op_mode;
				#ifdef DEBUG_PRINT
				printf("\r\n STRESS --> DAILY");
				#endif /*DEBUG_PRINT*/
				gc_protocol_ctrl_pah8002(op_mode, 0);
				
				ble_send_enable(pdata,len); /*hmLee 2016.10.24 스트레스 측정 mw로 이전 */
			}
			else if(pdata[2]==3)
			{

				s[0] = 0x13;
				s[1]	= 0x00;
				s[2] = g_stress_level;
				s[3] = 0x00;
			
				//printf("\r\n stress info");
			
				ble_send( (uint8_t*)s,4);
				
				g_stress_level = 0;
				//규창 심박값 초기화 플래그
//				sendinitVarFlag = true;
			}
			//Stress 1=매우좋음 2=좋음 3=보통 4=나쁨
			break;
		}
			case 0x14:
			{
				//밴트 상태 조회
			

			
			//printf("\r\n op response");
					
			s[0] =	0x14;
			s[1]	=	0x00;
			s[2]	=	op_mode; //1=일상 2=활동 3=스트레스 4=수면 5=코치
			s[3]	=	0x00;
			s[4] = (HR_info.HR_sta==0 || HR_info.HR_sta > 140) ? 60 : HR_info.HR_sta;
			s[5] = 0x00;
			ble_send( (uint8_t*)s,6);
			printf("op_mode = %d",op_mode);
			break;
			}
//			case 0x15:
			case 0x16:
			{
				printf("\r\n Alarm on/off 0x%x 0x%x", pdata[2], pdata[4]);
				// 알림 ON/OFF
				if(pdata[2]==1)
				{
					//iOS
					if(pdata[4]==1)
					{
						gc_conn_os = GC_CONNECTED_OS_IOS;
						//Call
						noti_switch.call = pdata[6];
					}
					else	if(pdata[4]==2)
					{
						//SMS
						noti_switch.SMS = pdata[6];
					}

				}
				else if(pdata[2]==2)
				{
					//android
					printf("\r\n connected OS : Android");
					gc_conn_os = GC_CONNECTED_OS_ANDROID;
					if(pdata[4]==1)
					{
						//Call
						noti_switch.call = pdata[6];
					}
					else	if(pdata[4]==2)
					{
						//SMS
						noti_switch.SMS = pdata[6];
					}

				}
				
				break;
			}	
			case 0x17:
			{
				// 알림 
				if(pdata[2] == 1)
				{
					//iOS
				}
				else if(pdata[2] == 2)
				{
					//android
					if(pdata[4] == 1 && noti_switch.call ==1)
					{		
						noti_num = 20;
					}
					else if(pdata[4] == 2 && noti_switch.call ==1)
					{		
						noti_num = 7;
					}
					else if(pdata[4] == 3 && noti_switch.SMS == 1)
					{		
						noti_num = 9;
						g_noti_count =  (pdata[6] |  pdata[7]<<8 );
					}
//					else if(pdata[4] == 4 && noti_switch.call ==1)
//					{		
//						noti_num = 9;
//						g_noti_count =  (pdata[6] |  pdata[7]<<8 );
//					}
				}
				
				break;
			}
			
			//규창 피쳐데이터 전송 기능 추가
			case 0xA1:
			//featuredata
			{	
				  feature_request_time = pdata[2] | pdata[3] << 8 | pdata[4] << 16 | pdata[5] << 24 ;
					//uint8_t *r_page_buf;
					//_GC_FEATURE_DATA __featuredatalog;
					//memset(r_page_buf, 0, GC_FLASH_PAGE_SIZE);
					struct feature_tbl_form_s featureSend;
				  featureSend = find_req_feature(feature_request_time);
					printf("\r\n feature : time:%d", featureSend.time);
					printf("\r\n feature : n_var:%d pressure:%d prediff:%d x_mean:%d y_mean:%d z_mean:%d n_mean:%d x_var:%d y_var:%d z_var:%d", featureSend.norm_var, featureSend.pressure, featureSend.pre_diff_sum,featureSend.x_mean, featureSend.y_mean, featureSend.z_mean,featureSend.norm_mean,featureSend.x_var ,featureSend.y_var,	featureSend.z_var);
					printf("\r\n feature : step:%d rope_cnt:%d s_swing:%d l_swing:%d",featureSend.nStep,featureSend.jumping_rope_count ,featureSend.small_swing_count,featureSend.large_swing_count);
					printf("\r\n feature : hb:%d",featureSend.hb);
				  /*r_page_buf = find_req_feature(feature_request_time, &);
			    memcpy(&__featuredatalog, r_page_buf, GC_FLASH_PAGE_SIZE);
					
				
					struct feature_tbl_form_s featureSend;
				  featureSend.time = __featuredatalog.time;
					featureSend.pressure = __featuredatalog.pressure;
					featureSend.pre_diff_sum = __featuredatalog.pre_diff_sum;
					featureSend.nStep = __featuredatalog.nStep;
					featureSend.jumping_rope_count = __featuredatalog.jumping_rope_count;
					featureSend.small_swing_count = __featuredatalog.small_swing_count;
					featureSend.large_swing_count = __featuredatalog.large_swing_count;
					featureSend.hb = __featuredatalog.hb;
					featureSend.x_mean = __featuredatalog.x_mean;
					featureSend.y_mean = __featuredatalog.y_mean;
					featureSend.z_mean = __featuredatalog.z_mean;
					featureSend.norm_mean = __featuredatalog.norm_mean;
					featureSend.x_var = __featuredatalog.x_var;
					featureSend.y_var = __featuredatalog.y_var;
					featureSend.z_var = __featuredatalog.z_var;
					featureSend.norm_var = __featuredatalog.norm_var;*/
				 //printf("\r\n feature_request_time %d", feature_request_time);
				  //printf("\r\n featuredata response1");
					s[0] = 0xA1; //cmd
					s[1] = 0x00;
					s[2] = featureSend.time & 0x000000ff;
				  s[3] = (featureSend.time & 0x0000ff00)>>8; 
					s[4] = (featureSend.time & 0x00ff0000)>>16; 
					s[5] = (featureSend.time & 0xff000000)>>24;
					s[6] = featureSend.norm_var  & 0x000000ff;
				  s[7] = (featureSend.norm_var & 0x0000ff00)>>8; 
					s[8] = (featureSend.norm_var & 0x00ff0000)>>16; 
					s[9] = (featureSend.norm_var & 0xff000000)>>24;
					s[10] = featureSend.x_var & 0x00ff;
					s[11] = (featureSend.x_var & 0xff00) >>8;
				  s[12] = featureSend.y_var & 0x00ff;
					s[13] = (featureSend.y_var & 0xff00) >>8;
					s[14] = featureSend.z_var & 0x00ff;
					s[15] = (featureSend.z_var & 0xff00) >>8;
					s[16] = featureSend.x_mean & 0x00ff;
				  s[17] = (featureSend.x_mean & 0xff00)>>8; 
					s[18] = featureSend.y_mean & 0x00ff;
					s[19] = (featureSend.y_mean & 0xff00)>>8; 
				 
				  
					ble_send( (uint8_t*)s,20);
				
				  //printf("\r\n featuredata response2");
					s[0] = 0xA2; //cmd
					s[1] = 0x00;
					s[2] = featureSend.z_mean & 0x00ff;
				  s[3] = (featureSend.z_mean & 0xff00)>>8;
					s[4] = featureSend.norm_mean & 0x00ff;
					s[5] = (featureSend.norm_mean & 0xff00)>>8;
				  s[6] = featureSend.nStep & 0x00ff;
					s[7] = (featureSend.nStep & 0xff00) >>8;
				  s[8] = featureSend.jumping_rope_count & 0x00ff;
					s[9] = (featureSend.jumping_rope_count & 0xff00) >>8;
					s[10] = featureSend.small_swing_count & 0x00ff;
					s[11] = (featureSend.small_swing_count & 0xff00) >>8;
					s[12] = featureSend.large_swing_count & 0x00ff;
					s[13] = (featureSend.large_swing_count & 0xff00) >>8;
				  s[14]= featureSend.pre_diff_sum & 0x00ff;
					s[15] = (featureSend.pre_diff_sum & 0xff00)>>8;
					s[16] = featureSend.accumulated_step & 0x00ff;
					s[17] = (featureSend.accumulated_step & 0xff00) >>8;
					s[18] = featureSend.display_step & 0x00ff;
					s[19] = (featureSend.display_step & 0xff00) >>8;
			
					
					ble_send( (uint8_t*)s,20);
					
					
					s[0] = 0xA3; //cmd
					s[1] = 0x00;
					s[2] = featureSend.pressure  & 0x000000ff;
				  s[3] = (featureSend.pressure & 0x0000ff00)>>8; 
					s[4] = (featureSend.pressure & 0x00ff0000)>>16; 
					s[5] = (featureSend.pressure & 0xff000000)>>24;
					s[6] = featureSend.hb & 0x00ff;
					s[7] = 0x00;
					
					ble_send( (uint8_t*)s,8);
					
					break;
			}
			case 0xB1:
				pastfeature_request_firsttime = pdata[2] | pdata[3] << 8 | pdata[4] << 16 | pdata[5] << 24 ;
				pastfeature_request_lasttime = pdata[6] | pdata[7] << 8 | pdata[8] << 16 | pdata[9] << 24 ;
				feature_request_today_time = pdata[10] | pdata[11] << 8 | pdata[12] << 16 | pdata[13] << 24 ;
				//m_ble_status.send_enable = 1;
				printf("\r\n reqtime first %d", pastfeature_request_firsttime);
				findpaststartADDR();
				past_feature_flag = true;
				//extern char send_rawdata;
				//send_rawdata=1;	
				break;
//			case 0x18:
//			case 0x19:
//			case 0x1a:
//			case 0x1b:
//			case 0x1c:
//			case 0x1d:
//			case 0x1e:
//			case 0x1f:
	}

		return false;
}

int ble_rcv_rtc(unsigned char* pdata, unsigned short len)
{
#if (!__BOARD2__  &&  !__BOARD3__)
	unsigned int total_sec;
#endif
	int year;
	unsigned char month,day,hour,minute,second;
	uint8_t req_data[GC_FLASH_PAGE_DATA_SIZE]; /* 4: address */
	memset(req_data, 0xff, sizeof(req_data));
	memcpy(req_data, &pdata[16], sizeof(pdata)-2);
	
	year =							pdata[2] |  pdata[3]<<8;
	month =						pdata[4] |  pdata[5]<<8;
	day =								pdata[6] |  pdata[7]<<8;
	hour = 							pdata[8] |  pdata[9]<<8;
	minute = 					pdata[10] |  pdata[11]<<8;
	second = 				pdata[12] |  pdata[13]<<8;
	day_of_week =	pdata[14] |  pdata[15]<<8;
	gmt_offset =	pdata[16] |  pdata[17]<<8;
//moom//	printf("\r\n%d-%d-%d   %d:%d:%d (%d) [%d]",year,month,day,hour,minute,second,day_of_week,gmt_offset);
#if 0 /* hkim 1.3.9.10 2016.1.26 세계시간 동기화 */		
	if(year<2015 && year>2038)
		return false;
#else
		if(year<2000 && year>2038)
		return false;
#endif 

	if(month<1 && month>12)
		return false;

	if(day<1 && day>31)
		return false;

	if( hour>23)
		return false;

	if( minute>=60)
		return false;

	if( second>=60)
		return false;	
	
		if(day_of_week == 0 && day_of_week>7)
		return false;	
	
	rtc_set_time(year,month,day,hour,minute,second);
	//ks_printf("Protocol : gmt_offset : %x %d", gmt_offset, gmt_offset); 
	gc_flash_page_write_data(GC_FLASH_TBL_TYPE_GMT_OFFSET,req_data);

#if __GREENCOM_24H__
#if (__BOARD2__ || __BOARD3__)

	if(is_rtc_valid()==false)
		req_rtc_cmd.cmd=2;
	/* GMT 시간을 RTC에 설정 */
	gc_flash_page_write_data(GC_FLASH_TBL_TYPE_GMT_OFFSET,req_data);
	//ks_printf("\r\nrtc=%d %d %d %d %d %d",year, pdata[5], pdata[6], pdata[7], pdata[8], pdata[9]);

	read_gmt_offset_from_flash(GC_FLASH_PAGE_GMT_OFFSET, &__gmt_offset);
	printf("\r\n GMT offset current / protocol %d %d", __gmt_offset, gmt_offset);
	if (__gmt_offset != gmt_offset)
	{
		save_gmt_offset(gmt_offset);   /* v1.4.10.0 hkim 2016.02.16 */
	}
#else

	total_sec = get_GMT(year,month,day,hour,minute,second);

#if 0 /* hkim 1.3.9.10 2016.1.26 세계시간 동기화 */	
	g_time = total_sec*20;
#else
	g_time = (total_sec + (gmt_offset)) * 20;
#endif
#endif  /* #if (__BOARD2__ || __BOARD3__) */
#endif /* __GREENCOM_24H__ */
	
	if(req_rtc_cmd.cmd==false)
	req_rtc_cmd.cmd=1;
	
	//printf("\r\n gmt_offset %d",gmt_offset);
	return true;
}


extern int g_total_send_count;
int ble_send_enable(unsigned char* pdata, unsigned short len)
{

	if( pdata[1]==0x01 || pdata[2]==0x01 )
	{
		m_ble_status.send_enable = 1;


	}
	else if( pdata[1]==0x02 || pdata[2]==0x02 )
	{
		m_ble_status.send_enable = 0;

		//ks_printf("\r\n tx_busy_count=%d  total_send_count=%d",g_total_tx_busy_count, g_total_send_count );
		g_total_tx_busy_count=0;
		g_total_send_count=0;		
	}

	return true;
}

int ble_sn(unsigned char* pdata, unsigned short len)
{

	memset(&req_sn_cmd,0,sizeof(req_sn_cmd));

	memcpy(req_sn_cmd.sn, &pdata[3], 15);
	req_sn_cmd.sn[15]=0;
	req_sn_cmd.cmd = true;
	

	return true;
}


//int g_total_send_count=0;
//void ble_send( uint8_t* data, uint16_t length )
//{
//	printf("\r\nble_send response");
//	uint32_t err_code;
//#if __GREENCOM_24H__
//	int i;
//#endif

//#if 1//__GREENCOM_FITNESS__
//	g_total_send_count++;
//	printf("\r\ng_total_send_count response");
//#endif

//#if __GREENCOM_24H__
//	if (dbg_feature_send == 1) /* v1.3.9 hkim 2015.12.05 */
//	{		
//		print_co(PRINT_MAGENTA, "\r\nSEND= ");
//		for(i=0;i<length;i++)
//		{
//			ks_printf("%02x ",data[i] );
//		}
//	}
//#endif
//	err_code = ble_nus_string_send(&m_nus, data  , length);
//	if (err_code != NRF_ERROR_INVALID_STATE)
//	{
//		APP_ERROR_CHECK(err_code);
//	}
//}


//void read_gmt_offset_from_flash(unsigned short  page, void* data )
//{
//	unsigned char buf[260];
//	unsigned int address;

//	address = page<<8;
//	SPI_FLASH_BufferRead(buf, address );
//	
//	memcpy(data,buf+4,sizeof(short));
//}

int save_gmt_offset(short __gmt_offset)
{
	short __offset=0;
	unsigned char offset_buf[GC_FLASH_PAGE_SIZE] ;
	
	__offset = __gmt_offset;
	offset_buf[0] = __offset;
	gc_flash_page_write_data(GC_FLASH_TBL_TYPE_GMT_OFFSET,offset_buf);
	
	return true;
}


#ifdef __GREENCOM_24H_BT_SENSOR_TEST__ /* hkim 0729 */
unsigned int bt_acc_xyz_count=1;
int ble_rcv_acc_xyz_info(unsigned char* pdata, unsigned len)
{
	short bt_acc_x=0, bt_acc_y=0, bt_acc_z=0;
	
	memcpy(&bt_acc_x, &pdata[4], 2);
	memcpy(&bt_acc_y, &pdata[6], 2);
	memcpy(&bt_acc_z, &pdata[8], 2);
	
	//ks_printf("\r\n BT x/y/z %d, %d, %d(bt_count %d)", bt_acc_x, bt_acc_y, bt_acc_z, bt_acc_xyz_count);
	
	get_realtime_step(bt_acc_xyz_count, bt_acc_x, bt_acc_y, bt_acc_z);
	bt_acc_xyz_count++;
	return true;
}
#endif  /* __GREENCOM_24H_BT_SENSOR_TEST__ */
