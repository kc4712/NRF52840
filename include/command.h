

#define		SCC_RX_EXECUTE_BUFFER_SIZE	64


typedef struct{
	char* name;
	int (*function)(int argc, char* argv[]);
} COMMAND_DEFINE;

typedef struct{
	int		pointer;
	char	buffer[SCC_RX_EXECUTE_BUFFER_SIZE];
} SCC_RX_EXECUTE;





int cmd_dump(int,char**);
int cmd_edit(int,char**);
int cmd_nus_ble_send(int argc,char* argv[]);
int cmd_reset(int,char**);
int cmd_help(int,char**);
int cmd_spi(int,char**);
int cmd_twi_read(int,char**);
int cmd_twi_write(int,char**);
int cmd_gpio_read_all( int argc, char* argv[]);
int cmd_gpio_write( int argc, char* argv[]);
int cmd_gpio_reset( int argc, char* argv[]);
int cmd_clock(int,char**);
int cmd_acc( int argc, char* argv[]);
int cmd_pressure( int argc, char* argv[]);
int cmd_oledpower(int,char**);


int cmd_dp(int,char**);
int cmd_dp_clock(int,char**);
int cmd_dp_step(int,char**);
int cmd_dp_hr(int,char**);
int cmd_dp_act(int,char**);
int cmd_dp_sleep(int,char**);
int cmd_dp_coach(int,char**);
int cmd_dp_stress(int,char**);
int cmd_dp_call(int,char**);
int cmd_dp_unanswer(int,char**);
int cmd_dp_sms(int,char**);
int cmd_dp_sns(int,char**);
int cmd_icon(int,char**);
int cmd_op(int,char**);
int cmd_noti(int,char**);
int cmd_hwtest( int argc, char* argv[]);
int cmd_accpower( int argc, char* argv[]);
int cmd_flashpower( int argc, char* argv[]);
int cmd_hrpower( int argc, char* argv[]);
int cmd_poweroff( int argc, char* argv[]);
int cmd_txpower( int argc, char* argv[]);
int cmd_getslpinfo( int argc, char* argv[]);

#ifdef GC_NRF52_PAHR
//int cmd_spi_pah8002_read(int,char**);
int cmd_pah8002_burst_read( int argc, char* argv[]);
int cmd_pah8002_loop_read( int argc, char* argv[]);
int cmd_pah8002_write(int,char**);
int cmd_dump_pah8002(int,char**);
int cmd_pah8002_checksum( int argc, char* argv[]);
#else
int cmd_afe4400_read(int,char**);
int cmd_afe4400_write(int,char**);
#endif 
int cmd_sensor(int,char**);
int cmd_time(int,char**);
int cmd_bmd101(int,char**);
int cmd_motion(int,char**);
int cmd_walk(int,char**);
int cmd_stack(int,char**);
int cmd_time(int,char**);
int cmd_oled_test_mode(int,char**);

int cmd_fread(int,char**);
int cmd_fwrite(int,char**);
int cmd_ferase(int,char**);
int cmd_flash_table_write(  int argc, char* argv[]);
int cmd_spi_tx(int argc, char* argv[]); //GC_NRF52_PAHR
void __cmd_nus_ble_send(void);
void DumpMemory(void* pMemory,int count);


int cmd_parse(char *cmd,char **argv);
int strncmpi(char* s1, char* s2, int len);

//int ks_get_command(SCC_RX_EXECUTE *scc_rx_execute);
int ks_get_command(SCC_RX_EXECUTE *scc_rx_execute, unsigned char c);

void command_execute(char* buffer);

//void debug_shell(void);
void debug_shell(unsigned char c);

void initialize_cmd_variable(void);

int cmd_charge(int,char**);
int cmd_battery(int,char**);
int cmd_opmode(int,char**);
int cmd_version( int argc, char* argv[]);

#ifdef _FIT_ETRI_TEST_
int cmd_etrihw(int,char**);
int cmd_etri_flash_read(int,char**);
int cmd_etri_flash_write(int,char**);
int cmd_etri_flash_erase(int,char**);
int cmd_etri_flash_sleep(int,char**);
int cmd_etri_flash_wakeup(int,char**);
int cmd_etri_acc_sleep( int argc, char**);
int cmd_etri_acc_wakeup( int argc, char**);
int cmd_etri_hr_sleep( int argc, char**);
int cmd_etri_hr_touch( int argc, char**);
int cmd_etri_hr_normal( int argc, char**);
int cmd_etri_mcu_poweroff( int argc, char**);
int cmd_etri_remain_battery(int argc, char* argv[]);
#endif /* _FIT_ETRI_TEST_ */

int cmd_pah_read( int argc, char* argv[]);
int cmd_reset_boot( int argc, char* argv[]);
int cmd_feraseall(int argc, char* argv[]);
