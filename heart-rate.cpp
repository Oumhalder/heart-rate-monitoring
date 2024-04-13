#include <Wire.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <Arduino_JSON.h>

#include "PageIndex.h"

#define PulseSensor_PIN 34
#define LED_PIN         23 
#define Button_PIN      32

const char* ssid = "****";
const char* password = "****";

unsigned long previousMillisGetHB = 0;
unsigned long previousMillisResultHB = 0;

const long intervalGetHB = 35;
const long intervalResultHB = 100;

int timer_Get_BPM = 0;

int PulseSensorSignal;
int UpperThreshold = 520;
int LowerThreshold = 500; 

int cntHB = 0;
boolean ThresholdStat = true;
int BPMval = 0;

bool get_BPM = false;

byte tSecond = 0;
byte tMinute = 0;
byte tHour   = 0;

char tTime[10];

const char* PARAM_INPUT_1 = "BTN_Start_Get_BPM";
String BTN_Start_Get_BPM = "";

JSONVar JSON_All_Data;

AsyncWebServer server(80);
AsyncEventSource events("/events");

void GetHeartRate() {
  unsigned long currentMillisGetHB = millis();

  if (currentMillisGetHB - previousMillisGetHB >= intervalGetHB) {
    previousMillisGetHB = currentMillisGetHB;

    PulseSensorSignal = analogRead(PulseSensor_PIN);

    if (PulseSensorSignal > UpperThreshold && ThresholdStat == true) {
      if (get_BPM == true) cntHB++;
      ThresholdStat = false;
      digitalWrite(LED_PIN,HIGH);
    }

    if (PulseSensorSignal < LowerThreshold) {
      ThresholdStat = true;
      digitalWrite(LED_PIN,LOW);
    }
  }

  unsigned long currentMillisResultHB = millis();

  if (currentMillisResultHB - previousMillisResultHB >= intervalResultHB) {
    previousMillisResultHB = currentMillisResultHB;

    if (get_BPM == true) {
      timer_Get_BPM++;
      if (timer_Get_BPM > 10) {
        timer_Get_BPM = 1;

        tSecond += 10;
        if (tSecond >= 60) {
          tSecond = 0;
          tMinute += 1;
        }
        if (tMinute >= 60) {
          tMinute = 0;
          tHour += 1;
        }

        sprintf(tTime, "%02d:%02d:%02d",  tHour, tMinute, tSecond);

        BPMval = cntHB * 6;
        Serial.print("BPM : ");
        Serial.println(BPMval);
        
        cntHB = 0;
      }
    }
  }
}

void setup() {
  Serial.begin(115200);
  analogReadResolution(10);

  pinMode(LED_PIN,OUTPUT); 
  pinMode(Button_PIN, INPUT_PULLUP);

  sprintf(tTime, "%02d:%02d:%02d",  tHour, tMinute, tSecond);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  int connecting_process_timed_out = 20;
  connecting_process_timed_out = connecting_process_timed_out * 2;
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    if (connecting_process_timed_out > 0) connecting_process_timed_out--;
    if (connecting_process_timed_out == 0) {
      delay(1000);
      ESP.restart();
    }
  }

  server.on("/", HTTP_GET, [](AsyncWebServerRequest * request) {
    request->send_P(200, "text/html", MAIN_page);
  });

  events.onConnect([](AsyncEventSourceClient * client) {
    if (client->lastId()) {
      Serial.printf("Client reconnected! Last message ID that it got is: %u\n", client->lastId());
    }
    client->send("hello!", NULL, millis(), 10000);
  });

  server.on("/BTN_Comd", HTTP_GET, [] (AsyncWebServerRequest * request) {
    if (request->hasParam(PARAM_INPUT_1)) {
      BTN_Start_Get_BPM = request->getParam(PARAM_INPUT_1)->value();
      Serial.println();
      Serial.print("BTN_Start_Get_BPM : ");
      Serial.println(BTN_Start_Get_BPM);
    }
    else {
      BTN_Start_Get_BPM = "No message";
      Serial.println();
      Serial.print("BTN_Start_Get_BPM : ");
      Serial.println(BTN_Start_Get_BPM);
    }
    request->send(200, "text/plain", "OK");
  });

  server.addHandler(&events);
  server.begin();

  Serial.println("------------");
  Serial.print("ESP32 IP address : ");
  Serial.println(WiFi.localIP());
  Serial.println("------------");
}

void loop() {
  if (digitalRead(Button_PIN) == LOW || BTN_Start_Get_BPM == "START" || BTN_Start_Get_BPM == "STOP") {
    delay(100);
    BTN_Start_Get_BPM = "";
    cntHB = 0;
    BPMval = 0;
    tSecond = 0;
    tMinute = 0;
    tHour   = 0;
    sprintf(tTime, "%02d:%02d:%02d",  tHour, tMinute, tSecond);
    get_BPM = !get_BPM;
    if (get_BPM == true) {
      Serial.println("Start Getting BPM");
    }
    else {
      Serial.println("STOP");
    }
  }

  GetHeartRate();
}
