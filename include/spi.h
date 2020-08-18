
#include  "define.h"


#define SPI_OPERATING_FREQUENCY  ( 0x02000000UL << (uint32_t)Freq_8Mbps )  // by skim  /*!< Slave clock frequency. */

/*  SPI0 */
#ifdef  _DK_BOARD_

#define SPI_PSELSCK0              23   /*!< GPIO pin number for SPI clock (note that setting this to 31 will only work for loopback purposes as it not connected to a pin) */
#define SPI_PSELMOSI0             20   /*!< GPIO pin number for Master Out Slave In    */
#define SPI_PSELMISO0             22   /*!< GPIO pin number for Master In Slave Out    */
#define SPI_PSELSS0               21   /*!< GPIO pin number for Slave Select           */

#else

#define SPI_PSELSCK0              3   /*!< GPIO pin number for SPI clock  */
#define SPI_PSELMOSI0             5   /*!< GPIO pin number for Master Out Slave In    */
#define SPI_PSELMISO0             4   /*!< GPIO pin number for Master In Slave Out    */
#define SPI_PSELSS0               6   /*!< GPIO pin number for Slave Select           */

#endif

/*  SPI1 */

#if (__BOARD2__ || __BOARD3__)
#define SPI_PSELSCK1              1   /*!< new board             */
#define	PIN_CLK_32768		27

#else
#define SPI_PSELSCK1              27   /*!< GPIO pin number for SPI clock              */

#endif

#define SPI_PSELMOSI1             28   /*!< GPIO pin number for Master Out Slave In    */
#define SPI_PSELMISO1             29   /*!< GPIO pin number for Master In Slave Out    */
#if __BOARD3__
#define SPI_PSELSS1               10   /*!< GPIO pin number for Slave Select           */
#else
#define SPI_PSELSS1               26   /*!< GPIO pin number for Slave Select           */
#endif

//#define DEBUG
#ifdef DEBUG
#define DEBUG_EVENT_READY_PIN0    10    /*!< when DEBUG is enabled, this GPIO pin is toggled everytime READY_EVENT is set for SPI0, no toggling means something has gone wrong */
#define DEBUG_EVENT_READY_PIN1    11    /*!< when DEBUG is enabled, this GPIO pin is toggled everytime READY_EVENT is set for SPI1, no toggling means something has gone wrong */
#endif

#define NUMBER_OF_TEST_BYTES     2    /*!< number of bytes to send to slave to test if Initialization was successful */
#define TEST_BYTE                0xBB /*!< Randomly chosen test byte to transmit to spi slave */
#define TIMEOUT_COUNTER          0x3000UL  /*!< timeout for getting rx bytes from slave */

/** @def  TX_RX_MSG_LENGTH
 * number of bytes to transmit and receive. This amount of bytes will also be tested to see that
 * the received bytes from slave are the same as the transmitted bytes from the master */
#define TX_RX_MSG_LENGTH   100

/** @def ERROR_PIN_SPI0
 * This pin is set active high when there is an error either in TX/RX for SPI0 or if the received bytes does not totally match the transmitted bytes.
 * This functionality can be tested by temporarily disconnecting the MISO pin while running this example.
 */
#define ERROR_PIN_SPI0   8UL

/** @def ERROR_PIN_SPI1
 * This pin is set active high when there is an error either in TX/RX for SPI1 or if the received bytes does not totally match the transmitted bytes.
 * This functionality can be tested by temporarily disconnecting the MISO pin while running this example.
 */
#define ERROR_PIN_SPI1   9UL
























