
#ifndef __PRINT_H__
#define __PRINT_H__


#define GRAY_COLOR             "\033[1;30m"
#define RED_COLOR              "\033[1;31m"
#define GREEN_COLOR            "\033[1;32m"
#define MAGENTA_COLOR          "\033[1;35m"
#define YELLOW_COLOR           "\033[1;33m"
#define BLUE_COLOR             "\033[1;34m"
#define DBLUE_COLOR            "\033[0;34m"
#define WHITE_COLOR            "\033[1;37m"

#define NORMAL_COLOR           "\033[0m"


typedef enum
{
	PRINT_GRAY=0,
	PRINT_RED,
	PRINT_GREEN,
	PRINT_MAGENTA,
	PRINT_YELLOW,
	PRINT_BLUE,
	PRINT_WHITE,
}PRINT_COLOR;



int ks_printf(const char *form, ... );
int print_co(int color, const char *form, ... );

#endif

