#include <stdint.h>
#include <string.h>
#include "nordic_common.h"
#include "nrf.h"
#include "nrf_delay.h"
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

//규창 171018 featureData extern용
#include "gc_activity.h"

#include "nrf_drv_spi.h"
#include "spi_flash.h"
#include "flash_control.h"

extern volatile uint8_t gc_flash_status;

volatile uint16_t f_page_index_write_pos_step=0; 	
volatile uint16_t f_page_index_write_pos_kcal=0; 
volatile uint16_t f_page_index_write_pos_act=0; 

/**
 * @brief get starting page index according to table
* @return 
 */
uint32_t gc_flash_page_start(uint32_t table_type)
{
	switch(table_type)
	{
		case GC_FLASH_TBL_TYPE_STEP:
			return GC_FLASH_PAGE_START_STEP;
		
		case GC_FLASH_TBL_TYPE_KCAL:
			return GC_FLASH_PAGE_START_KCAL;
			
		case GC_FLASH_TBL_TYPE_ACT:
			return GC_FLASH_PAGE_START_ACT;
		
		case GC_FLASH_TBL_TYPE_SN:
			return GC_FLASH_PAGE_SN;
				
		case GC_FLASH_TBL_TYPE_GMT_OFFSET:
			return GC_FLASH_PAGE_GMT_OFFSET; //-hmlee 2016.08.19
			
		case GC_FLASH_TBL_TYPE_STABLE_HR:
			return GC_FLASH_PAGE_STABLE_HR;
		
		case GC_FLASH_TBL_TYPE_PERSINFO:
			return GC_FLASH_PAGE_PERSINFO;
		
		case GC_FLASH_TBL_TYPE_BAT_STEP:
			return GC_FLASH_PAGE_BAT_STEP;
		
		case GC_FLASH_TBL_TYPE_BAT_KCAL:
			return GC_FLASH_PAGE_BAT_KCAL;
			
		case GC_FLASH_TBL_TYPE_BAT_ACT:
			return GC_FLASH_PAGE_BAT_ACT;
		
		case GC_FLASH_TBL_TYPE_POWER_OFF:
			return GC_FLASH_PAGE_POWER_OFF;
		
			case GC_FLASH_TBL_TYPE_RESET_LOG:
		return GC_FLASH_PAGE_RESET_LOG;
	
			case GC_FLASH_TBL_TYPE_RESET_DATA:
		return GC_FLASH_PAGE_RESET_DATA;
				
		default:
			break;
	}
	return 0;
}

/**
 * @brief get current position of writable page 
* @return 
 */
uint32_t get_flash_page_write_position(uint32_t table_type)
{
	switch(table_type)
	{
		case GC_FLASH_TBL_TYPE_STEP:
			return f_page_index_write_pos_step;
		
		case GC_FLASH_TBL_TYPE_KCAL:
			return f_page_index_write_pos_kcal;
		
		case GC_FLASH_TBL_TYPE_ACT:
			return f_page_index_write_pos_act;
		
		case GC_FLASH_TBL_TYPE_SN:
			return GC_FLASH_PAGE_SN;
		
		case GC_FLASH_TBL_TYPE_GMT_OFFSET:
			return GC_FLASH_PAGE_GMT_OFFSET;
		
		case GC_FLASH_TBL_TYPE_STABLE_HR:
			return GC_FLASH_PAGE_STABLE_HR;
		
		case GC_FLASH_TBL_TYPE_PERSINFO:
			return GC_FLASH_PAGE_PERSINFO;
		
		case GC_FLASH_TBL_TYPE_BAT_STEP:
			return GC_FLASH_PAGE_BAT_STEP;
		
		case GC_FLASH_TBL_TYPE_BAT_KCAL:
			return GC_FLASH_PAGE_BAT_KCAL;
		
		case GC_FLASH_TBL_TYPE_BAT_ACT:
			return GC_FLASH_PAGE_BAT_ACT;	
		
		case GC_FLASH_TBL_TYPE_POWER_OFF:
			return GC_FLASH_PAGE_POWER_OFF;
		
		case GC_FLASH_TBL_TYPE_RESET_LOG:
			return GC_FLASH_PAGE_RESET_LOG;
		
		case GC_FLASH_TBL_TYPE_RESET_DATA:
			return GC_FLASH_PAGE_RESET_DATA;
						
		default:
			break;
	}
	
	return false;
}

/**
 * @brief move to next page writing position
* @return 
 */
uint32_t move_flash_page_write_position(uint32_t table_type)
{
	switch(table_type)
	{
		case GC_FLASH_TBL_TYPE_STEP:
		{
			if (f_page_index_write_pos_step == GC_FLASH_PAGE_END_STEP)
				f_page_index_write_pos_step = GC_FLASH_PAGE_START_STEP;
			else
				f_page_index_write_pos_step += 1;
		}
		break;
		
		case GC_FLASH_TBL_TYPE_KCAL:
		{
			if (f_page_index_write_pos_kcal == GC_FLASH_PAGE_END_KCAL)
				f_page_index_write_pos_kcal = GC_FLASH_PAGE_START_KCAL;
			else
				f_page_index_write_pos_kcal += 1;
		}
		break;
		
		case GC_FLASH_TBL_TYPE_ACT:
		{
			if (f_page_index_write_pos_act == GC_FLASH_PAGE_END_ACT)
				f_page_index_write_pos_act = GC_FLASH_PAGE_START_ACT;
			else
				f_page_index_write_pos_act += 1;
		}
		break;
		
		default:
			break;
	}
	
	return false;
}

/**
 * @brief check whether page index is last page in a sector or not
* @return true : last page,  false : not last page
 */
int32_t check_flash_page_last(uint32_t table_type, uint32_t page_index)
{
	uint32_t	remain=0;
	
	remain = (page_index +1) % (FLASH_PAGE_NUM_IN_SECTOR);
	
	if (remain != 0)
		return false;
			
	return true;
}

/**
 * @brief erase next sector if current writted page is last page in sector
* @return 
 */
int32_t erase_flash_next_sector(uint32_t table_type, uint32_t page_index)
{
	/* check written page is last page of sector or not */
	if (check_flash_page_last(table_type, page_index) == false)
		return false;
	
	//printf("\r\n page index %d is Last!", page_index);
	/* erase next sector */
	switch(table_type)
	{
		case GC_FLASH_TBL_TYPE_STEP:
			{
				if (page_index == GC_FLASH_PAGE_END_STEP)
				{
					printf("\r\n erase page index %d", GC_FLASH_PAGE_START_STEP);
					gc_flash_erase(GC_FLASH_PAGE_START_STEP);
				}
				else
				{
					printf("\r\n erase page index %d", page_index+1);
					gc_flash_erase(page_index+1);
				}
			}
			break;
			
		case GC_FLASH_TBL_TYPE_KCAL:
		{
				if (page_index == GC_FLASH_PAGE_END_KCAL)
				{
					printf("\r\n erase page index %d", GC_FLASH_PAGE_START_KCAL);
					gc_flash_erase(GC_FLASH_PAGE_START_KCAL);
				}
				else
				{
					printf("\r\n erase page index %d", page_index+1);
					gc_flash_erase(page_index+1);
				}
		}
		break;
			
		case GC_FLASH_TBL_TYPE_ACT:
		{
				if (page_index == GC_FLASH_PAGE_END_ACT)
				{
					printf("\r\n erase page index %d", GC_FLASH_PAGE_START_ACT);
					gc_flash_erase(GC_FLASH_PAGE_START_ACT);
				}
				else
				{
					printf("\r\n erase page index %d", page_index+1);
					gc_flash_erase(page_index+1);
				}
		}
		break;
			
		default:
			break;
	}
	
	return true;
}


void gc_flash_read(uint16_t  page_index, uint8_t * data ) /* 254 */
{
	uint8_t buf[GC_FLASH_PAGE_SIZE];
	uint32_t page_addr;

	page_addr = page_index <<8;
	
	SPI_FLASH_BufferRead(buf, page_addr );
	
	memcpy(data,
								(buf+GC_FLASH_PAGE_ADDR_SIZE),
								GC_FLASH_PAGE_DATA_SIZE);
}



int32_t gc_flash_fwrite(uint32_t  page_index, uint8_t* data )
{
	int32_t err_code=0;
	uint8_t buf[GC_FLASH_PAGE_SIZE];
	uint32_t page_addr;
	
	memset(buf,GC_FLASH_NO_DATA,sizeof(buf));

	page_addr = page_index<<8;
	memcpy(buf+4, data, GC_FLASH_PAGE_SIZE);

	err_code = SPI_FLASH_PageWrite(buf, page_addr );

	return err_code;
}



void gc_flash_erase(uint16_t  page_index)
{
	int32_t page_addr;
	uint32_t __page_index=0;

	__page_index = page_index;
	page_addr = __page_index << 8;
	
	SPI_FLASH_SectorErase(page_addr );	
}

/* get data from page that exactly macted to req_time */
uint16_t gc_flash_page_read_data(uint32_t table_type, uint32_t req_time, uint8_t *req_buf)
{
	uint16_t 	__page=0, __page_index=0;
	uint32_t	page_time=0;
	uint16_t 	s_page=0, e_page=0;
	uint8_t		page_buf[GC_FLASH_PAGE_SIZE];
	
	memset(page_buf, 0, GC_FLASH_PAGE_SIZE);
	
	/* set start/end page number according to table type */
	s_page = gc_flash_page_start(table_type);
	if (table_type < GC_FLASH_TBL_TYPE_SN)
		e_page = s_page + FLASH_PAGE_NUM_IN_SECTOR;
	else
		e_page = s_page;
	
	/* exact matching */
	for(__page=s_page; __page < (e_page+1); __page++) 
	{
		__page_index = __page; /* FIXME */
		gc_flash_read(__page_index, page_buf );
		if (table_type >= GC_FLASH_TBL_TYPE_SN) //-hmlee 2016.08.19
		{
			memcpy(req_buf, page_buf, GC_FLASH_PAGE_SIZE);
			return true;
		}	
		page_time = *((uint32_t *)page_buf);
		//printf("\r\npage_time : %d",page_time);
		if ( (page_time == 0xffffffff) || (page_time != req_time) )
			continue;
		
		if ( page_time == req_time)
		{
			memcpy(req_buf, page_buf, GC_FLASH_PAGE_SIZE);
			return true;
		}
	}
	return false;
}

int32_t gc_flash_page_write_data(uint32_t table_type, uint8_t *req_buf) /* 250 */
{
	int32_t	err_code=0;
	uint32_t page_index=0,__page_index=0;
	uint8_t	page_buf[GC_FLASH_PAGE_SIZE];
	
	memset(page_buf, GC_FLASH_NO_DATA, GC_FLASH_PAGE_SIZE);
	
	page_index = get_flash_page_write_position(table_type);
	__page_index = page_index;
	
	if (page_index == GC_FLASH_NO_DATA)
		return false;
	
	if ( table_type >= GC_FLASH_TBL_TYPE_SN)
	{
		gc_flash_erase(page_index);
		nrf_delay_ms(200);
	}
	
	err_code = gc_flash_fwrite(page_index, req_buf);
	if (err_code == NRF_SUCCESS)
	{
		/* erase next sector if no blank page exist */
		erase_flash_next_sector(table_type, __page_index);
	}

	move_flash_page_write_position(table_type);
	
	return true;
}



void find_flash_page_write_position_at_boot(void)
{
	int32_t _step, _kcal, _act=0;
	int32_t _loop_start, _loop_end=0;
	uint8_t	page_buf[GC_FLASH_PAGE_SIZE];
	
	_loop_start = GC_FLASH_PAGE_START_STEP;
	_loop_end	= GC_FLASH_PAGE_END_STEP;
	
	/* find first empty page for step */
	for (_step=_loop_start; _step < (_loop_end+1); _step++)
	{
		memset(page_buf, GC_FLASH_NO_DATA, sizeof(page_buf));
		
		gc_flash_read(_step, page_buf );

		if (page_buf[0] == GC_FLASH_NO_DATA)
		{
			f_page_index_write_pos_step = _step;
			break;
		}
	}
	
	/* find first empty page for kcal */
	_loop_start = GC_FLASH_PAGE_START_KCAL;
	_loop_end	= GC_FLASH_PAGE_END_KCAL;
	
	for (_kcal=_loop_start; _kcal < (_loop_end+1); _kcal++)
	{
		memset(page_buf, GC_FLASH_NO_DATA, sizeof(page_buf));
		
		gc_flash_read(_kcal, page_buf );

		if (page_buf[0] == GC_FLASH_NO_DATA)
		{
			f_page_index_write_pos_kcal = _kcal;
			break;
		}
	}
	
	/* find first empty page for kcal */
	_loop_start = GC_FLASH_PAGE_START_ACT;
	_loop_end	= GC_FLASH_PAGE_END_ACT;
	
	for (_act=_loop_start; _act < (_loop_end+1); _act++)
	{
		memset(page_buf, GC_FLASH_NO_DATA, sizeof(page_buf));
		
		gc_flash_read(_act, page_buf );
	
		if (page_buf[0] == GC_FLASH_NO_DATA)
		{
			f_page_index_write_pos_act = _act;
			break;
		}
	}
#ifndef _FIT_ETRI_TEST_
#ifdef DEBUG_PRINT
	printf("\r\n first empty page addr: step(0x%x), kcal(0x%x), act(0x%x)", 
					(f_page_index_write_pos_step << 8), (f_page_index_write_pos_kcal << 8), 
					(f_page_index_write_pos_act << 8));
	#endif
#endif 
}



//extern volatile struct feature_tbl_form_s featuredata;
/*
//규창 171101 피쳐 저장 기능 변경 
void find_recent_feature_save(struct feature_tbl_form_s featuredata) {
	tTime time;
	tTime stdTime;
	
	get_time(&time, 1);
	//convert_gmt_to_display_time(&time, &stdTime);
	printf("\r\n direct today???:%d %d %d %d %d %d", time.year, time.month, time.day , time.hour, time.minute, time.second);
	printf("\r\n direct today???:%d", (get_GMT(time.year,  time.month,  time.day,  0,  0, 0 )/(60)));
	
	
	extern volatile short gmt_offset;
	unsigned char offset_data[GC_FLASH_PAGE_SIZE];
		
	
	//규창 171031 플래시 주소 읽는 함수 구현이 다소 이상하다..
	//주소값이 16페이지를 기준으로 와리가리 할 가능성이 매우 높음
	//gc_flash_read(GC_FLASH_PAGE_GMT_OFFSET, offset_data );
	//gc_flash_page_read_data(GC_FLASH_TBL_TYPE_GMT_OFFSET,1,offset_data);
	printf("\r\n offset_data : %d,%d",offset_data[0],offset_data[1]);
	//gmt_offsets = offset_data[0] |  offset_data[1]<<8;
	
	unsigned int  today = 0;
	if (gmt_offset >= 0){
		today = (get_GMT(time.year,  time.month,  time.day,  0,  0, 0 )/(60)) + gmt_offset;
	}else{
		today = (get_GMT(time.year,  time.month,  time.day,  0,  0, 0 )/(60)) - gmt_offset;
	}
	
	printf("\r\n today:%d", today);
	//for(uint16_t __page=GC_FLASH_PAGE_START_TIME; __page <= (GC_FLASH_PAGE_END_DAY3_TIME+1); __page+=GC_FLASH_SECTOR_NUM_TIME) 
	//{
	//	gc_flash_erase(__page);
	//}
	//(get_GMT(time.year,  time.month,  time.day,  time.hour,  time.minute, time.second )/(60))
	
	uint8_t		daycheck_buf[GC_FLASH_PAGE_SIZE];
	struct feature_tbl_form_s __feature1day1;
	struct feature_tbl_form_s __feature1day2;
	struct feature_tbl_form_s __feature1day3;
	
	memset(daycheck_buf, 0, GC_FLASH_PAGE_SIZE);
	gc_flash_read(GC_FLASH_PAGE_START_TIME, daycheck_buf );
	memcpy(&__feature1day1, daycheck_buf, sizeof(struct feature_tbl_form_s));
	
	
	memset(daycheck_buf, 0, GC_FLASH_PAGE_SIZE);
	gc_flash_read(GC_FLASH_PAGE_START_DAY2_TIME, daycheck_buf );
	memcpy(&__feature1day2, daycheck_buf, sizeof(struct feature_tbl_form_s));
	
	memset(daycheck_buf, 0, GC_FLASH_PAGE_SIZE);
	gc_flash_read(GC_FLASH_PAGE_START_DAY3_TIME, daycheck_buf );
	memcpy(&__feature1day3, daycheck_buf, sizeof(struct feature_tbl_form_s));
	
	
	printf("\r\n __feature1day1:%d  __feature1day2:%d  __feature1day3:%d ", __feature1day1.time, __feature1day2.time, __feature1day3.time );
	unsigned int day1 = today - __feature1day1.time;
	unsigned int day2 = today - __feature1day2.time;
	unsigned int day3 = today - __feature1day3.time;
	unsigned int refSaveAddr = 0; 
	printf("\r\n day1:%d  day2:%d  day3:%d ", day1, day2, day3 );
	//순차적일 경우
	//day3 0,  day2 1440, day1 2880
	
	//
	//
//	if(day1 == 2880){
	//	printf("\r\n ######1DAY FLASH DELETE");
		//for(uint16_t pageindex = GC_FLASH_PAGE_START_TIME ; pageindex <= GC_FLASH_PAGE_END_TIME-15 ; pageindex+=GC_FLASH_SECTOR_NUM_TIME){
			//gc_flash_erase(pageindex);
			//nrf_delay_ms(100);
		//}
	//}else if(day2 == 2880){
		//printf("\r\n ######2DAY FLASH DELETE");
		//for(uint16_t pageindex = GC_FLASH_PAGE_START_DAY2_TIME ; pageindex <= GC_FLASH_PAGE_END_DAY2_TIME-15 ; pageindex+=GC_FLASH_SECTOR_NUM_TIME){
			//gc_flash_erase(pageindex);
			//nrf_delay_ms(100);
		//}
	//}else if(day3 == 2880){
		//printf("\r\n ######3DAY FLASH DELETE");
		//for(uint16_t pageindex = GC_FLASH_PAGE_START_DAY3_TIME ; pageindex <= GC_FLASH_PAGE_END_DAY3_TIME-15 ; pageindex+=GC_FLASH_SECTOR_NUM_TIME){
			//gc_flash_erase(pageindex);
			//nrf_delay_ms(100);
		//}
	//}
	if(today % __feature1day1.time != 0){
		printf("\r\n ######1DAY FLASH DELETE");
		for(uint16_t pageindex = GC_FLASH_PAGE_START_TIME ; pageindex <= GC_FLASH_PAGE_END_TIME-15 ; pageindex+=GC_FLASH_SECTOR_NUM_TIME){
			gc_flash_erase(pageindex);
			nrf_delay_ms(100);
		}
	}else if(today % __feature1day2.time != 0){
		printf("\r\n ######2DAY FLASH DELETE");
		for(uint16_t pageindex = GC_FLASH_PAGE_START_DAY2_TIME ; pageindex <= GC_FLASH_PAGE_END_DAY2_TIME-15 ; pageindex+=GC_FLASH_SECTOR_NUM_TIME){
			gc_flash_erase(pageindex);
			nrf_delay_ms(100);
		}
	}else if(today % __feature1day3.time != 0){
		printf("\r\n ######3DAY FLASH DELETE");
		for(uint16_t pageindex = GC_FLASH_PAGE_START_DAY3_TIME ; pageindex <= GC_FLASH_PAGE_END_DAY3_TIME-15 ; pageindex+=GC_FLASH_SECTOR_NUM_TIME){
			gc_flash_erase(pageindex);
			nrf_delay_ms(100);
		}
	}
	
	
	
	
	if(__feature1day1.time == today){
		refSaveAddr = featuredata.time  - today + GC_FLASH_PAGE_START_TIME;
		printf("\r\n 1DAY FLASH SELECT");
	}else if(__feature1day2.time == today) {
		refSaveAddr = featuredata.time  - today + GC_FLASH_PAGE_START_DAY2_TIME;
		printf("\r\n 2DAY FLASH SELECT");
	}else if(__feature1day3.time == today){
		refSaveAddr = featuredata.time - today + GC_FLASH_PAGE_START_DAY3_TIME;
		printf("\r\n 3DAY FLASH SELECT");
	}else if(__feature1day1.time == -1 ){
		refSaveAddr = featuredata.time - today + GC_FLASH_PAGE_START_TIME;
		printf("\r\n 1DAY FLASH SELECT");
	}else if(__feature1day2.time == -1 ){
		refSaveAddr = featuredata.time - today + GC_FLASH_PAGE_START_DAY2_TIME;
		printf("\r\n 2DAY FLASH SELECT");
	}else if(__feature1day3.time == -1 ){
		refSaveAddr = featuredata.time - today + GC_FLASH_PAGE_START_DAY3_TIME;
		printf("\r\n 3DAY FLASH SELECT");
	}

	
	printf("\r\n Feature Save Time check %d %d %d", featuredata.time, today, refSaveAddr);
	uint8_t __data[GC_FLASH_PAGE_DATA_SIZE];
	memcpy(__data, &featuredata, sizeof(struct feature_tbl_form_s));
	gc_flash_fwrite(refSaveAddr, __data);
	
	if( refSaveAddr >= GC_FLASH_PAGE_START_TIME && refSaveAddr <= GC_FLASH_PAGE_END_TIME && __feature1day1.time == -1){
			printf("\r\n Dummy FirstAddress at Firstday");
			nrf_delay_ms(100);
			struct feature_tbl_form_s __DummyFeature;
			uint8_t __dataAnother[GC_FLASH_PAGE_DATA_SIZE];
			unsigned int  nowday = 0;
			if (gmt_offset >= 0){
				today = (get_GMT(time.year,  time.month,  time.day,  0,  0, 0 )/(60)) - gmt_offset;
			}else{
				today = (get_GMT(time.year,  time.month,  time.day,  0,  0, 0 )/(60)) + gmt_offset;
			}
			__DummyFeature.time = today;
			memcpy(__dataAnother, &__DummyFeature, sizeof(struct feature_tbl_form_s));
			gc_flash_fwrite(GC_FLASH_PAGE_START_TIME, __dataAnother);
	}
	if( refSaveAddr >= GC_FLASH_PAGE_START_DAY2_TIME && refSaveAddr <= GC_FLASH_PAGE_END_DAY2_TIME && __feature1day2.time == -1){
			printf("\r\n Dummy FirstAddress at Secondday");
			nrf_delay_ms(100);
			struct feature_tbl_form_s __DummyFeature;
			uint8_t __dataAnother[GC_FLASH_PAGE_DATA_SIZE];
			unsigned int  nowday = 0;
			if (gmt_offset >= 0){
				today = (get_GMT(time.year,  time.month,  time.day,  0,  0, 0 )/(60)) - gmt_offset;
			}else{
				today = (get_GMT(time.year,  time.month,  time.day,  0,  0, 0 )/(60)) + gmt_offset;
			}
			__DummyFeature.time = today;
			memcpy(__dataAnother, &__DummyFeature, sizeof(struct feature_tbl_form_s));
			gc_flash_fwrite(GC_FLASH_PAGE_START_TIME, __dataAnother);
	}
	if( refSaveAddr >= GC_FLASH_PAGE_START_DAY3_TIME && refSaveAddr <= GC_FLASH_PAGE_END_DAY3_TIME && __feature1day3.time == -1){
			printf("\r\n Dummy FirstAddress at Thirdday");
			nrf_delay_ms(100);
			struct feature_tbl_form_s __DummyFeature;
			uint8_t __dataAnother[GC_FLASH_PAGE_DATA_SIZE];
			unsigned int  nowday = 0;
			if (gmt_offset >= 0){
				today = (get_GMT(time.year,  time.month,  time.day,  0,  0, 0 )/(60)) - gmt_offset;
			}else{
				today = (get_GMT(time.year,  time.month,  time.day,  0,  0, 0 )/(60)) + gmt_offset;
			}
			__DummyFeature.time = today;
			memcpy(__dataAnother, &__DummyFeature, sizeof(struct feature_tbl_form_s));
			gc_flash_fwrite(GC_FLASH_PAGE_START_TIME, __dataAnother);
	}
	
	if (refSaveAddr % 16 == 0){
		printf("\r\n feature erase next Sector %d", refSaveAddr+16);
		gc_flash_erase(refSaveAddr+16);
	}else if(refSaveAddr==GC_FLASH_PAGE_END_DAY3_TIME){
		printf("\r\n feature erase next Sector %d", GC_FLASH_PAGE_START_TIME);
		gc_flash_erase(GC_FLASH_PAGE_START_TIME);
	}
	
	
	
	
	
	
	
	nrf_delay_ms(100);
	
	//refSaveAddr = featuredata.time 
		- (get_GMT(stdTime.year,  stdTime.month,  stdTime.day,  0,  0, 0 )/(60)) 
		+ GC_FLASH_PAGE_START_TIME;
	//show_feature(featuredata.time);
		
}*/

/*
void show_feature(unsigned int times) {
	tTime time;
	tTime stdTime;
	
	get_time(&time, 1);
	//convert_gmt_to_display_time(&time, &stdTime);
	printf("\r\n direct today???:%d %d %d %d %d %d", time.year, time.month, time.day , time.hour, time.minute, time.second);
	printf("\r\n direct today???:%d", (get_GMT(time.year,  time.month,  time.day,  0,  0, 0 )/(60)));
	
	
	
	extern volatile short gmt_offset;
	unsigned char offset_data[GC_FLASH_PAGE_SIZE];
		
	
	//규창 171031 플래시 주소 읽는 함수 구현이 다소 이상하다..
	//주소값이 16페이지를 기준으로 와리가리 할 가능성이 매우 높음
	//gc_flash_read(GC_FLASH_PAGE_GMT_OFFSET, offset_data );
	//gc_flash_page_read_data(GC_FLASH_TBL_TYPE_GMT_OFFSET,1,offset_data);
	printf("\r\n offset_data : %d,%d",offset_data[0],offset_data[1]);
	//gmt_offsets = offset_data[0] |  offset_data[1]<<8;
	unsigned int  today = 0;
	if (gmt_offset >= 0){
		today = (get_GMT(time.year,  time.month,  time.day,  0,  0, 0 )/(60)) - gmt_offset;
	}else{
		today = (get_GMT(time.year,  time.month,  time.day,  0,  0, 0 )/(60)) + gmt_offset;
	}
	printf("\r\n today:%d", today);
//	for(uint16_t __page=GC_FLASH_PAGE_START_TIME; __page <= (GC_FLASH_PAGE_END_DAY3_TIME+1); __page+=GC_FLASH_SECTOR_NUM_TIME) 
//	{
//		gc_flash_erase(__page);
//	}
	//(get_GMT(time.year,  time.month,  time.day,  time.hour,  time.minute, time.second )/(60))
	
	uint8_t		daycheck_buf[GC_FLASH_PAGE_SIZE];
	struct feature_tbl_form_s __feature1day1;
	struct feature_tbl_form_s __feature1day2;
	struct feature_tbl_form_s __feature1day3;
	
	memset(daycheck_buf, 0, GC_FLASH_PAGE_SIZE);
	gc_flash_read(GC_FLASH_PAGE_START_TIME, daycheck_buf );
	memcpy(&__feature1day1, daycheck_buf, sizeof(struct feature_tbl_form_s));
	
	
	memset(daycheck_buf, 0, GC_FLASH_PAGE_SIZE);
	gc_flash_read(GC_FLASH_PAGE_START_DAY2_TIME, daycheck_buf );
	memcpy(&__feature1day2, daycheck_buf, sizeof(struct feature_tbl_form_s));
	
	memset(daycheck_buf, 0, GC_FLASH_PAGE_SIZE);
	gc_flash_read(GC_FLASH_PAGE_START_DAY3_TIME, daycheck_buf );
	memcpy(&__feature1day3, daycheck_buf, sizeof(struct feature_tbl_form_s));
	
	unsigned int day1 = today - __feature1day1.time;
	unsigned int day2 = today - __feature1day2.time;
	unsigned int day3 = today - __feature1day3.time;
	unsigned int refSaveAddr = 0; 
	
	if(__feature1day1.time == today){
		refSaveAddr = times - today + GC_FLASH_PAGE_START_TIME;
		printf("\r\n 1DAY FLASH SELECT");
		}else if(__feature1day2.time == today) {
			refSaveAddr = times - today + GC_FLASH_PAGE_START_DAY2_TIME;
			printf("\r\n 2DAY FLASH SELECT");
		}else if(__feature1day3.time == today){
			refSaveAddr = times - today + GC_FLASH_PAGE_START_DAY3_TIME;
			printf("\r\n 3DAY FLASH SELECT");
		}else if(__feature1day1.time == -1 ){
			refSaveAddr = times - today + GC_FLASH_PAGE_START_TIME;
			printf("\r\n 1DAY FLASH SELECT");
		}else if(__feature1day2.time == -1 ){
			refSaveAddr = times - today + GC_FLASH_PAGE_START_DAY2_TIME;
			printf("\r\n 2DAY FLASH SELECT");
		}else if(__feature1day3.time == -1 ){
			refSaveAddr = times - today + GC_FLASH_PAGE_START_DAY3_TIME;
			printf("\r\n 3DAY FLASH SELECT");
		}
	uint8_t		r_page_buf[GC_FLASH_PAGE_SIZE];
	struct feature_tbl_form_s __featuredatalog;
	memset(r_page_buf, 0, GC_FLASH_PAGE_SIZE);
	printf("\r\n Feature Save Time check %d", refSaveAddr);
	gc_flash_read(refSaveAddr, r_page_buf );
	nrf_delay_ms(100);
	memcpy(&__featuredatalog, r_page_buf, sizeof(struct feature_tbl_form_s));
	printf("\r\n feature : %d", __featuredatalog.time);
	printf("\r\n feature : n_var:%d pressure:%d prediff:%d x_mean:%d y_mean:%d z_mean:%d n_mean:%d x_var:%d y_var:%d z_var:%d",
		__featuredatalog.norm_var, __featuredatalog.pressure, __featuredatalog.pre_diff_sum,__featuredatalog.x_mean, __featuredatalog.y_mean, __featuredatalog.z_mean,__featuredatalog.norm_mean,__featuredatalog.x_var ,__featuredatalog.y_var,	__featuredatalog.z_var);
	printf("\r\n feature : step:%d rope_cnt:%d s_swing:%d l_swing:%d",__featuredatalog.nStep,__featuredatalog.jumping_rope_count ,__featuredatalog.small_swing_count,__featuredatalog.large_swing_count);
	printf("\r\n feature : hb:%d",__featuredatalog.hb);
}*/



//규창 171102 피쳐 시간이 언제가 가장 최근자인지 확인후 저장

uint16_t writingAddr = 0;


void find_recent_feature_save_on_boot() {
	uint16_t 	__page=0;
	
	uint8_t		r_page_buf[GC_FLASH_PAGE_SIZE];
	
	struct feature_tbl_form_s __featuredatalog;
	memset(r_page_buf, 0, GC_FLASH_PAGE_SIZE);
	if (writingAddr == 0) {
		
		for(__page=GC_FLASH_PAGE_START_TIME; __page < (GC_FLASH_PAGE_END_DAY3_TIME+1); __page++) 
		{
			gc_flash_read(__page, r_page_buf );
				memcpy(&__featuredatalog, r_page_buf, sizeof(struct feature_tbl_form_s));
			if (__featuredatalog.time == -1){
				writingAddr = __page;
				printf("\r\n boot on writingAddr for feature[%d]", writingAddr);
				break;
			}
		}
 }
}





void find_recent_feature_save(struct feature_tbl_form_s featuredata) {
	printf("\r\n feature writingAddr[%d]", writingAddr);
	uint16_t 	__page=0;
	
	uint8_t		r_page_buf[GC_FLASH_PAGE_SIZE];
	
	struct feature_tbl_form_s __featuredatalog;
	memset(r_page_buf, 0, GC_FLASH_PAGE_SIZE);
	if (writingAddr == 0) {
		printf("\r\n feature writingAddr[%d]", writingAddr);
		for(__page=GC_FLASH_PAGE_START_TIME; __page < (GC_FLASH_PAGE_END_DAY3_TIME+1); __page++) 
		{
			gc_flash_read(__page, r_page_buf );
				memcpy(&__featuredatalog, r_page_buf, sizeof(struct feature_tbl_form_s));
			if (__featuredatalog.time == -1){
				writingAddr = __page;
				break;
			}
		}
 }
	
 printf("\r\n Save Feature Time %d at Addr %d", featuredata.time, writingAddr);
	uint8_t __data[GC_FLASH_PAGE_DATA_SIZE];
	memcpy(__data, &featuredata, sizeof(struct feature_tbl_form_s));
	gc_flash_fwrite(writingAddr, __data);
  writingAddr++;
	
	nrf_delay_ms(10);
	 if (writingAddr % 16 == 0){
			printf("\r\n feature erase next Sector %d", writingAddr+16);
			gc_flash_erase(writingAddr+16);
		 nrf_delay_ms(10);
	 }else if(writingAddr==GC_FLASH_PAGE_END_DAY3_TIME){
			printf("\r\n feature erase next Sector %d", GC_FLASH_PAGE_START_TIME);
			gc_flash_erase(GC_FLASH_PAGE_START_TIME);
		  writingAddr = GC_FLASH_PAGE_START_TIME;
			nrf_delay_ms(10);
	 }
	 
	//규창 171018 피쳐 저장루틴
	/*uint8_t __data[GC_FLASH_PAGE_DATA_SIZE];
	memcpy(__data, &featuredata, sizeof(struct feature_tbl_form_s));
	if (__page_index != GC_FLASH_PAGE_END_TIME){
		
		gc_flash_erase(__page_index+GC_FLASH_SECTOR_NUM_TIME);
		nrf_delay_ms(200);
		gc_flash_fwrite(writingAddr, __data);
	}else{
		gc_flash_erase(GC_FLASH_PAGE_START_TIME);
		nrf_delay_ms(200);
		gc_flash_fwrite(GC_FLASH_PAGE_START_TIME, __data);
	}
	nrf_delay_ms(100);*/
}



struct  feature_tbl_form_s find_req_feature(unsigned int req_time) {
	printf("\r\n BLE Through reqTime : %d", req_time);
	
	
	/*
	findAddr = writingAddr-20;
	
	if(findAddr < GC_FLASH_PAGE_START_TIME){
		uint16_t temp = GC_FLASH_PAGE_START_TIME - findAddr;
		 // findaddr == 1580 temp==20
		// 5919 - 20 == 5899
	   // findaddr == 1581 temp==19
		// 5919 - 19 == 5899
		// 
		findAddr = GC_FLASH_PAGE_END_DAY3_TIME - temp;
	}*/
	struct feature_tbl_form_s ret_data;
	uint8_t ret_page_buf[GC_FLASH_PAGE_SIZE];
	memset(ret_page_buf, 0, GC_FLASH_PAGE_SIZE);
	for (uint16_t i=0; i<21; i++){
		uint16_t findAddr = 0;
		findAddr = writingAddr-i;
		if(findAddr < GC_FLASH_PAGE_START_TIME){
			uint16_t temp = GC_FLASH_PAGE_START_TIME - findAddr;
			findAddr = GC_FLASH_PAGE_END_DAY3_TIME - temp;
		}
		gc_flash_read(findAddr, ret_page_buf );
		memcpy(&ret_data, ret_page_buf, sizeof(struct feature_tbl_form_s));
		if(ret_data.time == req_time){
			printf("\r\n find loop time %d", i); 
			break;
		}else {
			ret_data.time = req_time;
			ret_data.norm_var = 0;
			ret_data.x_var = 0;
			ret_data.y_var = 0;
			ret_data.z_var = 0;
			
			ret_data.x_mean = 0;
			ret_data.y_mean = 0;
			ret_data.z_mean = 0;
			ret_data.norm_mean = 0;
			
			ret_data.nStep = 0;
			ret_data.jumping_rope_count = 0;
			ret_data.small_swing_count = 0;
			ret_data.large_swing_count = 0;
			
			ret_data.pre_diff_sum = 0;
			
			ret_data.accumulated_step = 0;
			ret_data.display_step = 0;
			ret_data.pressure = 0;
			
			ret_data.hb = 0;
		}
	}
	//gc_flash_read(findAddr, ret_page_buf );
	//memcpy(&ret_data, ret_page_buf, sizeof(struct feature_tbl_form_s));
	
	/*ret_data.time = 0;
	ret_data.norm_var = 0;
	ret_data.x_var = 0;
	ret_data.y_var = 0;
	ret_data.z_var = 0;
	
	ret_data.x_mean = 0;
	ret_data.y_mean = 0;
	ret_data.z_mean = 0;
	ret_data.norm_mean = 0;
	
	ret_data.nStep = 0;
	ret_data.jumping_rope_count = 0;
	ret_data.small_swing_count = 0;
	ret_data.large_swing_count = 0;
	
	ret_data.pre_diff_sum = 0;
	
	ret_data.accumulated_step = 0;
	ret_data.display_step = 0;
	ret_data.pressure = 0;
	
	ret_data.hb = 0;*/
	
	return ret_data;
}


uint16_t findpastAddr = GC_FLASH_PAGE_START_TIME;


void findpaststartADDR(){
	unsigned int olderstFeaturetime = 1000000000;
	unsigned int lastFeaturetime = 0;
		struct feature_tbl_form_s ret_data;
	extern unsigned int pastfeature_request_firsttime;
	extern unsigned int pastfeature_request_lasttime;
	extern unsigned int pastfeature_request_time;
	extern uint16_t olderstAddr;
	uint16_t loopStartAddr = writingAddr+32;
	if(loopStartAddr >GC_FLASH_PAGE_END_DAY3_TIME){
			//loopStartAddr = (writingAddr+16 - GC_FLASH_PAGE_END_DAY3_TIME)+GC_FLASH_PAGE_START_TIME;
			loopStartAddr = GC_FLASH_PAGE_START_TIME;
	}
	bool loopEndAddr = false;
	uint8_t ret_page_buf[GC_FLASH_PAGE_SIZE];
	memset(ret_page_buf, 0, GC_FLASH_PAGE_SIZE);
	for (uint16_t i=GC_FLASH_PAGE_START_TIME; i<GC_FLASH_PAGE_END_DAY3_TIME+1; i++){
		gc_flash_read(i, ret_page_buf );
		memcpy(&ret_data, ret_page_buf, sizeof(struct feature_tbl_form_s));
			if (pastfeature_request_firsttime == 0){
				
					if((ret_data.time % 10 == 0) && i > loopStartAddr && loopEndAddr == false ){
						printf("\r\n finding olderstAddr %d", i); 
						
						
						olderstFeaturetime = ret_data.time;
						pastfeature_request_time =  ret_data.time;
						olderstAddr = i;
						printf("\r\n olderstFeaturetime %d, pastfeature_request_firsttime %d", olderstFeaturetime, pastfeature_request_firsttime);
						//pastfeature_request_time =  ret_data.time;
						loopEndAddr=true;
					}
					if((ret_data.time % 10 ==0) && ret_data.time > lastFeaturetime ){
						printf("\r\n find last time %d %d", i, ret_data.time); 
						extern uint16_t lastAddr;
						lastFeaturetime = ret_data.time;
						pastfeature_request_lasttime= ret_data.time;
						lastAddr = i;
					}
			} else {
					//printf("\r\n finding exactly %d", i); 
					if (ret_data.time == pastfeature_request_firsttime  - 10){
						printf("\r\n finding time %d at addr %d", ret_data.time, i); 
						pastfeature_request_time =  ret_data.time;
						olderstAddr = i;
					} else if (ret_data.time == pastfeature_request_firsttime ) {
						printf("\r\n finding time %d at addr  %d ", ret_data.time, i); 
						pastfeature_request_time =  ret_data.time;
						olderstAddr = i;
					}
					
					if((ret_data.time % 10 ==0) && ret_data.time > lastFeaturetime ){
						printf("\r\n find last time %d %d", i, ret_data.time); 
						extern uint16_t lastAddr;
						lastFeaturetime = ret_data.time;
						pastfeature_request_lasttime= ret_data.time;
						lastAddr = i;
					}
			}
	}
}

struct feature_tbl_form_s find_req_past_feature(uint16_t oldAddr,  unsigned int req_time){
	//printf("\r\n BLE Through reqTime : %d", req_time);
	
	
	/*
	findAddr = writingAddr-20;
	
	if(findAddr < GC_FLASH_PAGE_START_TIME){
		uint16_t temp = GC_FLASH_PAGE_START_TIME - findAddr;
		 // findaddr == 1580 temp==20
		// 5919 - 20 == 5899
	   // findaddr == 1581 temp==19
		// 5919 - 19 == 5899
		// 
		findAddr = GC_FLASH_PAGE_END_DAY3_TIME - temp;
	}*/
	//uint16_t olderstFeatureAddr = 0;
	struct feature_tbl_form_s ret_data;
	uint8_t ret_page_buf[GC_FLASH_PAGE_SIZE];
	memset(ret_page_buf, 0, GC_FLASH_PAGE_SIZE);
	//for (uint16_t i=GC_FLASH_PAGE_START_TIME; i<GC_FLASH_PAGE_END_DAY3_TIME+1; i++){
		gc_flash_read(oldAddr, ret_page_buf );
		memcpy(&ret_data, ret_page_buf, sizeof(struct feature_tbl_form_s));
		//f((ret_data.time % 10 ==0) && ret_data.time < olderstFeaturetime ){
			//printf("\r\n find loop time %d", i); 
			//olderstFeaturetime = ret_data.time;
			//olderstFeatureAddr = i;
		
	// ret_data.time = req_time;
		
	
		/*printf("\r\n olderst Feature time? %d",olderstFeaturetime); 
		printf("\r\n olderstFeatureAddr? %d",olderstFeatureAddr); 
		olderstFeaturetime = 1000000000;*/
		/*extern bool past_feature_flag;
		past_feature_flag = false;*/
	
	//findpastAddr = findpastAddr+GC_FLASH_SECTOR_NUM_TIME;
	
	//gc_flash_read(findAddr, ret_page_buf );
	//memcpy(&ret_data, ret_page_buf, sizeof(struct feature_tbl_form_s));
	
	/*ret_data.time = 0;
	ret_data.norm_var = 0;
	ret_data.x_var = 0;
	ret_data.y_var = 0;
	ret_data.z_var = 0;
	
	ret_data.x_mean = 0;
	ret_data.y_mean = 0;
	ret_data.z_mean = 0;
	ret_data.norm_mean = 0;
	
	ret_data.nStep = 0;
	ret_data.jumping_rope_count = 0;
	ret_data.small_swing_count = 0;
	ret_data.large_swing_count = 0;
	
	ret_data.pre_diff_sum = 0;
	
	ret_data.accumulated_step = 0;
	ret_data.display_step = 0;
	ret_data.pressure = 0;
	
	ret_data.hb = 0;*/
	
	return ret_data;
}





void savefeatureshow() {
	uint16_t 	__page=0, __page_index=0;
	uint32_t	page_time=0;
	uint16_t 	s_page=0, e_page=0;
	uint8_t		r_page_buf[GC_FLASH_PAGE_SIZE];
	uint8_t *req_buf;
	struct feature_tbl_form_s __featuredatalog;
	memset(r_page_buf, 0, GC_FLASH_PAGE_SIZE);
	
	/* set start/end page number according to table type */
	//s_page = gc_flash_page_start(table_type);
	s_page = GC_FLASH_PAGE_START_TIME;
	/*if (table_type < GC_FLASH_TBL_TYPE_SN)
		e_page = s_page + FLASH_PAGE_NUM_IN_SECTOR;
	else*/
		e_page = s_page;
	
	/* exact matching */
	for(__page=GC_FLASH_PAGE_START_TIME; __page < (GC_FLASH_PAGE_END_DAY3_TIME+1); __page++) 
	{
		__page_index = __page; /* FIXME */
		gc_flash_read(__page_index, r_page_buf );
		
		/*if (table_type >= GC_FLASH_TBL_TYPE_SN) //-hmlee 2016.08.19
		{*/
			memcpy(&__featuredatalog, r_page_buf, GC_FLASH_PAGE_SIZE);
		printf("\r\n Find Feature? __page_index: %d // saving GMT:%d ",__page_index, __featuredatalog.time);
		nrf_delay_ms(200);
		/*printf("\r\n feature : n_var:%d pressure:%d prediff:%d x_mean:%d y_mean:%d z_mean:%d n_mean:%d x_var:%d y_var:%d z_var:%d",
		__featuredatalog.norm_var, __featuredatalog.pressure, __featuredatalog.pre_diff_sum,__featuredatalog.x_mean, __featuredatalog.y_mean, __featuredatalog.z_mean,__featuredatalog.norm_mean,__featuredatalog.x_var ,__featuredatalog.y_var,	__featuredatalog.z_var);
		printf("\r\n feature : step:%d rope_cnt:%d s_swing:%d l_swing:%d",__featuredatalog.nStep,__featuredatalog.jumping_rope_count ,__featuredatalog.small_swing_count,__featuredatalog.large_swing_count);
		printf("\r\n feature : hb:%d",__featuredatalog.hb);*/
		
	}
}




void FlashAllErase() {
	uint16_t 	__page=0;
	

	/* exact matching */
	for(__page=GC_FLASH_PAGE_START_TIME; __page <= (GC_FLASH_PAGE_END_DAY3_TIME+1); __page+=GC_FLASH_SECTOR_NUM_TIME) 
	{
		gc_flash_erase(__page);
		
		/*if (table_type >= GC_FLASH_TBL_TYPE_SN) //-hmlee 2016.08.19
		{*/
			//memcpy(&__featuredatalog, r_page_buf, GC_FLASH_PAGE_SIZE);
		printf("\r\n now page_index: %d ",__page);
		nrf_delay_ms(200);
		/*printf("\r\n feature : n_var:%d pressure:%d prediff:%d x_mean:%d y_mean:%d z_mean:%d n_mean:%d x_var:%d y_var:%d z_var:%d",
		__featuredatalog.norm_var, __featuredatalog.pressure, __featuredatalog.pre_diff_sum,__featuredatalog.x_mean, __featuredatalog.y_mean, __featuredatalog.z_mean,__featuredatalog.norm_mean,__featuredatalog.x_var ,__featuredatalog.y_var,	__featuredatalog.z_var);
		printf("\r\n feature : step:%d rope_cnt:%d s_swing:%d l_swing:%d",__featuredatalog.nStep,__featuredatalog.jumping_rope_count ,__featuredatalog.small_swing_count,__featuredatalog.large_swing_count);
		printf("\r\n feature : hb:%d",__featuredatalog.hb);*/
		
	}
}







