#include <SoftwareSerial.h>
#include <LiquidCrystal.h>

// initialize the library by associating any needed LCD interface pin
// with the arduino pin number it is connected to
const int rs = 13, en = 12, d4 = 5, d5 = 10, d6 = 3, d7 = 2;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);


// Define constants with meaningful names
#define REQUEST_INTERVAL_MILLIS      2000
#define REPLY_TIMEOUT_MILLIS         100
#define FRAME_START_BYTE_0           0x4E
#define FRAME_START_BYTE_1           0x57
#define FRAME_END_BYTE               0x68
#define BMS_TX_PIN                   4

#define RECEIVE_OK                   0
#define RECEIVE_FINISHED             1
#define RECEIVE_ERROR                2

#define FRAME_HEADER_LENGTH          11
#define FRAME_TRAILER_LENGTH         9
#define FRAME_INDEX_OF_CELL_INFO_LENGTH  (FRAME_HEADER_LENGTH + 1) // +1 for token 0x79
#define MINIMAL_FRAME_LENGTH         19


// Flag to send static Info only once after reset.
bool isStaticInfoSent = false;

#include "SoftwareSerialTX.h"

// Define SoftwareSerialTX object with meaningful name
SoftwareSerialTX jkBmsSerial(BMS_TX_PIN);

// Flag to indicate if a frame has been recently requested, and other time-related variables
bool isFrameRequested = false;
uint32_t millisOfLastRequestedJKDataFrame = -REQUEST_INTERVAL_MILLIS; // Initial value to start first request immediately
uint32_t millisOfLastReceivedByte = 0;     // For timeout

// Variables to manage received frame information
uint16_t replyFrameBufferIndex;
uint16_t replyFrameLength;                 // Received length of frame
uint8_t jkReplyFrameBuffer[350];

// Counter for BMS frame timeouts (every 2 seconds)
uint16_t timeoutFrameCounter = 0;

// Function prototypes
void initJKReplyFrameBuffer();
void printJKReplyFrameBuffer();
void printReceivedData();
uint8_t readJKBMSStatusFrameByte();

// Define command to read cell voltage
const uint8_t readCellVoltageCommand[] = {0xDD, 0xA5, 0x04, 0x00, 0xFF, 0xFC, 0x77};
struct JKReplyStruct *JKFAllReplyPointer;


// Define JK BMS request status frame with meaningful names
uint8_t jkBmsRequestStatusFrame[21] = { 0x4E, 0x57 /*4E 57 = StartOfFrame*/, 0x00, 0x13 /*0x13 | 19 = LengthOfFrame*/, 0x00, 0x00,
                                        0x00, 0x00/*BMS ID, highest byte is default 00*/, 0x06/*Function 1=Activate, 3=ReadIdentifier, 6=ReadAllData*/,
                                        0x03/*Frame source 0=BMS, 1=Bluetooth, 2=GPRS, 3=PC*/, 0x00 /*TransportType 0=Request, 1=Response, 2=BMSActiveUpload*/,
                                        0x00/*0=ReadAllData or commandToken*/, 0x00, 0x00, 0x00,
                                        0x00/*RecordNumber High byte is random code, low 3 bytes is record number*/, FRAME_END_BYTE/*0x68 = EndIdentifier*/,
                                        0x00, 0x00, 0x01, 0x29 /*Checksum, high 2 bytes for checksum not yet enabled -> 0, low 2 Byte for checksum*/
                                      };

void setup() {
  Serial.begin(115200);
  jkBmsSerial.begin(115200);  // Set the baud rate to 115200
  lcd.begin(16, 2);
}

float totalVoltage = 0.0f;
float current = 0.0f;
float power ;
uint8_t batteryValue;

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

void printPower(float voltage_val , float current_val) {
  // Calculate and print power
  power = voltage_val * current_val;
  Serial.print(F("Power              : "));
  Serial.print(power, 2);
  Serial.println(F(" W"));
}

void printBatteryCapacity(uint8_t batteryValue) {
  Serial.print(F("Remaining Capacity : "));
  Serial.print(static_cast<uint16_t>(batteryValue));
  Serial.println(F(" %"));
}




void extractBatteryInformation() {
  const uint8_t batteryInfoStartToken = 0x85;
  const uint8_t totalVoltageStartToken = 0x83;
  const uint8_t currentStartToken = 0x84;



  for (uint16_t i = 0; i < replyFrameBufferIndex; ++i) {
    if (jkReplyFrameBuffer[i] == totalVoltageStartToken) {
      // Total Voltage information found
      if (i + 2 < replyFrameBufferIndex) {
        uint16_t totalVoltageValue = (jkReplyFrameBuffer[i + 1] << 8) | jkReplyFrameBuffer[i + 2];
        totalVoltage = static_cast<float>(totalVoltageValue) * 0.01;
        printTotalVoltage(totalVoltageValue);
      } else {
        Serial.println(F("Total Voltage information not found"));
      }
    } else if (jkReplyFrameBuffer[i] == currentStartToken) {
      // Current information found
      if (i + 2 < replyFrameBufferIndex) {
        uint16_t currentData = (jkReplyFrameBuffer[i + 1] << 8) | jkReplyFrameBuffer[i + 2];
        current = static_cast<float>(currentData) * 0.01f;
        printCurrent(currentData);
      } else {
        Serial.println(F("Current Information not found"));
      }
    } else if (jkReplyFrameBuffer[i] == batteryInfoStartToken) {
      // Battery information found, extract and convert to decimal
      if (i + 1 < replyFrameBufferIndex) {
        batteryValue = jkReplyFrameBuffer[i + 1];
        printBatteryCapacity(batteryValue);
      } else {
        Serial.println(F("Remaining Capacity information not found"));
      }
    }
  }
  printPower(totalVoltage, current);
}
unsigned long previousMillis = 0;
const long interval = 1500;  // 1 second interval

enum DisplayState {
  VOLTAGE,
  CURRENT,
  POWER,
  CAPACITY
};

DisplayState currentState = VOLTAGE;
void updateLCD(DisplayState state);

void updateLCD(DisplayState state) {
  lcd.clear();

  switch (state) {
    case VOLTAGE:
      lcd.setCursor(0, 0);
      lcd.print("Voltage: ");
      lcd.print(totalVoltage, 2);
      lcd.print(" V");
      break;

    case CURRENT:
      lcd.setCursor(0, 0);
      lcd.print("Current: ");
      lcd.print(current, 2);
      lcd.print(" A");
      break;

    case POWER:
      lcd.setCursor(0, 0);
      lcd.print("Power: ");
      lcd.print(power, 2);
      lcd.print(" W");
      break;

    case CAPACITY:
      lcd.setCursor(0, 0);
      lcd.print("Capacity: ");
      lcd.print(batteryValue);
      lcd.print(" %");
      break;
  }
}

void loop() {
  unsigned long currentMillis = millis();

  // Check if it's time to request a JK data frame
  if (currentMillis - millisOfLastRequestedJKDataFrame >= REQUEST_INTERVAL_MILLIS) {
    millisOfLastRequestedJKDataFrame = currentMillis; // set for next check

    // Flush input buffer and send request to JK-BMS
    Serial.flush();
    Serial.println();
    Serial.print(F("Sending Request Frame To JK-BMS"));
    jkBmsSerial.write(jkBmsRequestStatusFrame, sizeof(jkBmsRequestStatusFrame));

    isFrameRequested = true; // enable check for serial input
    initJKReplyFrameBuffer();
    millisOfLastReceivedByte = currentMillis; // initialize reply timeout
  }

  if (isFrameRequested && Serial.available()) {
    if (readJKBMSStatusFrame()) {
      // Frame completely received, now process it
      processJKBMSStatusFrame();
      extractBatteryInformation();
    }
  } else if (isFrameRequested && currentMillis - millisOfLastReceivedByte >= REPLY_TIMEOUT_MILLIS) {
    // Here we have requested frame, but serial was not available for a longer time => timeout at receiving
    // If no bytes received before (because of BMS disconnected), print it only once
    handleFrameReceiveTimeout();
  }

  // Update LCD based on the current state
  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;
    updateLCD(currentState);

    // Switch to the next state
    currentState = static_cast<DisplayState>((currentState + 1) % 4);
  }
}




uint8_t readJKBMSStatusFrameByte() {
  uint8_t receivedByte = Serial.read();
  jkReplyFrameBuffer[replyFrameBufferIndex] = receivedByte;

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
      replyFrameLength = (jkReplyFrameBuffer[2] << 8) + receivedByte;
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
        } else if (replyFrameBufferIndex == replyFrameLength + 1) {
          uint16_t computedChecksum = 0;
          for (uint16_t i = 0; i < replyFrameLength - 2; i++) {
            computedChecksum += jkReplyFrameBuffer[i];
          }
          uint16_t receivedChecksum = (jkReplyFrameBuffer[replyFrameLength] << 8) + receivedByte;
          if (computedChecksum != receivedChecksum) {
            Serial.print(F("Checksum error, computed checksum=0x"));
            Serial.print(computedChecksum, HEX);
            Serial.print(F(", received checksum=0x"));
            Serial.println(receivedChecksum, HEX);
            return RECEIVE_ERROR;
          } else {
            return RECEIVE_FINISHED;
          }
        }
      }
      break;
  }

  replyFrameBufferIndex++;
  return RECEIVE_OK;
}


void processJKBMSStatusFrame() {
  /*
     Do it once at every debug start
  */
  if (replyFrameBufferIndex == 0) {
    Serial.println(F("replyFrameBufferIndex is 0"));
  } else {
    //Serial.print(replyFrameBufferIndex + 1);
    //Serial.println(F(" bytes received"));
    //printJKReplyFrameBuffer();
  }
  Serial.println();

  isFrameRequested = false; // Everything OK, do not try to receive more
  if (timeoutFrameCounter > 0) {
    // First frame after timeout
    timeoutFrameCounter = 0;
    Serial.println(F("Successfully receiving first BMS status frame after BMS communication timeout")); // Switch on LCD display, triggered by successfully receiving first BMS status frame
  }
  processReceivedData();
  // printJKReplyFrameBuffer();
}

bool readJKBMSStatusFrame() {
  millisOfLastReceivedByte = millis();
  uint8_t receiveResultCode = readJKBMSStatusFrameByte();
  if (receiveResultCode == RECEIVE_FINISHED) {
    /* All JK-BMS status frame data received
    */
    Serial.println("");
    Serial.println("Recieved BMS Response ...");
    return true;
  } else if (receiveResultCode != RECEIVE_OK) {
    /*
       Error here
    */
    Serial.print(F("Receive error="));
    Serial.print(receiveResultCode);
    Serial.print(F(" at index"));
    Serial.println(replyFrameBufferIndex);
    isFrameRequested = false; // do not try to receive more
    // printJKReplyFrameBuffer();
    totalVoltage = 0.0f;
    current = 0.0f;
    power = 0.0f;
    batteryValue = 0;
  }
  return false;
}

void handleFrameReceiveTimeout() {
  //sDoErrorBeep = true;
  isFrameRequested = false; // Do not try to receive more

  if (replyFrameBufferIndex != 0 || timeoutFrameCounter == 0) {
    /**
        No byte received here -BMS may be off or disconnected
        Do it only once if we receive 0 bytes
    */
    Serial.println();
    Serial.print(F("Receive timeout at ReplyFrameBufferIndex="));
    Serial.println(replyFrameBufferIndex);
    totalVoltage = 0.0f;
    current = 0.0f;
    power = 0.0f;
    batteryValue = 0;
    
    if (replyFrameBufferIndex != 0) {
      // printJKReplyFrameBuffer();
    }
  }
  timeoutFrameCounter++;
  if (timeoutFrameCounter == 0) {
    timeoutFrameCounter--; // To avoid overflow, we have an unsigned integer here
  }
}


void printJKReplyFrameBuffer() {
  for (uint16_t i = 0; i < (replyFrameBufferIndex + 1); ++i) {
    /*
       Insert newline and address after header (11 byte), before and after cell data before trailer (9 byte) and after each 16 byte
    */
    if (i == FRAME_HEADER_LENGTH || i == FRAME_HEADER_LENGTH + 2
        || i == (uint16_t)(FRAME_INDEX_OF_CELL_INFO_LENGTH + 1 + jkReplyFrameBuffer[FRAME_INDEX_OF_CELL_INFO_LENGTH])
        || i == ((replyFrameBufferIndex + 1) - FRAME_TRAILER_LENGTH)
        || (i < ((replyFrameBufferIndex + 1) - FRAME_TRAILER_LENGTH) && i % 16 == 0) /* no 16 byte newline in trailer*/
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
    if (jkReplyFrameBuffer[i] < 0x10) {
      Serial.print('0'); // padding with zero
    }
    Serial.print(jkReplyFrameBuffer[i], HEX);
    Serial.print(' ');

  }
  Serial.println();
}

void initJKReplyFrameBuffer() {
  replyFrameBufferIndex = 0;
}


void processReceivedData() {
  /*
     Set the static pointer to the start of the reply data which depends on the number of cell voltage entries
     The JKFrameAllDataStruct starts behind the header + cell data header 0x79 + CellInfoSize + the variable length cell data (CellInfoSize is contained in jkReplyFrameBuffer[12])
  */
  JKFAllReplyPointer = reinterpret_cast<JKReplyStruct*>(&jkReplyFrameBuffer[FRAME_HEADER_LENGTH + 2
                       + jkReplyFrameBuffer[FRAME_INDEX_OF_CELL_INFO_LENGTH]]);
}
