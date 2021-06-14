#include <SoftwareSerial.h>       // 0,1번핀 제외하고 Serial 통신을 하기 위해 선언
#include <LiquidCrystal_I2C.h>    // LCD 1602 I2C용 라이브러리
#include <Wire.h>                 // i2C 통신을 위한 라이브러리
#include <DHT.h>                  // 온습도 라이브러리
#include <IRremote.h>             // IR 센서 라이브러리

#define DHTPIN A0                 // SDA 핀의 설정
#define DHTTYPE DHT22             // DHT22 (AM2302) 센서종류 설정
#define cdsPin A1                 // 조도센서 Pin
#define soundSensor A3            // 소리 센서

SoftwareSerial BTSerial(11, 12);  // BLE TX,RX
unsigned long clapCntOnMillis=1;  // 박수 횟수 체크 타이머
unsigned long preAutoOffMillis=1; // 자동 On 타이머 시작 시간
unsigned long preAutoOnMillis=1;  // 자동 Off 타이머 시작 시간
unsigned long preAutoWindMillis=1;// 자동 송풍 타이머 시작 시간
unsigned long previousMillis=1;   // 타이머 시작 시간
unsigned long currentMillis;      // 타이머 체크 시간
//LCD 관련 변수
int delayTimer = 5000;                // 타이머 간격 시간
LiquidCrystal_I2C lcd(0x27,16,2);     // 접근주소: 0x3F or 0x27
// 온습도 관련 변수
DHT dht(DHTPIN, DHTTYPE);                 // 온습도 객체 (DHT22 (AM2302) 센서종류 설정)
// IR 관련 변수
int ir = 3;                           // ir Pin
IRsend irsend;                        // irsend 객체 형성
// 취침 자동전원 관련 변수   
unsigned long offTimer = 1800000;         // 종료 타이머 반복 시간 간격
unsigned long onTimer = 5400000;          // 실행 타이머 반복 시간 간격
unsigned long windTimer = 1200000;        // 송풍 타이머 시간
byte offTimerCnt = 0;                     // 종료 타이머 작동 반응 변수
byte onTimerCnt = 0;                      // 실행 타이머 작동 반응 변수 
byte windTimerCnt = 0;                    // 송풍 타이머 작동 반응 변수
boolean autoFuncState = 0;                // 자동 취침 기능 사용 여부 0:미사용 1:사용
boolean autoAirconState = 0;              // 자동 취침 이용할 때, 에어컨 작동 여부
boolean autoFanState = 0;                 // 자동 취침 이용할 때, 선풍기 작동 여부
// 조도센서 관련 변수
int cds = 0;                          // 조도센서 값
int cdsCnd = 800;                     // 조도센서 기준 값
boolean cnt = 1;                      // 조도센서 변화 반응 변수
boolean cdsFuncState = 0;             // 조도센서를 이용한 기능 사용 여부
boolean airconState = 0;              // 조도센서 이용할 때, 에어컨 작동 여부
boolean fanState = 0;                 // 조도센서 이용할 때, 선풍기 작동 여부
// 소리 센서 관련 변수
byte minLimit = 50;                       // 측정 가능한 최소 값
byte minSound = 100;                      // 소음 허용 범위
byte maxSound = 110;                      // 소음 허용 범위
byte maxLimit = 200;                      // 측정 가능한 최소 값
byte countClap = 0;                       // 인식 박수 횟수
byte delayClap = 1000;                    // 박수 사이 간격(ms)
boolean soundFuncState = 1;               // 전등 박수 기능 사용
boolean debugSound = 0;                   // 오작동 시, 센서 상태 확인
// 에어컨 관련 변수
boolean windFuncState = 1;                // 송풍 기능 사용

//에어컨 ir 코드
const uint16_t onIrSignal[] PROGMEM = { 9000,4400, 600,1650, 600,1600, 600,500, 600,500, 600,500, 600,500, 600,1650, 600,1650, 550,1650, 550,1700, 600,1650, 550,500, 600,1650, 600,1650, 550,1650, 600,500, 600,500, 600,500, 550,550, 600,500, 600,500, 600,1650, 550,1700, 550,1650, 600,500, 600,500, 600,500, 600,500, 600,500, 600,500, 600,500, 550,550, 600,500, 550,550, 600,500, 600,500, 550,550, 600,500, 600,1650, 550,500, 600,550, 550,500, 600,550, 550,500, 600,500, 600,550, 550,550, 550,550, 550,550, 550,550, 550,550, 550,550, 600,500, 600,1650, 550,500, 600,500, 600,500, 600,500, 600,500, 600,500, 600,500, 600,500, 600,500, 600,500, 600,500, 600,500, 600,500, 600,500, 600,500, 600,500, 600,500, 600,500, 600,500, 600,500, 600,500, 600,500, 600,500, 600,1650, 600,500, 600,500, 600,500, 600,500, 600,500, 600,500, 600,500, 600,500, 600,500, 600,500, 600,1650, 600,500, 600,1650, 550,550, 550,550, 550,550, 550,550, 550,550, 550,1650, 600,1600, 600,1650, 600,1650, 550,1650, 600,500, 600,500, 550,1700, 550 };
const uint16_t offIrSignal[] PROGMEM = { 8950,4400, 600,1650, 550,1700, 550,550, 550,550, 550,550, 600,500, 600,1650, 550,1650, 550,1650, 600,1650, 550,1700, 550,550, 550,1650, 600,1650, 550,1650, 600,500, 600,500, 600,500, 600,500, 600,500, 600,500, 600,1650, 600,1650, 550,1650, 600,500, 600,500, 600,500, 600,500, 600,500, 600,500, 600,500, 600,500, 600,500, 600,500, 600,500, 600,500, 600,500, 600,500, 600,1650, 600,500, 550,550, 550,550, 550,550, 550,550, 550,550, 600,500, 550,550, 600,500, 550,550, 550,550, 550,550, 550,550, 550,550, 550,1700, 600,500, 550,550, 550,550, 550,550, 550,550, 600,500, 600,500, 550,550, 600,500, 600,500, 550,550, 550,550, 550,550, 550,550, 600,500, 550,550, 550,550, 550,550, 600,500, 600,500, 600,500, 600,500, 600,500, 600,500, 550,550, 600,500, 600,500, 600,500, 600,500, 600,500, 600,500, 600,500, 600,500, 550,550, 600,1650, 550,550, 600,1650, 550,500, 600,500, 600,500, 600,500, 600,500, 600,1650, 600,1650, 550,1650, 600,1650, 550,1650, 600,1650, 600,1650, 550,500, 600 };
const uint16_t windIrSignal[] PROGMEM = { 8930,4470, 530,1720, 530,1720, 480,620, 480,620, 530,570, 530,570, 530,1670, 530,1720, 530,1720, 530,1720, 480,1720, 530,570, 530,570, 530,570, 530,570, 530,570, 530,570, 530,570, 580,520, 580,520, 580,520, 580,1670, 530,1720, 530,1670, 580,520, 580,520, 580,570, 530,570, 530,570, 530,570, 530,570, 530,570, 530,570, 530,570, 530,570, 530,570, 530,570, 530,1720, 530,1720, 530,520, 580,520, 580,520, 580,570, 530,570, 530,570, 530,570, 530,570, 530,570, 530,570, 530,570, 530,570, 530,570, 530,570, 530,570, 530,1720, 530,1670, 580,520, 580,520, 580,520, 580,570, 530,570, 530,570, 530,570, 530,570, 530,570, 530,570, 530,570, 530,570, 530,570, 530,570, 530,570, 530,570, 530,570, 530,570, 580,520, 580,520, 580,520, 580,1720, 480,570, 530,570, 530,570, 530,570, 580,520, 580,520, 580,520, 580,520, 580,520, 580,520, 580,1670, 530,570, 530,1720, 530,570, 530,570, 530,570, 530,570, 530,570, 530,1720, 530,1720, 530,1720, 480,1720, 530,570, 530,1720, 530,1720, 480,1720, 530 };

/**
 * LED 블링크 함수
 */
void blinkLED(int delayValue){
  digitalWrite(LED_BUILTIN, HIGH);
  delay(delayValue);
  digitalWrite(LED_BUILTIN, LOW);
}

/**
 * 상태 변경 함수
 */
void changeState(boolean &state){
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
  String switchData = String(cdsFuncState) + ',' + String(airconState) + ',' + String(fanState) + '/' + String(autoFuncState) + ',' 
                          + String(autoAirconState) + ',' + String(autoFanState) + '/' + String(soundFuncState) + ',' + String(windFuncState);
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
bool time_check(unsigned long preMillis, unsigned long delayTime){
  int check  = 0;
  
  // 설정한 delayTime이 지나면 실행
  if(currentMillis - preMillis >= delayTime && previousMillis!=0){
    check = 1;
  }
  
  return check;
}

//TODO
void sound_on_lcd(){
    int analogValue = analogRead(soundSensor);
    // Print a message to the LCD.
    lcd.clear();
    lcd.backlight();                // 백라이트 켜기
    lcd.setCursor(0,0);             // 1번째, 1라인
    lcd.print("SOUND");
    lcd.setCursor(0,1);             // 1번째, 2라인
    lcd.print(analogValue);
    
    delay(900);
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
void sgnRead_on_lcd(byte type){
    String str = "";
    if(type==1) str="IR Send";
    if(type==2) str="Zigbee Send";
    if(type==3) str="Data Send";
    
    // Print a message to the LCD.
    lcd.clear();
    lcd.backlight();                // 백라이트 켜기
    lcd.setCursor(0,0);             // 1번째, 1라인
    lcd.print("BT Read");
    lcd.setCursor(0,1);             // 1번째, 2라인
    lcd.print(str);
}

/**
 * 온습도, 조도 값 전송
 */
void sendTempData(){
    //z는 온습도, 통계 새로고침
    float humi = dht.readHumidity();
    float temp = dht.readTemperature();
    cds = analogRead(cdsPin);

    String sendTempData = String(humi) + "," + String(temp) + "," + String(cds);
    Serial.println(sendTempData);
    BTSerial.write(sendTempData.c_str());
}

/**
 * IR 송신 함수
 */
void irTransmit(byte data){
  long irKeyCodes; // 리모컨 고유 코드
      
  // 신호 구분
  switch (data) {
    //선풍기
    case 'A': irKeyCodes = 0x807FC03F; Serial.println(F("전원")); break;  
    case 'B': irKeyCodes = 0x807FE01F; Serial.println(F("풍량")); break; 
    case 'C': irKeyCodes = 0x807FD02F; Serial.println(F("예약")); break; 
    case 'D': irKeyCodes = 0x807FC837; Serial.println(F("풍향")); break; 
    case 'E': irKeyCodes = 0x807FF00F; Serial.println(F("바람모드")); break; 

    //빔프로젝터
    case 'F': irKeyCodes = 0xBD807F; Serial.println(F("전원/종료")); break; 
    case 'G': irKeyCodes = 0xBD50AF; Serial.println(F("메뉴")); break; 
    case 'H': irKeyCodes = 0xBD1AE5; Serial.println(F("이전 재생")); break; 
    case 'I': irKeyCodes = 0xBDFA05; Serial.println(F("재생/일시정지")); break; 
    case 'J': irKeyCodes = 0xBDEA15; Serial.println(F("다음 재생")); break; 
    case 'K': irKeyCodes = 0xBDD02F; Serial.println(F("상")); break; 
    case 'L': irKeyCodes = 0xBDF00F; Serial.println(F("하")); break; 
    case 'M': irKeyCodes = 0xBD926D; Serial.println(F("좌")); break; 
    case 'N': irKeyCodes = 0xBD52AD; Serial.println(F("우")); break; 
    case 'O': irKeyCodes = 0xBDB04F; Serial.println(F("확인")); break; 
    case 'P': irKeyCodes = 0xBD0AF5; Serial.println(F("뒤로 감기")); break; 
    case 'Q': irKeyCodes = 0xBD2AD5; Serial.println(F("이전")); break; 
    case 'R': irKeyCodes = 0xBD8A75; Serial.println(F("앞으로 감기")); break;
    case 'S': irKeyCodes = 0xBD08F7; Serial.println(F("음량 감소")); break; 
    case 'T': irKeyCodes = 0xBD56A9; Serial.println(F("음소거")); break; 
    case 'U': irKeyCodes = 0xBD30CF; Serial.println(F("음량 증가")); break; 
    case 'V': irKeyCodes = 0xBD20DF; Serial.println(F("외부입력")); break; 
  }
  
  digitalWrite(ir, HIGH);
  Serial.println(F("Sending"));      // 시리얼 모니터 출력
  irsend.sendNEC(irKeyCodes, 32); // 리모컨 고유코드 전송
  digitalWrite(ir, LOW);
}

/**
 * aircon 작동 함수
 */
void airconTransmit(byte data) {
  const uint8_t khz = 38;
  
  if(data == 97){
    // 실행
    irsend.sendRaw_P(onIrSignal, sizeof(onIrSignal) / sizeof(onIrSignal[0]), khz);
  }else if(data == 98){
    // 종료
    irsend.sendRaw_P(offIrSignal, sizeof(offIrSignal) / sizeof(offIrSignal[0]), khz);
  }else if(data == 99){
    if(windFuncState==1){
      // 송풍 20분 후 종료
      irsend.sendRaw_P(windIrSignal, sizeof(windIrSignal) / sizeof(windIrSignal[0]), khz);
      pre_time_set(preAutoWindMillis);                    // 송풍 타이머 설정
      windTimerCnt=1;                                     // 송풍 카운트 1 설정
    }else{
      airconTransmit('b');
    }
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
  pinMode(LED_BUILTIN, OUTPUT); // LED
}

void loop() {
  cur_time_set();               // 타이머 기준 시간 설정
  //LCD OFF 체크
  if(time_check(previousMillis, delayTimer)){
    lcd.noBacklight();   
  }
  
  //블루투스 통신 데이터 수신
  if(BTSerial.available()){
    String receiveData = BTSerial.readString();
    blinkLED(100);
    if (receiveData.length()==1){
      char data = receiveData[0];
      // Line feed 필터, 특정 문자 이외 필터
      if(data != 10 && data > 64 && data < 87){
        //A~V까지 IR 송신에 사용
        irTransmit(data);                       // IR 신호 송신
        pre_time_set(previousMillis);           // LCD 타이머 시작 시간 설정
        sgnRead_on_lcd(1);                      // 송신 여부 LCD 표시
      }else if(data == 87 || data == 88){
        //W, X는 나노보드 모터제어에 사용
        Serial.write(data);                     // 나노보드에 신호 송신
        pre_time_set(previousMillis);           // LCD 타이머 시작 시간 설정
        sgnRead_on_lcd(2);                      // 송신 여부 LCD 표시
      }else if(data == 89){
        //Y는 사운드 센서 값 표시 (디버그용)
        changeState(debugSound);
      }else if(data == 90){
        //Z는 온습도, 통계 데이터 전송
        sendTempData();                         // 온습도, 조도 값 송신
        pre_time_set(previousMillis);           // LCD 타이머 시작 시간 설정
        sgnRead_on_lcd(3);                      // 송신 여부 LCD 표시
      }else if(data == 97 || data == 98 || data == 99){
        //a, b, c는 에어컨 ON, OFF, 송풍에 사용
        airconTransmit(data);                   // 에어컨 신호 송신
        pre_time_set(previousMillis);           // LCD 타이머 시작 시간 설정
        sgnRead_on_lcd(1);                      // 송신 여부 LCD 표시
      }else if(data > 112 && data < 122){
        //q, r, s, t, u, v, w, x, y는 조도센서, 자동취침 기능 사용 여부
        switch (data) {
        case 'q':
          changeState(windFuncState);  
          break;
        case 'r':
          changeState(soundFuncState); 
          break;  
        case 's': 
          changeState(autoFuncState); 
          pre_time_set(previousMillis);         // LCD 타이머 시작 시간 설정
          break;  
        case 't': 
          changeState(autoAirconState);
          break;  
        case 'u': 
          changeState(autoFanState);
          break;  
        case 'v': 
          changeState(cdsFuncState); 
          pre_time_set(previousMillis);         // LCD 타이머 시작 시간 설정
          break; 
        case 'w': 
          changeState(airconState);
          break;  
        case 'x': 
          //기능 삭제
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
      
      if(separator=="off") offTimer = val.toInt()*1000*60;
      if(separator=="on") onTimer = val.toInt()*1000*60;
    }
  }

  // 조도센서 기능 사용했을 때
  if(cdsFuncState==1){
    cds = analogRead(cdsPin);                         // A1으로 들어오는 값을 cds에 저장
    if(cds < cdsCnd && cnt==1){                       // cds값이 cdsCnd보다 작으면 불이 켜져있음
      if(airconState==1) airconTransmit('a');         // 에어컨 켜기
      if(fanState==1) irTransmit('A');                // 선풍기 켜기
      cnt=0;                                          // 설정에 따른 중복 작동 방지
    }else if(cds >= cdsCnd && cnt==0){                // cds값이 cdsCnd보다 크면 불이 꺼져있음
      if(airconState==1) airconTransmit('c');         // 에어컨 끄기
      if(fanState==1) irTransmit('A');                // 선풍기 끄기
      cnt=1;                                          // 설정에 따른 중복 작동 방지
    }
    
    delay(100);                                       // 조도 센서 잡음값 필터
  }

  // 자동취침 기능 사용했을 때
  if(autoFuncState==1){
    if(preAutoOffMillis==1&&offTimerCnt==0&&onTimerCnt==0){
      pre_time_set(preAutoOffMillis);                     // 시작 타이머 시작
      offTimerCnt=1;                                      // 종료 카운트 1 설정
    }
    if(time_check(preAutoOffMillis, offTimer) && offTimerCnt==1){
      if(autoAirconState==1) airconTransmit('c');         // 에어컨 송풍 켜기 또는 종료
      if(autoFanState==1) irTransmit('A');                // 선풍기 끄기
      if(windFuncState==1) onTimer += windTimer;          // 송풍 체크에 따른 종료 시간 지연 설정
      pre_time_set(preAutoOnMillis);                      // 종료 타이머 시작
      offTimerCnt=0;                                      // 종료 카운트 0 설정
      onTimerCnt=1;                                       // 시작 카운트 1 설정
    }
    if(time_check(preAutoOnMillis, onTimer) && onTimerCnt==1){
      if(autoAirconState==1) airconTransmit('a');         // 에어컨 켜기
      if(autoFanState==1) irTransmit('A');                // 선풍기 켜기
      if(windFuncState==1) onTimer -= windTimer;          // 송풍 체크에 따른 종료 시간 지연 설정
      pre_time_set(preAutoOffMillis);                     // 시작 타이머 시작
      offTimerCnt=1;                                      // 종료 카운트 1 설정
      onTimerCnt=0;                                       // 시작 카운트 0 설정
    }
  }
  
  // 에어컨 종료 전, 30분간 송풍 사용  
  if(time_check(preAutoWindMillis, windTimer) && windTimerCnt==1 && windFuncState==1){
    airconTransmit('b');                                // 에어컨 끄기
    windTimerCnt=0;                                     // 송풍 카운트 0 설정
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
        if(cds > cdsCnd){               // 전등이 꺼져있으면, 전등 켜기
          Serial.write('W');
        } else if(cds <= cdsCnd){      // 전등이 켜져있으면, 전등 끄기
          Serial.write('X');
        }
        countClap = 0;
      }
    }
  }

  if(debugSound==1){
    sound_on_lcd();
  }
}
