#include <SoftwareSerial.h>       // 0,1번핀 제외하고 Serial 통신을 하기 위해 선언
#include <LiquidCrystal_I2C.h>    // LCD 1602 I2C용 라이브러리
#include <Wire.h>                 // i2C 통신을 위한 라이브러리
#include <DHT.h>                  // 온습도 라이브러리
#include <IRremote.h>             // IR 센서 라이브러리

SoftwareSerial BTSerial(12, 11);  // BLE RX,TX
unsigned long clapCntOnMillis=1;  // 박수 회수 체크 타이머
unsigned long preAutoOnMillis=1;  // 자동 On 타이머 시작 시간
unsigned long preAutoOffMillis=1; // 자동 Off 타이머 시작 시간
unsigned long previousMillis=1;   // 타이머 시작 시간
unsigned long currentMillis;      // 타이머 체크 시간
//LCD 관련 변수
int delayTimer = 5000;            // 타이머 간격 시간
LiquidCrystal_I2C lcd(0x27,16,2); // 접근주소: 0x3F or 0x27
// 온습도 관련 변수
int dhtPin = A0;                  // 온습도 Pin
DHT dht(dhtPin, DHT22);           // 온습도 객체 (DHT22 (AM2302) 센서종류 설정)
// IR 관련 변수
int ir = 3;                       // ir Pin
IRsend irsend;                    // irsend 객체 형성
// 취침 자동전원 관련 변수
int onTimer = 30;                 // 시작 타이머 반복 시간 간격
int offTimer = 90;                // 종료 타이머 반복 시간 간격
int onTimerCnt = 0;               // 시작 타이머 작동 반응 변수
int offTimerCnt = 0;              // 종료 타이머 작동 반응 변수 
int autoFuncState = 0;            // 자동 취침 기능 사용 여부 0:미사용 1:사용
int autoAirconState = 0;          // 자동 취침 이용할 때, 에어컨 작동 여부
int autoFanState = 0;             // 자동 취침 이용할 때, 선풍기 작동 여부
// 조도센서 관련 변수
int cdsPin = A1;                  // 조도센서 Pin
int cds = 0;                      // 조도센서 값
int cdsCnd = 670;                 // 조도센서 기준 값
int cnt = 1;                      // 조도센서 변화 반응 변수
int cdsFuncState = 0;             // 조도센서를 이용한 기능 사용 여부
int airconState = 0;              // 조도센서 이용할 때, 에어컨 작동 여부
int lightState = 0;               // 조도센서 이용할 때, 전등 작동 여부
int fanState = 0;                 // 조도센서 이용할 때, 선풍기 작동 여부
// 소리 센서 관련 변수
int soundSensor = A3;             // 소리 센서
int minLimit = 50;                // 측정 가능한 최소 값
int minSound = 100;                // 소음 허용 범위
int maxSound = 105;               // 소음 허용 범위
int maxLimit = 200;               // 측정 가능한 최소 값
int countClap = 0;                // 인식 박수 횟수
int delayClap = 1000;              // 박수 사이 간격(ms)
int soundFuncState = 1;           // 전등 박수 기능 사용

//에어컨 ir 코드
const uint16_t onIrSignal[] PROGMEM = { 9000,4400, 600,1650, 600,1600, 600,500, 600,500, 600,500, 600,500, 600,1650, 600,1650, 550,1650, 550,1700, 600,1650, 550,500, 600,1650, 600,1650, 550,1650, 600,500, 600,500, 600,500, 550,550, 600,500, 600,500, 600,1650, 550,1700, 550,1650, 600,500, 600,500, 600,500, 600,500, 600,500, 600,500, 600,500, 550,550, 600,500, 550,550, 600,500, 600,500, 550,550, 600,500, 600,1650, 550,500, 600,550, 550,500, 600,550, 550,500, 600,500, 600,550, 550,550, 550,550, 550,550, 550,550, 550,550, 550,550, 600,500, 600,1650, 550,500, 600,500, 600,500, 600,500, 600,500, 600,500, 600,500, 600,500, 600,500, 600,500, 600,500, 600,500, 600,500, 600,500, 600,500, 600,500, 600,500, 600,500, 600,500, 600,500, 600,500, 600,500, 600,500, 600,1650, 600,500, 600,500, 600,500, 600,500, 600,500, 600,500, 600,500, 600,500, 600,500, 600,500, 600,1650, 600,500, 600,1650, 550,550, 550,550, 550,550, 550,550, 550,550, 550,1650, 600,1600, 600,1650, 600,1650, 550,1650, 600,500, 600,500, 550,1700, 550 };
const uint16_t offIrSignal[] PROGMEM = { 8950,4400, 600,1650, 550,1700, 550,550, 550,550, 550,550, 600,500, 600,1650, 550,1650, 550,1650, 600,1650, 550,1700, 550,550, 550,1650, 600,1650, 550,1650, 600,500, 600,500, 600,500, 600,500, 600,500, 600,500, 600,1650, 600,1650, 550,1650, 600,500, 600,500, 600,500, 600,500, 600,500, 600,500, 600,500, 600,500, 600,500, 600,500, 600,500, 600,500, 600,500, 600,500, 600,1650, 600,500, 550,550, 550,550, 550,550, 550,550, 550,550, 600,500, 550,550, 600,500, 550,550, 550,550, 550,550, 550,550, 550,550, 550,1700, 600,500, 550,550, 550,550, 550,550, 550,550, 600,500, 600,500, 550,550, 600,500, 600,500, 550,550, 550,550, 550,550, 550,550, 600,500, 550,550, 550,550, 550,550, 600,500, 600,500, 600,500, 600,500, 600,500, 600,500, 550,550, 600,500, 600,500, 600,500, 600,500, 600,500, 600,500, 600,500, 600,500, 550,550, 600,1650, 550,550, 600,1650, 550,500, 600,500, 600,500, 600,500, 600,500, 600,1650, 600,1650, 550,1650, 600,1650, 550,1650, 600,1650, 600,1650, 550,500, 600 };

/**
 * 상태 변경 함수
 */
void changeState(int &state){
  if(state==0){
    state = 1;
  }else{
    state = 0;
  }
}

/**
 * 전송을 위한 스위치 상태값 생성
 */
String setSwitchData(){
  String switchData = String(cdsFuncState) + ',' + String(airconState) + ',' + String(lightState) + ',' + String(fanState)
                            + '/' + String(autoFuncState) + ',' + String(autoAirconState) + ',' + String(autoFanState) + '/' + String(soundFuncState);
  return switchData;      
}

/**
 * 현재 시간 설정
 */
void cur_time_set(){
  currentMillis = millis();
}

/**
 * 타이머 시작시간 설정
 */
void pre_time_set(unsigned long &preMillis){
  preMillis = currentMillis;   // 타이머 설정
}

/**
 * 타이머 체크 함수 (1=종료)
 */
bool time_check(unsigned long preMillis, int delayTime){
  int check  = 0;
  
  // 설정한 delayTime이 지나면 실행
  if(currentMillis - preMillis >= delayTime && previousMillis!=0){
    check = 1;
  }
  
  return check;
}

/**
 * 대기 화면
 */
void waiting_on_lcd(){
    // Print a message to the LCD.
    lcd.clear();
    lcd.backlight();                // 백라이트 켜기
    lcd.setCursor(0,0);             // 1번째, 1라인
    lcd.print("Signal");
    lcd.setCursor(0,1);             // 1번째, 2라인
    lcd.print("waiting...");
}

/**
 * BT, Signal 표시 화면 (IR = 1, Zigbee = 2)
 */
void sgnRead_on_lcd(int type){
    String str = "";
    if(type==1) str="IR Send";
    if(type==2) str="Zigbee Send";
    if(type==3) str="Temp Send";
    if(type==4) str="CDS_OFF";
    if(type==5) str="CDS_ON";  
    if(type==6) str="AUTO_OFF";
    if(type==7) str="AUTO_ON"; 
    
    // Print a message to the LCD.
    lcd.clear();
    lcd.backlight();                // 백라이트 켜기
    lcd.setCursor(0,0);             // 1번째, 1라인
    lcd.print("BT Read");
    lcd.setCursor(0,1);             // 1번째, 2라인
    lcd.print(str);
}

/**
 * 온습도 표시 화면
 */
void dht_on_lcd(){
    float humi = dht.readHumidity();
    float temp = dht.readTemperature();
    //int result = isnan(temp) || isnan(humi);
    
    // Print a message to the LCD.
    lcd.clear();
    lcd.backlight();                // 백라이트 켜기
    lcd.setCursor(0,0);             // 1번째, 1라인
    lcd.print(String(humi) + " %");
    lcd.setCursor(0,1);             // 1번째, 2라인
    lcd.print(String(temp) + " `C");
}

/**
 * CDS 값 표시 화면
 */
void cds_on_lcd(){
    lcd.clear();
    lcd.backlight();                // 백라이트 켜기
    lcd.setCursor(0,0);             // 1번째, 1라인
    lcd.print("CDS : ");
    lcd.setCursor(0,1);             // 1번째, 2라인
    lcd.print(cds);
}

/**
 * 온습도, 조도 값 전송
 */
void sendTempData(){
    //z는 온습도, 통계 새로고침
    float humi = dht.readHumidity();
    float temp = dht.readTemperature();
    cds = analogRead(cdsPin);
    
    String sendTempData = String(humi) + ',' + String(temp) + ',' + String(cds);
    Serial.println(sendTempData);
    BTSerial.write(sendTempData.c_str());
    
    pre_time_set(previousMillis);
    sgnRead_on_lcd(1);        // 송신 여부 표시
}

/**
 * IR 송신 함수
 */
void irTransmit(byte data){
  long irKeyCodes; // 리모컨 고유 코드
      
  // 신호 구분
  switch (data) {
    //선풍기
    case 'A': irKeyCodes = 0x807FC03F; Serial.println("전원"); break;  
    case 'B': irKeyCodes = 0x807FE01F; Serial.println("풍량"); break; 
    case 'C': irKeyCodes = 0x807FD02F; Serial.println("예약"); break; 
    case 'D': irKeyCodes = 0x807FC837; Serial.println("풍향"); break; 
    case 'E': irKeyCodes = 0x807FF00F; Serial.println("바람모드"); break; 

    //빔프로젝터
    case 'F': irKeyCodes = 0xBD807F; Serial.println("전원/종료"); break; 
    case 'G': irKeyCodes = 0xBD50AF; Serial.println("메뉴"); break; 
    case 'H': irKeyCodes = 0xBD1AE5; Serial.println("이전 재생"); break; 
    case 'I': irKeyCodes = 0xBDFA05; Serial.println("재생/일시정지"); break; 
    case 'J': irKeyCodes = 0xBDEA15; Serial.println("다음 재생"); break; 
    case 'K': irKeyCodes = 0xBDD02F; Serial.println("상"); break; 
    case 'L': irKeyCodes = 0xBDF00F; Serial.println("하"); break; 
    case 'M': irKeyCodes = 0xBD926D; Serial.println("좌"); break; 
    case 'N': irKeyCodes = 0xBD52AD; Serial.println("우"); break; 
    case 'O': irKeyCodes = 0xBDB04F; Serial.println("확인"); break; 
    case 'P': irKeyCodes = 0xBD0AF5; Serial.println("뒤로 감기"); break; 
    case 'Q': irKeyCodes = 0xBD2AD5; Serial.println("이전"); break; 
    case 'R': irKeyCodes = 0xBD8A75; Serial.println("앞으로 감기"); break;
    case 'S': irKeyCodes = 0xBD08F7; Serial.println("음량 감소"); break; 
    case 'T': irKeyCodes = 0xBD56A9; Serial.println("음소거"); break; 
    case 'U': irKeyCodes = 0xBD30CF; Serial.println("음량 증가"); break; 
    case 'V': irKeyCodes = 0xBD20DF; Serial.println("외부입력"); break; 
  }
  
  digitalWrite(ir, HIGH);
  Serial.println("Sending");      // 시리얼 모니터 출력
  irsend.sendNEC(irKeyCodes, 32); // 리모컨 고유코드 전송
  digitalWrite(ir, LOW);
}

/**
 * aircon 작동 함수
 */
void airconTransmit(byte data) {
  const uint8_t khz = 38;
  
  if(data == 97){
    irsend.sendRaw_P(onIrSignal, sizeof(onIrSignal) / sizeof(onIrSignal[0]), khz);
  }else{
    irsend.sendRaw_P(offIrSignal, sizeof(offIrSignal) / sizeof(offIrSignal[0]), khz);
  }
}

void setup(){
  Serial.begin(9600);           // Serial 통신 초기화
  BTSerial.begin(9600);         // 통신 속도 9600bps로 블루투스 시리얼 통신 시작
  BTSerial.setTimeout(100);     // 통신할 때, 100ms까지만 대기
  dht.begin();                  // 온습도 객체 초기화
  lcd.init();                   // LCD 초기화
  waiting_on_lcd();             // 대기 문구 LCD 표시
  pinMode(ir, OUTPUT);          // ir LED
  pinMode(soundSensor, INPUT);  // 소리 센서
}

void loop() {
  cur_time_set();               // LCD 타이머 기준 시간 설정

  //LCD OFF 체크
  if(time_check(previousMillis, delayTimer)){
    lcd.noBacklight();   
  }

  //블루투스 통신 데이터 수신
  if(BTSerial.available()){
    String receiveData = BTSerial.readString();
    if (receiveData.length()==1){
      char data = receiveData[0];
      // Line feed 필터, 특정 문자 이외 필터
      if(data != 10 && data > 64 && data < 87){
        //A~V까지 IR 송신에 사용
        irTransmit(data);         // IR 신호 송신
        pre_time_set(previousMillis);           // LCD 타이머 시작 시간 설정
        sgnRead_on_lcd(1);        // 송신 여부 LCD 표시
      }else if(data == 87 || data == 88){
        //W, X는 나노보드 모터제어에 사용
        Serial.write(data);       // 나노보드에 신호 송신
        pre_time_set(previousMillis);           // LCD 타이머 시작 시간 설정
        sgnRead_on_lcd(2);        // 송신 여부 LCD 표시
      }else if(data == 89){
        //Y는 온습도 표시
        pre_time_set(previousMillis);           // LCD 타이머 시작 시간 설정
        dht_on_lcd();             // 온습도 LCD 표시
      }else if(data == 90){
        //Z는 온습도, 통계 데이터 전송
        sendTempData();           // 온습도, 조도 값 송신
        pre_time_set(previousMillis);           // LCD 타이머 시작 시간 설정
        sgnRead_on_lcd(3);        // 송신 여부 LCD 표시
      }else if(data == 97 || data == 98){
        //a, b는 에어컨 ON, OFF에 사용
        airconTransmit(data);     // 에어컨 신호 송신
        pre_time_set(previousMillis);           // LCD 타이머 시작 시간 설정
        sgnRead_on_lcd(1);        // 송신 여부 LCD 표시
      }else if(data > 113 && data < 122){
        //r, s, t, u, v, w, x, y는 조도센서, 자동취침 기능 사용 여부
        switch (data) {
        case 'r':
          changeState(soundFuncState); 
          break;  
        case 's': 
          changeState(autoFuncState); 
          pre_time_set(previousMillis);                 // LCD 타이머 시작 시간 설정
          sgnRead_on_lcd(6 + cdsFuncState); // 밝기 관련 자동 기능 사용여부 표시
          break;  
        case 't': 
          changeState(autoAirconState);
          break;  
        case 'u': 
          changeState(autoFanState);
          break;  
        case 'v': 
          changeState(cdsFuncState); 
          pre_time_set(previousMillis);                 // LCD 타이머 시작 시간 설정
          sgnRead_on_lcd(4 + cdsFuncState); // 밝기 관련 자동 기능 사용여부 표시
          break; 
        case 'w': 
          changeState(airconState);
          break;  
        case 'x': 
          changeState(lightState); 
          break; 
        case 'y': 
          changeState(fanState); 
          break; 
        }
        String sendFuncData = setSwitchData();
        Serial.println(sendFuncData);
      }else if(data == 122){
        //z는 현재 조도센서 관련 설정 송신
        String sendFuncData = setSwitchData();
        Serial.println(sendFuncData);
        BTSerial.write(sendFuncData.c_str());
      }
    }else{
      int first = receiveData.indexOf(",");
      String separator = receiveData.substring(0, first);
      String val = receiveData.substring(first+1, receiveData.length());
      
      if(separator=="on") onTimer = val.toInt()*1000*60;
      if(separator=="off") offTimer = val.toInt()*1000*60;
    }
  }

  // 조도센서 기능 사용했을 때
  if(cdsFuncState==1){
    cds = analogRead(cdsPin);                         // A1으로 들어오는 값을 cds에 저장
    if(cds < cdsCnd && cnt==1){                       // cds값이 cdsCnd보다 작으면 불이 켜져있음
      if(airconState==1) airconTransmit('a');         // 에어컨 켜기
      if(lightState==1) Serial.write('W');            // 전등 켜기   
      if(fanState==1) irTransmit('A');                // 선풍기 켜기
      cnt=0;                                          // 설정에 따른 중복 작동 방지
    }else if(cds >= cdsCnd && cnt==0){                // cds값이 cdsCnd보다 크면 불이 꺼져있음
      if(airconState==1) airconTransmit('b');         // 에어컨 끄기
      if(lightState==1) Serial.write('X');            // 전등 끄기  
      if(fanState==1) irTransmit('A');                // 선풍기 끄기
      cnt=1;                                          // 설정에 따른 중복 작동 방지
    }
    
    delay(100);                                       // 조도 센서 잡음값 필터
  }

  // 자동취침 기능 사용했을 때
  if(autoFuncState==1){
    if(preAutoOnMillis==1&&onTimerCnt==0&&offTimerCnt==0){
      if(autoAirconState==1) airconTransmit('a');         // 에어컨 켜기
      if(autoFanState==1) irTransmit('A');                // 선풍기 켜기
      pre_time_set(preAutoOnMillis);                      // 시작 타이머 시작
      onTimerCnt=1;
    }
    if(time_check(preAutoOnMillis, onTimer) && onTimerCnt==1){
      if(autoAirconState==1) airconTransmit('b');         // 에어컨 끄기
      if(autoFanState==1) irTransmit('A');                // 선풍기 끄기
      pre_time_set(preAutoOffMillis);                     // 종료 타이머 시작
      onTimerCnt=0;                                       // 시작 카운트 0 설정
      offTimerCnt=1;                                      // 종료 카운트 1 설정
    }
    if(time_check(preAutoOffMillis, offTimer) && offTimerCnt==1){
      if(autoAirconState==1) airconTransmit('a');         // 에어컨 켜기
      if(autoFanState==1) irTransmit('A');                // 선풍기 켜기
      pre_time_set(preAutoOnMillis);                      // 시작 타이머 시작
      onTimerCnt=1;                                       // 시작 카운트 1 설정
      offTimerCnt=0;                                      // 종료 카운트 0 설정
    }
  }

  // 전등 박수 기능 사용
  if(soundFuncState==1){
    int analogValue = analogRead(soundSensor);
    
    // 소리 최소, 최대값 사이인지 여부
    if ((minLimit <= analogValue) && (analogValue <= maxLimit)){
      // 소리 체크
      if(!((minSound <= analogValue) && (analogValue <= maxSound))){
        pre_time_set(clapCntOnMillis);
        countClap = (countClap < 2) ? countClap+1 : 0;
        delay(100);
      }
  
      // 박수가 delayClap를 초과하면, 초기화
      if(time_check(clapCntOnMillis, delayClap)){
        countClap = 0;
      }
      
      // 박수 2번 쌓였을 때 실행
      if(countClap == 2) {
        cds = analogRead(cdsPin);
        if(cds > cdsCnd){                                   // 전등이 꺼져있으면, 전등 켜기
          Serial.write('W');
        } else if(cds <= cdsCnd){                           // 전등이 켜져있으면, 전등 끄기
          Serial.write('X');
        }
        countClap = 0;
      }
    }
  }
}
