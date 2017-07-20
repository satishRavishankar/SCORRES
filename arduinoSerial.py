# Import relevant libraries
import serial # For serial communication
import serial.tools.list_ports # For listing all active comports
import csv # To log data
from time import localtime, strftime # To get current local datetime
from os.path import isfile # To check if a file exists
from apscheduler.scheduler import Scheduler # To schedule RS485 requests

################################################################
## Finding and connecting to arduino on serial port

# Declare and intialize variables
arduinoPort = "notFound" # Intitialization of variable that will contain Arduino port name
lenPorts = 0 # Intialize variable that will contain length of ports

def arduinoSerialConnect():
    while True:
        ports = list(serial.tools.list_ports.comports()) # List active/used comports
        lenPorts = len(ports) # Get length of ports. If Arduino is detected, ports will have its details in element [1]
        if lenPorts < 2: # If length of ports list is less than 2 then arduino not detected
            print("Arduino not connected. Waiting...")
        else:
            break # Else if length is 2 or more, arduino may have been detected

    possiblePortData = "".join(ports[1]) # ports is a list of tuples. the 2nd tuple has com port of arduino.

    # Getting port no.
    if "ttyACM" in possiblePortData: # Searching through string
            print("Arduino connected at -")
            # Constructing full address of Arduino 
            arduinoPort = "/dev/ttyACM" + possiblePortData[possiblePortData.find("ttyACM") + len("ttyACM")]
            print(arduinoPort) # print address
    else: # else arduino not detected even in element [1] of ports list. Display error and terminate program.
        print("Error. Connect Arduino. Terminating program.") 
        exit()


    ser = serial.Serial(arduinoPort) # Open serial port for communication with arduino
    ser.flushInput()
    
    return arduinoPort, ser
################################################################

################################################################
# Reading individual lines and returning them
# Once Arduino has reset and RPi has started getting data packets, print the incoming data
def arduinoSerialRead(ser):
    inflow = ser.readline()
    #print("bytes format - ",inflow) # print bytes data (raw)
    try:
        inflow = inflow[:-2].decode('UTF-8') # Incoming data is in bytes format. Decode this to ASCII.
    except:
        inflow = "9,9,9,9,9,9,9,9,9,9,9,9"
    #print("ASCII format - ",inflow) # print ASCII data
    return inflow

################################################################

################################################################
# Sending serial data to Arduino
def arduinoSerialWrite(ser, message):
    ser.write(message.encode())
################################################################

################################################################
def logArrayData2CSV(filename, dataString, arr_no):
##    arr1_bed1_trialID = '110101'
##    arr1_bed2_trialID = '121102'
##    arr1_bed3_trialID = '130101'
##    arr2_bed1_trialID = '240002'
##    arr2_bed2_trialID = '251001'
##    arr2_bed3_trialID = '260002'
##    arr3_bed1_trialID = '370211'
##    arr3_bed2_trialID = '381212'
##    arr3_bed3_trialID = '390211'
    #Indexing - TrialIDs[arr_no -1][bed_no -1] (zero indexing)
    trialIDs = [['110101','121102','130101'],['240002','251001','260002'],['370211','381212','390211']]
    #folderName = 'data/'
    #filename = folderName + 'data_' + strftime("%Y%m%d_%H%M", localtime()) + '.csv'
    #print(filename)
    if isfile(filename):
        #print('if')
        currentCSVFile = open(filename,'a') # If it exists open to append
    else:
        #print('else')
        currentCSVFile = open(filename,'w') # If file doesn't exist, open to write line
        # Write header line to new file
        currentCSVFile.write('timestamp,array_no,bed1_trialID,h11,h12,h13,bed2_trialID,h21,h22,h23,bed3_trialID,h31,h32,h33\n')
    # Construct data string to be written to file
    dataString = strftime('%Y-%m-%d %H:%M:%S',localtime()) + ',' + str(arr_no) + ',' + trialIDs[arr_no -1][0] + ',' + dataString[2:14] + trialIDs[arr_no -1][1] + ',' + dataString[14:26] + trialIDs[arr_no -1][2] + ',' + dataString[26:]
    # Write data string to file and close it
    currentCSVFile.write(str(dataString) + '\n')
    currentCSVFile.close()

################################################################

################################################################
def logTankAreaData2CSV(filename, dataString):
    #folderName = 'data/'
    #filename = folderName + 'data_' + strftime("%Y%m%d_%H%M", localtime()) + '.csv'
    #print(filename)
    if isfile(filename):
        #print('if')
        currentCSVFile = open(filename,'a') # If it exists open to append
    else:
        #print('else')
        currentCSVFile = open(filename,'w') # If file doesn't exist, open to write line
        # Write header line to new file
        currentCSVFile.write('timestamp,solenoidValve1_status,solenoidValve2_status,solenoidValve3_status,floatSensor1,floatSensor2,waterFlow\n')
    # Construct data string to be written to file
    dataString = strftime('%Y-%m-%d %H:%M:%S',localtime()) + ',' + dataString
    # Write data string to file and close it
    currentCSVFile.write(str(dataString) + '\n')
    currentCSVFile.close()

################################################################


################################################################
# Functions that send command packet over to Master Arduino
# One function for each array (or box)
def dataRequest_array1():
    arduinoSerialWrite(serialPort, "s-1-e") # "s-": start of cmd, "-e": end of cmd

def dataRequest_array2():
    arduinoSerialWrite(serialPort, "s-2-e") # "s-": start of cmd, "-e": end of cmd

def dataRequest_array3():
    arduinoSerialWrite(serialPort, "s-3-e") # "s-": start of cmd, "-e": end of cmd

# Function that sends solenoid valve commands and requests status update on valves and tank float sensors
valveCommands = "000" # Intializing global variable that will be passed to below function with values such that all valves are closed
def commands_dataRequest_tankArea(solenoidValveCommands):
    # Construct packet to be sent
    commandPacket = "s-vt-" + solenoidValveCommands + "-e"
    arduinoSerialWrite(serialPort, commandPacket) # Send packet to rs485 bus
################################################################

################################################################
# Test running functions
null, serialPort = arduinoSerialConnect()

# Scheduling these data request functions to be executed at specific
# interval using cron-like scheduling
rs485Schedule = Scheduler() # Define a scheduler
rs485Schedule.start() # Start the defined scheduler

# Keep only one of the debug cron-like schedules or all of the real schedules
# Array 2 ###
# Schedule for debugging - one data packet sent every 5 seconds
#rs485Schedule.add_cron_job(dataRequest_array2, second='*/10')
# Real schedule - one data packet sent every 10 minutes
rs485Schedule.add_cron_job(dataRequest_array2, minute='*/10')
# Array 3 ###
# Schedule for debugging - one data packet sent every 5 seconds
#rs485Schedule.add_cron_job(dataRequest_array3, second='*/10')
# Real schedule - one data packet sent every 10 minutes on the 10th second
rs485Schedule.add_cron_job(dataRequest_array3, minute='*/10', second=10)
# Array 1 ###
# Schedule for debugging - one data packet sent every 5 seconds
#rs485Schedule.add_cron_job(dataRequest_array1, second='*/10')
# Real schedule - one data packet sent every 10 minutes on the 20th second
rs485Schedule.add_cron_job(dataRequest_array1, minute='*/10', second=20)
# Tank Area ###
# Schedule for debugging - one data packet sent every 5 seconds
#rs485Schedule.add_cron_job(lambda: commands_dataRequest_tankArea(valveCommands), second=10)
# Real schedule - one data packet sent every 1 minute on the 30th second
rs485Schedule.add_cron_job(lambda: commands_dataRequest_tankArea(valveCommands), minute='*/1', second=30) #lambda defines a function that function can be called later

while 1:
    #count+=1
    inflowPacket = arduinoSerialRead(serialPort) # Read incoming rs485 packet
    print(inflowPacket)
    #print("first 7 chars - " + inflowPacket[:7])
    # Check if the read packet is from one of the arrays
    if inflowPacket[:12] == 'Data - array':
        #print(inflowPacket)
        # Construct complete file path to which data is to be written and write packet as row to CSV file
        folderName = 'data/'
        filename = folderName + 'data_' + strftime("%Y%m%d_%H%M", localtime()) + '.csv'
        currentArray = int(inflowPacket[12]) # Array number from which packet has come in
        logArrayData2CSV(filename, inflowPacket[12:], currentArray)
    # Else check if read packet is from the tank area
    elif inflowPacket[:12] == 'Val & tnk - ':
        # Construct complete file path to which tank area data is to be written and write packet as row to CSV file
        folderName = 'tankAreaData/'
        filename = folderName + 'data_' + strftime("%Y%m%d_%H%M", localtime()) + '.csv'
        logTankAreaData2CSV(filename, inflowPacket[12:])
        
    # Currently solenoid valve statuses are set based on commands in solenoidValveCommand.txt
    solenoidValveCommandSource = open("solenoidValveCommand.txt") # Open the text file
    valveCommandsRead = solenoidValveCommandSource.read() # Read the contents of the file
    # Check file contents for commands for the 3 valves and create valve-x-Command substring accordingly
    if "valve 1 - open" in valveCommandsRead:
        valve1Command = "1"
    else:
        valve1Command = "0"

    if "valve 2 - open" in valveCommandsRead:
        valve2Command = "1"
    else:
        valve2Command = "0"

    if "valve 3 - open" in valveCommandsRead:
        valve3Command = "1"
    else:
        valve3Command = "0"
    # Put all valve-x-command substrings together to construct the valve commands string
    # This string will be padded and sent to the rs485 bus by the cron-like job set up to run every minute
    valveCommands = valve1Command + valve2Command + valve3Command
    #print('outflowPacket = ' + outFlowPacket + '\n')
    #commands_dataRequest_tankArea(valveCommands)
################################################################
