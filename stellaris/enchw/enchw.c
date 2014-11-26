/* ENC28J60 hardware implementation for EMLIB devices.
 *
 * The enchw_device_t multi-device support is unused so far as it was added to
 * enc28j60.c later */

#include "enchw.h"
#include <inc/hw_memmap.h>
#include <inc/hw_ssi.h>
#include <inc/hw_types.h>
#include <driverlib/ssi.h>
#include <driverlib/uart.h>
#include <driverlib/gpio.h>
#include <driverlib/sysctl.h>
#include "encdebug.h"

//#include "enchw-config.h"

/*
 * Port mapping:
 * 		PE0 : SSI1Clk
 * 		PE1 : SSI1Fss
 * 		PE2 : SSI1Rx
 * 		PE3 : SSI1Tx
 */


static volatile uint8_t j=0;
#define pause() //j=250u;while(++j)

void enchw_setup(enchw_device_t __attribute__((unused)) *dev)
{
    //
    // The SSI0 peripheral must be enabled for use.
    //
//    SysCtlPeripheralEnable(SYSCTL_PERIPH_SSI1);

    //
    // For this example SSI0 is used with PortA[5:2].  The actual port and pins
    // used may be different on your part, consult the data sheet for more
    // information.  GPIO port A needs to be enabled so these pins can be used.
    // TODO: change this to whichever GPIO port you are using.
    //
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOE);

    //
    // Configure the pin muxing for SSI0 functions on port A2, A3, A4, and A5.
    // This step is not necessary if your part does not support pin muxing.
    //
/*    GPIOPinConfigure(GPIO_PE0_SSI1CLK);
    GPIOPinConfigure(GPIO_PE1_SSI1FSS);
    GPIOPinConfigure(GPIO_PE2_SSI1RX);
    GPIOPinConfigure(GPIO_PE3_SSI1TX);
*/
    //
    // Configure the GPIO settings for the SSI pins.  This function also gives
    // control of these pins to the SSI hardware.  Consult the data sheet to
    // see which functions are allocated per pin.
    //
//    GPIOPinTypeSSI(GPIO_PORTE_BASE, GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3);
//    GPIOPadConfigSet(GPIO_PORTE_BASE, GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3, GPIO_STRENGTH_2MA, GPIO_PIN_TYPE_STD);
//    GPIODirModeSet(GPIO_PORTE_BASE, GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_3, GPIO_DIR_MODE_OUT);
//    GPIODirModeSet(GPIO_PORTE_BASE, GPIO_PIN_2, GPIO_DIR_MODE_IN);

    GPIOPinTypeGPIOInput(GPIO_PORTE_BASE, GPIO_PIN_2);
    GPIOPinTypeGPIOOutput(GPIO_PORTE_BASE, GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_3);

    //
    // Configure and enable the SSI port for SPI master mode.  Use SSI0,
    // system clock supply, idle clock level low and active low clock in
    // freescale SPI mode, master mode, 1MHz SSI frequency, and 8-bit data.
    // For SPI mode, you can set the polarity of the SSI clock when the SSI
    // unit is idle.  You can also configure what clock edge you want to
    // capture data on.  Please reference the datasheet for more information on
    // the different SPI modes.
    //
    //SSIConfigSetExpClk(SSI1_BASE, SysCtlClockGet(), SSI_FRF_MOTO_MODE_0,
    //                   SSI_MODE_MASTER, 1000000, 8);

    //
    // Enable the SSI0 module.
    //
    //SSIEnable(SSI1_BASE);
}

#define CLK_LOW() GPIOPinWrite(GPIO_PORTE_BASE, GPIO_PIN_0, 0)
#define CLK_HIGH() GPIOPinWrite(GPIO_PORTE_BASE, GPIO_PIN_0, GPIO_PIN_0)

void enchw_select(enchw_device_t __attribute__((unused)) *dev)
{
	CLK_LOW();
	GPIOPinWrite(GPIO_PORTE_BASE, GPIO_PIN_1, 0);
	pause();
}

void enchw_unselect(enchw_device_t __attribute__((unused)) *dev)
{
	GPIOPinWrite(GPIO_PORTE_BASE, GPIO_PIN_1, 2);
	CLK_LOW();
	pause();
}

static void debugChar(uint8_t data){
	static const char digits [] = "0123456789ABCDEF";

	uint8_t index = (data&0xF0u)>>4u;
	UARTCharPut(UART0_BASE, digits[index]);
	index = data&0xFu;
	UARTCharPut(UART0_BASE, digits[index]);
}

static void debugData(uint8_t sent, uint8_t recvd){
	debugChar(sent);
	UARTCharPut(UART0_BASE, ':');
	debugChar(recvd);
	UARTCharPut(UART0_BASE, ' ');
}

uint8_t enchw_exchangebyte(enchw_device_t __attribute__((unused)) *dev, uint8_t byte)
{

	uint8_t result = 0x0u;
	int i=0;

	for(i=0;i<8;i++){
		uint8_t mask = 1u<<(7-i);

		// write output
		GPIOPinWrite(GPIO_PORTE_BASE, GPIO_PIN_3, (byte & mask) ? GPIO_PIN_3 : 0);

		//Toggle clock
		CLK_HIGH();
		pause();

		// read input
		result |= (GPIOPinRead(GPIO_PORTE_BASE, GPIO_PIN_2)) ?  mask : 0;

		//Toggle clock
		CLK_LOW();
		pause();

	}

	enc_debug(0, 1);

	return result;
}
