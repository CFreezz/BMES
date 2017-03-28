// I2CTestMain.c
// Runs on LM4F120/TM4C123
// Test the functions provided in I2C0.c by periodically sampling
// the temperature, parsing the result, and sending it to the UART.
// Daniel Valvano
// May 3, 2015

/* This example accompanies the book
   "Embedded Systems: Real Time Interfacing to Arm Cortex M Microcontrollers",
   ISBN: 978-1463590154, Jonathan Valvano, copyright (c) 2014
   Section 8.6.4 Programs 8.5, 8.6 and 8.7

 Copyright 2014 by Jonathan W. Valvano, valvano@mail.utexas.edu
    You may use, edit, run or distribute this file
    as long as the above copyright notice remains
 THIS SOFTWARE IS PROVIDED "AS IS".  NO WARRANTIES, WHETHER EXPRESS, IMPLIED
 OR STATUTORY, INCLUDING, BUT NOT LIMITED TO, IMPLIED WARRANTIES OF
 MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE APPLY TO THIS SOFTWARE.
 VALVANO SHALL NOT, IN ANY CIRCUMSTANCES, BE LIABLE FOR SPECIAL, INCIDENTAL,
 OR CONSEQUENTIAL DAMAGES, FOR ANY REASON WHATSOEVER.
 For more information about my classes, my research, and my books, see
 http://users.ece.utexas.edu/~valvano/
 */

// U0Rx (VCP receive) connected to PA0
// U0Tx (VCP transmit) connected to PA1
// I2C0SCL connected to PB2 and to pin 4 of HMC6352 compass or pin 3 of TMP102 thermometer
// I2C0SDA connected to PB3 and to pin 3 of HMC6352 compass or pin 2 of TMP102 thermometer
// SCL and SDA lines pulled to +3.3 V with 10 k resistors (part of breakout module)
// ADD0 pin of TMP102 thermometer connected to GND
#include <stdint.h>
#include "I2C0.h"
#include "PLL.h"
#include "UART.h"
#include <stdarg.h>
#include <stdbool.h>
#include "../inc/hw_i2c.h"
#include "../inc/hw_memmap.h"
#include "../inc/hw_types.h"
#include "../inc/hw_gpio.h"
#include "../driverlib/i2c.h"
#include "../driverlib/sysctl.h"
#include "../driverlib/gpio.h"
#include "../driverlib/pin_map.h"
// For debug purposes, this program may peek at the I2C0 Master
// Control/Status Register to try to provide a more meaningful
// diagnostic message in the event of an error.  The rest of the
// interface with the I2C hardware occurs through the functions
// in I2C0.c.
#define I2C0_MASTER_MCS_R       (*((volatile unsigned long *)0x40020004))

//#define PART_TM4C1230E6PM
//#define TARGET_IS_TM4C123_RB1

#define GPIO_PB2_I2C0SCL        0x00010803
#define GPIO_PB3_I2C0SDA        0x00010C03

// DEBUGPRINTS==0 configures for no test prints, other value prints test text
// This tests the math used to convert the raw temperature value
// from the thermometer to a string that is displayed.  Verify
// that the left and right columns are the same.
#define DEBUGPRINTS 0
// DEBUGWAIT is time between test prints as a parameter for the Delay() function
// DEBUGWAIT==16,666,666 delays for 1 second between lines
// This is useful if the computer terminal program has limited
// screen or log space to prevent the text from scrolling too
// fast.
// Definition has no effect if test prints are off.
#define DEBUGWAIT   16666666

// delay function for testing from sysctl.c
// which delays 3*ulCount cycles
//#ifdef __TI_COMPILER_VERSION__
//  //Code Composer Studio Code
//  void Delay(unsigned long ulCount){
//  __asm (  "    subs    r0, #1\n"
//      "    bne     Delay\n"
//      "    bx      lr\n");
//}

//#else
//  //Keil uVision Code
//  __asm void
//  Delay(unsigned long ulCount)
//  {
//    subs    r0, #1
//    bne     Delay
//    bx      lr
//  }

//#endif
// function parses raw 16-bit number from temperature sensor in form:
// rawdata[0] = 0
// rawdata[15:8] 8-bit signed integer temperature
// rawdata[7:4] 4-bit unsigned fractional temperature (units 1/16 degree C)
//  or
// rawdata[0] = 1
// rawdata[15:7] 9-bit signed integer temperature
// rawdata[6:3] 4-bit unsigned fractional temperature (units 1/16 degree C)


//sends an I2C command to the specified slave
void I2CSend(uint8_t slave_addr, uint8_t num_of_args, ...)
{
    // Tell the master module what address it will place on the bus when
    // communicating with the slave.
    I2CMasterSlaveAddrSet(I2C0_BASE, slave_addr, false);
     
    //stores list of variable number of arguments
    va_list vargs;
     
    //specifies the va_list to "open" and the last fixed argument
    //so vargs knows where to start looking
    va_start(vargs, num_of_args);
     
    //put data to be sent into FIFO
    I2CMasterDataPut(I2C0_BASE, va_arg(vargs, uint32_t));
     
    //if there is only one argument, we only need to use the
    //single send I2C function
    if(num_of_args == 1)
    {
        //Initiate send of data from the MCU
        I2CMasterControl(I2C0_BASE, I2C_MASTER_CMD_SINGLE_SEND);
         
        // Wait until MCU is done transferring.
        while(I2CMasterBusy(I2C0_BASE));
         
        //"close" variable argument list
        va_end(vargs);
    }
     
    //otherwise, we start transmission of multiple bytes on the
    //I2C bus
    else
    {
        //Initiate send of data from the MCU
        I2CMasterControl(I2C0_BASE, I2C_MASTER_CMD_BURST_SEND_START);
         
        // Wait until MCU is done transferring.
        while(I2CMasterBusy(I2C0_BASE));
         
        //send num_of_args-2 pieces of data, using the
        //BURST_SEND_CONT command of the I2C module
        for(uint8_t i = 1; i < (num_of_args - 1); i++)
        {
            //put next piece of data into I2C FIFO
            I2CMasterDataPut(I2C0_BASE, va_arg(vargs, uint32_t));
            //send next data that was just placed into FIFO
            I2CMasterControl(I2C0_BASE, I2C_MASTER_CMD_BURST_SEND_CONT);
     
            // Wait until MCU is done transferring.
            while(I2CMasterBusy(I2C0_BASE));
        }
     
        //put last piece of data into I2C FIFO
        I2CMasterDataPut(I2C0_BASE, va_arg(vargs, uint32_t));
        //send next data that was just placed into FIFO
        I2CMasterControl(I2C0_BASE, I2C_MASTER_CMD_BURST_SEND_FINISH);
        // Wait until MCU is done transferring.
        while(I2CMasterBusy(I2C0_BASE));
         
        //"close" variable args list
        va_end(vargs);
    }
}
//read specified register on slave device
uint32_t I2CReceive(uint32_t slave_addr, uint8_t reg)
{
    //specify that we are writing (a register address) to the
    //slave device
    I2CMasterSlaveAddrSet(I2C0_BASE, slave_addr, false);
 
    //specify register to be read
    I2CMasterDataPut(I2C0_BASE, reg);
 
    //send control byte and register address byte to slave device
    I2CMasterControl(I2C0_BASE, I2C_MASTER_CMD_BURST_SEND_START);
     
    //wait for MCU to finish transaction
    while(I2CMasterBusy(I2C0_BASE));
     
    //specify that we are going to read from slave device
    I2CMasterSlaveAddrSet(I2C0_BASE, slave_addr, true);
     
    //send control byte and read from the register we
    //specified
    I2CMasterControl(I2C0_BASE, I2C_MASTER_CMD_SINGLE_RECEIVE);
     
    //wait for MCU to finish transaction
    while(I2CMasterBusy(I2C0_BASE));
     
    //return data pulled from the specified register
    return I2CMasterDataGet(I2C0_BASE);
}
//initialize I2C module 0
//Slightly modified version of TI's example code
uint32_t	g_ui32SysClock;
void InitI2C0(void)
{
    //enable I2C module 0
    SysCtlPeripheralEnable(SYSCTL_PERIPH_I2C0);
 
    //reset module
    SysCtlPeripheralReset(SYSCTL_PERIPH_I2C0);
     
    //enable GPIO peripheral that contains I2C 0
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOB);
 
    // Configure the pin muxing for I2C0 functions on port B2 and B3.
    GPIOPinConfigure(GPIO_PB2_I2C0SCL);
    GPIOPinConfigure(GPIO_PB3_I2C0SDA);

     
    // Select the I2C function for these pins.
    GPIOPinTypeI2CSCL(GPIO_PORTB_BASE, GPIO_PIN_2);
    GPIOPinTypeI2C(GPIO_PORTB_BASE, GPIO_PIN_3);
 
    // Enable and initialize the I2C0 master module.  Use the system clock for
    // the I2C0 module.  The last parameter sets the I2C data transfer rate.
    // If false the data rate is set to 100kbps and if true the data rate will
    // be set to 400kbps.
		//g_ui32SysClock = SysCtlClockFreqSet((SYSCTL_OSC_MAIN | SYSCTL_XTAL_25MHZ | SYSCTL_USE_PLL | SYSCTL_CFG_VCO_320), 40000000);
//    I2CMasterInitExpClk(I2C0_BASE, SysCtlClockGet(), false);
//		I2CMasterDisable(I2C0_BASE);
//g_ui32SysClock = SysCtlClockFreqSet((SYSCTL_OSC_MAIN | SYSCTL_XTAL_25MHZ | SYSCTL_USE_PLL | SYSCTL_CFG_VCO_320), 40000000);
//I2CMasterInitExpClk(I2C4_BASE, g_ui32SysClock, false);
	 I2CSlaveEnable(I2C0_BASE);
   I2CSlaveInit(I2C0_BASE,0x08);
	 
//	 I2CSlaveFIFOEnable(I2C0_BASE, I2C_FIFO_CFG_TX_SLAVE | I2C_FIFO_CFG_RX_SLAVE); 
	 I2CTxFIFOConfigSet(I2C0_BASE, I2C_FIFO_CFG_TX_SLAVE);	
	 I2CRxFIFOConfigSet(I2C0_BASE, I2C_FIFO_CFG_RX_SLAVE);		
 
    //clear I2C FIFOs
    HWREG(I2C0_BASE + I2C_O_FIFOCTL) = 80008000;
}
 int tmp;
	int byte = 0;
//volatile uint16_t heading = 0;
//volatile uint8_t controlReg = 0;
int main(void){
 // unsigned short rawData = 0;             // 16-bit data straight from thermometer
 // int tempInt = 0;                        // integer value of temperature (-128 to 127)
 // int tempFra = 0;                        // fractional value of temperature (0 to 9375)
//  PLL_Init(Bus80MHz);
 // UART_Init();
 // I2C_Init();

    // Set the clocking to run directly from the external crystal/oscillator.
//    SysCtlClockSet(SYSCTL_SYSDIV_1 | SYSCTL_USE_PLL | SYSCTL_OSC_INT | SYSCTL_XTAL_16MHZ);
// SysCtlClockFreqSet((SYSCTL_OSC_MAIN | SYSCTL_XTAL_25MHZ | SYSCTL_USE_PLL | SYSCTL_CFG_VCO_320), 40000000);
    //initialize I2C module 0
    InitI2C0();

 
/* Configuration for 30 FPS */
 
// CLKRC register: Prescaler = 2
	
	//
//	tmp = I2CReceive(0x11,1);	
//	I2CSend(0x11, tmp);
	

    while(1){
			//! \return Returns \b I2C_SLAVE_ACT_NONE to indicate that no action has been
//! requested of the I2C Slave, \b I2C_SLAVE_ACT_RREQ to indicate that
//! an I2C master has sent data to the I2C Slave, \b I2C_SLAVE_ACT_TREQ
//! to indicate that an I2C master has requested that the I2C Slave send
//! data, \b I2C_SLAVE_ACT_RREQ_FBR to indicate that an I2C master has sent
//! data to the I2C slave and the first byte following the slave's own address
//! has been received, \b I2C_SLAVE_ACT_OWN2SEL to indicate that the second I2C
//! slave address was matched, \b I2C_SLAVE_ACT_QCMD to indicate that a quick
//! command was received, and \b I2C_SLAVE_ACT_QCMD_DATA to indicate that the
//! data bit was set when the quick command was received.
		tmp =	I2CSlaveStatus(I2C0_BASE);
			if(tmp != 0){
				tmp = tmp + 0;
			}
			if(tmp == I2C_SLAVE_ACT_RREQ){
				byte = I2CSlaveDataGet(I2C0_BASE);
			}else if(tmp == I2C_SLAVE_ACT_TREQ){
				I2CSlaveDataPut(I2C0_BASE,0x12);
			}
			
			
			
			
	//	tmp = I2CReceive(0x43,0x01);
	//	I2CSend(0x43,0x01);	
		
		};
	
  
}
