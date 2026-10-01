// ESP32 + DHT22 + 2 LED + Blynk IoT
// Könyvtárak: Blynk, DHT sensor library (Adafruit), Adafruit Unified Sensor

// ---- Blynk adatok (a Blynk Console-ból másold be) ----
#define BLYNK_TEMPLATE_ID   "T********F"
#define BLYNK_TEMPLATE_NAME "Kezdo projekt"
#define BLYNK_AUTH_TOKEN "********"

#define BLYNK_PRINT Serial

#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>
#include <DHT.h>

// ---- WiFi ----
char ssid[] = "********";
char pass[] = "********";

// ---- Lábak ----
#define DHT_PIN   4    // DHT22 adatláb
#define DHT_TYPE  DHT22
#define LED1_PIN  26   // kézzel kapcsolható LED
#define LED2_PIN  23   // automatikus LED (28 °C felett)

// ---- Virtuális lábak (Blynk) ----
// V0: hőmérséklet (Gauge / Value Display)
// V1: LED1 kapcsoló (Switch)
// V2: LED1 állapot (LED widget)
// V3: LED2 állapot (LED widget)
#define VPIN_TEMP       V0
#define VPIN_LED1_SW    V1
#define VPIN_LED1_STATE V2
#define VPIN_LED2_STATE V3

const float TEMP_LIMIT = 28.0;   // ennél melegebbnél kapcsol be a LED2
const float HYSTERESIS = 0.5;    // villogás elkerülése (28.0 fölött be, 27.5 alatt ki)

DHT dht(DHT_PIN, DHT_TYPE);
BlynkTimer timer;

bool led1On = false;
bool led2On = false;

void setLed1(bool on) {
  led1On = on;
  digitalWrite(LED1_PIN, on ? HIGH : LOW);
  Blynk.virtualWrite(VPIN_LED1_STATE, on ? 255 : 0);
}

void setLed2(bool on) {
  led2On = on;
  digitalWrite(LED2_PIN, on ? HIGH : LOW);
  Blynk.virtualWrite(VPIN_LED2_STATE, on ? 255 : 0);
}

// Kapcsoló az appból
BLYNK_WRITE(VPIN_LED1_SW) {
  setLed1(param.asInt() == 1);
}

// Csatlakozáskor szinkronizáljuk az állapotokat
BLYNK_CONNECTED() {
  Blynk.syncVirtual(VPIN_LED1_SW);
  Blynk.virtualWrite(VPIN_LED1_STATE, led1On ? 255 : 0);
  Blynk.virtualWrite(VPIN_LED2_STATE, led2On ? 255 : 0);
}

void readAndSend() {
  float t = dht.readTemperature();
  if (isnan(t)) {
    Serial.println("DHT22 olvasási hiba!");
    return;
  }

  Serial.printf("Hőmérséklet: %.1f °C\n", t);
  Blynk.virtualWrite(VPIN_TEMP, t);

  // LED2 automatika hiszterézissel
  if (!led2On && t > TEMP_LIMIT) {
    setLed2(true);
  } else if (led2On && t < TEMP_LIMIT - HYSTERESIS) {
    setLed2(false);
  }
}

void setup() {
  Serial.begin(115200);
  pinMode(LED1_PIN, OUTPUT);
  pinMode(LED2_PIN, OUTPUT);
  digitalWrite(LED1_PIN, LOW);
  digitalWrite(LED2_PIN, LOW);

  dht.begin();
  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);

  timer.setInterval(2000L, readAndSend);  // DHT22 max ~0,5 Hz, 2 mp megfelelő
}

void loop() {
  Blynk.run();
  timer.run();
}
