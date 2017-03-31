#include "..//inc//tm4c123gh6pm.h"
#include <stdint.h>
#include "Timer0A.h"

/*

     __________________
 L  |    L        R    | R
    |____R_      _L____|
           |    |        
           |    |
					  NXT.
*/

#define PF1       (*((volatile uint32_t *)0x40025008))//Left Big Servo
#define PF2       (*((volatile uint32_t *)0x40025010))//Left Small Servo Left
#define PF3       (*((volatile uint32_t *)0x40025020))//Left Small Servo Right
#define PD1 			(*((volatile uint32_t *)0x40007008))//Right Big Servo
#define PD2 			(*((volatile uint32_t *)0x40007010))//Right Small Servo Left
#define PD3 			(*((volatile uint32_t *)0x40007020))//Right Small Servo Right
	
void EnableInterrupts(void);  // Enable interrupts

int start = 0;
uint32_t itr = 1;
uint32_t LSSL_closed = 195;
uint32_t LSSL_open = 145;
uint32_t LSSR_closed = 65;
uint32_t LSSR_open = 115;
uint32_t RSSL_closed = 194;
uint32_t RSSL_open = 144;
uint32_t RSSR_closed = 59;
uint32_t RSSR_open = 109;


void reset(void){
	PF1 = 0x02;
	PF2 = 0x04;
	PF3 = 0x08;
	PD1 = 0x02;
	PD2 = 0x04;
	PD3 = 0x08;
}

uint32_t duty_pf1 = 0;
uint32_t duty_pf2 = 0;
uint32_t duty_pf3 = 0;
uint32_t duty_pd1 = 0;
uint32_t duty_pd2 = 0;
uint32_t duty_pd3 = 0;

void increment(void){
	if(!start){
		return;
	}
	
	itr++;
	
	if(itr == duty_pf1){
		PF1 = 0x0;	
	}
	if(itr == duty_pf2){
		PF2 = 0x0;
	}
	if(itr == duty_pf3){
		PF3 = 0x0;
	}
	if(itr == duty_pd1){
		PD1 = 0x0;
	}
	if(itr == duty_pd2){
		PD2 = 0x0;
	}
	if(itr == duty_pd3){
		PD3 = 0x0;
	}
	if(itr>=2000){
		reset();
		itr=1;
	}
}

//MUST HAVE 80MHz CLOCK
void servo_init(void){
	start = 0;
	SYSCTL_RCGCGPIO_R |= 0x20;       // activate port F
  while((SYSCTL_PRGPIO_R&0x0020) == 0){};// ready?
  GPIO_PORTF_DIR_R |= 0x0E;        // make PF3-1 output (PF3-1 built-in LEDs)
  GPIO_PORTF_AFSEL_R &= ~0x0E;     // disable alt funct on PF3-1
  GPIO_PORTF_DEN_R |= 0x0E;        // enable digital I/O on PF3-1
                                   // configure PF3-1 as GPIO
  GPIO_PORTF_PCTL_R = (GPIO_PORTF_PCTL_R&0xFFFFF0FF)+0x00000000;
  GPIO_PORTF_AMSEL_R = 0;          // disable analog functionality on PF
		
	SYSCTL_RCGCGPIO_R |= 0x08;        // 1) activate port D
  while((SYSCTL_PRGPIO_R&0x08)==0){};   // allow time for clock to stabilize
                                    // 2) no need to unlock PD3-0
  GPIO_PORTD_AMSEL_R &= ~0x0F;      // 3) disable analog functionality on PD3-0
  GPIO_PORTD_PCTL_R &= ~0x0000FFFF; // 4) GPIO
  GPIO_PORTD_DIR_R |= 0x0F;         // 5) make PD3-0 out
  GPIO_PORTD_AFSEL_R &= ~0x0F;      // 6) regular port function
  GPIO_PORTD_DEN_R |= 0x0F;         // 7) enable digital I/O on PD3-0
	Timer0A_Init(&increment, 800);  // initialize timer0A (16 Hz)
}

void servo_start(void){
	start = 1;
	EnableInterrupts();
}


uint32_t LBS_forward = 125;
uint32_t LBS_middle = 157;
uint32_t LBS_backward = 190;
uint32_t RBS_forward = 180;
uint32_t RBS_middle = 145;
uint32_t RBS_backward = 115;
void servo_Lforward(void){
	duty_pf1 = LBS_forward;
}

void servo_Lbackward(void){
	duty_pf1 = LBS_backward;
}

void servo_Rforward(void){
	duty_pd1 = RBS_forward;
}

void servo_Rbackward(void){
	duty_pd1 = RBS_backward;
}

void servo_Lmiddle(void){
	duty_pf1 = LBS_middle;
}

void servo_Rmiddle(void){
	duty_pd1 = RBS_middle;
}
