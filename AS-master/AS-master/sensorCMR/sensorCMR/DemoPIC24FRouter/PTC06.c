#include "PTC06.h"
//#include "eeprom.h"
//#include "delay32.h"
BYTE length_photo[2];
WORD photo_len;
BYTE reset_cmd[] =
{
	0x56,
	0x00,
	0x26,
	0x00,
        0x00
};
BYTE reset_rcv [] =
{
	0x76,
	0x00,
	0x26,
	0x00
};
BYTE start_photo_cmd [] =
{
	0x56,
	0x00,
	0x36,
	0x01,
	0x00
};
BYTE start_photo_rcv [] =
{
	0x76,
	0x00,
	0x36,
	0x00,
	0x00
};
BYTE read_len_cmd [] =
{
	0x56,
	0x00,
	0x34,
	0x01,
	0x00
};
BYTE read_len_rcv [] =
{
	0x76,
	0x00,
	0x34,
	0x00,
	0x04,
	0x00,
	0x00
};
BYTE get_photo_cmd [] =
{
	0x56,
	0x00,
	0x32,
	0x0C,
	0x00,
	0x0A,

	0x00,
	0x00,
	0x00,
	0x00,

	0x00,
	0x00,
	0x00,
	0x00,

	0xFF,
	0xFF

};
BYTE get_photo_rcv [] =
{
	0x76,
	0x00,
	0x32,
	0x00,
	0x00
};

#ifdef ZOOM_SIZE
BYTE zoom_size_cmd [] =
{
	0x56,
	0x00,
	0x54,
	0x01,
	0x11,
        0x00
};
#endif

BYTE zoom_size_rcv [] =
{
	0x76,
	0x00,
	0x54,
	0x00,
	0x00,
};
BYTE stop_photo_cmd [] =
{
	0x56,
	0x00,
	0x36,
	0x01,
	0x03
};

BYTE stop_photo_rcv [] =
{
	0x76,
	0x00,
	0x36,
	0x00,
	0x00
};
#ifdef SIZE_640x480
BYTE photo_size_cmd [] =
{
	0x56,
	0x00,
	0x31,
	0x05,
	0x04,
	0x01,
	0x00,
	0x19,
	0x00,
	0x00
};

#endif
#ifdef SIZE_320x240
BYTE photo_size_cmd [] =
{
	0x56,
	0x00,
	0x31,
	0x05,
	0x04,
	0x01,
	0x00,
	0x19,
	0x11,
	0x00
};
#endif
#ifdef SIZE_160x120
BYTE photo_size_cmd [] =
{
	0x56,
	0x00,
	0x31,
	0x05,
	0x04,
	0x01,
	0x00,
	0x19,
	0x22,
	0x00
};
#endif
BYTE photo_size_rcv [] =
{
	0x76,
	0x00,
	0x31,
	0x00,
	0x00
};
BYTE save_power_cmd [] =
{
	0x56,
	0x00,
	0x3E,
	0x03,
	0x00,
	0x01,
	0x00
};
BYTE save_power_rcv [] =
{
	0x76,
	0x00,
	0x3E,
	0x00,
	0x00
};
BYTE compress_rate_cmd [] =
{
	0x56,
	0x00,
	0x31,
	0x05,
	0x01,
	0x01,
	0x12,
	0x04,
	0x36
};

BYTE compress_rate_rcv [] =
{
	0x76,
	0x00,
	0x31,
	0x00,
	0x00
};
BYTE set_bps_cmd [] =
{
	0x56,
	0x00,
	0x24,
	0x03,
	0x01,
#ifdef BAUD_RATE_115200
	0x0D,
	0xA6
#endif
#ifdef  BAUD_RATE_9600
        0xAE,
        0xC8
#endif
#ifdef  BAUD_RATE_19200
        0x56,
        0xE4
#endif
#ifdef  BAUD_RATE_38400
        0x2A,
        0xF2
#endif
#ifdef  BAUD_RATE_57600
        0x1C,
        0x4C
#endif
};
BYTE set_bps_rcv [] =
{
	0x76,
	0x00,
	0x24,
	0x00,
	0x00
};


void sendByte(BYTE byte){
    ConsolePut(byte);
}
// gui lenh xuong cmr
void sendComand(BYTE buffer[], WORD len){
    WORD i = 0;
    for(i = 0; i<len; i++){
        sendByte(buffer[i]);
    }

}
// nhan data tu cmr
BYTE getCommand( BYTE buffer[], WORD len){
    WORD i=0;
    for(i = 0;i<len; i++){
          buffer[i]= ConsoleGet();
          CLRWDT();
          }
     
  if(buffer[0] == CMD_CMR){
     // UART1PutROMString("step\n");
        return 1;
        }
    else
        return 0;
}
BYTE getCommand1( BYTE buffer[], WORD len){
    WORD i=0;
    BYTE j=2;
    while(j!=0)
    for(i = 0;i<len; i++){
          buffer[i]= ConsoleGet();
          CLRWDT();
          j--;
          }
    }


// reset cmr
BYTE sendReset(void){
    BYTE buffer[50];
    WORD i = 0;
    UART1PutROMString("start\n");
    sendComand(reset_cmd, 5);
    getCommand(buffer,10);

    for( i =0; i<4; i++){
        if(buffer[i]!= reset_rcv[i]) return 0;
      }
    UART1PutROMString("end");
    return 1;
    }
/*lenh lay do dai anh*/
BYTE sendGetLength(void){
    BYTE buffer[7];
    BYTE i = 0;
    //UART1PutROMString("start\n");
    sendComand(read_len_cmd, 5);
    if(!getCommand(buffer, 9)){
         for(i = 0;i<9; i++){
         UART1PrintChar(buffer[i]);
        }
        return 0;
    }
    for(i = 0; i<7; i++){

        if(buffer[i]!=read_len_rcv[i]){

            return 0;
             }
    }
    setPhotoLen(buffer[7], buffer[8]);
   for(i = 0;i<9; i++){
         UART1PrintChar(buffer[i]);
        }
     //printf("end\n");
    return 1;
}
/*lenh gan lenh lay du lieu */
void setPhotoLen(BYTE len1, BYTE len2){
    WORD temp;
    temp  = len1*0x100 +len2;
    get_photo_cmd[12] =len1;
    get_photo_cmd[13] = len2;
    photo_len =temp;

}
/*lenh chup anh*/
BYTE sendStartPhoto(void){
    BYTE buffer[5];
    BYTE i = 0;
    //UART1PutROMString("start\n");
    sendComand(start_photo_cmd, 10);
    if(!getCommand(buffer, 5)) return 0;
    for(i = 0; i<5; i++){
        if(buffer[i]!= start_photo_rcv[i]){
            return 0;
        }
    }
     //printf("end\n");
    return 1;
}
/*lenh lay du lieu anh va tra lai do dai anh(tinh bang byte)*/
WORD sendGetPhoto(BYTE txbuffer[]){
    WORD i = 0;
    WORD temp;
   //UART1PutROMString("start\n");
    temp=10+photo_len;
    UART1PrintWord(temp);
    sendComand(get_photo_cmd, 16);
    //UART1PutROMString("start1\n");
    getCommand(txbuffer,temp);
       for(i = 0; i<5; i++){
        if(txbuffer[i]!=get_photo_rcv[i]){
          //   printf("erro1\n");
            return 0;
        }
    }
     if (txbuffer[5] != 0xFF || txbuffer[6] != 0xD8 ||
		txbuffer[3 + photo_len] != 0xFF || txbuffer[4 + photo_len] != 0xD9)
	{
		return 0;
	}
	for (i = 0; i < photo_len; i++)
		txbuffer[i] = txbuffer[i + 5];
    //UART1PutROMString("end_length_data\n");
    return photo_len;
}
//lenh chup va lay anh
WORD takePhoto(BYTE buffer[]){
    WORD len;
    if(!sendStartPhoto()) goto error_exit;
    if(!sendGetLength()) goto error_exit;
    //len=sendGetPhoto(buffer);
    if(len = 0) goto error_exit;
    return len;
    error_exit:
    return 0;
}


void sendBuffer(BYTE const *buffer, WORD len){
    int i = 0;
    for(i = 0; i<len; i++){
        ConsolePut(buffer[i]);
    }
}
WORD getBuffer(BYTE *buffer, WORD len){
    WORD i =0;
    for(i = 0; i<len; i++)
        buffer[i] = ConsoleGet();
    if(buffer[0] == CMD_CMR)
        return 1;
    else
        return 0;
}
/*lenh dung chup anh xoa bo dem cache*/
BYTE sendStopPhoto(void){
    BYTE i;
    BYTE buffer[5];
//UART1PutROMString("start\n");
    sendComand(stop_photo_cmd, 5);
    if(!getCommand(buffer, 5)) return 0;
    for(i = 0; i<5; i++)

        if(buffer[i]!=stop_photo_rcv[i]){

            return 0;
        }
 //printf("end xoa\n");
    return 1;
}
/*config kich thuoc anh*/
BYTE sendPhotoSize(void)
{
    BYTE i;
    BYTE buffer[5];
   // printf("start\n");
    sendComand(photo_size_cmd, 10);
    if(!getCommand(buffer, 5)){
      //  printf("error1\n");
        return 0;
    }
    for(i = 0; i<5; i++)

        if(buffer[i] !=photo_size_rcv[i]){

            return 0;

        }
     for(i = 0;i<5; i++){
         UART1PrintChar(buffer[i]);
        }
   // printf("end\n");
    return 1;
}

BYTE sendZoomSize(void)
{
    BYTE i;
    BYTE buffer[5];
#ifdef ZOOM_SIZE
    sendComand(zoom_size_cmd, 6);
#endif
    if(!getBuffer(buffer, 5)) return 0;
    for(i = 0; i<5; i++)
        if(buffer[i]!= zoom_size_rcv[i]) return 0;
    return 1;
}
BYTE sendSavePower(void)
{
    BYTE i;
    BYTE buffer[5];
    sendComand(save_power_cmd, 7);
    if(!getBuffer(buffer, 5)) return 0;
    for(i = 0; i<5; i++)
        if(buffer[i]!= save_power_rcv[i]) return 0;
    return 1;
}
BYTE photoInit(void)
{
    //if(!sendReset()) goto error;
    if(!sendPhotoSize()) goto error;
    if(!sendStopPhoto()) goto error;
    if(!sendCompressRate()) goto error;

    return 1;
    error:
    return 0;
}
BYTE sendCompressRate(void){
    BYTE i;
    BYTE buffer[5];
//UART1PutROMString("start\n");
     sendComand(compress_rate_cmd, 9);
    if(!getCommand(buffer, 5)) return 0;
    for(i = 0; i<5; i++){

        if(buffer[i]!= compress_rate_rcv[i]){

            return 0;
        }
    }
 //printf("end\n");
    return 1;

}
BYTE setBaudRate(void)
{
    BYTE i;
    BYTE buffer[5];
    sendComand(set_bps_cmd, 7);


    if(!getBuffer(buffer, 5)) return 0;
    for(i = 0; i<5; i++)
        if(buffer[i]!= set_bps_rcv[i]) return 0;

    return 1;
}


