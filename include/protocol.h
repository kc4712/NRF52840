

#include "ble_nus.h"


#define	GREENCOM_STX		0xc8
#define	GREENCOM_ETX		0xc9

typedef struct
{
	char call;
	char	SMS;
}NOTI_SWITCH;


typedef struct
{
	char		connected;
	char 	sending;
	char		send_enable;
	
} BLE_STATUS;



typedef __packed struct
{

	unsigned char		stx;
	unsigned char		len;

	unsigned char		acc_x0;
	unsigned char		acc_x1;
	unsigned char		acc_y0;
	unsigned char		acc_y1;
	unsigned char		acc_z0;
	unsigned char		acc_z1;	

	unsigned char		gyro_x0;
	unsigned char		gyro_x1;
	unsigned char		gyro_y0;
	unsigned char		gyro_y1;
	unsigned char		gyro_z0;
	unsigned char		gyro_z1;

	unsigned char		press_0;
	unsigned char		press_1;
	unsigned char		press_2;
	unsigned char		press_3;

	unsigned char		hb;
	
	unsigned char		etx;
	
} BLE_TX_RAW_PACKET;




typedef __packed struct
{
	unsigned char		cmd;
	unsigned char		seq_number;

	short			acc_x0;
	short			acc_y0;
	short			acc_z0;
	short			press0;

	short			acc_x1;
	short			acc_y1;
	short			acc_z1;
	short			press1;
	
	unsigned char		hb0;
	unsigned char		hb1;
	
} BLE_TX_RAW_PACKET2;












typedef __packed struct
{

	unsigned char		cmd;
	unsigned char		type;
	unsigned char		len;

	unsigned char		status;
	unsigned char		adc_msb;
	unsigned char		adc_lsb;
	unsigned char		battery;

	unsigned char		hb;
	
	unsigned short	step;
#if 1 /* v1.3.5 hkim 2015.10.28 */
	unsigned short	jumping_rope_count; 
#endif 
#if 1 /* v1.3.9 hkim 2015.12.05 */
	unsigned int accumulated_step;
#endif 	
} BLE_TX_HB_PACKET;


typedef __packed struct
{

	unsigned char		cmd;
	unsigned char		type;
	unsigned char		len;
	unsigned char		status;

} BLE_TX_BASE_PACKET;

typedef __packed struct
{

	unsigned char		cmd;
	unsigned char		type;
	unsigned char		len;
	unsigned char		status;

	unsigned short	year;
	unsigned char		month;
	unsigned char		day;
	unsigned char		hour;
	unsigned char		minute;
	unsigned char		second;

} BLE_TX_RTC_PACKET;


typedef __packed struct
{

	unsigned char		cmd;
	unsigned char		type;
	unsigned char		len;
	unsigned char		status;
	unsigned int		number;
	unsigned int		time;
	unsigned int		next_time; //v1.2.3 09.24 skim
	

} BLE_TX_TOTAL_NUMBER;


typedef __packed struct
{

	unsigned char		cmd;
	unsigned char		type;
	unsigned char		len;
	unsigned char		result;
	unsigned char		sn[15];
	
} BLE_TX_SN;

typedef __packed struct
{

	unsigned char		cmd;
	unsigned char		type;
	unsigned char		len;
	unsigned char		status;
	int				pressure;
	short			x;
	short			y;
	short			z;
	
} BLE_TX_ACC;


typedef  struct
{

	int time;
	int number;

	int done_number;
	unsigned char start_trigger;
	
} REQ_DATA_CMD;


typedef  struct
{
	int time;

	unsigned char start_trigger;
	
} REQ_NUMBER_CMD;

typedef  struct
{

	unsigned char cmd;
	
} REQ_BATTERY_HB_CMD;


typedef  struct
{

	unsigned char cmd;
	
} REQ_BASE_CMD;


typedef  struct
{

	unsigned char cmd;
	char sn[16];
	
} REQ_SN_CMD;

typedef  struct
{

	unsigned char cmd;
	
} REQ_PERSONAL_INFO_CMD;








/* proto type define */
void protocol_init(void);
void protocol_process(ble_nus_t* );


void greencom_feature_process(unsigned int count);
void greencom_feature_send_process(ble_nus_t*  p_nus);
void greencom_battery_hb_send_process(ble_nus_t*  p_nus);
void greencom_rtc_response_send_process(ble_nus_t*  p_nus);
void greencom_total_number_send_process(ble_nus_t*  p_nus);
void greencom_sn_response_send_process(ble_nus_t*  p_nus);
void greencom_personal_info_response_send_process(ble_nus_t*  p_nus);
void greencom_acc_info_response_process(ble_nus_t*  p_nus);


int ble_receive_handler(unsigned char* pdata, unsigned short len);
int ble_rcv_rtc(unsigned char* pdata, unsigned short len);
int ble_send_enable(unsigned char* pdata, unsigned short len);
int ble_sn(unsigned char* pdata, unsigned short len);
int ble_rcv_personal_info(unsigned char* pdata, unsigned short len);
	
void ble_send( uint8_t* data, uint16_t length );


extern REQ_DATA_CMD		req_data_cmd;
extern REQ_BATTERY_HB_CMD	req_batt_hb_cmd;
extern REQ_BASE_CMD	req_rtc_cmd;
extern REQ_NUMBER_CMD	req_number_cmd;
extern REQ_SN_CMD	req_sn_cmd;
extern REQ_PERSONAL_INFO_CMD		req_persional_info_cmd;
extern BLE_STATUS	m_ble_status;
extern REQ_BASE_CMD	req_acc_cmd;


void read_gmt_offset_from_flash(unsigned short  page, void* data );
int save_gmt_offset(short __gmt_offset);
void feature_flash_erase(unsigned short  page);
void save_gmt_offset_to_flash(unsigned short  page, void* data );
int ble_rcv_acc_xyz_info(unsigned char* pdata, unsigned len);
