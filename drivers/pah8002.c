#include <stdint.h>
#include <string.h>
#include <stdlib.h>

#include "pah8002.h"
#include "pah8002_comm.h"
#include "pah8002_api_c.h"
#include "boards.h" 
#include "pah8002_reg_ir2g1.h"
//#include "pah8002_dd_vendor_1_Auto.h"
//#include "pah8002_dd_vendor_1_noAuto.h"
#include "app_uart.h"
#include "print.h"
#include "nrf_delay.h"
#include "gc_util.h"
#include "bma2x2.h"
#include "gc_saadc.h"
#ifndef GC_COACH_FIT_PCB_PAHR_IF_TWI 
	#include "nrf_drv_spi.h"
#endif
#include "pxialg_dyn.h"
#include "gc_activity.h"
#include "define.h"
#include "SEGGER_RTT.h"

#define SEGGER_RTT_printf(...)
//#define printf(...)

#ifdef GC_NRF52_SDNN
#define SDNN_COLLECTION_SECONDS 60 
#endif /* GC_NRF52_SDNN */


extern struct PERSONAL_INFO person_info; /* hkim 2016.8.16 */
volatile int32_t g_stress_level;

extern uint8_t op_mode;
volatile uint8_t hr_processing; /* 1 : enable HR, 0 : disable HR */


//static int pah8002_check(void);
volatile uint8_t motion_hr, sdnn_hr; //hkim-1008
//규창 센서 보정을 위한 배열
uint8_t calibration_hr[10] = {0,};
int calibration_count = 0;

uint8_t calibration_hr_buffer[10] = {0,};
int calibration_hr_buffer_count = 0;

bool reset_minHR = false;

//규창 안씀 이제
//volatile uint8_t _pah8002_no_fifo_int_flag; //hkim-1008
volatile uint8_t _pah8002_sleep_process_flag; //hkim-1008
volatile int trustLevel;
//규창 콘솔 hw테스트용 플래그
volatile bool testPPG = false;

//#ifdef GC_NRF52_PAHR
//extern uint8_t pah8002_rx_buff[4];
//#endif 




/// 규창 여기서부터 원본 선언들 
/////////////////////////////////////////////////////////////////////////////////////////
//********************************************************************//
//  	In our Original Setting, We will report 20 Frame while you detect Interrupt . 
//	Customer need Use i2c Burst Read to get 240 Bytes FIFO Data.(HEART_RATE_MODE_SAMPLES_PER_READ *4Byte)
//	If Customer MCU I2C BURST_READ LENGTH last than 240 Bytes.
//	You can adjust the parameters HEART_RATE_MODE_SAMPLES_PER_CH_READ to  meet the platform.
//	EX: MCU I2C BURST_READ Max LENGTH = 128
//	       Customer can set  HEART_RATE_MODE_SAMPLES_PER_CH_READ (10)
//	       Customer need Use i2c Burst Read to get 120 Bytes FIFO Data.
//	       In this setting, Interrupt Period will about 500ms
//	      HEART_RATE_MODE_SAMPLES_PER_CH_READ Min. Value is 10.
//********************************************************************//



//#define MEMS_ZERO	//Default Accelerometer data are all zero

//#define PPG_MODE_ONLY

uint8_t pahr_mode = NONE ;
uint8_t pah8002_ppg_data[HEART_RATE_MODE_SAMPLES_PER_READ * 4] ;
static uint8_t _touch_flag = 0 ;
volatile uint8_t _pah8002_interrupt = 0 ;
static pah8002_data_t _pah8002_data;
static uint32_t _timestamp = 0 ;
#ifdef MEMS_ZERO
static int16_t _mems_data[HEART_RATE_MODE_SAMPLES_PER_CH_READ * 3] ;
#endif

static uint8_t _ir_dac = 0 ;
static uint8_t _ir_expo = 0;
static uint8_t _chip_id = 0 ;

static void *_pah8002_alg_buffer = NULL;

static bool pah8002_sw_reset(void);
static bool pah8002_start(void);
static int pah8002_wakeup(void);

/// 규창 여기서부터 원본 선언들 끝/// 

///////////////////규창 아래는 첫부팅 초기화와 ble명령용이므로 냅둠 //////////////////////////////////////////////////////////////////////


//규창 심박센서 초기화용 
#ifdef _NRF52_HW_1_2_ADC_SPI_WORKAROUND_
extern const nrf_drv_spi_t spi_pah8002;
#endif
void gc_disable_pah8002()
{
		pah8002_deinit();
		nrf_delay_ms(1);
#ifdef _NRF52_HW_1_2_ADC_SPI_WORKAROUND_
		nrf_drv_spi_uninit(&spi_pah8002);
#endif
	
}

void gc_enable_pah8002()
{
#ifdef _NRF52_HW_1_2_ADC_SPI_WORKAROUND_
	gc_spi_pah8002_init();
#endif 	
	nrf_delay_ms(10);

	pah8002_init();
}
//규창 심박센서 초기화용 끝


//규창 심박센서 초기화용 
//ble로 심박센서 켤때 용도
extern volatile uint8_t gc_adc_battery_flag;
/* control : 0 (disable), 1 (enable) */

extern unsigned int epoch_hr;
extern unsigned int HR_sum;
extern struct s_hart_rate HR_info;

int32_t gc_protocol_ctrl_pah8002(uint8_t op_mode, uint8_t hr_control)
{
	if (hr_control == 1)
	{
		if (op_mode == DAILY_MODE || hr_processing==1 )	
			return false;
		
		//gc_disable_saadc();
		//printf("\r\n Enable PAH8002!");
		
		//규창 171027 플래너를 위해 심박 켜고 끄는 기능 off
		//pah8002_init();
		//gc_enable_pah8002();
		hr_processing = 1;
		gc_adc_battery_flag = GC_ADC_BATTERY_FLAG_NONE;
		epoch_hr = 0;
		HR_sum = 0;
		HR_info.HR_avg = 0;
		HR_info.HR_max = 0;
		HR_info.HR_min = 0xFF;
	}
	else /* disable HR */
	{
		if (op_mode != DAILY_MODE || hr_processing==0 )	
			return false;
		
		//printf("\r\n Disable PAH8002!");
		//gc_disable_pah8002();
		//규창 171027 플래너를 위해 심박 켜고 끄는 기능 off
	 //	pah8002_deinit();
		hr_processing = 0;
	}
	
	return true;
}

//규창 심박센서 초기화용 
///////////////////규창 아래는 첫부팅 초기화와 ble명령용이므로 냅둠 끝 //////////////////////////////////////////////////////////////////////



//++++++++++++++++++++++PAH8002 functions++++++++++++++++++++++++++++++++++++++++++++++++++++
static bool pah8002_sw_reset()
{
		uint8_t data = 0;
		//printf(">>> pah8002_sw_reset \r\n");
		pah8002_wakeup();
		//delay_ms(5);
		if(0 != pah8002_write_reg(0x7f, 0x00))
		{
			goto RTN;
		}
		if(0 != pah8002_read_reg(0, &data))
		{
			goto RTN;
		}		
		//printf("\r\nID = %d", data);
		if(data != 0x02)
		{
			goto RTN;
		}
	  if(0 != pah8002_write_reg(0xe1, 0))	//write 0 to trigger Software Reset
		{
			goto RTN;
		}			
		
		
		//delay 5ms
		//규창 터치, 노말, 노말 et 모드 변경시 sw 리셋하는 신뢰성으로 인해 딜레이 x10배
  		delay_ms(5);
		//printf("<<< pah8002_sw_reset \r\n");
		return true;
RTN:
		//printf("<<< pah8002_sw_reset RTN\r\n");
		return false;		
}


static bool pah8002_start()
{	
		uint8_t data = 0;
		int samples_per_read =  HEART_RATE_MODE_SAMPLES_PER_READ ;
	  //printf(">>> pah8002_start \r\n");

		pah8002_wakeup();
		//delay_ms(5);
		if(0 != pah8002_write_reg(0x7f, 0x01))
		{
			goto RTN;
		}				
		else if(0 != pah8002_write_reg(0xea, (samples_per_read+1)))
		{
			goto RTN;
		}		
		else if(0 != pah8002_write_reg(0xd5, 1))	//TG enable. REQTIMER_ENABLE
		{
			goto RTN;
		}
		else if(0 != pah8002_read_reg(0xd5, &data))	//TG enable. REQTIMER_ENABLE
		{
			goto RTN;
		}

		//printf("<<< pah8002_start %d\r\n", data);
	
		return true;
RTN:
		//printf("<<< FAIL pah8002_start %d\r\n",data);
		return false;
}

static bool pah8002_touch_mode_init()
{
	int i =0 ;

	//printf(">>> pah8002_touch_mode_init \r\n");
	pah8002_wakeup();	
	//delay_ms(5);
	for(i = 0; i < INIT_TOUCH_REG_ARRAY_SIZE;i++)
	{
	    if ( pah8002_write_reg(  init_touch_register_array[i][0],
                                init_touch_register_array[i][1]) != 0 )
	    {
	       goto RTN;
	    }
  	}	
	//printf("<<< pah8002_touch_mode_init \r\n");
	return true;
RTN:
	//printf("<<< FAIL pah8002_touch_mode_init \r\n");
	return false;		
}

static bool pah8002_normal_mode_init()
{
	int i =0 ;
	//printf(">>> pah8002_normal_mode_init \r\n");
	pah8002_wakeup();
	//delay_ms(5);
	for(i = 0; i < INIT_PPG_REG_ARRAY_SIZE;i++)
	{
	    if ( pah8002_write_reg( init_ppg_register_array[i][0],
                                init_ppg_register_array[i][1]) != 0 )
	    {
	       goto RTN;
	    }
  	}	

	//printf("<<< pah8002_normal_mode_init \r\n");

	return true;
RTN:
	//printf("<<< FAIL pah8002_normal_mode_init \r\n");
	return false;		
}

static bool pah8002_stress_mode_init()
{
	int i =0 ;
	//printf("\r\n>>> pah8002_stress_mode_init");
	pah8002_wakeup();
	//delay_ms(5);
	//printf("\r\n [STRESS MODE] size %d", INIT_STRESS_REG_ARRAY_SIZE);
	for(i = 0; i < INIT_STRESS_REG_ARRAY_SIZE;i++)
	{
	    if ( pah8002_write_reg(  init_stress_register_array[i][0],
                                init_stress_register_array[i][1]) != 0 )
	    {
	       goto RTN;
	    }
 	 }	

	//printf("\r\n<<< pah8002_stress_mode_init");

	return true;
RTN:
	return false;		
}


static uint8_t pah8002_get_touch_flag_ppg_mode()
{
  static uint8_t touch_sts_output = 1 ;
  int32_t *s = (int32_t *)pah8002_ppg_data ;
  int32_t ch0 ;
  int32_t ch1 ;
  int64_t ir_rawdata;
  int i;
  static int touch_cnt = 0, no_touch_cnt = 0 ;
	//규창 default 600
  #define TouchDetection_Upper_TH (600)  
  #define TouchDetection_Lower_TH (512)
	
  #define TouchDetection_Count_TH (3)				//(3+1)*50ms = 200ms
  #define NoTouchDetection_Count_TH (3)			//(3+1)*50ms = 200ms
  
  for(i=0; i<HEART_RATE_MODE_SAMPLES_PER_READ; i+=TOTAL_CHANNELS)
  {
  	ch0 = *s;
  	ch1 = *(s+1);
  	ir_rawdata = ch0 - ch1 ;
  	ir_rawdata = (ir_rawdata * _ir_dac * _ir_expo)>>20 ;
		
  	//printf("\r\n ir_rawdata [%I64u]",ir_rawdata);
  	if( ir_rawdata > TouchDetection_Upper_TH)
  	{
			//printf("\r\n touch_cnt [%d]",touch_cnt);
  		touch_cnt++;
  		no_touch_cnt = 0;
  	}
  	else if( ir_rawdata < TouchDetection_Lower_TH)
  	{
			//printf("\r\n no_touch_cnt [%d]",no_touch_cnt);
  		no_touch_cnt++;
  		touch_cnt = 0 ;
  	}
  	else
  	{
  		touch_cnt = 0 ;
  		no_touch_cnt = 0;
  	}	
  	
  	s+=TOTAL_CHANNELS;
  } 
  
  if(touch_cnt > TouchDetection_Count_TH)
  {
  	touch_sts_output = 1;
  }
  else if( no_touch_cnt > NoTouchDetection_Count_TH)
  {
  	touch_sts_output = 0;
  }
 //printf("\r\n IR expo time 0x%x, IR DAC 0x%x   %d", _ir_expo, _ir_dac, ((ch0 - ch1)*_ir_dac*_ir_expo)>>20);
		
 // printf("\r\n<<< pah8002_get_touch_flag_ppg_mode %d, %d",touch_cnt, no_touch_cnt);
  //printf("\r\n<<< pah8002_get_touch_flag_ppg_mode %d", touch_sts_output);
  
  return touch_sts_output;						
}


bool pah8002_enter_normal_mode()
{
//	printf(">>> pah8002_enter_normal_mode\r\n");
	if(pahr_mode == NORMAL_MODE) return true;

	//pah8002_enter_suspend_mode();
	
	//1. software reset
	if( !pah8002_sw_reset())
		goto RTN;
					
	
  //2. load registers for normal mode
	if( !pah8002_normal_mode_init())
		goto RTN;
					
	pah8002_write_reg(0x7f, 0x00);	//Bank0
	pah8002_read_reg(0x0D, &_ir_expo);	// IR Exposure Time
	pah8002_write_reg(0x7f, 0x01);	//Bank1
	pah8002_read_reg(0xBA, &_ir_dac);	//IR Led DAC
	//printf("\r\n IR expo time 0x%x, IR DAC 0x%x", _ir_expo, _ir_dac);
	//3. enable sensor
	if( !pah8002_start())
		goto RTN;						
	pahr_mode = NORMAL_MODE;
	_pah8002_sleep_process_flag = 1;	
//	printf("<<< pah8002_enter_normal_mode ir_dac %x, ir_expo %x\r\n", _ir_dac, _ir_expo);
	return true;
RTN:
	return false ;				
}

bool pah8002_enter_stress_mode()
{
	//printf(">>> pah8002_enter_stress_mode\r\n");
	if(pahr_mode == PAHR_STRESS_MODE) return true;

	//1. software reset
	if( !pah8002_sw_reset())
		goto RTN;
	
	//2. load registers for normal mode
	if( !pah8002_stress_mode_init())
		goto RTN;
					
	pah8002_write_reg(0x7f, 0x00);	//Bank0
	pah8002_read_reg(0x0D, &_ir_expo);	// IR Exposure Time
	pah8002_write_reg(0x7f, 0x01);	//Bank1
	pah8002_read_reg(0xBA, &_ir_dac);	//IR Led DAC
	
	//3. enable sensor
	if( !pah8002_start())
		goto RTN;						
				
	pahr_mode = PAHR_STRESS_MODE;  				
	//printf("<<< pah8002_enter_stress_mode \r\n");
	return true;
RTN:
	return false ;				
}

bool pah8002_enter_touch_mode()
{
	//printf(">>> pah8002_enter_touch_mode\r\n");
	if(pahr_mode == TOUCH_MODE) return true;

	//pah8002_enter_suspend_mode();
	
	//1. software reset
	if( !pah8002_sw_reset() )
		goto RTN;

	
	
	//2. load registers for touch mode
	if( !pah8002_touch_mode_init())
		goto RTN;
					
	//3. enable sensor
	if( !pah8002_start())
		goto RTN;
						
	pahr_mode = TOUCH_MODE;
	//슬립모드 용도인듯 하다
	//_pah8002_sleep_process_flag = 0; //hkim-1008
	//printf("<<< pah8002_enter_touch_mode\r\n");
	return true;
		
RTN:
	return false ;				
}



bool pah8002_get_touch_flag( uint8_t *touch_flag)
{
	//printf(">>> pah8002_touch_status \r\n");
	pah8002_wakeup();
	//delay_ms(5);
	if(0 != pah8002_write_reg(0x7f, 0x02))
	{
		goto RTN;
	}	
	else if(0 != pah8002_read_reg(0x45, touch_flag))	
	{
		goto RTN;
	}	
	//printf("<<< pah8002_touch_status %d\r\n", *touch_flag);
	return true;
	
RTN:		
	return false;
}

static int pah8002_wakeup()
{
	int retry = 0 ;
	int success = 0 ;
	uint8_t data = 0 ;
	pah8002_read_reg(0, &data);
	pah8002_read_reg(0, &data);

	do
	{
		pah8002_write_reg(0x7f, 0x00);
		pah8002_read_reg(0, &data);
		if(data == 0x02) success++;
		else success = 0 ;
					
		if(success >=2) break;
		retry ++;
				
	}while(retry < 20);	

	if(_chip_id == 0)
	{
	  pah8002_read_reg(0x02, &data);
		
		_chip_id = data & 0xF0 ;
		/*
		if(_chip_id != 0xD0)
	  {
			debug_//printf("Not support anymore\r\n");
			while(1){};				
		}
		*/
	}
	
	pah8002_write_reg(0x7f, 0x02);
	pah8002_write_reg(0x70, 0x00);
		
	//printf("pah8002_wakeup retry %d \r\n", retry);
			
	return retry;
}

bool pah8002_enter_suspend_mode()
{
	int i = 0 ;
	
	//printf("\r\n<<<<<<<<<<<pah8002_enter_suspend_mode");	 
	pah8002_sw_reset();
	
	for(i = 0; i < SUSPEND_REG_ARRAY_SIZE;i++)
	{
	    if ( pah8002_write_reg(suspend_register_array[i][0],
                                suspend_register_array[i][1]) != 0 )
	    {
	       return false;
	    }
  }	
	pahr_mode = SUSPEND_MODE;  

  return true;
}

bool _pah8002_task()
{
	uint8_t cks[4] = {0,};
	uint8_t int_req = 0;
			
	//printf("\r\n>>> pah8002_task");
	pah8002_wakeup();
	//delay_ms(5);
	
	
	if(0 != pah8002_write_reg(0x7f, 0x02))
	{
		//printf("\r\n : Switch to Bank-2 fail");
	}
  else if(0 != pah8002_read_reg(0x73, &int_req))
	{
		//printf("\r\n : INT_reg_array fail");
	}
	else
	{
		if( (int_req & 0x04) != 0)
		{
			//overflow
			while(1){
				//printf("int_req & 0x04??????\n");
			}
		}
		
		if( (int_req & 0x02) != 0)
		{
			//touch
		 	//printf("touch interrupt\n");
		}
		
		if( (int_req & 0x08) != 0)
		{
			//overflow
			while(1){
			//printf("int_req & 0x08??????\n");			
			}
		}
		//printf("int_req & 0x01!= 0 ?????? %d \r\n",int_req & 0x01);
		if( (int_req & 0x01) != 0)
		{
   		int samples_per_read = HEART_RATE_MODE_SAMPLES_PER_READ ;
			//printf("\r\nFIFO interrupt");
			//pah8002_get_touch_flag(&state->pah8002_touch_flag);
			if(0 != pah8002_write_reg(0x7f, 0x03))
			{}
			else if(0 != pah8002_burst_read_reg(0, pah8002_ppg_data, samples_per_read*4))
			{}
			else if(0 != pah8002_write_reg(0x7f, 0x02))
			{}
			else if(0 != pah8002_burst_read_reg(0x80, cks, 4))   
			{}
			else if(0 != pah8002_write_reg(0x75, 0x01))	//read fifo first, then clear SRAM FIFO interrupt
			{}
			else if(0 != pah8002_write_reg(0x75, 0x00))
			{}
			else
			{
				uint32_t *s = (uint32_t *)pah8002_ppg_data ;
				uint32_t cks_cal = *s;
				uint32_t cks_rx = *((uint32_t *)cks) ;
				uint32_t i ;
				
				//checksum compare
				for(i=1; i<samples_per_read; i++)
				{
					cks_cal = cks_cal ^ (*(s+i)) ;
				}
				
				if(cks_cal != cks_rx)
				{
					//printf("\r\nchecksum error");
				}
				else
				{
					//printf("\r\nchecksum OK %d", cks_cal);
				}	
				_touch_flag = pah8002_get_touch_flag_ppg_mode();				
			}
		}
		else
		{
			//printf("\r\nnot fifo interrupt%d", int_req);
			//전임자 추가... 에러나서 리턴 시켜야하는 왜 터치플래그를 바꿀까...?
		  _touch_flag = 0;
			
			//전임자는 false리턴 못하게 함
			return false;
		}
	}
	
	//printf("\r\n<<< pah8002_task");
	return true;
}

static bool pah8002_normal_long_et_mode_init()
{
	int i =0 ;
	//printf("\r\n>>> pah8002_normal_long_et_mode_init");
	pah8002_wakeup();
	//delay_ms(5);
	for(i = 0; i < INIT_PPG_LONG_REG_ARRAY_SIZE;i++)
	{
	    if ( pah8002_write_reg( init_ppg_long_register_array[i][0],
                                init_ppg_long_register_array[i][1]) != 0 )
	    {
	       goto RTN;
	    }
  }
	
	
	//printf("\r\n<<< pah8002_normal_long_et_mode_init ");

	return true;
RTN:
	return false;		
}

static bool pah8002_enter_normal_long_et_mode()
{
	//printf(">>> pah8002_enter_normal_long_et_mode\r\n");
	if(pahr_mode == NORMAL_LONG_ET_MODE) return true;

	//1. software reset
	if( !pah8002_sw_reset())
		goto RTN;
					
//2. load registers for normal mode
	if( !pah8002_normal_long_et_mode_init())
		goto RTN;
					
	pah8002_write_reg(0x7f, 0x00);	//Bank0
	pah8002_read_reg(0x0D, &_ir_expo);	// IR Exposure Time
	pah8002_write_reg(0x7f, 0x01);	//Bank1
	pah8002_read_reg(0xBA, &_ir_dac);	//IR Led DAC
	
	//3. enable sensor
	if( !pah8002_start())
		goto RTN;						
	pahr_mode = NORMAL_LONG_ET_MODE;  								
	//printf("<<< pah8002_enter_normal_long_et_mode ir_dac %x, ir_expo %x\r\n", _ir_dac, _ir_expo);
	return true;
RTN:
	return false ;						
}

//규창 모드체인저 추가 초당 1회 인터럽트고 여기는 꼭 거친다.
//그럼 10회는 노말모드로 측정하다 다음 10회는 리셋걸고 롱모드
//그리고 다음 10회는 리셋걸고 노말모드. 다시 또 10회는 리셋걸고 롱모드
//리셋레지스터 건들면서 무한 반복 측정...
//메인에서 평균 수식하면서 15초 마다 터치모드... 일단은 냅둬보자.... 어차피
// STS때문에 터치모드 동작도 안한다.
int modechange = 0;
static void pah8002_dyn_switch_ppg_mode()
{
	modechange++;
	uint8_t b2a4, b2a5 ;
	uint16_t value ;
	pah8002_wakeup();
	//delay_ms(5);
	pah8002_write_reg(0x7F, 0x02);
	pah8002_read_reg(0xa4, &b2a4);
	pah8002_read_reg(0xa5, &b2a5);
	value = b2a5;
	value <<= 8 ;
	value += b2a4 ;
	//printf("\r\nREADY TO LONG ET? %d ",value);
	if (value > 1440)//160
	{
		pah8002_enter_normal_long_et_mode();
	}
}


//---------------------------------------PAH8002 functions-----------------------------------------------
bool pah8002_init(void)
{
	uint8_t ret = 0;
	uint32_t open_size = 0;
	
	//Algorithm initialization
	_pah8002_data.frame_count = 0 ;
	_pah8002_data.nf_ppg_channel = TOTAL_CHANNELS_FOR_ALG;
	_pah8002_data.nf_ppg_per_channel = HEART_RATE_MODE_SAMPLES_PER_CH_READ;
	_pah8002_data.ppg_data = (int32_t *)pah8002_ppg_data;
#ifdef MEMS_ZERO
	memset(_mems_data, 0, sizeof(_mems_data));
	_pah8002_data.nf_mems = HEART_RATE_MODE_SAMPLES_PER_CH_READ;
	_pah8002_data.mems_data = _mems_data;
#endif
	open_size = pah8002_query_open_size();
	_pah8002_alg_buffer = malloc(open_size);
	//printf("\r\n [PAH8002] allocate algorithm buffer(size %d byte).", open_size);
	if (_pah8002_alg_buffer == NULL) 
	{
		//printf("\r\n [PAH8002] can't allocate algorithm buffer(size %d byte). stop", open_size);
		return false;
	}
	
	
	
	ret = pah8002_open(_pah8002_alg_buffer);
	if (ret != MSG_SUCCESS)
		return false;
		
	// Set 0: +/-2G, 1: +/-4G, 2: +/-8G, 3: +/-16G
	if (MSG_SUCCESS != pah8002_set_param(PAH8002_PARAM_IDX_GSENSOR_MODE, 1))   
		return false;
	// IR x2
	if (MSG_SUCCESS != pah8002_set_param(PAH8002_PARAM_IDX_HAS_IR_CH, 2))   
		return false;

//	printf("\r\nPPG CH#, %d", TOTAL_CHANNELS_FOR_ALG);
	delay_ms(300);
#ifdef PPG_MODE_ONLY
	return pah8002_enter_normal_mode();
#else	
	return pah8002_enter_touch_mode();
#endif	
}

void pah8002_deinit(void)
{
	pah8002_enter_suspend_mode();
	
	pah8002_close();
	
	if (_pah8002_alg_buffer)
	{
		free(_pah8002_alg_buffer);
		_pah8002_alg_buffer = NULL;
	}
}

void pah8002_log(void)
{
	int i = 0 ;
	uint32_t *ppg_data = (uint32_t *)_pah8002_data.ppg_data ;
	int16_t *mems_data = _pah8002_data.mems_data ;
	printf("\r\nFrame Count, %d", _pah8002_data.frame_count);
	printf("\r\nTime, %d", _pah8002_data.time);
	printf("\r\nPPG, %d, %d, ", _pah8002_data.touch_flag, _pah8002_data.nf_ppg_per_channel);
	for(i=0; i<_pah8002_data.nf_ppg_channel * _pah8002_data.nf_ppg_per_channel; i++)
	{
		printf("%d, ", *ppg_data);
		ppg_data ++;
	}
	//printf("\n");
	printf("\r\nMEMS, %d, ", _pah8002_data.nf_mems);
	for(i=0; i<_pah8002_data.nf_mems*3; i++)
	{
		printf("%d, ", *mems_data);
		mems_data ++;
	}
	printf("\n");	
}


 void data_convert_4ch_to_3ch(uint32_t *pdata, uint32_t len)
 {
 	uint32_t i = 0, j = 0;
 	for(i=0, j=2; j<len; i+=3, j+=4)
 	{
 		*(pdata+i+1) = *(pdata+j);
 		*(pdata+i+2) = *(pdata+j+1);
 		//*(pdata+i+3) = *(pdata+j+2); /* hkim-temp */
 	}	
 }


extern unsigned char g_hr;	
void pah8002_task(void)
{
	uint8_t ret;
	float hr = 0;
	int hr_trust_level = 0;
	uint32_t sys_tick;
	if(_pah8002_interrupt == 1)
	{
		_pah8002_interrupt = 0;

		if(pahr_mode == TOUCH_MODE)
		{
			if (!pah8002_get_touch_flag(&_touch_flag))
			{
				//printf("\r\npah8002_get_touch_flag() fail \r\n");
				while(1);
			}

			if (_touch_flag)
			{
				if (!pah8002_enter_normal_mode())
				{
					//printf("\r\npah8002_enter_normal_mode() fail \r\n");
					while(1);
				}
				
				_timestamp = get_sys_tick();
				//규창 우리는 가속센서 다른데서 컨트롤
				//accelerometer_start();
			}
		}
		else if(pahr_mode == NORMAL_MODE || pahr_mode == NORMAL_LONG_ET_MODE)
		{
			//전임자는 리턴 안하고 _pah8002_task();강제 진입
			if(!_pah8002_task()) return;
			pah8002_dyn_switch_ppg_mode();

#ifdef PPG_MODE_ONLY
#else			
			if(_touch_flag == 0)
			{
				//전임자 루틴 중 전역 심박 초기화용
//				printf("\r\n * No touch");
				pah8002_enter_touch_mode();
				g_hr = 0;
				motion_hr = 0;
				trustLevel = 0;
				//규창 우리는 가속센서 다른데서 컨트롤 
				//accelerometer_stop();				
			}
#endif
			
			//process algorithm
#ifdef MEMS_ZERO
#else			
			accelerometer_get_fifo(&_pah8002_data.mems_data, &_pah8002_data.nf_mems);
#endif
			sys_tick = get_sys_tick();
			_pah8002_data.time = sys_tick - _timestamp;
			_timestamp = sys_tick;
			_pah8002_data.touch_flag = _touch_flag;
			//data_convert_4ch_to_3ch((uint32_t *)pah8002_ppg_data, HEART_RATE_MODE_SAMPLES_PER_READ); /* KKK */
			
			// log 3ch ppg_data before pah8002_entrance()
			//pah8002_log();
			
			if(testPPG == true){
				pah8002_log_Greencomm();
			}
			
			
			ret = pah8002_entrance(&_pah8002_data);
			if((ret & 0x0f) != 0)
			{
				switch(ret) //check error status
				{
					case MSG_ALG_NOT_OPEN:
//						printf("\r\nAlgorithm is not initialized.");
						break;
					case MSG_MEMS_LEN_TOO_SHORT:
//						printf("\r\nMEMS data length is shorter than PPG data length.");
						break;
					case MSG_NO_TOUCH:
//						printf("\r\nPPG is no touch.");
						break;
					case MSG_PPG_LEN_TOO_SHORT:
//						printf("\r\nPPG data length is too short.");
						break;
					case MSG_FRAME_LOSS:
//						printf("\r\nFrame count is not continuous.");
						break;
					case MSG_ALG_REOPEN:
//						printf("\r\nMSG_ALG_REOPEN.");
						break;
				  case MSG_INVALID_ARGUMENT:
//						printf("\r\nMSG_INVALID_ARGUMENT.");
						break;
					case MSG_NO_MEM:
//						printf("\r\nMSG_NO_MEM");
						break;
					case MSG_SIGNAL_POOR:
//						printf("\r\nMSG_SIGNAL_POOR");
						break;
				}
			}
			if ((ret & 0xf0) == MSG_HR_READY)
			{
				
				
				pah8002_get_hr(&hr);
				//printf("\r\nHR = %d", (int)(hr));
				
				
				//규창 전임자가 선언한 심박 전역 변수... 지금은 어쩔수 없다.
				motion_hr=(int)(hr);
				//g_hr = motion_hr;
				//규창 전임자가 선언한 심박 신뢰도  전역 변수... 지금은 어쩔수 없다.
			  
				pah8002_get_hr_trust_level(&hr_trust_level);
				trustLevel = (int)hr_trust_level;
			}
			_pah8002_data.frame_count++;
		}
	}
	
}


void pah8002_intr_isr(void)
{
	_pah8002_interrupt = 1 ;
}


void pah8002_log_Greencomm(void)
{
	int i = 0 ;
	int j = 1;
	uint32_t *ppg_data = (uint32_t *)_pah8002_data.ppg_data ;
	uint32_t *ch3 = {0,};
	for(i=0; i<_pah8002_data.nf_ppg_channel * _pah8002_data.nf_ppg_per_channel; i++)
	{
		if(j > 33){
		if (j% 3 == 0){
			ch3 = ppg_data;
			//memcpy(*ppg_data, &ch3, sizeof(*ppg_data));
		
		
	    if ( *ppg_data == NULL || *ppg_data == 0){
				printf("\r\nFAIL");
			 printf("%d, ", *ppg_data);
			  break;
		  }
		  if ( ((ppg_data - ch3) > 1500) || ((ppg_data - ch3) < (-1500) )){
			 printf("\r\nFAIL");
			 printf("%d, ", *ppg_data);
			  break;
		  }
		 }
		 
	 }else {
		 if ((j%3) == 0 && (*ppg_data == NULL || *ppg_data == 0 )){
			 printf("\r\nFAIL");
			 printf("%d, ", *ppg_data);
			  break;
		 }
	 }
	 
		printf("%d, ", *ppg_data);
	  if (i >= 59){
			 printf("\r\n Test ALLOK");
			  break;
		 }
		ppg_data ++;
		 j++;
	  
	}
	//빨간색 포스트잇 1차 기구로 170000 의 PPG RAW를 가짐
	/*if (((*ch3/1000) < 1024) || ((*ch3/1000) > 4096))	{
	 printf("FAIL");
		printf("\r\n%d", *ch3/1000);
	}
	else if(ch3 == 0 ){
	 printf("FAIL");
		printf("\r\n%d", *ch3/1000);
	}
	else{
	 printf("OK");
		printf("\r\n%d", *ch3/1000);
	}*/
	//pah8002_deinit();
	gc_protocol_ctrl_pah8002(DAILY_MODE, 0); 
	testPPG=false;
}
