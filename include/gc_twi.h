#ifndef _GC_TWI_H_
#define _GC_TWI_H_

#include "nrf_drv_twi.h"

#define TWI_SLAVE_ADDR_PAH8002  0x15  //I2C 7-bit ID


uint8_t gc_twi_write(uint8_t slave_addr, uint8_t reg_addr, uint8_t tx_data);
uint8_t gc_twi_read(uint8_t slave_addr, uint8_t reg_addr, uint8_t *rx_data, uint32_t length);


#endif 
