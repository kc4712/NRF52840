#ifndef _PAH8002_COMM_H_
#define _PAH8002_COMM_H_

#include <stdint.h>
#include "stdbool.h"

#define ARRAY_SIZE(arr) (sizeof(arr) / sizeof(arr[0]))

// return 0 if the function is successful.
uint8_t pah8002_write_reg(uint8_t addr, uint8_t data);
uint8_t pah8002_read_reg(uint8_t addr, uint8_t *data);
uint8_t pah8002_burst_read_reg(uint8_t addr, uint8_t *data, uint32_t rx_size);



//SPI.H에 있던 쓰기 읽기 함수들 
bool spi_write_reg(uint8_t addr, uint8_t data);
bool spi_read_reg(uint8_t addr, uint8_t *data, uint32_t rx_size);
bool spi_burst_read_reg(uint8_t addr, uint8_t *data, uint32_t rx_size);



//그린콤 추가 함수들
void pah8002_raw_data_checksum(void);
void get_pah8002_checksum(void);
void compare_pah8002_checksum(void);


#endif
