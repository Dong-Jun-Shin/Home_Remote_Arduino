#include <SoftwareSerial.h>
#include "Servo.h"

SoftwareSerial XbeeSerial(4,5);   // RX, TX
Servo servoObj; //서보객체
int servoPin = 12;                 // 모터 Pin

/**
 * 모터 작동 함수
 */
void motorRun(char msg){
  /*전등 켜기 설정*/
  if(msg=='W'){
    servoObj.write(0);            //각도 60도로 움직임
  }

  /*전등 끄기 설정*/
  if(msg=='X'){
    servoObj.write(35);             //각도 0도로 움직임
  }
}

/**
 * 신호 변화 표시 LED 함수 (BLE = 1, IR = 2, Zigbee = 3)
 */
void ledBlink(int type){
  for(int i=0; i<type; i=i+1){
    digitalWrite(13, HIGH);
    delay(200);
    digitalWrite(13, LOW);
    delay(200);
  }
}

void setup() {
  XbeeSerial.begin(9600);
  XbeeSerial.listen();              // SoftwareSerial에서 수신 시작
  servoObj.attach(servoPin);        //서보 시그널 핀설정
  servoObj.write(35);                //서보 초기각도 0도 설정
  pinMode(13, OUTPUT);              // 신호 수신LED
}

void loop() {
  XbeeSerial.listen();      // SoftwareSerial에서 수신 시작

  /*소프트웨어 시리얼로 수신*/
  if(XbeeSerial.available()){
    char receive_msg = XbeeSerial.read();
    if(receive_msg == 87 || receive_msg == 88){
      motorRun(receive_msg);  // 시리얼 전송에 따른 동작 함수
      ledBlink(3);            // 수신 여부 표시
    }
  }
}
