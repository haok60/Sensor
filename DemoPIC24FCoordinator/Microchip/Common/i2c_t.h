/* 
 * File:   i2c.h
 * Author: mypc
 *
 * Created on December 14, 2018, 9:58 AM
 */

#ifndef I2C_T_H
#define	I2C_T_H
#include "generic.h"
#define SLAVE_ADDRESS   0x4A
// Enable I2C Module
void I2C1_Enable(void);

// Disable I2C Module
void I2C1_Disable(void);

// Check if bus is idle
void I2C1_Idle(void);

// Start Event
void I2C1_Start(void);

// Stop Event
void I2C1_Stop(void);

// Restart Event
void I2C1_Restart(void);

// Acknowledge Insert
void I2C1_Ack(void);

// NotAcknowledge Insert
void I2C1_NAck(void);

// Send Data
void I2C1_Send(BYTE data);

// Receive Data
BYTE I2C1_Receive(void);

// Write Complete check
void I2C1_WriteCmpt(BYTE sAddress);

// Write Command with write-check 16-bit addressing
BYTE I2C1_iWrite(BYTE sAddress, unsigned int rAddress, 
                          BYTE rData);

// Write Command with write-check 8-bit addressing
BYTE I2C1_iWrite8(BYTE sAddress, BYTE rAddress, 
                          BYTE rData);

// Read a byte from I2C1 bus
BYTE I2C1_iRead8(BYTE sAddress, BYTE rAddress);




//test

//#define I2C_WADDR(x) (x & 0xFE) //clear R/W bit of I2C address
//#define I2C_RADDR(x) (x | 0x01) // set R/W bit of I2C address
//#define I2C_ACK 0x0000 /*Transmit 1 to send NACK as acknowledge*/
//#define I2C_NAK 0x0020 /*Transmit 0 to send ACK as acknowledge */
//
//void I2C1Init(void);
//void startI2C1(void);
//void rstartI2C1(void);
//void stopI2C1(void);
//void putI2C1(BYTE val);
//BYTE getI2C1(BYTE ack2Send);
//void write1I2C1(BYTE addr ,BYTE d1);
//void write2I2C1(BYTE addr,BYTE d1, BYTE d2);
//void read1I2C1 (BYTE addr,BYTE* pu8_d1);
//void read2I2C1 (BYTE addr,BYTE* pu8_d1, BYTE* pu8_d2);
//
////I2C2
//void I2C2Init(void);
//void startI2C2(void);
//void rstartI2C2(void);
//void stopI2C2(void);
//void putI2C2(BYTE val);
//BYTE getI2C2(BYTE ack2Send);
//void write1I2C2(BYTE addr ,BYTE d1);
//void write2I2C2(BYTE addr,BYTE d1, BYTE d2);
//void read1I2C2(BYTE addr,BYTE* pu8_d1);
//void read2I2C2(BYTE addr,BYTE* pu8_d1, BYTE* pu8_d2);
#endif	/* I2C_H */

