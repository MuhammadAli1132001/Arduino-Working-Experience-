#include <SoftwareSerial.h>
#define MILLISECONDS_BETWEEN_JK_DATA_FRAME_REQUESTS     2000
#define TIMEOUT_MILLIS_FOR_FRAME_REPLY                  100 // I measured 26 ms between request end and end of received 273 byte result
#define JK_FRAME_START_BYTE_0   0x4E
#define JK_FRAME_START_BYTE_1   0x57
#define JK_FRAME_END_BYTE       0x68
#define JK_BMS_TX_PIN            4

#define JK_BMS_RECEIVE_OK           0
#define JK_BMS_RECEIVE_FINISHED     1
#define JK_BMS_RECEIVE_ERROR        2

#define JK_BMS_FRAME_HEADER_LENGTH              11
#define JK_BMS_FRAME_TRAILER_LENGTH             9
#define JK_BMS_FRAME_INDEX_OF_CELL_INFO_LENGTH  (JK_BMS_FRAME_HEADER_LENGTH + 1) // +1 for token 0x79
#define MINIMAL_JK_BMS_FRAME_LENGTH             19
bool sStaticInfoWasSent = false; // Flag to send static Info only once after reset.

#include "SoftwareSerialTX.h"

//SoftwareSerial testSerial(1,3); // RX, TX
SoftwareSerialTX TxToJKBMS(JK_BMS_TX_PIN);
bool sFrameIsRequested = false;             // If true, request was recently sent so now check for serial input

uint32_t sMillisOfLastRequestedJKDataFrame = -MILLISECONDS_BETWEEN_JK_DATA_FRAME_REQUESTS; // Initial value to start first request immediately
uint32_t sMillisOfLastReceivedByte = 0;     // For timeout
uint16_t sReplyFrameBufferIndex;
uint16_t sReplyFrameLength;                 // Received length of frame

uint8_t JKReplyFrameBuffer[350];
uint16_t sTimeoutFrameCounter = 0;      // Counts BMS frame timeouts, (every 2 seconds)

void initJKReplyFrameBuffer();
void printJKReplyFrameBuffer();
void printReceivedData();
uint8_t readJK_BMSStatusFrameByte();

const uint8_t readCellVoltageCommand[] = {0xDD, 0xA5, 0x04, 0x00, 0xFF, 0xFC, 0x77};
const uint8_t TestJKReplyStatusFrame[] PROGMEM = { /* Header*/0x4E, 0x57, 0x01, 0x2D, 0x00, 0x00, 0x00, 0x00, 0x06, 0x00, 0x01,
/*Length of Cell voltages*/
0x79, 0x30,
/*Cell voltages*/
0x01, 0x0C, 0xC6, 0x02, 0x0C, 0xBE, 0x03, 0x0C, 0xC7, 0x04, 0x0C, 0xC7, 0x05, 0x0C, 0xC7, 0x06, 0x0C, 0xC5, 0x07, 0x0C, 0xC6, 0x08,
        0x0C, 0xC7, 0x09, 0x0C, 0xC2, 0x0A, 0x0C, 0xC2, 0x0B, 0x0C, 0xC2, 0x0C, 0x0C, 0xC2, 0x0D, 0x0C, 0xC1, 0x0E, 0x0C, 0xBE,
        0x0F, 0x0C, 0xC1, 0x10, 0x0C, 0xC1,
        /*JKFrameAllDataStruct*/
        0x80, 0x00, 0x16, 0x81, 0x00, 0x15, 0x82, 0x00, 0x15, /*Voltage*/0x83, 0x14, 0x6C, /*Current*/0x84, 0x08, 0xD0, /*SOC*/0x85,
//        0x80, 0x00, 0x16, 0x81, 0x00, 0x15, 0x82, 0x00, 0x15, /*Voltage*/0x83, 0x14, 0x6C, /*Current*/0x84, 0x80, 0xD0, /*SOC*/0x85,
        0x47, 0x86, 0x02, 0x87, 0x00, 0x04, 0x89, 0x00, 0x00, 0x01, 0xE0, 0x8A, 0x00, 0x0E, /*Warnings*/0x8B, 0x00, 0x00, 0x8C,
        0x00, 0x07, 0x8E, 0x16, 0x26, 0x8F, 0x10, 0xAE, 0x90, 0x0F, 0xD2, 0x91, 0x0F, 0xA0, 0x92, 0x00, 0x05, 0x93, 0x0B, 0xEA,
        0x94, 0x0C, 0x1C, 0x95, 0x00, 0x05, 0x96, 0x01, 0x2C, 0x97, 0x00, 0x07, 0x98, 0x00, 0x03, 0x99, 0x00, 0x05, 0x9A, 0x00,
        0x05, 0x9B, 0x0C, 0xE4, 0x9C, 0x00, 0x08, 0x9D, 0x01, 0x9E, 0x00, 0x5A, 0x9F, 0x00, 0x46, 0xA0, 0x00, 0x64, 0xA1, 0x00,
        0x64, 0xA2, 0x00, 0x14, 0xA3, 0x00, 0x46, 0xA4, 0x00, 0x46, 0xA5, 0xFF, 0xEC, 0xA6, 0xFF, 0xF6, 0xA7, 0xFF, 0xEC, 0xA8,
        0xFF, 0xF6, 0xA9, 0x0E, 0xAA, 0x00, 0x00, 0x01, 0x40, 0xAB, 0x01, 0xAC, 0x01, 0xAD, 0x04, 0x11, 0xAE, 0x01, 0xAF, 0x01,
        0xB0, 0x00, 0x0A, 0xB1, 0x14, 0xB2, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x00, 0x00, 0x00, 0x00, 0xB3, 0x00, 0xB4, 0x49,
        0x6E, 0x70, 0x75, 0x74, 0x20, 0x55, 0x73, 0xB5, 0x32, 0x31, 0x30, 0x31, 0xB6, 0x00, 0x00, 0xE2, 0x00, 0xB7, 0x31, 0x31,
        0x2E, 0x58, 0x57, 0x5F, 0x53, 0x31, 0x31, 0x2E, 0x32, 0x36, 0x5F, 0x5F, 0x5F, 0xB8, 0x00, 0xB9, 0x00, 0x00, 0x04, 0x00,
        0xBA, 0x49, 0x6E, 0x70, 0x75, 0x74, 0x20, 0x55, 0x73, 0x65, 0x72, 0x64, 0x61, 0x4A, 0x4B, 0x5F, 0x42, 0x32, 0x41, 0x32,
        0x30, 0x53, 0x32, 0x30, 0x50, 0xC0, 0x01,
        /*Trailer*/
        0x00, 0x00, 0x00, 0x00, 0x68, 0x00, 0x00, 0x51, 0xC2 };

uint8_t JKRequestStatusFrame[21] = { 0x4E, 0x57 /*4E 57 = StartOfFrame*/, 0x00, 0x13 /*0x13 | 19 = LengthOfFrame*/, 0x00, 0x00,
        0x00, 0x00/*BMS ID, highest byte is default 00*/, 0x06/*Function 1=Activate, 3=ReadIdentifier, 6=ReadAllData*/,
        0x03/*Frame source 0=BMS, 1=Bluetooth, 2=GPRS, 3=PC*/, 0x00 /*TransportType 0=Request, 1=Response, 2=BMSActiveUpload*/,
        0x00/*0=ReadAllData or commandToken*/, 0x00, 0x00, 0x00,
        0x00/*RecordNumber High byte is random code, low 3 bytes is record number*/, JK_FRAME_END_BYTE/*0x68 = EndIdentifier*/,
        0x00, 0x00, 0x01, 0x29 /*Checksum, high 2 bytes for checksum not yet enabled -> 0, low 2 Byte for checksum*/};

        
void setup() {
  Serial.begin(115200);
  TxToJKBMS.begin(115200);  // Set the baud rate to match your device
}

void loop() {

  if (millis() - sMillisOfLastRequestedJKDataFrame >= MILLISECONDS_BETWEEN_JK_DATA_FRAME_REQUESTS) {
        sMillisOfLastRequestedJKDataFrame = millis(); // set for next check
        /*
         * Flush input buffer and send request to JK-BMS
         */
        while (Serial.available()) {
            Serial.read();
        }
        Serial.println();
        Serial.println(F("Send requestFrame with TxToJKBMS"));
        for (uint8_t i = 0; i < sizeof(JKRequestStatusFrame); ++i) {
            Serial.print(F(" 0x"));
            Serial.print(JKRequestStatusFrame[i], HEX);
        }
        Serial.println();
        Serial.flush();

    for (uint8_t i = 0; i < sizeof(JKRequestStatusFrame); ++i) {
        TxToJKBMS.write(JKRequestStatusFrame[i]);
    }
        
        sFrameIsRequested = true; // enable check for serial input
        initJKReplyFrameBuffer();
        sMillisOfLastReceivedByte = millis(); // initialize reply timeout
    }

   
      processReceivedData(); // for statistics

       if (sFrameIsRequested) {
        if (Serial.available()) {
            if (readJK_BMSStatusFrame()) {
                /*
                 * Frame completely received, now process it
                 */
                processJK_BMSStatusFrame(); // Process the complete receiving of the status frame and set the appropriate flags
            }


        } else if (millis() - sMillisOfLastReceivedByte >= TIMEOUT_MILLIS_FOR_FRAME_REPLY) {
            /*
             * Here we have requested frame, but serial was not available fore a longer time => timeout at receiving
             * If no bytes received before (because of BMS disconnected), print it only once
             */
            handleFrameReceiveTimeout();
        }
    }
}


uint8_t readJK_BMSStatusFrameByte() {
    uint8_t tReceivedByte = Serial.read();
    JKReplyFrameBuffer[sReplyFrameBufferIndex] = tReceivedByte;

    /*
     * Plausi check and get length of frame
     */
    if (sReplyFrameBufferIndex == 0) {
        // start byte 1
        if (tReceivedByte != JK_FRAME_START_BYTE_0) {
            Serial.println(F("Error start frame token != 0x4E"));
            return JK_BMS_RECEIVE_ERROR;
        }
    } else if (sReplyFrameBufferIndex == 1) {
        if (tReceivedByte != JK_FRAME_START_BYTE_1) {
            // Error
            return JK_BMS_RECEIVE_ERROR;
        }

    } else if (sReplyFrameBufferIndex == 3) {
        // length of frame
        sReplyFrameLength = (JKReplyFrameBuffer[2] << 8) + tReceivedByte;

    } else if (sReplyFrameLength > MINIMAL_JK_BMS_FRAME_LENGTH && sReplyFrameBufferIndex == sReplyFrameLength - 3) {
        // Check end token 0x68
        if (tReceivedByte != JK_FRAME_END_BYTE) {
            Serial.print(F("Error end frame token 0x"));
            Serial.print(tReceivedByte, HEX);
            Serial.print(F(" at index"));
            Serial.print(sReplyFrameBufferIndex);
            Serial.print(F(" is != 0x68. sReplyFrameLength= "));
            Serial.print(sReplyFrameLength);
            Serial.print(F(" | 0x"));
            Serial.println(sReplyFrameLength, HEX);
            return JK_BMS_RECEIVE_ERROR;
        }

    } else if (sReplyFrameLength > MINIMAL_JK_BMS_FRAME_LENGTH && sReplyFrameBufferIndex == sReplyFrameLength + 1) {
        /*
         * Frame received completely, perform checksum check
         */
        uint16_t tComputedChecksum = 0;
        for (uint16_t i = 0; i < sReplyFrameLength - 2; i++) {
            tComputedChecksum = tComputedChecksum + JKReplyFrameBuffer[i];
        }
        uint16_t tReceivedChecksum = (JKReplyFrameBuffer[sReplyFrameLength] << 8) + tReceivedByte;
        if (tComputedChecksum != tReceivedChecksum) {
            Serial.print(F("Checksum error, computed checksum=0x"));
            Serial.print(tComputedChecksum, HEX);
            Serial.print(F(", received checksum=0x"));
            Serial.println(tReceivedChecksum, HEX);

            return JK_BMS_RECEIVE_ERROR;
        } else {
            return JK_BMS_RECEIVE_FINISHED;
        }
    }
    sReplyFrameBufferIndex++;
    return JK_BMS_RECEIVE_OK;
}

void processJK_BMSStatusFrame() {
        /*
         * Do it once at every debug start
         */
        if (sReplyFrameBufferIndex == 0) {
            Serial.println(F("sReplyFrameBufferIndex is 0"));
        } else {
            Serial.print(sReplyFrameBufferIndex + 1);
            Serial.println(F(" bytes received"));
            printJKReplyFrameBuffer();
        }
        Serial.println();
    
    sFrameIsRequested = false; // Everything OK, do not try to receive more
    if (sTimeoutFrameCounter > 0) {
        // First frame after timeout
        sTimeoutFrameCounter = 0;
        Serial.println(F("successfully receiving first BMS status frame after BMS communication timeout")); // Switch on LCD display, triggered by successfully receiving first BMS status frame
    }
    processReceivedData();
   // printJKReplyFrameBuffer();
    
}

bool readJK_BMSStatusFrame() {
    sMillisOfLastReceivedByte = millis();
    uint8_t tReceiveResultCode = readJK_BMSStatusFrameByte();
    if (tReceiveResultCode == JK_BMS_RECEIVE_FINISHED) {
        /*
         * All JK-BMS status frame data received
         */
        return true;

    } else if (tReceiveResultCode != JK_BMS_RECEIVE_OK) {
        /*
         * Error here
         */
        Serial.print(F("Receive error="));
        Serial.print(tReceiveResultCode);
        Serial.print(F(" at index"));
        Serial.println(sReplyFrameBufferIndex);
        sFrameIsRequested = false; // do not try to receive more
        printJKReplyFrameBuffer();
    }
    return false;
}


void handleFrameReceiveTimeout() {
    //sDoErrorBeep = true;
    sFrameIsRequested = false; // Do not try to receive more

    if (sReplyFrameBufferIndex != 0 || sTimeoutFrameCounter == 0) {
        /*
         * No byte received here -BMS may be off or disconnected
         * Do it only once if we receive 0 bytes
         */
        Serial.print(F("Receive timeout at ReplyFrameBufferIndex="));
        Serial.println(sReplyFrameBufferIndex);
        if (sReplyFrameBufferIndex != 0) {
            printJKReplyFrameBuffer();
        }
    }
    sTimeoutFrameCounter++;
    if (sTimeoutFrameCounter == 0) {
        sTimeoutFrameCounter--; // To avoid overflow, we have an unsigned integer here
    }

}
void printJKReplyFrameBuffer() {
    for (uint16_t i = 0; i < (sReplyFrameBufferIndex + 1); ++i) {
        /*
         * Insert newline and address after header (11 byte), before and after cell data before trailer (9 byte) and after each 16 byte
         */
        if (i == JK_BMS_FRAME_HEADER_LENGTH || i == JK_BMS_FRAME_HEADER_LENGTH + 2
                || i
                        == (uint16_t) (JK_BMS_FRAME_INDEX_OF_CELL_INFO_LENGTH + 1
                                + JKReplyFrameBuffer[JK_BMS_FRAME_INDEX_OF_CELL_INFO_LENGTH])
                || i == ((sReplyFrameBufferIndex + 1) - JK_BMS_FRAME_TRAILER_LENGTH)
                || (i < ((sReplyFrameBufferIndex + 1) - JK_BMS_FRAME_TRAILER_LENGTH) && i % 16 == 0) /* no 16 byte newline in trailer*/
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
        if (JKReplyFrameBuffer[i] < 0x10) {
            Serial.print('0'); // padding with zero
        }
        Serial.print(JKReplyFrameBuffer[i], HEX);
        Serial.print(' ');

    }
    Serial.println();
}
void initJKReplyFrameBuffer() {
    sReplyFrameBufferIndex = 0;
}
struct JKReplyStruct *sJKFAllReplyPointer;

void processReceivedData() {
    /*
     * Set the static pointer to the start of the reply data which depends on the number of cell voltage entries
     * The JKFrameAllDataStruct starts behind the header + cell data header 0x79 + CellInfoSize + the variable length cell data (CellInfoSize is contained in JKReplyFrameBuffer[12])
     */
    sJKFAllReplyPointer = reinterpret_cast<JKReplyStruct*>(&JKReplyFrameBuffer[JK_BMS_FRAME_HEADER_LENGTH + 2
            + JKReplyFrameBuffer[JK_BMS_FRAME_INDEX_OF_CELL_INFO_LENGTH]]);
}


        
 






  
