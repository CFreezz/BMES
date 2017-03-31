//I2C.h
//
//
//	PE4 -  ADC  - SCL Recieve
//	PE5 -  ADC  - SDA Receive
//  PB3 - GPIO  - SDA Send
//
//////////////////////////////
// I2C_Init
// initializes the ADC and sda pin
// Input: none
// Output: none
////////////////////////////////////////
//change to the correct SDA send pin, there is no SCL Send pin
void I2C_Init(void);
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
//>>THIS FUNCTION BLOCKS UNTIL NXT SENDS AN I2C POLL<<
// if the NXT polls the I2C lines and this function is not running it will get a NULL (0)
// proper usage: NXT drives the 3 motors, constantly polling the I2C line for information from the tm4
// 	the tm4 will send a byte to tell it what it wants its motors to do, and the NXT can send back a byte of information
//
int32_t I2C_SendRecieve(int32_t ByteToSend);
//
//
//
int32_t I2C_Message(int32_t ByteToSend);
int32_t I2C_MessageSet(int32_t ByteToSend, int32_t ByteToSet);