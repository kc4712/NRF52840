/** @file
 *
 * @defgroup gc_ble_ancs.c
 * @{
 * @ingroup  
 * @brief    BLE Apple Notification Center Service
 *
 */

#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>

#include "ble_ancs_c.h"
#include "ble_db_discovery.h"
#include "nordic_common.h"
#include "nrf.h"
#include "app_error.h"
#include "ble_hci.h"
#include "ble_gap.h"
#include "ble_advdata.h"
#include "ble_advertising.h"
#include "ble_conn_params.h"
#include "device_manager.h"
#include "app_timer.h"
#include "pstorage.h"
#include "nrf_soc.h"
#include "bsp.h"
#include "bsp_btn_ble.h"
#include "softdevice_handler.h"
#include "nrf_delay.h"
#include "app_timer_appsh.h"
#include "nrf_log.h"
#include "ble_ancs_c.h"
#include "gc_ble_ancs.h"
#include "SEGGER_RTT.h"

extern uint8_t noti_num;
extern volatile short g_noti_count;

void gc_notification_ancs_handle(ble_ancs_c_evt_notif_t * p_notif)
{
	uint8_t	ancs_category;
	uint8_t	ancs_noti_event;
	uint8_t 	ancs_count;
	
	ancs_category 	=	p_notif->category_id;
	ancs_count				= 	p_notif->category_count;
	ancs_noti_event = 	p_notif->evt_id;
	
	switch(p_notif->category_id)
	{
		case GC_NOTI_ANCS_CTGRY_CALL_INCOME:
			if (ancs_noti_event == GC_NOTI_ANCS_EVENT_ADD)
			{
				printf("\r\n Incoming call-Add-count(%d)", ancs_count);
				noti_num = 7;
				g_noti_count = ancs_count;
			}
			else if (ancs_noti_event == GC_NOTI_ANCS_EVENT_RM)
			{
				printf("\r\n Incoming call-delete-count(%d)", ancs_count);
				noti_num = 20;
				g_noti_count = ancs_count;
			}
			else {}
		break;
				
		case GC_NOTI_ANCS_CTGRY_CALL_MISS:
			if (ancs_noti_event == GC_NOTI_ANCS_EVENT_ADD)
			{
				printf("\r\n Missed call-Add-count(%d)", ancs_count);
				noti_num = 8;
				g_noti_count = ancs_count;
			}
			break;
			
		case GC_NOTI_ANCS_CTGRY_CALL_SOCIAL:
			if (ancs_noti_event == GC_NOTI_ANCS_EVENT_ADD)
			{
				printf("\r\n Social-Add-count(%d)", ancs_count);
				noti_num = 9;
				g_noti_count = ancs_count;
			}
			else if (ancs_noti_event == GC_NOTI_ANCS_EVENT_RM)
			{
				printf("\r\n Social-delete-count(%d)", ancs_count);
				noti_num = 0;
				g_noti_count = ancs_count;
			}
			else {}
		break;
				
		default:
			//printf("\r\n No matched category");
			break;
		
	}	
}

