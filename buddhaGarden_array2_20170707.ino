// Libraries included
#include <SPI.h>
#include <SD.h>
#include <OneWire.h> 
#include <DallasTemperature.h>
#include <SoftwareSerial.h> // For RS485 comms

/////////////////////////////////////////////////////////////////
// Settings

//// SD CARD SETTINGS
//// SD card chip select pin on Arduino
//const int chipSelect = 4;

// HUMIDITY SENSORS SETTINGS
// Analog pin numbers to which humidity sensors are connected
const int humiditySensorPins[3][3] = {{0,1,2},{3,4,5},{6,7,8}}; // Row 1 - control bed 1, Row 2 - test bed, Row 3 - control bed 2
// Declare and initialize global variable that will contain soil humidity values
int soilHumidityValues[3][3] = {{0}};// Easier to manipulate global array from function

///////////////
//// TEMPERATURE SENSORS SETTINGS
//// Pin to which all oneWire sensor data wires are connected
//const int oneWirePin = 5;
//// Setup a oneWire instance to communicate with any OneWire devices  
//// (not just Maxim/Dallas temperature ICs) 
//OneWire oneWire(oneWirePin);
//// Pass our oneWire reference to Dallas Temperature. 
//DallasTemperature oneWireSensors(&oneWire);
///////////////

// SOLENOID VALVE SETTINGS
// Initialize solenoid valve status to closed
int valveStatus = LOW;
//Pin to which DC-AC relay energizing solenoid valve is connected
const int solenoidRelay = 7;

// RS485 SETTINGS
#define SSerialRX        11  //Serial Receive pin
#define SSerialTX        12  //Serial Transmit pin
#define SSerialTxControl1 8   //RS485 Direction control RE/DE
#define SSerialTxControl2 9   //RS485 Direction control RE/DE
#define RS485Transmit    HIGH
#define RS485Receive     LOW
#define Pin13LED         13
//String byteReceived;
String receivedCmd;
// Create software serial object for RS485 comms
SoftwareSerial RS485Serial(SSerialRX, SSerialTX); // RX, TX
/////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////
//// Funtion that writes to SD card an input string
//void sdWrite(String writeString){
//  
//  while(1){ 
//  // open the file. note that only one file can be open at a time,
//  // so you have to close this one before opening another.
//  // Limit on file name length = 8 characters
//    File dataFile = SD.open("snsrData.txt", FILE_WRITE);
//  // if the file is available, write to it:
//  if (dataFile) {
//    dataFile.println(writeString);
//    dataFile.close();
//    // print to the serial port too:
//    //Serial.println(writeString);
//    break;
//  }
//  // if the file isn't open, pop up an error:
//  // Arduino tries to open SD card again and agin in loop.
//  else {
//    //Serial.println("Error opening snsrData.txt. Check connections and reset Arduino.");
//  }
//
//  }
//  
//}
/////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////
// Read humidity (or any analog sensor) at given analog pin
int readHumiditySensors(int analogPins[3][3]){
  // Declare variable that will contain humidity values
  //int humidityValues [3][3];
  // Loop in 2 each row and then to each row-element
  for (int bedNo = 0; bedNo < 3; bedNo++) {
    for (int sensorNo = 0; sensorNo < 3; sensorNo++){
       soilHumidityValues[bedNo][sensorNo] = analogRead(analogPins[bedNo][sensorNo]);
      }
    }
  
  //return humidityValues;
}
/////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////
//// Read temperature from nth 1-wire sensor
//int readTemperatureSensor(int n){
//  // read 1-wire temperature sensor
//  oneWireSensors.requestTemperatures(); // Send the command to get temperature readings
//  int currentTemperature = oneWireSensors.getTempCByIndex(n); // Why "byIndex"?  
//  // You can have more than one DS18B20 on the same bus.  
//  // 0 refers to the first IC on the wire
//  return currentTemperature;
//}
/////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////
/*
  SerialEvent occurs whenever a new data comes in the
 hardware serial RX.  This routine is run between each
 time loop() runs, so using delay inside loop can delay
 response.  Multiple bytes of data may be available.
 */
//void serialEvent() {
//  while (Serial.available()) {
//    // get the new byte:
//    String inCmd = Serial.readString();
//    //delay(50);
//    Serial.print("Command entered - ");
//    Serial.println(inCmd);
//    // add it to the inputString:
//    if(inCmd == "111"){
//      Serial.println("Command translation - Open valve");
//      valveStatus = HIGH;
//      digitalWrite(solenoidRelay, valveStatus);
//      //digitalWrite(acLed, valveStatus);
//    }
//    else{
//      Serial.println("Command translation - Close valve");
//      valveStatus = LOW;
//      digitalWrite(solenoidRelay, valveStatus);
//      //digitalWrite(acLed, valveStatus);
//    }
//    }
//  }
/////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////
void setup() {
  // Open serial communications and wait for port to open:
  Serial.begin(9600);
  delay(2000);

  //Serial.println("SerialRemote");  // Can be ignored
  
  //Serial.print("Initializing SD card...");

//  // see if the card is present and can be initialized:
//  if (!SD.begin(chipSelect)) {
//    //Serial.println("Card failed, or not present");
//    // don't do anything more:
//    return;
//  }
//  //Serial.println("SD card initialized.");

  // Writing first line of new write to indicate breaks in writing.
  // Temperature, hum1, hum2,....., hum9, valve status

  // Start up 1-wire library 
  //oneWireSensors.begin(); 

  //Serial.println("Enter '1' to open solenoid valve and anything else to close: ");

  // Initialize pin connected to solenoid relay as output
  pinMode(solenoidRelay, OUTPUT);

  // RS485 SETUP
  // initialize digital pin LED_BUILTIN as an output.
  pinMode(Pin13LED, OUTPUT);
  pinMode(SSerialTxControl1, OUTPUT);
  pinMode(SSerialTxControl2, OUTPUT);  
  digitalWrite(SSerialTxControl1, RS485Receive);  // Init Transceiver
  digitalWrite(SSerialTxControl2, RS485Receive);  // Init Transceiver
  // Start the software serial port, to another device
  RS485Serial.begin(4800);   // set the data rate 
  
}
/////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////
void loop() {
  // Check if RS485 input is available.
  // If it is, check msg/instruction and perform sensors read
  // action if needed.
  if (RS485Serial.available()) 
  {
    receivedCmd = RS485Serial.readString();   // Read the byte 
    Serial.print("Received command = ");
    Serial.println(receivedCmd);
    
    digitalWrite(Pin13LED, HIGH);  // Show activity
    delay(10);              
    digitalWrite(Pin13LED, LOW);   
  
  //if(receivedCmd.substring(0,1)=="s-2-e")
  if(receivedCmd=="s-2-e")
  {
      // make a string for assembling the data to log:
  String dataString = "Data - array2";

  // read temperature sensor 0 of n temperature sensors
  //int caseAirTemperature = readTemperatureSensor(0);
  //dataString += String(caseAirTemperature)+",";
  
  // read humidity sensors and add to dataString in loop
  readHumiditySensors(humiditySensorPins);
  for(int bedNo = 0; bedNo <3; bedNo++){
    for(int sensorNo = 0; sensorNo < 3; sensorNo++){
      dataString += "," + String(soilHumidityValues[bedNo][sensorNo]);
    }
  }

  
//  // Add valve status to data string
//  dataString += String(valveStatus);
  
  // Write observation to file
  //sdWrite(dataString);
  // Send observation to serial monitor
  Serial.println(dataString);
  // Send observation to RS485 bus
  digitalWrite(SSerialTxControl1, RS485Transmit);  // Enable RS485 Transmit    
    digitalWrite(SSerialTxControl2, RS485Transmit);  // Enable RS485 Transmit    
    RS485Serial.print(dataString); // Send data packet
    delay(100);   
    digitalWrite(SSerialTxControl1, RS485Receive);  // Disable RS485 Transmit      
    digitalWrite(SSerialTxControl2, RS485Receive);  // Disable RS485 Transmit      
  
  
  // Signal successful reading
  digitalWrite(Pin13LED, HIGH);   // turn the LED on 
  delay(1000);
  digitalWrite(Pin13LED, LOW);    // turn the LED off 
  }
  

  //delay(1000);
  //delay(900000);
  
}
}
/////////////////////////////////////////////////////////////////
