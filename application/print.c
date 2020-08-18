

#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <stdlib.h>

#include "define.h"
#include "scc.h"
#include "print.h"

#if defined(__USE_SPI_TERMINAL__)
#include "afe4400.h"
#include <spi_master.h>
#endif


// perform "printf" function via serial port

int ks_printf(const char *form, ... )
{
	va_list  argptr;
	char	console_tx_buffer[128]; 
#if !defined(__USE_SPI_TERMINAL__)
	int	i;
#endif
  
	va_start(argptr,form);
	vsprintf(console_tx_buffer, form, argptr );
	va_end(argptr);

#if !defined(__USE_SPI_TERMINAL__)	
	for(i=0;console_tx_buffer[i];i++)
		uart_put(console_tx_buffer[i]);
#else
	wait_print_tx_interval();
	spi_master_tx_rx(get_AFE4400_SPI_Address(), strlen(console_tx_buffer), (uint8_t *)console_tx_buffer, (uint8_t *)console_tx_buffer);
#endif
	

	return TRUE;
}




void ks_puts(char* string )
{
#if !defined(__USE_SPI_TERMINAL__)
	int	i;
#endif

#if !defined(__USE_SPI_TERMINAL__)
	for(i=0;*(string+i);i++)
		uart_put(*(string+i));
#else
	wait_print_tx_interval();
	spi_master_tx_rx(get_AFE4400_SPI_Address(), strlen(string), (uint8_t *)string, (uint8_t *)string);
#endif
	
}




int print_co(int color, const char *form, ... )
{
	va_list  argptr;
	char	console_tx_buffer[128]; 
#if !defined(__USE_SPI_TERMINAL__)
	int	i;
#endif

	switch(color)
	{
	case PRINT_GRAY:
		ks_puts(GRAY_COLOR);
		break;
		
	case PRINT_RED:
		ks_puts(RED_COLOR);
		break;

	case PRINT_GREEN:
		ks_puts(GREEN_COLOR);
		break;

	case PRINT_MAGENTA:
		ks_puts(MAGENTA_COLOR);
		break;

	case PRINT_YELLOW:
		ks_puts(YELLOW_COLOR);
		break;

	case PRINT_BLUE:
		ks_puts(BLUE_COLOR);
		break;

	case PRINT_WHITE:
		ks_puts(WHITE_COLOR);
		break;
	}

  
	va_start(argptr,form);
	vsprintf(console_tx_buffer, form, argptr );
	va_end(argptr);

#if !defined(__USE_SPI_TERMINAL__)	
	for(i=0;console_tx_buffer[i];i++)
		uart_put(console_tx_buffer[i]);
#else
	wait_print_tx_interval();
	spi_master_tx_rx(get_AFE4400_SPI_Address(), strlen(console_tx_buffer), (uint8_t *)console_tx_buffer, (uint8_t *)console_tx_buffer);
#endif


	ks_puts(NORMAL_COLOR);

	return TRUE;
}





