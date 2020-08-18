#include <stdio.h>
#include "boards.h"
#include "app_util_platform.h"
#include "app_uart.h"
#include "app_error.h"
#include "nrf_drv_twi.h"
#include "nrf_delay.h"
#include <math.h>


//UART buffer size. 
#define UART_TX_BUF_SIZE 256
#define UART_RX_BUF_SIZE 1

//Pins to connect shield. 
#define MS5637_I2C_SCL_PIN 26
#define MS5637_I2C_SDA_PIN 25
//규창 참고 pca10040.h 파일
	  //#define ARDUINO_SCL_PIN             26    // SCL signal pin 
    //#define ARDUINO_SDA_PIN             25    // SDA signal pin
		
		
#define I2C_RX_BUF_SIZE	10

//Common addresses definition for accelereomter. 
#define MS5637_ADDR        (0xEC>>1)
#define AN520_I2C_ADDR	0x76 // 7 bit address 
		//#define ADDR_W 0xEC //Module address write mode
	//#define ADDR_R 0xED // Module address read mode

#define CMD_RESET 									0x1E // ADC reset command
#define CMD_ADC_READ 						0x00 // ADC read command
#define CMD_ADC_CONV 					0x40 // ADC conversion command
#define CMD_ADC_D1 								0x00 // ADC D1 conversion
#define CMD_ADC_D2 								0x10 // ADC D2 conversion
#define CMD_ADC_256 							0x00 // ADC OSR=256
#define CMD_ADC_512 							0x02 // ADC OSR=512
#define CMD_ADC_1024 						0x04 // ADC OSR=1024
#define CMD_ADC_2048 						0x06 // ADC OSR=2048
#define CMD_ADC_4096 						0x08 // ADC OSR=4096
#define CMD_ADC_8192 						0x0A // ADC OSR=8192
#define CMD_PROM_RD 						0xA0 // Prom read command


#define NUMBER_OF_SAMPLES 20

// Define version of GCC. 
#define GCC_VERSION (__GNUC__ * 10000 \
                     + __GNUC_MINOR__ * 100 \
                     + __GNUC_PATCHLEVEL__)



#ifdef __GNUC_PATCHLEVEL__
#if GCC_VERSION < 50505
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wmissing-braces"           // Hack to GCC 4.9.3 bug. Can be deleted after switch on using GCC 5.0.0
#endif
#endif

#ifdef __GNUC_PATCHLEVEL__
#if GCC_VERSION < 50505
#pragma GCC diagnostic pop
#endif
#endif




void AN520cmd_reset(void);
uint32_t cmd_adc(uint8_t cmd);
uint16_t cmd_prom(uint8_t coef_num);
uint8_t AN520crc4(uint16_t n_prom[]);

//규창 메인루틴에 있던것들 분할용 함수 추가
void AN520_crc_init(void);
void AN520_Result(float *temp,  float *pressure, float *altitude);
