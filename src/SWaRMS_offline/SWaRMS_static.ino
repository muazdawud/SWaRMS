

#include <Arduino.h>
#include "Stepper.h"
#include "HX711.h"
#include <WiFi.h>
#include <WiFiUdp.h>
#include <WebServer.h>
#include <ESPmDNS.h>
#include <LittleFS.h>
#include <WebSocketsClient.h>
#include <WebSocketsServer.h>


#define stepSpeed 10U
#define stepPerRevolution 2048U

#define CP_Distance 5U
#define IG_Distance 3U
#define EG_Distance 3U

#define loadCellSck 22U
#define loadCellData 23U
#define tareWeight 2280.f
#define averageRead 3
#define W_Threshold 30

#define Fan_Pin 12U

#define RedLED 25
#define BlueLED 33
#define YellowLED 32

#define CP_Button 35
#define EG_Button 34
#define IG_Button 21

#define USERNAME "admin"
#define PASSWORD "0000"


const uint8_t CP_Pin[] = { 15, 2, 4, 16 };
const uint8_t EG_Pin[] = { 17, 5, 18, 19 };
const uint8_t IG_PIN[] = { 13, 14, 27, 26 };


Stepper IG_Stepper(stepPerRevolution, IG_PIN[0], IG_PIN[1], IG_PIN[2], IG_PIN[3]);
Stepper EG_Stepper(stepPerRevolution, EG_Pin[0], EG_Pin[1], EG_Pin[2], EG_Pin[3]);
Stepper CP_Stepper(stepPerRevolution, CP_Pin[0], CP_Pin[1], CP_Pin[2], CP_Pin[3]);
WebSocketsServer webSocket = WebSocketsServer(81);
WebServer server(80);
HX711 scale;
WiFiUDP UDP;
IPAddress ip;
IPAddress timeServerIP;


volatile bool timer_flag = false;
volatile uint32_t interrupt_counter = 0;
volatile uint32_t startup_timer_sync = 0;
portMUX_TYPE timerMux = portMUX_INITIALIZER_UNLOCKED;
hw_timer_t *timer = NULL;


uint8_t UNIVERSAL_LOCK = 0;
uint8_t LTS = 0;

volatile uint8_t oldState = 0;
volatile uint8_t startup_flag = 0;
uint8_t stepping = 0;

uint32_t weight = 0;
volatile uint32_t count = 0;
volatile uint8_t test_weight = 0;
uint8_t weight_redundant_check = 0;

volatile uint8_t CP_State = 0;
volatile uint8_t IG_State = 0;
volatile uint8_t EG_State = 0;

uint8_t CP_Position = 0;
uint8_t IG_Position = 0;
uint8_t EG_Position = 0;

volatile uint8_t CP_Debounce = 0;
volatile uint8_t IG_Debounce = 0;
volatile uint8_t EG_Debounce = 0;

volatile uint8_t fan_Flag = 0;
volatile uint32_t fan_counter = 0;

uint8_t auth_flag = 0;

const char *ssid = "REDMI A7 Pro";
const char *password = "ABCD1234";

const char *mDNS_name = "SWaRMS_Dashboard";

const char *NTPServerName = "time.nist.gov";
const int NTP_PACKET_SIZE = 48;
byte NTPBuffer[NTP_PACKET_SIZE];
uint32_t timeUNIX = 0;
unsigned long prevActualTime = 0;
volatile uint32_t NTPcount = 0;
volatile uint8_t updateTime = 0;
uint8_t timeRoutine = 0;

String date = "--";
String timeStr = "--";
String location = "Bayero University Kano - Rimin Gata";
String ambientTemp = "--";
String ambientHum = "--";
String innerTemp = "--";
String innerHum = "--";


void CP_Step();
void EG_Step();
void IG_Step();
void initScale();
void testStepper();
void interrupt_routine();
uint32_t handlePollScale();


void startUDP();
void notFound();
void loginAuth();
uint32_t getTime();
void startLittleFS();
void startWebServer();
void startWebSocket();
void interrupt_routine();
bool streamFile(String filename);
inline int getHours(uint32_t UNIXTime);
void sendNTPpacket(IPAddress &address);
inline int getMinutes(uint32_t UNIXTime);
void webSocketEvent(uint8_t num, WStype_t type, uint8_t *payload, size_t length);


void IRAM_ATTR onTimer() {

  portENTER_CRITICAL_ISR(&timerMux);
  interrupt_counter++;
  portEXIT_CRITICAL_ISR(&timerMux);

  if ((interrupt_counter - startup_timer_sync) >= 5) {

    if (startup_flag) {
      digitalWrite(RedLED, (oldState ^= 1));
      digitalWrite(BlueLED, (oldState));
      digitalWrite(YellowLED, (oldState));
    } else {
      portENTER_CRITICAL_ISR(&timerMux);
      timer_flag = true;
      portEXIT_CRITICAL_ISR(&timerMux);
    }

    portENTER_CRITICAL_ISR(&timerMux);
    startup_timer_sync = interrupt_counter;
    portEXIT_CRITICAL_ISR(&timerMux);
  }

  portENTER_CRITICAL_ISR(&timerMux);
  if ((interrupt_counter - fan_counter) > 60) {
    fan_counter = interrupt_counter;
    fan_Flag ^= 1;
  }
  portEXIT_CRITICAL_ISR(&timerMux);

  portENTER_CRITICAL_ISR(&timerMux);
  if ((interrupt_counter - NTPcount) > 600) {
    NTPcount = interrupt_counter;
    updateTime ^= 1;
  }
  portEXIT_CRITICAL_ISR(&timerMux);
}


void setup() {

  Serial.begin(115200);

  pinMode(RedLED, OUTPUT);
  pinMode(BlueLED, OUTPUT);
  pinMode(YellowLED, OUTPUT);

  pinMode(CP_Button, INPUT);
  pinMode(EG_Button, INPUT);
  pinMode(IG_Button, INPUT);

  pinMode(Fan_Pin, OUTPUT);

  timer = timerBegin(10000);
  timerAttachInterrupt(timer, &onTimer);
  timerAlarm(timer, 100, true, 0);

  startup_flag = 1;

  testStepper();

  initScale();

  startLittleFS();

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
  }

  Serial.println(WiFi.localIP());

  startUDP();
  startWebServer();
  startWebSocket();

  WiFi.hostByName(NTPServerName, timeServerIP);

  while (!MDNS.begin(mDNS_name)) {}
  MDNS.addService("http", "tcp", 80);
}


void loop() {

  server.handleClient();
  webSocket.loop();

  if (timer_flag) {

    portENTER_CRITICAL(&timerMux);
    timer_flag = false;
    portEXIT_CRITICAL(&timerMux);

    if (!startup_flag) {
      interrupt_routine();
    }
  }

  if (CP_State) {

    CP_Step();
  } else if (IG_State && !UNIVERSAL_LOCK && !(CP_State)) {

    IG_Step();

    if (LTS) {
      UNIVERSAL_LOCK = 1;
      LTS = 0;
    }
  } else if (EG_State && !(CP_State) && !(IG_State)) {

    EG_Step();
  }

  if (test_weight && scale.is_ready()) {

    weight = handlePollScale();

    if (/*weight >= ((W_Threshold / 10) * 7)*/ weight >= W_Threshold) {
      digitalWrite(YellowLED, HIGH);
    } else {
      digitalWrite(YellowLED, LOW);
    }
  }

  if (weight >= ((W_Threshold / 10) * 5)) {
    digitalWrite(Fan_Pin, HIGH);
  }else{
    digitalWrite(Fan_Pin, LOW);
  }

  if (updateTime) {
    sendNTPpacket(timeServerIP);
    timeRoutine = 1;
    updateTime = 0;
  }
}


void initScale() {

  scale.begin(loadCellData, loadCellSck);
  scale.set_scale(tareWeight);
  scale.tare();

  weight = handlePollScale();

  if (weight >= ((W_Threshold / 10) * 7)) {
    digitalWrite(YellowLED, HIGH);
  } else {
    digitalWrite(YellowLED, LOW);
  }
}


void testStepper() {

  IG_Stepper.setSpeed(stepSpeed);
  EG_Stepper.setSpeed(stepSpeed);
  CP_Stepper.setSpeed(stepSpeed);

  IG_Stepper.step(stepPerRevolution);
  IG_Stepper.step(-1 * stepPerRevolution);

  EG_Stepper.step(stepPerRevolution);
  EG_Stepper.step(-1 * stepPerRevolution);

  CP_Stepper.step(stepPerRevolution);
  CP_Stepper.step(-1 * stepPerRevolution);

  startup_flag = 0;

  digitalWrite(RedLED, LOW);
  digitalWrite(BlueLED, HIGH);
  digitalWrite(YellowLED, LOW);
}


void startWebServer() {

  server.on("/", HTTP_GET, [] {
    if (!streamFile("/index.html")) {
      streamFile("/notFound.html");
    }
  });

  server.on("/$login", HTTP_POST, loginAuth);

  server.on("/HomePage", HTTP_GET, [] {
    if (!streamFile("/homepage.html")) {
      streamFile("/notFound.html");
    }
  });

  server.onNotFound(notFound);

  server.begin();
}


void startWebSocket() {

  webSocket.begin();
  webSocket.onEvent(webSocketEvent);
}


void startLittleFS() {

  LittleFS.begin(true);
}



void interrupt_routine() {

  stepping = (CP_State) | (EG_State) | (IG_State);

  if (!digitalRead(CP_Button) && !(stepping)) {
    if (CP_Debounce && (!EG_Debounce && !IG_Debounce)) {
      CP_State = 1;
      CP_Debounce = 0;
    } else {
      CP_Debounce = 1;
    }

  } else {
    CP_Debounce = 0;
  }

  if (!digitalRead(IG_Button) && !(stepping)) {
    if (IG_Debounce && (!EG_Debounce && !CP_Debounce)) {
      if (!UNIVERSAL_LOCK) {
        IG_State = 1;
        IG_Debounce = 0;
      }
    } else {
      IG_Debounce = 1;
    }

  } else {
    IG_Debounce = 0;
  }

  if (!digitalRead(EG_Button) && !(stepping)) {
    if (EG_Debounce && (!IG_Debounce && !CP_Debounce)) {
      EG_State = 1;
      EG_Debounce = 0;
    } else {
      EG_Debounce = 1;
    }

  } else {
    EG_Debounce = 0;
  }

  if ((interrupt_counter - count) >= 100) {
    test_weight = 1;
    count = interrupt_counter;
  }

  if (weight >= W_Threshold) {
    if (++weight_redundant_check > 3) {
      if (!(IG_Position) && !(UNIVERSAL_LOCK)) {
        if (CP_Position) {

          CP_State = 1;
        }
        if (EG_Position) {

          EG_State = 1;
        }

        LTS = 1;
        IG_State = 1;
      }

      weight_redundant_check = 0;
    }
  } else {
    LTS = 0;
    UNIVERSAL_LOCK = 0;
    weight_redundant_check = 0;
  }

  if (timeRoutine && !stepping) {

    uint32_t timeL = getTime();

    if (timeL) {
      timeUNIX = timeL;
    }

    uint32_t actualTime = timeUNIX;

    if (actualTime != prevActualTime && timeUNIX != 0) {
      prevActualTime = actualTime;
      timeStr = String(getHours(actualTime)) + ":" + String(getMinutes(actualTime));
    }

    timeRoutine = 0;
  }

  stepping = 0;
}


uint32_t handlePollScale() {

  int32_t scaleAverage = 0;

  scaleAverage = scale.get_units();

  test_weight = 0;

  if (scaleAverage < 0) {
    return 0;
  }

  return scaleAverage;
}


void CP_Step() {

  oldState = 1;

  int8_t steps = (CP_Position) ? -1 : 1;

  for (uint8_t i = 0; i < CP_Distance; i++) {
    for (uint16_t k = 0; k < stepPerRevolution; k++) {
      CP_Stepper.step(steps);
    }
    digitalWrite(BlueLED, (oldState ^= 1));
  }

  digitalWrite(BlueLED, HIGH);
  CP_State = 0;
  CP_Position ^= 1;
  oldState = 0;
}


void IG_Step() {

  oldState = 1;

  int8_t steps = (IG_Position) ? -1 : 1;

  for (uint8_t i = 0; i < IG_Distance; i++) {
    for (uint16_t k = 0; k < stepPerRevolution; k++) {
      IG_Stepper.step(steps);
    }
    digitalWrite(BlueLED, (oldState ^= 1));
  }

  digitalWrite(BlueLED, HIGH);
  IG_State = 0;
  IG_Position ^= 1;
  oldState = 0;
}


void EG_Step() {

  oldState = 1;

  int8_t steps = (EG_Position) ? -1 : 1;

  for (uint8_t i = 0; i < EG_Distance; i++) {
    for (uint16_t k = 0; k < stepPerRevolution; k++) {
      EG_Stepper.step(steps);
    }
    digitalWrite(BlueLED, (oldState ^= 1));
  }

  digitalWrite(BlueLED, HIGH);
  EG_State = 0;
  EG_Position ^= 1;
  oldState = 0;
}


bool streamFile(String filename) {

  if (filename == "/index.html") {
    auth_flag = 1;
  }

  if (auth_flag) {

    String path = filename;

    String contentType = "";

    if (filename.endsWith(".html") || filename.endsWith(".hml")) {
      contentType = "text/html";
    } else if (filename.endsWith(".css")) {
      contentType = "text/css";
    } else if (filename.endsWith(".js")) {
      contentType = "application/javascript";
    } else {
      contentType = "text/plain";
    }

    File file = LittleFS.open(path);

    if (!file) {
      return 0;
    }

    size_t sent = server.streamFile(file, contentType);
    file.close();

    if (filename == "/index.html") {
      auth_flag = 0;
    }

    return 1;
  } else {
    server.sendHeader("Location", "/");
    server.send(303, "text/plain", "Unauthorized Access");

    auth_flag = 0;
  }

  return 0;
}


void notFound() {

  if (!auth_flag) {

    auth_flag = 1;
  }

  streamFile("/notFound.html");

  auth_flag = 0;
}


void loginAuth() {

  if (!server.hasArg("username") || !server.hasArg("password") || server.arg("username").isEmpty() || server.arg("password").isEmpty()) {
    server.send(400, "text/plain", "Unauthorized");

    auth_flag = 0;

    return;
  }

  else if (server.arg("username") == USERNAME && server.arg("password") == PASSWORD) {
    server.sendHeader("Location", "/HomePage");
    server.send(200, "text/plain", "Login Successful. Redirecting...");

    auth_flag = 1;

    return;
  }

  else {
    server.send(400, "text/plain", "Unauthorized");

    auth_flag = 0;

    return;
  }
}


void webSocketEvent(uint8_t num, WStype_t type, uint8_t *payload, size_t length) {

  switch (type) {
    case WStype_DISCONNECTED:
      break;
    case WStype_CONNECTED:
      {
        ip = webSocket.remoteIP(num);
      }
      break;
    case WStype_TEXT:
      {
        String text = "";
        char *data = (char *)payload;

        if (strcmp(data, "Refresh") == 0) {

          text = "Station Info: " + date + "," + timeStr + "," + String((int)num) + "," + location + "," + ambientTemp + "," + ambientHum + "," + String((int)UNIVERSAL_LOCK) + "," + String(W_Threshold) + "," + String(weight) + "," + innerTemp + "," + innerHum + "," + String(fan_Flag);
          webSocket.sendTXT(num, text.c_str());
        }
      }
  }
}


void startUDP() {
  UDP.begin(123);
}


uint32_t getTime() {
  if (UDP.parsePacket() == 0) {
    return 0;
  }
  UDP.read(NTPBuffer, NTP_PACKET_SIZE);
  uint32_t NTPTime = (NTPBuffer[40] << 24) | (NTPBuffer[41] << 16) | (NTPBuffer[42] << 8) | NTPBuffer[43];

  const uint32_t seventyYears = 2208988800UL;

  uint32_t UNIXTime = NTPTime - seventyYears;
  return UNIXTime;
}


void sendNTPpacket(IPAddress &address) {
  memset(NTPBuffer, 0, NTP_PACKET_SIZE);

  NTPBuffer[0] = 0b11100011;

  UDP.beginPacket(address, 123);
  UDP.write(NTPBuffer, NTP_PACKET_SIZE);
  UDP.endPacket();
}


inline int getMinutes(uint32_t UNIXTime) {
  return UNIXTime / 60 % 60;
}


inline int getHours(uint32_t UNIXTime) {
  return (UNIXTime / 3600 % 24) + 1;
}