#ifndef _GC_ANCS_H_
#define _GC_ANCS_H_

#include "ble_ancs_c.h"


#define GC_NOTI_ANCS_CTGRY_CALL_INCOME		1
#define GC_NOTI_ANCS_CTGRY_CALL_MISS					2
#define GC_NOTI_ANCS_CTGRY_CALL_SOCIAL			4

#define GC_NOTI_ANCS_EVENT_ADD				0
#define GC_NOTI_ANCS_EVENT_MODI			1
#define GC_NOTI_ANCS_EVENT_RM				2


void gc_notification_ancs_handle(ble_ancs_c_evt_notif_t * p_notif);

#endif 
