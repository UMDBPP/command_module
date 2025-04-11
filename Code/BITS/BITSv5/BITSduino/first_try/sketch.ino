 //UMD Nearspace Iridium Tracking Payload (BITS) {Balloon Iridium Tracking System)
//Written by Jonathan Molter and Luke Renegar
//Uses an Iridium 9603 SBD Modem for effective unlimited range, without the need for our own RF blackmagics*
//This software is specifically written with the bits3 board revisision in mind
// ACTIVE REV, 7/25/20

// Runs on Teensy 3.2

#include <XBee.h> //If using 900HP's this must be the custom cpp (or really any post gen2 XBees)
#include <IridiumSBD.h>
#include <Wire.h> //Needed for I2C to GNSS
#include <SparkFun_u-blox_GNSS_v3.h> //http://librarymanager/All#SparkFun_u-blox_GNSS_v3
#include <SingleFileDrive.h>
#include <LittleFS.h>

#include "BITSv5.h"

// Config Settings
#define DIAGNOSTICS false // Change this to see diagnostics
const bool sendingMessages = true; // Whether or not the device is sending messages; begins as true TODO

// Hard shutdowns are a bad idea, trace NS-88
// const long shutdownTimeInterval = 14400000; // In milliseconds; 14400000 is 4
// hours; defines after what period of time the program stops sending messages
const long gpsLogInterval = 1000;
const long gpsLandedInterval = 1000;

//Interval definitions
const long signalCheckInterval = 15000;
unsigned long messageTimeInterval = 60000; // In milliseconds; 300000 is 5 minutes; defines how frequently the program sends messages, now changeable                                                                      

// Program Timing
unsigned long startTime;
unsigned long lastMillisOfMessage = 0;
unsigned long lastSignalCheck = 0;
unsigned long lastLog = 0;

// The best global
int arm_status = 42;  // armed when = 42

void setup() {
  setup_handler();
}

void loop() {
  loop_handler();
}
