#include <ArduinoJson.h>
#include <IOXhop_FirebaseStream.h>
#include <IOXhop_FirebaseESP32.h>
#include <M5StickCPlus.h>
#include <WiFi.h>



#define	VERSION_MAJOR		0
#define	VERSION_SUB			0
#define	VERSION_SUB_SUB		1

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



  // WiFi Connected
  Serial.println("\nWiFi Connected.");
  Serial.println(WiFi.localIP());
  M5.Lcd.setRotation(3);
  M5.Lcd.fillScreen(BLACK);
  M5.Lcd.setCursor(10, 30);
  M5.Lcd.printf("Ver: %d.%d-%d", VERSION_MAJOR,VERSION_SUB,VERSION_SUB_SUB);

  // Firebase初期化
  Firebase.begin(FIREBASE_DATABASE_URL);

  Firebase.stream("/M5Stack/counter",[](FirebaseStream stream) {
    String eventType = stream.getEvent();
    eventType.toLowerCase();
    Serial.println(eventType);

    if(eventType == "put"){
      String path = stream.getPath();
      String data = stream.getDataString();
      //ディスプレイ表示
      M5.Lcd.setCursor(10,30);
      M5.Lcd.fillScreen(BLACK);
      M5.Lcd.setTextColor(WHITE);
      M5.Lcd.printf("M5Stack: %s", &data[0]);
      M5.Lcd.setCursor(10, 50);
      M5.Lcd.printf("M5StickC: %d", count);
    }


  });


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

  if (M5.BtnA.wasReleased()) {
    // カウントアップ
    count++;

    // ディスプレイ表示
    M5.Lcd.setCursor(10, 30);
    M5.Lcd.fillScreen(RED);
    M5.Lcd.setTextColor(YELLOW);
    M5.Lcd.printf("Count Up: %d", count);
  }

  if(M5.BtnB.wasReleased()) {
    // カウントダウン
    count--;

    // ゼロ以下にはしない
    if (count <= 0) count = 0;

    // ディスプレイ表示
    M5.Lcd.setCursor(10, 30);
    M5.Lcd.fillScreen(GREEN);
    M5.Lcd.setTextColor(BLACK);
    M5.Lcd.setTextSize(3);
    M5.Lcd.printf("Count Down: %d", count);
  }


  //2秒押し続けて離すと送信
  if(M5.BtnA.wasReleasefor(2000)) {
    // ディスプレイ表示
    M5.Lcd.setCursor(10, 30);
    M5.Lcd.fillScreen(BLUE);
    M5.Lcd.setTextColor(WHITE);
    M5.Lcd.setTextSize(3);
    M5.Lcd.printf("Count Send: %d", count);

    // カウント送信
    Firebase.setInt("/M5StickC/counter", count);
    
  }

}






