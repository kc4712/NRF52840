
#ifndef __OLED_H__
#define __OLED_H__


#ifdef GC_COACH_FIT_PCB
  #define  OLED_RESET_PIN  	12
#elif defined GC_COACH_FIT_DK52
  #define  OLED_RESET_PIN  	14
  #define  OLED_POWER  			12
#else
#endif 


/* I2C 7 bit address
		original Slave address 0x3C (0x78 >> 1) */
#define  OLED_I2C_ADDRESS  0x78	
#define 	KEY_INPUT		15

// proto type define
void Init_OLED_PORT(void);
void OLED_Power(int onoff);

void OLED_Write_Command(unsigned char cmd);
void OLED_Write_Data(unsigned char);

void OLED_Clear(void);
void OLED_Clear_Line(int n);

void Enable_OLED(void);
void Disable_OLED(void);
char Get_OLED_Power_Status(void);

#endif




