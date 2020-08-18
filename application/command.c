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
#include "define.h"
#include  "scc.h"
#include "nrf_drv_twi.h"
#include "gc_twi.h"
#include "command.h" 
#include "print.h"
#include "bma2x2.h"
#include "oled.h"
#include "rtc.h"
#include "gc_time.h"
#include "spi_flash.h"
#include "font.h"
#include "flash_control.h"
#include "spi_flash.h"
#include "gc_gpio.h"
#include "gc_sleep.h"
#ifdef GC_NRF52_PAHR
	#include "pah8002.h"
	//#include "pah8002_dd_vendor_1.h"
	#include "pah8002_comm.h"
#else
	#include "afe4400.h"
#endif
#include "an520.h"
#include "display.h"
#include "flash_control.h"
#include "gc_activity.h"

unsigned char oled_test_mode =0;
extern volatile int32_t gc_saadc;
//extern int g_pressure;
extern volatile uint8_t pahr_mode;
extern volatile uint8_t g_battery;
extern volatile unsigned char is_charging;
volatile unsigned char hr_dp = 0;
volatile unsigned int hr_test_count;
static SCC_RX_EXECUTE		scc_rx_execute={0,};
extern volatile struct ACTIVITY_INFO act_info;
extern volatile struct s_hart_rate HR_info;
//규창 콘솔 hw테스트용 플래그 pah8002.c 선언
extern volatile bool testPPG;

const COMMAND_DEFINE cmddefine[] = {
	#if 0 //hkim-1007
	{"dump",		cmd_dump},
	{"edit",		cmd_edit},
	
	{"reset",		cmd_reset},
	{"reset_boot",		cmd_reset_boot},

	{"blet", cmd_nus_ble_send},
	{"spi",		cmd_spi},
	{"twir",		cmd_twi_read},
	{"twiw",		cmd_twi_write},
	{"gpioread",		cmd_gpio_read_all},
	{"gpiowrite",		cmd_gpio_write},
	{"gpiodefault",		cmd_gpio_reset},
	{"clock",		cmd_clock},	
	{"acc",			cmd_acc},
	{"pressure",		cmd_pressure},
	{"oledpower",		cmd_oledpower},
	{"dp", cmd_dp},
	{"dp_clock", cmd_dp_clock},
	{"dp_step", cmd_dp_step},
	{"dp_hr", cmd_dp_hr},
	{"dp_act", cmd_dp_act},
	{"dp_act", cmd_dp_act},
	{"dp_coach", cmd_dp_coach},
	{"dp_stress", cmd_dp_stress},
	{"dp_call", cmd_dp_call},
	{"dp_unanswer", cmd_dp_unanswer},
	{"dp_sms", cmd_dp_sms},
	{"dp_sns", cmd_dp_sns},
	{"icon", cmd_icon},
	{"op", cmd_op},
	{"noti", cmd_noti},
	{"accpower",		cmd_accpower},
	{"flashpower",		cmd_flashpower},
	{"hrpower", cmd_hrpower},
	{"poweroff",		cmd_poweroff},
	{"txpower",	cmd_txpower},
	{"oled_test",		cmd_oled_test_mode},
	{"charge",		cmd_charge},
	{"bat",		cmd_battery},
	{"pahr",		cmd_pah8002_burst_read},		/* PAH8002 read ( < 255) */
	{"pahlr",		cmd_pah8002_loop_read},		/* PAH8002 read ( < 255) */
	{"pahw",		cmd_pah8002_write},								/* PAH8002 write 1 byte */
	{"pahcs",		cmd_pah8002_checksum},			/* PAH8002 4-channel raw data checksum */
	{"pahdump", cmd_dump_pah8002},	/* dump AFE4400 register */
	{"sensor",		cmd_sensor},

	/** @brief  flash command **/
	{"fread",		cmd_fread},
	{"fwrite",		cmd_fwrite},	
	{"ferase",	cmd_ferase},
	{"ftwrite", cmd_flash_table_write}, /* write step/kcal/act table dummy data to flash */
	{"spitx", cmd_spi_tx}, //GC_NRF52_PAHR
	
	{"xx",		cmd_bmd101},
	{"yy",	cmd_motion},
	{"walk",		cmd_walk},
	{"stack",		cmd_stack},
	{"time", 		cmd_time},
#else /* if 0 */
#ifndef _FIT_ETRI_TEST_ 
	{"reset",		cmd_reset},
	{"reset_boot",		cmd_reset_boot},
	{"fread",		cmd_fread},
	{"fwrite",		cmd_fwrite},	
	{"ferase",	cmd_ferase},
	{"feraseall",		cmd_feraseall},	
	{"oledpower",		cmd_oledpower},
	{"op", cmd_opmode},
	{"noti", cmd_noti},
	{"hwtest", cmd_hwtest},
	{"ver", cmd_version},
	{"poweroff", cmd_poweroff},
	{"getslpinfo", cmd_getslpinfo},
	{"help",		cmd_help},
	{"?",			cmd_help},
#else /* #ifndef _FIT_ETRI_TEST_  */
	#if 0 
	{"1.  H/W check         : H/W confirmation test", cmd_etrihw},
	{"2.  flashread         : Read from flash device", cmd_etri_flash_read},
	{"3.  flashwrite        : Write data to flash device", cmd_etri_flash_write},
	{"4.  flasherase        : Erase flash device", cmd_etri_flash_erase},
	{"5.  flashsleep        : Enter flash device into sleep ", cmd_etri_flash_sleep},
	{"6.  flashwakeup       : Wake-up flash device from sleep", cmd_etri_flash_wakeup},
	{"7.  accsleep          : Enter acceleration sensor into sleep", cmd_etri_acc_sleep},
	{"8.  accwakeup         : Wake-up acceleration sensor from sleep", cmd_etri_acc_wakeup},
	{"9.  hrsleep           : Enter heart rate sensor into sleep", cmd_etri_hr_sleep},
	{"10. hrtouch           : Enter heart rate sensor into touch recognition mode", cmd_etri_hr_touch},
	{"11. hrnormal          : Enter heart rate sensor into normal operation mode", cmd_etri_hr_normal},
	{"12. mcuoff            : Power down of MCU", cmd_etri_mcu_poweroff},
	#else
	{"1.    H/W confirmation", cmd_etrihw},
	{"2.    read flash", cmd_etri_flash_read},
	{"3.    write data to flash", cmd_etri_flash_write},
	{"4.    erase flash", cmd_etri_flash_erase},
	{"5.    sleep flash device", cmd_etri_flash_sleep},
	{"6.    wake-up flash", cmd_etri_flash_wakeup},
	{"7.    sleep accel sensor", cmd_etri_acc_sleep},
	{"8.    wake-up accel sensor", cmd_etri_acc_wakeup},
	{"9.    sleep HR sensor", cmd_etri_hr_sleep},
	{"10.   HR sensor touch mode", cmd_etri_hr_touch},
	{"11.   HR sensor normal mode", cmd_etri_hr_normal},
	{"12.   MCU power down", cmd_etri_mcu_poweroff},
	{"13.   Battery", cmd_etri_remain_battery},
	{"?     command list",			cmd_help},
	#endif
#endif /* #ifndef _FIT_ETRI_TEST_  */
#endif /* if 0 */
	{NULL,			NULL}
};



void initialize_cmd_variable(void)
{
	scc_rx_execute.pointer=0;
}




void debug_shell(unsigned char c)
{
	SCC_RX_EXECUTE *str;
	str=&scc_rx_execute;	
	
	if(ks_get_command(str,c))
	{
		command_execute(str->buffer);
#ifndef _FIT_ETRI_TEST_
		printf("\n\rcoach_plus>");
#else
		printf("\n\rETRI>");
#endif
	}
}



int ks_get_command(SCC_RX_EXECUTE *scc_rx_execute, unsigned char c)
{
	char inchar;


	inchar = (char)c;

	//while( uart_get(&inchar) )
	{
		
		if(inchar=='\b'){
			if(scc_rx_execute->pointer){
				scc_rx_execute->pointer--;
				printf("\b \b");
			}
		}
		else{
			if(inchar=='\r' || inchar=='\n'){
				scc_rx_execute->buffer[scc_rx_execute->pointer]=0;
				scc_rx_execute->pointer=0;
				return true;
			}
			if(scc_rx_execute->pointer<SCC_RX_EXECUTE_BUFFER_SIZE-2){
				scc_rx_execute->buffer[scc_rx_execute->pointer]=inchar;
				scc_rx_execute->pointer++;
				printf("%c",inchar);
			}
		}
	}

	return false;	
}

	

int cmd_parse(char *cmd,char **argv)
{
	char *cptr;
	int argc=0;

	cptr = strtok(cmd," \t,\r\n*");
	if(cptr)
		argv[argc++] = cptr;
	else
		return NO;
    
	while((cptr = strtok(NULL," \t,\r\n*"))!=0 ){
		argv[argc++] = cptr;
	}
  
	return argc;
}


int tolower(int c)
{
	if(c>='A' && c<='Z')
		return c+'a'-'A';
	
	return c;
}


int strncmpi(char* s1, char* s2, int len)
{
	while(*s1) {

		if( tolower(*s1) > tolower(*s2) )
			return 1;
		else if( tolower(*s1) < tolower(*s2) )
			return -1;

		if( -- len == 0 )
			return 0;

		s1 ++;
		s2 ++;
	}
	
	if(*s2 == '\0')
		return 0;

	return -1;
}

		
void command_execute(char* buffer)
{
	int length,i;
	int argc;
	char* argv[30];
	
	argc = cmd_parse( buffer, argv );

	if( argc > 0 ){
		length = strlen( argv[0] );
		
		for( i = 0; cmddefine[i].name; i++ ) {
			if( !strncmpi( cmddefine[i].name, argv[0], length ) )
				break;
		}

		if( cmddefine[i].function ) {
			cmddefine[i].function( argc, argv );
   		}
   		else{
			printf("\r\n%s ?", argv[0]);
   		}
    			
	}
}

void DumpMemory(void* pMemory,int count)
{
	int  i, k;
	unsigned char a;
	unsigned long start,start1;    
	unsigned long end;     

	start=start1=(unsigned long)pMemory;
	end=start+(count-1); /* hkim 2016.7.23 count --> (count-1) */

	for(k=0;k<32;k++){
		printf("\n\r%08lx:",start);
    
		for(i=0;i<start%16;i++) printf( "   " );
		
		for( ; i < 16; i++){
			a=*(unsigned char *)start;
			printf(" %02x",a);
			start++;
			if(start>end){
				i++;
				break;
			}
		}
	
		for( ; i < 16; i++) printf("  ");
		printf("  ");

		for(i=0;i<start1%16;i++) printf(" ");

		for( ;i<16;i++){
			a=*(unsigned char *)start1;
			
			if( ' ' <= a && a <= 'z' ){
				printf("%c",a);
				start1++;
			}
			else
			{
				printf(".");
				start1++;
			}

			if(start1>end) break;
		}
		

		if( i != 16 ) break;
	}
}

#if 0 //hkim-1007
int cmd_dump( int argc, char* argv[])
{
	unsigned long start;    
	unsigned long end;    
	static unsigned long last_dumped=0; 
	char *pseudo;

	if( argc < 2 ){
		start = last_dumped;
		end   = start + 0xff;
	}
	else if( argc >= 2 ){
		start = strtoul(argv[1],&pseudo,16);
		if( argc < 3 )
			end   = start + 0xff;
		else
			end = strtoul(argv[2],&pseudo,16);
	}

    DumpMemory((unsigned char*)start,end-start);   
        
	last_dumped = end+1;
	
	return true;
}

			
int cmd_edit(int argc,char* argv[])
{
/*
	unsigned long start;    
 	SCC_RX_EXECUTE *str;
 	char *pseudo;
 	           
	if( argc < 2 ){
		printf( "\n\n\rEdit(Store) an integer value at the memory");
		printf( "\n\n\rUsage : Edit [Start Address]\n\r");
		return NO;
	}
	
	start = strtoul(argv[1],&pseudo,16);
	str=&scc_rx_execute;
	while(1){
		printf("\n\r%08lx: ",start);
		printf("%08x -> ",*(unsigned long *)start);
		
		while(!ks_get_command(str));
    
		if(*(str->buffer)== '.') break;
		else if(*(str->buffer))
			*(unsigned int *)start=strtoul(str->buffer,&pseudo,16);
			
		start+=4;
	}
*/	
	return true;
}

int cmd_reset( int argc, char* argv[])
{
//	uint32_t err_code;
	printf("\r\ncommand reset");
	//NRF_POWER->GPREGRET = 0xB1;
	//err_code = bootloader_dfu_start();
  //APP_ERROR_CHECK(err_code);
	save_data_before_reset(); /*16.12.20 for save_before_reset hm_lee*/
	nrf_delay_ms(10); /* FIXME */
	save_info_before_reset_point(5,0,0,0,0);	/*16.12.14 for test_log hm_lee*/
	NVIC_SystemReset();		/* include nrf.h */
	
	return true;
}

int cmd_reset_boot( int argc, char* argv[])
{
	NRF_POWER->GPREGRET = 0xB1;
	//	uint32_t err_code;
	printf("\r\ncommand reset boot");
	//NRF_POWER->GPREGRET = 0xB1;
	//err_code = bootloader_dfu_start();
  //APP_ERROR_CHECK(err_code);
	save_data_before_reset(); /*16.12.20 for save_before_reset hm_lee*/
	nrf_delay_ms(10); /* FIXME */
	save_info_before_reset_point(5,0,0,0,0);	/*16.12.14 for test_log hm_lee*/
	NVIC_SystemReset();		/* include nrf.h */
}

int cmd_nus_ble_send(int argc,char* argv[])
{
	if(argc != 1)
	{
		printf("\n\r Invalid input");
		return false;
	}
	__cmd_nus_ble_send();
	return true;
}

int cmd_spi( int argc, char* argv[])
{

	
	return TRUE;
}

#ifdef GC_NRF52_PAHR
int cmd_pah8002_write( int argc, char* argv[])
{
	int32_t result=0;
	uint8_t address, data;
	//int8_t *pseudo;

	if(argc<3)
	{
		printf("\n\n pah8002 [address] [data]");	
		return false;
	}

	//address = strtoul(argv[1],&pseudo,16);
	address = strtoul(argv[1],NULL,16);
	data 			= strtoul(argv[2],NULL,16);

	result = pah8002_write_reg(address, data);
	if (result == 0)
		printf("\r\n Write 0x%x to 0x%x. done", data, address);
	else
		printf("\r\n Write 0x%x to 0x%x. fail", data, address);
	
	return true;
}


int cmd_pah8002_burst_read( int argc, char* argv[])
{
	int32_t ret=0;
	uint32_t rx_length=0;
	uint8_t address;
	//int8_t *pseudo;
	uint8_t pah8002_spi_burst_rx_buff[256];
	
	if(argc<3)
	{
		printf("\r\n Usage : pahr [address] [size]");	
		return false;
	}
		
	address = (uint8_t)strtoul(argv[1],NULL,16);
	rx_length = atoi(argv[2]);
	
	if (rx_length > 255)
	{
		printf("\r\n max length is 255");
		return false;
	}
	
	memset(pah8002_spi_burst_rx_buff, 0, sizeof(pah8002_spi_burst_rx_buff));
#ifdef GC_COACH_FIT_PCB_PAHR_IF_TWI 
	ret = pah8002_burst_read_reg(address, pah8002_spi_burst_rx_buff, rx_length,0);
#else 	
	/* rx_data of SPI response include 1 byte length of tx_data. 
	    So that reason, request rx_legnth is rx_length+1.
			Real rx_data is started from *(rx_data+1) not *(rx_data+0) */
	ret = pah8002_burst_read_reg(address, pah8002_spi_burst_rx_buff, rx_length+1,0);
#endif 
	if (ret != NRF_SUCCESS)
	{
		printf("\n\n read addr fail. stop");
		return false;
	}
#ifdef GC_COACH_FIT_PCB_PAHR_IF_TWI 
	DumpMemory(pah8002_spi_burst_rx_buff, rx_length);
#else
	DumpMemory(pah8002_spi_burst_rx_buff, rx_length+1);
#endif

	return true;
}

int cmd_pah8002_loop_read( int argc, char* argv[])
{
	int32_t ret=0;
	uint32_t total_rx_length=0, segment=0;
	uint8_t address;
	//int8_t *pseudo;

	if(argc !=4)
	{
		printf("\r\n Usage : pahlr [address] [total size] [segment]");	
		return false;
	}
		
	address = (uint8_t)strtoul(argv[1],NULL,16);
	total_rx_length = atoi(argv[2]);
	segment = atoi(argv[3]);
	#if 0 
	if (total_rx_length < 255)
	{
		printf("\r\n length should be over 255");
		return false;
	}
	#endif
	
	if ( (total_rx_length % segment) != 0 )
	{
		printf("\r\n total length can't divided with segment(%d)", segment);
		return false;
	}

	ret = pah8002_burst_read_reg(address, NULL, total_rx_length, segment);
	if (ret != NRF_SUCCESS)
	{
		printf("\n\n read addr fail. stop");
		return false;
	}

	return true;
}

int cmd_pah8002_checksum( int argc, char* argv[])
{
	if(argc != 1)
	{
		printf("\n\n pahcs");
		return false;
	}
	
	pah8002_raw_data_checksum();
	
	return true;
}

int cmd_dump_pah8002( int argc, char* argv[])
{
	/* PPG 1,23
		  Dump register of PAH8002 which has been configured to PPG mode.
			183 register is configured by pah8002_normal_mode_init().
		  Because of lack of UART buffer size, read 3 times for different register part */
	if(argc<3)
	{
		printf("\n\n pahdump <touch/normal> [1/2/3]");	
		return false;
	}
	
	//__print_pah8002_reg_val(argv[1], atoi(argv[2])); //hkim-temporaly 1007
	
	return true;
}

#else /* GC_NRF52_PAHR */
int cmd_afe4400_read( int argc, char* argv[])
{
	int address, data=0;
	char *pseudo;
	int i;
	int	check_status=0;
	
	if(argc<2)
	{
		printf("\n\afer [address]  (address 0 => show all)");	
	
		return FALSE;
	}

	address = strtoul(argv[1],&pseudo,16);

	if(address==0)
	{
			#if 1
			for(i=1;i<=30;i++)
			{
					data =  AFE4400_Read_Data(i);
					printf("\n\r[0x%02x] = 0x%02x", i, data);

					// 생산 코드 
					if(i==1)
					{
							if(data != AFE4400_REG1)
								check_status++;
					}
					else if(i==2)
					{
							if(data!=AFE4400_REG2)
								check_status++;
					}
					else if(i==3)
					{
							if(data!=AFE4400_REG3)
								check_status++;
					}
					else if(i==4)
					{
							if(data!=AFE4400_REG4)
								check_status++;
					}
				}
				#endif

				printf("\n\r");

				for(i=32;i<=35;i++)
				{
					data =  AFE4400_Read_Data(i);
					printf("\n\r[0x%02x] = 0x%02x", i, data);				
				}

				printf("\n\r");

				for(i=41;i<=48;i++)
				{
					data =  AFE4400_Read_Data(i);
					printf("\n\r[0x%02x] = 0x%02x", i, data);				
				}


				if(check_status==0)
					printf("\n\rAFE4400_STATUS OK");	
				else
					printf("\n\rAFE4400_STATUS FAIL");
			
				return true;
		}
		else /* not read all */
		{
				data =  AFE4400_Read_Data(address);
				printf("\n\rread : [0x%02x] = 0x%02x", address, data);
				return true;
		}

}

int cmd_afe4400_write( int argc, char* argv[])
{
	int address, data=0;
	char *pseudo;
	
	if(argc<3)
	{
		printf("\n\afew [address] [data]");
	
		return FALSE;
	}

	address = strtoul(argv[1],&pseudo,16);

	data = strtoul(argv[2],&pseudo,16);

	AFE4400_Write_Data(address, data);
	data =  AFE4400_Read_Data(address);
	printf("\n\rwrite [0x%02x] = 0x%02x", address, data);
	return true;
}
#endif /* hkim 2016.7.7 */

int cmd_sensor( int argc, char* argv[])
{

	
	return true;
}

int cmd_fread( int argc, char* argv[])
{
	unsigned char buf[255];
	char *pseudo;
	int address;

	memset(buf,0,sizeof(buf));

	if(argc<2)
	{
		printf("\r\nfread [adress(hex)]");
		return true;
	}

	address = strtoul(argv[1],&pseudo,16);
	
	SPI_FLASH_BufferRead(buf, address );

	DumpMemory(buf+4,256);
	//printf("\r\n %x %x %x %x %x", buf[0],buf[1],buf[2],buf[3],buf[4]);

	return true;
}

int cmd_fwrite( int argc, char* argv[])
{
	unsigned char buf[260];
	char *pseudo;
	int address;
	
	if(argc<3)
	{
		printf("\r\nfwrite [page adress(hex)] [string]");
		return true;
	}

	address = strtoul(argv[1],&pseudo,16);

	memset(buf, 0xff, sizeof(buf));
	
	memcpy(buf+4, argv[2], strlen(argv[2]) );
	
	SPI_FLASH_PageWrite(buf, address );
	
	return true;
}

int cmd_ferase( int argc, char* argv[])
{
	char *pseudo;
	int address;
//	int _sector=0; /* v1.4.11.1 */
	
	if(argc<2)
	{
		printf("\r\nferase [sector adress(4k) or all]");
		return true;
	}
#if 1 /* v1.4.11.1 hkim 2016.2.20 */
	address = strtoul(argv[1],&pseudo,16);
	
	SPI_FLASH_SectorErase(address );
#else
	if (!strcmp(argv[1], "all"))
	{
		#if 0 /* hkim v1.5.16.0 */
		stop_feature_process = true;	
		#endif 
		
		/* 모든 섹터 삭제 */
		for (_sector = 0; _sector < SECTOR_ALL_NUM; _sector++)
		{
			address = _sector * SECTOR_SIZE;
			printf("\r\nerase sector : %x", address);
			SPI_FLASH_SectorErase(address );
			nrf_delay_ms(200);
		}
		init_db_write_page();
		
		#if 0 /* hkim v1.5.16.0 */
		stop_feature_process = false;
		#endif 
	}
	else
	{
		address = strtoul(argv[1],&pseudo,16);	
		SPI_FLASH_SectorErase(address );
	}
#endif 
	
	return true;
}

extern volatile uint16_t f_page_index_write_pos_step;
int cmd_flash_table_write(  int argc, char* argv[])
{
	uint32_t _start,_end=0, _page=0;
	uint8_t req_data[GC_FLASH_PAGE_DATA_SIZE]; /* 4: address */
	
	memset(req_data, GC_FLASH_NO_DATA, sizeof(req_data));
	
	_GC_TBL_STEP				_tbl_step;
	_GC_TBL_CALORIE	_tbl_kcal;
	_GC_TBL_MEASURE_ACTIVITY		_tbl_act;
	
	if(argc != 4)
	{
		printf("\r\ftwrite <step/kcal/act/sn/offset> <page start index> <page end index>");
		return true;
	}
	
	_start = (uint32_t)atoi(argv[2]);
	_end = (uint32_t)atoi(argv[3]);
	
	/* initialize structure */
	memset(&_tbl_step, 0, sizeof(_GC_TBL_STEP));
	memset(&_tbl_kcal, 0, sizeof(_GC_TBL_CALORIE));
	memset(&_tbl_act, 0, sizeof(_GC_TBL_MEASURE_ACTIVITY));
	memset(req_data, 0xff, sizeof(req_data));
	
	if (!strcmp(argv[1], "step"))
	{
		if ( (_start > GC_FLASH_PAGE_END_STEP) || (_end > GC_FLASH_PAGE_END_STEP) )
		{
			printf("\r\n Invalid range %d/%d. exit", _start, _end);
			return false;
		}
		
		for (_page=_start; _page < _end+1; _page++)
		{
			/* create dummy data */
			_tbl_step.date	= 1;
			_tbl_step.step	= _page << 8;
			memcpy(req_data, &_tbl_step, sizeof(_GC_TBL_STEP));

			gc_flash_page_write_data(GC_FLASH_TBL_TYPE_STEP, req_data);
			nrf_delay_ms(10); /* FIXME */
		}
	}
	else	if (!strcmp(argv[1], "kcal"))
	{
		if ( (_start < GC_FLASH_PAGE_START_KCAL) || (_start > GC_FLASH_PAGE_END_KCAL) || (_end > GC_FLASH_PAGE_END_KCAL) )
		{
			printf("\r\n Invalid range %d/%d. exit", _start, _end);
			return false;
		}
		
		for (_page=_start; _page < _end+1; _page++)
		{
			/* create dummy data */
			_tbl_kcal.date		= 1;
			_tbl_kcal.daily		= _page << 8;
			_tbl_kcal.sports	= _page << 8;
			_tbl_kcal.sleep		= _page << 8;
			_tbl_kcal.coach	= _page << 8;

			memcpy(req_data, &_tbl_kcal, sizeof(_GC_TBL_CALORIE));

			gc_flash_page_write_data(GC_FLASH_TBL_TYPE_KCAL, req_data);
			nrf_delay_ms(10); /* FIXME */
		}
	}
	else	if (!strcmp(argv[1], "act"))
	{
		if ( (_start < GC_FLASH_PAGE_START_ACT) || (_start > GC_FLASH_PAGE_END_ACT) || (_end > GC_FLASH_PAGE_END_ACT) )
		{
			printf("\r\n Invalid range %d/%d. exit", _start, _end);
			return false;
		}
		
		for (_page=_start; _page < _end+1; _page++)
		{
			/* create dummy data */
			_tbl_act.time									= 1;
			_tbl_act.calorie						= _page << 8;
			_tbl_act.intensity_high	= _page << 8;
			_tbl_act.intensity_mid		= _page << 8;
			_tbl_act.intensity_low		= _page << 8;
			_tbl_act.hr_max						= _page << 8;
			_tbl_act.hr_min							= _page << 8;
			_tbl_act.hr_avg							= _page << 8;
			
			memcpy(req_data, &_tbl_act, sizeof(_GC_TBL_MEASURE_ACTIVITY));

			gc_flash_page_write_data(GC_FLASH_TBL_TYPE_ACT, req_data);
			nrf_delay_ms(10); /* FIXME */
		}
	}
	else	if (!strcmp(argv[1], "sn"))
	{
		strncpy(req_data, argv[2], strlen(argv[2]));
				
		gc_flash_page_write_data(GC_FLASH_TBL_TYPE_SN, req_data);
	}
		else	if (!strcmp(argv[1], "offset"))
	{
		uint16_t __offset=0;
		
		__offset = (uint16_t)atoi(argv[2]);
		memcpy(req_data, &__offset, sizeof(uint16_t));
				
		gc_flash_page_write_data(GC_FLASH_TBL_TYPE_GMT_OFFSET, req_data);
	}
	else {}
	return true;
}

int cmd_spi_tx(int argc, char* argv[])
{
	char *pseudo;
	uint32_t value, len;
	
	if(argc<3)
	{
		printf("\r\n spiw [value] (value : 1 byte)");
		return true;
	}
	value = strtoul(argv[1],&pseudo,16);
	len = atoi(argv[2]);

	printf("\r\n CMD SPI-Tx : value 0x%x, Len %d", value, len);
	
	__cmd_spi_tx(value, len);
	return true;
}	

#ifdef GC_NRF52_TWI
extern const nrf_drv_twi_t m_twi_master_1; 
#ifdef GC_COACH_FIT_PCB_PAHR_IF_TWI 
  extern const nrf_drv_twi_t m_twi_master_2; 
#endif

int cmd_twi_read( int argc, char* argv[])
{
	uint8_t rx_buf[400];
	uint32_t 	read_count=0;
	char			*ptr;
	long			slave_dev_addr=0, reg_addr=0;
	
	memset(rx_buf, 0, sizeof(rx_buf));
		
	if (argc != 4)
	{
		printf("\n\rUsage    : twir <slave addr(hex)> <reg(hex)> <number(dec)>");
		printf("\n\rExample : twir 0x18 0x00 1"); 
		printf("\r\n********** 7bit TWI slave address Info **********");
		printf("\r\n BMA255/OLED/RTC/Baro/PAH8002 : 0x18, 0x3C, 0x51, 0x76, 0x15");
		return false;
	}
		
	slave_dev_addr = strtol(argv[1], &ptr,16);
	reg_addr = strtol(argv[2], &ptr,16);
	read_count = atoi(argv[3]);
	
	if (read_count > sizeof(rx_buf))
	{
		printf("\r\n Max length is %d", sizeof(rx_buf));
		return false;
	}
		
	printf("\r\n Input : slave 0x%x, reg_addr 0x%x, len %d", slave_dev_addr, reg_addr, read_count);
	if ( gc_twi_read((uint8_t)slave_dev_addr, (uint8_t)reg_addr, rx_buf, read_count) != NRF_SUCCESS )
		return false;
	
	DumpMemory(rx_buf, read_count);

		return true;
}

int cmd_twi_write( int argc, char* argv[])
{
	char			*ptr;
	long			slave_dev_addr=0, reg_addr=0;
	uint8_t 	input_data;
	
	/* if register address is 0xff, it is ignaored */
	if (argc != 4)
	{
		printf("\n\rUsage    : twiw <slave addr(hex)> <register addr(hex)> <data(hex)> --> reg_addr & data is only 1 Byte");
		printf("\n\rExample : twiw 0x18 0x01 0xff"); 
		printf("\r\n********** TWI slave address Info **********");
		printf("\r\n BMA255/OLED/RTC/Baro/PAH8002 : 0x18, 0x3C, 0x51, 0x76, 0x15");
		return false;
	}
		
	slave_dev_addr = strtol(argv[1], &ptr,16);
	reg_addr 						= strtol(argv[2], &ptr,16);
	input_data 					= strtol(argv[3], &ptr,16);
	
	printf("\r\n Input : slave 0x%x, reg_addr 0x%x, data 0x%x", slave_dev_addr, reg_addr, input_data);
	
	if (gc_twi_write((uint8_t)slave_dev_addr, (uint8_t)reg_addr, input_data) != NRF_SUCCESS)
		return false;
	
	return true;
}
#endif /* GC_NRF52_TWI */

int cmd_gpio_read_all( int argc, char* argv[])
{
	uint32_t 	pin_num;
	
	if ( argc > 1)
	{
		printf("\n\rUsage : gpioread");
		return false;
	}
	
	printf("\r\n * Get GPIO Direction & Level <0: Low, 1 : High>");
	printf("\r\n * GPIO num  Dir(0 : in, 1 : out)  Level(0 : Low, 1 : High)");
	for (pin_num = 0; pin_num < 32; pin_num++)
	{
		uint32_t gpio_config = NRF_GPIO->PIN_CNF[pin_num];
		uint32_t dir = gpio_config & 0x01;
						
		printf("\r\n %02d : %d %d", pin_num, dir, nrf_gpio_pin_read(pin_num));
	}
	
		
	return true;
}

int cmd_gpio_write( int argc, char* argv[])
{
	uint32_t 	pin_num;
	
	if ( argc < 4 )
	{
		printf("\n\rUsage : gpiowrite <pin num> <in> <pull (up/down/no)>");
		return false;
	}
	
	pin_num = atoi(argv[1]);
	
	if ( !strcmp(argv[2],"in") )
	{
		if ( !strcmp(argv[3], "up") )
			nrf_gpio_cfg_input(pin_num, NRF_GPIO_PIN_PULLUP);
		else if ( !strcmp(argv[3], "down") )
			nrf_gpio_cfg_input(pin_num, NRF_GPIO_PIN_PULLDOWN);
		else if ( !strcmp(argv[3], "no") )
			nrf_gpio_cfg_input(pin_num, NRF_GPIO_PIN_NOPULL);
		else 
		{
			printf("\r\n invalid gpio pull config %s", argv[3]);
			return false;
		}
		
	}
	else if ( !strcmp(argv[2],"out") )
	{
		nrf_gpio_cfg_output(pin_num);
	}
	else 
	{
		printf("\r\n invalid gpio direction %s", argv[2]);
		return false;
	}
	
	return true;
}

int cmd_gpio_reset( int argc, char* argv[])
{
	uint32_t 	pin_num;
	
	if ( argc != 2)
	{
		printf("\n\rUsage : gpioreset <pin number>");
		return false;
	}
	
	pin_num = atoi(argv[1]);
	
	printf("\r\n * back gpio config to default");
	nrf_gpio_cfg_default(pin_num);

	return true;
}

/** 
 * @brief get RTC and convert it to clock time
 */
int cmd_clock( int argc, char* argv[])
{
	tTime time;
	get_time(&time, 1);
	
	printf("\r\n [%d/%d/%d %dh %dm %ds]", time.year,  time.month,  time.day,  time.hour,  time.minute, time.second);
	
	return true;
}

int cmd_acc( int argc, char* argv[])
{
	struct bma2x2_accel_data sample_xyz;
	
	memset(&sample_xyz, 0, sizeof(struct bma2x2_accel_data));
	
	bma2x2_read_accel_xyz(&sample_xyz);/* Read the accel XYZ data*/

	printf("\r\nacc= %d %d %d", sample_xyz.x, sample_xyz.y, sample_xyz.z );
	
	return true;
}

/*int cmd_pressure( int argc, char* argv[])
{
	measure_pressure(0);
	nrf_delay_ms(10);
	measure_pressure(1);
	nrf_delay_ms(10);
	measure_pressure(2);
	nrf_delay_ms(10);
	measure_pressure(3);
	nrf_delay_ms(10);
	measure_pressure(4);

	printf("\r\npressure= %4.2f", (float)g_pressure/(float)100.0 );
	
	return true;
}*/

int cmd_rtcwrite( int argc, char* argv[])
{
	unsigned char data;

	if(argc<3)
	{
		printf("\r\nrtcwrite [reg] [data]");
		return true;
	}

	data = atoi(argv[2]);

	rtc_i2c_write(atoi(argv[1]), &data, 1);


	return true;
}

/** 
 * @brief ready to display screen
 */
int cmd_oledpower( int argc, char* argv[])
{
	if(argc<2)
	{
		printf("\r\noledpower [0/1]");
		return true;
	}
#ifdef GC_COACH_FIT_PCB 
	if ( atoi(argv[1]) == 0 )
		OLED_Write_Command(0xAE); //Set Display Off
	else
		OLED_Write_Command(0xAF); //Set Display On
#elif defined GC_COACH_FIT_DK52
	OLED_Power(atoi(argv[1]));
#else
#endif 
	
	return true;
}

/** 
 * @brief display string on display. 'OLED_Power' must be executed before
 */

extern unsigned char tuch_count;
int cmd_dp( int argc, char* argv[])
{
	int num;
	num	=atoi(argv[1]);
	switch((num))
	{
	case 0:
		cmd_dp_clock(0,0);
		break;

	case 1:
		cmd_dp_step(0,0);
		break;

	case 2:
		cmd_dp_hr(0,0);
		break;

	case 3:
		cmd_dp_act(0,0);
		break;

	case 4:
		cmd_dp_sleep(0,0);
		break;

	case 5:
		cmd_dp_coach(0,0);
		break;

	case 6:
		cmd_dp_stress(0,0);
		break;

	case 7:
		cmd_dp_call(0,0);
		break;

	case 8:
		cmd_dp_unanswer(0,0);
		break;

	case 9:
		cmd_dp_sms(0,0);
		break;

	default:
		cmd_dp_sns(0,0);
		break;	
	}
	return true;
}
	int i;
	int high,low,line,row;
int cmd_dp_clock( int argc, char* argv[])
{
	OLED_clock();
		
	return true;
}

int cmd_dp_step( int argc, char* argv[])
{
	OLED_step();
	
	return true;
}

extern unsigned char g_hr;
int cmd_dp_hr( int argc, char* argv[])
{	
	OLED_hr();

	return true;
}

int cmd_dp_act( int argc, char* argv[])
{
	OLED_act();
	
	return true;
}

int cmd_dp_sleep( int argc, char* argv[])
{	
	OLED_sleep();

	return true;
}
  
int cmd_dp_coach( int argc, char* argv[])
{
	OLED_coach();

	return true;
}
  
  
int cmd_dp_stress( int argc, char* argv[])
{
	OLED_stress(1);
	
	return true;
}
  
  
int cmd_dp_call( int argc, char* argv[])
{
	OLED_call();
	
	return true;
}
  
  
int cmd_dp_unanswer( int argc, char* argv[])
{
	OLED_unanswer();
	
	return true;
}
  
  
int cmd_dp_sms( int argc, char* argv[])
{
	OLED_sms();
	
	return true;
}
  
  
int cmd_dp_sns( int argc, char* argv[])
{
	OLED_sns();
	
	return true;
}

extern volatile unsigned char noti_num;
extern unsigned char OLED_power_count;

extern volatile unsigned char op_mode;
extern unsigned char tuch_count;
int cmd_op( int argc, char* argv[])
{
	if(atoi(argv[1])!=0)
	{
		op_mode=atoi(argv[1]);
	}
	OLED_power_count=OLED_ON_count1;
	tuch_count=(op_mode==DAILY_MODE)? 1:5;
	
	return true;
}
  
int cmd_noti( int argc, char* argv[])
{
	noti_num=atoi(argv[1]);
	
		
	return true;
}

int cmd_accpower( int argc, char* argv[])
{
	if(argc != 3)
	{
		printf("\r\n 1 : low power 1");
		printf("\r\n 2 : suspend");
		printf("\r\n 3 : deep suspend");
		printf("\r\n 4 : low power 2");
		printf("\r\n 5 : standby");
		printf("\r\r\n Usage : accpower [0/1] [1/2/3/4/5]");
		return true;
	}
	
	if(atoi(argv[1])) {
		bma2x2_set_power_mode(BMA2x2_MODE_NORMAL);
	}
	else {
		
		switch(atoi(argv[2]))
		{
			case 1:
				bma2x2_set_power_mode(BMA2x2_MODE_LOWPOWER1); 
			break;
			
			case 2:
				bma2x2_set_power_mode(BMA2x2_MODE_SUSPEND); 
			break;
						
			case 3:
				bma2x2_set_power_mode(BMA2x2_MODE_DEEP_SUSPEND); 
			break;
									
			case 4:
				bma2x2_set_power_mode(BMA2x2_MODE_LOWPOWER2); 
			break;
												
			case 5:
				bma2x2_set_power_mode(BMA2x2_MODE_STANDBY); 
			break;
			
			default:
				break;
			
		}
	}
	
	return true;
}


int cmd_flashpower( int argc, char* argv[])
{
	unsigned char tx_data[4],rx_data[4];
	
	if(argc<2)
	{
		printf("\r\nflashpower [0/1]");
		return true;
	}
	
	if(atoi(argv[1]))
		tx_data[0]=W25X_ReleasePowerDown;
	else
		tx_data[0]=W25X_PowerDown;

	tx_data[1]=0;
	tx_data[2]=0;
	tx_data[3]=0;
	return spi_send_recv(tx_data, rx_data, 1);
}

int cmd_hrpower( int argc, char* argv[])
{
	if(argc<2)
	{
		printf("\r\nhrpower [0:Off, 1:On]");
		return true;
	}

	if(atoi(argv[1]))
	{
		pah8002_init();
	}
	else
	{
		pah8002_deinit();
	}
	
	return true;
}

int cmd_poweroff( int argc, char* argv[])
{
	if(argc<2)
	{
		printf("\r\npoweroff [0]");
		return true;
	}

	if(atoi(argv[1]))
	{
	}
	else
	{
		uint32_t gc_gpio=0;

		/* close gpio */
		for (gc_gpio=0; gc_gpio < 31; gc_gpio++)
		{
			nrf_delay_ms(10);
			nrf_gpio_cfg_default(gc_gpio);
		}
		
		nrf_delay_ms(100);
		#if 0		
		/* close TWI */
		nrf_drv_twi_uninit(&m_twi_master_1);
		nrf_drv_twi_uninit(&m_twi_master_2);
		
		/* close uart */
		nrf_drv_uart_uninit();
		#endif 
		
		gc_power_off_prepare();
		sd_power_system_off();
	}
	
	return true;
}

int cmd_txpower( int argc, char* argv[])
{
	unsigned int ret;
	
	if(argc<2)
	{
		printf("\r\ntxpower [accepted values are -40, -30, -20, -16, -12, -8, -4, 0, and 4 dBm]");
		return true;
	}


	ret = sd_ble_gap_tx_power_set(  atoi(argv[1]) );

	printf("\r\n ret = %d", ret );

	
	return true;
}

int cmd_oled_test_mode( int argc, char* argv[])
{
	oled_test_mode = atoi(argv[1]);
	
	return true;
}

int cmd_icon( int argc, char* argv[])
{
//	const unsigned char battery_0 [] = {
//	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
//	0x00, 0x00, 0xC0, 0xE0, 0x60, 0xE0, 0x80, 0x00, 0x00, 0x00, 0x00, 0xC0, 0x60, 0xC0, 0xC0, 0x00,
//	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
//	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
//	0x00, 0x80, 0xC0, 0xC0, 0xFF, 0x78, 0x00, 0x3F, 0x3F, 0x20, 0x20, 0x20, 0x3F, 0x07, 0x00, 0xFD,
//	0xFF, 0xC0, 0x80, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
//	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xC0, 0xF0,
//	0x3C, 0xCE, 0xC7, 0x83, 0x81, 0x80, 0x80, 0x80, 0x80, 0x80, 0xC0, 0x00, 0x00, 0x00, 0x00, 0x80,
//	0x80, 0x80, 0x80, 0x80, 0x81, 0x83, 0x87, 0x8E, 0x1C, 0x30, 0xE0, 0xC0, 0x00, 0x00, 0x00, 0x00,
//	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
//	0x3F, 0xFF, 0xC6, 0xBF, 0xF9, 0x00, 0x01, 0x01, 0x01, 0xDF, 0xFF, 0x00, 0x00, 0x00, 0x00, 0x00,
//	0x00, 0x87, 0xFF, 0x79, 0x01, 0x01, 0x01, 0x07, 0xFF, 0xFD, 0x80, 0xE0, 0xFF, 0x3F, 0x00, 0x00,
//	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
//	0x00, 0x00, 0x00, 0x00, 0x01, 0x03, 0x87, 0xEC, 0x7C, 0xB8, 0x9E, 0x1F, 0x11, 0x30, 0x30, 0x30,
//	0x30, 0x30, 0x30, 0x3F, 0xBF, 0x78, 0xF0, 0x90, 0x18, 0x1C, 0x0F, 0x07, 0x07, 0x01, 0x00, 0x00,
//	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
//	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xF8, 0xFF, 0x03, 0xFC, 0xFF, 0x00, 0x00, 0x00, 0x00,
//	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF, 0xFE, 0x07, 0xFF, 0xF8, 0x00, 0x00, 0x00, 0x00, 0x00,
//	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x44, 0x2C, 0x20, 0x30,
//	0x78, 0x30, 0x44, 0x2C, 0x20, 0x30, 0x78, 0x34, 
//	};

//		int i,j;
//		int width=50;
//		int hight=48;
//		OLED_Clear();
//		line=0;
//		row=0;
//		high=(row+32)/16; 
//		low=(row+32)%16;



//		for(j=0;j<hight/8;j++)
//		{
//			OLED_Write_Command(0xB0+line+j);
//			OLED_Write_Command(0x10+high);
//			OLED_Write_Command(low);
//			
//			for(i=j*width;i<(j+1)*width;i++)
//			{
//				OLED_Write_Data(battery_0[i]);
//			}
//		}
	unsigned char step_buf[254] ;
	unsigned char step_buf2[254] ;
	int time= 60327;
	step_buf[0]=time&0x000000ff;
	step_buf[1]=(time&0x0000ff00)>>8;
	step_buf[2]=(time&0x00ff0000)>>16;
	step_buf[3]=(time&0xff000000)>>24;
	sprintf((char*)&step_buf[4],"angelofwhite");
	if(gc_flash_page_write_data(2,step_buf))
	{
		printf("\r\n step_buf \n write OK");
	}
	printf("\r\n step_buf \n %s",step_buf);
	if(gc_flash_page_read_data(2,time, step_buf2))
	{
		printf("\r\n step_buf \n resd OK");
	}
	printf("\r\n step_buf \n %s",&step_buf2[4]);
	
	return true;
		
}


  

int cmd_bmd101( int argc, char* argv[])
{

#if defined(__USE_SPI_TERMINAL__)
/*
	int i,j;
	char inchar;
	char buffer[256];
	int count=0;


	printf("\n\r");

	for(i=0;i<100000;i++)
	{
		
		if( uart_get_bmd101(&inchar))
		{
			buffer[count]=inchar;

			count++;
			if(count>=256)
			{
				
				count=0;
				for(j=0;j<256;j=j+16)
					printf(" \n\r%02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x"
						,buffer[j], buffer[j+1],buffer[j+2],buffer[j+3],buffer[j+4],buffer[j+5],buffer[j+6],buffer[j+7],buffer[j+8], buffer[j+9],buffer[j+10],buffer[j+11],buffer[j+12],buffer[j+13],buffer[j+14],buffer[j+15]);				

				printf("\n\r");
			}


		}

	}

*/
	bm101_process();

#else

	//printf("\n\rno bm101");

#endif


	return TRUE;
}



int cmd_motion( int argc, char* argv[])
{

	pah8002_task();
	return TRUE;
}




int cmd_walk( int argc, char* argv[])
{
  int32_t aa ;
  int32_t bb ;
	int64_t diff;
	
	aa = atoi(argv[1]);
	bb = atoi(argv[2]);
	
	printf("\r\n %d %d", aa,bb);
	printf("\r\n %ld %ld", (long)aa, (long)bb);
	
	diff = aa - bb;
	
	printf("\r\n %d %d %ld", aa, bb, (long)diff);
	
	return true;
}



int cmd_stack( int argc, char* argv[])
{

	
	return true;
}

int cmd_time( int argc, char* argv[])
{
	unsigned int c1;

	c1 = NRF_RTC0->COUNTER;
	printf("\n\r NRF_RTC0 : %d", c1);

	printf("\r\n tick = %d", get_sys_tick());
	return TRUE;
}

int cmd_charge( int argc, char* argv[])
{
	is_charging = atoi(argv[1]);
	if(is_charging)
	{
		printf("\r\n charge start[%d]",is_charging);
	}
	else
	{
		printf("\r\n charge end[%d]",is_charging);
	}
	return TRUE;
}

int cmd_battery( int argc, char* argv[])
{
	printf("\r\n saadc %d battery %d %%", gc_saadc, g_battery);
	return TRUE;
}
#else /* if 0 */
/** 
 * @brief ready to display screen
 */
int cmd_oledpower( int argc, char* argv[])
{
	if(argc<2)
	{
		printf("\r\noledpower [0/1]");
		return true;
	}
#ifdef GC_COACH_FIT_PCB 
	if ( atoi(argv[1]) == 0 )
		OLED_Write_Command(0xAE); //Set Display Off
	else
		OLED_Write_Command(0xAF); //Set Display On
#elif defined GC_COACH_FIT_DK52
	OLED_Power(atoi(argv[1]));
#else
#endif 
	
	return true;
}

int cmd_reset( int argc, char* argv[])
{
//	uint32_t err_code;
	printf("\r\ncommand reset");
	//NRF_POWER->GPREGRET = 0xB1;
	//err_code = bootloader_dfu_start();
  //APP_ERROR_CHECK(err_code);
	save_data_before_reset(); /*16.12.20 for save_before_reset hm_lee*/
	nrf_delay_ms(10); /* FIXME */
	save_info_before_reset_point(6,0,0,0,0);	/*16.12.14 for test_log hm_lee*/
	NVIC_SystemReset();		/* include nrf.h */
	
	return true;
}

int cmd_reset_boot( int argc, char* argv[])
{
	NRF_POWER->GPREGRET = 0xB1;
	//	uint32_t err_code;
	printf("\r\ncommand reset boot");
	//NRF_POWER->GPREGRET = 0xB1;
	//err_code = bootloader_dfu_start();
  //APP_ERROR_CHECK(err_code);
	save_data_before_reset(); /*16.12.20 for save_before_reset hm_lee*/
	nrf_delay_ms(10); /* FIXME */
	save_info_before_reset_point(5,0,0,0,0);	/*16.12.14 for test_log hm_lee*/
	NVIC_SystemReset();		/* include nrf.h */
}

int cmd_fread( int argc, char* argv[])
{
	unsigned char buf[255];
	char *pseudo;
	int address;

	memset(buf,0,sizeof(buf));

	if(argc<2)
	{
		printf("\r\nfread [adress(hex)]");
		return true;
	}

	address = strtoul(argv[1],&pseudo,16);
	
	SPI_FLASH_BufferRead(buf, address );

	DumpMemory(buf+4,256);
	//printf("\r\n %x %x %x %x %x", buf[0],bu989f[1],buf[2],buf[3],buf[4]);

	return true;
}

int cmd_fwrite( int argc, char* argv[])
{
	unsigned char buf[260];
	char *pseudo;
	int address;
	
	if(argc<3)
	{
		printf("\r\nfwrite [page adress(hex)] [string]");
		return true;
	}

	address = strtoul(argv[1],&pseudo,16);

	memset(buf, 0xff, sizeof(buf));
	
	memcpy(buf+4, argv[2], strlen(argv[2]) );
	
	SPI_FLASH_PageWrite(buf, address );
	
	return true;
}

int cmd_ferase( int argc, char* argv[])
{
	char *pseudo;
	int address;
	int i;
//	int _sector=0; /* v1.4.11.1 */
	
	if(argc<2)
	{
		printf("\r\nferase [sector adress(4k)]");
		return true;
	}

	address = strtoul(argv[1],&pseudo,16);
	
	SPI_FLASH_SectorErase(address );

	if (!strcmp(argv[1], "all"))
	{
		#if 0 /* hkim v1.5.16.0 */
		stop_feature_process = true;	
		#endif 
		
		/* 모든 섹터 삭제 */
		for (i = 0; i < 144; i++)
		{
			address = i * 0x100;
			printf("\r\nerase sector : %x", address);
			SPI_FLASH_SectorErase(address );
			nrf_delay_ms(200);
		}
		//init_db_write_page();
		
		#if 0 /* hkim v1.5.16.0 */
		stop_feature_process = false;
		#endif 
	}
	else
	{
		address = strtoul(argv[1],&pseudo,16);	
		SPI_FLASH_SectorErase(address );
	}

	
	return true;
}

extern uint8_t op_mode;
int cmd_opmode( int argc, char* argv[])
{
	uint8_t mode, ctrl=0;
	
	if(argc<3)
	{
		printf("\r\nopmode <mode> <enable/disable>");
		return true;
	}
	
	mode = (uint8_t)atoi(argv[1]);
	ctrl = (uint8_t)atoi(argv[2]);

	op_mode = mode;
	gc_protocol_ctrl_pah8002(mode,ctrl);
	
	return true;
}

extern volatile unsigned char noti_num;
int cmd_noti( int argc, char* argv[])
{
	noti_num=atoi(argv[1]);
	
		
	return true;
}

extern volatile unsigned char OLED_switch;
int cmd_hwtest( int argc, char* argv[]) /* KKK */
{
	int i,j,high,low; /*16.12.13 hm.lee	생산지원*/
	uint8_t reg, r_data=0x00, w_data=0x00, test_data = 0xff;
	uint8_t r_buf[260], w_buf[260];
	
	OLED_Power(1); /*16.12.13 hm.lee	생산지원*/

	//hr_dp =1; /*16.12.13 hm.lee	생산지원*/
	//hr_test_count =0;
	//op_mode = ACT_MODE;/*16.12.13 hm.lee	생산지원*/
	#ifndef HR_TEST_ONLY
	for(j=0;j<6;j++) /*16.12.13 hm.lee	생산지원*/
	{
		for(i=0;i<64;i++)
		{	
			high=(i+32)/16;
			low=(i+32)%16;
			OLED_Write_Command(0xB0+j);
			OLED_Write_Command(0x10+high);
			OLED_Write_Command(low);
			OLED_Write_Data(0XFF);
		}
	}
	printf("\r\n *********************************************");
	printf("\r\n iBody24 Fitness device test");
	printf("\r\n *********************************************");
		printf("\r\n 0x%x 0x%x 0x%x", w_data, r_data ,test_data);
	printf("\r\n %25s ---------------  ", "RTC Test");
	reg 	= 0x03;
	w_data	= 0xFF;
	rtc_i2c_write(reg, &w_data, 1);
	nrf_delay_ms(100);
	rtc_i2c_read(reg, 1, &r_data);
	//printf("\r\n 0x%x 0x%x", w_data, r_data);
	
	if ( w_data == r_data )
		printf("OK");
	else
		printf("FAIL");
		printf("\r\n 0x%x 0x%x 0x%x", w_data, r_data ,test_data);
	nrf_delay_ms(100);
	
	printf("\r\n %25s ---------------  ", "Accel Sensor Test");
	reg 	= 0x30;
	w_data	= 0x3F;
	bma2x2_write_reg(reg, &w_data, 1);
	nrf_delay_ms(100);
	bma2x2_read_reg(reg, &r_data, 1);
	nrf_delay_ms(100);
	bma2x2_read_reg(0x00, &test_data, 1);
	//printf("\r\n 0x%x 0x%x", w_data, r_data);
	
	if ( w_data == r_data )
		printf("OK");
	else
		printf("FAIL");
		
	printf("\r\n 0x%x 0x%x 0x%x", w_data, r_data ,test_data);
	w_data = 0xFF;
	r_data = 0xFF;
	test_data = 0xFF;
	nrf_delay_ms(100);
	/*printf("\r\n %25s ---------------  ", "Pressure SensorTest");
	measure_pressure(0);
	nrf_delay_ms(10);
	measure_pressure(1);
	nrf_delay_ms(10);
	measure_pressure(2);
	nrf_delay_ms(10);
	measure_pressure(3);
	nrf_delay_ms(10);
	measure_pressure(4);

	if ( ((float)g_pressure/(float)100.0) > 1000)
		printf("OK");
	else
		printf("FAIL");
*/	
	nrf_delay_ms(100);
	#endif /*HR_TEST_ONLY*/
	printf("\r\n %25s ---------------  ", "HR Sensor Test");
	int retry = 0 ;
	int success = 0 ;
	uint8_t data = 0 ;
	//pah8002_read_reg(0, &data);
	for (int i = 0; i<10;i++){
		pah8002_read_reg(0, &data);
	}
  if(data == 0x02){
		printf("OK");
	}else{
		printf("FAIL");
	}
	/*do
	{
		//pah8002_write_reg(0x7f, 0x00);
		pah8002_read_reg(0, &data);
		if(data == 0x02) success++;
		else success = 0 ;
					
		if(success >=2){
			printf("OK");
		}
		break;
		retry ++;
				
	}while(retry < 20);	
	if(retry > 19){
		printf("FAIL");
	}*/
	
	nrf_delay_ms(100);
	#ifndef HR_TEST_ONLY
	printf("\r\n %25s ---------------  ", "Flash R/W Test");
	memset(w_buf, 0xff, sizeof(w_buf));
	w_buf[4] = 0x11;
	w_buf[5] = 0x22;
	//printf("\r\n W : 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x", w_buf[0], w_buf[1], w_buf[2], w_buf[3], w_buf[4], w_buf[5]);
	SPI_FLASH_SectorErase(0x00 );
	nrf_delay_ms(200);
	SPI_FLASH_PageWrite(w_buf, 0x00 );
	nrf_delay_ms(200);
	SPI_FLASH_BufferRead(r_buf, 0x00 );

	//printf("\r\n R : 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x", r_buf[0], r_buf[1], r_buf[2], r_buf[3], r_buf[4], r_buf[5]);
	if ( (w_buf[4] == r_buf[4]) && (w_buf[5] == r_buf[5]) )
		printf("OK");
	else
		printf("FAIL");
	
	printf("\r\n %25s ---------------  ", "BLE stop advertising");
	if ( sd_ble_gap_adv_stop() == NRF_SUCCESS)
		printf("OK");
	else
		printf("FAIL");	
	
	printf("\r\n %25s ---------------  ", "BLE start advertising");
	if (ble_advertising_start(BLE_ADV_MODE_FAST) == NRF_SUCCESS)
		printf("OK");
	else
		printf("FAIL");	
	
	nrf_gpio_pin_clear(GC_GPIO_MOTOR_ENABLE);
	nrf_delay_ms(1000);
	nrf_gpio_pin_set(GC_GPIO_MOTOR_ENABLE);
	#endif /*HR_TEST_ONLY*/
	printf("\r\n %25s ---------------  ", "HR Sensor ReadPPG");
	gc_protocol_ctrl_pah8002(ACT_MODE, 1); 
	//pah8002_init();
	//pah8002_enter_touch_mode();
	testPPG = true;
	
	/* display OLED during 1 sec */
}

#ifdef _FIT_ETRI_TEST_
int cmd_etrihw( int argc, char* argv[]) 
{
	uint8_t reg, r_data, w_data;
	uint8_t r_buf[260], w_buf[260];
	
	printf("\r\n *********************************************");
	printf("\r\nETRI H/W device test");
	printf("\r\n *********************************************");
	printf("\r\n %25s ---------------  ", "RTC Test");
	reg 	= 0x03;
	w_data	= 0xFF;
	rtc_i2c_write(reg, &w_data, 1);
	nrf_delay_ms(100);
	rtc_i2c_read(reg, 1, &r_data);
	//printf("\r\n 0x%x 0x%x", w_data, r_data);
	
	if ( w_data == r_data )
		printf("OK");
	else
		printf("FAIL");
	
	nrf_delay_ms(100);
	
	printf("\r\n %25s ---------------  ", "Accel Sensor Test");
	reg 	= 0x30;
	w_data	= 0x3F;
	bma2x2_write_reg(reg, &w_data, 1);
	nrf_delay_ms(100);
	bma2x2_read_reg(reg, &r_data, 1);
	//printf("\r\n 0x%x 0x%x", w_data, r_data);
	
	if ( w_data == r_data )
		printf("OK");
	else
		printf("FAIL");
	
	nrf_delay_ms(100);
		printf("\r\n %25s ---------------  ", "HR Sensor Test");
	reg 	= 0x0F;
	w_data	= 0x3F;
	pah8002_write_reg(0x7f, 0x00); /* switch bank */
	nrf_delay_ms(10);
	pah8002_write_reg(reg, w_data);
	nrf_delay_ms(10);
	pah8002_read_reg(reg, &r_data);
	//printf("\r\n 0x%x 0x%x", w_data, r_data);
	if ( w_data == r_data )
		printf("OK");
	else
		printf("FAIL");
	
	nrf_delay_ms(100);
	printf("\r\n %25s ---------------  ", "Flash R/W Test");
	memset(w_buf, 0xff, sizeof(w_buf));
	w_buf[4] = 0x11;
	w_buf[5] = 0x22;
	//printf("\r\n W : 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x", w_buf[0], w_buf[1], w_buf[2], w_buf[3], w_buf[4], w_buf[5]);
	SPI_FLASH_SectorErase(0x00 );
	nrf_delay_ms(200);
	SPI_FLASH_PageWrite(w_buf, 0x00 );
	nrf_delay_ms(200);
	SPI_FLASH_BufferRead(r_buf, 0x00 );

	//printf("\r\n R : 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x", r_buf[0], r_buf[1], r_buf[2], r_buf[3], r_buf[4], r_buf[5]);
	if ( (w_buf[4] == r_buf[4]) && (w_buf[5] == r_buf[5]) )
		printf("OK");
	else
		printf("FAIL");
	
	printf("\r\n %25s ---------------  ", "BLE test(stop)");
	if ( sd_ble_gap_adv_stop() == NRF_SUCCESS)
		printf("OK");
	else
		printf("FAIL");	
	
	printf("\r\n %25s ---------------  ", "BLE test(start)");
	if (ble_advertising_start(BLE_ADV_MODE_FAST) == NRF_SUCCESS)
		printf("OK");
	else
		printf("FAIL");	
} 

int cmd_etri_flash_read( int argc, char* argv[]) /* read page 0 */
{
	unsigned char buf[255];
	uint32_t i=0;
	char *pseudo;
	int address;

	memset(buf,0,sizeof(buf));

	if(argc > 1)
	{
		printf("\r\nflashread [read page 0]");
		return true;
	}

	SPI_FLASH_BufferRead(buf, 0);
	DumpMemory(buf+4,256);

	//DumpMemory(buf+4,256);
	//printf("\r\n %x %x %x %x %x", buf[0],buf[1],buf[2],buf[3],buf[4]);

	return true;
}

int cmd_etri_flash_write( int argc, char* argv[]) /* write abcdef to page 0 */
{
	unsigned char buf[260];
	uint8_t input_str[260];
	char *pseudo;
	int address;
	
	if(argc > 1)
	{
		printf("\r\nflashwrite [write 'abcdef' to sector 0]");
		return true;
	}

	address = 0;

	memset(buf, 0xff, sizeof(buf));
	memset(input_str, 0, sizeof(input_str));
	
	strcpy(input_str, "abcdef");
	
	memcpy(buf+4, input_str, strlen(input_str) );
	
	SPI_FLASH_PageWrite(buf, address );
	printf("\r\nWrite 'abcdef' to flash. done");
	
	return true;
}

int cmd_etri_flash_erase( int argc, char* argv[]) /* erase sector 0 */
{
	char *pseudo;
	int address;
//	int _sector=0; /* v1.4.11.1 */
	
	if(argc > 1)
	{
		printf("\r\nflasherase [erase sector 0]");
		return true;
	}

	address = 0;
	
	SPI_FLASH_SectorErase(address );
	printf("\r\nErase flash. done");

	return true;
} 

int cmd_etri_flash_sleep( int argc, char* argv[])
{
	int32_t err_code;
	unsigned char tx_data[4],rx_data[4];
	
	if(argc > 1)
	{
		printf("\r\nflashsleep");
		return true;
	}
	
	tx_data[0]=W25X_PowerDown;
	tx_data[1]=0;
	tx_data[2]=0;
	tx_data[3]=0;
	
	err_code = spi_send_recv(tx_data, rx_data, 1);
	
	if (err_code == NRF_SUCCESS)
		printf("\r\nFlash device sleep mode. done");
	else
		printf("\r\nFlash device sleep mode. fail");
}

int cmd_etri_flash_wakeup( int argc, char* argv[])
{
	unsigned char tx_data[4],rx_data[4];
	int32_t err_code;
	
	if(argc > 1)
	{
		printf("\r\nflashwakeup");
		return true;
	}
	
	tx_data[0]=W25X_ReleasePowerDown;
	tx_data[1]=0;
	tx_data[2]=0;
	tx_data[3]=0;
	
	err_code = spi_send_recv(tx_data, rx_data, 1);
	if (err_code == NRF_SUCCESS)
		printf("\r\nWake-up flash device. done");
	else
		printf("\r\nWake-up flash device. fail");
} 

int cmd_etri_acc_sleep( int argc, char* argv[])
{
	if(argc > 1)
	{
		printf("\r\r\naccsleep [deep suspend mode]");
		return true;
	}

	bma2x2_set_power_mode(BMA2x2_MODE_DEEP_SUSPEND); 
	
	printf("\r\nAccel sensor sleep mode. done");
	return true;
} 

int cmd_etri_acc_wakeup( int argc, char* argv[])
{
	if(argc > 1)
	{
		printf("\r\r\naccwakeup [normal mode]");
		return true;
	}
	
	bma2x2_set_power_mode(BMA2x2_MODE_NORMAL);
	
	printf("\r\nWake-up accel sensor. done");
	return true;
} 

int cmd_etri_hr_sleep( int argc, char* argv[])
{
	if(argc > 1)
	{
		printf("\r\nhrsleep");
		return true;
	}
	
	//gc_disable_pah8002();
	pah8002_deinit();
	printf("\r\nHeart rate sensor sleep mode. done");
	
	return true;
}

int cmd_etri_hr_touch( int argc, char* argv[])
{
	if(argc > 1)
	{
		printf("\r\nhrtouch");
		return true;
	}
	
	if (pahr_mode == PAHR_SUSPEND_MODE)
		//gc_enable_pah8002();
	pah8002_init();
	
	//pah8002_enter_touch_mode();
	
	printf("\r\nHeart rate sensor touch mode. done");
	return true;
} 

int cmd_etri_hr_normal( int argc, char* argv[])
{
	if(argc > 1)
	{
		printf("\r\nhrnormal");
		return true;
	}
	
	if (pahr_mode == PAHR_SUSPEND_MODE)
		//gc_enable_pah8002();
	pah8002_init();
		
	//pah8002_enter_normal_mode();
	printf("\r\nHeart rate sensor normal operation mode. done");

	return true;
}

int cmd_etri_mcu_poweroff( int argc, char* argv[])
{
	uint32_t gc_gpio=0;
	
	if(argc > 1)
	{
		printf("\r\nmcuoff");
		return true;
	}

	/* close gpio */
	for (gc_gpio=0; gc_gpio < 31; gc_gpio++)
	{
		nrf_delay_ms(10);
		nrf_gpio_cfg_default(gc_gpio);
	}
	
	nrf_delay_ms(100);
	
	nrf_gpio_cfg_sense_input(GC_GPIO_PIN_ACC_INTR_1, NRF_GPIO_PIN_NOPULL, NRF_GPIO_PIN_SENSE_HIGH);
	sd_power_system_off();
	
	printf("\r\nPower down MCU. done");
	
	return true;
} 

int cmd_etri_remain_battery(int argc, char* argv[])
{
	printf("\r\n Battery : %d(%%)", g_battery);
	
	return true;
}
#endif /* _FIT_ETRI_TEST_ */
#endif //hkim-1007
int cmd_help(int argc,char* argv[])
{
	int i;
	
	if(argc!=1){
		printf("\n\rTest argc and argv\n\r");
		for(i=0;i<argc;i++) printf("%s ",argv[i]);
	}
#ifndef _FIT_ETRI_TEST_ 	
	printf("\n\n\r   - HELP MODE -");
	printf("\n\rDump ussage : Dump [start address] [end address]");
	printf("\n\rEdit ussage : Edit [start address]");
#endif 
	
	printf("\n\n\r---------- Command List ----------");
	i=0;
	while( cmddefine[i].function ) printf("\n\r%s ", cmddefine[i++].name);
	printf("\n\r");
	
	return true;
}


int cmd_feraseall(int argc,char* argv[])
{
	printf("\n\rFlash All Erase\n\r");
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
	return true;
}


int cmd_version( int argc, char* argv[])
{
	printf("\n\r\t%s",PRODUCT_VERSION);
	HR_info.HR_avg = 80;
	HR_info.HR_min =  70;
	HR_info.HR_max = 90;
	return true;
}

int cmd_poweroff( int argc, char* argv[])
{
	gc_power_off_prepare();
	nrf_gpio_pin_clear(GC_GPIO_PIN_HR_POWER_EN);
	nrf_delay_ms(100);
	nrf_gpio_pin_clear(GC_GPIO_DEBUG_ON);
	nrf_delay_ms(100);
	sd_power_system_off();
	NRF_POWER->TASKS_LOWPWR = 1;
	NRF_POWER->SYSTEMOFF = POWER_SYSTEMOFF_SYSTEMOFF_Enter;
	return true;
}

int cmd_getslpinfo( int argc, char* argv[])
{
	_GC_POWER_OFF_INFO_	*aa;
	uint8_t __resp[GC_FLASH_PAGE_SIZE];
	
	memset(__resp,0,sizeof(GC_FLASH_PAGE_SIZE));
	
	if(gc_flash_page_read_data(GC_FLASH_TBL_TYPE_POWER_OFF,1,__resp))
	{
		aa = (_GC_POWER_OFF_INFO_ *)__resp;
		printf("\r\n %d-%d-%d %d:%d;%d", 
							aa->sleep_start_year, aa->sleep_start_month, 
							aa->sleep_start_day, aa->sleep_start_hour, aa->sleep_start_minute, aa->sleep_start_second);
		
		printf("\r\n %d setps \r\n %d cal \r\n %d cal \r\n %d cal \r\n %d cal \r\n ", 
							aa->step, aa->step_cal, aa->activty_cal, aa->sleep_cal, aa->coach_cal);
							
	}
   //규창 뭘 리턴하려고 한걸까?...
}
