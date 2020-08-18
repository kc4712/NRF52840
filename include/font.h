

//extern const unsigned char battery_100[];
//extern const unsigned char battery_80[];
//extern const unsigned char battery_60[];
//extern const unsigned char battery_40[];
//extern const unsigned char battery_20[];
//extern const unsigned char battery_0[];
//extern const unsigned char BT[];

//extern const unsigned char step_icon[];
//extern const unsigned char HR_icon[];
//extern const unsigned char act_icon[];
//extern const unsigned char sleep_icon[];
//extern const unsigned char Coach_icon[];
//extern const unsigned char Stress_icon[];
//extern const unsigned char call_icon[];
//extern const unsigned char Unanswer_icon[];
//extern const unsigned char SMS_icon[];
//extern const unsigned char SNS_icon[];
//extern const unsigned char Cal_icon[];

//extern const unsigned char snumNone [];
//extern const unsigned char snumSlash [];
//extern const unsigned char snumDot [];
//extern const unsigned char snumColon [];
//extern const unsigned char snum0 [];
//extern const unsigned char snum1 [];
//extern const unsigned char snum2 [];
//extern const unsigned char snum3 [];
//extern const unsigned char snum4 [];
//extern const unsigned char snum5 [];
//extern const unsigned char snum6 [];
//extern const unsigned char snum7 [];
//extern const unsigned char snum8 [];
//extern const unsigned char snum9 [];

//extern const unsigned char lnumNone [];
//extern const unsigned char lnumSlash [];
//extern const unsigned char lnumDot [];
//extern const unsigned char lnumColon [];
//extern const unsigned char lnum0 [];
//extern const unsigned char lnum1 [];
//extern const unsigned char lnum2 [];
//extern const unsigned char lnum3 [];
//extern const unsigned char lnum4 [];
//extern const unsigned char lnum5 [];
//extern const unsigned char lnum6 [];
//extern const unsigned char lnum7 [];
//extern const unsigned char lnum8 [];
//extern const unsigned char lnum9 [];

//extern const unsigned char mon[];
//extern const unsigned char tue[];
//extern const unsigned char wed[];
//extern const unsigned char thu[];
//extern const unsigned char fri[];
//extern const unsigned char sat[];
//extern const unsigned char sun[];

void dp_snum(int line,int row,int num);
void dp_mnum(int line,int row,int num);
void dp_lnum(int line,int row,int num);
void dp_BT(int line,int row,int num);
void dp_battery(int line,int row,int g_battery);
void dp_unit(int line,int row,int num);
void OLED_blank(int line,int row,int length);
void dp_week(int line,int row,int num);
void dp_charge(int line,int row,int g_battery);
void dp_charge2(int line,int row,int g_battery);
void dp_icon_mini(int line,int row,int num);
void dp_icon_small(int line,int row,int num);
void dp_icon_large(int line,int row,int num);
void dp_schar(int line,int row,char num);
