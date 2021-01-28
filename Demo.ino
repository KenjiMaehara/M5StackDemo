#include <ArduinoJson.h>
#include <IOXhop_FirebaseStream.h>
#include <IOXhop_FirebaseESP32.h>
#include <M5Stack.h>
#include <WiFi.h>


#define WIFI_SSID "20200815me"
#define WIFI_PASSWORD "0815asdf"
//test03

// FirebaseのデータベースURL（ご自身のデータベースURLに変更してください）
#define FIREBASE_DATABASE_URL "https://m5data-befb9-default-rtdb.firebaseio.com"


//timer interrupt variable.
volatile unsigned long usecCount = 0;
hw_timer_t *interrupptTimer = NULL;
portMUX_TYPE mutex = portMUX_INITIALIZER_UNLOCKED;

int secCount = 0;
int emgAramCount = -1;
int emgAramSoundCount = -1;


//Timer count
void IRAM_ATTR usecTimer()
{
  portENTER_CRITICAL_ISR(&mutex);
  usecCount += 5;
  portEXIT_CRITICAL_ISR(&mutex);

  if(((usecCount / 1000000) % 60) > 0){
    if(secCount > -1)
    {
      secCount++;
    }

    usecCount = 0;

  }
}



// カウント初期化
int count = 0;


void setup() {
  M5.begin();

  // Wi-Fi接続
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("connecting");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }
  Serial.println();

  // Firebase初期化
  Firebase.begin(FIREBASE_DATABASE_URL);

  // WiFi Connected
  Serial.println("\nWiFi Connected.");
  Serial.println(WiFi.localIP());
  M5.Lcd.setTextSize(3);
  M5.Lcd.setCursor(10, 100);
  M5.Lcd.println("Button Click! Ver0.0-4");



 //interrupt timer setting
  //timerBegin is count per 100 microsec.
  interrupptTimer = timerBegin(0, 80, true);

  //interrupt method setting
  timerAttachInterrupt(interrupptTimer, &usecTimer, true);

  //interrupt timing setting.
  timerAlarmWrite(interrupptTimer, 5, true);
  timerAlarmDisable(interrupptTimer);

  portENTER_CRITICAL(&mutex);
  timerAlarmEnable(interrupptTimer);
  portEXIT_CRITICAL(&mutex);
}



void loop() {
  M5.update();

 
  #if 1
  if(secCount > 10){

    count++;
    Firebase.setInt("/M5Stack/counter02", count);
    secCount=0;


  }
  #endif


}






