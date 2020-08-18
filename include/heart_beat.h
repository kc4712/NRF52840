

#define	HB_LED_OFF_ADC_THR	50000



int PreProcess(int adc_in);
void store_data(int diff_data);
int cal_first_area(void);
int cal_second_area(void);



void HeartBeat_Process(void);
void HeartBeat_Process_InTask(void);
unsigned char get_sort_hb(void);

void post_calculation(unsigned char current_hb);
void bubble_sort(unsigned char list[], int n);
void init_post_cal(void);

void set_LED_OFF_ADC_THR(int value);
	

extern char g_hb_cal_signal;
extern char g_hb_touched;
extern unsigned char g_sort_hb;

