//------------------------------------------------------------------//
//2001.08.23
//Chih-Hsiung Shih
//1.Add two data types:UInt64, SIn64. These two data types can only be used in 
//	Microsoft C++, since  they have no ANSI equivalent.
//2.Still not present confitional preprocessor for WIN98, WIN2000, WINNT....
//------------------------------------------------------------------//
#ifndef	__PIXART_SOFTWARE_BASE_DATA_TYPE__
#define	__PIXART_SOFTWARE_BASE_DATA_TYPE__
//------------------------------------------------------------------//
// For Microsoft Windows Serise
//------------------------------------------------------------------//
	typedef	unsigned long int		UInt32;
	typedef unsigned short int		UInt16;
	typedef unsigned char			UInt8;

	typedef signed long int			SInt32;
	typedef signed short int		SInt16;
	typedef signed char				SInt8;
	
//	#define	NULL					0

	#define	_TRUE					1	
	#define _FALSE					0	
	#define	_NULL					0
	#define _PI						3.14159265358979

	#define	IN
	#define OUT
	#define	INOUT

	#define	_IN
	#define _OUT
	#define	_INOUT

	typedef signed char			int8_T;
	typedef unsigned char		uint8_T;
	typedef short				int16_T;
	typedef unsigned short		uint16_T;
	typedef int					int32_T;
	typedef unsigned int		uint32_T;
	typedef float				real32_T;
	typedef double				real64_T;

// For UNIX
// For Linux
// For Mac
//....
#ifndef __cplusplus 
//		typedef unsigned char bool;
	#define false 0
	#define true 1
#endif

#ifndef NULL
	#define NULL 0
#endif
#endif
