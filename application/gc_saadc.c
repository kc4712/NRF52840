/** @file
 * @brief SAADC for battery sensing
 *
 *
 */

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "nrf.h"
#include "nrf_drv_saadc.h"
#include "app_uart.h"
#include "app_error.h"
#include "app_util_platform.h"
#include "gc_saadc.h"
#include "gc_gpio.h"
#include "nrf_drv_gpiote.h"
#include "define.h"
#include "SEGGER_RTT.h"
#include "nrf_uart.h"
#include "nrf_uarte.h"

#define SEGGER_RTT_printf(...)

/* adc level = 1313V) ~ 185(4.3V) */
#ifdef GC_COACH_FIT_HW_1_1
unsigned char battery_adc_table[97]=
{
	0,	/* 137 , 3.35V*/
	0,	/* 138 */
	5,	/* 139, 3.4V */
	5,	/* 140 */
	5,	/* 140 */
	5,	/* 141 */
	5,	/* 142 */
	6,	/* 143 */
	6,	/* 144 */
	7,	/* 145, 3.45V */
	7,	/* 146 */
	9,	/* 147 */
	9,	/* 148 */
	9,	/* 149 */
	10,	/* 150, 3.5V*/
	10,	/* 151 */
	10,	/* 152 */
	10,	/* 153 */
	15,	/* 154 */
	17,	/* 155, 3.55V */
	17,	/* 156 */
	18,	/* 157 */
	18,	/* 158 */
	19,	/* 159 */
	19,	/* 160 */
	20,	/* 161, 3.6V */	
	20,	/* 162 */
	20,	/* 163 */
	20,	/* 164 */
	23,	/* 165 */
	23,	/* 166 */
	23,	/* 167 */
	26,	/* 168, 3.65V */
	26,	/* 169 */
	26,	/* 170 */
	28,	/* 171 */
	28,	/* 172 */
	28,	/* 173 */
	30,	/* 174, 3.7V */
	30,	/* 175 */
	30,	/* 176 */
	33,	/* 177 */
	33,	/* 178 */
	33,	/* 179 */
	35,	/* 180, 3.75V */
	35,	/* 181 */
	38,	/* 182 */
	38,	/* 183 */
	38,	/* 184 */
	40,	/* 185, 3.8V */
	40,	/* 186 */
	40,	/* 187 */
	40,	/* 188 */
	45,	/* 189 */
	45,	/* 190 */
	50,	/* 191, 3.85V */
	50,	/* 192 */
	60,	/* 193 */
	60,	/* 194 */
	65,	/* 195 */
	65,	/* 196 */
	70,	/* 197, 3.9V*/	
	70,	/* 198 */
	70,	/* 199 */
	70,	/* 200 */
	73,	/* 201 */
	73,	/* 202 */
	76,	/* 203, 3.95V */
	76,	/* 204 */
	76,	/* 205 */
	78,	/* 206 */
	78,	/* 207 */
	78,	/* 208 */
	80,	/* 209, 4V */
	80,	/* 210 */
	80,	/* 211 */
	83,	/* 212 */
	83,	/* 213 */
	85,	/* 214, 4.05V */
	85,	/* 215 */	
	85,	/* 216 */
	88,	/* 217 */
	88,	/* 218 */
	88,	/* 219 */
	90,	/* 220, 4.10V */
	90,	/* 221 */
	90,	/* 222 */
	90,	/* 223 */
	90,	/* 224 */
	90,	/* 225 */
	95,	/* 226, 4.15V */
	95,	/* 227 */
	95,	/* 228 */
	98,	/* 229 */
	98,	/* 230 */
	100,	/* 231, 4.2V */
	100,	/* 232 */
};
#elif defined GC_COACH_FIT_HW_1_2 
/* If 3.0V ADC=588,589,590 */
#if 0 
unsigned char battery_adc_table[97]=
{
	0,	/* 655 , 3.35V*/
	0,	/* 656 */
	
	5,	/* 665, 3.4V */
	5,	/* 666 */
	6,	/* 667 */
	6,	/* 668 */
	
	7,	/* 675, 3.45V */
	7,	/* 676 */
	8,	/* 677 */
	8,	/* 678 */
	9,	/* 679 */
	9,	/* 680 */
	
	10,	/* 685, 3.5V*/
	10,	/* 686 */
	15,	/* 687 */
	15,	/* 688 */

	
	17,	/* 695, 3.55V */
	18,	/* 696 */
	19,	/* 697 */
	19,	/* 698 */

	
	20,	/* 704, 3.6V */	
	21,	/* 705 */
	22,	/* 706 */
	23,	/* 707 */
	23,	/* 708 */

	
	
	26,	/* 714, 3.65V */
	27,	/* 715 */
	28,	/* 716 */
	28,	/* 717 */

	
	30,	/* 724, 3.7V */
	31,	/* 725 */
	32,	/* 726 */
	33,	/* 727 */
	33,	/* 728 */

	
	35,	/* 734, 3.75V */
	35,	/* 735 */
	38,	/* 736 */
	38,	/* 737 */
	38,	/* 738 */
	
	40,	/* 744, 3.8V */
	40,	/* 745 */
	40,	/* 746 */
	40,	/* 747 */
	45,	/* 748 */
	45,	/* 749 */
	
	
	50,	/* 754, 3.85V */
	50,	/* 755 */
	60,	/* 756 */
	60,	/* 757 */
	65,	/* 758 */
	65,	/* 759 */
	
	
	70,	/* 764, 3.9V*/	
	70,	/* 765 */
	70,	/* 766 */
	70,	/* 767 */
	73,	/* 768 */
	73,	/* 769 */
	
	
	76,	/* 774, 3.95V */
	76,	/* 775 */
	76,	/* 776 */
	78,	/* 777 */
	78,	/* 778 */
	78,	/* 779 */
	
	80,	/* 784, 4V */
	80,	/* 785 */
	80,	/* 786 */
	83,	/* 787 */
	83,	/* 788 */
	
	85,	/* 794, 4.05V */
	85,	/* 795 */	
	85,	/* 796 */
	88,	/* 797 */
	88,	/* 798 */
	88,	/* 799 */
	
	90,	/* 804, 4.10V */
	90,	/* 805 */
	90,	/* 806 */
	90,	/* 807 */
	90,	/* 808 */
	90,	/* 809 */
	
	95,	/* 814, 4.15V */
	95,	/* 815 */
	95,	/* 816 */
	98,	/* 817 */
	98,	/* 818 */
	100,	/* 824, 4.2V */
	100,	/* 825 */
};
#endif /* if 0 */
#else
#endif 


#define SAMPLES_IN_BUFFER 1               //Number of SAADC samples in RAM before returning a SAADC event. Do not change this constant for low power SAADC
//static nrf_saadc_value_t       m_buffer_pool[2][SAMPLES_IN_BUFFER];
//static uint32_t                m_adc_evt_counter;
volatile uint8_t g_battery=50;
//규창 원본 타입 
//volatile uint32_t gc_saadc=0;
volatile int16_t gc_saadc=0;

volatile uint8_t pre_battery=0;


//규창 배터리플래그는 메인에서 쓰니 위로 빼둠
volatile uint8_t gc_adc_battery_flag;

//#ifdef _NRF52_HW_1_2_ADC_SPI_WORKAROUND_
//volatile uint8_t gc_adc_battery_flag;
//#endif /* _NRF52_HW_1_2_ADC_SPI_WORKAROUND_ */

#ifdef GC_COACH_FIT_HW_1_1
uint8_t get_battery_level(uint32_t adc)
{
	short conv;
	
	//printf("\r\n ADC %d", adc);

	conv=adc-137;

	if(conv<0)
		conv=0;

	if(conv>96)
		conv=96;
	
	return battery_adc_table[conv];
}
#elif defined GC_COACH_FIT_HW_1_2
uint8_t get_battery_level(uint32_t adc)
{
	//SEGGER_RTT_printf(0,"\r\n ADC %d", adc);

	if(adc < 660)
		return 0;

	if(adc >= 834)
		return 100;
	/*
	if (adc >= 660 && adc < 670)
		return 5;
	else if  (adc >= 670 && adc < 675)
		return 8;
	else if  (adc >= 675 && adc < 680)
		return 9;
	else if  (adc >= 680 && adc < 685)
		return 10;
	else if  (adc >= 685 && adc < 690)
		return 15;
	else if  (adc >= 690 && adc < 695)
		return 17;
	else if  (adc >= 695 && adc < 700)
		return 19;
	else if  (adc >= 700 && adc < 705)
		return 22;
	else if  (adc >= 705 && adc < 710)
		return 25;
	else if  (adc >= 710 && adc < 715)
		return 27;
	else if  (adc >= 715 && adc < 720)
		return 29;
	else if  (adc >= 720 && adc < 725)
		return 30;
	else if  (adc >= 725 && adc < 730)
		return 33;
	else if  (adc >= 730 && adc < 735)
		return 35;
	else if  (adc >= 735 && adc < 740)
		return 38;
	else if  (adc >= 740 && adc < 745)
		return 40;
	else if  (adc >= 745 && adc < 750)
		return 43;
	else if  (adc >= 750 && adc < 755)
		return 50;
	else if  (adc >= 755 && adc < 760)
		return 65;
	else if  (adc >= 760 && adc < 765)
		return 70;
	else if  (adc >= 765 && adc < 770)
		return 73;
	else if  (adc >= 770 && adc < 775)
		return 76;
	else if  (adc >= 775 && adc < 780)
		return 78;
	else if  (adc >= 780 && adc < 785)
		return 80;		
	else if  (adc >= 785 && adc < 790)
		return 83;
	else if  (adc >= 790 && adc < 795)
		return 85;
	else if  (adc >= 795 && adc < 800)
		return 88;
	else if  (adc >= 805 && adc < 810)
		return 90;
	else if  (adc >= 810 && adc < 815)
		return 95;
	else if  (adc >= 815 && adc < 822)
		return 98;	
	else if  (adc >= 822 && adc < 834)
		return 99;	
	else {}
		
	return 0;
	*/
	if(adc >= 820)
	{
		return 100;
	}
	else if(adc >=819 )
	{
		return 99;
	}
	else if(adc >= 817)
	{
		return 98;
	}
	else if(adc >= 815)
	{
		return 97;
	}
	else if(adc >= 813)
	{
		return 96;
	}
	else if(adc >= 811)
	{
		return 95;
	}
	else if(adc >= 809)
	{
		return 94;
	}
	else if(adc >= 807)
	{
		return 93;
	}
	else if(adc >= 806)
	{
		return 92;
	}
	else if(adc >= 804)
	{
		return 91;
	}
	else if(adc >= 802)
	{
		return 90;
	}
	else if(adc >= 801)
	{
		return 89;
	}
	else if(adc >= 799)
	{
		return 88;
	}
	else if(adc >= 797)
	{
		return 87;
	}
	else if(adc >= 795)
	{
		return 86;
	}
	else if(adc >= 793)
	{
		return 85;
	}
	else if(adc >= 792)
	{
		return 84;
	}
	else if(adc >= 790)
	{
		return 83;
	}
	else if(adc >= 789)
	{
		return 82;
	}
	else if(adc >= 788)
	{
		return 81;
	}
	else if(adc >= 786)
	{
		return 80;
	}
	else if(adc >= 785)
	{
		return 79;
	}
	else if(adc >= 783)
	{
		return 78;
	}
	else if(adc >= 781)
	{
		return 77;
	}
	else if(adc >= 780)
	{
		return 76;
	}
	else if(adc >= 778)
	{
		return 75;
	}
	else if(adc >= 777)
	{
		return 74;
	}
	else if(adc >= 776)
	{
		return 73;
	}
	else if(adc >= 775)
	{
		return 72;
	}
	else if(adc >= 773)
	{
		return 71;
	}
	else if(adc >= 772)
	{
		return 70;
	}
	else if(adc >= 770)
	{
		return 69;
	}
	else if(adc >= 769)
	{
		return 68;
	}
	else if(adc >= 768)
	{
		return 67;
	}
	else if(adc >= 766)
	{
		return 66;
	}
	else if(adc >= 765)
	{
		return 65;
	}
	else if(adc >= 763)
	{
		return 64;
	}
	else if(adc >= 762)
	{
		return 63;
	}
	else if(adc >= 761)
	{
		return 62;
	}
	else if(adc >= 759)
	{
		return 61;
	}
	else if(adc >= 758)
	{
		return 60;
	}
	else if(adc >= 757)
	{
		return 59;
	}
	else if(adc >= 756)
	{
		return 58;
	}
	else if(adc >= 755)
	{
		return 56;
	}
	else if(adc >= 754)
	{
		return 55;
	}
	else if(adc >= 753)
	{
		return 54;
	}
	else if(adc >= 752)
	{
		return 53;
	}
	else if(adc >= 751)
	{
		return 52;
	}
	else if(adc >= 750)
	{
		return 50;
	}
	else if(adc >= 749)
	{
		return 48;
	}
	else if(adc >= 747)
	{
		return 45;
	}
	else if(adc >= 746)
	{
		return 42;
	}
	else if(adc >= 745)
	{
		return 40;
	}
	else if(adc >= 743)
	{
		return 39;
	}
	else if(adc >= 742)
	{
		return 37;
	}
	else if(adc >= 740)
	{
		return 34;
	}
	else if(adc >= 739)
	{
		return 29;
	}
	else if(adc >= 738)
	{
		return 28;
	}
	else if(adc >= 737)
	{
		return 27;
	}
	else if(adc >= 736)
	{
		return 26;
	}
	else if(adc >= 735)
	{
		return 25;
	}
	else if(adc >= 734)
	{
		return 23;
	}
	else if(adc >= 733)
	{
		return 21;
	}
	else if(adc >= 731)
	{
		return 20;
	}
	else if(adc >= 730)
	{
		return 18;
	}
	else if(adc >= 727)
	{
		return 15;
	}
	else if(adc >= 725)
	{
		return 14;
	}
	else if(adc >= 724)
	{
		return 13;
	}
	else if(adc >= 723)
	{
		return 11;
	}
	else if(adc >= 722)
	{
		return 9;
	}
	else if(adc >= 719)
	{
		return 7;
	}
	else if(adc >= 717)
	{
		return 6;
	}
	else if(adc >= 714)
	{
		return 5;
	}
	else if(adc >= 706)
	{
		return 4;
	}
	else if(adc >= 699)
	{
		return 3;
	}
	else if(adc >= 688)
	{
		return 2;
	}
	else if(adc >= 680)
	{
		return 1;
	}
	else
	{
		return 0;
	}
}
#else
#endif /* GC_COACH_FIT_HW_1_1 */

/* nrf saadc 이벤트 콜백 이제 사용안함
void gc_disable_saadc()
{
			nrf_drv_saadc_abort();

			nrf_drv_saadc_uninit();
			NRF_SAADC->INTENCLR = (SAADC_INTENCLR_END_Clear << SAADC_INTENCLR_END_Pos);
			NVIC_ClearPendingIRQ(SAADC_IRQn);

			nrf_drv_saadc_channel_uninit(0);
			gc_adc_battery_flag = GC_ADC_BATTERY_FLAG_FINISH;
			m_adc_evt_counter = 0;
}

void saadc_callback(nrf_drv_saadc_evt_t const * p_event)
{
	if (p_event->type == NRF_DRV_SAADC_EVT_DONE)
	{
			err_code = nrf_drv_saadc_buffer_convert(p_event->data.done.p_buffer, SAMPLES_IN_BUFFER);
			APP_ERROR_CHECK(err_code);
		
			printf("\r\nADC event number: %d\r\n",(int)m_adc_evt_counter);
			for (int i = 0; i < SAMPLES_IN_BUFFER; i++)
			{
				 printf("\r\n  p_event->data.done.p_buffer[i] = %d ", p_event->data.done.p_buffer[i]);
					gc_saadc = p_event->data.done.p_buffer[i];
					printf("\r\n saadc_callback %d ", gc_saadc);
					
					g_battery = get_battery_level(gc_saadc);
				
					if(g_battery == 0)
					{
						g_battery = pre_battery;
					}
					else
					{
						pre_battery  = g_battery;
					}
					//SEGGER_RTT_printf(0,"\r\n battery : %d(percent)", g_battery);
			}
			m_adc_evt_counter++;
			
			//NRF_SAADC->INTENCLR = (SAADC_INTENCLR_END_Clear << SAADC_INTENCLR_END_Pos);
			
//#ifdef _NRF52_HW_1_2_ADC_SPI_WORKAROUND_
		///nrf_drv_saadc_abort();
//#endif // _NRF52_HW_1_2_ADC_SPI_WORKAROUND_ 
			//nrf_drv_saadc_uninit();
     //NRF_SAADC->INTENCLR = (SAADC_INTENCLR_END_Clear << SAADC_INTENCLR_END_Pos);
			//NVIC_ClearPendingIRQ(SAADC_IRQn);

			//nrf_drv_saadc_channel_uninit(0);
			//gc_adc_battery_flag = GC_ADC_BATTERY_FLAG_FINISH;
			//m_adc_evt_counter = 0;
			

//#ifdef _NRF52_HW_1_2_ADC_SPI_WORKAROUND_			
			
		
			gc_adc_battery_flag = GC_ADC_BATTERY_FLAG_FINISH;
			m_adc_evt_counter = 0;
			//nrf_gpio_pin_set(GC_GPIO_PIN_HR_POWER_EN);
			//gc_spi_pah8002_init();
			//pah8002_deinit();
			//pah8002_init();
			//nrf_saadc_task_trigger(NRF_SAADC_TASK_STOP);
			
		
//#endif // _NRF52_HW_1_2_ADC_SPI_WORKAROUND_ 
	}
}
*/


//전역 변수 ADC측정값
nrf_saadc_value_t result;

//본 타이머 핸들러는 메인에서 초기화 해서 부르고 있음
/**@brief Function for handling the Battery measurement timer timeout.
 *
 * @details This function will be called each time the battery level measurement timer expires.
 *          This function will start the ADC.
 *
 * @param[in] p_context   Pointer used for passing some arbitrary information (context) from the
 *                        app_start_timer() call to the timeout handler.
 */
void battery_level_meas_timeout_handler(void * p_context)
{
			UNUSED_PARAMETER(p_context);
	
			nrf_saadc_channel_config_t config;
			config.acq_time = NRF_SAADC_ACQTIME_10US;
			config.gain = NRF_SAADC_GAIN1_6;
			config.mode = NRF_SAADC_MODE_SINGLE_ENDED;
			config.pin_p = NRF_SAADC_INPUT_AIN0;
			config.pin_n = NRF_SAADC_INPUT_DISABLED; // Single ended -> should be ground to zero
			config.reference = NRF_SAADC_REFERENCE_INTERNAL;
			config.resistor_p = NRF_SAADC_RESISTOR_DISABLED;
			config.resistor_n = NRF_SAADC_RESISTOR_DISABLED;
		
			nrf_saadc_enable();
			nrf_saadc_channel_init(NRF_SAADC_INPUT_AIN0, &config);
			nrf_saadc_buffer_init(&result,1); // One sample
	
	
			gc_adc_battery_flag = GC_ADC_BATTERY_FLAG_START;
			
			nrf_saadc_task_trigger(NRF_SAADC_TASK_START);

			while(!nrf_saadc_event_check(NRF_SAADC_EVENT_STARTED)){
						//printf(" NRF_SAADC_EVENT_STARTED");
			}
			nrf_saadc_event_clear(NRF_SAADC_EVENT_STARTED);

			nrf_saadc_task_trigger(NRF_SAADC_TASK_SAMPLE);


			while(!nrf_saadc_event_check(NRF_SAADC_EVENT_END)){
						//printf(" NRF_SAADC_EVENT_END");
			}

			nrf_saadc_event_clear(NRF_SAADC_EVENT_END);

			nrf_saadc_task_trigger(NRF_SAADC_TASK_STOP);
			while(!nrf_saadc_event_check(NRF_SAADC_EVENT_STOPPED)){
						//printf(" NRF_SAADC_EVENT_STOPPED");
			}
			nrf_saadc_event_clear(NRF_SAADC_EVENT_STOPPED);

			//printf(" \r\nADC result = 0x%08x %d (irq=)",result, result);
			gc_saadc = result;
			//printf("\r\n ADC result = %d ", gc_saadc);

			g_battery = get_battery_level(gc_saadc);

			if(g_battery == 0)
			{
					g_battery = pre_battery;
			}
			else
			{
					pre_battery  = g_battery;
			}
			//SEGGER_RTT_printf(0,"\r\n battery : %d(percent)", g_battery);
			gc_adc_battery_flag = GC_ADC_BATTERY_FLAG_FINISH;
			
			
			//printf("\r\n nrf_saadc_enable_check? %d",nrf_saadc_enable_check());
			nrf_saadc_disable();
			//printf("\r\n nrf_saadc_enable_check? %d",nrf_saadc_enable_check());
			
}

void saadc_init(void)
{
    /* NRF SAADC 콜백 사용안함 직접 읽도록 변경
	  ret_code_t err_code;
  	nrf_drv_saadc_config_t saadc_config = NRF_DRV_SAADC_DEFAULT_CONFIG; 
    //channel_config= NRF_DRV_SAADC_DEFAULT_CHANNEL_CONFIG_SE(NRF_SAADC_INPUT_AIN0);
	
	 saadc_config.interrupt_priority = APP_IRQ_PRIORITY_LOWEST;
	 nrf_saadc_channel_config_t channel_config =
    NRF_DRV_SAADC_DEFAULT_CHANNEL_CONFIG_SE(NRF_SAADC_INPUT_AIN0);
	
    err_code = nrf_drv_saadc_init(&saadc_config, saadc_callback);
	  //err_code = nrf_drv_saadc_init(NULL, saadc_callback);
		APP_ERROR_CHECK(err_code);

    err_code = nrf_drv_saadc_channel_init(0, &channel_config);
    APP_ERROR_CHECK(err_code);
		
	

    err_code = nrf_drv_saadc_buffer_convert(m_buffer_pool[0],SAMPLES_IN_BUFFER);
    APP_ERROR_CHECK(err_code);
    
    err_code = nrf_drv_saadc_buffer_convert(m_buffer_pool[1],SAMPLES_IN_BUFFER);
    APP_ERROR_CHECK(err_code);*/
		
		/* default init for SAADC 
		 config.acq_time = NRF_SAADC_ACQTIME_20US;
    config.gain = NRF_SAADC_GAIN1_4;
    config.mode = NRF_SAADC_MODE_SINGLE_ENDED;
    config.pin_p = NRF_SAADC_INPUT_AIN0;
    config.pin_n = NRF_SAADC_INPUT_AIN0; // Single ended -> should be ground to zero
    config.reference = NRF_SAADC_REFERENCE_VDD4;
    config.resistor_p = NRF_SAADC_RESISTOR_DISABLED;
    config.resistor_n = NRF_SAADC_RESISTOR_DISABLED;
		*/
		
		//규창  adc 기존 Default SE방식과 똑같이 초기화
		nrf_saadc_channel_config_t config;
    config.acq_time = NRF_SAADC_ACQTIME_10US;
    config.gain = NRF_SAADC_GAIN1_6;
    config.mode = NRF_SAADC_MODE_SINGLE_ENDED;
    config.pin_p = NRF_SAADC_INPUT_AIN0;
    config.pin_n = NRF_SAADC_INPUT_DISABLED; // Single ended -> should be ground to zero
    config.reference = NRF_SAADC_REFERENCE_INTERNAL;
    config.resistor_p = NRF_SAADC_RESISTOR_DISABLED;
    config.resistor_n = NRF_SAADC_RESISTOR_DISABLED;
		
    nrf_saadc_enable();
    nrf_saadc_channel_init(NRF_SAADC_INPUT_AIN0, &config);
    nrf_saadc_buffer_init(&result,1); // One sample
		
		
		gc_adc_battery_flag = GC_ADC_BATTERY_FLAG_START;
			
			nrf_saadc_task_trigger(NRF_SAADC_TASK_START);

			while(!nrf_saadc_event_check(NRF_SAADC_EVENT_STARTED)){
						//printf(" NRF_SAADC_EVENT_STARTED");
			}
			nrf_saadc_event_clear(NRF_SAADC_EVENT_STARTED);

			nrf_saadc_task_trigger(NRF_SAADC_TASK_SAMPLE);


			while(!nrf_saadc_event_check(NRF_SAADC_EVENT_END)){
						//printf(" NRF_SAADC_EVENT_END");
			}

			nrf_saadc_event_clear(NRF_SAADC_EVENT_END);

			nrf_saadc_task_trigger(NRF_SAADC_TASK_STOP);
			while(!nrf_saadc_event_check(NRF_SAADC_EVENT_STOPPED)){
						//printf(" NRF_SAADC_EVENT_STOPPED");
			}
			nrf_saadc_event_clear(NRF_SAADC_EVENT_STOPPED);

			//printf(" \r\nADC result = 0x%08x %d (irq=)",result, result);
			gc_saadc = result;
			//printf("\r\n ADC result = %d ", gc_saadc);

			g_battery = get_battery_level(gc_saadc);

			if(g_battery == 0)
			{
					g_battery = pre_battery;
			}
			else
			{
					pre_battery  = g_battery;
			}
			//SEGGER_RTT_printf(0,"\r\n battery : %d(percent)", g_battery);
			gc_adc_battery_flag = GC_ADC_BATTERY_FLAG_FINISH;
			
			printf("\r\n nrf_saadc_enable_check? %d",nrf_saadc_enable_check());
			nrf_saadc_disable();
			printf("\r\n nrf_saadc_enable_check? %d",nrf_saadc_enable_check());
			
			
	  
}


//규창  battery sense도 이제 안씀 직접 타이머 열어서 읽음
/*
void gc_battery_sense(void)
{
		#ifdef _NRF52_HW_1_2_ADC_SPI_WORKAROUND_
		gc_adc_battery_flag = GC_ADC_BATTERY_FLAG_START;
		#endif // _NRF52_HW_1_2_ADC_SPI_WORKAROUND_ 

		// To low power, uninit() after init() 
		//ret_code_t err_code;
		//saadc_init();
		//nrf_saadc_task_trigger(NRF_SAADC_TASK_START);
		//NVIC_SetPendingIRQ(SAADC_IRQn);
		//printf("\r\n call BAT!");
		//err_code = nrf_drv_saadc_sample(); // Check error
		// APP_ERROR_CHECK(err_code);


		//err_code =  nrf_drv_saadc_sample(); //If disabled. system reboot
		//APP_ERROR_CHECK(err_code);
		//gc_disable_saadc();
}*/

/** @file
 * @brief battery sensing 
 *
 */
void gpiote_battery_sense_evt_handler(nrf_drv_gpiote_pin_t pin, nrf_gpiote_polarity_t action)
{
	if(pin == GC_GPIO_BAT_CHARGE_DETECT  && action == NRF_GPIOTE_POLARITY_LOTOHI )
  {
		nrf_gpio_pin_set(GC_GPIO_DEBUG_ON);
		app_uart_flush();
	}
}

void init_battery_sense_event(void)
{
	nrf_gpio_cfg_input(GC_GPIO_BAT_CHARGE_DETECT, NRF_GPIO_PIN_NOPULL); 
	
	nrf_drv_gpiote_in_config_t in_config = GPIOTE_CONFIG_IN_SENSE_LOTOHI(true);
	//in_config.pull = NRF_GPIO_PIN_PULLUP;

	if ( nrf_drv_gpiote_in_init(GC_GPIO_BAT_CHARGE_DETECT, &in_config, gpiote_battery_sense_evt_handler) != NRF_SUCCESS )
		printf("\r\n GPIOTE input init. fail");
	
	nrf_drv_gpiote_in_event_enable(GC_GPIO_BAT_CHARGE_DETECT, true);
}
