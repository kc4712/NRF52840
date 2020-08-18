
#include <stdint.h>
#include <string.h>
#include "nordic_common.h"
#include "nrf.h"
#include "nrf51_bitfields.h"
#include "ble_hci.h"
#include "ble_advdata.h"
#include "ble_conn_params.h"
#include "softdevice_handler.h"
#include "app_timer.h"
#include "app_button.h"
#include "ble_nus.h"
#include "simple_uart.h"
#include "boards.h"
#include "ble_error_log.h"
#include "ble_debug_assert_handler.h"
#include "app_util_platform.h"


#include "define.h"
#include "print.h"
#include "font.h"
#include "protocol.h"
#include "heart_beat.h"
#include "oled.h"
#include "rtc.h"
#include "afe4400.h"

#include "bma2x2.h"
#include "bmg160.h"

#include "greencom_feature.h"
#include "nrf_delay.h"


BLE_STATUS	m_ble_status;
BLE_TX_RAW_PACKET	m_ble_tx_raw_packet;
BLE_TX_HB_PACKET	m_ble_tx_HB_packet;
BLE_TX_RTC_PACKET	m_ble_tx_RTC_packet;
BLE_TX_TOTAL_NUMBER	m_ble_tx_TotalNumber_packet;

#if ___FITNESS_BLE_100MS___
BLE_TX_RAW_PACKET2		m_ble_tx_raw_packet2;
#endif

REQ_DATA_CMD	req_data_cmd;
REQ_NUMBER_CMD	req_number_cmd;
REQ_BATTERY_HB_CMD	req_batt_hb_cmd;
REQ_BASE_CMD	req_rtc_cmd;
REQ_SN_CMD		req_sn_cmd;
REQ_PERSONAL_INFO_CMD		req_persional_info_cmd;
REQ_BASE_CMD	req_acc_cmd;

extern int g_hb;
extern char g_product_mode;

#if 1 /* v1.3.1 hkim 2015.10.6 */
//unsigned int hr_alarm_start_indic=0; 
extern unsigned int g_time;
extern unsigned char hr_avg_ble;
#endif /* if 1 v1.3.1 */

#if 1 /* hkim 1.3.9.10 2016.1.26 세계시간 동기화 */	
short gmt_offset;								/* 특정지역의 GMT offset, 한국의 경우 GMT + 9h */
#endif

void protocol_init(void)
{

	memset(&m_ble_status, 0, sizeof(m_ble_status));
	/* connect 즉시 보내지 않기 위해 */
	m_ble_status.sending = 2;
	m_ble_status.send_enable = 0;

	memset(&m_ble_tx_raw_packet, 0, sizeof(m_ble_tx_raw_packet));
	m_ble_tx_raw_packet.stx = GREENCOM_STX;
	m_ble_tx_raw_packet.etx = GREENCOM_ETX;
	m_ble_tx_raw_packet.len = sizeof(m_ble_tx_raw_packet)-3;

#if ___FITNESS_BLE_100MS___
	memset(&m_ble_tx_raw_packet2, 0, sizeof(m_ble_tx_raw_packet2));
	m_ble_tx_raw_packet2.cmd = 'P';
#endif

	memset(&m_ble_tx_HB_packet, 0, sizeof(m_ble_tx_HB_packet));
	//m_ble_tx_HB_packet.stx = GREENCOM_STX;
	//m_ble_tx_HB_packet.etx = GREENCOM_ETX;
	m_ble_tx_HB_packet.cmd = 0x04;
	m_ble_tx_HB_packet.len = 7;	
	m_ble_tx_HB_packet.type = 2;


	m_ble_tx_RTC_packet.cmd=0x02;
	m_ble_tx_RTC_packet.type=0x02;
	m_ble_tx_RTC_packet.len=0x01;
	m_ble_tx_RTC_packet.status=0x04;


	m_ble_tx_TotalNumber_packet.cmd=0x0a;
	m_ble_tx_TotalNumber_packet.type=0x02;
	m_ble_tx_TotalNumber_packet.len=0x09;
	m_ble_tx_TotalNumber_packet.status=0x00;

	memset(&req_data_cmd, 0, sizeof(req_data_cmd));
	memset(&req_number_cmd, 0, sizeof(req_number_cmd));
	memset(&req_batt_hb_cmd, 0, sizeof(req_batt_hb_cmd));
	memset(&req_rtc_cmd, 0, sizeof(req_rtc_cmd));
	memset(&req_sn_cmd, 0, sizeof(req_sn_cmd));
	memset(&req_persional_info_cmd, 0, sizeof(req_persional_info_cmd));
	memset(&req_acc_cmd, 0, sizeof(req_acc_cmd));



	memset(&g_personal_info, 0xff, sizeof(g_personal_info));

}




extern int g_pressure;
extern unsigned char g_battery;

#if __HB_LOG__	
extern int g_HB_LOGBUF[];
extern int g_HB_LOG_active;
#endif			


int g_total_tx_busy_count=0;
int glob_elapsed_time=0; /* 1.3.9 hkim 2015.12.5 for debuging */

void protocol_process(ble_nus_t*	p_nus)
{
#if __GREENCOM_FITNESS__
	static char timeout_count=0;
	struct bma2x2_accel_data sample_xyz;
#endif
	//struct bmg160_data_t data_gyro;
	static int s_count=0;
#if ___FITNESS_BLE_100MS___
	static unsigned short ble_100ms_count=0;
#endif	
	//static float ssum=0;

	if(m_ble_status.connected)
	{


#if __GREENCOM_FITNESS__	// 피트니스  용 

#if 0 /* v1.3.9 hkim 2015.12.3 */
		if( !m_ble_status.sending &&  m_ble_status.send_enable)
#else
		if (m_ble_status.send_enable)
#endif 
		{		
			/* accessing the bma2x2acc_data parameter by using sample_xyz*/
			bma2x2_read_accel_xyz(&sample_xyz);/* Read the accel XYZ data*/

#if ___FITNESS_BLE_100MS___

			ble_100ms_count++;

			if(ble_100ms_count&1)
			{
				m_ble_tx_raw_packet2.acc_x0 = sample_xyz.x;
				m_ble_tx_raw_packet2.acc_y0 = sample_xyz.y;
				m_ble_tx_raw_packet2.acc_z0 = sample_xyz.z;

				m_ble_tx_raw_packet2.press0 = (short)(g_pressure-100000);

				m_ble_tx_raw_packet2.hb0 = g_sort_hb;
			}
			else
			{
				m_ble_tx_raw_packet2.acc_x1 = sample_xyz.x;
				m_ble_tx_raw_packet2.acc_y1 = sample_xyz.y;
				m_ble_tx_raw_packet2.acc_z1 = sample_xyz.z;

				m_ble_tx_raw_packet2.press1 = (short)(g_pressure-100000);

				m_ble_tx_raw_packet2.hb1 = g_sort_hb;
				m_ble_tx_raw_packet2.seq_number = (unsigned char)(ble_100ms_count>>1);

				m_ble_status.sending=true;
								//ks_printf("\r\n t1=%d",NRF_RTC1->COUNTER-glob_elapsed_time ); 
				ble_send( (uint8_t*)&m_ble_tx_raw_packet2  , sizeof(m_ble_tx_raw_packet2) );
							//glob_elapsed_time	= NRF_RTC1->COUNTER; 

			}


#else /* ___FITNESS_BLE_100MS___ */
			m_ble_tx_raw_packet.acc_x0 = sample_xyz.x>>8;
			m_ble_tx_raw_packet.acc_x1 = sample_xyz.x&0x00ff;

			m_ble_tx_raw_packet.acc_y0 = sample_xyz.y>>8;
			m_ble_tx_raw_packet.acc_y1 = sample_xyz.y&0x00ff;

			m_ble_tx_raw_packet.acc_z0 = sample_xyz.z>>8;
			m_ble_tx_raw_packet.acc_z1 = sample_xyz.z&0x00ff;
			

#if 0		// gyro 제거 	
			bmg160_get_data_XYZ(&data_gyro);/* Read the gyro XYZ data*/

			data_gyro.datax=data_gyro.datax*100;
			data_gyro.datay=data_gyro.datay*100;
			data_gyro.dataz=data_gyro.dataz*100;

			//m_ble_tx_raw_packet.gyro_x = data_gyro.datax;
			m_ble_tx_raw_packet.gyro_x0 = data_gyro.datax>>8;
			m_ble_tx_raw_packet.gyro_x1 = data_gyro.datax&0x00ff;
			
			//m_ble_tx_raw_packet.gyro_y = data_gyro.datay;
			m_ble_tx_raw_packet.gyro_y0 = data_gyro.datay>>8;
			m_ble_tx_raw_packet.gyro_y1 = data_gyro.datay&0x00ff;
			
			//m_ble_tx_raw_packet.gyro_z = data_gyro.dataz;
			m_ble_tx_raw_packet.gyro_z0 = data_gyro.dataz>>8;
			m_ble_tx_raw_packet.gyro_z1 = data_gyro.dataz&0x00ff;

//			ssum+=(float)data_gyro.dataz/10.0;

			//ks_printf("\r\n%1.1f %1.1f %1.1f              *%1.1f",(float)data_gyro.datax/10.0 , (float)data_gyro.datay/10.0, (float)data_gyro.dataz/10.0 , ssum );
#endif /* #if 0		*/

			m_ble_tx_raw_packet.press_0 = g_pressure>>24;
			m_ble_tx_raw_packet.press_1 = g_pressure>>16;
			m_ble_tx_raw_packet.press_2 = g_pressure>>8;
			m_ble_tx_raw_packet.press_3 = g_pressure;

			//ks_printf("\r\n p = %x,  %02x %02x %02x %02x",g_pressure, m_ble_tx_raw_packet.press_0, m_ble_tx_raw_packet.press_1, m_ble_tx_raw_packet.press_2, m_ble_tx_raw_packet.press_3  );

			m_ble_tx_raw_packet.hb = g_sort_hb;
			
				m_ble_status.sending=true;
			
			ble_send( (uint8_t*)&m_ble_tx_raw_packet  , sizeof(m_ble_tx_raw_packet) );
			
#endif /* ___FITNESS_BLE_100MS___ */
		
			timeout_count = 0;
		}
		else
		{
			if( m_ble_status.send_enable)
			{
			printf("\r\nTX busy %d",timeout_count);
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
#endif /* __GREENCOM_FITNESS__ */

#if __GREENCOM_24H__	// 24H  용 
		s_count++;

		if(  (s_count%1200)==0 )
		{		
#if __GREENCOM_FITNESS__
			timeout_count = 0;
#endif
		}

#endif
		
	}
	else
	{
#if __GREENCOM_FITNESS__	
		timeout_count=0;
#endif
		s_count=0;
	}


}


extern unsigned short g_step;
extern unsigned char is_uart_mode; 	/* v1.4.10.0 hkim 2016.02.12 */
unsigned char stop_feature_process; /* v1.4.11.1 hkim 2016.02.17 */
#if __GREENCOM_24H__
void greencom_feature_process(unsigned int count)
{
	struct bma2x2_accel_data sample_xyz;
//	char s[20];
	
#if 1 /* v1.4.10.0 hkim 2016.02.12 */
	if (is_uart_mode == true)	return;
#endif
#if 1 /* v1.4.11.1 hkim 2016.02.17 */
	if (stop_feature_process == true)	return;
#endif
	
	bma2x2_read_accel_xyz(&sample_xyz);/* Read the accel XYZ data*/

	//ks_printf("\r\n%d %d %d", sample_xyz.x, sample_xyz.y, sample_xyz.z );

#ifndef __GREENCOM_24H_BT_SENSOR_TEST__
	get_realtime_feature(count,sample_xyz.x, sample_xyz.y, sample_xyz.z );
#endif

	AFE4400_current_control(count);



/*
	sprintf(s,"%d  ",g_step);
	TextOut(0, 1,s);
*/	

}
#endif

#if 1 /* v1.3.5 hkim 2015.10.28 */
extern unsigned short g_total_day_jumprope;
#endif

#if 1 /* v1.3.9 hkim 2015.12.05 */
extern unsigned int g_accumulated_step;
#endif
void greencom_battery_hb_send_process(ble_nus_t*  p_nus)
{
	m_ble_tx_HB_packet.battery = g_battery;
#if 0 /* v1.3.1 hkim 2015.10.6 */
	m_ble_tx_HB_packet.hb = g_sort_hb;
#endif 
	m_ble_tx_HB_packet.adc_lsb = NRF_ADC->RESULT;
	m_ble_tx_HB_packet.step = g_total_daystep;
#if 1 /* v1.3.5 hkim 2015.10.28 */
	m_ble_tx_HB_packet.jumping_rope_count	= g_total_day_jumprope;
#endif 
#if 1 /* v1.3.9 hkim 2015.12.05 */
m_ble_tx_HB_packet.accumulated_step = g_accumulated_step;
#endif
	if(nrf_gpio_pin_read(BATTERY_CHARGE_DETECT_INPUT)==0)
		m_ble_tx_HB_packet.status = 1;
	else
	{
		if(g_battery==100)
			m_ble_tx_HB_packet.status = 3;
		else
			m_ble_tx_HB_packet.status = 2;
	}

	m_ble_status.sending=true;

#if 1 /* v1.3.1 hkim 2015.10.6 */
	m_ble_tx_HB_packet.hb = hr_avg_ble;
#endif /* if 1 v1.2.5 */

	ble_send((uint8_t*)&m_ble_tx_HB_packet  , sizeof(m_ble_tx_HB_packet));

	req_batt_hb_cmd.cmd=0;
}


void set_day_step(void);

void greencom_rtc_response_send_process(ble_nus_t*  p_nus)
{
	tTime time;

#if 0 /* v1.2.4  because of low memory :( hkim 2015.09.30 */	
	ks_printf("\r\nrtc response");
#endif 
	m_ble_status.sending=true;

#if 0 /* 1.3.8 hkim 2015.11.23 */
	get_time( &time, 0);
#else
	if (get_time( &time, 1) == false)
		return;
#endif 

	m_ble_tx_RTC_packet.year = time.year;
	m_ble_tx_RTC_packet.month = time.month;
	m_ble_tx_RTC_packet.day = time.day;
	m_ble_tx_RTC_packet.hour = time.hour;
	m_ble_tx_RTC_packet.minute = time.minute;
	m_ble_tx_RTC_packet.second = time.second;

	if(req_rtc_cmd.cmd==2)
		set_day_step();

	ble_send((uint8_t*)&m_ble_tx_RTC_packet  , sizeof(m_ble_tx_RTC_packet));

	req_rtc_cmd.cmd=0;
}

void greencom_feature_send_process(ble_nus_t*  p_nus)
{
	static struct feature_tbl_form_s load_data[5];
	static unsigned short read_page=0;
	static unsigned char send_state=0;
	unsigned int time;
	
#if __GREENCOM_24H__
	if(m_ble_status.connected)
	{

		if(req_data_cmd.start_trigger==1)
		{

			read_page = get_feature_data_page(  req_data_cmd.time, &time  );

			if(time==0xffffffff)
			{
				ks_printf("\r\n no matching gmt_time");
				req_data_cmd.start_trigger=0;
				return;
			}

			if(	req_data_cmd.number<5 || req_data_cmd.number>60)
			{
				ks_printf("\r\n out of range");
				req_data_cmd.start_trigger=0;
				return;
			}
		
			req_data_cmd.start_trigger=2;
			req_data_cmd.done_number=0;
			send_state=0;
		}

		switch(send_state)
		{
		case 0:
			if(req_data_cmd.done_number>=req_data_cmd.number   )
			{
				req_data_cmd.start_trigger=0;
				return;
			}

			if(read_page==get_write_page_pointer())	// 현재 시간 넘게 요청 
			{
				req_data_cmd.start_trigger=0;
				return;
			}
			
			feature_flash_read(read_page, load_data );
			send_state=1;
			break;

		case 1:
			bl_st_feature_make_data(&load_data[0]);
		case 2:
		case 3:
			bl_st_feature_report_sequence(p_nus, send_state+(req_data_cmd.done_number*3));
			send_state++;
			break;

		case 4:
			bl_st_feature_make_data(&load_data[1]);
		case 5:
		case 6:	
			bl_st_feature_report_sequence(p_nus, send_state+(req_data_cmd.done_number*3));
			send_state++;
			break;

		case 7:
			bl_st_feature_make_data(&load_data[2]);
		case 8:
		case 9:	
			bl_st_feature_report_sequence(p_nus, send_state+(req_data_cmd.done_number*3));
			send_state++;
			break;

		case 10:
			bl_st_feature_make_data(&load_data[3]);
		case 11:
		case 12:
			bl_st_feature_report_sequence(p_nus, send_state+(req_data_cmd.done_number*3));
			send_state++;
			break;

		case 13:
		case 14:	
			bl_st_feature_make_data(&load_data[4]);
		case 15:
			bl_st_feature_report_sequence(p_nus, send_state+(req_data_cmd.done_number*3));
			send_state++;
			break;

		case 16:
			req_data_cmd.done_number+=5;
			send_state=0;
			read_page++;
			break;
		
		}
		
	}
	else
		req_data_cmd.start_trigger=0;
	
#endif

}






void greencom_total_number_send_process(ble_nus_t*  p_nus)
{
	unsigned short read_page=0;
	short total_number;
	unsigned int ret_time;
	struct feature_tbl_form_s feature_data[5]; // v1.2.3 09.24 skim
	
#if __GREENCOM_24H__
	if(m_ble_status.connected)
	{

		if(req_number_cmd.start_trigger==1)
		{

			read_page = get_feature_data_page(  req_number_cmd.time , &ret_time );

			//ks_printf("\r\n read_page=%d",read_page);

			if(ret_time==0xffffffff)
			{
				ks_printf("\r\n no matching gmt_time");
				total_number=0;
			}
			else
			{
				total_number = get_write_page_pointer()-read_page ;
				if(total_number<0)
					total_number=total_number+MAX_DB_PAGE;
			}

			m_ble_tx_TotalNumber_packet.number=((unsigned int)total_number)*5;
			m_ble_tx_TotalNumber_packet.time=ret_time;
			
#if 1 /* v1.2.3 09.24 skim */
			read_page++;
			if(read_page==MAX_DB_PAGE)
				read_page=0;
			
			feature_flash_read(read_page, feature_data );
			m_ble_tx_TotalNumber_packet.next_time=feature_data[0].time;

			
//			ks_printf("\r\ntotal number response n=%d t=%d nt=%d",m_ble_tx_TotalNumber_packet.number, 
																//m_ble_tx_TotalNumber_packet.time, m_ble_tx_TotalNumber_packet.next_time );
#endif /* if 1 */
			m_ble_status.sending=true;


			ble_send((uint8_t*)&m_ble_tx_TotalNumber_packet  , sizeof(m_ble_tx_TotalNumber_packet));

			req_rtc_cmd.cmd=0;

		

			req_number_cmd.start_trigger=0;
		}

	}
#endif

}


extern char	g_serial_number_valid;

void greencom_sn_response_send_process(ble_nus_t*  p_nus)
{
	char	flash_str[256];
	BLE_TX_SN	ble_tx_sn;

	Flash_Write_Text(FLASH_SERIAL_NUMBER_PAGE, req_sn_cmd.sn );

	memset(&ble_tx_sn, 0 ,sizeof(ble_tx_sn));
	
	Flash_Read_Text(FLASH_SERIAL_NUMBER_PAGE, flash_str );

	if(strcmp(req_sn_cmd.sn, flash_str)==0)
	{
		ble_tx_sn.result=4;
	}
	else
		ble_tx_sn.result=3;

	ble_tx_sn.cmd=0x05;
	ble_tx_sn.type=0x02;
	ble_tx_sn.len=16;
	
	ks_printf("\r\nsn_write:%s  sn_read:%s", req_sn_cmd.sn, flash_str);

	memcpy(ble_tx_sn.sn, flash_str, 15 );

	ble_send((uint8_t*)&ble_tx_sn  , sizeof(ble_tx_sn));

	req_sn_cmd.cmd = 0;
	g_serial_number_valid=true;
}



void greencom_personal_info_response_send_process(ble_nus_t*  p_nus)
{
	BLE_TX_BASE_PACKET	ble_tx_packet;

	ble_tx_packet.cmd=0x03;
	ble_tx_packet.type=0x02;
	ble_tx_packet.len=1;
	ble_tx_packet.status=4;

	ble_send((uint8_t*)&ble_tx_packet  , sizeof(ble_tx_packet));

	req_persional_info_cmd.cmd = 0;
}


void greencom_acc_info_response_process(ble_nus_t*  p_nus)
{
	BLE_TX_ACC	ble_tx_packet;
	struct bma2x2_accel_data sample_xyz;


	bma2x2_read_accel_xyz(&sample_xyz);/* Read the accel XYZ data*/

	ble_tx_packet.cmd=0x09;
	ble_tx_packet.type=0x02;
	ble_tx_packet.len=(11);
	ble_tx_packet.status=0;

#if 0 /* v1.3.3 hkim bug fix */
	ble_tx_packet.x = g_pressure;
#else
	ble_tx_packet.pressure = g_pressure;
#endif 
	ble_tx_packet.x = sample_xyz.x;
	ble_tx_packet.y = sample_xyz.y;
	ble_tx_packet.z = sample_xyz.z;

	ble_send((uint8_t*)&ble_tx_packet  , sizeof(ble_tx_packet));

	req_acc_cmd.cmd = 0;
}





int ble_receive_handler(unsigned char* pdata, unsigned short len)
{
#if 1 //hkim-debug
  int i=0;
	ks_printf("\r\n ** BT Rx **");
	ks_printf("\r\n");
	for (i=0; i < len; i++)
		ks_printf("%02x ", pdata[i]);
#endif 
	
	switch(pdata[0])
	{
#if __GREENCOM_24H__

/*
	// only for Test
	case 'A':
		req_data_cmd.time = 223200;
		req_data_cmd.number = 15;
		req_data_cmd.done_number=0;
		req_data_cmd.start_trigger=1;
		ks_printf("\r\n REQ_feature_data %d %d",req_data_cmd.time, req_data_cmd.number );
		return true;
*/				
	case 0x0a:
	case 0x0b:
		if(pdata[1]==0x03 && pdata[2]==0x08 && len==11)
		{
			if(req_data_cmd.start_trigger==0)
			{
				req_data_cmd.time = pdata[3]<<0 | pdata[4]<<8 | pdata[5]<<16 | pdata[6]<<24;
				req_data_cmd.number = pdata[7]<<0 | pdata[8]<<8 | pdata[9]<<16 | pdata[10]<<24;
				req_data_cmd.done_number=0;
				req_data_cmd.start_trigger=1;
				ks_printf("\r\n REQ_feature_data %d %d",req_data_cmd.time, req_data_cmd.number );
				return true;
			}
			else
				ks_printf("\r\n req_feature_data busy" );
		}
		else if(pdata[1]==0x01 && pdata[2]==0x04 && len==7)
		{
		
			if(req_number_cmd.start_trigger==0)
			{
				req_number_cmd.time = pdata[3] | pdata[4]<<8 | pdata[5]<<16 | pdata[6]<<24;
				req_number_cmd.start_trigger=1;
				//ks_printf("\r\n REQ_total_number  time=%d", req_number_cmd.time );
				return true;
			}
			else
				ks_printf("\r\n req_total_number busy" );
			
		}

		break;

	case 0x02:	// RTC 정보 
#if 0 /* hkim 1.3.9.10 2016.1.26 세계시간 동기화 */
		if(len!=11 )
			return false;
#endif
		return ble_rcv_rtc(pdata,len);

	case 0x03:	// personal info
		if(len!=7 )
			return false;
		return ble_rcv_personal_info(pdata,len);

	case 0x09:	// acc info
		if(pdata[1]==0x01 && pdata[2]==0x0 && len==3)
		{
			req_acc_cmd.cmd = 1;
		
			return true;
		}
		break;

	case 0x11:
		if(len>=2)
		{
			if(pdata[1]==0x01) 
			{
				ks_printf("\r\n erase feature data");
				feature_flash_erase(0);
				nrf_delay_ms(200);
				feature_flash_erase(MAX_DB_PAGE-1);
				nrf_delay_ms(200);
				NVIC_SystemReset();
			}
		}
		break;
#ifdef __GREENCOM_24H_BT_SENSOR_TEST__ /* hkim 0729 */
	case 0x12:	// acc sensor test using BT
		return ble_rcv_acc_xyz_info(pdata,len);
#endif 

#else

	case 0x10:	// raw enable
		if(len!=3 )
			return false;
		return ble_send_enable(pdata,len);

#endif

	case 0x07:	// motor control
		if(len>=2)
		{
			if(pdata[1]==0x01) 
				nrf_gpio_pin_set(MOTOR_ENABLE_PIN);
			else
				nrf_gpio_pin_clear(MOTOR_ENABLE_PIN);
		
			return true;
		}
		break;

	case 0x08:	// product mode
		if(len>=2)
		{
			if(pdata[1]==0x02 ) 
				g_product_mode=true;
			else if(pdata[1]==0x01 ) 
			{
				g_product_mode=false;
				NVIC_SystemReset(); 
			}
			return true;
		}
		break;

	case 0x04:	// battery, hb 정보 
		if(pdata[1]==0x01 && pdata[2]==0x0 && len==3)
		{
			req_batt_hb_cmd.cmd = 1;
		
			return true;
		}
		break;

	case 0x05:	// sn
		if(pdata[1]==0x01)
		{
			return ble_sn(pdata,len);
		}
		break;

	case 0x0d:	// version v1.2.3 09.23 skim
		if(pdata[1]==0x01)
		{
			char s[20];

			memset(s,0,sizeof(s)); /* hkim v1.3.9.10,  10 ---> 16 */

			s[0]=0x0d;
			s[1]=0x01;
			s[2]=strlen(PRODUCT_VERSION); /* hkim v1.3.9.10 ,  10 ---> 16 */
			s[3]=0x04;
			
			sprintf(&s[4],"%s",PRODUCT_VERSION);
			ble_send( (uint8_t*)s  , 4 + strlen(PRODUCT_VERSION) ); /* hkim v1.3.9.10,   10 ---> 16 */
		}
		break;
	case 0x14: /* v1.3.9 hkim 2015.12.13 */
		{
			char s[20];
			int	__result=0;
			struct flash_step_info step_info;
			
			memset(s,0,sizeof(s));
			memset(&step_info,0,sizeof(struct flash_step_info));
			
			s[0]=0x14;
			s[1]=0x02;
			s[2]=0x08;
			
			__result = get_step_info_from_flash(FLASH_ACCUMULATED_STEP_PAGE, pdata[3],&step_info);
			if (__result == false)
			{
				s[3] = 0x03;
				s[4] = 0x7fffffff; 
				s[8] = 0x7fffffff;
			}
			else
			{
				s[3] = 0x04;
				s[4] = step_info.time;
				s[8] = step_info.step;
			}
			ks_printf("\r\n [BLE_SEND] result,time,step %d (%x %x %x %x) (%x %x %x %x)",
																s[3],
																s[4],s[5],s[6],s[7],
																s[8],s[9],s[10],s[11]);
			ble_send( (uint8_t*)s  , 12 );
		}
	default:

		break;

	}


	return false;
}

#if 0 /* move to front v1.2.5 2015.10.4  */
extern unsigned int g_time;
#endif 

int ble_rcv_rtc(unsigned char* pdata, unsigned short len)
{
#if (!__BOARD2__  &&  !__BOARD3__)
	unsigned int total_sec;
#endif
	int year;
	short __gmt_offset; /* v1.4.10.0 hkim 2016.02.16 */

#if 0 /* hkim 1.3.9.10 2016.1.26 세계시간 동기화 */
	if(pdata[1]!=1 || pdata[2]!=8)
		return false;
#endif

	year = pdata[3] |  pdata[4]<<8;
	
#if 0 /* hkim 1.3.9.10 2016.1.26 세계시간 동기화 */		
	if(year<2015 && year>2038)
		return false;
#else
		if(year<2000 && year>2038)
		return false;
#endif 

	if(pdata[5]<1 && pdata[5]>12)
		return false;

	if(pdata[6]<1 && pdata[6]>31)
		return false;

	if( pdata[7]>23)
		return false;

	if( pdata[8]>=60)
		return false;

	if( pdata[9]>=60)
		return false;	

#if 1 /* hkim 1.3.9.10 2016.1.26 세계시간 동기화. offset 은 분이다. 예를 들어 9시간이면 540 이 내려온다 */
	gmt_offset = pdata[11] | pdata[12] << 8;
	//ks_printf("Protocol : gmt_offset : %x %d", gmt_offset, gmt_offset); 
#endif

#if (__BOARD2__ || __BOARD3__)

	if(is_rtc_valid()==false)
		req_rtc_cmd.cmd=2;
	/* GMT 시간을 RTC에 설정 */
	rtc_set_time(year, pdata[5], pdata[6], pdata[7], pdata[8], pdata[9]);
	//ks_printf("\r\nrtc=%d %d %d %d %d %d",year, pdata[5], pdata[6], pdata[7], pdata[8], pdata[9]);

	read_gmt_offset_from_flash(FLASH_GMT_OFFSET_PAGE, &__gmt_offset);
	//ks_printf("\r\n GMT offset current / protocol %d %d", __gmt_offset, gmt_offset);
	if (__gmt_offset != gmt_offset)
	{
		save_gmt_offset(gmt_offset);   /* v1.4.10.0 hkim 2016.02.16 */
	}
#else

	total_sec = get_GMT(year,pdata[5],   pdata[6], pdata[7], pdata[8], pdata[9]     );

#if 0 /* hkim 1.3.9.10 2016.1.26 세계시간 동기화 */	
	g_time = total_sec*20;
#else
	g_time = (total_sec + (gmt_offset * 60)) * 20;
#endif

#endif

	if(req_rtc_cmd.cmd==false)
	req_rtc_cmd.cmd=1;

	return true;
}


extern int g_total_send_count;

int ble_send_enable(unsigned char* pdata, unsigned short len)
{

	if(pdata[1]==0x01)
	{
		m_ble_status.send_enable = 1;


	}
	else if(pdata[1]==0x02)
	{
		m_ble_status.send_enable = 0;

		ks_printf("\r\n tx_busy_count=%d  total_send_count=%d",g_total_tx_busy_count, g_total_send_count );
		g_total_tx_busy_count=0;
		g_total_send_count=0;		
	}

	

	return true;
}

int ble_sn(unsigned char* pdata, unsigned short len)
{

	memset(&req_sn_cmd,0,sizeof(req_sn_cmd));

	memcpy(req_sn_cmd.sn, &pdata[3], 15);
	req_sn_cmd.sn[15]=0;
	req_sn_cmd.cmd = true;
	

	return true;
}

#if 1 /* v1.2.5 enable motor if hr is over than 80% of max hr. hkim 2015.10.3 */
unsigned short personal_max_hb;
#endif 
int ble_rcv_personal_info(unsigned char* pdata, unsigned short len)
{

	g_personal_info.sex = pdata[3];
	g_personal_info.age = pdata[4];
	g_personal_info.height = pdata[5];
	g_personal_info.weight = pdata[6];

	req_persional_info_cmd.cmd = true;
#if 1 /* v1.2.5 enable motor if hr is over than 80% of max hr. hkim 2015.10.3 */
	personal_max_hb = (220 - g_personal_info.age) * 0.7;
	//ks_printf("\r\n** Age %d, Max hr  %d",  g_personal_info.age, personal_max_hb);
#endif 
	return true;
}

#ifdef __GREENCOM_24H_BT_SENSOR_TEST__ /* hkim 0729 */
unsigned int bt_acc_xyz_count=1;
int ble_rcv_acc_xyz_info(unsigned char* pdata, unsigned len)
{
	short bt_acc_x=0, bt_acc_y=0, bt_acc_z=0;
	
	memcpy(&bt_acc_x, &pdata[4], 2);
	memcpy(&bt_acc_y, &pdata[6], 2);
	memcpy(&bt_acc_z, &pdata[8], 2);
	
	ks_printf("\r\n BT x/y/z %d, %d, %d(bt_count %d)", bt_acc_x, bt_acc_y, bt_acc_z, bt_acc_xyz_count);
	
	get_realtime_feature(bt_acc_xyz_count, bt_acc_x, bt_acc_y, bt_acc_z);
	bt_acc_xyz_count++;
	return true;
}
#endif  /* __GREENCOM_24H_BT_SENSOR_TEST__ */
