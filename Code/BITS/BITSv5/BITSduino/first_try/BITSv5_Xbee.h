#ifndef _BITSv5_XBEE_H
#define _BITSv5_XBEE_H

#define XBEE_DEBUG true

// XBee Addresses
const uint32_t BitsSL = 0x417B4A3B; // BITS (white)Specific to the XBee on Bits
                                    // (the Serial Low address value)
const uint32_t GroundSL = 0x417B4A36; // GroundStations (u.fl)
const uint32_t BlueSL = 0x417B4A3A;   // Blue (Blue Tape)
const uint32_t WireSL = 0x419091AC;   // Wire (wire antenna)
const uint32_t GHOULSL = 0x4210F7E4;  // GHOUL
const uint32_t UniSH = 0x0013A200;    // Common across any and all XBees

// XBee object / global variables
XBee xbee = XBee();
XBeeResponse response = XBeeResponse();
ZBTxStatusResponse txStatus = ZBTxStatusResponse();
ZBRxResponse rx = ZBRxResponse();
ModemStatusResponse msr = ModemStatusResponse();
const int xbeeRecBufSize = 49; // Rec must be ~15bytes larger than send
const int xbeeSendBufSize = 34;
uint8_t xbeeRecBuf[xbeeRecBufSize];
uint8_t xbeeSendBuf[xbeeSendBufSize];

void xbeeRead(void);
bool xbeeSend(uint32_t TargetSL, uint8_t *payload);
void processGroundMessage(void);
void processBlueMessage(void);
void processWireMessage(void);
void processGHOULMessage(void);

#endif
