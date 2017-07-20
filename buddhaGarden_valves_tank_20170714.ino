// Citation:
// Liquid flow rate sensing code - DIYhacking.com Arvind Sanjeev

// Libraries included
#include <SoftwareSerial.h> // For RS485 comms

// SOLENOID VALVE SETTINGS
// Initialize solenoid valve statuses to closed
int valve1status = LOW;
int valve2status = LOW;
int valve3status = LOW;
//Pin to which DC-AC relays energizing solenoid valves are connected
#define solenoidRelay_array1 5
#define solenoidRelay_array2 6
#define solenoidRelay_array3 7

// TANK FLOAT LEVEL SENSOR SETTINGS
#define floatSensor_top 3
#define floatSensor_bottom 4

// FLOW SENSOR SETTINGS
#define flowSensor 2
float flowSensorCalibrationFactor; // Flow sensor output 0.2 pulses/L
//unsigned long oldTime; // Will store time when flow rate was calculated
//float minutesSinceLastRead;
volatile unsigned long pulseCount;  // Holds pulse count and in incremented by ISR
//double pulseFrequency; // Holds value for pulses per second from flow sensor
//double flow_litresPerMinute;
double flow_litresSinceLastRead;

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

  // Setup pins connected to solenoid relays as output
  pinMode(solenoidRelay_array1, OUTPUT);
  pinMode(solenoidRelay_array2, OUTPUT);
  pinMode(solenoidRelay_array3, OUTPUT);
  // Intialize pins as high since relay is active low
  digitalWrite(solenoidRelay_array1, HIGH);
  digitalWrite(solenoidRelay_array2, HIGH);
  digitalWrite(solenoidRelay_array3, HIGH);

  // Flow sensor setup
  pinMode(flowSensor, INPUT); // Intitialize pin connected to flow sensor output as input
  digitalWrite(flowSensor, HIGH); // Turn on pullup resistors to set pin to known value in absence of input
                                  // Falling edges are counted so setting it to high
  // Intialize relevant variables
  flowSensorCalibrationFactor = 0.2; // Flow sensor output 0.2 pulses/L
//  oldTime = 0; // Will store time when flow rate was calculated
//  minutesSinceLastRead = 0;
  pulseCount = 0;  // Holds pulse count and in incremented by ISR
//  pulseFrequency = 0; // Holds value for pulses per second from flow sensor
//  flow_litresPerMinute = 0;
  flow_litresSinceLastRead = 0;
  // Attach interrupt to pin that is connected to flow sensor output so pulses can be counted using an ISR
  attachInterrupt(digitalPinToInterrupt(flowSensor), pulseCounter, FALLING);

  // Float sensors setup
  // Initialize pins connected to float sensors as input
  pinMode(floatSensor_top, INPUT);
  pinMode(floatSensor_bottom, INPUT);
  digitalWrite(floatSensor_top, HIGH); // Pull up
  digitalWrite(floatSensor_bottom, HIGH);  

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
  
  if(receivedCmd.substring(0,5)=="s-vt-" && receivedCmd.substring(8,10)=="-e")
  //if(receivedCmd=="s-vt-e")
  {
  // Parse commands to solenoid valves
  valve1status = String(receivedCmd.charAt(5)).toInt();
  valve2status = String(receivedCmd.charAt(6)).toInt();
  valve3status = String(receivedCmd.charAt(7)).toInt();
  // Open/close valve based on current command
  digitalWrite(solenoidRelay_array1, !bool(valve1status)); // !(not) used because relays are active low
  digitalWrite(solenoidRelay_array2, !bool(valve2status));
  digitalWrite(solenoidRelay_array3, !bool(valve3status));
    
  // make a string for assembling the data (statuses) to log:
  // Data string format: Val & tnk - valve1status,valve2status,valve3status,floatSensor1,floatSensor2,waterFlow
  String dataString = "Val & tnk - ";
  // Adding valve statuses
  dataString += String(valve1status) + "," + String(valve2status) + "," + String(valve3status);
  // Adding float sensor statuses
  dataString += "," + String(!digitalRead(floatSensor_top)) + "," + String(!digitalRead(floatSensor_bottom)); // ! (NOT) is used because float sensor pins are pulled up and short to ground when float sensors are activated

  /////////////// Flow sensor block alternate
  // Measure flow rate and volume of water
  // Formula: Pulse frequency, f = (0.2*Q)   Q=L/Min
  detachInterrupt(digitalPinToInterrupt(flowSensor)); // Detach interrupt while calculating flow rate
//  pulseFrequency = pulseCount/((millis() - oldTime)/1000.0);
//  flow_litresPerMinute = pulseFrequency/flowSensorCalibrationFactor; // Q = f/0.2
//  minutesSinceLastRead = (millis() - oldTime)/(1000.0*60.0);
//  flow_litresSinceLastRead = flow_litresPerMinute*minutesSinceLastRead;
//  oldTime = millis();
  flow_litresSinceLastRead = pulseCount/(flowSensorCalibrationFactor*60); // Simplified from method above
  pulseCount = 0; // Reset pulse count
  attachInterrupt(digitalPinToInterrupt(flowSensor), pulseCounter, FALLING); // re-attach interrupt
  dataString += "," + String(flow_litresSinceLastRead);
  ///////////////


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

/////////////////////////////////////////////////////////////////
// Insterrupt Service Routine that counts pulses from flow meter
void pulseCounter()
{
  // Increment the pulse counter
  pulseCount++;
}
/////////////////////////////////////////////////////////////////
