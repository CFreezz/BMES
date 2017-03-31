//I2C.c
//
// A bit bash that receives and sends 1 byte thru
//	the NXT I2C Protocol
//
// The low voltage of the NXT is too high to be recognized by the tm4c123
//	so 2 ADC on PE4 and PE5 are used, this should be replaced by analog circuits
//
// The NXT is the master and drives the SCL, 
// 	the tm4c is slave and does not have open drain turned on
//
// sorry in advance
//
//	PE4 -  ADC  - SCL Recieve
//	PE5 -  ADC  - SDA Receive
//  PB3 - GPIO  - SDA Send
//  Yes the SDA lines are connected
#define SCL  (*((volatile uint32_t *)0x40005010)) //PB2
#define SDA  (*((volatile uint32_t *)0x40005020)) //PB3
#include <stdint.h>
#include "../inc/tm4c123gh6pm.h"
#include "ADCSWTrigger.h"
#include "I2C.h"
//
//
//////////////////////////////
// I2C_Init
// initializes the ADC and sda pin
// Input: none
// Output: none
////////////////////////////////////////
//change to the correct SDA send pin, there is no SCL Send pin
void I2C_Init(void){
		SYSCTL_RCGCGPIO_R |= 0x0002;          // activate port B
  while((SYSCTL_PRGPIO_R&0x0002) == 0){};// ready?
  GPIO_PORTB_AFSEL_R &= ~0x0C;           // 3) disable alt funct on PB2,3
  GPIO_PORTB_DEN_R |= 0x0C;             // 5) enable digital I/O on PB2,3
                                        // 6) configure PB2,3 as I2C
  GPIO_PORTB_PCTL_R &= ~0x0000FF00;;//+0x00003300;
  GPIO_PORTB_AMSEL_R &= ~0x0C;          // 7) disable analog functionality on PB2,3
  GPIO_PORTB_AFSEL_R &= ~0x0C;      // regular port function		
	GPIO_PORTB_DIR_R &= ~0x08; //BOTH INPUT 
	GPIO_PORTB_DIR_R |= 0x04; //Scl TO OUTPUT
	ADC_Init89();
	
	
}
//
//
// global, sent to nxt
int32_t ToNXT = 0;
// recieved from NXT
int32_t FromNXT = 0;
//
//
//
/////////////////////
// I2C_SendRecieve
// Will recieve exactly one byte from the NXT,
// then send exactly one byte from the tm4c123
// Input: Message to Send
// Output: Message Recieved
///////////////////////////
int32_t test = 0;
uint32_t ADCvals[2];
int32_t I2C_SendRecieve(int32_t ByteToSend){
	//initialize values
	ToNXT = ByteToSend;
	int32_t scl = 0;
	ADC_In89(&ADCvals[0]);
	scl = ADCvals[0];
	// what the SDA recognizes as a HI value
	int vHi = 4000;	
	uint8_t tmp = 0;
	//WARNING this function BLOCKS until NXT has sent a 0x04
	while(1){
		tmp = 0;			
		while(scl > 2300){ ADC_In89(&ADCvals[0]); scl = ADCvals[0];};//initial HI	
			while(scl < 4000){ADC_In89(&ADCvals[0]); scl = ADCvals[0];};
			if(ADCvals[1] > vHi){tmp = tmp + 0x01;}tmp = tmp << 1;
			while(scl > 2300){ADC_In89(&ADCvals[0]); scl = ADCvals[0];};//1
			SCL = 0x04;
			while(scl < 4000){ADC_In89(&ADCvals[0]); scl = ADCvals[0];};
			if(ADCvals[1] > vHi){tmp = tmp + 0x01;}tmp = tmp << 1;
			while(scl > 2300){ADC_In89(&ADCvals[0]); scl = ADCvals[0];};//2
			while(scl < 4000){ADC_In89(&ADCvals[0]); scl = ADCvals[0];};
			if(ADCvals[1] > vHi){tmp = tmp + 0x01;}tmp = tmp << 1;
			while(scl > 2300){ADC_In89(&ADCvals[0]); scl = ADCvals[0];};//3
			while(scl < 4000){ADC_In89(&ADCvals[0]); scl = ADCvals[0];};
			if(ADCvals[1] > vHi){tmp = tmp + 0x01;}tmp = tmp << 1;
			while(scl > 2300){ADC_In89(&ADCvals[0]); scl = ADCvals[0];};//4
			while(scl < 4000){ADC_In89(&ADCvals[0]); scl = ADCvals[0];};
			if(ADCvals[1] > vHi){tmp = tmp + 0x01;}tmp = tmp << 1;
			while(scl > 2300){ADC_In89(&ADCvals[0]); scl = ADCvals[0];};//5
			while(scl < 4000){ADC_In89(&ADCvals[0]); scl = ADCvals[0];};
			if(ADCvals[1] > vHi){tmp = tmp + 0x01;}tmp = tmp << 1;
			while(scl > 2300){ADC_In89(&ADCvals[0]); scl = ADCvals[0];};//6
			while(scl < 4000){ADC_In89(&ADCvals[0]); scl = ADCvals[0];};
			if(ADCvals[1] > vHi){tmp = tmp + 0x01;}tmp = tmp << 1;
			while(scl > 2300){ADC_In89(&ADCvals[0]); scl = ADCvals[0];};//7
			while(scl < 4000){ADC_In89(&ADCvals[0]); scl = ADCvals[0];};
			while(scl > 2300){ADC_In89(&ADCvals[0]); scl = ADCvals[0];};//8, R/W

			test = tmp;
			if(tmp == 0x04){
				break;
			}
			while(scl < 4000){ADC_In89(&ADCvals[0]); scl = ADCvals[0];};
		for(int k = 0; k < 1000; k++){
			int y = 2;
		}
			
	}
	//0x04 recieved, continue I2C
	GPIO_PORTB_DIR_R |= 0x08; //SDA TO OUTPUT
	SDA = 0x00;
  while(scl < 4000){ADC_In89(&ADCvals[0]); scl = ADCvals[0];};
  while(scl > 2300){ADC_In89(&ADCvals[0]); scl = ADCvals[0];};//9, ACK
	GPIO_PORTB_DIR_R &= ~0x08; //SDA TO INPUT	
  while(scl < 4000){ADC_In89(&ADCvals[0]); scl = ADCvals[0];};
/////////////////////////////////////////////////////////////////////////////////////////
// 0x04 recognized, receive byte	
/////////////////////////////////////////////////////////////////////////////////////////
		tmp = 0;
		ADC_In89(&ADCvals[0]);
		if(ADCvals[1] > vHi){tmp = tmp + 0x01;}tmp = tmp << 1;
		while(scl > 2300){ADC_In89(&ADCvals[0]); scl = ADCvals[0];};//1
			while(scl < 4000){ADC_In89(&ADCvals[0]); scl = ADCvals[0];};
			if(ADCvals[1] > vHi){tmp = tmp + 0x01;}tmp = tmp << 1;
			while(scl > 2300){ADC_In89(&ADCvals[0]); scl = ADCvals[0];};//2
			while(scl < 4000){ADC_In89(&ADCvals[0]); scl = ADCvals[0];};
			if(ADCvals[1] > vHi){tmp = tmp + 0x01;}tmp = tmp << 1;
			while(scl > 2300){ADC_In89(&ADCvals[0]); scl = ADCvals[0];};//3
			while(scl < 4000){ADC_In89(&ADCvals[0]); scl = ADCvals[0];};
			if(ADCvals[1] > vHi){tmp = tmp + 0x01;}tmp = tmp << 1;
			while(scl > 2300){ADC_In89(&ADCvals[0]); scl = ADCvals[0];};//4
			while(scl < 4000){ADC_In89(&ADCvals[0]); scl = ADCvals[0];};
			if(ADCvals[1] > vHi){tmp = tmp + 0x01;}tmp = tmp << 1;
			while(scl > 2300){ADC_In89(&ADCvals[0]); scl = ADCvals[0];};//5
			while(scl < 4000){ADC_In89(&ADCvals[0]); scl = ADCvals[0];};
			if(ADCvals[1] > vHi){tmp = tmp + 0x01;}tmp = tmp << 1;
			while(scl > 2300){ADC_In89(&ADCvals[0]); scl = ADCvals[0];};//6
			while(scl < 4000){ADC_In89(&ADCvals[0]); scl = ADCvals[0];};
			if(ADCvals[1] > vHi){tmp = tmp + 0x01;}tmp = tmp << 1;
			while(scl > 2300){ADC_In89(&ADCvals[0]); scl = ADCvals[0];};//7
			while(scl < 4000){ADC_In89(&ADCvals[0]); scl = ADCvals[0];};
			if(ADCvals[1] > vHi){tmp = tmp + 0x01;};			
			while(scl > 2300){ADC_In89(&ADCvals[0]); scl = ADCvals[0];};//8
			
			test = tmp;
			FromNXT = tmp;
			GPIO_PORTB_DIR_R |= 0x08; //SDA TO OUTPUT
			SDA = 0x00;
			while(scl < 4000){ADC_In89(&ADCvals[0]); scl = ADCvals[0];};
			while(scl > 2300){ADC_In89(&ADCvals[0]); scl = ADCvals[0];};//9, ACK
			GPIO_PORTB_DIR_R &= ~0x08; //SDA TO INPUT
			while(scl < 4000){ADC_In89(&ADCvals[0]); scl = ADCvals[0];}; //clock low
	while(scl > 2300){ADC_In89(&ADCvals[0]); scl = ADCvals[0];};	//clock hi REPEAT
	while(scl < 4000){ADC_In89(&ADCvals[0]); scl = ADCvals[0];}; //clock low			
	while(scl > 2300){ADC_In89(&ADCvals[0]); scl = ADCvals[0];};	//clock hi REPEAT	
	while(scl < 4000){ADC_In89(&ADCvals[0]); scl = ADCvals[0];}; //clock low		
///////////////////////////////////////////////////////////////////////////////////////////////////
//byte received, wait on 0x04 again
////////////////////////////////////////////////////////////////////////////////////////////////////
			tmp = 0;
			ADC_In89(&ADCvals[0]);
			if(ADCvals[1] > vHi){tmp = tmp + 0x01;}tmp = tmp << 1;
			while(scl > 2300){ADC_In89(&ADCvals[0]); scl = ADCvals[0];};//1
			while(scl < 4000){ADC_In89(&ADCvals[0]); scl = ADCvals[0];};
			if(ADCvals[1] > vHi){tmp = tmp + 0x01;}tmp = tmp << 1;
			while(scl > 2300){ADC_In89(&ADCvals[0]); scl = ADCvals[0];};//2
			while(scl < 4000){ADC_In89(&ADCvals[0]); scl = ADCvals[0];};
			if(ADCvals[1] > vHi){tmp = tmp + 0x01;}tmp = tmp << 1;
			while(scl > 2300){ADC_In89(&ADCvals[0]); scl = ADCvals[0];};//3
			while(scl < 4000){ADC_In89(&ADCvals[0]); scl = ADCvals[0];};
			if(ADCvals[1] > vHi){tmp = tmp + 0x01;}tmp = tmp << 1;
			while(scl > 2300){ADC_In89(&ADCvals[0]); scl = ADCvals[0];};//4
			while(scl < 4000){ADC_In89(&ADCvals[0]); scl = ADCvals[0];};
			if(ADCvals[1] > vHi){tmp = tmp + 0x01;}tmp = tmp << 1;
			while(scl > 2300){ADC_In89(&ADCvals[0]); scl = ADCvals[0];};//5
			while(scl < 4000){ADC_In89(&ADCvals[0]); scl = ADCvals[0];};
			if(ADCvals[1] > vHi){tmp = tmp + 0x01;}tmp = tmp << 1;
			while(scl > 2300){ADC_In89(&ADCvals[0]); scl = ADCvals[0];};//6
			while(scl < 4000){ADC_In89(&ADCvals[0]); scl = ADCvals[0];};
			if(ADCvals[1] > vHi){tmp = tmp + 0x01;}tmp = tmp << 1;
			while(scl > 2300){ADC_In89(&ADCvals[0]); scl = ADCvals[0];};//7
			while(scl < 4000){ADC_In89(&ADCvals[0]); scl = ADCvals[0];};
			if(ADCvals[1] > vHi){tmp = tmp + 0x01;};			
			while(scl > 2300){ADC_In89(&ADCvals[0]); scl = ADCvals[0];};//8

				test = tmp;//expect address again			
	GPIO_PORTB_DIR_R |= 0x08; //SDA TO OUTPUT
	SDA = 0x00;	
  while(scl < 4000){ADC_In89(&ADCvals[0]); scl = ADCvals[0];};
  while(scl > 2300){ADC_In89(&ADCvals[0]); scl = ADCvals[0];};//9, ACK
	GPIO_PORTB_DIR_R &= ~0x08; //SDA TO INPUT	
  while(scl < 4000){ADC_In89(&ADCvals[0]); scl = ADCvals[0];};
/////////////////////////////////////////////////////////////////////////////////
////		0x04 received, sending byte                                      ////////
/////////////////////////////////////////////////////////////////////////////////
				int32_t t = ToNXT>>4;//ToNXT << 8;
				GPIO_PORTB_DIR_R |= 0x08; //SDA TO OUTPUT	
				SDA = t&(0x00000008);///
				t = ToNXT>>3;
			while(scl > 2300){ADC_In89(&ADCvals[0]); scl = ADCvals[0];};//1
			while(scl < 4000){ADC_In89(&ADCvals[0]); scl = ADCvals[0];};
				SDA = t&(0x00000008);///
				t = ToNXT>>2;
			while(scl > 2300){ADC_In89(&ADCvals[0]); scl = ADCvals[0];};//2
			while(scl < 4000){ADC_In89(&ADCvals[0]); scl = ADCvals[0];};
				SDA = t&(0x00000008);///
				t = ToNXT>>1;			
			while(scl > 2300){ADC_In89(&ADCvals[0]); scl = ADCvals[0];};//3
			while(scl < 4000){ADC_In89(&ADCvals[0]); scl = ADCvals[0];};
				SDA = t&(0x00000008);///
				t = ToNXT;			
			while(scl > 2300){ADC_In89(&ADCvals[0]); scl = ADCvals[0];};//4
			while(scl < 4000){ADC_In89(&ADCvals[0]); scl = ADCvals[0];};
				SDA = t&(0x00000008);///
				t = ToNXT<<1;			
			while(scl > 2300){ADC_In89(&ADCvals[0]); scl = ADCvals[0];};//5
			while(scl < 4000){ADC_In89(&ADCvals[0]); scl = ADCvals[0];};
				SDA = t&(0x00000008);///
				t = ToNXT<<2;			
			while(scl > 2300){ADC_In89(&ADCvals[0]); scl = ADCvals[0];};//6	
			while(scl < 4000){ADC_In89(&ADCvals[0]); scl = ADCvals[0];};	
				SDA = t&(0x00000008);///
				t = ToNXT<<3;			
			while(scl > 2300){ADC_In89(&ADCvals[0]); scl = ADCvals[0];};//7	
			while(scl < 4000){ADC_In89(&ADCvals[0]); scl = ADCvals[0];};	
				SDA = t&(0x00000008);///
				t = t >> 1;			
			while(scl > 2300){ADC_In89(&ADCvals[0]); scl = ADCvals[0];};//8
					SDA = 0x00;	
			while(scl < 4000){ADC_In89(&ADCvals[0]); scl = ADCvals[0];};			
			while(scl > 2300){ADC_In89(&ADCvals[0]); scl = ADCvals[0];};//9, ACK
			GPIO_PORTB_DIR_R &= ~0x08; //SDA TO INPUT
			while(scl < 4000){ADC_In89(&ADCvals[0]); scl = ADCvals[0];};	
	
			//wait a little while
			for(int k = 0; k < 100; k++){
				int x = 2;
			}
	
  	return FromNXT;
}

/////////////
//I2C Send Receive Message
//
////////////////
int32_t I2C_Message(int32_t ByteToSend){
	while(I2C_SendRecieve(0xF0) != 0x0F){};
	I2C_SendRecieve(ByteToSend);	
	return I2C_SendRecieve(ByteToSend);
	
}

int32_t I2C_MessageSet(int32_t ByteToSend, int32_t ByteToSet){
	while(I2C_SendRecieve(0xF0) != 0x0F){};
	I2C_SendRecieve(ByteToSend);
	I2C_SendRecieve(ByteToSet);
	return I2C_SendRecieve(ByteToSend);
	
}







