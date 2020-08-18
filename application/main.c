/* Copyright (c) 2014 Nordic Semiconductor. All Rights Reserved.
 *
 * The information contained herein is property of Nordic Semiconductor ASA.
 * Terms and conditions of usage are described in detail in NORDIC
 * SEMICONDUCTOR STANDARD SOFTWARE LICENSE AGREEMENT.
 *
 * Licensees are granted free, non-transferable use of the information. NO
 * WARRANTY of ANY KIND is provided. This heading must NOT be removed from
 * the file.
 *
 */

/** @file
 *
 * @defgroup ble_sdk_uart_over_ble_main main.c
 * @{
 * @ingroup  ble_sdk_app_nus_eval
 * @brief    UART over BLE application main file.
 *
 * This file contains the source code for a sample application that uses the Nordic UART service.
 * This application uses the @ref srvlib_conn_params module.
 */
//#define printf(...)
#include <stdint.h>
#include <string.h>
#include "nordic_common.h"
#include "nrf.h"
#include "nrf_delay.h"
#include "app_scheduler.h"
#include "app_timer_appsh.h"
#include "ble_hci.h"
#include "ble_advdata.h"
#include "ble_advertising.h"
#include "ble_conn_params.h"
#include "softdevice_handler.h"
#include "app_timer.h"
#include "app_button.h"
#include "ble_nus.h"
#include "app_uart.h"
#include "app_util_platform.h"
#include "bsp.h"
#include "bsp_btn_ble.h"
#include "app_timer.h"
#include "device_manager.h"
#include "pstorage.h"
#include "gc_saadc.h"
#include "ble_ancs_c.h"
#include "ble_db_discovery.h"
#ifdef GC_NRF52_DEV
	#include "nrf_drv_config.h"
	#include "define.h"
	#include "oled.h"
	#include "nrf_gpio.h"
	#include "spi.h"
	#include "afe4400.h"
	#include "rtc.h"
	#include "nrf_drv_twi.h"
	#include "nrf_drv_spi.h"
	#include "bma2x2.h"
	#include "spi_flash.h"
	#include "an520.h"
	#include "pah8002.h"
	#include "spi_pah8002.h"
	#include "twi_pah8002.h"
	#include "gc_twi.h"
	#include "nrf_drv_ppi.h"
	#include "bma2x2.h"
	#include "flash_control.h"
	#include "gc_gpio.h"
	#include "nrf_drv_gpiote.h"
	#include "gc_sleep.h"
	#include "nrf_drv_saadc.h"
	#include "pah8002.h"
	#include "gc_ble_ancs.h"
#endif /* GC_NRF52_DEV */
#ifdef GREENCOMM_NUS /* app_shell hkim 2016.6.2 */
#include "command.h" 
#include "print.h"
#endif /* GREENCOMM_NUS */

#include "gc_util.h"
#include "gc_capsense.h"
#include "SEGGER_RTT.h"

/*moon*/
#include "gc_activity.h" 
#include "protocol.h"
#include "display.h"
#include "font.h"
#include "app_util.h"

#ifdef BLE_DFU_APP_SUPPORT
#include "ble_dfu.h"
#include "dfu_app_handler.h"
#endif /* BLE_DFU_APP_SUPPORT */

#define GC_NRF52_ANCS

#ifndef BLE_DFU_APP_SUPPORT
	#define IS_SRVC_CHANGED_CHARACT_PRESENT 0                                           /**< Include the service_changed characteristic. If not enabled, the server's database cannot be changed for the lifetime of the device. */
#else
	#define IS_SRVC_CHANGED_CHARACT_PRESENT  1                                          /**< Include or not the service_changed characteristic. if not enabled, the server's database cannot be changed for the lifetime of the device*/
#endif /* BLE_DFU_APP_SUPPORT */

#define CENTRAL_LINK_COUNT              0                                           /**< Number of central links used by the application. When changing this number remember to adjust the RAM settings*/
#define PERIPHERAL_LINK_COUNT           1                                           /**< Number of peripheral links used by the application. When changing this number remember to adjust the RAM settings*/

#ifdef GREENCOMM_BT /* hkim 2016.6.9 If UUID is included in advertising, no enough space for device name */
  #ifdef GC_COACH_FIT_PCB
  #define DEVICE_NAME                     "GC"//"DK52_PCB"                              /**< Name of device. Will be included in the advertising data. */
  #elif defined GC_COACH_FIT_DK52
	  #define DEVICE_NAME                     "DK52_NUS"                               /**< Name of device. Will be included in the advertising data. */
	#else
	#endif
#else /* GREENCOMM_BT */
	#define DEVICE_NAME                     "Nordic_UART" 
#endif  /* GREENCOMM_BT */
#define NUS_SERVICE_UUID_TYPE           BLE_UUID_TYPE_VENDOR_BEGIN                  /**< UUID type for the Nordic UART Service (vendor specific). */

#ifdef GC_NRF52_BLE
	#define APP_ADV_INTERVAL                32                                       /**< The advertising interval (in units of 0.625 ms. This value corresponds to 20 ms). */ /* v1.5.13.0 hkim 2016.4.14 iOS BLE background scan(초기값 64)*/
#else
	#define APP_ADV_INTERVAL                64                                          /**< The advertising interval (in units of 0.625 ms. This value corresponds to 40 ms). */
#endif /* GC_NRF52_BLE */

#define APP_ADV_TIMEOUT_IN_SECONDS      0                                         /**< The advertising timeout (in units of seconds). */

#define APP_TIMER_PRESCALER             15                                           /**< Value of the RTC1 PRESCALER register. 15*/
#define APP_TIMER_OP_QUEUE_SIZE         4                                           /**< Size of timer operation queues. 3*/

//#ifdef GC_NRF52_ANCS
#if 0 
#define MIN_CONN_INTERVAL               MSEC_TO_UNITS(500, UNIT_1_25_MS)             /**< Minimum acceptable connection interval (20 ms), Connection interval uses 1.25 ms units. */
#define MAX_CONN_INTERVAL               MSEC_TO_UNITS(1000, UNIT_1_25_MS)             /**< Maximum acceptable connection interval (75 ms), Connection interval uses 1.25 ms units. */
#else /* GC_NRF52_ANCS */
#define MIN_CONN_INTERVAL               MSEC_TO_UNITS(20, UNIT_1_25_MS)            /**< Minimum acceptable connection interval (0.5 seconds). */
#define MAX_CONN_INTERVAL               MSEC_TO_UNITS(75, UNIT_1_25_MS)           /**< Maximum acceptable connection interval (1 second). */
#endif /* GC_NRF52_ANCS */

#define SLAVE_LATENCY                   0                                           /**< Slave latency. */
#define CONN_SUP_TIMEOUT                MSEC_TO_UNITS(4000, UNIT_10_MS)             /**< Connection supervisory timeout (4 seconds), Supervision Timeout uses 10 ms units. */
#define FIRST_CONN_PARAMS_UPDATE_DELAY  APP_TIMER_TICKS(5000, APP_TIMER_PRESCALER)  /**< Time from initiating event (connect or start of notification) to first time sd_ble_gap_conn_param_update is called (5 seconds). */
#define NEXT_CONN_PARAMS_UPDATE_DELAY   APP_TIMER_TICKS(30000, APP_TIMER_PRESCALER) /**< Time between each call to sd_ble_gap_conn_param_update after the first call (30 seconds). */
#define MAX_CONN_PARAMS_UPDATE_COUNT    3                                           /**< Number of attempts before giving up the connection parameter negotiation. */

#define DEAD_BEEF                       0xDEADBEEF                                  /**< Value used as error code on stack dump, can be used to identify stack location on stack unwind. */
#define SEC_PARAM_BOND                   1                                          /**< Perform bonding. */
#define SEC_PARAM_MITM                   0                                          /**< Man In The Middle protection not required. */
#define SEC_PARAM_LESC                   0                                          /**< LE Secure Connections not enabled. */
#define SEC_PARAM_KEYPRESS               0                                          /**< Keypress notifications not enabled. */
#define SEC_PARAM_IO_CAPABILITIES        BLE_GAP_IO_CAPS_NONE                       /**< No I/O capabilities. */
#define SEC_PARAM_OOB                    0                                          /**< Out Of Band data not available. */
#define SEC_PARAM_MIN_KEY_SIZE           7                                          /**< Minimum encryption key size. */
#define SEC_PARAM_MAX_KEY_SIZE           16                                         /**< Maximum encryption key size. */

#ifdef BLE_DFU_APP_SUPPORT
#define DFU_REV_MAJOR                    0x00                                       /** DFU Major revision number to be exposed. */
#define DFU_REV_MINOR                    0x01                                       /** DFU Minor revision number to be exposed. */
#define DFU_REVISION                     ((DFU_REV_MAJOR << 8) | DFU_REV_MINOR)     /** DFU Revision number to be exposed. Combined of major and minor versions. */
#define APP_SERVICE_HANDLE_START         0x000C                                     /**< Handle of first application specific service when when service changed characteristic is present. */
#define BLE_HANDLE_MAX                   0xFFFF                                     /**< Max handle value in BLE. */

STATIC_ASSERT(IS_SRVC_CHANGED_CHARACT_PRESENT);                                     /** When having DFU Service support in application the Service Changed Characteristic should always be present. */
#endif // BLE_DFU_APP_SUPPORT

#ifdef GC_NRF52_UART 
	#define UART_TX_BUF_SIZE                1024                                         /**< UART TX buffer size. */
	#define UART_RX_BUF_SIZE               	1024                                        /**< UART RX buffer size. */
#elif defined GC_NRF51_UART
	#define UART_TX_BUF_SIZE                256                                         /**< UART TX buffer size. */
	#define UART_RX_BUF_SIZE                256                                         /**< UART RX buffer size. */
#else
#endif 

static ble_nus_t                        m_nus;                                       /**< Structure to identify the Nordic UART Service. */
static ble_db_discovery_t        m_ble_db_discovery;                       /**< Structure used to identify the DB Discovery module. */
static dm_handle_t               m_peer_handle;                            /**< Identifies the peer that is currently connected. */

static uint16_t                         m_conn_handle = BLE_CONN_HANDLE_INVALID;    /**< Handle of the current connection. */

static dm_application_instance_t         m_app_handle;                              /**< Application identifier allocated by device manager */

#ifndef GC_NRF52_ANCS
static ble_uuid_t                       m_adv_uuids[] = {{BLE_UUID_NUS_SERVICE, NUS_SERVICE_UUID_TYPE}, };  /**< Universally unique service identifier. */
#endif 
#ifdef BLE_DFU_APP_SUPPORT
static dm_application_instance_t         m_app_handle;                              /**< Application identifier allocated by device manager */
static ble_dfu_t                         m_dfus;                                    /**< Structure used to identify the DFU service. */
#endif // BLE_DFU_APP_SUPPORT

#ifdef GREENCOMM_NUS /* app_timer hkim 2016.6.1 */
#define		BOOT_MARK_ADDRESS	0x20003ff0
#define		BOOT_MARK				((unsigned int)'HKIM')
#define 		BOOTLOADER_DFU_START 0xB1
	
#define SENSOR_TIMER_INTERVAL		APP_TIMER_TICKS(50, APP_TIMER_PRESCALER)
APP_TIMER_DEF(m_shell_timer_id);
//static app_timer_id_t                        m_feature_timer_id;

//규창 배터리 읽는 타이머 주기는 10분마다 한번씩
#define BATTERY_LEVEL_MEAS_INTERVAL         APP_TIMER_TICKS(600000, APP_TIMER_PRESCALER) /**< Battery level measurement interval (ticks). This value corresponds to 600 seconds. */
APP_TIMER_DEF(m_battery_timer_id);   


//규창 가속센서 데이터 심박센서로 넣기
//#define ACC_FOR_INTERVAL         APP_TIMER_TICKS(33, APP_TIMER_PRESCALER) /**< Battery level measurement interval (ticks). This value corresponds to 600 seconds. */
//APP_TIMER_DEF(m_accppg_timer_id);   


#endif 


#define SECURITY_REQUEST_DELAY          APP_TIMER_TICKS(1500, APP_TIMER_PRESCALER)  /**< Delay after connection until security request is sent, if necessary (ticks). */
#define PAHR_TIMER_INTERVAL		APP_TIMER_TICKS(1000, APP_TIMER_PRESCALER)
APP_TIMER_DEF(m_sec_req_timer_id);                                         /**< Security request timer. The timer lets us start pairing request if one does not arrive from the Central. */
//APP_TIMER_DEF(m_pahr_timer_id); 
void SysTick_Handler(void);
void accforppg_handler(void * p_context);


#ifdef GC_NRF52_TWI
// [max 255, otherwise "int16_t" won't be sufficient to hold the sum
//  of accelerometer samples]
#define NUMBER_OF_SAMPLES 20

/**
 * @brief Union to keep raw and converted data from accelerometer samples at one memory space.
 */
typedef union{
    uint8_t raw;
    int8_t  conv;
}elem_t;
/**
 * @brief Structure for holding samples from accelerometer.
 */
typedef struct
{
    elem_t  x;
    elem_t  y;
    elem_t  z;
    uint8_t tilt;
} sample_t;
/* Indicates if reading operation from accelerometer has ended. */
static volatile bool m_xfer_done = true;
/* Indicates if setting mode operation has ended. */
static volatile bool m_set_mode_done = false;
/* TWI instance. */
const nrf_drv_twi_t m_twi_master_1 = NRF_DRV_TWI_INSTANCE(0);
#endif /* GC_NRF52_TWI */


/*moon*/
volatile int is_uart = 1;
volatile int is_recall = 0;
volatile int is_recall_act = 0;
#define WATION_MOTION_TRIGGER_TIME  25        // 0.5CJ 
#define WATION_MOTION_DISPLAY_TIME  (50*5)    // 5CJ 

//규창 디스플레이 스윙온
//int Detect_Watch_Motion(int acc_x, int acc_y, int acc_z);
 extern volatile unsigned char day_of_week;
//tTime slp_start_time;
tTime slp_end_time;
tTime reset_end_time;
extern volatile struct ACTIVITY_INFO act_info;
extern volatile struct calorie_save kcal;
extern volatile struct SLEEP_INFO slp_info;
extern volatile struct s_hart_rate HR_info;
extern volatile unsigned char hr_dp;
extern volatile unsigned char noti_num;
extern volatile unsigned char is_info_recall;
extern volatile uint8_t _pah8002_interrupt;
unsigned int n_prom[8];
extern volatile unsigned int hr_test_count;

volatile unsigned char go_sleep = 0;
volatile unsigned char tuch_count =0;
extern tTime stdTime;
extern volatile uint8_t pahr_mode;
extern volatile unsigned int g_total_daystep;
extern volatile unsigned char op_mode;
volatile char g_product_mode=0;
extern volatile unsigned short OLED_power_count;
extern volatile unsigned char OLED_switch;

extern volatile uint8_t g_battery;
extern volatile int16_t gc_saadc;
int adc;
extern unsigned char g_hr;
unsigned char pre_g_hr;
extern unsigned char g_sdnn_hr;

extern volatile short gmt_offset;

char BLE_SSID[16] = "";
volatile unsigned char is_charging = 0;
volatile uint8_t g_acc_move_signal=0;
int stress_timmer;
int ble_100ms_count=0;
char send_rawdata=2;
char send_raw_bf[20];
extern volatile struct PERSONAL_INFO person_info;
extern volatile struct s_hart_rate HR_info;
extern volatile struct STEP_INFO step;
extern tTime time;
float avg_2sec_hr=0;
float avg_2sec_count=0;
float f_hr=0;
uint8_t hr_power_en;
extern volatile uint8_t hr_processing;
volatile uint8_t gc_conn_os; /* hkim 2016.11.21 */
extern volatile char BT_vib;
void check_saved_info(void);
extern int g_total_tx_busy_count;

////////////////////////////////////////////////////////
//규창 gc_activity에서 최고 최저 평균 심박 산출 새로 작성키 위한 변수
unsigned int epoch_hr=0;
//규창 안정심박 임시 70
//unsigned char HR_max=0,HR_avg,HR_min=0xFF,HR_sta=0;
//unsigned char HR_sta=0;
unsigned int HR_sum=0;
//규창 최저심박 센서 이상 상태일 때 갱신용
//pah8002.c에서 끌어옴
extern volatile bool reset_minHR;


//////////////////////////////////////////////////////////

#ifdef GC_NRF52_ANCS
static ble_ancs_c_t		m_ancs_c;
#define VENDOR_SPECIFIC_UUID_COUNT      4 
#define SCHED_MAX_EVENT_DATA_SIZE       MAX(APP_TIMER_SCHED_EVT_SIZE, \
                                            BLE_STACK_HANDLER_SCHED_EVT_SIZE) /**< Maximum size of scheduler events. */
#define SCHED_QUEUE_SIZE                10                                    /**< Maximum number of events in the scheduler queue. */
static ble_gap_sec_params_t      m_sec_param;                              /**< Security parameter for use in security requests. */
/**@brief String literals for the iOS notification categories. used then printing to UART. */
const char * lit_catid[BLE_ANCS_NB_OF_CATEGORY_ID] =
{
    "Other",
    "Incoming Call",
    "Missed Call",
    "Voice Mail",
    "Social",
    "Schedule",
    "Email",
    "News",
    "Health And Fitness",
    "Business And Finance",
    "Location",
    "Entertainment"
};

/**@brief String literals for the iOS notification event types. used then printing to UART. */
const char * lit_eventid[BLE_ANCS_NB_OF_EVT_ID] =
{
    "Added",
    "Modified",
    "Removed"
};

/**@brief String literals for the iOS notification attribute types. used then printing to UART. */
const char * lit_attrid[BLE_ANCS_NB_OF_ATTRS] =
{
    "App Identifier",
    "Title",
    "Subtitle",
    "Message",
    "Message Size",
    "Date",
    "Positive Action Label",
    "Negative Action Label"
};

static ble_ancs_c_evt_notif_t m_notification_latest;                       /**< Local copy to keep track of the newest arriving notifications. */

#define APP_ADV_FAST_INTERVAL           40        /**< The advertising interval (in units of 0.625 ms). The default value corresponds to 25 ms. */
#define APP_ADV_SLOW_INTERVAL           3200   /**< Slow advertising interval (in units of 0.625 ms). The default value corresponds to 2 seconds. */
#define APP_ADV_FAST_TIMEOUT            180      /**< The advertising time-out in units of seconds. */
#define APP_ADV_SLOW_TIMEOUT            180     /**< The advertising time-out in units of seconds. */
#define ADV_INTERVAL_FAST_PERIOD        30      /**< The duration of the fast advertising period (in seconds). */
static ble_ancs_c_t              m_ancs_c;                                 /**< Structure used to identify the Apple Notification Service Client. */
static dm_handle_t               m_peer_handle;                            /**< Identifies the peer that is currently connected. */
#endif /* GC_NRF52_ANCS */


/**@brief Function for assert macro callback.
 *
 * @details This function will be called in case of an assert in the SoftDevice.
 *
 * @warning This handler is an example only and does not fit a final product. You need to analyse 
 *          how your product is supposed to react in case of Assert.
 * @warning On assert from the SoftDevice, the system can only recover on reset.
 *
 * @param[in] line_num    Line number of the failing ASSERT call.
 * @param[in] p_file_name File name of the failing ASSERT call.
 */
void assert_nrf_callback(uint16_t line_num, const uint8_t * p_file_name)
{
	#if 0 /* hkim 2016.11.5 */ /* FIXME */
    APP_ERROR_HANDLER(nrf_error);	
	#endif
}


/**@brief Function for the GAP initialization.
 *
 * @details This function will set up all the necessary GAP (Generic Access Profile) parameters of 
 *          the device. It also sets the permissions and appearance.
 */
static void gap_params_init(void)
{
    uint32_t                err_code;
		//ble_gap_addr_t gap_address;
    ble_gap_conn_params_t   gap_conn_params;
    ble_gap_conn_sec_mode_t sec_mode;

    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&sec_mode);
    
    err_code = sd_ble_gap_device_name_set(&sec_mode,
                                          (const uint8_t *) BLE_SSID,
                                          strlen(BLE_SSID));
    APP_ERROR_CHECK(err_code);

    memset(&gap_conn_params, 0, sizeof(gap_conn_params));

    gap_conn_params.min_conn_interval = MIN_CONN_INTERVAL;
    gap_conn_params.max_conn_interval = MAX_CONN_INTERVAL;
    gap_conn_params.slave_latency     = SLAVE_LATENCY;
    gap_conn_params.conn_sup_timeout  = CONN_SUP_TIMEOUT;

    err_code = sd_ble_gap_ppcp_set(&gap_conn_params);
    APP_ERROR_CHECK(err_code);
}


/**@brief Function for handling the data from the Nordic UART Service.
 *
 * @details This function will process the data received from the Nordic UART BLE Service and send
 *          it to the UART module.
 *
 * @param[in] p_nus    Nordic UART Service structure.
 * @param[in] p_data   Data to be send to UART module.
 * @param[in] length   Length of the data.
 */
/**@snippet [Handling the data received over BLE] */
static void nus_data_handler(ble_nus_t * p_nus, uint8_t * p_data, uint16_t length)
{
    for (uint32_t i = 0; i < length; i++)
    {
        while(app_uart_put(p_data[i]) != NRF_SUCCESS);
    }
    while(app_uart_put('\n') != NRF_SUCCESS);

#ifdef GREENCOMM_NUS		
		if( p_data[0]=='r' && p_data[1]=='e' && p_data[2]=='s'  && p_data[3]=='e' && p_data[4]=='t' )
		{
				printf("\r\nreset command from BLE APP");

				if( p_data[6]=='b' && p_data[7]=='o' && p_data[8]=='o' && p_data[9]=='t' )
				{
					/*int save_time = (get_GMT(time.year,  time.month,  time.day,  time.hour,  time.minute, time.second )/(86400))*1440;
					int save_step=0, save_cal=0;
					int i;
					
					uint8_t req_data[GC_FLASH_PAGE_DATA_SIZE]; // 4: address 
					_GC_TBL_STEP				_tbl_step;
					_GC_TBL_CALORIE	_tbl_kcal;
					
					// initialize structure //
					memset(&_tbl_step, 0, sizeof(_GC_TBL_STEP));
					memset(&_tbl_kcal, 0, sizeof(_GC_TBL_CALORIE));
					
					memset(req_data, 0xff, sizeof(req_data));
					
					_tbl_step.date	= save_time;
					_tbl_step.step	= step.display_step;
					memcpy(req_data, &_tbl_step, sizeof(_GC_TBL_STEP));
					
					for(i=0;i<=5;i++)
					{
						save_step = gc_flash_page_write_data(GC_FLASH_TBL_TYPE_BAT_STEP, req_data);
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
						save_cal = gc_flash_page_write_data(GC_FLASH_TBL_TYPE_BAT_KCAL, req_data);
						nrf_delay_ms(10); // FIXME 
						
						if(save_cal)
						{
							i=6;
						}
					}*/
					
					
					
					save_data_before_reset(); 
					nrf_delay_ms(100);
					
					
					
						#if 0 /* hkim 2016.8.27 */
						*(unsigned int *)BOOT_MARK_ADDRESS = BOOT_MARK;
						#else
						NRF_POWER->GPREGRET = BOOTLOADER_DFU_START;
						#endif
				}
				//save_data_before_reset(); /*16.12.20 for save_before_reset hm_lee*/
				//nrf_delay_ms(10); /* FIXME */
				//save_info_before_reset_point(3,0,0,0,0);	/*16.12.14 for test_log hm_lee*/
				NVIC_SystemReset();    
				while(1);
		}
		if( p_data[0]=='v' && p_data[1]=='e' && p_data[2]=='r'  )
		{
			char s[20];
			printf("%s",PRODUCT_VERSION);
			sprintf(s,"%s",PRODUCT_VERSION);
			printf("\r\n%s",s);
			ble_send( (uint8_t*)s,20);
		}
	#if 0 /* hkim-1012 */	
		if( p_data[0]=='o' && p_data[1]=='l' && p_data[2]=='e' && p_data[3]=='d') /* OLED */
		{
			if( p_data[5]=='o' && p_data[6]=='n')
			{
				printf("\r\n OLED on");
				OLED_Write_Command(0xAF);
			}
			if( p_data[5]=='o' && p_data[6]=='f' && p_data[7]=='f')
			{
				printf("\r\n OLED off");
				OLED_Write_Command(0xAE);
			}
		}
		
		if( p_data[0]=='h' && p_data[1]=='r') /* hr */
		{
			if( p_data[3]=='o' && p_data[4]=='n')
			{
				printf("\r\n cmd : hr on");
				pah8002_init();
				pah8002_enter_normal_mode();
			}
			if( p_data[3]=='o' && p_data[4]=='f' && p_data[5]=='f')
			{
				printf("\r\n cmd : hr off");
				pah8002_deinit();
			}
		}
		
		if( p_data[0]=='f' && p_data[1]=='l' && p_data[2]=='a' && p_data[3]=='s' && p_data[4]=='h') /* hr */
		{
			unsigned char tx_data[4],rx_data[4];
			
			if( p_data[6]=='o' && p_data[7]=='n')
			{
				printf("\r\n flash on");
				tx_data[0]=W25X_ReleasePowerDown;
			}
			if( p_data[6]=='o' && p_data[7]=='f' && p_data[8]=='f')
			{
				printf("\r\n flash off");
				tx_data[0]=W25X_PowerDown;
			}
			tx_data[1]=0;
			tx_data[2]=0;
			tx_data[3]=0;
	
			spi_send_recv(tx_data, rx_data, 1);
		}
		
		if( p_data[0]=='a' && p_data[1]=='c' && p_data[2]=='c') 
		{
			if( p_data[4]=='o' && p_data[5]=='n')
			{
				printf("\r\n acc on");
				bma2x2_set_power_mode(BMA2x2_MODE_NORMAL);
			}
			if( p_data[4]=='o' && p_data[5]=='f' && p_data[6]=='f')
			{
				printf("\r\n acc off");
				bma2x2_set_power_mode(BMA2x2_MODE_DEEP_SUSPEND);
			}
		}
		
		if( p_data[0]=='m' && p_data[1]=='c' && p_data[2]=='u') 
		{
			if( p_data[4]=='o' && p_data[5]=='f' && p_data[6]=='f')
			{
				printf("\r\n mcu off");
				gc_power_off_prepare();
				sd_power_system_off();
			}
		}
		
		if( p_data[0]=='p' && p_data[1]=='o' && p_data[2]=='w' && p_data[3]=='e' && p_data[4]=='r'  && p_data[5]=='o' && p_data[6]=='f' && p_data[7]=='f')
		{
			if( p_data[9]=='a' && p_data[10]=='l' && p_data[11]=='l')
			{
				unsigned char tx_data[4],rx_data[4];
				
				printf("\r\n poweroff all");
				memset(tx_data, 0, sizeof(tx_data));
				
				tx_data[0]=W25X_PowerDown;
				
				OLED_Write_Command(0xAE); 
				pah8002_deinit();
				spi_send_recv(tx_data, rx_data, 1);
				
				gc_power_off_prepare();
				sd_power_system_off();
			}
		}
		#endif /* if 0 hkim-1012 */
		printf("\r\n ble_receive_handler[%x]",p_data[0]);
		ble_receive_handler(p_data,length);
#endif /* GREENCOMM_NUS */
}
/**@snippet [Handling the data received over BLE] */

#ifdef BLE_DFU_APP_SUPPORT
/**@brief Function for loading application-specific context after establishing a secure connection.
 *
 * @details This function will load the application context and check if the ATT table is marked as
 *          changed. If the ATT table is marked as changed, a Service Changed Indication
 *          is sent to the peer if the Service Changed CCCD is set to indicate.
 *
 * @param[in] p_handle The Device Manager handle that identifies the connection for which the context
 *                     should be loaded.
 */
static void app_context_load(dm_handle_t const * p_handle)
{
    uint32_t                 err_code;
    static uint32_t          context_data;
    dm_application_context_t context;

    context.len    = sizeof(context_data);
    context.p_data = (uint8_t *)&context_data;

    err_code = dm_application_context_get(p_handle, &context);
    if (err_code == NRF_SUCCESS)
    {
        // Send Service Changed Indication if ATT table has changed.
        if ((context_data & (DFU_APP_ATT_TABLE_CHANGED << DFU_APP_ATT_TABLE_POS)) != 0)
        {
            err_code = sd_ble_gatts_service_changed(m_conn_handle, APP_SERVICE_HANDLE_START, BLE_HANDLE_MAX);
            if ((err_code != NRF_SUCCESS) &&
                (err_code != BLE_ERROR_INVALID_CONN_HANDLE) &&
                (err_code != NRF_ERROR_INVALID_STATE) &&
                (err_code != BLE_ERROR_NO_TX_PACKETS) &&
                (err_code != NRF_ERROR_BUSY) &&
                (err_code != BLE_ERROR_GATTS_SYS_ATTR_MISSING))
            {
                APP_ERROR_HANDLER(err_code);
            }
        }

        err_code = dm_application_context_delete(p_handle);
        APP_ERROR_CHECK(err_code);
    }
    else if (err_code == DM_NO_APP_CONTEXT)
    {
        // No context available. Ignore.
    }
    else
    {
        APP_ERROR_HANDLER(err_code);
    }
}

/** @snippet [DFU BLE Reset prepare] */
#endif /* BLE_DFU_APP_SUPPORT */

/**@brief Function for stopping advertising.
 */
static void advertising_stop(void)
{
    uint32_t err_code;

    err_code = sd_ble_gap_adv_stop();
    APP_ERROR_CHECK(err_code);

    err_code = bsp_indication_set(BSP_INDICATE_IDLE);
    APP_ERROR_CHECK(err_code);
}

/** @snippet [DFU BLE Reset prepare] */
/**@brief Function for preparing for system reset.
 *
 * @details This function implements @ref dfu_app_reset_prepare_t. It will be called by
 *          @ref dfu_app_handler.c before entering the bootloader/DFU.
 *          This allows the current running application to shut down gracefully.
 */
void reset_prepare(void)
{
    uint32_t err_code;

    if (m_conn_handle != BLE_CONN_HANDLE_INVALID)
    {
        // Disconnect from peer.
        err_code = sd_ble_gap_disconnect(m_conn_handle, BLE_HCI_REMOTE_USER_TERMINATED_CONNECTION);
        APP_ERROR_CHECK(err_code);
        err_code = bsp_indication_set(BSP_INDICATE_IDLE);
        APP_ERROR_CHECK(err_code);
    }
    else
    {
        // If not connected, the device will be advertising. Hence stop the advertising.
        advertising_stop();
    }

    err_code = ble_conn_params_stop();
    APP_ERROR_CHECK(err_code);

    nrf_delay_ms(500);
}

#ifdef GC_NRF52_ANCS
/**@brief Function for initializing services that will be used by the application.
 */
#define ATTR_DATA_SIZE                  BLE_ANCS_ATTR_DATA_MAX                      /**< Allocated size for attribute data. */
static uint8_t m_attr_title[ATTR_DATA_SIZE];                               /**< Buffer to store attribute data. */
static uint8_t m_attr_subtitle[ATTR_DATA_SIZE];                            /**< Buffer to store attribute data. */
static uint8_t m_attr_message[ATTR_DATA_SIZE];                             /**< Buffer to store attribute data. */
static uint8_t m_attr_message_size[ATTR_DATA_SIZE];                        /**< Buffer to store attribute data. */
static uint8_t m_attr_date[ATTR_DATA_SIZE];                                /**< Buffer to store attribute data. */
static uint8_t m_attr_posaction[ATTR_DATA_SIZE];                           /**< Buffer to store attribute data. */
static uint8_t m_attr_negaction[ATTR_DATA_SIZE];                           /**< Buffer to store attribute data. */

/**@brief Function for printing an iOS notification.
 *
 * @param[in] p_notif  Pointer to the iOS notification.
 */
void notif_print(ble_ancs_c_evt_notif_t * p_notif)
{
    NRF_LOG("\r\nNotification\r\n");
    NRF_LOG_PRINTF("Event:       %s\r\n", lit_eventid[p_notif->evt_id]);
    NRF_LOG_PRINTF("Category ID: %s\r\n", lit_catid[p_notif->category_id]);
    NRF_LOG_PRINTF("Category Cnt:%u\r\n", (unsigned int) p_notif->category_count);
    NRF_LOG_PRINTF("UID:         %u\r\n", (unsigned int) p_notif->notif_uid);

    NRF_LOG("Flags:\r\n");
    if(p_notif->evt_flags.silent == 1)
    {
        NRF_LOG(" Silent\r\n");
    }
    if(p_notif->evt_flags.important == 1)
    {
        NRF_LOG(" Important\r\n");
    }
    if(p_notif->evt_flags.pre_existing == 1)
    {
        NRF_LOG(" Pre-existing\r\n");
    }
    if(p_notif->evt_flags.positive_action == 1)
    {
        NRF_LOG(" Positive Action\r\n");
    }
    if(p_notif->evt_flags.negative_action == 1)
    {
        NRF_LOG(" Positive Action\r\n");
    }
}


/**@brief Function for printing iOS notification attribute data.
 * 
 * @param[in] p_attr           Pointer to an iOS notification attribute.
 * @param[in] p_ancs_attr_list Pointer to a list of attributes. Each entry in the list stores 
                               a pointer to its attribute data, which is to be printed.
 */
void notif_attr_print(ble_ancs_c_evt_notif_attr_t * p_attr)
{
    if (p_attr->attr_len != 0)
    {
        NRF_LOG_PRINTF("%s: %s\r\n", lit_attrid[p_attr->attr_id], p_attr->p_attr_data);
    }
    else if (p_attr->attr_len == 0)
    {
        NRF_LOG_PRINTF("%s: (N/A)\r\n", lit_attrid[p_attr->attr_id]);
    }
}


/**@brief Function for setting up GATTC notifications from the Notification Provider.
 *
 * @details This function is called when a successful connection has been established.
 */
void apple_notification_setup(void)
{
    uint32_t err_code;

    nrf_delay_ms(100); // Delay because we cannot add a CCCD to close to starting encryption. iOS specific.

    err_code = ble_ancs_c_notif_source_notif_enable(&m_ancs_c);
    APP_ERROR_CHECK(err_code);

    err_code = ble_ancs_c_data_source_notif_enable(&m_ancs_c);
    APP_ERROR_CHECK(err_code);

    NRF_LOG("Notifications Enabled.\r\n");
}

/**@brief Function for handling the Apple Notification Service client.
 *
 * @details This function is called for all events in the Apple Notification client that
 *          are passed to the application.
 *
 * @param[in] p_evt  Event received from the Apple Notification Service client.
 */
void on_ancs_c_evt(ble_ancs_c_evt_t * p_evt)
{
	uint32_t err_code = NRF_SUCCESS;

	switch (p_evt->evt_type)
	{
			case BLE_ANCS_C_EVT_DISCOVERY_COMPLETE:
					NRF_LOG("Apple Notification Service discovered on the server.\r\n");
					err_code = ble_ancs_c_handles_assign(&m_ancs_c,p_evt->conn_handle, &p_evt->service);
					APP_ERROR_CHECK(err_code);
					apple_notification_setup();
					break;

			case BLE_ANCS_C_EVT_NOTIF:
					m_notification_latest = p_evt->notif;
					gc_notification_ancs_handle(&m_notification_latest);
					//notif_print(&m_notification_latest);
					break;

			case BLE_ANCS_C_EVT_NOTIF_ATTRIBUTE:
					//notif_attr_print(&p_evt->attr);
					break;

			case BLE_ANCS_C_EVT_DISCOVERY_FAILED:
					NRF_LOG("Apple Notification Service not discovered on the server.\r\n");
					break;

			default:
					// No implementation needed.
					break;
	}
}

/**@brief Function for handling the Apple Notification Service client errors.
 *
 * @param[in] nrf_error  Error code containing information about what went wrong.
 */
void apple_notification_error_handler(uint32_t nrf_error)
{
		printf("\r\n ancs error notification");
	#if 0 /* hkim 2016.11.5 */ /* FIXME */
    APP_ERROR_HANDLER(nrf_error);	
	#endif
}
#endif /* GC_NRF52_ANCS */

#ifdef GC_NRF52_ANCS 
static void services_init(void)
{
	ble_ancs_c_init_t ancs_init_obj;
	ble_nus_init_t nus_init;
	uint32_t          err_code;

	memset(&ancs_init_obj, 0, sizeof(ancs_init_obj));
	memset(&nus_init, 0, sizeof(nus_init));

	nus_init.data_handler = nus_data_handler;
	ble_nus_init(&m_nus, &nus_init);
	
	/* ANCS service */
	err_code = ble_ancs_c_attr_add(&m_ancs_c,
																 BLE_ANCS_NOTIF_ATTR_ID_TITLE,
																 m_attr_title,
																 ATTR_DATA_SIZE);
	APP_ERROR_CHECK(err_code);
	
	err_code = ble_ancs_c_attr_add(&m_ancs_c,
																 BLE_ANCS_NOTIF_ATTR_ID_SUBTITLE,
																 m_attr_subtitle,
																 ATTR_DATA_SIZE);
	APP_ERROR_CHECK(err_code);

	err_code = ble_ancs_c_attr_add(&m_ancs_c,
																 BLE_ANCS_NOTIF_ATTR_ID_MESSAGE,
																 m_attr_message,
																 ATTR_DATA_SIZE);
	APP_ERROR_CHECK(err_code);

	err_code = ble_ancs_c_attr_add(&m_ancs_c,
																 BLE_ANCS_NOTIF_ATTR_ID_MESSAGE_SIZE,
																 m_attr_message_size,
																 ATTR_DATA_SIZE);
	APP_ERROR_CHECK(err_code);

	err_code = ble_ancs_c_attr_add(&m_ancs_c,
																 BLE_ANCS_NOTIF_ATTR_ID_DATE,
																 m_attr_date,
																 ATTR_DATA_SIZE);
	APP_ERROR_CHECK(err_code);

	err_code = ble_ancs_c_attr_add(&m_ancs_c,
																 BLE_ANCS_NOTIF_ATTR_ID_POSITIVE_ACTION_LABEL,
																 m_attr_posaction,
																 ATTR_DATA_SIZE);
	APP_ERROR_CHECK(err_code);

	err_code = ble_ancs_c_attr_add(&m_ancs_c,
																 BLE_ANCS_NOTIF_ATTR_ID_NEGATIVE_ACTION_LABEL,
																 m_attr_negaction,
																 ATTR_DATA_SIZE);
	APP_ERROR_CHECK(err_code);

	ancs_init_obj.evt_handler   = on_ancs_c_evt;
	ancs_init_obj.error_handler = apple_notification_error_handler;

	err_code = ble_ancs_c_init(&m_ancs_c, &ancs_init_obj);
	APP_ERROR_CHECK(err_code);
}
#else /* GC_NRF52_ANCS */
static void services_init(void)
{
			ble_nus_init_t nus_init;

			memset(&nus_init, 0, sizeof(nus_init));

			nus_init.data_handler = nus_data_handler;
			
			ble_nus_init(&m_nus, &nus_init);

	#ifdef BLE_DFU_APP_SUPPORT
			/** @snippet [DFU BLE Service initialization] */
			ble_dfu_init_t   dfus_init;

			// Initialize the Device Firmware Update Service.
			memset(&dfus_init, 0, sizeof(dfus_init));

			dfus_init.evt_handler   = dfu_app_on_dfu_evt;
			dfus_init.error_handler = NULL;
			dfus_init.evt_handler   = dfu_app_on_dfu_evt;
			dfus_init.revision      = DFU_REVISION;

			err_code = ble_dfu_init(&m_dfus, &dfus_init);
			APP_ERROR_CHECK(err_code);

			dfu_app_reset_prepare_set(reset_prepare);
			dfu_app_dm_appl_instance_set(m_app_handle);
			/** @snippet [DFU BLE Service initialization] */
	#endif // BLE_DFU_APP_SUPPORT

}
#endif /* GC_NRF52_ANCS */



/**@brief Function for handling an event from the Connection Parameters Module.
 *
 * @details This function will be called for all events in the Connection Parameters Module
 *          which are passed to the application.
 *
 * @note All this function does is to disconnect. This could have been done by simply setting
 *       the disconnect_on_fail config parameter, but instead we use the event handler
 *       mechanism to demonstrate its use.
 *
 * @param[in] p_evt  Event received from the Connection Parameters Module.
 */
static void on_conn_params_evt(ble_conn_params_evt_t * p_evt)
{
    uint32_t err_code;
    
    if(p_evt->evt_type == BLE_CONN_PARAMS_EVT_FAILED)
    {
        err_code = sd_ble_gap_disconnect(m_conn_handle, BLE_HCI_CONN_INTERVAL_UNACCEPTABLE);
        APP_ERROR_CHECK(err_code);
    }
}


/**@brief Function for handling errors from the Connection Parameters module.
 *
 * @param[in] nrf_error  Error code containing information about what went wrong.
 */
static void conn_params_error_handler(uint32_t nrf_error)
{
	#if 0 /* hkim 2016.11.5 */ /* FIXME */
    APP_ERROR_HANDLER(nrf_error); 
	#endif 
}


/**@brief Function for initializing the Connection Parameters module.
 */
static void conn_params_init(void)
{
			uint32_t               err_code;
			ble_conn_params_init_t cp_init;
			
			memset(&cp_init, 0, sizeof(cp_init));

			cp_init.p_conn_params                  = NULL;
			cp_init.first_conn_params_update_delay = FIRST_CONN_PARAMS_UPDATE_DELAY;
			cp_init.next_conn_params_update_delay  = NEXT_CONN_PARAMS_UPDATE_DELAY;
			cp_init.max_conn_params_update_count   = MAX_CONN_PARAMS_UPDATE_COUNT;
			cp_init.start_on_notify_cccd_handle    = BLE_GATT_HANDLE_INVALID;
			cp_init.disconnect_on_fail             = false;
			cp_init.error_handler                  = conn_params_error_handler;
			
			err_code = ble_conn_params_init(&cp_init);
			APP_ERROR_CHECK(err_code);
}


/**@brief Function for putting the chip into sleep mode.
 *
 * @note This function will not return.
 */
static void sleep_mode_enter(void)
{
    uint32_t err_code = bsp_indication_set(BSP_INDICATE_IDLE);
    APP_ERROR_CHECK(err_code);

    // Prepare wakeup buttons.
    err_code = bsp_btn_ble_sleep_mode_prepare();
    APP_ERROR_CHECK(err_code);

    // Go to system-off mode (this function will not return; wakeup will cause a reset).
		configure_ram_retention();
    
		err_code = sd_power_system_off();
    APP_ERROR_CHECK(err_code);
}


/**@brief Function for handling advertising events.
 *
 * @details This function will be called for advertising events which are passed to the application.
 *
 * @param[in] ble_adv_evt  Advertising event.
 */
#ifdef GC_NRF52_ANCS
void on_adv_evt(ble_adv_evt_t ble_adv_evt)
{
    uint32_t err_code;

    switch (ble_adv_evt)
    {
        case BLE_ADV_EVT_DIRECTED:
            err_code = bsp_indication_set(BSP_INDICATE_ADVERTISING_DIRECTED);
            APP_ERROR_CHECK(err_code);
            break;

        case BLE_ADV_EVT_FAST:
            err_code = bsp_indication_set(BSP_INDICATE_ADVERTISING);
            APP_ERROR_CHECK(err_code);
            break;

        case BLE_ADV_EVT_FAST_WHITELIST:
            err_code = bsp_indication_set(BSP_INDICATE_ADVERTISING_WHITELIST);
            APP_ERROR_CHECK(err_code);
            break;

        case BLE_ADV_EVT_SLOW:
            err_code = bsp_indication_set(BSP_INDICATE_ADVERTISING_SLOW);
            APP_ERROR_CHECK(err_code);
            break;

        case BLE_ADV_EVT_IDLE:
            //sleep_mode_enter();
            break;

        case BLE_ADV_EVT_WHITELIST_REQUEST:
        {
            ble_gap_whitelist_t whitelist;
            ble_gap_addr_t    * p_whitelist_addr[BLE_GAP_WHITELIST_ADDR_MAX_COUNT];
            ble_gap_irk_t     * p_whitelist_irk[BLE_GAP_WHITELIST_IRK_MAX_COUNT];

            whitelist.addr_count = BLE_GAP_WHITELIST_ADDR_MAX_COUNT;
            whitelist.irk_count  = BLE_GAP_WHITELIST_IRK_MAX_COUNT;
            whitelist.pp_addrs   = p_whitelist_addr;
            whitelist.pp_irks    = p_whitelist_irk;

            err_code = dm_whitelist_create(&m_app_handle, &whitelist);
            APP_ERROR_CHECK(err_code);

            err_code = ble_advertising_whitelist_reply(&whitelist);
            APP_ERROR_CHECK(err_code);
            break;
        }

        default:
            break;
    }
}
#else /* GC_NRF52_ANCS */
static void on_adv_evt(ble_adv_evt_t ble_adv_evt)
{
    uint32_t err_code;

		//printf("\r\nonadv_evt %d", ble_adv_evt); 
    switch (ble_adv_evt)
    {
        case BLE_ADV_EVT_FAST:
            err_code = bsp_indication_set(BSP_INDICATE_ADVERTISING);
            APP_ERROR_CHECK(err_code);
            break;
        case BLE_ADV_EVT_IDLE:
						printf("[ADV-EVT] idle");
						//sleep_mode_enter(); //hkim-temporaly
            break;
        default:
            break;
    }
}
#endif /* GC_NRF52_ANCS */


/**@brief Function for the application's SoftDevice event handler.
 *
 * @param[in] p_ble_evt SoftDevice event.
 */
#ifdef GC_NRF52_ANCS
static void on_ble_evt(ble_evt_t * p_ble_evt)
{
    uint32_t err_code = NRF_SUCCESS;

    switch (p_ble_evt->header.evt_id)
    {
				case BLE_EVT_TX_COMPLETE:
					//printf("\n\rTXC");
					if(m_ble_status.sending==1)
						m_ble_status.sending=false;
					break;

        case BLE_GAP_EVT_CONNECTED:
            NRF_LOG("Connected.\n\r");
            err_code = bsp_indication_set(BSP_INDICATE_CONNECTED);
            APP_ERROR_CHECK(err_code);
            m_conn_handle = p_ble_evt->evt.gap_evt.conn_handle;
						m_ble_status.connected=true;
						//BT_vib =1;
						if(g_product_mode)
						{
							OLED_power_count=OLED_ON_count_time;
							tuch_count=(OLED_switch) ? display_page_mode : display_page_mode+1;
						}
            break;

        case BLE_GAP_EVT_DISCONNECTED: 
            NRF_LOG("Disconnected.\n\r");
            if (p_ble_evt->evt.gap_evt.conn_handle == m_ancs_c.conn_handle)
            {
                m_ancs_c.conn_handle = BLE_CONN_HANDLE_INVALID;
            }
						else 
						{
								m_conn_handle = BLE_CONN_HANDLE_INVALID;
						}
						m_ble_status.connected = false;
						//규창 임시..
						//op_mode = ((op_mode == STRESS_MODE) || (op_mode ==  COACH_MODE))? DAILY_MODE : op_mode;
						//gc_protocol_ctrl_pah8002(op_mode, 0);
						m_ble_status.send_enable = 0;
						g_total_tx_busy_count=0;
						send_rawdata = 2;
						nrf_gpio_pin_clear(GC_GPIO_MOTOR_ENABLE);
						BT_vib =0;
						noti_num = 20;
						if(g_product_mode)
						{
							OLED_power_count=OLED_ON_count_time;
							tuch_count=(OLED_switch) ? display_page_mode : display_page_mode+1;
						}
            break;
						
        case BLE_GATTS_EVT_TIMEOUT:
            NRF_LOG("Timeout.\n\r");
            // Disconnect on GATT Server and Client time-out events.
            err_code = sd_ble_gap_disconnect(p_ble_evt->evt.gatts_evt.conn_handle,
                                             BLE_HCI_REMOTE_USER_TERMINATED_CONNECTION);
            APP_ERROR_CHECK(err_code);
            break;

        case BLE_GATTC_EVT_TIMEOUT:
            NRF_LOG("Timeout.\n\r");
            // Disconnect on GATT Server and Client time-out events.
            err_code = sd_ble_gap_disconnect(p_ble_evt->evt.gattc_evt.conn_handle,
                                             BLE_HCI_REMOTE_USER_TERMINATED_CONNECTION);
            APP_ERROR_CHECK(err_code);
            break;

        default:
            // No implementation needed.
            break;
    }
    APP_ERROR_CHECK(err_code);
}
#else /* GC_NRF52_ANCS */
static void on_ble_evt(ble_evt_t * p_ble_evt)
{
    uint32_t                         err_code;
    
    switch (p_ble_evt->header.evt_id)
    {
				case BLE_EVT_TX_COMPLETE:
					//printf("\n\rTXC");

					if(m_ble_status.sending==1)
						m_ble_status.sending=false;
					break;
		
        case BLE_GAP_EVT_CONNECTED:
#ifdef GREENCOMM_BT
						printf("\r\n BLE_CONNECTED");
#endif
            err_code = bsp_indication_set(BSP_INDICATE_CONNECTED);
            APP_ERROR_CHECK(err_code);
            m_conn_handle = p_ble_evt->evt.gap_evt.conn_handle;
						m_ble_status.connected=true;
            break;
            
        case BLE_GAP_EVT_DISCONNECTED:
#ifdef GREENCOMM_BT
						printf("\r\n BLE_DISCONNECTED");
#endif
            err_code = bsp_indication_set(BSP_INDICATE_IDLE);
            APP_ERROR_CHECK(err_code);
            m_conn_handle = BLE_CONN_HANDLE_INVALID;
						m_ble_status.connected = false;
						op_mode = ((op_mode == STRESS_MODE) || (op_mode ==  COACH_MODE))? DAILY_MODE : op_mode;
						send_rawdata = 2;
            break;

        case BLE_GAP_EVT_SEC_PARAMS_REQUEST:
            // Pairing not supported
            err_code = sd_ble_gap_sec_params_reply(m_conn_handle, BLE_GAP_SEC_STATUS_PAIRING_NOT_SUPP, NULL, NULL);
            APP_ERROR_CHECK(err_code);
            break;

        case BLE_GATTS_EVT_SYS_ATTR_MISSING:
            // No system attributes have been stored.
            err_code = sd_ble_gatts_sys_attr_set(m_conn_handle, NULL, 0, 0);
            APP_ERROR_CHECK(err_code);
            break;
				
        default:
            // No implementation needed.
            break;
    }
}
#endif /* GC_NRF52_ANCS */


/**@brief Function for dispatching a SoftDevice event to all modules with a SoftDevice 
 *        event handler.
 *
 * @details This function is called from the SoftDevice event interrupt handler after a 
 *          SoftDevice event has been received.
 *
 * @param[in] p_ble_evt  SoftDevice event.
 */
#ifdef GC_NRF52_ANCS
static void ble_evt_dispatch(ble_evt_t * p_ble_evt)
{
		SEGGER_RTT_printf(0,"\r\n event 0x%x", p_ble_evt->header.evt_id);
#if 1 /* hkim debug */	
		if ( p_ble_evt->header.evt_id == BLE_GAP_EVT_CONNECTED )
			SEGGER_RTT_printf(0, "\r\n [BLE_EVT_DISP] event CONNECT");
		if ( p_ble_evt->header.evt_id == BLE_GAP_EVT_DISCONNECTED )
			SEGGER_RTT_printf(0, "\r\n [BLE_EVT_DISP] event DISCONNECT");
#endif 
    dm_ble_evt_handler(p_ble_evt);
    ble_db_discovery_on_ble_evt(&m_ble_db_discovery, p_ble_evt); /* event start from 0x30 */
	  ble_ancs_c_on_ble_evt(&m_ancs_c, p_ble_evt);
	
		ble_nus_on_ble_evt(&m_nus, p_ble_evt);
		ble_conn_params_on_ble_evt(p_ble_evt);
    bsp_btn_ble_on_ble_evt(p_ble_evt);
    on_ble_evt(p_ble_evt);
#if 1 /* hkim 2016.10.25 */
	if (p_ble_evt->header.evt_id == BLE_GAP_EVT_TIMEOUT)
	{
		//printf("\r\n [ADV_TM] start adv");
		ble_advertising_start(BLE_ADV_MODE_FAST);
	}
  else
	{
		ble_advertising_on_ble_evt(p_ble_evt);
	}
#else
    ble_advertising_on_ble_evt(p_ble_evt); 
#endif
}
#else /* GC_NRF52_ANCS */
static void ble_evt_dispatch(ble_evt_t * p_ble_evt)
{
		//printf("\r\n evt_dis %d", p_ble_evt->header.evt_id);
    ble_conn_params_on_ble_evt(p_ble_evt);
    ble_nus_on_ble_evt(&m_nus, p_ble_evt);
#ifdef BLE_DFU_APP_SUPPORT
    /** @snippet [Propagating BLE Stack events to DFU Service] */
    ble_dfu_on_ble_evt(&m_dfus, p_ble_evt);
    /** @snippet [Propagating BLE Stack events to DFU Service] */
#endif // BLE_DFU_APP_SUPPORT
    on_ble_evt(p_ble_evt);
    ble_advertising_on_ble_evt(p_ble_evt); 
    bsp_btn_ble_on_ble_evt(p_ble_evt); 
    
}
#endif /* GC_NRF52_ANCS */


/**@brief Function for the SoftDevice initialization.
 *
 * @details This function initializes the SoftDevice and the BLE event interrupt.
 */
#ifdef GC_NRF52_ANCS
/**@brief Function for dispatching a system event to interested modules.
 *
 * @details This function is called from the system event interrupt handler after a system
 *          event has been received.
 *
 * @param[in] sys_evt  System stack event.
 */
static void sys_evt_dispatch(uint32_t sys_evt)
{
    pstorage_sys_event_handler(sys_evt);
    ble_advertising_on_sys_evt(sys_evt);
}
#endif /* GC_NRF52_ANCS */

static void ble_stack_init(void)
{
    uint32_t err_code;
    
   nrf_clock_lf_cfg_t clock_lf_cfg = NRF_CLOCK_LFCLKSRC;
     //nrf_clock_lf_cfg_t clock_lf_cfg = NRF_
    // Initialize the SoftDevice handler module.
    SOFTDEVICE_HANDLER_INIT(&clock_lf_cfg, NULL);
    
    ble_enable_params_t ble_enable_params;
    err_code = softdevice_enable_get_default_config(CENTRAL_LINK_COUNT,
                                                    PERIPHERAL_LINK_COUNT,
                                                    &ble_enable_params);
   APP_ERROR_CHECK(err_code);

#if 0 /* hkim 2016.10.14. already configured as '5' */
    //ble_enable_params.common_enable_params.vs_uuid_count = VENDOR_SPECIFIC_UUID_COUNT;
#endif
	  
		//Check the ram settings against the used number of links
    CHECK_RAM_START_ADDR(CENTRAL_LINK_COUNT,PERIPHERAL_LINK_COUNT);
    
    // Enable BLE stack.
    err_code = softdevice_enable(&ble_enable_params);
		APP_ERROR_CHECK(err_code);

    // Register with the SoftDevice handler module for BLE events.
    err_code = softdevice_ble_evt_handler_set(ble_evt_dispatch);
    APP_ERROR_CHECK(err_code);

#ifdef GC_NRF52_ANCS
    // Register with the SoftDevice handler module for System events.
    err_code = softdevice_sys_evt_handler_set(sys_evt_dispatch);
    APP_ERROR_CHECK(err_code);
#endif 
}


/**@brief Function for handling events from the BSP module.
 *
 * @param[in]   event   Event generated by button press.
 */
void bsp_event_handler(bsp_event_t event)
{
		int save_time = (get_GMT(time.year,  time.month,  time.day,  time.hour,  time.minute, time.second )/(86400))*1440;
		unsigned char is_save_step=0, is_save_cal=0, i=0;
    uint32_t err_code;
    switch (event)
    {
        case BSP_EVENT_SLEEP:
					printf("\r\n bsp : sleep mode"); //hkim-debug
					//161005 hm_lee

					
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
						is_save_step = gc_flash_page_write_data(GC_FLASH_TBL_TYPE_BAT_STEP, req_data);
						nrf_delay_ms(10); /* FIXME */
						if(is_save_step)
						{
							i=6;
						}
					}				
					_tbl_kcal.date     = kcal.date;
					_tbl_kcal.daily		= kcal.step_cal;
					_tbl_kcal.sports	= kcal.activty_cal;
					_tbl_kcal.sleep		= kcal.sleep_cal;
					_tbl_kcal.coach	= kcal.coach_cal;
					memcpy(req_data, &_tbl_kcal, sizeof(_GC_TBL_CALORIE));
					
					for(i=0;i<=5;i++)
					{
						is_save_cal = gc_flash_page_write_data(GC_FLASH_TBL_TYPE_BAT_ACT, req_data);
						nrf_delay_ms(10); /* FIXME */
						if(is_save_cal)
						{
							i=6;
						}
					}
					//161005 hm_lee
             sleep_mode_enter();
            break;

        case BSP_EVENT_DISCONNECT:
            err_code = sd_ble_gap_disconnect(m_conn_handle, BLE_HCI_REMOTE_USER_TERMINATED_CONNECTION);
            if (err_code != NRF_ERROR_INVALID_STATE)
            {
                APP_ERROR_CHECK(err_code);
            }
            break;

        case BSP_EVENT_WHITELIST_OFF:
            err_code = ble_advertising_restart_without_whitelist();
            if (err_code != NRF_ERROR_INVALID_STATE)
            {
                APP_ERROR_CHECK(err_code);
            }
            break;

        default:
            break;
    }
}


/**@brief   Function for handling app_uart events.
 *
 * @details This function will receive a single character from the app_uart module and append it to 
 *          a string. The string will be be sent over BLE when the last character received was a 
 *          'new line' i.e '\n' (hex 0x0D) or if the string has reached a length of 
 *          @ref NUS_MAX_DATA_LENGTH.
 */
/**@snippet [Handling the data received over UART] */
void uart_event_handle(app_uart_evt_t * p_event)
{
    static uint8_t data_array[BLE_NUS_MAX_DATA_LEN];
    static uint8_t index = 0;
#ifndef GREENCOMM_NUS
		uint32_t       err_code;
#endif

    switch (p_event->evt_type)
    {
        case APP_UART_DATA_READY:
            UNUSED_VARIABLE(app_uart_get(&data_array[index]));

#ifdef GREENCOMM_NUS /* ble_uart hkim 2016.5.31 */
						debug_shell( data_array[index] ); 
#endif			
            index++;
#ifndef GREENCOMM_NUS /* ble_uart hkim 2016.5.31 */
				    if ((data_array[index - 1] == '\n') || (index >= (BLE_NUS_MAX_DATA_LEN)))
#else
            if ((data_array[index - 1] == '\r') || (index >= (BLE_NUS_MAX_DATA_LEN)))
#endif
            {
							
#ifndef GREENCOMM_NUS /* 2016.6.8 hkim. to prevent UART from sending all message through BLE */
                err_code = ble_nus_string_send(&m_nus, data_array, index);
                if (err_code != NRF_ERROR_INVALID_STATE)
                {
                    APP_ERROR_CHECK(err_code);
                }
#endif /* GREENCOMM_NUS */
                
                index = 0;
            }
            break;

        case APP_UART_COMMUNICATION_ERROR:
            APP_ERROR_HANDLER(p_event->data.error_communication);
            break;

        case APP_UART_FIFO_ERROR:
            APP_ERROR_HANDLER(p_event->data.error_code);
            break;

        default:
            break;
    }
}
/**@snippet [Handling the data received over UART] */


/**@brief  Function for initializing the UART module.
 */
/**@snippet [UART Initialization] */
static void uart_init(void)
{
    uint32_t                     err_code;
    const app_uart_comm_params_t comm_params =
    {
        RX_PIN_NUMBER,
        TX_PIN_NUMBER,
        RTS_PIN_NUMBER,
        CTS_PIN_NUMBER,
        APP_UART_FLOW_CONTROL_DISABLED,
        false,
        UART_BAUDRATE_BAUDRATE_Baud115200
    };

		
    APP_UART_FIFO_INIT( &comm_params,
                       UART_RX_BUF_SIZE,
                       UART_TX_BUF_SIZE,
                       uart_event_handle,
                       APP_IRQ_PRIORITY_LOW,
                       err_code);
		
		//printf("\r\n UART buffer size Tx/Rx : %d,%d", UART_TX_BUF_SIZE, UART_RX_BUF_SIZE);
    APP_ERROR_CHECK(err_code);
}
/**@snippet [UART Initialization] */

/**@brief Function for handling the Device Manager events.
 *
 * @param[in] p_evt  Data associated to the device manager event.
 */
static uint32_t device_manager_evt_handler(dm_handle_t const * p_handle,
                                           dm_event_t const  * p_event,
                                           ret_code_t        event_result)
{
	 uint32_t err_code;

	switch (p_event->event_id)
	{
			case DM_EVT_CONNECTION:
					m_peer_handle = (*p_handle);
					err_code      = app_timer_start(m_sec_req_timer_id, SECURITY_REQUEST_DELAY, NULL);
					APP_ERROR_CHECK(err_code);
					break;

			case DM_EVT_LINK_SECURED:
					err_code = ble_db_discovery_start(&m_ble_db_discovery,
																						p_event->event_param.p_gap_param->conn_handle);
					APP_ERROR_CHECK(err_code);
					break; 

			default:
					break;

	}
	return NRF_SUCCESS;
}


/**@brief Function for the Device Manager initialization.
 *
 * @param[in] erase_bonds  Indicates whether bonding information should be cleared from
 *                         persistent storage during initialization of the Device Manager.
 */
static void device_manager_init(bool erase_bonds)
{
    uint32_t               err_code;
    dm_init_param_t        init_param = {.clear_persistent_data = erase_bonds};
    dm_application_param_t register_param;

    // Initialize persistent storage module.
    err_code = pstorage_init();
    APP_ERROR_CHECK(err_code);

    err_code = dm_init(&init_param);
    APP_ERROR_CHECK(err_code);

    memset(&register_param.sec_param, 0, sizeof(ble_gap_sec_params_t));

    register_param.sec_param.bond         = SEC_PARAM_BOND;
    register_param.sec_param.mitm         = SEC_PARAM_MITM;
    register_param.sec_param.lesc         = SEC_PARAM_LESC;
    register_param.sec_param.keypress     = SEC_PARAM_KEYPRESS;
    register_param.sec_param.io_caps      = SEC_PARAM_IO_CAPABILITIES;
    register_param.sec_param.oob          = SEC_PARAM_OOB;
    register_param.sec_param.min_key_size = SEC_PARAM_MIN_KEY_SIZE;
    register_param.sec_param.max_key_size = SEC_PARAM_MAX_KEY_SIZE;
    register_param.evt_handler            = device_manager_evt_handler;
    register_param.service_type           = DM_PROTOCOL_CNTXT_GATT_SRVR_ID;
  
#ifdef GC_NRF52_ANCS  
    memcpy(&m_sec_param, &register_param.sec_param, sizeof(ble_gap_sec_params_t));
#endif

    err_code = dm_register(&m_app_handle, &register_param);
    APP_ERROR_CHECK(err_code);
}


/**@brief Function for initializing the Advertising functionality.
 */
 ble_gap_addr_t                    m_ble_addr; 
#ifdef GC_NRF52_ANCS
void advertising_init(void)
{
    uint32_t      err_code;
    ble_advdata_t advdata;
    ble_advdata_t scanrsp;
    uint8_t       flags = BLE_GAP_ADV_FLAGS_LE_ONLY_GENERAL_DISC_MODE; /* Dont' erase */
		ble_uuid_t    ancs_uuid;
    ble_uuid_t nus_uuids[] = {{BLE_UUID_NUS_SERVICE, NUS_SERVICE_UUID_TYPE}};
    
    ancs_uuid.uuid = ANCS_UUID_SERVICE;
    ancs_uuid.type = m_ancs_c.service.service.uuid.type;
       
    
    // Build and set advertising data.
    memset(&advdata, 0, sizeof(advdata));

    advdata.name_type               = BLE_ADVDATA_FULL_NAME;
    advdata.include_appearance      = false;
    advdata.flags                    = BLE_GAP_ADV_FLAGS_LE_ONLY_GENERAL_DISC_MODE;
		#if 1
		advdata.uuids_complete.uuid_cnt = 0;
		advdata.uuids_complete.p_uuids  = NULL;
		#else
		advdata.uuids_complete.uuid_cnt = 1;
		advdata.uuids_complete.p_uuids  = nus_uuids;
		#endif 
		advdata.uuids_solicited.uuid_cnt = 1;
    advdata.uuids_solicited.p_uuids  = &ancs_uuid;    
    
    memset(&scanrsp, 0, sizeof(scanrsp));
    scanrsp.uuids_complete.uuid_cnt = sizeof(nus_uuids) / sizeof(nus_uuids[0]);
    scanrsp.uuids_complete.p_uuids  = nus_uuids;
    
		/* option */
		ble_adv_modes_config_t options = {0};
    options.ble_adv_whitelist_enabled = BLE_ADV_WHITELIST_ENABLED;
    options.ble_adv_fast_enabled      = BLE_ADV_FAST_ENABLED;
    options.ble_adv_fast_interval     = APP_ADV_FAST_INTERVAL;
    options.ble_adv_fast_timeout      = APP_ADV_FAST_TIMEOUT;
    options.ble_adv_slow_enabled      = BLE_ADV_SLOW_ENABLED;
    options.ble_adv_slow_interval     = APP_ADV_SLOW_INTERVAL;
    options.ble_adv_slow_timeout      = APP_ADV_SLOW_TIMEOUT;
		    
    err_code = ble_advertising_init(&advdata, &scanrsp, &options, on_adv_evt, NULL);
    APP_ERROR_CHECK(err_code);
		
		err_code = sd_ble_gap_address_get(&m_ble_addr);
    //APP_ERROR_CHECK(err_code);
}
#else /* GC_NRF52_ANCS */
static void advertising_init(void)
{
    uint32_t      err_code;
    ble_advdata_t advdata;
    ble_advdata_t scanrsp;

    // Build advertising data struct to pass into @ref ble_advertising_init.
    memset(&advdata, 0, sizeof(advdata));
    advdata.name_type          = BLE_ADVDATA_FULL_NAME;
    advdata.include_appearance = false;
#ifdef GREENCOMM_BT  /* hkim 2016.6.9.  To forever BLE advertising */
		advdata.flags              = BLE_GAP_ADV_FLAGS_LE_ONLY_GENERAL_DISC_MODE;
#else
    advdata.flags              = BLE_GAP_ADV_FLAGS_LE_ONLY_LIMITED_DISC_MODE;
#endif /* GREENCOMM_BT */


#ifdef GREENCOMM_BT /* hkim 2016.6.9. iOS requirement to scan device in app background mode */
		advdata.uuids_complete.uuid_cnt = sizeof(m_adv_uuids) / sizeof(m_adv_uuids[0]);
    advdata.uuids_complete.p_uuids  = m_adv_uuids;
#endif /* GREENCOMM_BT */ 

    memset(&scanrsp, 0, sizeof(scanrsp));
    scanrsp.uuids_complete.uuid_cnt = sizeof(m_adv_uuids) / sizeof(m_adv_uuids[0]);
    scanrsp.uuids_complete.p_uuids  = m_adv_uuids;

    ble_adv_modes_config_t options = {0};
    options.ble_adv_fast_enabled  = BLE_ADV_FAST_ENABLED;
    options.ble_adv_fast_interval = APP_ADV_INTERVAL;
    options.ble_adv_fast_timeout  = APP_ADV_TIMEOUT_IN_SECONDS;

    err_code = ble_advertising_init(&advdata, &scanrsp, &options, on_adv_evt, NULL);
    APP_ERROR_CHECK(err_code);
		
		err_code = sd_ble_gap_address_get(&m_ble_addr);
    APP_ERROR_CHECK(err_code);
}
#endif /* GC_NRF52_ANCS */


/**@brief Function for initializing buttons and leds.
 *
 * @param[out] p_erase_bonds  Will be true if the clear bonding button was pressed to wake the application up.
 */
#ifdef GC_COACH_FIT_DK52
static void buttons_leds_init(bool * p_erase_bonds)
{
    bsp_event_t startup_event;

    uint32_t err_code = bsp_init(BSP_INIT_LED | BSP_INIT_BUTTONS,
                                 APP_TIMER_TICKS(100, APP_TIMER_PRESCALER), 
                                 bsp_event_handler);
    APP_ERROR_CHECK(err_code);

    err_code = bsp_btn_ble_init(NULL, &startup_event);
    APP_ERROR_CHECK(err_code);

    *p_erase_bonds = (startup_event == BSP_EVENT_CLEAR_BONDING_DATA);
}
#endif /* GC_COACH_FIT_DK52 */

#ifdef GC_NRF52_ANCS
/**@brief Function for initializing the Event Scheduler.
 */
static void scheduler_init(void)
{
    APP_SCHED_INIT(SCHED_MAX_EVENT_DATA_SIZE, SCHED_QUEUE_SIZE);
}

/**@brief Function for handling Database Discovery events.
 *
 * @details This function is a callback function to handle events from the database discovery module.
 *          Depending on the UUIDs that are discovered, this function s`hould forward the events
 *          to their respective service instances.
 *
 * @param[in] p_event  Pointer to the database discovery event.
 */
void db_disc_handler(ble_db_discovery_evt_t * p_evt)
{
    ble_ancs_c_on_db_disc_evt(&m_ancs_c, p_evt);
}

/**@brief Function for initializing the database discovery module.
 */
static void db_discovery_init(void)
{
    uint32_t err_code = ble_db_discovery_init(db_disc_handler);
    APP_ERROR_CHECK(err_code);
}
#endif /* GC_NRF52_ANCS */

/**@brief Function for placing the application in low power state while waiting for events.
 */

//타이머 주기 체크용 변수들 
static uint32_t __count=0;
static uint32_t __AN520count=0;

/* hkim 2016.8.18 
** FPU **
- When the FPU is in use, it triggers the FPU_IRQn interrupt when one of
  the six exception flags (IDC, IXC, UFC, OFC, DZC, IOC) is set.
  The FPU interrupt will always set the pending flag (even if the 
  interrupt is not enabled), irrespective of whether the user is interested in the exception bit.
  
  [IMPORTANT]
  The pending flag then prevents the SoftDevice from going into low power mode when sd_app_evt_wait() is called.
  Therefore, always clear the exception bits and the pending interrupt 
  before calling sd_app_evt_wait(). 

  FPU exception bit definition:
  http://infocenter.arm.com/help/index.jsp?topic=/com.arm.doc.dui0553a/BABBFJEC.html
*/
// Set bit 7 and bits 4..0 in the mask to one (0x ...00 1001 1111)

 #define FPU_EXCEPTION_MASK 0x0000009F
static void power_manage(void)
{
	/* Clear exceptions and PendingIRQ from the FPU unit */
    __set_FPSCR(__get_FPSCR()  & ~(FPU_EXCEPTION_MASK));      
    (void) __get_FPSCR();
    NVIC_ClearPendingIRQ(FPU_IRQn);
	
    uint32_t err_code = sd_app_evt_wait();
    APP_ERROR_CHECK(err_code);
}

#ifdef GREENCOMM_NUS /* app_timer hkim 2016.6.1 */
/** @brief Function starting the internal LFCLK XTAL oscillator.
 */
#define GPIO_TEST_PIN	16

void BLE_send_rawdata(short acc_x,short acc_y,short acc_z,unsigned char send_rawdata)
{
	static char timeout_count=0;  /* hkim 2016.9.26 */
	
	#if 0 /* hkim 2016.9.26 */
		if(send_rawdata==1)
	#else
		if(!m_ble_status.connected)	
			return;
		
		//printf("\r\n **** %d %d", m_ble_status.sending, m_ble_status.send_enable);
		if( !m_ble_status.sending &&  m_ble_status.send_enable == 1)
	#endif
		{
			if(__count%2==0)
			{	
					send_raw_bf[0]=0x10; //CMD
					send_raw_bf[1]=0x00;
					send_raw_bf[2]=acc_x&0x00ff; //AccX-1
					send_raw_bf[3]=(acc_x&0xff00)>>8;
					send_raw_bf[4]=acc_y&0x00ff; //AccY-1
					send_raw_bf[5]=(acc_y&0xff00)>>8;
					send_raw_bf[6]=acc_z&0x00ff; //AccZ-1
					send_raw_bf[7]=(acc_z&0xff00)>>8;
					send_raw_bf[8]=g_hr; //Hr-1
					send_raw_bf[9]=0x00;
			}
			else
			{
					send_raw_bf[10]=acc_x&0x00ff; //AccX-2
					send_raw_bf[11]=(acc_x&0xff00)>>8;
					send_raw_bf[12]=acc_y&0x00ff; //AccY-2
					send_raw_bf[13]=(acc_y&0xff00)>>8;
					send_raw_bf[14]=acc_z&0x00ff; //AccZ-2
					send_raw_bf[15]=(acc_z&0xff00)>>8;
					send_raw_bf[16]=g_hr; //Hr-2
					send_raw_bf[17]=0x00;
			
	//			printf("\r\n rawsample_xyz-    [%d][%d][%d][%d]",sample_xyz.x,sample_xyz.y,sample_xyz.z,g_hr);
	//			printf("\r\n raw response xyzh-[%d][%d][%d][%d]\n",
	//				(short)(send_raw_bf[10]|send_raw_bf[11]<<8),
	//				(short)(send_raw_bf[12]|send_raw_bf[13]<<8),
	//				(short)(send_raw_bf[14]|send_raw_bf[15]<<8),
	//				(short)(send_raw_bf[16]|send_raw_bf[17]<<8));
	//			printf("\r\n raw response xyzh-[%d][%d][%d][%d],[%d][%d][%d][%d]",send_raw_bf[2],send_raw_bf[4],send_raw_bf[6],send_raw_bf[8],send_raw_bf[10],send_raw_bf[12],send_raw_bf[14],send_raw_bf[16]);
				ble_send( (uint8_t*)send_raw_bf  , 18 );
				//printf("\r\n send_raw %d",__count);

			}
		}
		else /* hkim 2016.9.26 */
		{
			if( m_ble_status.send_enable)
			{
				//printf("\r\nTX busy %d",timeout_count);
				g_total_tx_busy_count++;
			}

			//if(timeout_count>=40 &&  m_ble_status.send_enable)
			if(m_ble_status.send_enable)
			{	
				timeout_count = 0;
				m_ble_status.sending = 0;
				return;
			}
			timeout_count++;
		}
}



//규창 171102 피쳐 전송기능
extern int pastfeature_request_firsttime;
extern int pastfeature_request_time;
extern int pastfeature_request_lasttime;
extern bool past_feature_flag;
int featurenum =0 ;

uint16_t olderstAddr = 0;
uint16_t lastAddr = 0;

void BLE_send_featuredata()
{
	char s[20];
	static char timeout_count=0;  /* hkim 2016.9.26 */
	
	#if 0 /* hkim 2016.9.26 */
		if(send_rawdata==1)
	#else
		if(!m_ble_status.connected)	{
			past_feature_flag = false;
			return;
		}
		if(past_feature_flag== false)
			return;
			
		//printf("\r\n **** %d %d", m_ble_status.sending, m_ble_status.send_enable);
		if( !m_ble_status.sending &&  past_feature_flag == true)
	#endif
		{			
			//find_req_past_feature(pastfeature_request_firsttime);
			//extern uint16_t olderstAddr;
			//extern uint16_t lasstAddr
			
			
			
			
			
			struct feature_tbl_form_s featureSend;
			
			
			
			//규창 171103 어드레스가 메모리 순환 끝 영역인  5919을 넘어갔을때 시작으로 돌림
			if (olderstAddr > GC_FLASH_PAGE_END_DAY3_TIME){
				olderstAddr = GC_FLASH_PAGE_START_TIME;
			}
			if (olderstAddr == lastAddr){
			
				past_feature_flag = false;
				olderstAddr = 0;
				lastAddr = 0;
				pastfeature_request_firsttime= 0;
				pastfeature_request_lasttime=0;
				pastfeature_request_time=0;
				
				//규창 피쳐데이터 전송 완료 메시지
				s[0] = 0xBF; //cmd
				s[1] = 0x00;
				ble_send( (uint8_t*)s,2);
				return;
			}

			
			featureSend = find_req_past_feature(olderstAddr,pastfeature_request_time);
			
			olderstAddr++;
			//pastfeature_request_time++;
			
			printf("\r\n olderstAddr %d lastAddr %d", olderstAddr, lastAddr);
			printf("\r\n pastfeature_request_firsttime %d", pastfeature_request_time);
			printf("\r\n feature : time %d", featureSend.time);
			printf("\r\n feature : n_var:%d pressure:%d prediff:%d x_mean:%d y_mean:%d z_mean:%d n_mean:%d x_var:%d y_var:%d z_var:%d", featureSend.norm_var, featureSend.pressure, featureSend.pre_diff_sum,featureSend.x_mean, featureSend.y_mean, featureSend.z_mean,featureSend.norm_mean,featureSend.x_var ,featureSend.y_var,	featureSend.z_var);
			printf("\r\n feature : step:%d rope_cnt:%d s_swing:%d l_swing:%d",featureSend.nStep,featureSend.jumping_rope_count ,featureSend.small_swing_count,featureSend.large_swing_count);
			printf("\r\n feature : hb:%d",featureSend.hb);
			
			if( featureSend.time == -1){
					printf("\r\nfeature not saved");
					return;
			}
			if( featureSend.time == 0){
					printf("\r\nfeature not saved");
					return;
			}
				 //printf("\r\n feature_request_time %d", feature_request_time);
				  //printf("\r\n featuredata response1");
				//	if (featurenum ==0){
					s[0] = 0xBA; //cmd
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
				//	featurenum++;
				//}
				//else if (featurenum == 1){
				  //printf("\r\n featuredata response2");
					s[0] = 0xBB; //cmd
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
			//		featurenum++;
			//	}else if(featurenum ==2){
					s[0] = 0xBC; //cmd
					s[1] = 0x00;
					s[2] = featureSend.pressure  & 0x000000ff;
				  s[3] = (featureSend.pressure & 0x0000ff00)>>8; 
					s[4] = (featureSend.pressure & 0x00ff0000)>>16; 
					s[5] = (featureSend.pressure & 0xff000000)>>24;
					s[6] = featureSend.hb & 0x00ff;
					s[7] = 0x00;
					ble_send( (uint8_t*)s,8);
				//	featurenum=0;
				//	pastfeature_request_firsttime++;
			//	}
			//	if(pastfeature_request_firsttime == pastfeature_request_lasttime){
			//		m_ble_status.send_enable = 0;
			//		featurenum=0;
			//		pastfeature_request_firsttime = 0;
			//		pastfeature_request_lasttime = 0;
			//		past_feature_flag = false;
			//	}
		}
		else // hkim 2016.9.26 
		{
			if(past_feature_flag == true)
			{
				//printf("\r\nTX busy %d",timeout_count);
				g_total_tx_busy_count++;
			}

			//if(timeout_count>=40 &&  m_ble_status.send_enable)
			if(past_feature_flag == true)
			{	
				timeout_count = 0;
				m_ble_status.sending = 0;
				return;
			}
			timeout_count++;
			
		}
}


	
void BLE_send_rawdata_test(short acc_x,short acc_y,short acc_z,unsigned char send_rawdata)
{
	//static unsigned int try_count=0; //hkim-temporaly
	static char timeout_count=0;  /* hkim 2016.9.26 */
	//printf("\r\n conn %d, sending %d, send_raw %d", m_ble_status.connected, m_ble_status.sending, send_rawdata);
	#if 0 /* hkim 2016.9.26 */
		if(send_rawdata==1)
	#else
		if(!m_ble_status.connected)	
			return;
		
#if 0 //debug 		
		try_count++;
		printf("\r\n ***** try_count %d", try_count);
		printf("\r\n sending %d", m_ble_status.sending);
#endif 
		
		if( !m_ble_status.sending &&  m_ble_status.send_enable == 1)
	#endif
		{
			ble_100ms_count++;
			if(ble_100ms_count&1)//__count%2==0)
			{
				send_raw_bf[0]=0x50; //CMD
				
				send_raw_bf[2]=acc_x&0x00ff; //AccX-1
				send_raw_bf[3]=(acc_x&0xff00)>>8;
				send_raw_bf[4]=acc_y&0x00ff; //AccY-1
				send_raw_bf[5]=(acc_y&0xff00)>>8;
				send_raw_bf[6]=acc_z&0x00ff; //AccZ-1
				send_raw_bf[7]=(acc_z&0xff00)>>8;
				send_raw_bf[8]=0x00; //press-1
				send_raw_bf[9]=0x00;
				send_raw_bf[18]=g_hr; //hr-1
			}
			else
			{
				send_raw_bf[1]=(unsigned char)(ble_100ms_count>>1);
				send_raw_bf[10]=acc_x&0x00ff; //AccX-2
				send_raw_bf[11]=(acc_x&0xff00)>>8;
				send_raw_bf[12]=acc_y&0x00ff; //AccY-2
				send_raw_bf[13]=(acc_y&0xff00)>>8;
				send_raw_bf[14]=acc_z&0x00ff; //AccZ-2
				send_raw_bf[15]=(acc_z&0xff00)>>8;
				send_raw_bf[16]=0x00; //press-2
				send_raw_bf[17]=0x00;
				send_raw_bf[19]=g_hr; //hr-2
				
				ble_send( (uint8_t*)send_raw_bf  , 20 );
			}
		}
		else /* hkim 2016.9.26 */
		{
			if( m_ble_status.send_enable)
			{
				//printf("\r\nTX busy %d(try %d)",timeout_count, try_count); //debug
				g_total_tx_busy_count++;
			}

			if(timeout_count>=40 &&  m_ble_status.send_enable)
			{	
				timeout_count = 0;
				m_ble_status.sending = 0;
				return;
			}
			
			timeout_count++;
		}
}

void BLE_send_ADC(short acc_x,short acc_y,short acc_z,unsigned char send_rawdata)
{
	//static unsigned int try_count=0; //hkim-temporaly
	static char timeout_count=0;  /* hkim 2016.9.26 */
	//printf("\r\n conn %d, sending %d, send_raw %d", m_ble_status.connected, m_ble_status.sending, send_rawdata);
	#if 0 /* hkim 2016.9.26 */
		if(send_rawdata==1)
	#else
		if(!m_ble_status.connected)	
			return;
		
#if 0 //debug 		
		try_count++;
		printf("\r\n ***** try_count %d", try_count);
		printf("\r\n sending %d", m_ble_status.sending);
#endif 
		
		if( !m_ble_status.sending &&  m_ble_status.send_enable == 1)
	#endif
		{
			ble_100ms_count++;
			if(ble_100ms_count&1)//__count%2==0)
			{
				send_raw_bf[0]=0x50; //CMD
				
				send_raw_bf[2]=gc_saadc&0x00ff; //AccX-1
				send_raw_bf[3]=(gc_saadc&0xff00)>>8;
				send_raw_bf[4]=g_battery&0x00ff; //AccY-1
				send_raw_bf[5]=(g_battery&0xff00)>>8;
				send_raw_bf[6]=0&0x00ff; //AccZ-1
				send_raw_bf[7]=(0&0xff00)>>8;
				send_raw_bf[8]=0x00; //press-1
				send_raw_bf[9]=0x00;
				send_raw_bf[18]=g_hr; //hr-1
			}
			else
			{
				send_raw_bf[1]=(unsigned char)(ble_100ms_count>>1);
				send_raw_bf[10]=gc_saadc&0x00ff; //AccX-2
				send_raw_bf[11]=(gc_saadc&0xff00)>>8;
				send_raw_bf[12]=g_battery&0x00ff; //AccY-2
				send_raw_bf[13]=(g_battery&0xff00)>>8;
				send_raw_bf[14]=0&0x00ff; //AccZ-2
				send_raw_bf[15]=(0&0xff00)>>8;
				send_raw_bf[16]=0x00; //press-2
				send_raw_bf[17]=0x00;
 				send_raw_bf[19]=g_hr; //hr-2
				
				ble_send( (uint8_t*)send_raw_bf  , 20 );
			}
		}
		else /* hkim 2016.9.26 */
		{
			if( m_ble_status.send_enable)
			{
				//printf("\r\nTX busy %d(try %d)",timeout_count, try_count); //debug
				g_total_tx_busy_count++;
			}

			if(timeout_count>=40 &&  m_ble_status.send_enable)
			{	
				timeout_count = 0;
				m_ble_status.sending = 0;
				return;
			}
			
			timeout_count++;
		}
}

#ifdef _NRF52_HW_1_2_ADC_SPI_WORKAROUND_
extern const nrf_drv_spi_t spi_pah8002;
extern volatile uint8_t gc_adc_battery_flag;
#endif /* _NRF52_HW_1_2_ADC_SPI_WORKAROUND_ */

extern volatile uint8_t motion_hr, sdnn_hr;
extern volatile int trustLevel;
extern volatile uint8_t _pah8002_no_fifo_int_flag;

extern volatile uint8_t pahr_mode;
volatile static uint8_t hr_sleep_status;
int32_t gc_hr_sleep_process()
{
	static uint32_t on_off_time;
	//printf("\r\n ************ HR sleep ****************");
	if ( pahr_mode == TOUCH_MODE)
	{
		//printf("\r\n touch mode !!!!!!!!!!");
		hr_sleep_status = GC_HR_SLEEP_STAT_NONE;
		on_off_time = 0;
		
		return true;
	}	

	//printf("\r\n **************** pahr_mode: %d, no_int_flag %d sleep status %d (count %d)", 
			//							pahr_mode, _pah8002_no_fifo_int_flag, hr_sleep_status, __count);
	if ( pahr_mode != NORMAL_MODE && pahr_mode != SUSPEND_MODE)
		return true;
	
	if ( hr_sleep_status == GC_HR_SLEEP_STAT_NONE )
	{
		//pah8002_enter_normal_mode();
		hr_sleep_status = GC_HR_SLEEP_STAT_ON;
		
		on_off_time = __count;
		
		printf("\r\n** [NONE->ON] count %d", on_off_time);
		printf("\r\n %04d/%02d/%02d %02d:%02d",stdTime.year,stdTime.month,stdTime.day,stdTime.hour,stdTime.minute);
	}
	else if (hr_sleep_status == GC_HR_SLEEP_STAT_ON && op_mode == SLEEP_MODE)
	{
		if (__count >= on_off_time + (20 * GC_HR_SLEEP_ON_DURATION) )
		{
			pah8002_enter_suspend_mode();
			hr_sleep_status = GC_HR_SLEEP_STAT_OFF;
			
			on_off_time = __count;
			
			printf("\r\n** [ON->OFF] count %d", on_off_time);
			printf("\r\n %04d/%02d/%02d %02d:%02d",stdTime.year,stdTime.month,stdTime.day,stdTime.hour,stdTime.minute);
		}
	}
	else if (hr_sleep_status == GC_HR_SLEEP_STAT_OFF && op_mode == SLEEP_MODE)
	{
		if (__count >= on_off_time + (20 * GC_HR_SLEEP_OFF_DURATION) )
		{
			pah8002_enter_normal_mode();
			hr_sleep_status = GC_HR_SLEEP_STAT_ON;
			
			on_off_time = __count;
			
			printf("\r\n** [OFF->ON] count %d", on_off_time);
			printf("\r\n %04d/%02d/%02d %02d:%02d",stdTime.year,stdTime.month,stdTime.day,stdTime.hour,stdTime.minute);
		}
	}

		
	return true; 
}

void into_sleep_mode(unsigned char go_sleep)
{
	if(go_sleep)
	{
		//gc_spi_pah8002_init();
		cmd_poweroff(0,0);
	}
}

void into_wakeup_mode(unsigned char go_sleep)
{
	unsigned int  cal_1=0,cal_2=0;
	//int week_day, save_time=0, save_cal=0,save_step=0, recall_slp=0 ,i =0;
	int save_time=0, save_cal=0,save_step=0, recall_slp=0 ,i =0;
	
	_GC_TBL_STEP				_tbl_step;
	_GC_TBL_CALORIE	_tbl_kcal;
	_GC_POWER_OFF_INFO_	slp_save;
	uint8_t req_data[GC_FLASH_PAGE_DATA_SIZE]; 
	uint8_t __resp[GC_FLASH_PAGE_SIZE];
	
	memset(req_data,0,sizeof(GC_FLASH_PAGE_SIZE));
	memset(__resp,0,sizeof(GC_FLASH_PAGE_SIZE));
	
	get_time( &time, 1);
	convert_gmt_to_display_time(&time, &slp_end_time);
		
	
	
	for(i=0;i<=5;i++)
	{
		
		recall_slp = gc_flash_page_read_data(GC_FLASH_TBL_TYPE_POWER_OFF,1,__resp);
		nrf_delay_ms(10); /* FIXME */
		
		if(recall_slp)
		{
			i=6;
		}
	}
	printf("\r\n flah check!!!%s",__resp);
	memcpy(&slp_save, __resp, sizeof(slp_save));
	//slp_save = (_GC_POWER_OFF_INFO_ *)__resp;
	printf("\r\n %d-%d-%d %d:%d;%d", 
							slp_save.sleep_start_year, slp_save.sleep_start_month, 
							slp_save.sleep_start_day, slp_save.sleep_start_hour, slp_save.sleep_start_minute, slp_save.sleep_start_second);
		
	printf("\r\n %d setps \r\n %d cal \r\n %d cal \r\n %d cal \r\n %d cal \r\n ", 
							slp_save.step, slp_save.step_cal, slp_save.activty_cal, slp_save.sleep_cal, slp_save.coach_cal);
	printf("\r\n slp_start at %02d %02d:%02d",slp_save.sleep_start_day,slp_save.sleep_start_hour,slp_save.sleep_start_minute);
	printf("\r\n slp_end at %02d %02d:%02d",slp_end_time.day,slp_end_time.hour,slp_end_time.minute);
	//아래 if문 slp_save.sleep_start_year가 부호비트가 없는 변수인데 왜...?
	if(slp_save.sleep_start_year != -1 && recall_slp && slp_save.sleep_start_year > 2010 && slp_end_time.year > 2010)
	{
		g_total_daystep = slp_save.step;

		if(slp_save.sleep_start_day!=slp_end_time.day) // 절전 모드 돌입일자와 다른 날짜에 절전 모드 종료
		{
			cal_1 = (unsigned int)(1440-(slp_save.sleep_start_hour*60+slp_save.sleep_start_minute))*((1.1 * person_info.weight * 2 *30.0F/60.0F * 0.0175F)*(10.0F/9.0F)*1000.0F);
			kcal.step_cal += cal_1;
			save_time = (get_GMT(slp_save.sleep_start_year, slp_save.sleep_start_month, slp_save.sleep_start_day, slp_save.sleep_start_hour, slp_save.sleep_start_minute, slp_save.sleep_start_second)/(86400))*1440;
			
			_tbl_step.date	= save_time;
			_tbl_step.step	= g_total_daystep;
			memcpy(req_data, &_tbl_step, sizeof(_GC_TBL_STEP));
					
			for(i=0;i<=5;i++)
			{
				save_step = gc_flash_page_write_data(GC_FLASH_TBL_TYPE_STEP, req_data);
				nrf_delay_ms(10); /* FIXME */
				
				if(save_step)
				{
					i=6;
				}
			}
			step.display_step = 0;
			g_total_daystep = 0;
			
			_tbl_kcal.date     = save_time;
			_tbl_kcal.daily		= slp_save.step_cal +cal_1;
			_tbl_kcal.sports	= slp_save.activty_cal;
			_tbl_kcal.sleep		= slp_save.sleep_cal;
			_tbl_kcal.coach	= slp_save.coach_cal;

			memcpy(req_data, &_tbl_kcal, sizeof(_GC_TBL_CALORIE));
			
			for(i=0;i<=5;i++)
			{
				save_cal = gc_flash_page_write_data(GC_FLASH_TBL_TYPE_KCAL, req_data);
				nrf_delay_ms(10); /* FIXME */
				
				if(save_cal)
				{
					i=6;
				}
			}
			cal_2 =  (unsigned int)((slp_end_time.hour*60+slp_end_time.minute))*((1.1 * person_info.weight * 2 *30.0F/60.0F * 0.0175F)*(10.0F/9.0F)*1000.0F);
			printf("\r\n slp_end_time= %d",slp_end_time.hour);
			printf("\r\n minute= %d",slp_end_time.minute);
			printf("\r\n weight= %d",person_info.weight);
			printf("\r\n height= %d",person_info.height);
			printf("\r\n cal21= %d",cal_2);
			kcal.step_cal=cal_2 ;
		}
		else // 절전 모드 돌입일자와 같은  날짜에 절전 모드 종료
		{
			cal_2 =  (unsigned int)((slp_end_time.hour*60+slp_end_time.minute)-(slp_save.sleep_start_hour*60+slp_save.sleep_start_minute))*((1.1 * person_info.weight * 2 *30.0F/60.0F * 0.0175F)*(10.0F/9.0F)*1000.0F);
			printf("\r\n slp_end_time= %d",slp_end_time.hour);
			printf("\r\n minute= %d",slp_end_time.minute);
			printf("\r\n weight= %d",person_info.weight);
			printf("\r\n height= %d",person_info.height);
			printf("\r\n cal22 = %d",cal_2);
			
			kcal.step_cal = slp_save.step_cal + cal_2;
			kcal.activty_cal = slp_save.activty_cal;
			kcal.sleep_cal		= slp_save.sleep_cal;
			kcal.coach_cal	= slp_save.coach_cal;
			act_info.act_calorie = slp_save.activty_cal;
		}
	}
	
	//SPI_FLASH_SectorErase(0X6A000);
	//		week_day = (((get_GMT(slp_end_time.year,slp_end_time.month,slp_end_time.day,slp_end_time.hour,slp_end_time.minute,slp_end_time.second)/(86400)))%7)-1;
	//day_of_week = (week_day > 0 ) ? week_day : week_day+6;
}

void recal_after_reset(void)
{
	int week_day,recall_slp=0 ,i =0;
	
	_GC_DATA_BRFOR_RESET	reset_save;
	uint8_t recall_data[GC_FLASH_PAGE_SIZE]; 
		
	memset(recall_data,0,sizeof(GC_FLASH_PAGE_SIZE));
		
	get_time( &time, 1);
	convert_gmt_to_display_time(&time, &reset_end_time);
		
	
		//uint8_t		r_page_buf[GC_FLASH_PAGE_SIZE];
	//memset(r_page_buf, 0, GC_FLASH_PAGE_SIZE);
	//gc_flash_read(GC_FLASH_PAGE_RESET_DATA, r_page_buf );
		
//		_GC_DATA_BRFOR_RESET	rs_data1;
//	memcpy(&rs_data1, r_page_buf, sizeof(_GC_DATA_BRFOR_RESET));
	
	
//	printf("\r\n %d,%d",rs_data1.resert_start_year,rs_data1.resert_start_year);
	
	
	
	/*
	for(i=0;i<=5;i++)
	{
		
		recall_slp = gc_flash_page_read_data(GC_FLASH_TBL_TYPE_RESET_DATA,1,recall_data);
		nrf_delay_ms(10); //FIXME 
		
		if(recall_slp)
		{
			i=6;
		}
	}*/
	 gc_flash_read(GC_FLASH_PAGE_RESET_DATA, recall_data );
	//#ifdef DEBUG_PRINT
	printf("\r\n flah check!!!%x",recall_data);
	//#endif
	memcpy(&reset_save, recall_data, sizeof(_GC_DATA_BRFOR_RESET));
	//slp_save = (_GC_POWER_OFF_INFO_ *)__resp;
	//#ifdef DEBUG_PRINT
	printf("\r\n %d-%d-%d %d:%d;%d", 
							reset_save.resert_start_year, reset_save.resert_start_month, 
							reset_save.resert_start_day, reset_save.resert_start_hour, reset_save.resert_start_minute, reset_save.resert_start_second);
		
	printf("\r\n %d setps \r\n %d cal \r\n %d cal \r\n %d cal \r\n %d cal \r\n ", 
							reset_save.step, reset_save.step_cal, reset_save.activty_cal, reset_save.sleep_cal, reset_save.coach_cal);
	printf("\r\n reset_start at %02d %02d:%02d",reset_save.resert_start_day,reset_save.resert_start_hour,reset_save.resert_start_minute);
	printf("\r\n reset_end at %02d %02d:%02d",reset_end_time.day,reset_end_time.hour,reset_end_time.minute);
//	printf("\r\n today? GMT%d",  (get_GMT(time.year,  time.month,  time.day,  time.hour,  time.minute, time.second )/(60)));
	//#endif
	if(reset_save.resert_start_day == reset_end_time.day)
	{
		printf("\r\n %s","[reset]");
		printf("\r\n op_mode : %d",op_mode);
		g_total_daystep = reset_save.step;
		kcal.step_cal = reset_save.step_cal;
		kcal.activty_cal = reset_save.activty_cal;
		kcal.sleep_cal		= reset_save.sleep_cal;
		kcal.coach_cal	= reset_save.coach_cal;
		op_mode = (reset_save.op_mode == COACH_MODE || reset_save.op_mode == STRESS_MODE) ? DAILY_MODE : reset_save.op_mode;
		printf("\r\n op_mode : %d",op_mode);
		slp_info.slp_time = reset_save.slp_time;
		slp_info.awake_count = reset_save.awake_count;
		slp_info.rolled_count =reset_save.rolled_count;
		act_info.weak_count = reset_save.weak_count;
		act_info.middle_count =reset_save.middle_count;
		act_info.strong_count =reset_save.strong_count;
		act_info.danger_count = reset_save.danger_count;
		act_info.act_time = reset_save.act_time;
		printf("\r\n %d,%d",act_info.act_time,reset_save.act_time);
		HR_info.HR_max = reset_save.HR_max;
		HR_info.HR_min = reset_save.HR_min;
		HR_info.HR_avg = reset_save.HR_avg;
		
		is_info_recall =1;
		is_recall_act = 1;
		is_recall =1;
	}
	//SPI_FLASH_SectorErase(0X12000);
	//gc_protocol_ctrl_pah8002(op_mode, 1);
	
	//규창 171031 rtc를 안쓰고 플래시를 쓴다라.... 시간도 리셋이나 펌업 이후 오락가락 한다...
	week_day = (((get_GMT(reset_end_time.year,reset_end_time.month,reset_end_time.day,reset_end_time.hour,reset_end_time.minute,reset_end_time.second)/(86400)))%7)-1;
	
	//규창 171031 반면 rtc는 정확... 뭐지...?
	//week_day = (((get_GMT(time.year,  time.month,  time.day,  time.hour,  time.minute, time.second )/(86400)))%7)-1;
	day_of_week = (week_day > 0 ) ? week_day : week_day+6;
	
	//규창 171018 
	//rtc_set_time(reset_end_time.year,reset_end_time.month,reset_end_time.day,reset_end_time.hour,reset_end_time.minute,reset_end_time.second);
}

//규창 임시 전역변수들 기압, 가속
float tempResult, pressureResult , altitudeResult;
struct bma2x2_accel_data sample_xyz;
//ppg전용
struct bma2x2_accel_data hr_xyz;
//규창 심박값 초기화 플래그
//extern bool sendinitVarFlag;

static void sensor_timer_handler(void * p_context)
{
	
	//printf("\n\r sensor_timer_handler start at %d",NRF_RTC1->COUNTER); //1count == 1/32768 sec
  //printf("\n\rSAADC???? %d", (int16_t)(gc_saadc));
	extern volatile unsigned short step_count;
	//규창 배터리 읽는 타이머가 생겨서 필요없음
	//int battery_read_period;
	
	//static char HR_reset_count_trust =0;

//#ifdef POWER_SAVE_MODE
		//printf("\r\n POWER_SAVE_MODE????");
		//into_sleep_mode(go_sleep);
//#endif /*POWER_SAVE_MODE*/
	
   //규창 전임자.... 최고 최저 평균 안정 심박 뽑아내는 연산 루틴...
	get_realtime_HR(__count, g_hr);
	avg_2sec_hr+=g_hr;
	if(g_hr)
	{
		avg_2sec_count++;
	}
	if(avg_2sec_count)
	{
		f_hr=avg_2sec_hr/avg_2sec_count;
	}
	else
	{
		f_hr=0;
	}
		
	if(__count%40==0 && __count != 0)
	{
		calorie (step_count,f_hr,op_mode,__count);
		avg_2sec_count = 0;
		avg_2sec_hr =0;
	}
	if(__count%1200==0)
	{
		activity_intensity(op_mode);
	}
	
	__count++;

#ifndef _FIT_ETRI_TEST_
	OLED_display(__count);
#endif 
	if(is_recall)
	{
		unsigned char recall_mode[4]= {0x00,0x00,0x01,0x00};
		if (op_mode == ACT_MODE)
		{
			recall_mode[0] = 0x0B;
		}
		else if (op_mode == SLEEP_MODE)
		{
			recall_mode[0] = 0x0C;
		}
			
		printf("\r\n data recall");
		ble_receive_handler(recall_mode,4);
		is_recall = 0;
	}
	if(op_mode==STRESS_MODE)
	{
		stress_timmer++;
		if(stress_timmer>2500)
		{
			op_mode = ((op_mode == STRESS_MODE) || (op_mode ==  COACH_MODE))? DAILY_MODE : op_mode;
			gc_protocol_ctrl_pah8002(op_mode, 0);
			m_ble_status.send_enable = 0;
			g_total_tx_busy_count=0;
			send_rawdata = 2;
		}
	}
	else
	{
		stress_timmer =0;
	}
	if(is_charging == 0 && is_uart == 1)
  {
		nrf_gpio_pin_clear(GC_GPIO_DEBUG_ON);
		is_uart = 0;
	}
	if(is_charging == 1 && is_uart == 0)
  {
		nrf_gpio_pin_set(GC_GPIO_DEBUG_ON);
		app_uart_flush();
		is_uart = 1;
	}
	bma2x2_read_accel_xyz(&sample_xyz);/* Read the accel XYZ data*/
	
	hr_xyz = sample_xyz;
	update_accelerometer_buffer(&hr_xyz);
	
	//printf("\r\nsample_xyz = %d %d %d",sample_xyz.x , sample_xyz.y, sample_xyz.z );
#ifndef _FIT_ETRI_TEST_ 
	//	printf("\n\r greencom_step_process start at %d",NRF_RTC1->COUNTER);  //1count == 1/32768 sec
	greencom_step_process(__count,sample_xyz.x,sample_xyz.y,sample_xyz.z);
//	printf("\n\r greencom_step_process finish at %d",NRF_RTC1->COUNTER);  //1count == 1/32768 sec
	
	//규창 디스플레이 스윙 온 루틴 
	/*if(OLED_switch == 0 && Detect_Watch_Motion(sample_xyz.x/50,sample_xyz.y/50,sample_xyz.z/50))
	{
		//if (OLED_switch == 0){
			
		  //tuch_count = display_page_step;
			//OLED_power_count=OLED_ON_count_time;
			
		 //OLED_switch = 1;
			//OLED_Power(OLED_switch);
		//}
		if (tuch_count == display_page_clock){
			tuch_count++;
		}
		else {
			tuch_count=display_page_clock;
	  }
}*/
	//printf("\r\n hr_mode : %d ",pahr_mode);
	
#endif	
	if(m_ble_status.send_enable == 0)
	{
		send_raw_bf[8]=0;
		send_raw_bf[16]=0;
	}
		
	BLE_send_rawdata(sample_xyz.x,sample_xyz.y,sample_xyz.z,send_rawdata);
	
	
	//규창 100ms마다 한번씩...
	if (__count % 2 == 0){ 
		BLE_send_featuredata();
	}
	//BLE_send_rawdata_test(sample_xyz.x,sample_xyz.y,sample_xyz.z,send_rawdata);
	//BLE_send_ADC(sample_xyz.x,sample_xyz.y,sample_xyz.z,send_rawdata);
	

	//규창 센서타이머는 50ms주기로 돌고 60000이 1분이니, 1200 보다 커질때 한번 기압값 출력
	// 규창 171027 기압데이터는 1초당 1개씩
	__AN520count++;
	if(__AN520count %20 == 0){
		//AN520..........
		//규창 기압센서값 1분 단위로 받기 위해 여기에 임시로 선언하여 받아보자.
		//float tempResult, pressureResult;
		AN520_Result(&tempResult, &pressureResult, &altitudeResult);
		
		//printf("\r\n1min Timer An520 Result [%f], [%f] [%f]", tempResult, pressureResult, altitudeResult);
		
		//규창 AN520count값이 1분 됬으니 0으로 바꾸자
		__AN520count = 0;
	}
						
	#define ht_interval 2
	g_acc_move_signal = 0;
	//규창 아래는 심박 값 읽어오는 루틴으로 얼추 100ms주기로pah8002_task()를 돌려 심박값을 읽어오도록 플래그가 설정되어있음
	if (__count % 2 == 0)
	{		
		if (hr_processing)
		{
			//printf("\r\n [__count : %d]",__count/20);
			//규창 정확하진 않지만 심박센서가 BPM값생겼다고 m4에게 알리는 인터럽트는 1초마다 발생중
			if(_pah8002_interrupt == 1)
			{
				pah8002_task(); 
				g_hr = motion_hr;
				if(g_hr){
					epoch_hr++;
					HR_sum += g_hr;
					
				//	printf("\r\n HR_sum[%d] [%d]",HR_sum, HR_info.HR_min);
					 
					
					
					HR_info.HR_avg=(unsigned char)(HR_sum/epoch_hr);
					HR_info.HR_max = (HR_info.HR_max<g_hr) ? g_hr : HR_info.HR_max;
					HR_info.HR_min = (HR_info.HR_min>g_hr) ? g_hr : HR_info.HR_min;
					
				//	printf("\r\n AVG HR[%d]  %d   %d",HR_info.HR_avg,HR_info.HR_min, HR_info.HR_max);
								

					
					if(op_mode == SLEEP_MODE && g_hr)
					{
						if(HR_info.HR_sta)
						{
								HR_info.HR_sta = (HR_info.HR_sta > g_hr && g_hr >= MIN_HR_VALUE) ? g_hr : HR_info.HR_sta ;
						}
						else
						{
							HR_info.HR_sta = (g_hr >= MIN_HR_VALUE) ? g_hr : 0;
						}
						//HR_info.HR_sta = HR_sta ;
						//printf("\r\n HR_info.HR_sta = %d, HR_sta = %d",HR_info.HR_sta ,HR_sta);
						printf("\r\n HR_info.HR_sta = %d",HR_info.HR_sta);
						//HR_info.HR_sta = (HR_info.HR_sta < HR_sta) ? HR_info.HR_sta : HR_sta  ; // 2016.11.28 hm_lee	최저 안정 심박수 유지
						//printf("\r\n HR_info.HR_sta = %d",HR_info.HR_sta );
						if(slp_info.slp_time >= 3600) // 수면 시간 이 2시간 이상일 경우 안정 심박수 갱신
						{
							//HR_info.HR_sta = (HR_sta >= MIN_HR_VALUE) ? HR_sta : HR_info.HR_sta ; 
							HR_info.HR_sta = (HR_info.HR_sta > g_hr && g_hr >= MIN_HR_VALUE) ? g_hr : HR_info.HR_sta ;
						}
					}
				}
				//printf("\r\nHR[%d]  trustlevel[%d] [%d] [%f] [%f] [%d]", motion_hr, trustLevel, pahr_mode, (g_hr - 64) / (220.0f - person_info.age - 64)*100, (5 * (220.0 - person_info.age - 64))+10*64 , person_info.age);
			}
			
		//gc_hr_sleep_process();
		}
		//규창 심박값 초기화 플래그 함께 보도록
		else //if (hr_processing == 0 && sendinitVarFlag == true)
		{
			//printf("\r\n reset g_hr var");
			g_hr = 0;
			pre_g_hr =0;
			motion_hr = 0;
		}

		
		is_charging = nrf_gpio_pin_read(GC_GPIO_BAT_CHARGE_DETECT) ;
	
	}
	//printf("\n\r sensor_timer_handler finish at %d",NRF_RTC1->COUNTER);  //1count == 1/32768 sec

}


#if 0 
static void pahr_timer_handler(void * p_context)
{
	pah8002_task(); 
	
	g_hr = get_pah8002_hr();
	
	gc_hr_sleep_process();
	
#ifndef _NRF52_HW_1_2_ADC_SPI_WORKAROUND_		
//	gc_battery_sense();
#endif
		
	is_charging = nrf_gpio_pin_read(GC_GPIO_BAT_CHARGE_DETECT) ;
}
#endif

/**@brief Function for handling the security request timer time-out.
 *
 * @details This function is called each time the security request timer expires.
 *
 * @param[in] p_context  Pointer used for passing context information from the
 *                       app_start_timer() call to the time-out handler.
 */
static void sec_req_timeout_handler(void * p_context)
{
    
    uint32_t             err_code;
    dm_security_status_t status;

    if (m_peer_handle.connection_id != DM_INVALID_ID)
    {
        err_code = dm_security_status_req(&m_peer_handle, &status);
			#if 0 /* hkim 2016.11.5 */ /* FIXME */
        APP_ERROR_CHECK(err_code);
			#endif 

        // If the link is still not secured by the peer, initiate security procedure.
        if (status == NOT_ENCRYPTED)
        {
						SEGGER_RTT_printf(0,"\r\n ** Pairing request");
					//규창 암호화 통신을 하게 된다면???
						if (gc_conn_os == GC_CONNECTED_OS_IOS)
						{
							err_code = dm_security_setup_req(&m_peer_handle);
							APP_ERROR_CHECK(err_code);
						}
        }
    }
}

/**@brief   Function for Timer initialization.
 *
 * @details Initializes the timer module.
 */
static void timers_init(void)
{
		uint32_t err_code;
	

    // Create debug shell  timer
    err_code = app_timer_create(&m_shell_timer_id,APP_TIMER_MODE_REPEATED,sensor_timer_handler);
		APP_ERROR_CHECK(err_code);
	
	//saadc 타이머 (배터리) 등록
  	err_code = app_timer_create(&m_battery_timer_id,APP_TIMER_MODE_REPEATED,battery_level_meas_timeout_handler);
		APP_ERROR_CHECK(err_code);
	
	//PPG에 ACC값 넣는 타이머 등록
  	//err_code = app_timer_create(&m_accppg_timer_id,APP_TIMER_MODE_REPEATED, accforppg_handler);
		//APP_ERROR_CHECK(err_code);
	
	
	
#ifndef _FIT_ETRI_TEST_ 
	  err_code = app_timer_create(&m_sec_req_timer_id, APP_TIMER_MODE_SINGLE_SHOT, sec_req_timeout_handler);
    APP_ERROR_CHECK(err_code);
#endif
		//err_code = app_timer_create(&m_pahr_timer_id,APP_TIMER_MODE_REPEATED,pahr_timer_handler);
    //APP_ERROR_CHECK(err_code);
}

static void application_timers_start(void)
{
   uint32_t err_code;

	// debug shell
	err_code = app_timer_start(m_shell_timer_id, SENSOR_TIMER_INTERVAL, NULL);
  APP_ERROR_CHECK(err_code);
	
	//배터리 10분주기로 읽도록 오픈
		err_code = app_timer_start(m_battery_timer_id, BATTERY_LEVEL_MEAS_INTERVAL, NULL);
		APP_ERROR_CHECK(err_code);
	
	//가속센서 타이머 시작함수 33ms
	//err_code = app_timer_start(m_accppg_timer_id, ACC_FOR_INTERVAL, NULL);
	//APP_ERROR_CHECK(err_code);
	
	//err_code = app_timer_start(m_pahr_timer_id, PAHR_TIMER_INTERVAL, NULL);
  //APP_ERROR_CHECK(err_code);
}
#endif /* GREENCOMM_NUS app_timer hkim */


#ifdef GC_NRF52_TWI
/**
 * @brief UART initialization.
 */
void gc_twi_init (void)
{
    ret_code_t err_code;
    
    const nrf_drv_twi_config_t twi_master1_config = {
       .scl                			= ARDUINO_SCL_PIN,
       .sda                		= ARDUINO_SDA_PIN,
       .frequency          = NRF_TWI_FREQ_400K,
#ifdef GC_COACH_FIT_PCB_PAHR_IF_TWI
			 .interrupt_priority = APP_IRQ_PRIORITY_LOWEST
#else
       .interrupt_priority = APP_IRQ_PRIORITY_LOW
#endif
    };
   
    err_code = nrf_drv_twi_init(&m_twi_master_1, &twi_master1_config, NULL, NULL);
#ifndef _FIT_ETRI_TEST_ 
		if (err_code == NRF_SUCCESS)
			printf("\r\n I2C sensor init. done");
		else
			printf("\r\n I2C init. fail");
#endif
    APP_ERROR_CHECK(err_code);
		
    nrf_drv_twi_enable(&m_twi_master_1);
}
#endif /* GC_NRF52_TWI */

#ifdef GC_NRF52_TWI


void gpiote_acc_intr_handler(nrf_drv_gpiote_pin_t pin, nrf_gpiote_polarity_t action)
{
	g_acc_move_signal = 1;
	
	if(pin == GC_GPIO_PIN_ACC_INTR_1  && action == NRF_GPIOTE_POLARITY_LOTOHI )
  {                                                               
		//printf("\r\n Acc INTR_1");                     
  }
	if(pin == GC_GPIO_PIN_ACC_INTR_2  && action == NRF_GPIOTE_POLARITY_LOTOHI )
  {                                                               
#ifndef _FIT_ETRI_TEST_ 
//		printf("\r\n Acc INTR_2");
#endif
		tuch_count++;
  }
}
/**
 * @brief acclerator sensor initialization
 * acc_intr_1 : move signal interrupt
 * acc_intr_2 : double tab interrupt
 */
int32_t accelerator_sensor_init(void)
{
	int32_t __result=0;
	
	nrf_gpio_cfg_input(GC_GPIO_PIN_ACC_INTR_1, NRF_GPIO_PIN_NOPULL);
	nrf_gpio_cfg_input(GC_GPIO_PIN_ACC_INTR_2, NRF_GPIO_PIN_NOPULL); /* FIXME 2016.8.26 */
	
	/* handle accelerator interrupt - move interrupt*/
	nrf_drv_gpiote_in_config_t acc_intr_1 = GPIOTE_CONFIG_IN_SENSE_LOTOHI(true);	
	//acc_intr_1.pull = NRF_GPIO_PIN_PULLUP;
	
	if ( nrf_drv_gpiote_in_init(GC_GPIO_PIN_ACC_INTR_1, &acc_intr_1, gpiote_acc_intr_handler) != NRF_SUCCESS )
		printf("\r\n GPIOTE,(GPIO %d) input init. fail", GC_GPIO_PIN_ACC_INTR_1);
	
	nrf_drv_gpiote_in_event_enable(GC_GPIO_PIN_ACC_INTR_1, true);
	
	
	/* handle accelerator interrupt - double tab interrupt*/
	nrf_drv_gpiote_in_config_t acc_intr_2 = GPIOTE_CONFIG_IN_SENSE_LOTOHI(true);	
	//acc_intr_1.pull = NRF_GPIO_PIN_PULLUP;
	
	if ( nrf_drv_gpiote_in_init(GC_GPIO_PIN_ACC_INTR_2, &acc_intr_2, gpiote_acc_intr_handler) != NRF_SUCCESS )
		printf("\r\n GPIOTE,(GPIO %d) input init. fail", GC_GPIO_PIN_ACC_INTR_1);
	
	nrf_drv_gpiote_in_event_enable(GC_GPIO_PIN_ACC_INTR_2, true);

	/* initialize BMA255 accelerator sensor */
	
	__result = bma2x2_data_readout_template(); /* initialize BOSCH accleration sensor */
#ifndef _FIT_ETRI_TEST_ 
	if (__result == NRF_SUCCESS) 
		printf("\r\n BMA255 init. done");
	else
		printf("\r\n BMA255 init. fail(result %d)", __result);
#endif
	
	return __result;
}
#endif /* GC_NRF52_TWI */
/**
 * @brief OLED initialization
 */
void oled_init(void)
{
	Init_OLED_PORT();
	OLED_Power(true);
	OLED_Clear();
}

void gc_dev_gpio_set()
{
	/* accelerator */
	
	
	/* OLED */
#ifdef GC_COACH_FIT_PCB
	nrf_gpio_cfg_output(OLED_RESET_PIN);

	nrf_gpio_pin_clear(OLED_RESET_PIN);
	nrf_delay_ms(100);
 	nrf_gpio_pin_set(OLED_RESET_PIN);
	
#elif defined GC_COACH_FIT_DK52
	nrf_gpio_cfg_output(OLED_POWER);
	nrf_gpio_cfg_output(OLED_RESET_PIN);
	
	nrf_gpio_pin_clear(OLED_POWER);
	nrf_delay_ms(100);
	nrf_gpio_pin_clear(OLED_RESET_PIN);
	nrf_delay_ms(100);
	
 	nrf_gpio_pin_set(OLED_POWER);
	nrf_delay_ms(100);
 	nrf_gpio_pin_set(OLED_RESET_PIN);
#else
#endif
	
	/* HR */
	/*nrf_gpio_cfg_output(SENSOR_POWER);

	nrf_gpio_pin_clear(SENSOR_POWER);
	nrf_delay_ms(100);
	nrf_gpio_pin_set(SENSOR_POWER);*/
	
	nrf_gpio_cfg_output(GC_SPI_PAH8002_PIN_RST);
	nrf_gpio_pin_clear(GC_SPI_PAH8002_PIN_RST);
	nrf_delay_ms(100);
 	nrf_gpio_pin_set(GC_SPI_PAH8002_PIN_RST); // High -> release reset,  Low -> generate reset 
	
	nrf_gpio_cfg_input(GC_SPI_PAH8002_PIN_INTR, NRF_GPIO_PIN_PULLUP);
	//nrf_gpio_cfg_input(GC_SPI_PAH8002_PIN_INTR, NRF_GPIO_PIN_NOPULL);
	
	/* motor */
	nrf_gpio_cfg_output(GC_GPIO_MOTOR_ENABLE);
	nrf_gpio_pin_clear(GC_GPIO_MOTOR_ENABLE);
	
	/* battery charge detect */
	nrf_gpio_cfg_input(GC_GPIO_BAT_CHARGE_DETECT, NRF_GPIO_PIN_NOPULL);
	
	/* cap touch sensor */
	nrf_gpio_cfg_input(GC_CAPSENSE_PIN, NRF_GPIO_PIN_NOPULL); 
	
	/* debug on through battery charge device */
	nrf_gpio_cfg_output(GC_GPIO_DEBUG_ON);
	
	/* LFCLK */
	nrf_gpio_cfg_input(PIN_CLK_32768, NRF_GPIO_PIN_NOPULL);
}

void gc_ppi_gpiote_init(void)
{
	
	uint32_t err_code; /* hkim 2016.7.7 */
#ifndef _FIT_ETRI_TEST_ 	
	err_code = nrf_drv_ppi_init();
	if (err_code == NRF_SUCCESS)
		printf("\r\n PPI init. done");
	else
		printf("\r\n PPI init. fail(code %d)", err_code);
#else
	nrf_drv_ppi_init();
#endif	

#ifndef _FIT_ETRI_TEST_ 	
	err_code = nrf_drv_gpiote_init();
	if (err_code == NRF_SUCCESS)
		printf("\r\n GPIOTE init. done");
	else
		printf("\r\n GPIOTE init. fail(code %d)", err_code);
#else
	nrf_drv_gpiote_init();
#endif

}

void  recall_GMT_offset(void)
{
	unsigned char offset_data[GC_FLASH_PAGE_SIZE];
		
	
	//규창 171031 플래시 주소 읽는 함수 구현이 다소 이상하다..
	//주소값이 16페이지를 기준으로 와리가리 할 가능성이 매우 높음
	gc_flash_read(GC_FLASH_PAGE_GMT_OFFSET, offset_data );
	//gc_flash_page_read_data(GC_FLASH_TBL_TYPE_GMT_OFFSET,1,offset_data);
	printf("\r\n offset_data : %d,%d",offset_data[0],offset_data[1]);
	gmt_offset = offset_data[0] |  offset_data[1]<<8;
	printf("\r\n gmt_offset : %d",gmt_offset);
  ////extern volatile short tmp_gmt_offset;
	////tmp_gmt_offset	= gmt_offset;
}


void gc_dev_initialize(void)
{
	int32_t __result = GC_RET_SUCCESS;

	gc_ppi_gpiote_init();

	/* initialize HR interface */
//#ifdef GC_NRF52_SPI_HR
	gc_spi_pah8002_init(); /* hkim 2016.7.7 */
//#elif defined GC_COACH_FIT_PCB_PAHR_IF_TWI
//	gc_twi_pah8002_init();
//#else
//#endif 

#ifdef GC_NRF52_SPI_FLASH
	gc_spi_flash_init(); 
#elif defined GC_NRF51_SPI_FLASH
	init_SPI_Flash_PORT();
#endif 

#ifdef GC_NRF52_TWI
	//규창 
	//twi_init();
	//170512 규창 an520 업으로 인해 일시 제거
	gc_twi_init();  
	/* initialize accelerator sensor */
#ifdef GC_COACH_FIT_PCB
	__result += accelerator_sensor_init(); 
#endif 
	/* initialize OLED */
#ifndef _FIT_ETRI_TEST_
	oled_init(); 
#endif 
#endif /* GC_NRF52_TWI */	

#ifdef GC_COACH_FIT_PCB
#ifndef _FIT_ETRI_TEST_
//170512 규창 an520 업으로 인해 일시 제거
	//init_an520();
	
	
#endif
	/* initialize RTC */
	init_rtc();
#endif 

	//init_capsense();  //temporaly double tab instead
	init_battery_sense_event();
	
	/* find flash empty page */
	find_flash_page_write_position_at_boot(); 
	recall_GMT_offset();
	check_saved_info();
  if (__result == 	NRF_SUCCESS) 
		printf("\r\n Device init. done");
	else
		printf("\r\n Dev init fail");
}

#ifdef GC_NRF52_PAHR /* code by pixart*/
volatile uint32_t tick_cnt;
/*****************************************************************************
 * Public functions
 ****************************************************************************/
/**
 * @brief	SysTick Interrupt Handler
 * @return	Nothing
 * @note	Systick interrupt handler updates the button status
 */
#if 0
void SysTick_Handler(void)
{
	tick_cnt ++;
}
#endif
uint32_t get_sys_tick()
{
//	uint32_t tick_cnt;
	
	tick_cnt = NRF_RTC0->COUNTER * 250 / 8192; 
	return tick_cnt; 
}

void delay_ms(uint32_t ms)
{
	uint32_t start_cnt = get_sys_tick() ;

	while( (get_sys_tick() - start_cnt) < ms ) {}
}
#endif  /* GC_NRF52_PAHR */

/**
 * @brief Application main function.
 */

void check_saved_info(void)
{
//	uint32_t result=0;
	unsigned char SN_data[GC_FLASH_PAGE_SIZE];
	unsigned char stb_HR[GC_FLASH_PAGE_SIZE];
	unsigned char ps_info[GC_FLASH_PAGE_SIZE];
	
	//printf("\r\n gmt_offset : %s",SN_data);
	
	//규창 171101 gc_flash_page_read_data 함수는 동작은 할 것으로 생각되나... 변경
	gc_flash_read(GC_FLASH_PAGE_SN, SN_data );
//	gc_flash_page_read_data(GC_FLASH_TBL_TYPE_SN,1,SN_data);
	sprintf(BLE_SSID,"F1%s",&SN_data[2]); 
		
#ifndef _FIT_ETRI_TEST_			
			printf("\r\n Another BLE_SSID => %s",BLE_SSID );
#endif
		if (SN_data[0] == 'F' && SN_data[1] == '1')
		{
			SN_data[10]=0;	// serial number =>  10byte cut
			#ifdef DEBUG_PRINT
			nrf_gpio_pin_set(GC_GPIO_DEBUG_ON);
			#endif
			sprintf(BLE_SSID,"F1%s",&SN_data[2]); 
		
#ifndef _FIT_ETRI_TEST_			
			printf("\r\n BLE_SSID => %s",BLE_SSID );
#endif
		}
		else
		{
			nrf_gpio_pin_set(GC_GPIO_DEBUG_ON);
			printf("\r\n SN empty");
			sprintf(BLE_SSID,DEFAULT_SN); 
			
			// sn 없으면 자동적으로 product mode
			g_product_mode=true;
		}
		//규창 171101 gc_flash_page_read_data 함수는 동작은 할 것으로 생각되나... 변경
		//gc_flash_page_read_data(GC_FLASH_TBL_TYPE_STABLE_HR,1,stb_HR);
		gc_flash_read(GC_FLASH_PAGE_STABLE_HR, stb_HR );
		HR_info.HR_sta = stb_HR[0];//((stb_HR[0] > 20)&&(stb_HR[0] < 140)) ? stb_HR[0] : 60;
		#ifdef DEBUG_PRINT
		printf("\r\n HR_info.HR_sta = %d",HR_info.HR_sta);
		#endif
		//규창 171101 gc_flash_page_read_data 함수는 동작은 할 것으로 생각되나... 변경
		//gc_flash_page_read_data(GC_FLASH_TBL_TYPE_PERSINFO,1,ps_info);
		gc_flash_read(GC_FLASH_PAGE_PERSINFO, ps_info );
		#ifdef DEBUG_PRINT
		#endif
		person_info.sex = (ps_info[0] == 0xFF || ps_info[0] == 0x00) ? 1 : ps_info[0];
		person_info.age = (ps_info[2] == 0xFF|| ps_info[2] == 0x00) ? 30 : ps_info[2];
		person_info.height = (ps_info[4] == 0xFF|| ps_info[4] == 0x00) ? 160 : ps_info[4];
		person_info.weight = (ps_info[6] == 0xFF|| ps_info[6] == 0x00) ? 60 : ps_info[6];
		printf("\r\n sex = %d",person_info.sex);
		printf("\r\n age = %d",person_info.age);
		printf("\r\n height = %d",person_info.height);
		printf("\r\n weight = %d",person_info.weight);
		
}


//전역 변수 ADC측정값
//extern volatile nrf_saadc_value_t result;
#ifdef _NRF52_HW_1_2_HR_POWER_EN_
int32_t hr_power_enable(void)
{
	/*while (!hr_power_en)
	{
//#ifndef _FIT_ETRI_TEST_
		printf("\r\n waiting for 3.4V");
//#endif
		if (gc_saadc >= 650)
		{*/
			hr_power_en = 1;
	/* PAH8002 LED enable */
		  nrf_gpio_cfg_output(GC_GPIO_PIN_HR_POWER_EN);
		  nrf_gpio_pin_clear(GC_GPIO_PIN_HR_POWER_EN);
	    nrf_delay_ms(100);
			nrf_gpio_pin_set(GC_GPIO_PIN_HR_POWER_EN);
			printf("\r\n GC_GPIO_PIN_HR_POWER_EN");
			/*return true;
		}
		nrf_delay_ms(1000); 
	}*/
	return true;
}
#endif /* _NRF52_HW_1_2_HR_POWER_EN_ */


int main(void)
{
    uint32_t err_code;
    bool erase_bonds=false; /* FIXME */

		sd_power_dcdc_mode_set(NRF_POWER_DCDC_ENABLE);
		nrf_gpio_pin_set(GC_GPIO_DEBUG_ON);
    // Initialize.
	  uart_init();
		APP_TIMER_INIT(APP_TIMER_PRESCALER, APP_TIMER_OP_QUEUE_SIZE, false);
		
#ifndef _FIT_ETRI_TEST_
		printf("\n\n\r");
		printf("\n\r***************************************************");
	  printf("\r\n iBody24 Fitness");
		SEGGER_RTT_printf(0,"\r\n iBody24 Fitness");
		printf("\n\r***************************************************");	
#else
		printf("\n\n\r");
		printf("\n\r***************************************************");
	  printf("\r\n ETRI current consumption test");
		printf("\n\r***************************************************");	
#endif
	
		
	#ifdef _NRF52_HW_1_2_HR_POWER_EN_
		//규창 adc 초기화
    saadc_init();
		//규창 부팅시 오락가락 하지 않도록 딜레이처리
    nrf_delay_ms(1500);
		hr_power_enable();
		//nrf_gpio_pin_set(GC_GPIO_PIN_HR_POWER_EN);
#endif /* _NRF52_HW_1_2_HR_POWER_EN_ */
    //규창 부팅시 오락가락 하지 않도록 딜레이처리
    nrf_delay_ms(1500);
		gc_dev_gpio_set(); 
#ifdef GC_NRF52_DEV
		/* hkim 2016.6.15. 
			   device initialization should be done after BT associated logic is finished */
		gc_dev_initialize();
#endif /* GC_NRF52_DEV */

//#ifdef POWER_SAVE_MODE
			//into_wakeup_mode(go_sleep);
//#endif //POWER_SAVE_MODE

	#ifdef SAVE_BEFORE_RESET
	if(g_product_mode == false)
	{
		recal_after_reset();
	}
	#endif /*SAVE_BEFORE_RESET*/

	#ifdef GC_COACH_FIT_DK52
			buttons_leds_init(&erase_bonds);
	#endif 
    ble_stack_init();
		device_manager_init(erase_bonds);
#ifdef GC_NRF52_ANCS
		db_discovery_init();
		scheduler_init();
#endif 
		gap_params_init();
    services_init();
    advertising_init();
    conn_params_init();
		
#ifndef _FIT_ETRI_TEST_
		//printf("\r\n%02X:%02X:%02X", m_ble_addr.addr[2], m_ble_addr.addr[1], m_ble_addr.addr[0]);
#endif
		printf("\r\n mac %02X:%02X:%02X:%02X:%02X:%02X", m_ble_addr.addr[5], m_ble_addr.addr[4], m_ble_addr.addr[3],m_ble_addr.addr[2], m_ble_addr.addr[1], m_ble_addr.addr[0]);
		//printf("\r\n mac %d:%d:%d:%d:%d:%d\n", m_ble_addr.addr[5], m_ble_addr.addr[4], m_ble_addr.addr[3],m_ble_addr.addr[2], m_ble_addr.addr[1], m_ble_addr.addr[0]);

		pah8002_deinit(); 
		//gc_disable_pah8002();
		//FlashAllErase();
		//savefeatureshow();
		find_recent_feature_save_on_boot();
#ifdef GREENCOMM_NUS /* app_timer hkim 2016.6.1 */
    timers_init(); 
		application_timers_start(); //app_timer hkim 2016.6.1 */ 
#endif

    err_code = ble_advertising_start(BLE_ADV_MODE_FAST);
    APP_ERROR_CHECK(err_code);
		
		//sd_ble_gap_tx_power_set(  4 ); 
#ifdef _FIT_ETRI_TEST_
		printf("\r\nPress Enter key and type '?' to display command");
#endif
		//gc_protocol_ctrl_pah8002(ACT_MODE, 1);
    // enter low power mode
			
	AN520cmd_reset(); // reset IC	
	AN520_crc_init();
	
	//규창 171013 피쳐 생성을 위한 심박센서 상시 오픈
	pah8002_init();
	hr_processing = 1;
		//규창 파워오프에에서 회복 하면 슬립0
	 //go_sleep = 0;
	 //규창 171031 이 라인에서 알수 없는 gmt_offset 데이터 깨짐 현상으로 재 로드
	 recall_GMT_offset();
	 
	 
    for (;;)
    {
			
        power_manage();
    }
}

/* ppg센서에 들어갈 가속센서 수집 타이머 임시 제거 (33ms)
void accforppg_handler(void * p_context)
{
			//UNUSED_PARAMETER(p_context);
			//printf("\r\naccforPPG_HANDLER");
			//bma2x2_read_accel_xyz(&sample_xyz);// Read the accel XYZ data
	//#ifdef MEMS_ZERO
			//hr_xyz = sample_xyz;
			//update_accelerometer_buffer(&hr_xyz);
	//#endif
			
}*/

void __cmd_nus_ble_send(void)
{
	ble_nus_string_send(&m_nus, "abcde", 5);
}
/** 
 * @}
 */




int g_total_send_count=0;

void ble_send( uint8_t* data, uint16_t length )
{
	//	printf("\r\nble_send response");
	uint32_t err_code;

	
	
#if 1//__GREENCOM_FITNESS__
	g_total_send_count++;
#endif

	err_code = ble_nus_string_send(&m_nus, data  , length);
	if ( (err_code != NRF_ERROR_INVALID_STATE) || (err_code != NRF_SUCCESS) )
	{
		m_ble_status.sending=true; /* hkim 2016.9.26 */
		//APP_ERROR_CHECK(err_code);
	}
}


//규창 디스플레이 스윙 연산 함수
/*
int Detect_Watch_Motion(int acc_x, int acc_y, int acc_z)
{
	static short moution_count=0;
	static short delay_count=0;
	static short reset_count=0;
	int r=false;

		if(acc_z>(acc_y+5)&& acc_y>-2)
		{ 
			moution_count++;
			if(moution_count == 10 && delay_count >20)
			{
				delay_count = 0;
				r=true;
			}
		}		
		else if(acc_x<(acc_y-5)&& acc_y>-2)
	 { 
			moution_count++;
			if(moution_count == 10 && delay_count >20)
			{
				delay_count = 0;
				r=true;
			}
		}	
	 else if( acc_y < -7)
	 {
			moution_count=0;
			delay_count++;
			r=false;
		}
		else
		{
			//reset_count++;
			//if(reset_count > 20)
			//{
				//moution_count = 0;
			//	delay_count = 0;
			//}
			r=false;			
		}
	//	printf("\n\r%d",wation_moution_count );

	return r;
}*/

