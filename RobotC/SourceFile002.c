#pragma config(Sensor, S1,TIR,sensorI2CCustom)
/*

*/

// First, define the Arduino Address
// Address is 0x0A on the Arduino: (Binary) 1010
// Bit shifted out with one 0, that becomes: (Binary) 10100
// Which is 0x14
#define ARDUINO_ADDRESS 0x04
#define ARDUINO_PORT S1

ubyte I2Cmessage[22];
ubyte I2Creply[20];

void i2c_read_registers_text(ubyte register_2_read, int message_size, int return_size){
memset(I2Creply, 0, sizeof(I2Creply));

I2Cmessage[0] = message_size; // Messsage Size
I2Cmessage[1] = ARDUINO_ADDRESS;
I2Cmessage[2] = register_2_read; // Register
sendI2CMsg(S1, &I2Cmessage[0], return_size);
wait1Msec(20);

readI2CReply(ARDUINO_PORT, &I2Creply[0], return_size);
string returned_string;
ubyte *I2Creply_pointer[20];
memcpy(I2Creply_pointer, I2Creply, sizeof(I2Creply));
stringFromChars(returned_string,(char*) I2Creply_pointer);
writeDebugStreamLine(returned_string);
writeDebugStreamLine(" ");
}

void i2c_write_registers(ubyte register_2_write, int message_size, int return_size, ubyte byte1, ubyte byte2, ubyte byte3, ubyte byte4){
memset(I2Creply, 0, sizeof(I2Creply));

message_size = message_size+3;

I2Cmessage[0] = message_size; // Messsage Size
I2Cmessage[1] = ARDUINO_ADDRESS;
I2Cmessage[2] = register_2_write; // Register

I2Cmessage[3] = byte1;
I2Cmessage[4] = byte2;
I2Cmessage[5] = byte3;
I2Cmessage[6] = byte4;

sendI2CMsg(ARDUINO_PORT, &I2Cmessage[0], return_size);
wait1Msec(20);
}

/*

*/
#define i2cScanPort  S1
char i2cScanDeviceMsg[6];
ubyte replyMsg[10];                  // reply Message byte Array of size 10
TI2CStatus nStatus;                 // status variable
task main()
{

nI2CRetries = 0;
while(true){


//i2c_write_registers(0x01, 0x01, 0x00, 0x0A, 0, 0, 0);
//i2c_read_registers_text(0x02, 0x01, 0x01);
i2cScanDeviceMsg[0] = 0x02;
i2cScanDeviceMsg[1] = 0x04;
i2cScanDeviceMsg[2] = 0xFF;
i2cScanDeviceMsg[3] = 0x42;
i2cScanDeviceMsg[4] = 0xAA;
i2cScanDeviceMsg[5] = 0xBB;
sendI2CMsg(i2cScanPort, i2cScanDeviceMsg, 1);  // send a message from 'i2cScanDeviceMsg[0]' to
      wait1Msec(20);                                            //'i2cScanPort' (S1), expecting an 8 byte return message
readI2CReply(i2cScanPort,&replyMsg[0], 1);
string msg = "";
replyMsg[9] = 0;
char rMsg[2];
rMsg[0] = replyMsg[0];
rMsg[1] = (char)0;
stringFromChars(msg, rMsg);
nxtDisplayString(5, "%s", msg);
wait1Msec(100);
}

}
