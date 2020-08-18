

#define OLED_ON_count_charging 1200 - 1
#define OLED_ON_count_time 1200 - (20 * 5)  //1200 - (20 * 디스플레이 시간[초])
#define OLED_ON_count_step 1200 - (20 * 4)  //1200 - (20 * 디스플레이 시간[초])
#define OLED_ON_count_cal 1200 - (20 * 4) //1200 - (20 * 디스플레이 시간[초])
#define OLED_ON_count_hr 1200 - (20 * 7)  //1200 - (20 * 디스플레이 시간[초])
#define OLED_ON_count_sleep 1200 - (20 * 4)  //1200 - (20 * 디스플레이 시간[초])
#define OLED_ON_count_act_info 1200 - (20 * 7)  //1200 - (20 * 디스플레이 시간[초])
//홈쇼핑으로 인한 기압표시화면 제거
//#define OLED_ON_count_pressure_info 1200 - (20 * 4)  //1200 - (20 * 디스플레이 시간[초])
#define OLED_ON_count_noti 1200 - (20 * 7)  //1200 - (20 * 디스플레이 시간[초])
#define OLED_OFF_count 1200

#define display_page_clock 0
#define display_page_step 1
#define display_page_calorie 2
#define display_page_mode 3
#define display_page_act_info 4

/*홈쇼핑으로 인한 기압표시화면 제거
#define display_page_pressure_info 3
#define display_page_mode 4
#define display_page_act_info 5
*/

#define display_page_hr 2
#define display_page_hr_pct 3

void OLED_display(int count );

void OLED_clock(void);
void OLED_step(void);
void OLED_hr(void);
void OLED_act(void);
void OLED_act_info(void);
void OLED_sleep(void);
void OLED_coach(void);
void OLED_stress(unsigned int stress_count);
void OLED_call(void);
void OLED_unanswer(int noti_count);
void OLED_sms(int noti_count);
void OLED_sns(int noti_count);
void OLED_hr_pct(void);
void OLED_charge(int battery);
void OLED_charge2(int battery);
void OLED_calorie(void);
void OLED_info(void);

 //홈쇼핑으로 인한 기압표시화면 제거
//void OLED_pressure_info(void);
