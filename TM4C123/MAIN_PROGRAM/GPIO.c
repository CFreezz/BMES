// GPIO.c
// Runs on LM4F120/TM4C123
// Initialize four GPIO pins as outputs.  Continually generate output to
// drive simulated stepper motor.
// Daniel Valvano
// May 3, 2015

/* This example accompanies the books
  "Embedded Systems: Introduction to ARM Cortex M Microcontrollers",
  ISBN: 978-1469998749, Jonathan Valvano, copyright (c) 2015
  Volume 1 Program 4.5

"Embedded Systems: Real Time Interfacing to ARM Cortex M Microcontrollers",
   ISBN: 978-1463590154, Jonathan Valvano, copyright (c) 2015
   Volume 2 Example 2.3, Program 2.11, Figure 2.32

 Copyright 2015 by Jonathan W. Valvano, valvano@mail.utexas.edu
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

// PD3 is an output to LED3, negative logic
// PD2 is an output to LED2, negative logic
// PD1 is an output to LED1, negative logic
// PD0 is an output to LED0, negative logic

//#define GPIO_PORTB_DATA_BITS_R  ((volatile uint32_t *)0x40005000)
	
//#define SCL  (*((volatile uint32_t *)0x40005010)) //PB2
//#define SDA  (*((volatile uint32_t *)0x40005020)) //PB3
#include <stdint.h>
#include "../inc/tm4c123gh6pm.h"
#include "SysTick.h"
#include "PLL.h"
#include "ADCSWTrigger.h"
#include "I2C.h"

#define LEDS (*((volatile uint32_t *)0x4000703C))
// access PD3-PD0
//------------Board_Init------------
// Initialize GPIO Port F for negative logic switches on PF0 and
// PF4 as the Launchpad is wired.  Weak internal pull-up
// resistors are enabled, and the NMI functionality on PF0 is
// disabled.
// Input: none
// Output: none
#define GPIO_LOCK_KEY           0x4C4F434B  // Unlocks the GPIO_CR register
#define PF0                     (*((volatile uint32_t *)0x40025004))
#define PF4                     (*((volatile uint32_t *)0x40025040))
#define PA5                     (*((volatile uint32_t *)0x40004080))
#define SWITCHES                (*((volatile uint32_t *)0x40025044))
#define SW1       0x10                      // on the left side of the Launchpad board
#define SW2       0x01                      // on the right side of the Launchpad board
#define SYSCTL_RCGC2_GPIOF      0x00000020  // port F Clock Gating Control
void Board_Init(void){            
  SYSCTL_RCGCGPIO_R |= 0x20;     // 1) activate Port F
  while((SYSCTL_PRGPIO_R&0x20) == 0){};// ready?
                                 // 2a) unlock GPIO Port F Commit Register
  GPIO_PORTF_LOCK_R = GPIO_LOCK_KEY;
  GPIO_PORTF_CR_R |= (SW1|SW2);  // 2b) enable commit for PF4 and PF0
                                 // 3) disable analog functionality on PF4 and PF0
  GPIO_PORTF_AMSEL_R &= ~(SW1|SW2);
                                 // 4) configure PF0 and PF4 as GPIO
  GPIO_PORTF_PCTL_R = (GPIO_PORTF_PCTL_R&0xFFF0FFF0)+0x00000000;
  GPIO_PORTF_DIR_R &= ~(SW1|SW2);// 5) make PF0 and PF4 in (built-in buttons)
                                 // 6) disable alt funct on PF0 and PF4
  GPIO_PORTF_AFSEL_R &= ~(SW1|SW2);
//  delay = SYSCTL_RCGC2_R;        // put a delay here if you are seeing erroneous NMI
  GPIO_PORTF_PUR_R |= (SW1|SW2); // enable weak pull-up on PF0 and PF4
  GPIO_PORTF_DEN_R |= (SW1|SW2); // 7) enable digital I/O on PF0 and PF4
}

//------------Board_Input------------
// Read and return the status of the switches.
// Input: none
// Output: 0x01 if only Switch 1 is pressed
//         0x10 if only Switch 2 is pressed
//         0x00 if both switches are pressed
//         0x11 if no switches are pressed
uint32_t Board_Input(void){
  return SWITCHES;
}
void GPIO_Init(void){
  SYSCTL_RCGCGPIO_R |= 0x08;        // 1) activate port D
  while((SYSCTL_PRGPIO_R&0x08)==0){};   // allow time for clock to stabilize
                                    // 2) no need to unlock PD3-0
  GPIO_PORTD_AMSEL_R &= ~0x0F;      // 3) disable analog functionality on PD3-0
  GPIO_PORTD_PCTL_R &= ~0x0000FFFF; // 4) GPIO
  GPIO_PORTD_DIR_R |= 0x0F;         // 5) make PD3-0 out
  GPIO_PORTD_AFSEL_R &= ~0x0F;      // 6) regular port function
  GPIO_PORTD_DEN_R |= 0x0F;         // 7) enable digital I/O on PD3-0
}



int32_t count = 'A';


////////
// List of Codes
////////
//
//'A' : Request Motor A Position
//'B' : Request Motor B Position
//'C' : Request Motor C Position
//'D' : Motor A Go Forward
//'E' : Motor B Go Forward
//'F' : Motor C Go Forward
//'G' : Motor A Go Backward
//'H' : Motor B Go Backward
//'I' : Motor C Go Backward
//'J' : Motor A Stop
//'K' : Motor B Stop
//'L' : Motor C Stop
//'M' : Set Motor A Position
//'N' : Set Motor B Position
//'O' : Set Motor C Position
//'P'
//'Q'
//'R'
//'S' : Request Color Sensor To Detect For Red
//'T'
//'U' : Request UltraSonic Sensor Value
//'V'
//'W'
//'X'
//'Y'
//'Z'

int32_t bloodStart = 0;
int32_t bloodEnd = 0;
int32_t Phase1(void){
	//search for the blooood
	//assume motors a b and c are 0, initially
	//move motor b forward while checking color and ultrasonic
	I2C_Message('D');
	int32_t color = I2C_Message('S');
	int32_t US = I2C_Message('U');
	while((color != 1) && (US < 10)){
		 color = I2C_Message('S');
		 US = I2C_Message('U');	
	}
	if(US >= 10){
		I2C_Message('K');//stop
		I2C_MessageSet('N',0);//reset
		return 0;//failure
	}
	//get the start of the blood
	bloodStart = I2C_Message('B');
	while((color != 1) && (US < 10)){
		 color = I2C_Message('S');
		 US = I2C_Message('U');	
	}	
	//get the end of the blood
	I2C_Message('K');//stop	
	bloodEnd = I2C_Message('B');	
	//extend motor A with the cleaning wipe
	I2C_MessageSet('M',100);
	//move motor B backward to beggining of blood
	I2C_MessageSet('N',bloodStart);
	//
	
	return 1;//success
}






int main(void){ // reset clears AFSEL, PCTL, AMSEL
	PLL_Init(Bus80MHz);
  SysTick_Init();
	Board_Init();
	I2C_Init();
	int32_t ret = 0;
	while(1){
		if(count != 'M'){
		 ret = I2C_Message(count);
		}else{
		 ret = I2C_MessageSet(count,'B');	
		}
		if(ret == 0x60){
			int32_t breakpoint = 0;
		}
		while(Board_Input() == 0x11){};
			count = count + 1;
			if(count > 'z'){
				count = 'A';
			}
		while(Board_Input() != 0x11){};		
			
		
	}
	

}
