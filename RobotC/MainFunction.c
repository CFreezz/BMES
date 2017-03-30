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
//////////////
//Initialization
//reset motor angles to 0
//////////////



///////////////
	while(true){
		i2cScanDeviceMsg[2] = 0x0F;
		I2Cmessage();
		while(rMsg[0] != 0xF0){//wait for acknowledgement signal 0xF0 from tm4c
			I2Cmessage();
		}
		//message begin acknowledged, see what tm4c123 wants
		i2cScanDeviceMsg[2] = 0x21;
		I2Cmessage();
		stringFromChars(msg,rMsg);
		nxtDisplayString(5, "%s", msg);
		code = rMsg[0];
		send = 0;
		//Message is now in code, break into cases
		//ALL CASES SHOULD EDIT THE 'SEND' VARIABLE, TM4 WILL BE SENT
		// A REPLY, EVEN IF THERES NOTHING MEANINGFUL TO SEND
		// 'A' : Send Motor A Position
		if(code == 'A'){

		send = 0x01;
		}
		// 'B' : Send Motor B Position
		if(code == 'B'){

		send = 0x01;
		}
		// 'C' : Send Motor C Position
		if(code == 'C'){

		send = 0x01;
		}
		// 'D' : Motor A Go Forward
		if(code == 'D'){

		send = 0x01;
		}
		// 'E' : Motor B Go Forward
		if(code == 'E'){

		send = 0x01;
		}
		// 'F' : Motor C Go Forward
		if(code == 'F'){

		send = 0x01;
		}
		// 'G' : Motor A Go Backward
		if(code == 'G'){

		send = 0x01;
		}
		// 'H' : Motor B Go Forward
		if(code == 'H'){

		send = 0x01;
		}
		// 'I' : Motor C Go Backward
		if(code == 'I'){

		send = 0x01;
		}
		// 'J' : Motor A Stop
		if(code == 'J'){

		send = 0x10;
		}
		// 'K' : Motor B Stop
		if(code == 'K'){

		send = 0x01;
		}
		// 'L' : Motor C Stop
		if(code == 'L'){

		send = 0x01;
		}
////////////
//Set is special; if M,N, or O is sent there is a follow up I2C
//message sent from the TM4C that says what position to set.
//all positions are positive.
//make sure physically motors are set to 0
// motors a and c are on a gear track, and can be damaged if they go too far.
//	TODO: find the max position they can be set after 0
////////////
		// 'M' : Set Motor A Position
		if(code == 'M'){
		ubyte pos	=	I2Cmessage();


		send = 0x01;
		}
		// 'N' : Set Motor A Position
		if(code == 'N'){
		ubyte pos	=	I2Cmessage();

		send = 0x01;
		}
		// 'O' : Set Motor A Position
		if(code == 'O'){
		ubyte pos	=	I2Cmessage();

		send = 0x01;
		}
//////////////////////////////
		// 'S' : Return Color Sensor Values for Red
		// essentially decide if the RGB value is red enough and send a yes or no
		if(code == 'S'){

		send = 0x01;
		}
		// 'U' : Return UltraSonic Sensor value
		if(code == 'U'){

		send = 0x01;
		}
/////////////////////////////////
//return to the tm4 the 'send' variable
// this acknowledges message completion and sends any
// requested data
		i2cScanDeviceMsg[2] = send;
		I2Cmessage();

//loop
		wait1Msec(100);

	}

}
