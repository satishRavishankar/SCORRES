/* YourDuino SoftwareSerialExample1 (Adapted)
   - Connect to another Arduino running "YD_SoftwareSerialExampleRS485_1Remote"
   - Connect this unit Pins 10, 11, Gnd
   - Pin 3 used for RS485 direction control
   - To other unit Pins 11,10, Gnd  (Cross over)
   - Open Serial Monitor, type in top window. 
   - Should see same characters echoed back from remote Arduino

   Questions: terry@yourduino.com 
*/

/*-----( Import needed libraries )-----*/
#include <SoftwareSerial.h>
/*-----( Declare Constants and Pin Numbers )-----*/
#define SSerialRX        11  //Serial Receive pin
#define SSerialTX        12  //Serial Transmit pin

#define SSerialTxControl1 8   //RS485 Direction control
#define SSerialTxControl2 9   //RS485 Direction control

#define RS485Transmit    HIGH
#define RS485Receive     LOW

#define Pin13LED         13

/*-----( Declare objects )-----*/
SoftwareSerial RS485Serial(SSerialRX, SSerialTX); // RX, TX

/*-----( Declare Variables )-----*/
String byteReceived;
String byteSend;
//char convBuffer[50];

void setup()   /****** SETUP: RUNS ONCE ******/
{
  // Start the built-in serial port, probably to Serial Monitor
  Serial.begin(9600);
  //Serial.println("YourDuino.com SoftwareSerial remote loop example");
  //Serial.println("Use Serial Monitor, type in upper window, ENTER");
  
  pinMode(Pin13LED, OUTPUT);   
  pinMode(SSerialTxControl1, OUTPUT);    
  pinMode(SSerialTxControl2, OUTPUT);    
  
  digitalWrite(SSerialTxControl1, RS485Receive);  // Init Transceiver   
  digitalWrite(SSerialTxControl2, RS485Receive);  // Init Transceiver   
  
  // Start the software serial port, to another device
  RS485Serial.begin(4800);   // set the data rate 

}//--(end setup )---


void loop()   /****** LOOP: RUNS CONSTANTLY ******/
{
  digitalWrite(Pin13LED, HIGH);  // Show activity
  if (Serial.available())
  {
    byteReceived = Serial.readString();
    
    digitalWrite(SSerialTxControl1, RS485Transmit);  // Enable RS485 Transmit   
    digitalWrite(SSerialTxControl2, RS485Transmit);  // Enable RS485 Transmit   
    RS485Serial.print(byteReceived);          // Send byte to Remote Arduino
    
    digitalWrite(Pin13LED, LOW);  // Show activity    
    delay(10);
    digitalWrite(SSerialTxControl1, RS485Receive);  // Disable RS485 Transmit       
    digitalWrite(SSerialTxControl2, RS485Receive);  // Disable RS485 Transmit       
  }
  
  if (RS485Serial.available())  //Look for data from other Arduino
   {
    //Serial.println("RS485 msg rcvd.");
    digitalWrite(Pin13LED, HIGH);  // Show activity
    byteReceived = RS485Serial.readString();    // Read received byte
    Serial.println(byteReceived);        // Show on Serial Monitor
    delay(10);
    digitalWrite(Pin13LED, LOW);  // Show activity   
   }  

}//--(end main loop )---

/*-----( Declare User-written Functions )-----*/

//NONE
//*********( THE END )***********

