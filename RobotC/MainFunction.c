#pragma config(Sensor, S1,TIR,sensorI2CCustom)
/*

*/

// First, define the Arduino Address
// Address is 0x0A on the Arduino: (Binary) 1010
// Bit shifted out with one 0, that becomes: (Binary) 10100
// Which is 0x14
#define ARDUINO_ADDRESS 0x04
#define ARDUINO_PORT S1
#define i2cScanPort  S1


char i2cScanDeviceMsg[6];
ubyte replyMsg[10];                  // reply Message byte Array of size 10
char rMsg[2];
string msg = "";

ubyte I2Cmessage(void){
	sendI2CMsg(i2cScanPort, i2cScanDeviceMsg, 1);  // send a message from 'i2cScanDeviceMsg[0]' to
    wait1Msec(20);                                 //'i2cScanPort' (S1), expecting a 1 byte return message
	readI2CReply(i2cScanPort,&replyMsg[0], 1);
	msg = "";
	replyMsg[9] = 0;
	rMsg[0] = replyMsg[0];
	rMsg[1] = (char)0;
	return rMsg[0];
}



task main()
{
	nI2CRetries = 0;
	i2cScanDeviceMsg[0] = 0x02;
	i2cScanDeviceMsg[1] = 0x04;
	int code = 0;
	ubyte send = 0;

/**
 * Initialization
 * reset motor angles to 0
 */



/* * * * * * * * * * * * * * * * */
	while(true){
		i2cScanDeviceMsg[2] = 0x0F;
		I2Cmessage();
		while(rMsg[0] != 0xF0){//wait for acknowledgement signal 0xF0 from tm4c
			I2Cmessage();
		}

		/* message begin acknowledged, see what tm4c123 wants */
		i2cScanDeviceMsg[2] = 0x21;
		I2Cmessage();
		stringFromChars(msg,rMsg);
		nxtDisplayString(5, "%s", msg);
		code = rMsg[0];
		send = 0;

		/*
		 * Message is now in code, break into cases
		 * ALL CASES SHOULD EDIT THE 'SEND' VARIABLE, TM4 WILL BE SENT
		 * A REPLY, EVEN IF THERES NOTHING MEANINGFUL TO SEND
		 */


		/* 'A' : Send Motor A position */
		if(code == 'A'){
			send = nMotorEncoder[motorA];
		}
		/* 'B' : Send Motor B position */
		if(code == 'B'){
			send = nMotorEncoder[motorB];
		}
		/* 'C' : Send Motor C position */
		if(code == 'C'){
			send = nMotorEncoder[motorC];
		}
		/* 'D' : Motor A go forward */
		if(code == 'D'){
			motor[motorA] = 69;
			send = 0x69;
		}
		/* 'E' : Motor B go forward */
		if(code == 'E'){
			motor[motorB] = 69;
			send = 0x69;
		}
		/* 'F' : Motor C go forward */
		if(code == 'F'){
			motor[motorC] = 69;
			send = 0x69;
		}
		/* 'G' : Motor A go backward */
		if(code == 'G'){
			motor[motorA] = -69;
			send = 0x69;
		}
		/* 'H' : Motor B go backward */
		if(code == 'H'){
			motor[motorB] = -69;
			send = 0x69;
		}
		/* 'I' : Motor C go backward */
		if(code == 'I'){
			motor[motorC] = -69;
			send = 0x69;
		}
		/* 'J' : Motor A stop */
		if(code == 'J'){
			motor[motorA] = 0;
			send = 0x00;
		}
		/* 'K' : Motor B stop */
		if(code == 'K'){
			motor[motorB] = 0;
			send = 0x00;
		}
		/* 'L' : Motor C stop */
		if(code == 'L'){
			motor[motorC] = 0;
			send = 0x00;
		}

/**
 * Set is special; if M,N, or O is sent there is a follow up I2C
 * message sent from the TM4C that says what position to set.
 * all positions are positive.
 * make sure physically motors are set to 0
 * motors a and c are on a gear track, and can be damaged if they go too far.
 * TODO: find the max position they can be set after 0
 */
		/* 'M' : Set Motor A position */
		if(code == 'M'){
			ubyte pos = I2Cmessage();
			while (nMotorEncoder[motorA] < pos){
				motor[motorA] = 69;
			}
			send = 0x69;
		}
		/* 'N' : Set Motor B position */
		if(code == 'N'){
			ubyte pos = I2Cmessage();
			while (nMotorEncoder[motorB] < pos){
				motor[motorB] = 69;
			}
			motor[motorB] = 0;
			send = 0x69;
		}
		/* 'O' : Set Motor C position */
		if(code == 'O'){
			ubyte pos = I2Cmessage();
			while (nMotorEncoder[motorC] < pos){
				motor[motorC] = 69;
			}
			send = 0x69;
		}
/* * * * * * * * * * * * * * * * */
		/*
		 * 'S' : Return color sensor values for red
		 * essentially decide if the RGB value is red enough and send a yes or no
		 */
		if(code == 'S'){
			send = 0x01;
		}
		/* 'U' : Return UltraSonic sensor value */
		if(code == 'U'){
			send = 0x01;
		}
/* * * * * * * * * * * * * * * * * */
/**
 * Return to the tm4 the 'send' variable
 * this acknowledges message completion and sends any
 * requested data
 */
		i2cScanDeviceMsg[2] = send;
		I2Cmessage();
		wait1Msec(100);

	}

}
