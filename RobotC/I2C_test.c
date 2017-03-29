#pragma config(Sensor, S1,TIR,sensorI2CCustom)
// First, define the Arduino Address
// Address is 0x0A on the Arduino: (Binary) 1010
// Bit shifted out with one 0, that becomes: (Binary) 10100
// Which is 0x14
#define i2cScanPort  S1
char i2cScanDeviceMsg[6];
ubyte replyMsg[10];                  // reply Message byte Array of size 10
TI2CStatus nStatus;                 // status variable
task main(){

	nI2CRetries = 0;
	while(true){

		/* Prepare a message to be sent NXT -> TI TM4C123 */
		i2cScanDeviceMsg[0] = 0x02;		// Expect 2 messages
		i2cScanDeviceMsg[1] = 0x04;		// Accept communication
		i2cScanDeviceMsg[2] = 0xFF;		// Actual message
		i2cScanDeviceMsg[3] = 0x42;
		i2cScanDeviceMsg[4] = 0xAA;
		i2cScanDeviceMsg[5] = 0xBB;


		/* Sends a message from 'i2cScanDeviceMsg[0]' to 'i2cScanPort' (S1) */
		sendI2CMsg(i2cScanPort, i2cScanDeviceMsg, 1);
		wait1Msec(20);

		/* Reads expected reply from the TM4C123 */
		readI2CReply(i2cScanPort,&replyMsg[0], 1);
		string msg = "";
		replyMsg[9] = 0;
		char rMsg[2];
		rMsg[0] = replyMsg[0];
		rMsg[1] = (char)0;

		/* Outputs recieved message to NXT screen */
		stringFromChars(msg, rMsg);
		nxtDisplayString(5, "%s", msg);
		wait1Msec(100);
	}

}
