String cmd;
int measurePin = A0;
int ledPower = 2;
int samplingTime = 280;
int deltaTime = 40;
int sleepTime = 9680;
int dustDensity;
int voMeasured;
int current_time = 0;
int previous_time = 0;
int mac=60;
//float Vo;
void serialEventRun(void) {
  if (Serial.available()) serialEvent();
}
void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  pinMode(ledPower,OUTPUT);
  pinMode(A0,INPUT);
  digitalWrite(ledPower,HIGH);
  //String join= "#LJ:"+String(mac,DEC);
  //Serial.println(join);
}
void loop() {
  current_time =millis();
  dustDensity = readDust();
  if (current_time-previous_time>=5000)//5s
  {
    String dustDensity1 = "#LB:" +String(mac,DEC)+ String(dustDensity,HEX);
    Serial.println(dustDensity1);
    previous_time = current_time;
  }
}
void serialEvent(){
  
  while(Serial.available()>0){
    cmd=Serial.readString();
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
      Serial.println(dustDensity1);
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
  int voMeasured = analogRead(A0);// Đọc giá trị ADC V0
  delayMicroseconds(deltaTime); //Delay 0.02ms
  digitalWrite(ledPower,HIGH); // Tắt LED
  delayMicroseconds(sleepTime);//Delay 9.68ms
  //Vo = (5*voMeasured)/1024;
  //dustDensity1 = (Vo - Voc)*0.2;
  return voMeasured;
}
