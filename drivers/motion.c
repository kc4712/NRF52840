#include "define.h"

#include <stdio.h>
#include <string.h>

#include "motion.h"


#include "nrf_delay.h"
#include "nrf_gpio.h"
#include "common.h"

#include "twi_master.h"



void Init_Motion_sensor(void)
{

	motion_init(MEMS_100HZ_ODR);


}

void motion_init(unsigned int ubMode)
{
	//LIS3DH_SetODR(LIS3DH_ODR_50Hz);

	if(ubMode ==MEMS_400HZ_ODR)
		LIS3DH_SetODR(LIS3DH_ODR_400Hz);
	
	else if(ubMode ==MEMS_100HZ_ODR)
		LIS3DH_SetODR(LIS3DH_ODR_100Hz);
	else
		LIS3DH_SetODR(LIS3DH_ODR_50Hz);
	
	LIS3DH_SetMode(LIS3DH_LOW_POWER);
	//LIS3DH_SetMode(LIS3DH_NORMAL);
	LIS3DH_SetFullScale(LIS3DH_FULLSCALE_4);  


//#if 0
	LIS3DH_SetAxis(LIS3DH_X_ENABLE | LIS3DH_Y_ENABLE | LIS3DH_Z_ENABLE);
	//LIS3DH_SetInt1Threshold(100);                  //14
	LIS3DH_SetInt1Threshold(45);                  //45
	LIS3DH_SetIntConfiguration(LIS3DH_INT1_ZHIE_ENABLE | LIS3DH_INT1_ZLIE_ENABLE |
	                                               LIS3DH_INT1_YHIE_ENABLE | LIS3DH_INT1_YLIE_ENABLE |
                                                      LIS3DH_INT1_XHIE_ENABLE | LIS3DH_INT1_XLIE_ENABLE ); 

	LIS3DH_SetInt1Pin( LIS3DH_I1_INT1_ON_PIN_INT1_ENABLE) ; 
	LIS3DH_SetIntMode(LIS3DH_INT_MODE_6D_MOVEMENT); //LIS3DH_INT_MODE_6D_MOVEMENT
	LIS3DH_FIFOModeEnable(LIS3DH_FIFO_BYPASS_MODE);
//#else

	#if 1

	LIS3DH_ENABLE_HPF();

	if(ubMode == MEMS_400HZ_ODR)
		{
		LIS3DH_SetClickCFG( LIS3DH_ZS_ENABLE | LIS3DH_YS_ENABLE | LIS3DH_XS_ENABLE);
		//LIS3DH_SetClickTHS(120);
		LIS3DH_SetClickTHS(120);
		LIS3DH_SetClickLIMIT(4);
		//LIS3DH_SetClickLIMIT(4);			// Limit = value / ODR(50Hz)
		//LIS3DH_SetClickLATENCY(80);	
		LIS3DH_SetClickLATENCY(60);	
		}
	else
		{
		LIS3DH_SetClickCFG( LIS3DH_ZS_ENABLE | LIS3DH_YS_ENABLE | LIS3DH_XS_ENABLE);
		//LIS3DH_SetClickTHS(120);
		LIS3DH_SetClickTHS(60);
		LIS3DH_SetClickLIMIT(1);
		//LIS3DH_SetClickLIMIT(4);			// Limit = value / ODR(50Hz)
		//LIS3DH_SetClickLATENCY(80);	
		LIS3DH_SetClickLATENCY(15);	
		}
	
	#else
	LIS3DH_SetClickCFG( LIS3DH_ZS_ENABLE | LIS3DH_YS_ENABLE | LIS3DH_XS_ENABLE);
	LIS3DH_SetClickTHS(60);
	LIS3DH_SetClickLIMIT(1);			// Limit = value / ODR(50Hz)
	LIS3DH_SetClickLATENCY(12);	
	#endif
	
	LIS3DH_SetInt2Pin(LIS3DH_CLICK_ON_PIN_INT2_ENABLE);
//#endif
	
}

unsigned char MOTION_Read_Data(unsigned char addr)
{
	unsigned char r_data;

	twi_master_transfer(MOTION_I2C_ADDRESS, &addr, 1, TWI_DONT_ISSUE_STOP);
	twi_master_transfer(MOTION_I2C_ADDRESS+1, &r_data, 1, TWI_ISSUE_STOP);

	return r_data;
}


unsigned int MOTION_Read_Data4(unsigned char addr)
{
	unsigned char r_data[4];

	twi_master_transfer(MOTION_I2C_ADDRESS, &addr, 1, TWI_DONT_ISSUE_STOP);
	twi_master_transfer(MOTION_I2C_ADDRESS+1, r_data, 4, TWI_ISSUE_STOP);

	printf("\n\r read4 = %x %x %x %x",r_data[0],  r_data[1],r_data[2],r_data[3]);


	return ((unsigned int)((unsigned int*)r_data));
}


void MOTION_Write_Data(unsigned char addr, unsigned char data)
{
	unsigned char w2_data[2];

	w2_data[0]=addr;
	w2_data[1] =data;
	
	if(twi_master_transfer(MOTION_I2C_ADDRESS, w2_data, 2, TWI_ISSUE_STOP)==false)
	{
		printf("\n\rMOTION_Write_Data error : i2c write");
	}
}


/*******************************************************************************
* Function Name		: LIS3DH_ReadReg
* Description		: Generic Reading function. It must be fullfilled with either
*			: I2C or SPI reading functions					
* Input			: Register Address
* Output		: Data REad
* Return		: None
*******************************************************************************/
u8_t LIS3DH_ReadReg(u8_t Reg, u8_t* Data)
{

	twi_master_transfer(MOTION_I2C_ADDRESS, &Reg, 1, TWI_DONT_ISSUE_STOP);
	twi_master_transfer(MOTION_I2C_ADDRESS+1, Data, 1, TWI_ISSUE_STOP);
	
  return 1;
}


/*******************************************************************************
* Function Name		: LIS3DH_WriteReg
* Description		: Generic Writing function. It must be fullfilled with either
*			: I2C or SPI writing function
* Input			: Register Address, Data to be written
* Output		: None
* Return		: None
*******************************************************************************/
u8_t LIS3DH_WriteReg(u8_t WriteAddr, u8_t Data)
{
  	unsigned char w2_data[2];

	w2_data[0]=WriteAddr;
	w2_data[1] =Data;
	
	return twi_master_transfer(MOTION_I2C_ADDRESS, w2_data, 2, TWI_ISSUE_STOP);
}



/*******************************************************************************
* Function Name  : LIS3DH_SetODR
* Description    : Sets LIS3DH Output Data Rate
* Input          : Output Data Rate
* Output         : None
* Return         : Status [MEMS_ERROR, MEMS_SUCCESS]
*******************************************************************************/
status_t LIS3DH_SetODR(LIS3DH_ODR_t ov){
  u8_t value;
  
  if( !LIS3DH_ReadReg(LIS3DH_CTRL_REG1, &value) )
    return MEMS_ERROR;
  
  value &= 0x0f;
  value |= ov<<LIS3DH_ODR_BIT;
  
  if( !LIS3DH_WriteReg(LIS3DH_CTRL_REG1, value) )
    return MEMS_ERROR;
  
  return MEMS_SUCCESS;
}


/*******************************************************************************
* Function Name  : LIS3DH_SetMode
* Description    : Sets LIS3DH Operating Mode
* Input          : Modality (LIS3DH_NORMAL, LIS3DH_LOW_POWER, LIS3DH_POWER_DOWN)
* Output         : None
* Return         : Status [MEMS_ERROR, MEMS_SUCCESS]
*******************************************************************************/
status_t LIS3DH_SetMode(LIS3DH_Mode_t md) {
  u8_t value;
  u8_t value2;
  static   u8_t ODR_old_value;
  
  if( !LIS3DH_ReadReg(LIS3DH_CTRL_REG1, &value) )
    return MEMS_ERROR;
  
  if( !LIS3DH_ReadReg(LIS3DH_CTRL_REG4, &value2) )
    return MEMS_ERROR;
  
  if((value & 0xF0)==0) 
    value = value | (ODR_old_value & 0xF0); //if it comes from POWERDOWN  
  
  switch(md) {
    
  case LIS3DH_POWER_DOWN:
    ODR_old_value = value;
    value &= 0x0F;
    break;
    
  case LIS3DH_NORMAL:
    value &= 0xF7;
    value |= (MEMS_RESET<<LIS3DH_LPEN);
    value2 &= 0xF7;
    value2 |= (MEMS_SET<<LIS3DH_HR);   //set HighResolution_BIT
    break;
    
  case LIS3DH_LOW_POWER:		
    value &= 0xF7;
    value |=  (MEMS_SET<<LIS3DH_LPEN);
    value2 &= 0xF7;
    value2 |= (MEMS_RESET<<LIS3DH_HR); //reset HighResolution_BIT
    break;
    
  default:
    return MEMS_ERROR;
  }
  
  if( !LIS3DH_WriteReg(LIS3DH_CTRL_REG1, value) )
    return MEMS_ERROR;
  
  if( !LIS3DH_WriteReg(LIS3DH_CTRL_REG4, value2) )
    return MEMS_ERROR;  
  
  return MEMS_SUCCESS;
}


/*******************************************************************************
* Function Name  : LIS3DH_SetAxis
* Description    : Enable/Disable LIS3DH Axis
* Input          : LIS3DH_X_ENABLE/DISABLE | LIS3DH_Y_ENABLE/DISABLE | LIS3DH_Z_ENABLE/DISABLE
* Output         : None
* Note           : You MUST use all input variable in the argument, as example
* Return         : Status [MEMS_ERROR, MEMS_SUCCESS]
*******************************************************************************/
status_t LIS3DH_SetAxis(LIS3DH_Axis_t axis) {
  u8_t value;
  
  if( !LIS3DH_ReadReg(LIS3DH_CTRL_REG1, &value) )
    return MEMS_ERROR;
  value &= 0xF8;
  value |= (0x07 & axis);
  
  if( !LIS3DH_WriteReg(LIS3DH_CTRL_REG1, value) )
    return MEMS_ERROR;   
  
  return MEMS_SUCCESS;
}



/*******************************************************************************
* Function Name  : LIS3DH_SetFullScale
* Description    : Sets the LIS3DH FullScale
* Input          : LIS3DH_FULLSCALE_2/LIS3DH_FULLSCALE_4/LIS3DH_FULLSCALE_8/LIS3DH_FULLSCALE_16
* Output         : None
* Return         : Status [MEMS_ERROR, MEMS_SUCCESS]
*******************************************************************************/
status_t LIS3DH_SetFullScale(LIS3DH_Fullscale_t fs) {
  u8_t value;
  
  if( !LIS3DH_ReadReg(LIS3DH_CTRL_REG4, &value) )
    return MEMS_ERROR;
  
  value &= 0xCF;	
  value |= (fs<<LIS3DH_FS);
  
  if( !LIS3DH_WriteReg(LIS3DH_CTRL_REG4, value) )
    return MEMS_ERROR;
  
  return MEMS_SUCCESS;
}
/*******************************************************************************
* Function Name  : LIS3DH_SetInt1Pin
* Description    : Set Interrupt1 pin Function
* Input          :  LIS3DH_CLICK_ON_PIN_INT1_ENABLE/DISABLE    | LIS3DH_I1_INT1_ON_PIN_INT1_ENABLE/DISABLE |              
                    LIS3DH_I1_INT2_ON_PIN_INT1_ENABLE/DISABLE  | LIS3DH_I1_DRDY1_ON_INT1_ENABLE/DISABLE    |              
                    LIS3DH_I1_DRDY2_ON_INT1_ENABLE/DISABLE     | LIS3DH_WTM_ON_INT1_ENABLE/DISABLE         |           
                    LIS3DH_INT1_OVERRUN_ENABLE/DISABLE  
* example        : SetInt1Pin(LIS3DH_CLICK_ON_PIN_INT1_ENABLE | LIS3DH_I1_INT1_ON_PIN_INT1_ENABLE |              
                    LIS3DH_I1_INT2_ON_PIN_INT1_DISABLE | LIS3DH_I1_DRDY1_ON_INT1_ENABLE | LIS3DH_I1_DRDY2_ON_INT1_ENABLE |
                    LIS3DH_WTM_ON_INT1_DISABLE | LIS3DH_INT1_OVERRUN_DISABLE   ) 
* Note           : To enable Interrupt signals on INT1 Pad (You MUST use all input variable in the argument, as example)
* Output         : None
* Return         : Status [MEMS_ERROR, MEMS_SUCCESS]
*******************************************************************************/
status_t LIS3DH_SetInt1Pin(LIS3DH_IntPinConf_t pinConf) {
  u8_t value;
  
  if( !LIS3DH_ReadReg(LIS3DH_CTRL_REG3, &value) )
    return MEMS_ERROR;
  
  value &= 0x00;
  value |= pinConf;
  
  if( !LIS3DH_WriteReg(LIS3DH_CTRL_REG3, value) )
    return MEMS_ERROR;
  
  return MEMS_SUCCESS;
}

/*******************************************************************************
* Function Name  : LIS3DH_SetInt2Pin
* Description    : Set Interrupt2 pin Function
* Input          : LIS3DH_CLICK_ON_PIN_INT2_ENABLE/DISABLE   | LIS3DH_I2_INT1_ON_PIN_INT2_ENABLE/DISABLE |               
                   LIS3DH_I2_INT2_ON_PIN_INT2_ENABLE/DISABLE | LIS3DH_I2_BOOT_ON_INT2_ENABLE/DISABLE |                   
                   LIS3DH_INT_ACTIVE_HIGH/LOW
* example        : LIS3DH_SetInt2Pin(LIS3DH_CLICK_ON_PIN_INT2_ENABLE/DISABLE | LIS3DH_I2_INT1_ON_PIN_INT2_ENABLE/DISABLE |               
                   LIS3DH_I2_INT2_ON_PIN_INT2_ENABLE/DISABLE | LIS3DH_I2_BOOT_ON_INT2_ENABLE/DISABLE |                   
                   LIS3DH_INT_ACTIVE_HIGH/LOW)
* Note           : To enable Interrupt signals on INT2 Pad (You MUST use all input variable in the argument, as example)
* Output         : None
* Return         : Status [MEMS_ERROR, MEMS_SUCCESS]
*******************************************************************************/
status_t LIS3DH_SetInt2Pin(LIS3DH_IntPinConf_t pinConf) {
  u8_t value;
  
  if( !LIS3DH_ReadReg(LIS3DH_CTRL_REG6, &value) )
    return MEMS_ERROR;
  
  value &= 0x00;
  value |= pinConf;
  
  if( !LIS3DH_WriteReg(LIS3DH_CTRL_REG6, value) )
    return MEMS_ERROR;
  
  return MEMS_SUCCESS;
}                       


/*******************************************************************************
* Function Name  : LIS3DH_SetIntConfiguration
* Description    : Interrupt 1 Configuration (without LIS3DH_6D_INT)
* Input          : LIS3DH_INT1_AND/OR | LIS3DH_INT1_ZHIE_ENABLE/DISABLE | LIS3DH_INT1_ZLIE_ENABLE/DISABLE...
* Output         : None
* Note           : You MUST use all input variable in the argument, as example
* Return         : Status [MEMS_ERROR, MEMS_SUCCESS]
*******************************************************************************/
status_t LIS3DH_SetIntConfiguration(LIS3DH_Int1Conf_t ic) {
  u8_t value;
  
  if( !LIS3DH_ReadReg(LIS3DH_INT1_CFG, &value) )
    return MEMS_ERROR;
  
  value &= 0x40; 
  value |= ic;
  
  if( !LIS3DH_WriteReg(LIS3DH_INT1_CFG, value) )
    return MEMS_ERROR;
  
  return MEMS_SUCCESS;
} 

     
/*******************************************************************************
* Function Name  : LIS3DH_SetIntMode
* Description    : Interrupt 1 Configuration mode (OR, 6D Movement, AND, 6D Position)
* Input          : LIS3DH_INT_MODE_OR, LIS3DH_INT_MODE_6D_MOVEMENT, LIS3DH_INT_MODE_AND, 
				   LIS3DH_INT_MODE_6D_POSITION
* Output         : None
* Return         : Status [MEMS_ERROR, MEMS_SUCCESS]
*******************************************************************************/
status_t LIS3DH_SetIntMode(LIS3DH_Int1Mode_t int_mode) {
  u8_t value;
  
  if( !LIS3DH_ReadReg(LIS3DH_INT1_CFG, &value) )
    return MEMS_ERROR;
  
  value &= 0x3F; 
  value |= (int_mode<<LIS3DH_INT_6D);
  
  if( !LIS3DH_WriteReg(LIS3DH_INT1_CFG, value) )
    return MEMS_ERROR;
  
  return MEMS_SUCCESS;
}

/*******************************************************************************
* Function Name  : LIS3DH_SetInt1Threshold
* Description    : Sets Interrupt 1 Threshold
* Input          : Threshold = [0,31]
* Output         : None
* Return         : Status [MEMS_ERROR, MEMS_SUCCESS]
*******************************************************************************/
status_t LIS3DH_SetInt1Threshold(u8_t ths) {
  if (ths > 127)
    return MEMS_ERROR;
  
  if( !LIS3DH_WriteReg(LIS3DH_INT1_THS, ths) )
    return MEMS_ERROR;    
  
  return MEMS_SUCCESS;
}

status_t LIS3DH_GetInt1Threshold(u8_t *pThrValue)
{

	if(!LIS3DH_ReadReg(LIS3DH_INT1_THS,pThrValue))
		return MEMS_ERROR;

	return MEMS_SUCCESS;
}


/*******************************************************************************
* Function Name  : LIS3DH_SetInt1Duration
* Description    : Sets Interrupt 1 Duration
* Input          : Duration value
* Output         : None
* Return         : Status [MEMS_ERROR, MEMS_SUCCESS]
*******************************************************************************/
status_t LIS3DH_SetInt1Duration(LIS3DH_Int1Conf_t id) {
  
  if (id > 127)
    return MEMS_ERROR;
  
  if( !LIS3DH_WriteReg(LIS3DH_INT1_DURATION, id) )
    return MEMS_ERROR;
  
  return MEMS_SUCCESS;
}


/*******************************************************************************
* Function Name  : LIS3DH_FIFOModeEnable
* Description    : Sets Fifo Modality
* Input          : LIS3DH_FIFO_DISABLE, LIS3DH_FIFO_BYPASS_MODE, LIS3DH_FIFO_MODE, 
				   LIS3DH_FIFO_STREAM_MODE, LIS3DH_FIFO_TRIGGER_MODE
* Output         : None
* Return         : Status [MEMS_ERROR, MEMS_SUCCESS]
*******************************************************************************/
status_t LIS3DH_FIFOModeEnable(LIS3DH_FifoMode_t fm) {
  u8_t value;  
  
  if(fm == LIS3DH_FIFO_DISABLE) { 
    if( !LIS3DH_ReadReg(LIS3DH_FIFO_CTRL_REG, &value) )
      return MEMS_ERROR;
    
    value &= 0x1F;
    value |= (LIS3DH_FIFO_BYPASS_MODE<<LIS3DH_FM);                     
    
    if( !LIS3DH_WriteReg(LIS3DH_FIFO_CTRL_REG, value) )           //fifo mode bypass
      return MEMS_ERROR;   
    if( !LIS3DH_ReadReg(LIS3DH_CTRL_REG5, &value) )
      return MEMS_ERROR;
    
    value &= 0xBF;    
    
    if( !LIS3DH_WriteReg(LIS3DH_CTRL_REG5, value) )               //fifo disable
      return MEMS_ERROR;   
  }
  
  if(fm == LIS3DH_FIFO_BYPASS_MODE)   {  
    if( !LIS3DH_ReadReg(LIS3DH_CTRL_REG5, &value) )
      return MEMS_ERROR;
    
    value &= 0xBF;
    value |= MEMS_SET<<LIS3DH_FIFO_EN;
    
    if( !LIS3DH_WriteReg(LIS3DH_CTRL_REG5, value) )               //fifo enable
      return MEMS_ERROR;  
    if( !LIS3DH_ReadReg(LIS3DH_FIFO_CTRL_REG, &value) )
      return MEMS_ERROR;
    
    value &= 0x1f;
    value |= (fm<<LIS3DH_FM);                     //fifo mode configuration
    
    if( !LIS3DH_WriteReg(LIS3DH_FIFO_CTRL_REG, value) )
      return MEMS_ERROR;
  }
  
  if(fm == LIS3DH_FIFO_MODE)   {
    if( !LIS3DH_ReadReg(LIS3DH_CTRL_REG5, &value) )
      return MEMS_ERROR;
    
    value &= 0xBF;
    value |= MEMS_SET<<LIS3DH_FIFO_EN;
    
    if( !LIS3DH_WriteReg(LIS3DH_CTRL_REG5, value) )               //fifo enable
      return MEMS_ERROR;  
    if( !LIS3DH_ReadReg(LIS3DH_FIFO_CTRL_REG, &value) )
      return MEMS_ERROR;
    
    value &= 0x1f;
    value |= (fm<<LIS3DH_FM);                      //fifo mode configuration
    
    if( !LIS3DH_WriteReg(LIS3DH_FIFO_CTRL_REG, value) )
      return MEMS_ERROR;
  }
  
  if(fm == LIS3DH_FIFO_STREAM_MODE)   {  
    if( !LIS3DH_ReadReg(LIS3DH_CTRL_REG5, &value) )
      return MEMS_ERROR;
    
    value &= 0xBF;
    value |= MEMS_SET<<LIS3DH_FIFO_EN;
    
    if( !LIS3DH_WriteReg(LIS3DH_CTRL_REG5, value) )               //fifo enable
      return MEMS_ERROR;   
    if( !LIS3DH_ReadReg(LIS3DH_FIFO_CTRL_REG, &value) )
      return MEMS_ERROR;
    
    value &= 0x1f;
    value |= (fm<<LIS3DH_FM);                      //fifo mode configuration
    
    if( !LIS3DH_WriteReg(LIS3DH_FIFO_CTRL_REG, value) )
      return MEMS_ERROR;
  }
  
  if(fm == LIS3DH_FIFO_TRIGGER_MODE)   {  
    if( !LIS3DH_ReadReg(LIS3DH_CTRL_REG5, &value) )
      return MEMS_ERROR;
    
    value &= 0xBF;
    value |= MEMS_SET<<LIS3DH_FIFO_EN;
    
    if( !LIS3DH_WriteReg(LIS3DH_CTRL_REG5, value) )               //fifo enable
      return MEMS_ERROR;    
    if( !LIS3DH_ReadReg(LIS3DH_FIFO_CTRL_REG, &value) )
      return MEMS_ERROR;
    
    value &= 0x1f;
    value |= (fm<<LIS3DH_FM);                      //fifo mode configuration
    
    if( !LIS3DH_WriteReg(LIS3DH_FIFO_CTRL_REG, value) )
      return MEMS_ERROR;
  }
  
  return MEMS_SUCCESS;
}
  

/*******************************************************************************
* Function Name  : LIS3DH_GetAccAxesRaw
* Description    : Read the Acceleration Values Output Registers
* Input          : buffer to empity by AxesRaw_t Typedef
* Output         : None
* Return         : Status [MEMS_ERROR, MEMS_SUCCESS]
*******************************************************************************/
status_t LIS3DH_GetAccAxesRaw(AxesRaw_t* buff) {
  i16_t value;
  u8_t *valueL = (u8_t *)(&value);
  u8_t *valueH = ((u8_t *)(&value)+1);

  if( !LIS3DH_ReadReg(LIS3DH_OUT_X_L, valueL) )
    return MEMS_ERROR;
  
  if( !LIS3DH_ReadReg(LIS3DH_OUT_X_H, valueH) )
    return MEMS_ERROR;
  
  buff->AXIS_X = value;
  
  if( !LIS3DH_ReadReg(LIS3DH_OUT_Y_L, valueL) )
    return MEMS_ERROR;
  
  if( !LIS3DH_ReadReg(LIS3DH_OUT_Y_H, valueH) )
    return MEMS_ERROR;
  
  buff->AXIS_Y = value;
  
  if( !LIS3DH_ReadReg(LIS3DH_OUT_Z_L, valueL) )
    return MEMS_ERROR;
  
  if( !LIS3DH_ReadReg(LIS3DH_OUT_Z_H, valueH) )
    return MEMS_ERROR;
  
  buff->AXIS_Z = value;
  
  return MEMS_SUCCESS; 
}



status_t LIS3DH_GetAccAxesRawByte(AxesRawByte_t* buff)
{
  u8_t value;
  
  LIS3DH_ReadReg(LIS3DH_OUT_X_H, &value);
  buff->AXIS_X = value;
  
  LIS3DH_ReadReg(LIS3DH_OUT_Y_H, &value);
  buff->AXIS_Y = value;
  
  LIS3DH_ReadReg(LIS3DH_OUT_Z_H, &value);
  buff->AXIS_Z = value;
  
  return MEMS_SUCCESS; 
}










/*******************************************************************************
* Function Name  : LIS3DH_SetClickCFG
* Description    : Set Click Interrupt config Function
* Input          : LIS3DH_ZD_ENABLE/DISABLE | LIS3DH_ZS_ENABLE/DISABLE  | LIS3DH_YD_ENABLE/DISABLE  | 
                   LIS3DH_YS_ENABLE/DISABLE | LIS3DH_XD_ENABLE/DISABLE  | LIS3DH_XS_ENABLE/DISABLE 
* example        : LIS3DH_SetClickCFG( LIS3DH_ZD_ENABLE | LIS3DH_ZS_DISABLE | LIS3DH_YD_ENABLE | 
                               LIS3DH_YS_DISABLE | LIS3DH_XD_ENABLE | LIS3DH_XS_ENABLE)
* Note           : You MUST use all input variable in the argument, as example
* Output         : None
* Return         : Status [MEMS_ERROR, MEMS_SUCCESS]
*******************************************************************************/
status_t LIS3DH_SetClickCFG(u8_t status) {
  u8_t value;
  
  if( !LIS3DH_ReadReg(LIS3DH_CLICK_CFG, &value) )
    return MEMS_ERROR;
  
  value &= 0xC0;
  value |= status;
  
  if( !LIS3DH_WriteReg(LIS3DH_CLICK_CFG, value) )
    return MEMS_ERROR;
  
  return MEMS_SUCCESS;
}  


/*******************************************************************************
* Function Name  : LIS3DH_SetClickTHS
* Description    : Set Click Interrupt threshold
* Input          : Click-click Threshold value [0-127]
* Output         : None
* Return         : Status [MEMS_ERROR, MEMS_SUCCESS]
*******************************************************************************/
status_t LIS3DH_SetClickTHS(u8_t ths) {
  
  if(ths>127)     
    return MEMS_ERROR;
  
  if( !LIS3DH_WriteReg(LIS3DH_CLICK_THS, ths) )
    return MEMS_ERROR;
  
  return MEMS_SUCCESS;
} 


/*******************************************************************************
* Function Name  : LIS3DH_SetClickLIMIT
* Description    : Set Click Interrupt Time Limit
* Input          : Click-click Time Limit value [0-127]
* Output         : None
* Return         : Status [MEMS_ERROR, MEMS_SUCCESS]
*******************************************************************************/
status_t LIS3DH_SetClickLIMIT(u8_t t_limit) {
  
  if(t_limit>127)     
    return MEMS_ERROR;
  
  if( !LIS3DH_WriteReg(LIS3DH_TIME_LIMIT, t_limit) )
    return MEMS_ERROR;
  
  return MEMS_SUCCESS;
} 


/*******************************************************************************
* Function Name  : LIS3DH_SetClickLATENCY
* Description    : Set Click Interrupt Time Latency
* Input          : Click-click Time Latency value [0-255]
* Output         : None
* Return         : Status [MEMS_ERROR, MEMS_SUCCESS]
*******************************************************************************/
status_t LIS3DH_SetClickLATENCY(u8_t t_latency) {
  
  if( !LIS3DH_WriteReg(LIS3DH_TIME_LATENCY, t_latency) )
    return MEMS_ERROR;
  
  return MEMS_SUCCESS;
} 


/*******************************************************************************
* Function Name  : LIS3DH_SetClickWINDOW
* Description    : Set Click Interrupt Time Window
* Input          : Click-click Time Window value [0-255]
* Output         : None
* Return         : Status [MEMS_ERROR, MEMS_SUCCESS]
*******************************************************************************/
status_t LIS3DH_SetClickWINDOW(u8_t t_window) {
  
  if( !LIS3DH_WriteReg(LIS3DH_TIME_WINDOW, t_window) )
    return MEMS_ERROR;
  
  return MEMS_SUCCESS;
}


/*******************************************************************************
* Function Name  : LIS3DH_GetClickResponse
* Description    : Get Click Interrupt Response by CLICK_SRC REGISTER
* Input          : char to empty by Click Response Typedef
* Output         : None
* Return         : Status [MEMS_ERROR, MEMS_SUCCESS]
*******************************************************************************/
status_t LIS3DH_GetClickResponse(u8_t* res) {
  u8_t value;
  
  if( !LIS3DH_ReadReg(LIS3DH_CLICK_SRC, &value) ) 
    return MEMS_ERROR;
  
  value &= 0x7F;
  
  if((value & LIS3DH_IA)==0) {        
    *res = LIS3DH_NO_CLICK;     
    return MEMS_SUCCESS;
  }
  else {
    if (value & LIS3DH_DCLICK){
      if (value & LIS3DH_CLICK_SIGN){
        if (value & LIS3DH_CLICK_Z) {
          *res = LIS3DH_DCLICK_Z_N;   
          return MEMS_SUCCESS;
        }
        if (value & LIS3DH_CLICK_Y) {
          *res = LIS3DH_DCLICK_Y_N;   
          return MEMS_SUCCESS;
        }
        if (value & LIS3DH_CLICK_X) {
          *res = LIS3DH_DCLICK_X_N;   
          return MEMS_SUCCESS;
        }
      }
      else{
        if (value & LIS3DH_CLICK_Z) {
          *res = LIS3DH_DCLICK_Z_P;   
          return MEMS_SUCCESS;
        }
        if (value & LIS3DH_CLICK_Y) {
          *res = LIS3DH_DCLICK_Y_P;   
          return MEMS_SUCCESS;
        }
        if (value & LIS3DH_CLICK_X) {
          *res = LIS3DH_DCLICK_X_P;   
          return MEMS_SUCCESS;
        }
      }       
    }
    else{
      if (value & LIS3DH_CLICK_SIGN){
        if (value & LIS3DH_CLICK_Z) {
          *res = LIS3DH_SCLICK_Z_N;   
          return MEMS_SUCCESS;
        }
        if (value & LIS3DH_CLICK_Y) {
          *res = LIS3DH_SCLICK_Y_N;   
          return MEMS_SUCCESS;
        }
        if (value & LIS3DH_CLICK_X) {
          *res = LIS3DH_SCLICK_X_N;   
          return MEMS_SUCCESS;
        }
      }
      else{
        if (value & LIS3DH_CLICK_Z) {
          *res = LIS3DH_SCLICK_Z_P;   
          return MEMS_SUCCESS;
        }
        if (value & LIS3DH_CLICK_Y) {
          *res = LIS3DH_SCLICK_Y_P;   
          return MEMS_SUCCESS;
        }
        if (value & LIS3DH_CLICK_X) {
          *res = LIS3DH_SCLICK_X_P;   
          return MEMS_SUCCESS;
        }
      }
    }
  }
  return MEMS_ERROR;
} 

/* Private functions ---------------------------------------------------------*/

/*******************************************************************************
* Function Name  : LIS3DH_GetWHO_AM_I
* Description    : Read identification code by WHO_AM_I register
* Input          : Char to empty by Device identification Value
* Output         : None
* Return         : Status [value of FSS]
*******************************************************************************/
status_t LIS3DH_GetWHO_AM_I(u8_t* val){
  
  if( !LIS3DH_ReadReg(LIS3DH_WHO_AM_I, val) )
  	return MEMS_ERROR;
  
  return MEMS_SUCCESS;
}






status_t LIS3DH_ENABLE_HPF(void)
{

	LIS3DH_WriteReg(LIS3DH_CTRL_REG2, 0x0);
	//LIS3DH_WriteReg(LIS3DH_CTRL_REG2, 0x08);

	return MEMS_SUCCESS;
}











