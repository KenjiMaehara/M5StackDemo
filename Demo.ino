#include <ArduinoJson.h>
#include <IOXhop_FirebaseStream.h>
#include <IOXhop_FirebaseESP32.h>
#include <M5Stack.h>
#include <WiFi.h>

#include "AudioFileSourceSD.h"
#include "AudioFileSourceID3.h"
#include "AudioGeneratorMP3.h"
#include "AudioOutputI2S.h"





#define WIFI_SSID "20200815me"
#define WIFI_PASSWORD "0815asdf"

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

    if(emgAramCount > -1)
    {
      emgAramCount++;
    }

    if(emgAramSoundCount > -1)
    {
      emgAramSoundCount++;
    }

    usecCount = 0;

  }
}



// カウント初期化
int count = 0;
int newCount = 0;
int stopSound = false;


AudioGeneratorMP3 *mp3;
AudioFileSourceSD *file;
AudioOutputI2S *out;
AudioFileSourceID3 *id3;


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
  M5.Lcd.println("Button Click!");



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




void playMP3(char *filename){
  file = new AudioFileSourceSD(filename);
  id3 = new AudioFileSourceID3(file);
  out = new AudioOutputI2S(0, 1); // Output to builtInDAC
  out->SetOutputModeMono(true);
  out->SetGain(1.0);
  mp3 = new AudioGeneratorMP3();
  mp3->begin(id3, out);
  while(mp3->isRunning()) {
    if (!mp3->loop()) mp3->stop();
    if (emgAramCount > 3){
      mp3->stop();
      break;
    } 

  }
}




void loop() {
  M5.update();

  if (M5.BtnA.wasReleased()) {
    // カウントアップ
    count++;

    // ディスプレイ表示
    M5.Lcd.setCursor(10, 100);
    M5.Lcd.fillScreen(YELLOW);
    M5.Lcd.setTextColor(BLACK);
    M5.Lcd.setTextSize(3);
    M5.Lcd.printf("Count Up: %d", count);
  }

  if(M5.BtnC.wasReleased()) {
    // カウントダウン
    count--;

    // ゼロ以下にはしない
    if (count <= 0) count = 0;

    // ディスプレイ表示
    M5.Lcd.setCursor(10, 100);
    M5.Lcd.fillScreen(GREEN);
    M5.Lcd.setTextColor(BLACK);
    M5.Lcd.setTextSize(3);
    M5.Lcd.printf("Count Down: %d", count);
  }

  if(M5.BtnB.wasReleased()) {
    // ディスプレイ表示
    M5.Lcd.setCursor(10, 100);
    M5.Lcd.fillScreen(BLUE);
    M5.Lcd.setTextColor(WHITE);
    M5.Lcd.setTextSize(3);
    M5.Lcd.printf("Count Send: %d", count);

    // カウント送信
    Firebase.setInt("/M5Stack/counter02", count);
    

  }

  #if 1
  if(secCount > 5 && emgAramCount == -1){

    M5.Lcd.setCursor(10, 100);
    M5.Lcd.fillScreen(GREEN);
    M5.Lcd.setTextColor(BLACK);
    M5.Lcd.setTextSize(3);
    M5.Lcd.printf("Count Down: %d", count);


    newCount = Firebase.getInt("/M5Stack/counter02");

    if(count != newCount)
    {
          // ディスプレイ表示
      M5.Lcd.setCursor(10, 100);
      M5.Lcd.fillScreen(RED);
      M5.Lcd.setTextColor(BLACK);
      M5.Lcd.setTextSize(3);
      M5.Lcd.printf("Count Send: %d", newCount);
      
      count = newCount;
      emgAramCount=0;
      playMP3("/Warning-Alarm01-1L.mp3");
      //emgAramCount=0;
    }

    secCount=0;


  }
  #endif


  #if 1
  if(emgAramCount > 3 && stopSound == false){
    mp3->stop();
    stopSound = true;
  }
  #endif



  if(emgAramCount > 10){

    count = 0;
    newCount = 0;

    // カウント送信
    Firebase.setInt("/M5Stack/counter02", 0);

    // ディスプレイ表示
    M5.Lcd.setCursor(10, 100);
    M5.Lcd.fillScreen(GREEN);
    M5.Lcd.setTextColor(BLACK);
    M5.Lcd.setTextSize(3);
    M5.Lcd.printf("Count Down: %d", count);


    stopSound=false;
    secCount=0;
    emgAramCount=-1;

  }


}






