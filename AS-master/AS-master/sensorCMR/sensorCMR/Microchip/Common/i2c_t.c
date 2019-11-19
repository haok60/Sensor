#include "i2c_t.h"
//
// Enable I2C Module


void I2C1_Enable(){
    I2C1CONbits.I2CEN = 1;
    I2C1BRG=0x4E;
}

// Disable I2C Module
void I2C1_Disable(){
    I2C1CONbits.I2CEN = 0;
}

// Check if bus is idle
void I2C1_Idle(){
    // Wait till bus is idle P = 1
    while((I2C1CON & 0x001F) || I2C1STATbits.TRSTAT);
}

// Start Event
void I2C1_Start(){
    // Generate Start Event
    I2C1CONbits.SEN = 1;
    // wait here till SEN = 0 <- Start event competed
    while(I2C1CONbits.SEN);
}

// Stop Event
void I2C1_Stop(){
    // Generate Start Event
    I2C1CONbits.PEN = 1;
    // wait here till SEN = 0 <- Start event competed
    while(I2C1CONbits.PEN);
    //__delay_us(150);
}

// Restart Event
void I2C1_Restart(){
    // Generate Start Event
    I2C1CONbits.RSEN = 1;
    // wait here till SEN = 0 <- Start event competed
    while(I2C1CONbits.RSEN);
}

// Acknowledge Insert
void I2C1_Ack(){
    // Generate Start Event
    I2C1CONbits.ACKDT = 0;
    I2C1CONbits.ACKEN = 1;
    while(I2C1CONbits.ACKEN);
}

// NotAcknowledge Insert
void I2C1_NAck(){
    // Generate Start Event
    I2C1CONbits.ACKDT = 1;
    I2C1CONbits.ACKEN = 1;
    while(I2C1CONbits.ACKEN);
}



// Send Data
void I2C1_Send(BYTE data){
    // To send byte out data needs to be copied to Transmit register
    IFS1bits.MI2C1IF = 0;
    I2C1TRN = data;
    while(I2C1STATbits.TBF);
    while(IFS1bits.MI2C1IF);    //Wait for ninth clock cycle
    IFS1bits.MI2C1IF = 0;        //Clear interrupt flag
    while(I2C1STATbits.ACKSTAT);
}

// Receive Data
BYTE I2C1_Receive(){
    char result;
    I2C1CONbits.RCEN = 1;
    while(I2C1CONbits.RCEN);
    return result = I2C1RCV;
}

// Write Complete check
void I2C1_WriteCmpt(BYTE sAddress){
//    LATG = 0x00;
    while(1){
        I2C1_Idle();
        I2C1_Start();
        
        // To send byte out data needs to be copied to Transmit register
        IFS1bits.MI2C1IF = 0;
        I2C1TRN = sAddress << 1;
        while(I2C1STATbits.TBF);
        while(IFS1bits.MI2C1IF);    //Wait for ninth clock cycle
        IFS1bits.MI2C1IF = 0;        //Clear interrupt flag
        while(I2C1STATbits.ACKSTAT){
//            LATG++;
//            if(LATG == 0x70){
//                LATG = 0x00;
//            }
//            break;
        }
        I2C1_Idle();
        I2C1_Stop();
        if(I2C1STATbits.ACKSTAT == 0){
            I2C1_Idle();
            I2C1_Stop();
            break;
        }    
    }
}

// Write Command with write-check 16-bit addressing
BYTE I2C1_iWrite(BYTE sAddress, unsigned int rAddress, 
                          BYTE rData){
    BYTE result;
//    BYTE i;
    
    //I2C1_Enable();
    I2C1_Idle();
    I2C1_Start();
    I2C1_Send(sAddress << 1);
    I2C1_Idle();
    I2C1_Send(rAddress >> 8);
    I2C1_Idle();
    I2C1_Send(rAddress & 0xFF);
    I2C1_Idle();
    I2C1_Send(rData);
    I2C1_Idle();
    I2C1_Stop();
    
    I2C1_WriteCmpt(SLAVE_ADDRESS);
    
    
    I2C1_Idle();
    I2C1_Start();
    I2C1_Send(sAddress << 1);
    I2C1_Idle();
    I2C1_Send(rAddress >> 8);
    I2C1_Idle();
    I2C1_Send(rAddress & 0xFF);
    I2C1_Idle();
    I2C1_Restart();
    I2C1_Send((sAddress << 1) + 1);
    I2C1_Idle();
    result = I2C1_Receive();
    I2C1_Idle();
    I2C1_NAck();
    I2C1_Stop();
    
    return result;
}

// Write Command with write-check 8-bit addressing
BYTE I2C1_iWrite8(BYTE sAddress, BYTE rAddress, 
                          BYTE rData){
    BYTE result;
    BYTE i;
    //I2C1_Enable();
    I2C1_Idle();
    I2C1_Start();
    I2C1_Send(sAddress << 1);
    I2C1_Idle();
    I2C1_Send(rAddress);
    I2C1_Idle();
    I2C1_Send(rData);
    I2C1_Idle();
    I2C1_Stop();
    
    I2C1_WriteCmpt(SLAVE_ADDRESS);
    for( i = 0; i < 10; i++);
    
    I2C1_Idle();
    I2C1_Start();
    I2C1_Send(sAddress << 1);
    I2C1_Idle();
    I2C1_Send(rAddress);
    I2C1_Idle();
    I2C1_Restart();
    I2C1_Send((sAddress << 1) + 1);
    I2C1_Idle();
    result = I2C1_Receive();
    I2C1_Idle();
    I2C1_NAck();
    I2C1_Stop();
    
    return result;
}


BYTE I2C1_iRead8(BYTE sAddress, BYTE rAddress)
{
    char result;
    
    I2C1_Idle();
    I2C1_Start();
    I2C1_Send(sAddress << 1);
    I2C1_Idle();
    I2C1_Send(rAddress);
    I2C1_Idle();
    I2C1_Restart();
    I2C1_Send((sAddress << 1) + 1);
    I2C1_Idle();
    result = I2C1_Receive();
    I2C1_Idle();
    I2C1_NAck();
    I2C1_Stop();
    
    return result;
}



////I2C1
//void I2C1Init(void){
////    TRISFBITS.TRISRG=1;
////    TRISFBITS.TRISF5=1;
//      TRISGbits.TRISG3=1;
//      TRISGbits.TRISG2=1;
////    TRISFBITS |=0x30;
//    I2C1CONbits.I2CEN=1;
//    I2C1BRG=0x4E;
//    
//}
//
//void startI2C1(void){
//    I2C1CONbits.SEN = 1;
//    while (I2C1CONbits.SEN);
//}
//
//void rstartI2C1(void) {
//    I2C1CONbits.RSEN = 1;
//    while (I2C1CONbits.RSEN);
//}
//
//void stopI2C1(void) {
//    I2C1CONbits.PEN=1; 
//    while (I2C1CONbits.PEN);
//}
//
//void putI2C1(BYTE val) {
//    I2C1TRN = val;
//    while (I2C1STATbits.TRSTAT);
//    
//}
//
//BYTE getI2C1(BYTE ack2Send){
//    BYTE inByte;
//    while (I2C1CON & 0x1F);
//    I2C1CONbits.RCEN = 1; 
//    while (!I2C1STATbits.RBF);
//    inByte = I2C1RCV;
//    while (I2C1CON & 0x1F);
//    I2C1CONbits.ACKDT = ack2Send;
//    I2C1CONbits.ACKEN = 1; 
//    while (I2C1CONbits.ACKEN);
//    return(inByte);
//    
//}
//
// void write1I2C1(BYTE addr ,BYTE d1) {
//   startI2C1();
//   putI2C1(I2C_WADDR(addr));
//   putI2C1(d1);
//   stopI2C1();
//}
//void write2I2C1(BYTE addr,BYTE d1, BYTE d2) {
//   startI2C1();
//   putI2C1(I2C_WADDR(addr));
//   putI2C1(d1);
//   putI2C1(d2);
//   stopI2C1();
//}
//
//void read1I2C1 (BYTE addr,BYTE* pu8_d1) {
//   startI2C1();
//   putI2C1(I2C_RADDR(addr));
//   *pu8_d1 = getI2C1(I2C_NAK); //last ack bit from master to slave during read must be a NAK
//   stopI2C1();
//}
//void read2I2C1 (BYTE addr,BYTE* pu8_d1, BYTE* pu8_d2) {
//   startI2C1();
//   putI2C1(I2C_RADDR(addr));
//   *pu8_d1 = getI2C1(I2C_ACK);
//   *pu8_d2 = getI2C1(I2C_NAK);
//   stopI2C1();
//}
//
//
////I2C2
//void I2C2Init(void){
//    TRISFbits.TRISF4=1;
//    TRISFbits.TRISF5=1;
//    I2C2CONbits.I2CEN=1;
////    I2C2CONBITS.I2CEN=1;
//    I2C2BRG=0x4E;
//    
//}
//
//void startI2C2(void){
//    I2C2CONbits.SEN = 1;
//    while (I2C2CONbits.SEN);
//}
//
//void rstartI2C2(void) {
//    I2C2CONbits.RSEN = 1;
//    while (I2C2CONbits.RSEN);
//}
//
//void stopI2C2(void) {
//    I2C2CONbits.PEN=1; 
//    while (I2C2CONbits.PEN);
//}
//
//void putI2C2(BYTE val) {
//    I2C2TRN = val;
//    while (I2C2STATbits.TRSTAT);
//    
//}
//
//BYTE getI2C2(BYTE ack2Send){
//    BYTE inByte;
//    while (I2C2CON & 0x1F);
//    I2C2CONbits.RCEN = 1; 
//    while (!I2C2STATbits.RBF);
//    inByte = I2C2RCV;
//    while (I2C2CON & 0x1F);
//    I2C2CONbits.ACKDT = ack2Send;
//    I2C2CONbits.ACKEN = 1; 
//    while (I2C2CONbits.ACKEN);
//    return(inByte);
//    
//}
//
// void write1I2C2(BYTE addr ,BYTE d1) {
//   startI2C2();
//   putI2C2(I2C_WADDR(addr));
//   putI2C2(d1);
//   stopI2C2();
//}
//void write2I2C2(BYTE addr,BYTE d1, BYTE d2) {
//   startI2C2();
//   putI2C2(I2C_WADDR(addr));
//   putI2C2(d1);
//   putI2C2(d2);
//   stopI2C2();
//}
//
//void read1I2C2 (BYTE addr,BYTE* pu8_d1) {
//   startI2C2();
//   putI2C2(I2C_RADDR(addr));
//   *pu8_d1 = getI2C2(I2C_NAK); //last ack bit from master to slave during read must be a NAK
//   stopI2C2();
//}
//void read2I2C2 (BYTE addr,BYTE* pu8_d1, BYTE* pu8_d2) {
//   startI2C2();
//   putI2C2(I2C_RADDR(addr));
//   *pu8_d1 = getI2C2(I2C_ACK);
//   *pu8_d2 = getI2C2(I2C_NAK);
//   stopI2C2();
//}

