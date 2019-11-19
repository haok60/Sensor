/*
***********************************************************************************************
*						SHT1x digital temperature and humidity sensor Driver
*										All Rights Reserved
* File name		: SHT1x.c
* Programmer 	: John Leung, TechToys Co. Hong Kong
* Web presence  : www.TechToys.com.hk
* Downloaded from www.microchipc.com - see site for more sample PIC C code
* Note			: 
* Language		: C18 complier version 2.40, MPLAB v7.41
* Hardware		: PIC18LF4550-STK1
* Date			: 11 Oct 2006			Version 1.0 

***********************************************************************************************
*										DESCRIPTION
*
* This module provides an interface to Sensirion SHT10 digital temperature & humidity sensor
*
* pinout function summarized as below
* ---SHT1x			MCU -----------------
* DATA - data		RA1
* SCK  - clock		RA2
***********************************************************************************************
*/

//#include "pic24fj128ga306.h"
#include "SHT1x.h"
#include "delay.h"

/*
*********************************************************************************************************
*										LOCAL DEFINITIONS
*********************************************************************************************************
*/
                            //adr  command  r/w
#define STATUS_REG_W 0x06   //000   0011    0
#define STATUS_REG_R 0x07   //000   0011    1
#define MEASURE_TEMP 0x03   //000   0001    1
#define MEASURE_HUMI 0x05   //000   0010    1
#define RESET_SHT    0x1e   //000   1111    0

#define noACK 			0
#define ACK   			1


/*
*********************************************************************************************************
*                                       SOFT RESET THE SENSOR
*
* Description :	Soft reset, resets the interface, clears the status register to default values
*		Wait minimum 11 ms before next command
* Arguments   : none
*			
* Returns     : 1 if no response from the sensor
* Notes       :
*********************************************************************************************************
*/
char s_softreset(void)
{ 
  BYTE error=0;
  s_connectionreset();              //reset communication
  error+=s_write_byte(RESET_SHT);       //send RESET-command to sensor
  return error;                     //error=1 in case of no response from the sensor
}


/*
*********************************************************************************************************
*                   MAKE MEASUREMENT ON HUMIDITY AND TEMPERATURE IN 12BITS ADN 14BITS
*
* Description :	Makes a measurement (humidity/temperature) with checksum
* Arguments   : 
*			
* Returns     : 
* Notes       : It takes approximately 11/55/210 ms for a 8/12/14bit measurement.
*		Measurement data is stored until readout.
*		Two bytes of measurement data and one byte of CRC checksum will then be transmitted. 
*		The uC must acknowledge each byte by pulling the DATA line low.
*********************************************************************************************************
*/
char s_measure(BYTE *p_value, BYTE *p_checksum, BYTE mode)
{ 
  unsigned error=0;
  unsigned int i;

  s_transstart();                   //transmission start
  switch(mode){                     //send command to sensor
    case TEMP	: error+=s_write_byte(MEASURE_TEMP); break;//gui lenh xuong yeu cau SHT gui thong so nhiet do
    case HUMI	: error+=s_write_byte(MEASURE_HUMI); break;//gui lenh xuong yeu cau SHT gui thong so do am
    default     : break;	 
  }
  (mode==HUMI)?DelayMs(55):DelayMs(210);
  for (i=0;i<65535;i++) if(DATA_RD==0) break;	//wait until sensor has finished the measurement
  if(DATA_RD) error+=1; 						//or timeout (~2 sec.) is reached

  *(p_value+1)  = s_read_byte(ACK);    		//read the first byte (MSB)
  *(p_value)	= s_read_byte(ACK);    	 	//read the second byte (LSB)
  *p_checksum 	= s_read_byte(noACK);  		//read checksum

  return error;
}

/*
*********************************************************************************************************
*                         				TRANSMISSION START SEQUENCE
*
* Description : To initiate a transmission, a “Transmission Start?sequence has to be issued.
* Arguments   : none
*			
* Returns     : none
* Notes		  : 
* 					generates a transmission start 
*       					_____         ________
* 					DATA: 	     |_______|
*								___     ___
* 					SCK :	___|   |___|   |______
*********************************************************************************************************
*/
void s_transstart(void)
{  
   SCK=0;                   //Initial state
   DATA_TRIS = 1;			//pullup resistor brings DATA pin high
   DelayUs(1);
   SCK=1;
   DelayUs(1);
   DATA_WR=0; DATA_TRIS=0;
   DelayUs(1);
   SCK=0;  
   DelayUs(5);
   SCK=1;
   DelayUs(1);
   DATA_TRIS=1;		   		//pullup resistor brings DATA pin high
   DelayUs(1);
   SCK=0;	   
}


/*
*********************************************************************************************************
*                         				 CONNECTION RESET SEQUENCE
*
* Description : This sequence resets the interface only. The status register preserves its content.
* Arguments   : none
*			
* Returns     : none
* Notes		  : 
* 					communication reset: DATA-line=1 and at least 9 SCK cycles followed by transstart
*       			  _____________________________________________________         ________
* 				DATA:                                                      |_______|
*  				         _    _    _    _    _    _    _    _    _        ___     ___
* 				SCK : __| |__| |__| |__| |__| |__| |__| |__| |__| |______|   |___|   |______
*********************************************************************************************************
*/
void s_connectionreset(void)
{  
  BYTE i;
	DATA_WR     = 1;			//set data pin high
	DATA_TRIS 	= 0; 			//set data pin an output
	SCK			= 0; 
	SCK_TRIS	= 0;			//set CLK pin an output low

  for(i=0;i<9;i++)             	//9 SCK cycles for connection reset sequence
  {
      SCK=1;
      DelayUs(1);
      SCK=0;
      DelayUs(1);
  }
  s_transstart();            	//transmission start
}


/*
*********************************************************************************************************
*                         				LOW LEVEL READ FUNCTION 
*
* Description : Read a byte form the Sensibus and gives an acknowledge in case of "ack == ACK" 
* Arguments   : 'ack' 	ACK (1) if acknowledge required
*						noACK (0) in case acknowledge NOT required
*			
* Returns     : return the byte read from the sensor
* Notes		  : 
*********************************************************************************************************
*/
char s_read_byte(BYTE ack)
{ 
    BYTE i,val=0;
    DATA_TRIS = 1;                    //set DATA line an input
    SCK = 0;
    for (i=0x80;i>0;i/=2)             //shift bit for masking
    {
        SCK=1;                          //clk for SENSI-BUS
        DelayUs(2);
        if (DATA_RD) val=(val | i);        //read bit
        SCK=0;
        DelayUs(2);
    }
    if(ack==ACK)
    {
        DATA_TRIS = 0;
        DATA_WR = 0;
    }
    SCK=1;                          //clk #9 for ack
    DelayUs(5);						//pulse-width approx. 5 us
    SCK=0;
    DATA_TRIS = 1;                  //release DATA-line
    return val;
}


/*
*********************************************************************************************************
*                         				LOW LEVEL WRITE FUNCTION 
*
* Description : Write a byte on the Sensibus and checks the acknowledge
* Arguments   : 'value' is the byte to write to the sensor
*			
* Returns     : 1 in case of an error (no acknowledge) from the sensor
* Notes		  : 
*********************************************************************************************************
*/
char s_write_byte(BYTE value)
{ 
  BYTE i,error=0;

  DATA_TRIS = 0;                                        //RA1 is output
  for (i=0x80;i>0;i/=2)             			//shift bit for masking
  {
      if (i & value) DATA_WR=1;                         //masking value with i , write to SENSI-BUS
      else DATA_WR=0;
      SCK=1;                          			//clk for SENSI-BUS
      DelayUs(5);	   				//pulse-width approx. 5 us
      SCK=0;
      DelayUs(5);
  }
  DATA_TRIS=1;                           		//release DATA-line, let SHT10 sensor controls DATA line
  SCK=1;                            			//clk #9 for ack 
  error=DATA_RD;                       			//check ack (DATA will be pulled down by SHT11)
  SCK=0;        
  return error;                     			//error=1 in case of no acknowledge
}
