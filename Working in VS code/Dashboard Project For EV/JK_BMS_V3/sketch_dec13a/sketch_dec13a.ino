#include <SoftwareSerial.h>
#include <LiquidCrystal.h>
#include "SoftwareSerialTX.h"

#define BATTERY_MONITOR_TX_PIN 4
#define buzzerpin 9

// LCD setup
const int lcdRs = 13, lcdEn = 12, lcdD4 = 5, lcdD5 = 10, lcdD6 = 3, lcdD7 = 2;
LiquidCrystal display(lcdRs, lcdEn, lcdD4, lcdD5, lcdD6, lcdD7);

// Define software serial object
SoftwareSerialTX batteryMonitorSerial(BATTERY_MONITOR_TX_PIN);

// Constants with descriptive names
#define REQUEST_INTERVAL_MILLIS      1000
#define REPLY_TIMEOUT_MILLIS         100
#define FRAME_START_BYTE_0           0x4E
#define FRAME_START_BYTE_1           0x57
#define FRAME_END_BYTE               0x68

#define FRAME_HEADER_LENGTH          11
#define FRAME_TRAILER_LENGTH         9
#define FRAME_INDEX_OF_CELL_INFO_LENGTH  (FRAME_HEADER_LENGTH + 1) // +1 for token 0x79
#define MINIMAL_FRAME_LENGTH         19

#define RECEIVE_OK                   0
#define RECEIVE_FINISHED             1
#define RECEIVE_ERROR                2

// Timing variables
unsigned long previousMillis = 0;
const long interval = 500;            // 0.5-second interval

bool isFrameRequested = false;
uint32_t millisOfLastRequestedBatteryFrame = -REQUEST_INTERVAL_MILLIS;
uint32_t millisOfLastReceivedByte = 0;

// Variables for received frame information
uint16_t replyFrameBufferIndex;
uint16_t replyFrameLength;
uint16_t timeoutFrameCounter = 0;
uint8_t batteryReplyFrameBuffer[350];
uint8_t batteryStatus;

float DistanceRemaining = 0.0f;
float DistanceTravel = 0.0f;
float totalVoltage = 0.0f;
float current = 0.0f;
float power = 0.0f;
float powerkilo = 0.0f;

// Function prototypes
void initBatteryReplyFrameBuffer();
void printBatteryReplyFrameBuffer();
void printReceivedData();
uint8_t readBatteryMonitorStatusFrameByte();
void updateDisplay();
void DistanceTravelAndRemaining();

uint8_t batteryMonitorRequestStatusFrame[21] = { // Define JK-BMS   request status frame
    0x4E, 0x57, 0x00, 0x13,
    0x00, 0x00, 0x00, 0x00,
    0x06, 0x03, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00,
    FRAME_END_BYTE,
    0x00, 0x00, 0x01, 0x29
};

void setup() {
  Serial.begin(115200);
  pinMode(buzzerpin, OUTPUT);

  batteryMonitorSerial.begin(115200);  // Set the baud rate to 115200
  display.begin(16, 2);
}

void printTotalVoltage(uint16_t value) {
  Serial.print(F("Battery Voltage    : "));
  Serial.print(static_cast<float>(value) * 0.01, 2);
  Serial.println(F(" V"));
}

void printCurrent(uint16_t value) {
  float currentInAmps = static_cast<float>(value) * 0.01f;
  Serial.print(F("Current Draw       : "));
  Serial.print(currentInAmps, 2);
  Serial.println(F(" A"));
}

void printPower(float voltage_val, float current_val) {
  power = voltage_val * current_val;
  Serial.print(F("Power              : "));
  Serial.print(power, 2);
  Serial.println(F(" W"));
}

void printBatteryCapacity(uint8_t batteryStatus) {
  Serial.print(F("Remaining Capacity : "));
  Serial.print(static_cast<uint16_t>(batteryStatus));
  Serial.println(F(" %"));
}

void DistanceTravelAndRemaining(){
  DistanceRemaining = batteryStatus * 0.6;
  DistanceTravel = (100 - batteryStatus) *0.6;
  Serial.print(F("Distance Remaining : "));
  Serial.print(DistanceRemaining, 2);
  Serial.println(F(" KM"));
  Serial.print(F("Distance Travel    : "));
  Serial.print(DistanceTravel, 2);
  Serial.println(F(" KM"));
}

void beepbuzzer(uint8_t duration){

//  for(int i=0; i<4; i++){
  digitalWrite(buzzerpin, HIGH);
  delay(duration);
  digitalWrite(buzzerpin, LOW);
  delay(duration+1000);
  //}
}
void extractBatteryInformation() {
  const uint8_t batteryInfoStartToken = 0x85;
  const uint8_t totalVoltageStartToken = 0x83;
  const uint8_t currentStartToken = 0x84;
  const uint8_t temperatureToken = 0x82;
  
  for (uint16_t i = 0; i < replyFrameBufferIndex; ++i) {
    if (batteryReplyFrameBuffer[i] == totalVoltageStartToken) {
      if (i + 2 < replyFrameBufferIndex) {                                                                   // Total Voltage information found
        uint16_t totalVoltageValue = (batteryReplyFrameBuffer[i + 1] << 8) | batteryReplyFrameBuffer[i + 2];//extract 2 bytes value & convert to decimal
        totalVoltage = static_cast<float>(totalVoltageValue) * 0.01;
        printTotalVoltage(totalVoltageValue);
      }
      else {
        Serial.println(F("Total Voltage information not found"));
      }
    }
    else if (batteryReplyFrameBuffer[i] == currentStartToken) {
      if (i + 2 < replyFrameBufferIndex) {                                                            // Current information found
        uint16_t currentData = (batteryReplyFrameBuffer[i + 1] << 8) | batteryReplyFrameBuffer[i + 2];//extract 2 bytes value & convert to decimal
        current = static_cast<float>(currentData) * 0.01f;
        printCurrent(currentData);
      }
      else {
        Serial.println(F("Current Information not found"));
      }
    }
    else if (batteryReplyFrameBuffer[i] == batteryInfoStartToken) {
      if (i + 1 < replyFrameBufferIndex) {                              // Battery information found, extract and convert to decimal
        batteryStatus = batteryReplyFrameBuffer[i + 1];                //one byte value
        printBatteryCapacity(batteryStatus);

        if (batteryStatus < 15.00){
          beepbuzzer(1000);
        }
      }
      else {
        Serial.println(F("Remaining Capacity information not found"));
      }
    }
  }
  printPower(totalVoltage, current);
}

void updateDisplay() {
  display.clear();
  display.setCursor(0, 0);
  display.print(batteryStatus);
  display.print("% ");
  display.print(DistanceRemaining,1);
  display.print("KM ");
  display.print(current, 1);
  display.print("A ");
  display.setCursor(0, 1);
  display.print(totalVoltage, 2);
  display.print("V    ");
  //display.print(DistanceTravel,1);
  //display.print("KM ");
  
  if (power >=1000){
    powerkilo = power / 1000;
    display.print(powerkilo,2);
    display.print("KW");
  }
  else{
    display.print(power, 2);
    display.print("W");
  }
  DistanceRemaining = 0.0f;
  DistanceTravel = 0.0f;
  totalVoltage = 0.0f;
  current = 0.0f;
  power = 0.0f;
  powerkilo = 0.0f;
}

void loop() {
  unsigned long currentMillis = millis();            //time since powered
  beepbuzzer(6000);
          
  // Check if it's time to request a JK-BMS frame
  if (currentMillis - millisOfLastRequestedBatteryFrame >= REQUEST_INTERVAL_MILLIS) {
    millisOfLastRequestedBatteryFrame = currentMillis; // set for next check

    // Flush input buffer and send request to the JK-BMS  
    Serial.flush();                                                  //clear serial stream, to send next data
    Serial.println();
    Serial.print(F("Sending Request Frame To JK-BMS  "));
    batteryMonitorSerial.write(batteryMonitorRequestStatusFrame, sizeof(batteryMonitorRequestStatusFrame));   //Request frame sended

    isFrameRequested = true; // enable check for serial input
    initBatteryReplyFrameBuffer();
    millisOfLastReceivedByte = currentMillis; // initialize reply timeout
  }

  if (isFrameRequested && Serial.available()) {
    if (readBatteryMonitorStatusFrameByte()) {
      // Frame completely received, now process it
      printBatteryReplyFrameBuffer();
      processBatteryMonitorStatusFrame();
      extractBatteryInformation();
      DistanceTravelAndRemaining();
    }
  }
  else if (isFrameRequested && currentMillis - millisOfLastReceivedByte >= REPLY_TIMEOUT_MILLIS) {
    // Here we have requested frame, but serial was not available for a longer time => timeout at receiving
    // If no bytes received before (because of JK-BMS   disconnected), print it only once
    handleFrameReceiveTimeout();
  }

  // Update LCD based on the current state
  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;
    updateDisplay();
  }
}


/*
   100 % -> Tested range physically
   for example 60 Km
   factor = 60 /100 = Kilometer per %
*/

uint8_t readBatteryMonitorStatusFrameByte() {
  uint8_t receivedByte = Serial.read();
  batteryReplyFrameBuffer[replyFrameBufferIndex] = receivedByte;

  switch (replyFrameBufferIndex) {
    case 0:
      if (receivedByte != FRAME_START_BYTE_0) {
        Serial.println(F("Error: Start frame token != 0x4E"));
        return RECEIVE_ERROR;
      }
      break;

    case 1:
      if (receivedByte != FRAME_START_BYTE_1) {
        return RECEIVE_ERROR;
      }
      break;

    case 3:
      replyFrameLength = (batteryReplyFrameBuffer[2] << 8) + receivedByte;
      break;

    default:
      if (replyFrameLength > MINIMAL_FRAME_LENGTH) {
        if (replyFrameBufferIndex == replyFrameLength - 3) {
          if (receivedByte != FRAME_END_BYTE) {
            Serial.print(F("Error: End frame token 0x"));
            Serial.print(receivedByte, HEX);
            Serial.print(F(" at index "));
            Serial.print(replyFrameBufferIndex);
            Serial.print(F(" is != 0x68. replyFrameLength= "));
            Serial.print(replyFrameLength);
            Serial.print(F(" | 0x"));
            Serial.println(replyFrameLength, HEX);
            return RECEIVE_ERROR;
          }
        }
        else if (replyFrameBufferIndex == replyFrameLength + 1) {
          uint16_t computedChecksum = 0;
          for (uint16_t i = 0; i < replyFrameLength - 2; i++) {
            computedChecksum += batteryReplyFrameBuffer[i];
          }
          uint16_t receivedChecksum = (batteryReplyFrameBuffer[replyFrameLength] << 8) + receivedByte;
          if (computedChecksum != receivedChecksum) {
            Serial.print(F("Checksum error, computed checksum=0x"));
            Serial.print(computedChecksum, HEX);
            Serial.print(F(", received checksum=0x"));
            Serial.println(receivedChecksum, HEX);
            return RECEIVE_ERROR;
          }
          else {
            return RECEIVE_FINISHED;
          }
        }
      }
      break;
  }

  replyFrameBufferIndex++;
  return RECEIVE_OK;
}

void processBatteryMonitorStatusFrame() {
  if (replyFrameBufferIndex == 0) {
    Serial.println(F("replyFrameBufferIndex is 0"));
  } else {}
  Serial.println();

  isFrameRequested = false;       // Everything OK, do not try to receive more
  if (timeoutFrameCounter > 0) {  // First frame after timeout
    timeoutFrameCounter = 0;
    Serial.println(F("Successfully receiving first JK-BMS   status frame after communication timeout")); // Switch on LCD display, triggered by successfully receiving first JK-BMS   status frame
  }
}

bool readBatteryMonitorStatusFrame() {
  millisOfLastReceivedByte = millis();
  uint8_t receiveResultCode = readBatteryMonitorStatusFrameByte();
  if (receiveResultCode == RECEIVE_FINISHED) { // All JK-BMS   status frame data received
    Serial.println("");
    Serial.println("Received JK-BMS   Response ...");
    return true;
  } else if (receiveResultCode != RECEIVE_OK) {// Error here
    Serial.print(F("Receive error="));
    Serial.print(receiveResultCode);
    Serial.print(F(" at index"));
    Serial.println(replyFrameBufferIndex);
    isFrameRequested = false; // do not try to receive more
    totalVoltage = 0.0f;
    current = 0.0f;
    power = 0.0f;
    batteryStatus = 0;
  }
  return false;
}

void handleFrameReceiveTimeout() {
  //sDoErrorBeep = true;
  isFrameRequested = false; // Do not try to receive more
  if (replyFrameBufferIndex != 0 || timeoutFrameCounter == 0) {
    /**
        No byte received here - JK-BMS   may be off or disconnected
        Do it only once if we receive 0 bytes
    */
    Serial.println();
    Serial.print(F("Receive timeout at ReplyFrameBufferIndex="));
    Serial.println(replyFrameBufferIndex);
    totalVoltage = 0.0f;
    current = 0.0f;
    power = 0.0f;
    batteryStatus = 0;
    if (replyFrameBufferIndex != 0) {}
  }
  timeoutFrameCounter++;
  if (timeoutFrameCounter == 0) {
    timeoutFrameCounter--; // To avoid overflow, we have an unsigned integer here
  }
}


void printBatteryReplyFrameBuffer() {
  for (uint16_t i = 0; i < (replyFrameBufferIndex + 1); ++i) {
    /*
       Insert newline and address after header (11 bytes), before and after cell data before trailer (9 bytes), and after each 16 bytes
    */
    if (i == FRAME_HEADER_LENGTH || i == FRAME_HEADER_LENGTH + 2
        || i == (uint16_t)(FRAME_INDEX_OF_CELL_INFO_LENGTH + 1 + batteryReplyFrameBuffer[FRAME_INDEX_OF_CELL_INFO_LENGTH])
        || i == ((replyFrameBufferIndex + 1) - FRAME_TRAILER_LENGTH)
        || (i < ((replyFrameBufferIndex + 1) - FRAME_TRAILER_LENGTH) && i % 16 == 0) /* no 16-byte newline in trailer*/
       ) {
      if (i != 0) {
        Serial.println();
      }
      Serial.print(F("0x"));
      if (i < 0x10) {
        Serial.print('0'); // padding with zero
      }
      Serial.print(i, HEX);
      Serial.print(F("  "));
    }

    Serial.print(F("0x"));
    if (batteryReplyFrameBuffer[i] < 0x10) {
      Serial.print('0'); // padding with zero
    }
    Serial.print(batteryReplyFrameBuffer[i], HEX);
    Serial.print(' ');
  }
  Serial.println();
}

void initBatteryReplyFrameBuffer() {
  replyFrameBufferIndex = 0;
}
