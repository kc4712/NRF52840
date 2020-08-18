

#ifndef __DEFINE_H__
#define __DEFINE_H__

#define __BOARD2__		0	//	2015.05.19  board
#define __BOARD3__		1	//	2015.06.24  board0

#define		YES		1
#define		NO		0

#define		TRUE	1
#define		FALSE	0

#define		true		1
#define		false	0
	
#define 		GC_CONNECTED_OS_IOS						1
#define		GC_CONNECTED_OS_ANDROID		2
	
#define GC_RET_SUCCESS	0

#define 	PRODUCT_VERSION	"1.0.0.7"


/**@brief Compile Option ************************

* @definition _NRF52_HW_1_2_ADC_SPI_WORKAROUND_
If AIN0 is used as battery sensing, SPI device which conneced to GPIO-3~6) does not work. 
In detail, MCU try to send data through SPI, driver return busy error. 
*/

#define __GREENCOM_MINI_BT_RAWDATA_TEST__
#define _NRF52_HW_1_2_ADC_SPI_WORKAROUND_
#define _NRF52_HW_1_2_HR_POWER_EN_
//#define __GREENCOM_24H_BT_SENSOR_TEST__
//#define		_DK_BOARD_				//   DK보드에서 동작 
//#define		__USE_SPI_TERMINAL__	//	BM101의 경우 SPI로 Debug하기 위함.
//#define POWER_SAVE_MODE // 밴드 슬립(절전)모드 동작
//#define AUTO_SLEEP // 자동 수면 측정
//#define POWER_SAVE_MODE_AND_AUTO_SLEEP_PROCESS // 밴드 슬립(절전)모드와 자동 수면 측정을 판단하기 위한 과정
#define SAVE_BEFORE_RESET // 밴드 리셋시 데이터 자장 및 호출
//#define DEBUG_PRINT 
#define DEFAULT_SN "F1"
#define MIN_HR_VALUE 45
//#define HR_TEST_ONLY //hwtest가 심박 test 전용으로 동작

void reset_prepare(void);
void recall_GMT_offset(void);
#endif






