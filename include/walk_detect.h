


void Save_Accel_Data(void);



#define RUN_HIGH_PEAK_THR	100000

#define SHORT_PERIOD_THR	33

#define MININUM_HIGH  27000
#define MININUM_LOW  -18000

#define MININUM_LOW_HIGH_INTERVAL 3

#define MAX_VALUE	10000000
#define MIN_VALUE	-10000000

#define THR_VALID_AREA 1000

#define ZERO_CROSS_THR	 2000
#define ZERO_CROSS_COUNT_THR	 18


typedef enum
{
	STATE_PEAK_DETECT_INIT=0,
	STATE_PEAK_DETECT_HIGH,
	STATE_PEAK_DETECT_LOW,
} PEAK_DETECT_STATE;

typedef enum
{
	DOMINANT_AXIS_NONE,
	DOMINANT_AXIS_X,
	DOMINANT_AXIS_Y,
	DOMINANT_AXIS_Z,
} DOMINANT_AXIS;


typedef struct
{
	signed char x;
	signed char y;
	signed char z;
} ACCEL_AD_DATA;




typedef struct
{
	signed char real_x[8];
	signed char real_y[8];
	signed char real_z[8];

	signed int  mean_square[50];
	signed int  hpf_mean_square[100];

	int accel_count;

	int	prev_high_time[4];
	int	prev_low_time[4];
	int	prev_high_value;
	int	prev_low_value;

	int	same_peroid_count;


	unsigned char real_pointer;
	unsigned char mean_square_count;
	unsigned char hpf_mean_square_count;
	unsigned char start_time;
	
} WALK_DETECT_DATA;


int Detect_Watch_Motion(ACCEL_AD_DATA* ad_data);
void store_LPF_data(signed char x, signed char y, signed char z);
int find_peak(int *lpf_buffer, int area, int start_time);
void Cal_Walk_Data(ACCEL_AD_DATA* ad_data);
void Init_Walk_detect(void);
int find_zerocorss(int *lpf_buffer);


void Cal_PET_Data(ACCEL_AD_DATA* ad_data);
void Calorie_Process(void);
int  store_PET_data(signed char x, signed char y, signed char z);


void Walk_Detect_Process(void);


extern int m_missing_count;

extern int m_walking_count;

extern WALK_DETECT_DATA m_WalkData;

extern int m_walking_sleep;

