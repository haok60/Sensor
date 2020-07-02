#include <SHT1x.h>

#include <SoftwareSerial.h>
#include "Arduino.h"
SoftwareSerial mySerial(8,9); //TX, RX
String cmd;
int measurePin = A6;
int ledPower = 7;
int samplingTime = 280;
int deltaTime = 40;
int sleepTime = 9680;
int dustDensity;
int voMeasured;
int current_time = 0;
int previous_time = 0;
#define dataPin  A3
#define clockPin A2
float temp_c;
float humidity;
int temp_s,humi_s;
int mac=5;
SHT1x sht1x(dataPin, clockPin);
//float Vo;
void serialEventRun(void) {
  if (mySerial.available()) serialEvent();
}
void setup() {
  Serial.begin(9600);
  // put your setup code here, to run once:
  pinMode(2,OUTPUT);
  pinMode(3,OUTPUT);
  digitalWrite(2,LOW);
  digitalWrite(3,LOW);
  mySerial.begin(9600);
  //Serial.begin(9600);
  pinMode(ledPower,OUTPUT);
  pinMode(measurePin,INPUT);
  digitalWrite(ledPower,HIGH);
  //String join= "#LJ:"+String(mac,DEC);
  //Serial.println(join);
}
void loop() {
  current_time =millis();
  dustDensity = readDust();
  readTandH();
  //Serial.println(temp_c);
  //Serial.println(hum_s);
  if (current_time-previous_time>=10000)//5s
  {
    String dustDensity1 = "#LB:0" +String(mac,DEC)+ String(dustDensity,HEX);
    mySerial.println(dustDensity1);
    delay(4000);
    readTandH();
    String TempHum = "#LT:0" +String(mac,DEC)+ String(temp_s,DEC)+String(humi_s,DEC);
    mySerial.println(TempHum);
    previous_time = current_time;
  }
}
void serialEvent(){
  
  while(mySerial.available()>0){
    cmd=mySerial.readString();
  }
  sendData(cmd);
}
int checkcode(String cmd)
{
  if(cmd.substring(2) == "000$")return 1;
  if(cmd.substring(2) == "600$")return 2;
  if(cmd.substring(2) == "610$")return 3;
  if(cmd.substring(2) == "620$")return 4;
  if(cmd.substring(2) == "700$")return 5;
  return 0;
}

void sendData(String cmd)
{
  int macrq= cmd.substring(0,2).toInt();
  if(macrq!=mac)return;
  switch(checkcode(cmd))
  {
    //dustDensity = readDust();
    case 5:
      String dustDensity1 = "#LR:" +String(mac,DEC)+ String(dustDensity,HEX);
      mySerial.println(dustDensity1);
      break;
    default:
    break;
  }
}

int readDust()
{
  static float Voc = 0.6;
  digitalWrite(ledPower,LOW); // Bật IR LED
  delayMicroseconds(samplingTime);  //Delay 0.28ms
  int voMeasured = analogRead(measurePin);// Đọc giá trị ADC V0
  delayMicroseconds(deltaTime); //Delay 0.02ms
  digitalWrite(ledPower,HIGH); // Tắt LED
  delayMicroseconds(sleepTime);//Delay 9.68ms
  //Vo = (5*voMeasured)/1024;
  //dustDensity1 = (Vo - Voc)*0.2;
  return voMeasured;
}
void readTandH()
{
  //temp_c = sht1x.readTemperatureC();
  temp_c = sht1x.readTemperatureC();
  humidity = sht1x.readHumidity();
  temp_s = temp_c*100;
  humi_s = humidity*100;
}
