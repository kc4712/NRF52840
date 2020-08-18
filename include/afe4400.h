
#ifndef __AFE4400_H__
#define __AFE4400_H__



#define  AFE_RESET_PIN  22
#define  SENSOR_POWER  13

#define  ADC_READY		21

#define HB_LED_CURRENT		0x13333 /* 0x16666(20mA) -> 0x13333(10mA), hkim 2015.9.17. greencomm */

#define	HB_LED_ON_COUNT	800
#define	HB_LED_OFF_COUNT	0


#define	AFE4400_REG1		0x7580
#define	AFE4400_REG2		0x7cfe
#define	AFE4400_REG3		0x7530
#define	AFE4400_REG4		0x7cff


// proto type define
unsigned int* get_AFE4400_SPI_Address(void);
void Init_AFE4400_PORT(void);
void AFE4400_Power(int onoff);
void Init_SPI(void);

void AFE4400_Read_mode(int enable);
unsigned int  AFE4400_Read_Data(unsigned char address);
void  AFE4400_Write_Data(unsigned char address, unsigned int data);

unsigned char AFE4400_sleep_control(unsigned char onflag);
void	AFE4400_current_control(unsigned int count);
void set_HB_Led_Current_Reg(unsigned int led_current);
void set_HB_led_on_count(unsigned short count);
short get_HB_LED_current_status(void);
	
#endif




