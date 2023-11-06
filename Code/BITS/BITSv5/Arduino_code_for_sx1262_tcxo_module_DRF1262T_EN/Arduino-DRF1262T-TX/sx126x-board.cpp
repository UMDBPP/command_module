/*
  ______                              _
 / _____)             _              | |
( (____  _____ ____ _| |_ _____  ____| |__
 \____ \| ___ |    (_   _) ___ |/ ___)  _ \
 _____) ) ____| | | || |_| ____( (___| | | |
(______/|_____)_|_|_| \__)_____)\____)_| |_|
    (C)2013 Semtech

Description: SX126x driver specific target board functions implementation

License: Revised BSD License, see LICENSE.TXT file include in the project

Maintainer: Miguel Luis and Gregory Cristian
*/
#include "sx126x-board.h"
#include "sx126x.h"
#include <SPI.h>
static uint8_t IrqNestLevel = 0;

void DelayMs( uint32_t ms )
{
    delay( ms );
}

 void BoardDisableIrq( void )
{
    noInterrupts();
    IrqNestLevel++;
}

 void BoardEnableIrq( void )
{
    IrqNestLevel--;
    if( IrqNestLevel == 0 )
    {
        interrupts();
    }
}

static uint16_t SpiInOut(  uint16_t outData )
{
    uint8_t rxData = 0;
 
    rxData = SPI.transfer(outData);
    return( rxData );
}

/*!
 * Antenna switch GPIO pins objects
 */

void SX126xIoInit( void )
{
  pinMode (NssPin, OUTPUT);
  pinMode (NResetPin, OUTPUT);
  pinMode (SwPin, OUTPUT);
  
  pinMode (BusyPin, INPUT);
  pinMode (Dio1Pin, INPUT);

  digitalWrite(NssPin,1);
  digitalWrite(NResetPin,1);
  digitalWrite(SwPin,1);
}

void SX126xReset( void )
{
    delay( 10 );
    digitalWrite(NResetPin,0);
    delay( 20 );
    digitalWrite(NResetPin,1);
    delay( 10 );
}

void SX126xWaitOnBusy( void )
{
    while( digitalRead(BusyPin) == 1 );
}

void SX126xWakeup( void )
{
    BoardDisableIrq( );

    digitalWrite(NssPin,0);

    SpiInOut( RADIO_GET_STATUS );
    SpiInOut( 0x00 );

    digitalWrite(NssPin,1);

    // Wait for chip to be ready.
    SX126xWaitOnBusy( );

    BoardEnableIrq( );
}
void SX126xWriteCommand( uint8_t command, uint8_t *buffer, uint16_t size )
{
 //   SX126xCheckDeviceReady( );    // undetermined

    digitalWrite(NssPin,0);

    SpiInOut( (uint8_t )command );

    for( uint16_t i = 0; i < size; i++ )
    {
        SpiInOut( buffer[i] );
    }

    digitalWrite(NssPin,1);

    if( command != RADIO_SET_SLEEP )
    {
        SX126xWaitOnBusy( );
    }
}

void SX126xReadCommand( uint8_t command, uint8_t *buffer, uint16_t size )
{
     //   SX126xCheckDeviceReady( );    // undetermined

     digitalWrite(NssPin,0);

    SpiInOut(( uint8_t )command );
    SpiInOut(  0x00 );
    for( uint16_t i = 0; i < size; i++ )
    {
        buffer[i] = SpiInOut( 0 );
    }

    digitalWrite(NssPin,1);

    SX126xWaitOnBusy( );
}

void SX126xWriteRegisters( uint16_t address, uint8_t *buffer, uint16_t size )
{
     //   SX126xCheckDeviceReady( );    // undetermined
    digitalWrite(NssPin,0);   
    SpiInOut(  RADIO_WRITE_REGISTER );
    SpiInOut( ( address & 0xFF00 ) >> 8 );
    SpiInOut(  address & 0x00FF );
    
    for( uint16_t i = 0; i < size; i++ )
    {
        SpiInOut(  buffer[i] );
    }

    digitalWrite(NssPin,1);

    SX126xWaitOnBusy( );
}

void SX126xWriteRegister( uint16_t address, uint8_t value )
{
    SX126xWriteRegisters( address, &value, 1 );
}

void SX126xReadRegisters( uint16_t address, uint8_t *buffer, uint16_t size )
{
      //   SX126xCheckDeviceReady( );    // undetermined

    digitalWrite(NssPin,0);  

    SpiInOut( RADIO_READ_REGISTER );
    SpiInOut( ( address & 0xFF00 ) >> 8 );
    SpiInOut( address & 0x00FF );
    SpiInOut(  0 );
    for( uint16_t i = 0; i < size; i++ )
    {
        buffer[i] = SpiInOut( 0 );
    }
    digitalWrite(NssPin,1);  

    SX126xWaitOnBusy( );
}
uint8_t SX126xReadRegister( uint16_t address )
{
    uint8_t data;
    SX126xReadRegisters( address, &data, 1 );
    return data;
}

void SX126xWriteBuffer( uint8_t offset, uint8_t *buffer, uint8_t size )
{
    //   SX126xCheckDeviceReady( );    // undetermined

     digitalWrite(NssPin,0); 

    SpiInOut(  RADIO_WRITE_BUFFER );
    SpiInOut(  offset );
    for( uint16_t i = 0; i < size; i++ )
    {
        SpiInOut( buffer[i] );
    }
    digitalWrite(NssPin,1); 

    SX126xWaitOnBusy( );
}
void SX126xReadBuffer( uint8_t offset, uint8_t *buffer, uint8_t size )
{
    //   SX126xCheckDeviceReady( );    // undetermined

     digitalWrite(NssPin,0); 

    SpiInOut(  RADIO_READ_BUFFER );
    SpiInOut(  offset );
    SpiInOut(  0 );
    for( uint16_t i = 0; i < size; i++ )
    {
        buffer[i] = SpiInOut( 0 );
    }
    digitalWrite(NssPin,1); 

    SX126xWaitOnBusy( );
}
void SX126xSetRfTxPower( int8_t power )
{
    SX126xSetTxParams( power, RADIO_RAMP_40_US );
}
bool SX126xCheckRfFrequency( uint32_t frequency )
{
    // Implement check. Currently all frequencies are supported
    return true;
}